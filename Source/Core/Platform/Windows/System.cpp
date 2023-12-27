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

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include <memory>

#include <Core/System.h>

using namespace std::literals::string_literals;
using namespace ArenaBuilder;

WEAK void System::ExitWithErrorMessage(const oschar_t* message)
{
    fputws(message, stderr);
    fflush(stderr);
    ExitProcess(EXIT_FAILURE);
}

WEAK void System::ExitWithErrorDialog(const oschar_t*)
{
    ExitProcess(EXIT_FAILURE);
}

//--------------------------------------------------------------------------------------------------

std::string Win32::GetErrorStringA(uint32_t errorCode)
{
    char* buffer = nullptr;

    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                   nullptr, errorCode, 0, reinterpret_cast<LPSTR>(&buffer), 0, nullptr);

    if (!buffer) {
        return "Win32 error code "s + std::to_string(errorCode);
    }

    Finally _freeBuffer{[&buffer]() { LocalFree(buffer); }};
    return buffer;
}

std::wstring Win32::GetErrorStringW(uint32_t errorCode)
{
    wchar_t* buffer = nullptr;

    FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                   nullptr, errorCode, 0, reinterpret_cast<LPWSTR>(&buffer), 0, nullptr);

    if (!buffer) {
        return L"Win32 error code "s + std::to_wstring(errorCode);
    }

    Finally _freeBuffer{[&buffer]() { LocalFree(buffer); }};
    return buffer;
}
