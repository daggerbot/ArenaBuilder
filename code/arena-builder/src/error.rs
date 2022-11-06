/*
 * Copyright (c) 2022 Martin Mills <daggerbot@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

use std::borrow::Cow;
use std::error::Error;
use std::fmt::{Display, Formatter};

/// Generic error type with only a message and optionally a source.
///
/// Errors are typically constructed with `From<&'static str>` or `From<String>`.
#[derive(Debug)]
pub struct GenericError {
    message: Cow<'static, str>,
    source: Option<Box<dyn 'static + Error + Send>>,
}

impl GenericError {
    /// Changes the error's source.
    pub fn with_source<E: 'static + Error + Send>(self, source: E) -> GenericError {
        GenericError {
            source: Some(Box::new(source)),
            ..self
        }
    }
}

impl Display for GenericError {
    fn fmt(&self, fmt: &mut Formatter) -> std::fmt::Result {
        fmt.write_str(self.message.as_ref())?;
        if let Some(ref source) = self.source {
            write!(fmt, ": {}", source)?;
        }
        Ok(())
    }
}

impl Error for GenericError {
    fn description(&self) -> &str {
        self.message.as_ref()
    }

    fn source(&self) -> Option<&(dyn 'static + Error)> {
        match self.source {
            None => None,
            Some(ref source) => Some(source.as_ref()),
        }
    }
}

impl From<&'static str> for GenericError {
    fn from(message: &'static str) -> GenericError {
        GenericError {
            message: Cow::Borrowed(message),
            source: None,
        }
    }
}

impl From<String> for GenericError {
    fn from(message: String) -> GenericError {
        GenericError {
            message: Cow::Owned(message),
            source: None,
        }
    }
}

impl From<std::io::Error> for GenericError {
    fn from(source: std::io::Error) -> GenericError {
        GenericError::from("i/o error").with_source(source)
    }
}

impl From<std::num::ParseIntError> for GenericError {
    fn from(source: std::num::ParseIntError) -> GenericError {
        GenericError::from("can't parse integer").with_source(source)
    }
}

impl From<std::num::TryFromIntError> for GenericError {
    fn from(source: std::num::TryFromIntError) -> GenericError {
        GenericError::from("can't convert integer").with_source(source)
    }
}

impl From<std::sync::mpsc::RecvError> for GenericError {
    fn from(source: std::sync::mpsc::RecvError) -> GenericError {
        GenericError::from("mpsc receive failed").with_source(source)
    }
}

impl<T: 'static + Send> From<std::sync::mpsc::SendError<T>> for GenericError {
    fn from(source: std::sync::mpsc::SendError<T>) -> GenericError {
        GenericError::from("mpsc send failed").with_source(source)
    }
}

impl From<regex::Error> for GenericError {
    fn from(source: regex::Error) -> GenericError {
        GenericError::from("regex error").with_source(source)
    }
}

impl From<sdl2::video::WindowBuildError> for GenericError {
    fn from(source: sdl2::video::WindowBuildError) -> GenericError {
        GenericError::from("can't create SDL window").with_source(source)
    }
}
