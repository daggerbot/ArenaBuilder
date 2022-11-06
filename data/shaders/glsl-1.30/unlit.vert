/*
 * Copyright (c) 2022 Martin Mills <daggerbot@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#version 130

uniform mat4 u_Transform;

in vec3 a_Position;
in vec2 a_TexCoord;
in vec4 a_Color;

out vec2 v_TexCoord;
out vec4 v_Color;

void main() {
    gl_Position = u_Transform * vec4(a_Position, 1.0);
    v_TexCoord = a_TexCoord;
    v_Color = a_Color;
}
