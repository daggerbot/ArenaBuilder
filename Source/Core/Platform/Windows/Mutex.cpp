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
#include <Core/Mutex.h>
#include <Core/System.h>

using namespace std::literals::string_literals;
using namespace ArenaBuilder;

RecursiveMutex::RecursiveMutex()
{
    std::string error;

    if (!Initialize(Out{error})) {
        System::ExitWithErrorMessage(Encoding::SystemToWide(error).c_str());
    }
}

RecursiveMutex::RecursiveMutex(Out<std::string> outError)
{
    Initialize(outError);
}

RecursiveMutex::~RecursiveMutex()
{
    if (m_handle) {
        CloseHandle(m_handle);
    }
}

void RecursiveMutex::Lock()
{
    std::string error;

    if (!TryLock(Out{error})) {
        System::ExitWithErrorMessage(Encoding::SystemToWide(error).c_str());
    }
}

bool RecursiveMutex::TryLock(OptionalOut<std::string> outError)
{
    switch (WaitForSingleObject(m_handle, INFINITE)) {
    case WAIT_OBJECT_0:
    case WAIT_ABANDONED:
        return true;

    default:
        if (auto error = outError.GetPointer()) {
            uint32_t errorCode = GetLastError();
            *error = "WaitForSingleObject: "s + Win32::GetErrorStringA(errorCode);
        }
        return false;
    }
}

void RecursiveMutex::Unlock()
{
    ReleaseMutex(m_handle);
}

bool RecursiveMutex::Initialize(Out<std::string> outError)
{
    m_handle = CreateMutexW(nullptr, false, nullptr);

    if (!m_handle) {
        uint32_t errorCode = GetLastError();
        *outError = "CreateMutexW: "s + Win32::GetErrorStringA(errorCode);
        return false;
    }

    return true;
}
