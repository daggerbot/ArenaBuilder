/*
 * Copyright (c) 2023 Martin Mills <daggerbot@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License
 * version 2.0 (the "License"). If a copy of the License was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#ifndef ARENABUILDER_CORE_MATH_VEC_H_INCLUDED
#define ARENABUILDER_CORE_MATH_VEC_H_INCLUDED

#include "../Types.h"

namespace ArenaBuilder {

    template<typename T>
    struct Vec2 {
        T x, y;
    };

    using Vec2i = Vec2<int32_t>;
    using Vec2f = Vec2<float>;
    using Vec2d = Vec2<double>;

    //----------------------------------------------------------------------------------------------

    template<typename T>
    struct Vec3 {
        T x, y, z;
    };

    using Vec3i = Vec3<int32_t>;
    using Vec3f = Vec3<float>;
    using Vec3d = Vec3<double>;

    //----------------------------------------------------------------------------------------------

    template<typename T>
    struct Vec4 {
        T x, y, z, w;
    };

    using Vec4i = Vec4<int32_t>;
    using Vec4f = Vec4<float>;
    using Vec4d = Vec4<double>;

} // namespace ArenaBuilder

#endif // ARENABUILDER_CORE_MATH_VEC_H_INCLUDED
