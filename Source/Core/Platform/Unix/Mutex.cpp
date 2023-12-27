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

#include <string.h>

#include <Core/Mutex.h>
#include <Core/System.h>

using namespace std::literals::string_literals;
using namespace ArenaBuilder;

RecursiveMutex::RecursiveMutex()
{
    std::string error;

    if (!Initialize(Out{error})) {
        System::ExitWithErrorMessage(("Failed to initialize mutex: "s + error).c_str());
    }
}

RecursiveMutex::RecursiveMutex(Out<std::string> outError)
{
    Initialize(outError);
}

RecursiveMutex::~RecursiveMutex()
{
    if (m_wasInit) {
        pthread_mutex_destroy(&m_mutex);
    }
}

void RecursiveMutex::Lock()
{
    std::string error;

    if (!TryLock(Out{error})) {
        System::ExitWithErrorMessage(error.c_str());
    }
}

bool RecursiveMutex::TryLock(OptionalOut<std::string> outError)
{
    if (!m_wasInit) {
        outError.SetValue("Mutex not initialized");
        return false;
    }

    if (auto errorCode = pthread_mutex_lock(&m_mutex)) {
        if (auto error = outError.GetPointer()) {
            *error = "pthread_mutex_lock: "s + strerror(errorCode);
        }
        return false;
    }

    return true;
}

void RecursiveMutex::Unlock()
{
    if (m_wasInit) {
        pthread_mutex_unlock(&m_mutex);
    }
}

bool RecursiveMutex::Initialize(Out<std::string> outError)
{
    pthread_mutexattr_t attributes;

    if (auto errorCode = pthread_mutexattr_init(&attributes)) {
        *outError = "pthread_mutexattr_init: "s + strerror(errorCode);
        return false;
    }
    Finally _freeAttributes{[&attributes]() { pthread_mutexattr_destroy(&attributes); }};

    if (auto errorCode = pthread_mutexattr_settype(&attributes, PTHREAD_MUTEX_RECURSIVE)) {
        *outError = "pthread_mutexattr_settype: "s + strerror(errorCode);
        return false;
    }

    if (auto errorCode = pthread_mutex_init(&m_mutex, &attributes)) {
        *outError = "pthread_mutex_init: "s + strerror(errorCode);
        return false;
    }

    m_wasInit = true;
    return true;
}
