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

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <limits>

#include <zip.h>

#include <Core/IO/Codec/Zip.h>
#include <Core/Debug.h>

using namespace std::literals::string_literals;
using namespace ArenaBuilder;

ZipArchiveReader::ZipArchiveReader(const oschar_t* path, Out<std::string> outError)
{
    Open(path, outError);
}

ZipArchiveReader::~ZipArchiveReader()
{
    Close();
}

bool ZipArchiveReader::Open(const oschar_t* path, Out<std::string> outError)
{
    FILE* file;
    zip_error_t zipError;
    zip_source_t* zipSource;

    Close();

    // Open the stdio file stream that libzip will use.
#ifdef _WIN32
    file = _wfopen(path, L"rb");
#else
    file = fopen(path, "rb");
#endif
    if (!file) {
        *outError = strerror(errno);
        return false;
    }

    // Initialize the zip error container.
    zip_error_init(&zipError);
    Finally _freeZipError{[&zipError]() { zip_error_fini(&zipError); }};

    // Create a zip source from the file stream.
    zipSource = zip_source_filep_create(file, 0, ZIP_LENGTH_TO_END, &zipError);
    if (!zipSource) {
        *outError = "zip_source_filep_create: "s + zip_error_strerror(&zipError);
        fclose(file);
        return false;
    }

    // Load the zip archive from the source.
    m_zip = zip_open_from_source(zipSource, ZIP_RDONLY | ZIP_CHECKCONS, &zipError);
    if (!m_zip) {
        *outError = "zip_open_from_source: "s + zip_error_strerror(&zipError);
        zip_source_free(zipSource);
        return false;
    }

    return true;
}

void ZipArchiveReader::Close()
{
    if (m_zip) {
        if (zip_close(m_zip)) {
            LOG_ERROR("zip_close: {}", zip_strerror(m_zip));
        }
        m_zip = nullptr;
    }
}

std::unique_ptr<Stream> ZipArchiveReader::OpenStream(std::string_view name, Out<std::string> outError)
{
    auto stream = std::make_unique<ZipInputStream>(*this, std::string{name}.c_str(), outError);

    if (!stream->IsOpen()) {
        if (outError->empty()) {
            *outError = "File not found";
        }
        stream.reset();
    }

    return stream;
}

//--------------------------------------------------------------------------------------------------

ZipInputStream::ZipInputStream(ZipArchiveReader& archive, const char* name, Out<std::string> outError)
{
    Open(archive, name, outError);
}

ZipInputStream::~ZipInputStream()
{
    Close();
}

bool ZipInputStream::Open(ZipArchiveReader& archive, const char* name, Out<std::string> outError)
{
    Close();

    if (!archive.IsOpen()) {
        *outError = "Archive is closed";
        return false;
    }

    m_zipFile = zip_fopen(archive.m_zip, name, 0);
    if (!m_zipFile) {
        *outError = "zip_fopen: "s + zip_strerror(archive.m_zip);
        return false;
    }

    return true;
}

void ZipInputStream::Close()
{
    zip_error_t zipError;

    if (m_zipFile) {
        if (int errorCode = zip_fclose(m_zipFile)) {
            zip_error_init_with_code(&zipError, errorCode);
            Finally _freeZipError{[&zipError]() { zip_error_fini(&zipError); }};
            LOG_ERROR("zip_fclose: {}", zip_error_strerror(&zipError));
        }
        m_zipFile = nullptr;
    }
}

size_t ZipInputStream::DoRead(void* buffer, size_t size, Out<std::string> outError)
{
    zip_int64_t result;

    if (size > std::numeric_limits<zip_uint64_t>::max()) {
        size = size_t(std::numeric_limits<zip_uint64_t>::max());
    }

    result = zip_fread(m_zipFile, buffer, zip_uint64_t(size));
    if (result < 0) {
        *outError = "zip_fread: "s + zip_file_strerror(m_zipFile);
        return 0;
    }

    return size_t(result);
}
