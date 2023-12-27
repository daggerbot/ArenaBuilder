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

#ifndef ARENABUILDER_CLIENT_CLIENT_H_INCLUDED
#define ARENABUILDER_CLIENT_CLIENT_H_INCLUDED

#include <memory>

#include <Core/Types.h>

union SDL_Event;
struct SDL_WindowEvent;

namespace ArenaBuilder {

    class RenderWindow;

    class Client {
    public:
        Client();
        Client(const Client&) = delete;
        Client(Client&&) = delete;
        ~Client();

        RenderWindow* GetRenderWindow() { return m_renderWindow.get(); }

        void Initialize();
        void Run();
        void ShutDown();

        void Quit() { m_quitRequested = true; }
        bool IsQuitting() const { return m_quitRequested; }

        Client& operator=(const Client&) = delete;
        Client& operator=(Client&&) = delete;

    private:
        std::unique_ptr<RenderWindow> m_renderWindow;

        bool m_quitRequested = false;

        void HandleSdlEvents();
        void HandleSdlEvent(const SDL_Event& event);
        void HandleSdlWindowEvent(const SDL_WindowEvent& event);
    };

} // namespace ArenaBuilder

#endif // ARENABUILDER_CLIENT_CLIENT_H_INCLUDED
