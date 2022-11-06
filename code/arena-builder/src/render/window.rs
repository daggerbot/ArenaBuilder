/*
 * Copyright (c) 2022 Martin Mills <daggerbot@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

use cgmath::Vector2;

use crate::error::GenericError;

/// Window surface where the game scene is rendered.
pub struct Window {
    sdl_window: sdl2::video::Window,
}

impl Window {
    /// Returns the size of the window's client area in pixels.
    pub fn size(&self) -> Vector2<u32> {
        self.sdl_window.size().into()
    }

    /// Swaps the front and back buffers.
    pub fn swap_buffers(&self) -> Result<(), GenericError> {
        self.sdl_window.gl_swap_window();
        Ok(())
    }
}

impl Window {
    /// Creates a new render window.
    pub(in crate::render) fn new() -> Result<Window, GenericError> {
        let sdl = sdl2::init()?;
        let sdl_video = sdl.video()?;
        let gl_attr = sdl_video.gl_attr();

        gl_attr.set_buffer_size(24);
        gl_attr.set_red_size(8);
        gl_attr.set_green_size(8);
        gl_attr.set_blue_size(8);
        gl_attr.set_alpha_size(0);
        gl_attr.set_depth_size(16);
        gl_attr.set_stencil_size(0);
        gl_attr.set_double_buffer(true);

        let sdl_window = sdl_video.window(crate::GAME_TITLE,
                                          Window::DEFAULT_SIZE.x,
                                          Window::DEFAULT_SIZE.y)
            .allow_highdpi()
            .opengl()
            .resizable()
            .build()?;

        Ok(Window {
            sdl_window,
        })
    }

    /// Returns the underlying SDL window.
    pub(in crate::render) fn sdl_window(&self) -> &sdl2::video::Window { &self.sdl_window }
}

impl Window {
    const DEFAULT_SIZE: Vector2<u32> = Vector2::new(640, 480);
}
