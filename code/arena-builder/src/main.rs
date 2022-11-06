/*
 * Copyright (c) 2022 Martin Mills <daggerbot@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

extern crate cgmath;
#[macro_use]
extern crate clap;
extern crate gl;
#[macro_use]
extern crate log;
extern crate regex;
extern crate rgb;
extern crate sdl2;
extern crate simple_logger;
extern crate zip;

#[macro_use]
pub mod macros;

pub mod data;
pub mod error;
pub mod playground;
pub mod render;
pub mod system;

pub const DATA_FILENAME: &'static str = "arena-builder.pk3";
pub const GAME_ID: &'static str = "arena-builder";
pub const GAME_TITLE: &'static str = "ArenaBuilder";

/// Parsed command line arguments.
#[derive(Parser)]
pub struct Args {
    #[arg(long)]
    pub data_path: Option<std::path::PathBuf>,
}

fn main() {
    let log_level = match std::env::var_os("TRACE") {
        None => log::Level::Debug,
        Some(_) => log::Level::Trace,
    };
    if let Err(err) = simple_logger::init_with_level(log_level) {
        fatal!("can't initialize logger: {}", err);
    }
    let args = match <Args as clap::Parser>::try_parse() {
        Ok(args) => args,
        Err(err) => fatal!("command line error: {}", err),
    };
    let system = match crate::system::System::init(&args) {
        Ok(system) => system,
        Err(err) => fatal!("initialization failed: {}", err),
    };
    let state = crate::playground::Playground;
    if let Err(err) = system.run(Box::new(state)) {
        fatal!("runtime error: {}", err);
    }
}
