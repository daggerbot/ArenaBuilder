/*
 * Copyright (c) 2022 Martin Mills <daggerbot@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

use std::cell::Cell;
use std::collections::HashMap;
use std::rc::Rc;
use std::sync::mpsc::Sender;

use gl::types::*;

use crate::data::DataLoader;
use crate::error::GenericError;
use crate::render::{GlExt, JobDispatch, RenderState};

const SHADER_NAME_PREFIX: &'static str = "shaders/glsl-1.30/";

/// OpenGL shader (stage) wrapper.
struct Shader {
    id: Rc<Cell<GLuint>>,
    jobs: Rc<JobDispatch>,
}

impl Shader {
    /// Compiles an OpenGL shader step from source.
    ///
    /// `unsafe`: The OpenGL context must be active on the current thread.
    unsafe fn compile(&self, source: Vec<u8>) -> Result<(), GenericError> {
        let id = self.try_id()?;

        // Clear any previous OpenGL errors so we know whether any error flags are from initializing
        // this shader.
        let gl: &gl::Gl = self.jobs.gl.as_ref();
        gl.flush_errors();

        // Compile the shader source.
        let source_ptr = source.as_ptr() as *const GLchar;
        let source_len = GLint::try_from(source.len())?;
        gl.ShaderSource(id, 1, &source_ptr, &source_len);
        gl.CompileShader(id);
        let mut status = gl::FALSE as GLint;
        gl.GetShaderiv(id, gl::COMPILE_STATUS, &mut status);

        if status != gl::TRUE as GLint {
            let mut len = 0;
            gl.GetShaderiv(id, gl::INFO_LOG_LENGTH, &mut len);
            let mut buf: Vec<u8> = std::iter::repeat(0).take(usize::try_from(len)? + 1).collect();
            gl.GetShaderInfoLog(id, GLsizei::try_from(buf.len())?, &mut len,
                                buf.as_mut_ptr() as *mut _);
            buf.truncate(usize::try_from(len)?);
            return Err(GenericError::from(format!("shader compilation failed: {}",
                                                  String::from_utf8_lossy(buf.as_ref()))));
        }

        // Check any remaining error flags.
        if let Some(err) = gl.try_get_error() {
            return Err(GenericError::from("shader initialization failed").with_source(err));
        }
        Ok(())
    }

    /// Creates a new OpenGL shader step.
    ///
    /// `unsafe`: The OpenGL context must be active on the current thread.
    unsafe fn new(jobs: &Rc<JobDispatch>, kind: GLenum) -> Result<Shader, GenericError> {
        let gl: &gl::Gl = jobs.gl.as_ref();
        let id = gl.CreateShader(kind);
        if id == 0 {
            return Err(GenericError::from("can't create shader")
                .with_source(gl.get_error()));
        }

        Ok(Shader {
            id: Rc::new(Cell::new(id)),
            jobs: jobs.clone(),
        })
    }

    /// Returns the shader's raw OpenGL name.
    fn try_id(&self) -> Result<GLuint, GenericError> {
        match self.id.get() {
            0 => Err(GenericError::from("shader expired")),
            id => Ok(id),
        }
    }
}

impl Drop for Shader {
    fn drop(&mut self) {
        let id = self.id.clone();
        let _ = self.jobs.dispatch(move |gl| {
            if id.get() != 0 {
                unsafe {
                    gl.DeleteShader(id.get());
                }
                id.set(0);
            }
            Ok(())
        });
    }
}

/// Struct containing all shader steps.
struct Shaders {
    unlit_frag: Rc<Shader>,
    unlit_vert: Rc<Shader>,
}

impl Shaders {
    // Loads and compiles all used shaders.
    unsafe fn load_all(data: &DataLoader, jobs: &Rc<JobDispatch>) -> Result<Shaders, GenericError> {
        debug!("compiling shaders...");

        // Set up asynchronous loading of all shader sources.
        let mut shader_map = HashMap::new();
        let (sender, receiver) = std::sync::mpsc::channel();
        let mut load = |name, kind| -> Result<Rc<Shader>, GenericError> {
            trace!("shader: {}", name);
            let shader = Rc::new(Shader::new(jobs, kind)?);
            let id = shader.try_id()?;
            shader_map.insert(id, shader.clone());
            let full_name = format!("{}{}", SHADER_NAME_PREFIX, name);
            data.read_all(full_name.clone(), ShaderReadHandler {
                id,
                name: full_name,
                sender: sender.clone(),
            }, Some(64 * 1024))?;
            Ok(shader)
        };

        // Define all of the shaders we're using.
        let shaders = Shaders {
            unlit_frag: (load)("unlit.frag", gl::FRAGMENT_SHADER)?,
            unlit_vert: (load)("unlit.vert", gl::VERTEX_SHADER)?,
        };

        // Compile sources as they become available
        while !shader_map.is_empty() {
            match receiver.recv() {
                Ok(ShaderReadResponse::Ok { name, id, source }) => {
                    let shader = match shader_map.remove(&id) {
                        None => panic!("invalid shader id"),
                        Some(shader) => shader,
                    };
                    if let Err(err) = shader.compile(source) {
                        return Err(GenericError::from(format!("{}: {}", name, err)));
                    }
                },
                Ok(ShaderReadResponse::Err { name, err }) => {
                    return Err(GenericError::from(format!("{}: {}", name, err)));
                },
                Err(err) => return Err(GenericError::from(err)),
            }
        }

        Ok(shaders)
    }
}

/// Handles asynchronous loading of shader sources.
struct ShaderReadHandler {
    id: GLuint,
    name: String,
    sender: Sender<ShaderReadResponse>,
}

impl crate::data::ErrorHandler for ShaderReadHandler {
    fn on_error(self: Box<Self>, err: GenericError) {
        let _ = self.sender.send(ShaderReadResponse::Err {
            err,
            name: self.name,
        });
    }
}

impl crate::data::ReadAllHandler for ShaderReadHandler {
    fn on_read_all(self: Box<Self>, buf: Vec<u8>) -> Result<(), GenericError> {
        self.sender.send(ShaderReadResponse::Ok {
            name: self.name,
            id: self.id,
            source: buf,
        })?;
        Ok(())
    }
}

/// Data received when reading a shader's source is completed or fails.
enum ShaderReadResponse {
    Ok { name: String, id: GLuint, source: Vec<u8> },
    Err { name: String, err: GenericError },
}

/// OpenGL shader program wrapper.
pub struct Program {
    id: Rc<Cell<GLuint>>,
    jobs: Rc<JobDispatch>,
    render_state: Rc<RenderState>,
}

impl Program {
    /// Returns the program's raw OpenGL name.
    pub(in crate::render) fn try_id(&self) -> Result<GLuint, GenericError> {
        match self.id.get() {
            0 => Err(GenericError::from("shader program expired")),
            id => Ok(id),
        }
    }
}

impl Program {
    /// Links the shader program.
    ///
    /// `unsafe`: The OpenGL context must be active on the current thread.
    unsafe fn link(&self, shaders: &[&Shader]) -> Result<(), GenericError> {
        let id = self.try_id()?;

        // Clear any previous OpenGL errors so we know whether any error flags are from initializing
        // this program.
        let gl: &gl::Gl = self.jobs.gl.as_ref();
        gl.flush_errors();

        // Link the shader.
        for shader in shaders {
            gl.AttachShader(id, shader.try_id()?);
        }
        gl.LinkProgram(id);
        let mut status = gl::FALSE as GLint;
        gl.GetProgramiv(id, gl::LINK_STATUS, &mut status);

        if status != gl::TRUE as GLint {
            let mut len = 0;
            gl.GetProgramiv(id, gl::INFO_LOG_LENGTH, &mut len);
            let mut buf: Vec<u8> = std::iter::repeat(0).take(usize::try_from(len)? + 1).collect();
            gl.GetProgramInfoLog(id, GLsizei::try_from(buf.len())?, &mut len,
                                 buf.as_mut_ptr() as *mut _);
            buf.truncate(usize::try_from(len)?);
            return Err(GenericError::from(format!("shader program linking failed: {}",
                                                  String::from_utf8_lossy(buf.as_ref()))));
        }

        // Check any remaining error flags.
        if let Some(err) = gl.try_get_error() {
            return Err(GenericError::from("shader program initialization failed").with_source(err));
        }
        Ok(())
    }

    /// Creates a new shader program.
    ///
    /// `unsafe`: The OpenGL context must be active on the current thread.
    unsafe fn new(jobs: &Rc<JobDispatch>, render_state: &Rc<RenderState>)
        -> Result<Program, GenericError>
    {
        let gl: &gl::Gl = jobs.gl.as_ref();
        let id = gl.CreateProgram();
        if id == 0 {
            return Err(GenericError::from("can't create shader program")
                .with_source(gl.get_error()));
        }

        Ok(Program {
            id: Rc::new(Cell::new(id)),
            jobs: jobs.clone(),
            render_state: render_state.clone(),
        })
    }
}

impl Drop for Program {
    fn drop(&mut self) {
        let id = self.id.clone();
        let render_state = self.render_state.clone();
        let _ = self.jobs.dispatch(move |gl| {
            if id.get() != 0 {
                unsafe {
                    if render_state.current_program.get() == id.get() {
                        gl.UseProgram(0);
                    }
                    gl.DeleteProgram(id.replace(0));
                }
            }
            Ok(())
        });
    }
}

/// All shader programs.
pub struct Programs {
    pub _unlit: Program,
}

impl Programs {
    /// Loads all used shader programs.
    ///
    /// `unsafe`: The OpenGL context must be active on the current thread.
    pub(in crate::render) unsafe fn load_all(data: &DataLoader,
                                             jobs: &Rc<JobDispatch>,
                                             render_state: &Rc<RenderState>)
                                             -> Result<Programs, GenericError>
    {
        let s = Shaders::load_all(data, jobs)?;

        debug!("linking shader programs...");

        let link = |name, shaders| {
            trace!("program: {}", name);
            let program = Program::new(jobs, render_state)?;
            if let Err(err) = program.link(shaders) {
                return Err(GenericError::from(format!("shader program '{}': {}", name, err)));
            }
            Ok(program)
        };

        Ok(Programs {
            _unlit: (link)("unlit", &[&s.unlit_vert, &s.unlit_frag])?,
        })
    }
}
