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

#include <Core/CommandLine.h>
#include <Core/Debug.h>
#include <Render/System.h>

#include "Client.h"
#include "RenderWindow.h"

using namespace ArenaBuilder;

namespace {

    class ClientCommandLineHandler : public CommandLineHandler {
    public:
        ClientParams clientParams;

        bool HandleOperand(OsStringView operand) override
        {
            FATAL("Unexpected operand: {}", operand);
        }

        bool HandleShortOption(oschar_t option, CommandLineParser&) override
        {
            FATAL("Invalid option: -{}", option);
        }

        bool HandleLongOption(OsStringView option, CommandLineParser& parser) override
        {
            if (option == OSSTR("data-dir")) {
                auto param = parser.GetParam();
                if (!param) {
                    FATAL("Missing parameter for --data-dir");
                }
                clientParams.dataDir = param;
                return true;
            } else {
                FATAL("Invalid option: --{}", option);
            }
        }
    };

} // namespace

ClientParams ClientParams::FromCommandLine(int argc, const oschar_t* const* argv)
{
    ClientCommandLineHandler handler;
    CommandLineParser::Parse(argc, argv, handler);
    return std::move(handler.clientParams);
}

//--------------------------------------------------------------------------------------------------

Client::Client()
{
}

Client::~Client()
{
}

void Client::Initialize(const ClientParams&)
{
    m_renderWindow = std::make_unique<RenderWindow>();
    m_renderSystem = std::make_unique<RenderSystem>(*this);
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
    m_renderSystem.reset();
    m_renderWindow.reset();
}

void* Client::GetService(const std::type_info& type)
{
    if (type == typeid(GlLoader)) {
        return static_cast<GlLoader*>(m_renderWindow.get());
    } else {
        return nullptr;
    }
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
