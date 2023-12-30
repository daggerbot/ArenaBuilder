# Copyright (c) 2021-2023 Marty Mills <daggerbot@gmail.com>
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
# OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
# CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

cmake_minimum_required(VERSION "3.19")

if(NOT TARGET "libzip::libzip")
    include("FindPackageHandleStandardArgs")
    find_package("ZLIB")
    unset(LIBZIP_VERSION)

    find_library(LIBZIP_LIBRARY "zip"
        HINTS "$ENV{LIBZIPDIR}"
        PATHS "${LIBZIP_PREFIX}")

    find_path(LIBZIP_zip_INCLUDE_DIR "zip.h"
        HINTS "$ENV{LIBZIPDIR}"
        PATHS "${LIBZIP_PREFIX}"
        PATH_SUFFIXES "include")

    find_path(LIBZIP_zipconf_INCLUDE_DIR "zipconf.h"
        HINTS "$ENV{LIBZIPDIR}"
        PATHS "${LIBZIP_PREFIX}"
        PATH_SUFFIXES "include")

    if(LIBZIP_zip_INCLUDE_DIR AND LIBZIP_zipconf_INCLUDE_DIR)
        set(LIBZIP_INCLUDE_DIRS "${LIBZIP_zip_INCLUDE_DIR}" "${LIBZIP_zipconf_INCLUDE_DIR}" CACHE INTERNAL "")
    endif()

    if(LIBZIP_LIBRARY AND LIBZIP_INCLUDE_DIRS)
        file(READ "${LIBZIP_zipconf_INCLUDE_DIR}/zipconf.h" _LIBZIP_ZIPCONF_H)
        string(REGEX MATCH "#[ \\t]*define[ \\t]+LIBZIP_VERSION_MAJOR[ \\t]+([0-9]+)" _LIBZIP_VERSION_MAJOR "${_LIBZIP_ZIPCONF_H}")
        string(REGEX MATCH "#[ \\t]*define[ \\t]+LIBZIP_VERSION_MINOR[ \\t]+([0-9]+)" _LIBZIP_VERSION_MINOR "${_LIBZIP_ZIPCONF_H}")
        string(REGEX MATCH "#[ \\t]*define[ \\t]+LIBZIP_VERSION_MICRO[ \\t]+([0-9]+)" _LIBZIP_VERSION_MICRO "${_LIBZIP_ZIPCONF_H}")

        if(_LIBZIP_VERSION_MAJOR AND _LIBZIP_VERSION_MINOR AND _LIBZIP_VERSION_MICRO)
            string(REGEX REPLACE "#[ \\t]*define[ \\t]+LIBZIP_VERSION_MAJOR[ \\t]+([0-9]+)" "\\1" _LIBZIP_VERSION_MAJOR "${_LIBZIP_VERSION_MAJOR}")
            string(REGEX REPLACE "#[ \\t]*define[ \\t]+LIBZIP_VERSION_MINOR[ \\t]+([0-9]+)" "\\1" _LIBZIP_VERSION_MINOR "${_LIBZIP_VERSION_MINOR}")
            string(REGEX REPLACE "#[ \\t]*define[ \\t]+LIBZIP_VERSION_MICRO[ \\t]+([0-9]+)" "\\1" _LIBZIP_VERSION_MICRO "${_LIBZIP_VERSION_MICRO}")
            set(LIBZIP_VERSION_MAJOR "${_LIBZIP_VERSION_MAJOR}" CACHE INTERNAL "")
            set(LIBZIP_VERSION_MINOR "${_LIBZIP_VERSION_MINOR}" CACHE INTERNAL "")
            set(LIBZIP_VERSION_PATCH "${_LIBZIP_VERSION_PATCH}" CACHE INTERNAL "")
            set(LIBZIP_VERSION "${LIBZIP_VERSION_MAJOR}.${LIBZIP_VERSION_MINOR}.${LIBZIP_VERSION_PATCH}" CACHE INTERNAL "")
        endif()

        unset(_LIBZIP_ZIPCONF_H)
        unset(_LIBZIP_VERSION_MAJOR)
        unset(_LIBZIP_VERSION_MINOR)
        unset(_LIBZIP_VERSION_MICRO)
    endif()

    if(LIBZIP_VERSION)
        set(LIBZIP_FOUND ON CACHE INTERNAL "")
        add_library("libzip::libzip" INTERFACE IMPORTED GLOBAL)
        target_include_directories("libzip::libzip" SYSTEM INTERFACE ${LIBZIP_INCLUDE_DIRS})
        target_link_libraries("libzip::libzip" INTERFACE
            ${LIBZIP_LIBRARY}
            ${ZLIB_LIBRARIES}
        )
    endif()

    find_package_handle_standard_args("libzip"
        REQUIRED_VARS LIBZIP_LIBRARY LIBZIP_FOUND
        VERSION_VAR LIBZIP_VERSION
        HANDLE_VERSION_RANGES)
endif()
