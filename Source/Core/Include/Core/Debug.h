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

#ifndef ARENABUILDER_CORE_DEBUG_H_INCLUDED
#define ARENABUILDER_CORE_DEBUG_H_INCLUDED

#include <iterator>

#include <fmt/core.h>
#include <fmt/xchar.h>

#include "Encoding.h"

// Logging macros
#ifdef NDEBUG
# define FATAL(...) do { ::ArenaBuilder::Debug::Internal::LogFatalErrorAndExit(nullptr, 0, OSSTR("") __VA_ARGS__); } while (0)
# define LOG_ERROR(...) do { ::ArenaBuilder::Debug::Internal::LogMessage(::ArenaBuilder::LogLevel::Error, nullptr, 0, OSSTR("") __VA_ARGS__); } while (0)
# define LOG_WARNING(...) do { ::ArenaBuilder::Debug::Internal::LogMessage(::ArenaBuilder::LogLevel::Warning, nullptr, 0, OSSTR("") __VA_ARGS__); } while (0)
# define LOG_INFO(...) do { ::ArenaBuilder::Debug::Internal::LogMessage(::ArenaBuilder::LogLevel::Info, nullptr, 0, OSSTR("") __VA_ARGS__); } while (0)
# define LOG_DEBUG(...) do { ::ArenaBuilder::Debug::Internal::DiscardMessage(OSSTR("") __VA_ARGS__); } while (0)
# define LOG_TRACE(...) do { ::ArenaBuilder::Debug::Internal::DiscardMessage(OSSTR("") __VA_ARGS__); } while (0)
#else
# define FATAL(...) do { ::ArenaBuilder::Debug::Internal::LogFatalErrorAndExit(__FILE__, __LINE__, OSSTR("") __VA_ARGS__); } while (0)
# define LOG_ERROR(...) do { ::ArenaBuilder::Debug::Internal::LogMessage(::ArenaBuilder::LogLevel::Error, __FILE__, __LINE__, OSSTR("") __VA_ARGS__); } while (0)
# define LOG_WARNING(...) do { ::ArenaBuilder::Debug::Internal::LogMessage(::ArenaBuilder::LogLevel::Warning, __FILE__, __LINE__, OSSTR("") __VA_ARGS__); } while (0)
# define LOG_INFO(...) do { ::ArenaBuilder::Debug::Internal::LogMessage(::ArenaBuilder::LogLevel::Info, __FILE__, __LINE__, OSSTR("") __VA_ARGS__); } while (0)
# define LOG_DEBUG(...) do { ::ArenaBuilder::Debug::Internal::LogMessage(::ArenaBuilder::LogLevel::Debug, __FILE__, __LINE__, OSSTR("") __VA_ARGS__); } while (0)
# define LOG_TRACE(...) do { ::ArenaBuilder::Debug::Internal::LogMessage(::ArenaBuilder::LogLevel::Trace, __FILE__, __LINE__, OSSTR("") __VA_ARGS__); } while (0)
#endif

namespace ArenaBuilder {

    enum class LogLevel {
        Fatal,
        Error,
        Warning,
        Info,
        Debug,
        Trace,
    };

    namespace Debug {

        void InitLogger();
        void EnableVerboseLogMessages();

        namespace Internal {

            bool BeginLogMessage(LogLevel level);
            void PutLogMessageChar(oschar_t ch);
            void EndLogMessage(const char* sourceFileName, int sourceLine);

            void BeginFatalError();
            void PutFatalErrorChar(oschar_t ch);
            [[noreturn]] void EndFatalErrorAndExit(const char* sourceFileName, int sourceLine);

            // Forwards format values, and possibly transforms their type. This is a workaround for
            // the limitation that fmt doesn't like to mix character types.
            template<typename T>
            constexpr const T& Forward(const T& value)
            {
                return value;
            }

#ifdef _WIN32

            constexpr StringWidener<const char*> Forward(const char* str)
            {
                return StringWidener{str};
            }

            constexpr StringWidener<std::string_view> Forward(std::string_view str)
            {
                return StringWidener{str};
            }

            inline StringWidener<std::string> Forward(std::string str)
            {
                return StringWidener{str};
            }

#endif // defined(_WIN32)

            // Output iterator which calls PutLogMessageChar each time a char is assigned.
            class LogMessageIterator {
            public:
                using difference_type = ptrdiff_t;
                using iterator_category = std::output_iterator_tag;
                using pointer = oschar_t*;
                using reference = oschar_t&;
                using value_type = oschar_t;

                constexpr LogMessageIterator() = default;
                constexpr LogMessageIterator(const LogMessageIterator&) = default;

                constexpr LogMessageIterator& operator=(const LogMessageIterator&) = default;

                LogMessageIterator& operator=(oschar_t ch)
                {
                    PutLogMessageChar(ch);
                    return *this;
                }

                constexpr LogMessageIterator& operator*() { return *this; }
                constexpr LogMessageIterator& operator++() { return *this; }
                constexpr LogMessageIterator& operator++(int) { return *this; }
            };

            // Output iterator which calls PutFatalErrorChar each time a char is assigned.
            class FatalErrorIterator {
            public:
                using difference_type = ptrdiff_t;
                using iterator_category = std::output_iterator_tag;
                using pointer = oschar_t*;
                using reference = oschar_t&;
                using value_type = oschar_t;

                constexpr FatalErrorIterator() = default;
                constexpr FatalErrorIterator(const FatalErrorIterator&) = default;

                constexpr FatalErrorIterator& operator=(const FatalErrorIterator&) = default;

                FatalErrorIterator& operator=(oschar_t ch)
                {
                    PutFatalErrorChar(ch);
                    return *this;
                }

                constexpr FatalErrorIterator& operator*() { return *this; }
                constexpr FatalErrorIterator& operator++() { return *this; }
                constexpr FatalErrorIterator& operator++(int) { return *this; }
            };

            template<typename...Args>
            void LogMessage(LogLevel level, const char* sourceFileName, int sourceLine,
                            std::basic_string_view<oschar_t> fmt, const Args&...args)
            {
                if (BeginLogMessage(level)) {
                    fmt::format_to(LogMessageIterator{}, fmt, Internal::Forward(args)...);
                    EndLogMessage(sourceFileName, sourceLine);
                }
            }

            template<typename...Args>
            [[noreturn]]
            void LogFatalErrorAndExit(const char* sourceFileName, int sourceLine,
                                      std::basic_string_view<oschar_t> fmt, const Args&...args)
            {
                BeginFatalError();
                fmt::format_to(FatalErrorIterator{}, fmt, Internal::Forward(args)...);
                EndFatalErrorAndExit(sourceFileName, sourceLine);
            }

            template<typename...Args>
            constexpr void DiscardMessage(std::basic_string_view<oschar_t>, const Args&...)
            {
            }

        } // namespace Internal

    } // namespace Debug

} // namespace ArenaBuilder

#endif // ARENABUILDER_CORE_DEBUG_H_INCLUDED
