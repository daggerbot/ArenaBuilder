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

#include <windows.h>

#include <Core/Encoding.h>
#include <Core/System.h>

using namespace std::literals::string_literals;
using namespace ArenaBuilder;

std::wstring Encoding::SystemToWide(std::string_view inStr)
{
    std::wstring outStr;
    int result;

    if (inStr.empty()) {
        return outStr;
    }

    // Pass 1: Determine the length of the converted string so we can allocate a buffer.
    result = MultiByteToWideChar(CP_ACP, 0, inStr.data(), int(inStr.length()), nullptr, 0);

    if (result <= 0) {
        if (uint32_t errorCode = GetLastError()) {
            System::ExitWithErrorMessage((L"MultiByteToWideChar: "s + Win32::GetErrorStringW(errorCode)).c_str());
        }
        return outStr;
    }

    outStr.resize(size_t(result));

    // Pass 2: Convert the string into the allocated buffer.
    result = MultiByteToWideChar(CP_ACP, 0, inStr.data(), int(inStr.length()), outStr.data(), int(outStr.length()));

    if (result <= 0) {
        if (uint32_t errorCode = GetLastError()) {
            System::ExitWithErrorMessage((L"MultiByteToWideChar: "s + Win32::GetErrorStringW(errorCode)).c_str());
        }
        result = 0;
    }

    outStr.resize(size_t(result));
    return outStr;
}
