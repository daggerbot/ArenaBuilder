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

#ifndef ARENABUILDER_CORE_MUTEX_H_INCLUDED
#define ARENABUILDER_CORE_MUTEX_H_INCLUDED

#ifndef _WIN32
# include <pthread.h>
#endif

#include <string>

#include "Types.h"

namespace ArenaBuilder {

    // Substitute for std::recursive_mutex, which may be unavailable on some configurations (i.e.
    // MinGW with Win32 threads).
    class RecursiveMutex {
    public:
        RecursiveMutex(); // Aborts on failure
        RecursiveMutex(const RecursiveMutex&) = delete;
        RecursiveMutex(RecursiveMutex&&) = delete;
        explicit RecursiveMutex(Out<std::string> outError);
        ~RecursiveMutex();

        void Lock();
        bool TryLock(OptionalOut<std::string> outError = {});
        void Unlock();

        RecursiveMutex& operator=(const RecursiveMutex&) = delete;
        RecursiveMutex& operator=(RecursiveMutex&&) = delete;

    private:
#ifdef _WIN32
        void* m_handle = nullptr;
#else
        pthread_mutex_t m_mutex = PTHREAD_MUTEX_INITIALIZER;
        bool m_wasInit = false;
#endif

        bool Initialize(Out<std::string> outError);
    };

} // namespace ArenaBuilder

#endif // ARENABUILDER_CORE_MUTEX_H_INCLUDED
