/*
 * Copyright (c) 2022 Martin Mills <daggerbot@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

macro_rules! fatal {
    ($($expr:expr),*) => {{
        let msg = format!($($expr),*);
        error!("{}", msg);
        panic!("{}", msg)
    }};
}
