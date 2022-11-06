/*
 * Copyright (c) 2022 Martin Mills <daggerbot@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

use std::rc::Rc;
use std::time::Instant;

use sdl2::event::{Event, WindowEvent};

use crate::data::DataLoader;
use crate::error::GenericError;
use crate::render::{Render, RenderSystem};

/// Contains most of the game's subsystems and global state.
pub struct System {
    data: Rc<DataLoader>,
    render: RenderSystem,
    sdl: sdl2::Sdl,
}

impl System {
    /// Returns the main data loader.
    pub fn data(&self) -> &DataLoader { &self.data }

    /// Initializes the game's subsystems.
    pub fn init(args: &crate::Args) -> Result<System, GenericError> {
        debug!("initializing...");

        let data = Rc::new(DataLoader::init(args)?);
        let sdl = sdl2::init()?;
        let render = RenderSystem::init(data.as_ref())?;

        Ok(System {
            data,
            render,
            sdl,
        })
    }

    /// Executes the main loop and consumes the system.
    pub fn run(mut self, mut state: Box<dyn 'static + State>) -> Result<(), GenericError> {
        debug!("game started!");

        let mut event_pump = self.sdl.event_pump()?;
        let mut prev_instant = Instant::now();

        'main_loop: loop {
            // Handle all pending SDL events.
            while let Some(event) = event_pump.poll_event() {
                match self.handle_event(event, state.as_mut()) {
                    UpdateResult::Ok => (),
                    UpdateResult::ChangeState(new_state) => state = new_state,
                    UpdateResult::Quit => break 'main_loop,
                    UpdateResult::Err(err) => return Err(err),
                }
            }

            // Update game time.
            let instant = Instant::now();
            let mut delta_ms = (instant - prev_instant).as_millis() as u32;
            prev_instant = instant;

            // Update game state.
            'update_loop: loop {
                match state.update(&mut self, delta_ms) {
                    UpdateResult::Ok => break 'update_loop,
                    UpdateResult::ChangeState(new_state) => {
                        state = new_state;
                        delta_ms = 0;
                    },
                    UpdateResult::Quit => break 'main_loop,
                    UpdateResult::Err(err) => return Err(err),
                }
            }

            // Render the current frame.
            if let Err(err) = self.render.render(|render| state.render(render, delta_ms)) {
                return Err(GenericError::from("rendering failed").with_source(err));
            }
            if let Err(err) = self.render.window().swap_buffers() {
                return Err(GenericError::from("failed to swap buffers").with_source(err));
            }
        }

        debug!("shutting down...");
        state.on_quit(&mut self);
        Ok(())
    }
}

impl System {
    /// Handles an SDL event.
    fn handle_event(&mut self, event: Event, _state: &mut dyn State) -> UpdateResult {
        match event {
            Event::Quit { .. } => UpdateResult::Quit,
            Event::Window { win_event, .. } => match win_event {
                WindowEvent::Close => UpdateResult::Quit,
                _ => UpdateResult::Ok,
            },
            _ => UpdateResult::Ok,
        }
    }
}

/// Main loop event dispatcher.
pub trait State {
    /// Called when the game is about to quit.
    fn on_quit(&mut self, _system: &mut System) {}

    /// Renders the current frame.
    fn render(&mut self, render: Render, delta_ms: u32) -> Result<(), GenericError>;

    /// Called once per frame to update the game state.
    fn update(&mut self, system: &mut System, delta_ms: u32) -> UpdateResult;
}

/// Result type for game state updates.
pub enum UpdateResult {
    Ok,
    ChangeState(Box<dyn 'static + State>),
    Quit,
    Err(GenericError),
}
