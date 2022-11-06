/*
 * Copyright (c) 2022 Martin Mills <daggerbot@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

mod shaders;
mod window;

pub use self::window::Window;

use std::cell::{Cell, RefCell};
use std::collections::VecDeque;
use std::error::Error;
use std::ffi::CStr;
use std::fmt::{Display, Formatter};
use std::rc::Rc;
use std::str::FromStr;
use std::sync::atomic::{AtomicBool, Ordering};

use cgmath::Vector2;
use gl::types::*;
use regex::Regex;
use rgb::RGB;

use crate::data::DataLoader;
use crate::error::GenericError;
use crate::render::shaders::Programs;

static SYSTEM_LOCKED: AtomicBool = AtomicBool::new(false);

/// Rendering subsystem.
pub struct RenderSystem {
    gl: Rc<gl::Gl>,
    gl_context: sdl2::video::GLContext,
    jobs: Rc<JobDispatch>,
    mode: Rc<Cell<RenderMode>>,
    _programs: Programs,
    _state: Rc<RenderState>,
    window: Window,
}

impl RenderSystem {
    /// Initializes the rendering subsystem.
    pub fn init(data: &DataLoader) -> Result<RenderSystem, GenericError> {
        if SYSTEM_LOCKED.swap(true, Ordering::Acquire) {
            return Err(GenericError::from("rendering system locked"));
        }

        let window = Window::new()?;
        let sdl_video = window.sdl_window().subsystem();
        let gl_context = window.sdl_window().gl_create_context()?;

        // Try to enable vsync.
        match sdl_video.gl_set_swap_interval(-1) {
            Ok(()) => debug!("enabled adaptive vsync"),
            Err(_) => match sdl_video.gl_set_swap_interval(1) {
                Ok(()) => debug!("enabled vsync"),
                Err(err) => warn!("can't enable vsync: {}", err),
            },
        }

        // Load OpenGL API.
        let mut load_error: Option<GenericError> = None;
        let gl = Rc::new(gl::Gl::load_with(|name| {
            let ptr = sdl_video.gl_get_proc_address(name);
            if ptr.is_null() && load_error.is_none() {
                load_error = Some(GenericError::from(format!("missing OpenGL symbol: {}", name)));
            }
            ptr as *mut _
        }));

        if let Some(err) = load_error {
            return Err(err);
        }

        RenderSystem::check_gl_version(gl.as_ref())?;

        // Initialize utilities..
        let mode = Rc::new(Cell::new(RenderMode::Idle));
        let jobs = Rc::new(JobDispatch {
            gl: gl.clone(),
            mode: mode.clone(),
            queue: RefCell::new(VecDeque::new()),
        });
        let state = Rc::new(RenderState {
            current_program: Cell::new(0),
        });

        // Load shader programs.
        let programs;

        unsafe {
            programs = Programs::load_all(data, &jobs, &state)?;
        }

        // Construct render system struct.
        let system = RenderSystem {
            gl: gl.clone(),
            gl_context,
            jobs,
            mode: mode.clone(),
            _programs: programs,
            _state: state,
            window,
        };

        // Success!
        Ok(system)
    }

    /// Invokes a callback with a rendering context.
    pub fn render<'a, F>(&'a mut self, f: F) -> Result<(), GenericError>
    where
        F: FnOnce(Render<'a>) -> Result<(), GenericError>,
    {
        if self.mode.get() == RenderMode::Rendering {
            return Err(GenericError::from("recursive rendering"));
        }

        self.window.sdl_window().gl_make_current(&self.gl_context)?;
        self.mode.set(RenderMode::Rendering);

        // Execute all pending jobs.
        while let Some(job) = self.jobs.queue.borrow_mut().pop_front() {
            if let Err(err) = (job)(self.gl.as_ref()) {
                error!("render job failed: {}", err);
            }
        }

        let render = Render {
            gl: self.gl.clone(),
            system: self,
        };

        f(render)
    }

    /// Returns the render window.
    pub fn window(&self) -> &Window { &self.window }
}

impl RenderSystem {
    /// Makes sure we're using a compatible OpenGL version.
    fn check_gl_version(gl: &gl::Gl) -> Result<(), GenericError> {
        let version_str;
        let exts_str;

        unsafe {
            let version_ptr = gl.GetString(gl::VERSION);
            if version_ptr.is_null() {
                return Err(GenericError::from("missing GL_VERSION string"));
            }
            let version_c_str = CStr::from_ptr(version_ptr as *const _);
            version_str = String::from_utf8_lossy(version_c_str.to_bytes()).into_owned();

            let exts_ptr = gl.GetString(gl::EXTENSIONS);
            if exts_ptr.is_null() {
                exts_str = String::new();
            } else {
                let exts_c_str = CStr::from_ptr(exts_ptr as *const _);
                exts_str = String::from_utf8_lossy(exts_c_str.to_bytes()).into_owned();
            }
        }

        debug!("OpenGL version: {}", version_str);
        trace!("OpenGL extensions: {}", exts_str);

        let regex = Regex::new("^([0-9]+)\\.([0-9]+)")?;
        let captures = match regex.captures(version_str.as_str()) {
            None => return Err(GenericError::from("can't parse GL_VERSION string")),
            Some(c) => c,
        };
        let major = u8::from_str(&captures[1])?;
        let minor = u8::from_str(&captures[2])?;

        if major < gl::API_VERSION.0 || (major == gl::API_VERSION.0 && minor < gl::API_VERSION.1) {
            return Err(GenericError::from(format!("unsupported OpenGL version: {}.{}",
                                                  major, minor)));
        }

        if major > gl::API_VERSION.0 {
            let mut compatibility = false;
            for ext in exts_str.split_ascii_whitespace() {
                if ext == "GL_ARB_compatibility" {
                    compatibility = true;
                    break;
                }
            }
            if !compatibility {
                warn!("possibly unsupported OpenGL core profile: {}.{}", major, minor);
            }
        }

        Ok(())
    }
}

impl Drop for RenderSystem {
    fn drop(&mut self) {
        self.mode.set(RenderMode::Dead);
    }
}

/// Rendering context.
pub struct Render<'a> {
    gl: Rc<gl::Gl>,
    system: &'a mut RenderSystem,
}

impl<'a> Render<'a> {
    /// Clears the screen to the specified color.
    pub fn clear(&mut self, color: RGB<f32>) {
        unsafe {
            self.gl.ClearColor(color.r, color.g, color.b, 0.0);
            self.gl.Clear(gl::COLOR_BUFFER_BIT);
        }
    }

    /// Returns the size of the rendering surface in pixels.
    pub fn surface_size(&self) -> Vector2<u32> {
        self.system.window().size()
    }
}

impl<'a> Drop for Render<'a> {
    fn drop(&mut self) {
        unsafe {
            self.gl.flush_errors();
        }

        // Unlock the rendering flag so rendering can once again occur.
        self.system.mode.set(RenderMode::Idle);
    }
}

/// State of the render system.
#[derive(Clone, Copy, Eq, PartialEq)]
enum RenderMode {
    Idle,
    Rendering,
    Dead,
}

/// Extensions for `Gl`.
trait GlExt {
    unsafe fn flush_errors(&self);
    unsafe fn get_error(&self) -> GlError;
    unsafe fn try_get_error(&self) -> Option<GlError>;
}

impl GlExt for gl::Gl {
    unsafe fn flush_errors(&self) {
        const MAX_ERRORS: u32 = 100;
        let mut n_errors = 0;

        while let Some(err) = self.try_get_error() {
            error!("{}", err);
            n_errors += 1;
            if n_errors == MAX_ERRORS {
                panic!("too many OpenGL errors");
            }
        }
    }

    unsafe fn get_error(&self) -> GlError {
        match self.try_get_error() {
            None => GlError(gl::NO_ERROR),
            Some(err) => err,
        }
    }

    unsafe fn try_get_error(&self) -> Option<GlError> {
        match self.GetError() {
            gl::NO_ERROR => None,
            error_code => Some(GlError(error_code)),
        }
    }
}

/// OpenGL error code type.
#[derive(Clone, Copy, Debug, Eq, Hash, PartialEq)]
struct GlError(pub GLenum);

impl GlError {
    fn as_str(self) -> Option<&'static str> {
        match self.0 {
            gl::INVALID_ENUM => Some("GL_INVALID_ENUM"),
            gl::INVALID_FRAMEBUFFER_OPERATION => Some("GL_INVALID_FRAMEBUFFER_OPERATION"),
            gl::INVALID_OPERATION => Some("GL_INVALID_OPERATION"),
            gl::INVALID_VALUE => Some("GL_INVALID_VALUE"),
            gl::OUT_OF_MEMORY => Some("GL_OUT_OF_MEMORY"),
            gl::STACK_OVERFLOW => Some("GL_STACK_OVERFLOW"),
            gl::STACK_UNDERFLOW => Some("GL_STACK_UNDERFLOW"),
            _ => None,
        }
    }
}

impl Display for GlError {
    fn fmt(&self, fmt: &mut Formatter) -> std::fmt::Result {
        match self.0 {
            gl::NO_ERROR => fmt.write_str("GL_NO_ERROR"),
            error_code => match self.as_str() {
                None => write!(fmt, "OpenGL error code {}", error_code),
                Some(s) => fmt.write_str(s),
            },
        }
    }
}

impl Error for GlError {
    fn description(&self) -> &str {
        match self.as_str() {
            None => "OpenGL error",
            Some(s) => s,
        }
    }
}

/// Renderer job dispatcher which runs jobs at a time determined by the mode of the renderer.
///
/// If the renderer is currently rendering, jobs are executed immediately. Otherwise, jobs are
/// executed the next time the renderer is active. If the render system is destroyed, jobs are
/// discarded.
struct JobDispatch {
    gl: Rc<gl::Gl>,
    mode: Rc<Cell<RenderMode>>,
    queue: RefCell<VecDeque<Box<dyn 'static + FnOnce(&gl::Gl) -> Result<(), GenericError>>>>,
}

impl JobDispatch {
    /// Pushes or executes the specified job.
    fn dispatch<F>(&self, f: F) -> Result<(), GenericError>
    where
        F: 'static + FnOnce(&gl::Gl) -> Result<(), GenericError>,
    {
        match self.mode.get() {
            RenderMode::Idle => {
                self.queue.borrow_mut().push_back(Box::new(f));
                Ok(())
            },
            RenderMode::Rendering => f(self.gl.as_ref()),
            RenderMode::Dead => Ok(()),
        }
    }
}

/// Current state of the renderer.
struct RenderState {
    current_program: Cell<GLuint>,
}
