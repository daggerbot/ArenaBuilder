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

#ifndef ARENABUILDER_CORE_TYPES_H_INCLUDED
#define ARENABUILDER_CORE_TYPES_H_INCLUDED

#include <cstdint>
#include <string>
#include <type_traits>
#include <utility>

#include "CompilerDefs.h"

namespace ArenaBuilder {

    using nullptr_t = decltype(nullptr);
    using ptrdiff_t = decltype(static_cast<int*>(0) - static_cast<int*>(0));
    using size_t = decltype(sizeof(int));

    using int8_t = std::int8_t;
    using int16_t = std::int16_t;
    using int32_t = std::int32_t;
    using int64_t = std::int64_t;
    using intptr_t = std::intptr_t;

    using uint8_t = std::uint8_t;
    using uint16_t = std::uint16_t;
    using uint32_t = std::uint32_t;
    using uint64_t = std::uint64_t;
    using uintptr_t = std::uintptr_t;

    // oschar_t: Preferred character type when using the underlying platform APIs.
    // OSSTR: Encodes a string literal as an array of oschar_t.
#ifdef _WIN32
# define OSSTR_(x) L##x
# define OSSTR(x) OSSTR_(x)
    using oschar_t = wchar_t;
#else
# define OSSTR(x) x
    using oschar_t = char;
#endif

    using OsString = std::basic_string<oschar_t>;
    using OsStringView = std::basic_string_view<oschar_t>;

    // Reference wrapper for function output parameters. This explicitly indicates at the call site
    // that a function call may modify the value. Construct with: Out{value}
    template<typename T>
    class Out {
    public:
        Out() = delete;
        constexpr Out(const Out<T>& other) = default;

        template<typename U>
        constexpr Out(const Out<U>& other)
            : m_ref{other.GetReference()}
        {
        }

        constexpr explicit Out(T& ref)
            : m_ref{ref}
        {
        }

        constexpr T* GetPointer() const { return &m_ref; }
        constexpr T& GetReference() const { return m_ref; }

        Out<T>& operator=(const Out<T>& other) = delete;
        Out<T>& operator=(Out<T>&& other) = delete;

        constexpr T& operator*() const { return m_ref; }
        constexpr T* operator->() const { return &m_ref; }

    private:
        T& m_ref;
    };

    // Pointer wrapper for optional function output parameters. This explicitly indicates at the
    // call site that a function call may modify the value. Construct with: Out{value}
    template<typename T>
    class OptionalOut {
    public:
        constexpr OptionalOut() = default;
        constexpr OptionalOut(const OptionalOut<T>& other) = default;

        template<typename U>
        constexpr OptionalOut(const Out<U>& other)
            : m_ptr{other.GetPointer()}
        {
        }

        template<typename U>
        constexpr OptionalOut(const OptionalOut<U>& other)
            : m_ptr{other.GetPointer()}
        {
        }

        constexpr OptionalOut(nullptr_t)
            : m_ptr{nullptr}
        {
        }

        constexpr OptionalOut(T* ptr)
            : m_ptr{ptr}
        {
        }

        constexpr T* GetPointer() const { return m_ptr; }
        constexpr bool HasValue() const { return bool(m_ptr); }

        // Sets the value if m_ptr is non-null. Does nothing if m_ptr is null.
        template<typename Value>
        void SetValue(Value&& value)
        {
            if (m_ptr) {
                *m_ptr = std::forward<Value>(value);
            }
        }

        OptionalOut<T>& operator=(const OptionalOut<T>& other) = delete;
        OptionalOut<T>& operator=(OptionalOut<T>&& other) = delete;

        constexpr explicit operator bool() const { return bool(m_ptr); }
        constexpr bool operator!() const { return !m_ptr; }

    private:
        T* m_ptr = nullptr;
    };

    // Object which invokes a callback (usually a lambda) from its destructor. This is typically
    // used to defer clean-up to the end of a scope.
    template<typename T>
    class Finally {
    public:
        Finally() = delete;
        Finally(const Finally<T>&) = delete;
        Finally(Finally<T>&&) = delete;

        explicit Finally(T&& callback)
            : m_callback{std::move(callback)}
        {
        }

        ~Finally()
        {
            m_callback();
        }

        Finally<T>& operator=(const Finally<T>&) = delete;
        Finally<T>& operator=(Finally<T>&&) = delete;

    private:
        T m_callback;
    };

} // namespace ArenaBuilder

#endif // ARENABUILDER_CORE_TYPES_H_INCLUDED
