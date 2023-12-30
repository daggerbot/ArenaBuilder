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

#ifndef ARENABUILDER_CORE_IO_BASE_H_INCLUDED
#define ARENABUILDER_CORE_IO_BASE_H_INCLUDED

#include <memory>

#include "../Types.h"

namespace ArenaBuilder {

    // Base class for I/O streams.
    class Stream {
    public:
        Stream() = default;
        Stream(const Stream&) = default;
        Stream(Stream&& other);
        virtual ~Stream() = 0;

        virtual void Close() = 0;
        virtual bool IsOpen() const = 0;

        // EOF (end of file/stream) flag
        bool Eof() const { return m_eof; }
        void ClearEof() { m_eof = false; }
        void SetEof(bool eof = true) { m_eof = eof; }

        // Reads until 'size' bytes are read, the end of the stream is reached, or an error occurs.
        size_t Read(void* buffer, size_t size, Out<std::string> outError);

        // Reads until 'size' bytes are read or an error occurs. If the end of the stream is reached
        // before 'size' bytes are read, this is treaded as an error and outError is set
        // accordingly.
        size_t ReadExact(void* buffer, size_t size, Out<std::string> outError);

        // Writes until 'size' bytes are written or an error occurs. If DoWrite() returns zero but
        // does not set outError, this is treated as an error and outError will be set to a fallback
        // string.
        size_t Write(const void* buffer, size_t size, Out<std::string> outError);

        Stream& operator=(const Stream&) = delete;
        Stream& operator=(Stream&&) = delete;

    protected:
        // Reads a single pass. Should return the number of bytes read on success (<= size), or 0 if
        // an error occurs or if the end of the stream is reached. This typically corresponds to one
        // syscall, i.e. read().
        virtual size_t DoRead(void* buffer, size_t size, Out<std::string> outError);

        // Writes a single pass. Should return the number of bytes written on success (<= size), or
        // 0 if an error occurs. If outError is empty after this returns zero, this is still treated
        // as an error and wrapper functions will set outError accordingly. This function typically
        // corresponds to a signle syscall, i.e. write().
        virtual size_t DoWrite(const void* buffer, size_t size, Out<std::string> outError);

    private:
        bool m_eof = false;

        bool ErrorIfClosed(Out<std::string> outError) const;
    };

    // Interface for opening named data streams for reading.
    class DataSource {
    public:
        virtual ~DataSource() = 0;
        virtual std::unique_ptr<Stream> OpenStream(std::string_view name, Out<std::string> outError) = 0;

        DataSource& operator=(const DataSource&) = delete;
        DataSource& operator=(DataSource&&) = delete;
    };

    inline DataSource::~DataSource() = default;

} // namespace ArenaBuilder

#endif // ARENABUILDER_CORE_IO_BASE_H_INCLUDED
