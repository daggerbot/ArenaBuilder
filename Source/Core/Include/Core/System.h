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

#ifndef ARENABUILDER_CORE_SYSTEM_H_INCLUDED
#define ARENABUILDER_CORE_SYSTEM_H_INCLUDED

#include "Types.h"

namespace ArenaBuilder {

    namespace System {

        // Shows an error message in a way that makes sense on the current platform, then exits with
        // an error code.
        [[noreturn]] void ExitWithErrorMessage(const oschar_t* message);

        // Shows an error message dialog if supported, or ignores the message if unsupported, then
        // exits with an error code.
        [[noreturn]] void ExitWithErrorDialog(const oschar_t* message);

        // Changes the error dialog handler used by ExitWithErrorMessage() and
        // ExitWithErrorDialog(). May be a no-op depending on the platform.
        void InitErrorDialogHandler();
        void SetErrorDialogHandler(void (*handler)(const oschar_t*));

    } // namespace System

#ifdef _WIN32
    namespace Win32 {

        std::string GetErrorStringA(uint32_t errorCode);
        std::wstring GetErrorStringW(uint32_t errorCode);

    } // namespace Win32
#endif // defined(_WIN32)

} // namespace ArenaBuilder

#endif // ARENABUILDER_CORE_SYSTEM_H_INCLUDED
