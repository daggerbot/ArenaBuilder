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

#include <SDL_events.h>

#include "Client.h"
#include "RenderWindow.h"

using namespace ArenaBuilder;

Client::Client()
{
}

Client::~Client()
{
}

void Client::Initialize()
{
    m_renderWindow = std::make_unique<RenderWindow>();
}

void Client::Run()
{
    while (!IsQuitting()) {
        HandleSdlEvents();
        if (IsQuitting()) {
            break;
        }

        m_renderWindow->SwapBuffers();
    }
}

void Client::ShutDown()
{
    m_renderWindow.reset();
}

void Client::HandleSdlEvents()
{
    SDL_Event event;

    while (!IsQuitting() && SDL_PollEvent(&event)) {
        HandleSdlEvent(event);
    }
}

void Client::HandleSdlEvent(const SDL_Event& event)
{
    switch (event.type) {
    case SDL_QUIT:
        Quit();
        break;

    case SDL_WINDOWEVENT:
        HandleSdlWindowEvent(event.window);
        break;

    default:
        break;
    }
}

void Client::HandleSdlWindowEvent(const SDL_WindowEvent& event)
{
    switch (event.event) {
    case SDL_WINDOWEVENT_CLOSE:
        Quit();
        break;

    default:
        break;
    }
}
