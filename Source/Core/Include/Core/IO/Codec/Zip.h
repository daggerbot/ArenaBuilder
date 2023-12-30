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

#ifndef ARENABUILDER_CORE_IO_CODEC_ZIP_H_INCLUDED
#define ARENABUILDER_CORE_IO_CODEC_ZIP_H_INCLUDED

#include "../Base.h"

struct zip;
struct zip_file;

namespace ArenaBuilder {

    class ZipArchiveReader final : public DataSource {
        friend class ZipInputStream;

    public:
        ZipArchiveReader() = default;
        ZipArchiveReader(const ZipArchiveReader&) = delete;
        ZipArchiveReader(ZipArchiveReader&&) = delete;
        explicit ZipArchiveReader(const oschar_t* path, Out<std::string> outError);
        ~ZipArchiveReader();

        bool Open(const oschar_t* path, Out<std::string> outError);
        void Close();
        bool IsOpen() const { return m_zip != nullptr; }

        std::unique_ptr<Stream> OpenStream(std::string_view name, Out<std::string> outError) override;

    private:
        struct ::zip* m_zip = nullptr;
    };

    class ZipInputStream final : public Stream {
    public:
        ZipInputStream() = default;
        ZipInputStream(const ZipInputStream&) = delete;
        ZipInputStream(ZipInputStream&&) = delete;
        explicit ZipInputStream(ZipArchiveReader& archive, const char* name, Out<std::string> outError);
        ~ZipInputStream();

        bool Open(ZipArchiveReader& archive, const char* name, Out<std::string> outError);
        void Close() override;
        bool IsOpen() const override { return m_zipFile != nullptr; }

    protected:
        size_t DoRead(void* buffer, size_t size, Out<std::string> outError) override;

    private:
        struct ::zip_file* m_zipFile = nullptr;
    };

} // namespace ArenaBuilder

#endif // ARENABUILDER_CORE_IO_CODEC_ZIP_H_INCLUDED
