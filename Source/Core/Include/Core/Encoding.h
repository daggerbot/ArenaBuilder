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

#ifndef ARENABUILDER_ENCODING_H_INCLUDED
#define ARENABUILDER_ENCODING_H_INCLUDED

#include <fmt/core.h>
#include <fmt/xchar.h>

#include "Types.h"

namespace ArenaBuilder {

    namespace Encoding {

        std::wstring SystemToWide(std::string_view inStr);

    } // namespace Encoding

    // Formats narrow strings as wide.
    template<typename T>
    struct StringWidener {
        const T& value;

        StringWidener() = delete;
        constexpr StringWidener(const StringWidener<T>&) = default;

        constexpr explicit StringWidener(const T& value)
            : value{value}
        {
        }

        StringWidener<T>& operator=(const StringWidener<T>&) = delete;
    };

} // namespace ArenaBuilder

template<typename T>
struct fmt::formatter<ArenaBuilder::StringWidener<T>, wchar_t> : formatter<std::wstring, wchar_t> {
    using formatter<std::wstring, wchar_t>::formatter;
    using formatter<std::wstring, wchar_t>::parse;

    wformat_context::iterator format(const ArenaBuilder::StringWidener<T>& str, wformat_context& ctx) const
    {
        return formatter<std::wstring, wchar_t>::format(ArenaBuilder::Encoding::SystemToWide(str.value), ctx);
    }
};

#endif // ARENABUILDER_ENCODING_H_INCLUDED
