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

#include <stdio.h>
#ifdef _WIN32
# include <windows.h>
#endif

#include <Core/Debug.h>
#include <Core/Mutex.h>
#include <Core/System.h>

// ANSI escape sequences for decorating log messages. These are supported on Windows and on most
// Unix terminal emulators.
#define ANSI_RESET "\x1B[0m"
#define ANSI_BOLD "\x1B[1m"
#define ANSI_BLACK "\x1B[30m"
#define ANSI_RED "\x1B[31m"
#define ANSI_GREEN "\x1B[32m"
#define ANSI_YELLOW "\x1B[33m"
#define ANSI_BLUE "\x1B[34m"
#define ANSI_MAGENTA "\x1B[35m"
#define ANSI_CYAN "\x1B[36m"
#define ANSI_WHITE "\x1B[37m"

using namespace ArenaBuilder;

namespace {

    struct LoggerState {
        RecursiveMutex mutex;
        bool locked = false; // Flag to prevent interrupting a message with another message
        OsString fatalErrorMessage; // Fatal errors must be written all at once to prevent interruptions
    };

    LoggerState* s_loggerState = nullptr;
    LogLevel s_maxLogLevel =
#ifdef NDEBUG
        LogLevel::Warning;
#else
        LogLevel::Debug;
#endif

    // The __FILE__ token may evaluate to a longer path than we want for log messages. This returns
    // the path relative to the game's source directory.
    const char* GetShortSourceFileName(const char* path)
    {
        constexpr char sourceDir[] = ARENABUILDER_SOURCE_DIR;

        if (!path) {
            return nullptr;
        }

        // Skip sourceDir
        for (size_t i = 0;; ++i) {
            if (!sourceDir[i]) {
                path += i;
                break;
            } else if (path[i] != sourceDir[i]) {
                return path;
            }
        }

        // Skip path separator
        if (*path == '/' || *path == '\\') {
            ++path;
        }

        return path;
    }

    void WriteLogMessageSuffix(const char* sourceFileName, int sourceLine)
    {
        if (sourceFileName) {
            sourceFileName = GetShortSourceFileName(sourceFileName);
#ifdef _WIN32
            // We can't simply use fwprintf for all of this, because some implementations of Windows
            // CRT have the types reversed for %s and %S.
            fputws(L" " ANSI_BOLD ANSI_BLACK "(", stderr);
            fputs(sourceFileName, stderr);
            fwprintf(stderr, L":%d)", sourceLine);
#else
            fprintf(stderr, " " ANSI_BOLD ANSI_BLACK "(%s:%d)", sourceFileName, sourceLine);
#endif
        }

#ifdef _WIN32
        fputws(L"" ANSI_RESET "\r\n", stderr);
#else
        fputs(ANSI_RESET "\n", stderr);
#endif
        fflush(stderr);
    }

} // namespace

void Debug::InitLogger()
{
    static LoggerState globalLoggerState;
    s_loggerState = &globalLoggerState;

#ifdef _WIN32

    AttachConsole(ATTACH_PARENT_PROCESS);
    _wfreopen(L"CONOUT$", L"w", stderr);

    SetConsoleMode(GetStdHandle(STD_ERROR_HANDLE),
                   ENABLE_PROCESSED_OUTPUT
                   | ENABLE_WRAP_AT_EOL_OUTPUT
                   | ENABLE_VIRTUAL_TERMINAL_PROCESSING
                   | DISABLE_NEWLINE_AUTO_RETURN);
#else // !defined(_WIN32)

    static char buffer[BUFSIZ];

    // stderr is unbuffered by default. By making it buffered, we improve logging performance by
    // reducing the number of syscalls, likely to a single write() per message.
    setbuf(stderr, buffer);

#endif // !defined(_WIN32)
}

void Debug::EnableVerboseLogMessages()
{
#ifdef NDEBUG
    s_maxLogLevel = LogLevel::Info;
#else
    s_maxLogLevel = LogLevel::Trace;
#endif
}

bool Debug::Internal::BeginLogMessage(LogLevel level)
{
    const oschar_t* prefix;

    if (int(level) > int(s_maxLogLevel)) {
        return false;
    }

    switch (level) {
    case LogLevel::Error:
        prefix = OSSTR(ANSI_BOLD ANSI_RED "Error: " ANSI_RESET);
        break;
    case LogLevel::Warning:
        prefix = OSSTR(ANSI_BOLD ANSI_YELLOW "Warning: " ANSI_RESET);
        break;
    case LogLevel::Info:
        prefix = OSSTR(ANSI_BOLD ANSI_BLUE "Info: " ANSI_RESET);
        break;
    case LogLevel::Debug:
        prefix = OSSTR(ANSI_BOLD ANSI_MAGENTA "Debug: " ANSI_RESET);
        break;
    case LogLevel::Trace:
        prefix = OSSTR(ANSI_BOLD ANSI_CYAN "Trace: " ANSI_BLACK);
        break;
    default:
        return false;
    }

    if (!s_loggerState->mutex.TryLock()) {
        return false;
    } else if (s_loggerState->locked) {
        s_loggerState->mutex.Unlock();
        return false;
    }

    s_loggerState->locked = true;

#ifdef _WIN32
    fputws(prefix, stderr);
#else
    fputs(prefix, stderr);
#endif

    return true;
}

void Debug::Internal::PutLogMessageChar(oschar_t ch)
{
#ifdef _WIN32
    fputwc(ch, stderr);
#else
    fputc(ch, stderr);
#endif
}

void Debug::Internal::EndLogMessage(const char* sourceFileName, int sourceLine)
{
    WriteLogMessageSuffix(sourceFileName, sourceLine);
    s_loggerState->locked = false;
    s_loggerState->mutex.Unlock();
}

void Debug::Internal::BeginFatalError()
{
    s_loggerState->mutex.Lock();
    s_loggerState->locked = true;
    s_loggerState->fatalErrorMessage.clear(); // In case we're interrupting another fatal error message
}

void Debug::Internal::PutFatalErrorChar(oschar_t ch)
{
    s_loggerState->fatalErrorMessage.push_back(ch);
}

void Debug::Internal::EndFatalErrorAndExit(const char* sourceFileName, int sourceLine)
{
#ifdef _WIN32
    // We can't simply use fwprintf for all of this, because some implementations of Windows CRT
    // have the types reversed for %s and %S.
    fputws(L"" ANSI_BOLD ANSI_RED "Fatal error: " ANSI_RESET ANSI_BOLD, stderr);
    fputws(s_loggerState->fatalErrorMessage.c_str(), stderr);
#else
    fprintf(stderr, ANSI_BOLD ANSI_RED "Fatal error: " ANSI_RESET ANSI_BOLD "%s", s_loggerState->fatalErrorMessage.c_str());
#endif

    WriteLogMessageSuffix(sourceFileName, sourceLine);

    if (sourceFileName) {
        sourceFileName = GetShortSourceFileName(sourceFileName);
        fmt::format_to(FatalErrorIterator{}, OsStringView{OSSTR(" ({}:{})")}, Forward(sourceFileName), sourceLine);
    }

    System::ExitWithErrorDialog(s_loggerState->fatalErrorMessage.c_str());
}
