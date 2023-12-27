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

#ifndef ARENABUILDER_CORE_COMMANDLINE_H_INCLUDED
#define ARENABUILDER_CORE_COMMANDLINE_H_INCLUDED

#include <optional>

#include "Types.h"

namespace ArenaBuilder {

    class CommandLineParser;

    // Handles incoming command line arguments from a CommandLineParser.
    class CommandLineHandler {
    public:
        virtual ~CommandLineHandler() = 0;

        // These functions may return false to stop parsing.
        virtual bool HandleOperand(OsStringView operand) = 0;
        virtual bool HandleShortOption(oschar_t option, CommandLineParser& parser) = 0;
        virtual bool HandleLongOption(OsStringView option, CommandLineParser& parser) = 0;

        CommandLineHandler& operator=(const CommandLineHandler&) = delete;
        CommandLineHandler& operator=(CommandLineHandler&&) = delete;
    };

    inline CommandLineHandler::~CommandLineHandler() = default;

    // Parses command line arguments.
    class CommandLineParser {
    public:
        CommandLineParser() = delete;
        CommandLineParser(const CommandLineParser&) = delete;
        CommandLineParser(CommandLineParser&&) = delete;

        // Returns false if parsing was stopped by returning false from any of the
        // CommandLineHandler functions.
        static bool Parse(int argc, const oschar_t* const* argv, CommandLineHandler& handler);

        // Consumes an option parameter if present. Returns null if no parameter is available. If an
        // option expects a parameter, this must be called to consume it even if it won't be used,
        // or else the parameter may be interpreted as something else.
        const oschar_t* GetParam();

        // Returns true if a parameter has been consumed for the current option. This will only
        // return true if the option has the syntax '--option=param' or if GetParam() has been
        // called. Otherwise, options with the syntax '--option param' or '-Oparam' may have their
        // parameters interpreted differently.
        bool HasParam() const { return m_param.has_value(); }

        CommandLineParser& operator=(const CommandLineParser&) = delete;
        CommandLineParser& operator=(CommandLineParser&&) = delete;

    private:
        enum class State { Initial, Operand, ShortOption, LongOption, OperandsOnly };

        int m_argCount;
        const oschar_t* const* m_args;
        int m_argIndex = 1;
        size_t m_charIndex = 0;
        State m_state = State::Initial;
        std::optional<const oschar_t*> m_param;

        explicit CommandLineParser(int argc, const oschar_t* const* argv)
            : m_argCount{argc}, m_args{argv}
        {
        }

        bool IsFinished() const { return m_argIndex >= m_argCount; }
        bool ParseNext(CommandLineHandler& handler);
    };

} // namespace ArenaBuilder

#endif // ARENABUILDER_CORE_COMMANDLINE_H_INCLUDED
