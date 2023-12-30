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

#ifndef ARENABUILDER_RENDER_GL_LOADER_H_INCLUDED
#define ARENABUILDER_RENDER_GL_LOADER_H_INCLUDED

#include <Core/Types.h>

namespace ArenaBuilder {

    class GlLoader {
    public:
        virtual ~GlLoader() = 0;
        virtual void* GetGlProcAddress(const char* name) = 0;

        GlLoader& operator=(const GlLoader&) = delete;
        GlLoader& operator=(GlLoader&&) = delete;
    };

    inline GlLoader::~GlLoader() = default;

} // namespace ArenaBuilder

#endif // ARENABUILDER_RENDER_GL_LOADER_H_INCLUDED
