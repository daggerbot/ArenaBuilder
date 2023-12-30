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

#include <SDL_video.h>

#include <Core/Debug.h>
#include <Core/GameDefs.h>
#include <Render/GL/Version.h>

#include "RenderWindow.h"

using namespace ArenaBuilder;

namespace {

    constexpr Vec2i DefaultWindowSize = {640, 480};

} // namespace

RenderWindow::RenderWindow()
    : m_sdlWindow{nullptr, &SDL_DestroyWindow}
    , m_glContext{nullptr, &SDL_GL_DeleteContext}
{
    SDL_GL_ResetAttributes();
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, SDL_TRUE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, RENDER_GL_MAJOR_VERSION);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, RENDER_GL_MINOR_VERSION);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    m_sdlWindow.reset(SDL_CreateWindow(
        GAME_TITLE,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        DefaultWindowSize.x,
        DefaultWindowSize.y,
        SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN));

    if (!m_sdlWindow) {
        FATAL("Can't create SDL window: {}", SDL_GetError());
    }

    m_glContext.reset(SDL_GL_CreateContext(m_sdlWindow.get()));
    if (!m_glContext) {
        FATAL("Can't create OpenGL context: {}", SDL_GetError());
    }

    // Enable vsync. Eventually we'll make this configurable.
    if (!SDL_GL_SetSwapInterval(-1)) {
        LOG_DEBUG("Enabled adaptive vsync");
    } else if (!SDL_GL_SetSwapInterval(1)) {
        LOG_DEBUG("Enabled vsync");
    } else {
        LOG_WARNING("Can't enable vsync: {}", SDL_GetError());
    }
}

RenderWindow::~RenderWindow()
{
}

Vec2i RenderWindow::GetClientSize() const
{
    Vec2i size{0, 0};
    SDL_GetWindowSize(m_sdlWindow.get(), &size.x, &size.y);
    return size;
}

void* RenderWindow::GetGlProcAddress(const char* name)
{
    return SDL_GL_GetProcAddress(name);
}

void RenderWindow::SwapBuffers()
{
    SDL_GL_SwapWindow(m_sdlWindow.get());
}
