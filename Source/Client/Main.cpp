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

#ifdef _WIN32
# include <windows.h>
#endif

#include <Core/CommandLine.h>
#include <Core/Debug.h>
#include <Core/System.h>

#include "Client.h"

using namespace ArenaBuilder;

namespace {

    int ClientMain(int argc, const oschar_t* const argv[])
    {
        System::InitErrorDialogHandler();
        Debug::InitLogger();

        Client client;

        // Use a scope so we can release params when it's no longer needed.
        {
            auto params = ClientParams::FromCommandLine(argc, argv);
            LOG_INFO("Initializing...");
            client.Initialize(params);
        }

        LOG_INFO("Game started!");
        client.Run();

        LOG_INFO("Shutting down...");
        client.ShutDown();
        return 0;
    }

} // namespace

#ifdef _WIN32

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
    return ClientMain(__argc, __wargv);
}

#else // !defined(_WIN32)

int main(int argc, char* argv[])
{
    return ClientMain(argc, argv);
}

#endif // !defined(_WIN32)
