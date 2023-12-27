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

#ifndef ARENABUILDER_CORE_STRINGUTILS_H_INCLUDED
#define ARENABUILDER_CORE_STRINGUTILS_H_INCLUDED

#include "Types.h"

namespace ArenaBuilder {

    // Gets a pointer to the first occurrence of ch in str. Returns null if str in null. Returns the
    // end of str if ch is zero.
    template<typename CharT>
    constexpr const CharT* FindInCString(const CharT* str, CharT ch)
    {
        if (!str) {
            return nullptr;
        }

        while (true) {
            if (*str == ch) {
                return str;
            } else if (!*str) {
                return nullptr;
            } else {
                ++str;
            }
        }
    }

    // Gets the length of a zero-terminated string. Returns zero if ptr is null.
    template<typename CharT>
    constexpr size_t GetCStringLength(const CharT* ptr)
    {
        size_t length = 0;

        if (ptr) {
            while (ptr[length]) {
                ++length;
            }
        }
        return length;
    }

} // namespace ArenaBuilder

#endif // ARENABUILDER_CORE_STRINGUTILS_H_INCLUDED
