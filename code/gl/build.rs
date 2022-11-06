/*
 * Copyright (c) 2022 Martin Mills <daggerbot@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

extern crate gl_generator;

use std::fs::File;
use std::io::Write;
use std::path::PathBuf;

use gl_generator::{Api, Fallbacks, Profile, Registry, StructGenerator};

const API_VERSION: (u8, u8) = (3, 0);

fn main() {
    let out_dir = PathBuf::from(std::env::var_os("OUT_DIR").expect("missing OUT_DIR"));
    let out_path = out_dir.join("generated.rs");
    let out_file = match File::create(&out_path) {
        Ok(f) => f,
        Err(err) => panic!("can't create '{}': {}", out_path.display(), err),
    };
    write_source(out_file).expect("write failed");
}

fn write_source<W: Write>(mut out: W) -> Result<(), std::io::Error> {
    writeln!(out, "pub const API_VERSION: (u8, u8) = ({}, {});",
             API_VERSION.0, API_VERSION.1)?;

    Registry::new(Api::Gl, API_VERSION, Profile::Compatibility, Fallbacks::None, &[])
        .write_bindings(StructGenerator, &mut out)?;

    out.flush()
}
