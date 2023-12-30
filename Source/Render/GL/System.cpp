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

#include <cstdio>

#include <glad/gl.h>

#include <Core/Debug.h>
#include <Render/GL/Loader.h>
#include <Render/GL/Version.h>
#include <Render/System.h>

using namespace ArenaBuilder;

namespace {

    GLADapiproc RequireGlProcAddress(void* userData, const char* name)
    {
        static_assert(sizeof(void*) == sizeof(GLADapiproc));

        GlLoader* loader = reinterpret_cast<GlLoader*>(userData);
        void* proc = loader->GetGlProcAddress(name);

        if (!proc) {
            FATAL("Missing OpenGL symbol: {}", name);
        }

        return reinterpret_cast<GLADapiproc>(proc);
    }

    void CheckGlVersion(GlLoader& loader)
    {
        const char* version;
        int major = 0, minor = 0;

        if (!glad_glGetString) {
            glad_glGetString = reinterpret_cast<PFNGLGETSTRINGPROC>(RequireGlProcAddress(&loader, "glGetString"));
        }

        version = reinterpret_cast<const char*>(glad_glGetString(GL_VERSION));
        if (!version) {
            FATAL("Missing GL_VERSION string");
        }

        if (!std::sscanf(version, "%d.%d", &major, &minor)) {
            FATAL("Can't parse GL_VERSION: {}", version);
        }

        if (major < RENDER_GL_MAJOR_VERSION || (major == RENDER_GL_MAJOR_VERSION && minor < RENDER_GL_MINOR_VERSION)) {
            FATAL("Unsupported GL_VERSION (need at least {}.{}): {}",
                  RENDER_GL_MAJOR_VERSION, RENDER_GL_MINOR_VERSION, version);
        }

        LOG_INFO("OpenGL version: {}", version);
    }

    void LoadGlApi(GlLoader& loader)
    {
        if (!gladLoadGLUserPtr(&RequireGlProcAddress, &loader)) {
            FATAL("Failed to load OpenGL API");
        }
    }

} // namespace

RenderSystem::RenderSystem(ServiceProvider& serviceProvider)
{
    auto& loader = serviceProvider.RequireService<GlLoader>();

    CheckGlVersion(loader);
    LoadGlApi(loader);
}

RenderSystem::~RenderSystem()
{
}
