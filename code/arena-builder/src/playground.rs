/*
 * Copyright (c) 2022 Martin Mills <daggerbot@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

use rgb::RGB;

use crate::error::GenericError;
use crate::render::Render;
use crate::system::{State, System, UpdateResult};

pub struct Playground;

impl State for Playground {
    fn render(&mut self, mut render: Render, _delta_ms: u32) -> Result<(), GenericError> {
        render.clear(RGB::new(0.0, 0.25, 0.5));
        Ok(())
    }

    fn update(&mut self, _system: &mut System, _delta_ms: u32) -> UpdateResult {
        UpdateResult::Ok
    }
}
