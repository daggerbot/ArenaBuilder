/*
 * Copyright (c) 2022 Martin Mills <daggerbot@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

use std::fs::File;
use std::io::Read;
use std::path::PathBuf;
use std::sync::mpsc::{Receiver, Sender};

use zip::ZipArchive;

use crate::error::GenericError;

/// Trait for responding to asynchronous errors.
pub trait ErrorHandler : 'static + Send {
    fn on_error(self: Box<Self>, err: GenericError);
}

/// Trait for responding to asynchronous read events.
pub trait ReadHandler : ErrorHandler {
    fn on_eof(self: Box<Self>) -> Result<(), GenericError>;
    fn on_read(&mut self, buf: &[u8]) -> Result<(), GenericError>;
}

/// Trait for responding to the reading of an entire file.
pub trait ReadAllHandler : ErrorHandler {
    fn on_read_all(self: Box<Self>, buf: Vec<u8>) -> Result<(), GenericError>;
}

/// Loads data from the game's main .pk3 archive in a worker thread.
pub struct DataLoader {
    request_sender: Sender<Request>,
}

impl DataLoader {
    /// Initializes the data loader and spawns the worker thread.
    pub fn init(args: &crate::Args) -> Result<DataLoader, GenericError> {
        let path = match args.data_path {
            None => DataLoader::get_default_path()?,
            Some(ref p) => p.clone(),
        };

        debug!("loading data from: {}", path.display());

        let file = match File::open(&path) {
            Ok(f) => f,
            Err(err) => {
                return Err(GenericError::from(format!("{}: open failed", path.display()))
                    .with_source(err));
            },
        };
        let zip = match ZipArchive::new(file) {
            Ok(zip) => zip,
            Err(err) => {
                return Err(GenericError::from(format!("{}: open zip failed", path.display()))
                    .with_source(err));
            },
        };
        let (request_sender, request_receiver) = std::sync::mpsc::channel();
        let worker = Worker {
            buf: [0; 1024],
            request_receiver,
            zip,
        };

        std::thread::spawn(move || worker.main());

        Ok(DataLoader {
            request_sender,
        })
    }

    /// Reads the specified data file. Invokes the handler in a loop until reading is finished.
    pub fn read<S, H>(&self, name: S, handler: H) -> Result<(), GenericError>
    where
        S: Into<String>,
        H: ReadHandler,
    {
        let name = name.into();
        let request = Request::Load {
            name: name.clone(),
            handler: Box::new(handler),
        };

        match self.request_sender.send(request) {
            Ok(()) => Ok(()),
            Err(err) => Err(GenericError::from(format!("{}: open request failed", name))
                .with_source(err)),
        }
    }

    /// Reads the specified data file. Invokes the handler when reading is finished.
    pub fn read_all<S, H>(&self, name: S, handler: H, max_len: Option<usize>)
        -> Result<(), GenericError>
    where
        S: Into<String>,
        H: ReadAllHandler,
    {
        self.read(name, ReadAllWrapper {
            data: Vec::new(),
            inner: Box::new(handler),
            max_len,
        })
    }
}

impl DataLoader {
    /// Gets the default data file path if not specified on the command line.
    fn get_default_path() -> Result<PathBuf, GenericError> {
        let exe_path = match std::env::current_exe() {
            Ok(p) => p,
            Err(err) => return Err(GenericError::from("can't get executable path")
                .with_source(err)),
        };
        let exe_dir = match exe_path.parent() {
            None => return Err(GenericError::from(format!("{}: can't get parent of executable path",
                                                          exe_path.display()))),
            Some(p) => p,
        };
        Ok(exe_dir.join(crate::DATA_FILENAME))
    }
}

impl Drop for DataLoader {
    fn drop(&mut self) {
        let _ = self.request_sender.send(Request::Quit);
    }
}

/// Request message sent to the worker thread.
enum Request {
    Load { name: String, handler: Box<dyn ReadHandler> },
    Quit,
}

/// Data handler worker thread data.
struct Worker {
    buf: [u8; 1024],
    request_receiver: Receiver<Request>,
    zip: ZipArchive<File>,
}

impl Worker {
    /// Invokes the read handler in a loop for the specified file.
    fn load(&mut self, name: &str, mut handler: Box<dyn ReadHandler>) {
        let mut reader = match self.zip.by_name(name) {
            Ok(r) => r,
            Err(err) => {
                let msg = format!("{}: open failed", name);
                let err = GenericError::from(msg).with_source(err);
                handler.on_error(err);
                return;
            },
        };

        'read_loop: loop {
            let n = match reader.read(&mut self.buf) {
                Ok(n) => n,
                Err(err) => {
                    let msg = format!("{}: read failed", name);
                    let err = GenericError::from(msg).with_source(err);
                    handler.on_error(err);
                    return;
                },
            };

            if n == 0 {
                break 'read_loop;
            }

            if let Err(err) = handler.on_read(&self.buf[..n]) {
                error!("{}: handler failed: {}", name, err);
                return;
            }
        }

        if let Err(err) = handler.on_eof() {
            error!("{}: handler failed: {}", name, err);
        }
    }

    /// Main loop for the worker thread.
    fn main(mut self) {
        'main_loop: loop {
            let request = match self.request_receiver.recv() {
                Ok(m) => m,
                Err(err) => panic!("data loader failed to receive request: {}", err),
            };

            match request {
                Request::Load { name, handler } => self.load(name.as_str(), handler),
                Request::Quit => break 'main_loop,
            }
        }
    }
}

/// Wrapper for reading the full contents of a data file.
struct ReadAllWrapper {
    data: Vec<u8>,
    inner: Box<dyn ReadAllHandler>,
    max_len: Option<usize>,
}

impl ErrorHandler for ReadAllWrapper {
    fn on_error(self: Box<Self>, err: GenericError) {
        self.inner.on_error(err)
    }
}

impl ReadHandler for ReadAllWrapper {
    fn on_eof(self: Box<Self>) -> Result<(), GenericError> {
        self.inner.on_read_all(self.data)
    }

    fn on_read(&mut self, buf: &[u8]) -> Result<(), GenericError> {
        if let Some(max_len) = self.max_len {
            let max_read = max_len - self.data.len();
            if buf.len() > max_read {
                return Err(GenericError::from("maximum file size exceeded"));
            }
        }
        self.data.extend(buf);
        Ok(())
    }
}
