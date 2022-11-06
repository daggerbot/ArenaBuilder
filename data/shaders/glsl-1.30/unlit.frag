/*
 * Copyright (c) 2022 Martin Mills <daggerbot@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#version 130

uniform sampler2D u_Texture;

in vec2 v_TexCoord;
in vec4 v_Color;

void main() {
    gl_FragColor = texture2D(u_Texture, v_TexCoord) * v_Color;
}
