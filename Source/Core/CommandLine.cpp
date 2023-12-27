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

#include <Core/CommandLine.h>
#include <Core/StringUtils.h>

using namespace ArenaBuilder;

bool CommandLineParser::Parse(int argc, const oschar_t* const* argv, CommandLineHandler& handler)
{
    CommandLineParser parser{argc, argv};

    while (!parser.IsFinished()) {
        if (!parser.ParseNext(handler)) {
            return false;
        }
    }

    return true;
}

const oschar_t* CommandLineParser::GetParam()
{
    if (IsFinished()) {
        return nullptr;
    } else if (m_param) {
        return *m_param;
    }

    switch (m_state) {
    case State::ShortOption:
        if (m_args[m_argIndex][m_charIndex]) {
            // Use the rest of the short option chain as the parameter, i.e. '-Oparam'.
            m_param = &m_args[m_argIndex][m_charIndex];
            m_charIndex += GetCStringLength(*m_param);
        } else if (m_argIndex + 1 < m_argCount) {
            // We're already at the end of the short option chain. Use the next arg as the
            // parameter, i.e. '-O param'.
            m_param = m_args[++m_argIndex];
            m_charIndex = GetCStringLength(*m_param);
        } else {
            // No parameter is available.
            m_param = nullptr;
        }
        return *m_param;

    case State::LongOption:
        if (m_argIndex + 1 < m_argCount) {
            // Use the next arg as the parameter, i.e. '--option param'. If the option has the
            // syntax '--option=param', then the parameter would have already been consumed by
            // ParseNext().
            m_param = m_args[++m_argIndex];
            m_charIndex = GetCStringLength(*m_param);
        } else {
            // No parameter is available.
            m_param = nullptr;
        }
        return *m_param;

    default:
        return nullptr;
    }
}

bool CommandLineParser::ParseNext(CommandLineHandler& handler)
{
    bool result;

    if (IsFinished()) {
        // Should be unreachable.
        return true;
    } else if (m_charIndex) {
        // Continue a short option chain, i.e. '-abc'.
        oschar_t option = m_args[m_argIndex][m_charIndex++];
        m_state = State::ShortOption;
        result = handler.HandleShortOption(option, *this);
    } else if (m_args[m_argIndex][0] != '-' || m_state == State::OperandsOnly) {
        // Arg is an operand.
        OsStringView operand = m_args[m_argIndex];
        m_charIndex = operand.length();
        result = handler.HandleOperand(operand);
    } else if (m_args[m_argIndex][1] == '-') {
        if (m_args[m_argIndex][2]) {
            // Arg is a long option.
            const oschar_t* optionStart = &m_args[m_argIndex][2];
            const oschar_t* equalsPtr = FindInCString(optionStart, oschar_t{'='});
            OsStringView option;

            if (equalsPtr) {
                // An explicit parameter is present, i.e. '--option=param'.
                option = {optionStart, size_t(equalsPtr - optionStart)};
                m_param = equalsPtr + 1;
            } else {
                // No explicit parameter is present. There still may be a parameter if GetParam() is
                // called, i.e. '--option param'.
                option = optionStart;
            }

            m_charIndex = 2 + GetCStringLength(optionStart);
            m_state = State::LongOption;
            result = handler.HandleLongOption(option, *this);
        } else {
            // The arg is '--'. All remaining args are operands, even if they start with '-'.
            ++m_argIndex;
            m_state = State::OperandsOnly;
            return true;
        }
    } else {
        // Start of a short option chain. Even if the arg is just '-', it will be interpreted as a
        // short option with option='\0'.
        oschar_t option = m_args[m_argIndex][1];
        m_charIndex = option ? 2 : 1;
        m_state = State::ShortOption;
        result = handler.HandleShortOption(option, *this);
    }

    // Clean up.
    if (!m_args[m_argIndex][m_charIndex]) {
        // We've reached the end of the current arg. Move to the next one.
        ++m_argIndex;
        m_charIndex = 0;
    }

    if (m_state != State::OperandsOnly) {
        m_state = State::Initial;
    }

    m_param = {};
    return result;
}
