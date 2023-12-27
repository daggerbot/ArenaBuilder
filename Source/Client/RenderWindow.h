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

#ifndef ARENABUILDER_CLIENT_RENDERWINDOW_H_INCLUDED
#define ARENABUILDER_CLIENT_RENDERWINDOW_H_INCLUDED

#include <memory>

#include <Core/Math/Vec.h>

struct SDL_Window;

namespace ArenaBuilder {

    // SDL window wrapper.
    class RenderWindow {
    public:
        RenderWindow();
        RenderWindow(const RenderWindow&) = delete;
        RenderWindow(RenderWindow&&) = delete;
        ~RenderWindow();

        Vec2i GetClientSize() const;

        void SwapBuffers();

        RenderWindow& operator=(const RenderWindow&) = delete;
        RenderWindow& operator=(RenderWindow&&) = delete;

    private:
        std::unique_ptr<SDL_Window, void(*)(SDL_Window*)> m_sdlWindow;
        std::unique_ptr<void, void(*)(void*)> m_glContext;
    };

} // namespace ArenaBuilder

#endif // ARENABUILDER_CLIENT_RENDERWINDOW_H_INCLUDED
