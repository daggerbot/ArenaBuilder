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

#include <Core/IO/Base.h>
#include <Core/Debug.h>

using namespace ArenaBuilder;

Stream::Stream(Stream&& other)
    : m_eof{other.m_eof}
{
    other.m_eof = false;
}

Stream::~Stream()
{
}

size_t Stream::Read(void* buffer, size_t size, Out<std::string> outError)
{
    char* position = reinterpret_cast<char*>(buffer);
    size_t bytesRemaining = size;
    size_t totalBytesRead = 0;
    size_t result;

    if (!size) {
        return 0;
    } else if (ErrorIfClosed(outError)) {
        return 0;
    }

    ASSERT(buffer != nullptr);

    while (totalBytesRead < size) {
        result = DoRead(position, bytesRemaining, outError);
        if (!result) {
            SetEof();
            break;
        }

        ASSERT(result <= bytesRemaining);
        position += result;
        bytesRemaining -= result;
        totalBytesRead += result;
    }

    return totalBytesRead;
}

size_t Stream::ReadExact(void* buffer, size_t size, Out<std::string> outError)
{
    size_t result = Read(buffer, size, outError);

    if (result < size && outError->empty()) {
        *outError = "Unexpected end of stream";
    }

    return result;
}

size_t Stream::Write(const void* buffer, size_t size, Out<std::string> outError)
{
    const char* position = reinterpret_cast<const char*>(buffer);
    size_t bytesRemaining = size;
    size_t totalBytesWritten = 0;
    size_t result;

    if (!size) {
        return 0;
    } else if (ErrorIfClosed(outError)) {
        return 0;
    }

    ASSERT(buffer != nullptr);

    while (totalBytesWritten < size) {
        result = DoWrite(position, bytesRemaining, outError);
        if (!result) {
            if (outError->empty()) {
                *outError = "Unexpected end of stream";
            }
            break;
        }

        ASSERT(result <= bytesRemaining);
        position += result;
        bytesRemaining -= result;
        totalBytesWritten += result;
    }

    return totalBytesWritten;
}

size_t Stream::DoRead(void*, size_t, Out<std::string> outError)
{
    *outError = "Stream is not readable";
    return 0;
}

size_t Stream::DoWrite(const void*, size_t, Out<std::string> outError)
{
    *outError = "Stream is not writable";
    return 0;
}

bool Stream::ErrorIfClosed(Out<std::string> outError) const
{
    if (!IsOpen()) {
        *outError = "Stream is closed";
        return true;
    }
    return false;
}
