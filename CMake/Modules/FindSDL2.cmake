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

if(NOT TARGET "SDL2::SDL2")
    include("FindPackageHandleStandardArgs")
    unset(SDL2_VERSION)

    find_library(SDL2_LIBRARY "SDL2"
        HINTS "$ENV{SDL2DIR}"
        PATHS "${SDL2_PREFIX}")

    find_library(SDL2_main_LIBRARY "SDL2main"
        HINTS "$ENV{SDL2DIR}"
        PATHS "${SDL2_PREFIX}")

    find_path(SDL2_INCLUDE_DIR "SDL_version.h"
        HINTS "$ENV{SDL2DIR}"
        PATHS "${SDL2_PREFIX}"
        PATH_SUFFIXES "include/SDL2" "include")

    if(SDL2_LIBRARY AND SDL2_INCLUDE_DIR)
        file(READ "${SDL2_INCLUDE_DIR}/SDL_version.h" _SDL2_VERSION_H)
        string(REGEX MATCH "#[ \\t]*define[ \\t]+SDL_MAJOR_VERSION[ \\t]+([0-9]+)" _SDL2_MAJOR_VERSION "${_SDL2_VERSION_H}")
        string(REGEX MATCH "#[ \\t]*define[ \\t]+SDL_MINOR_VERSION[ \\t]+([0-9]+)" _SDL2_MINOR_VERSION "${_SDL2_VERSION_H}")
        string(REGEX MATCH "#[ \\t]*define[ \\t]+SDL_PATCHLEVEL[ \\t]+([0-9]+)" _SDL2_PATCHLEVEL "${_SDL2_VERSION_H}")

        if(_SDL2_MAJOR_VERSION AND _SDL2_MINOR_VERSION AND _SDL2_PATCHLEVEL)
            string(REGEX REPLACE "#[ \\t]*define[ \\t]+SDL_MAJOR_VERSION[ \\t]+([0-9]+)" "\\1" _SDL2_MAJOR_VERSION "${_SDL2_MAJOR_VERSION}")
            string(REGEX REPLACE "#[ \\t]*define[ \\t]+SDL_MINOR_VERSION[ \\t]+([0-9]+)" "\\1" _SDL2_MINOR_VERSION "${_SDL2_MINOR_VERSION}")
            string(REGEX REPLACE "#[ \\t]*define[ \\t]+SDL_PATCHLEVEL[ \\t]+([0-9]+)" "\\1" _SDL2_PATCHLEVEL "${_SDL2_PATCHLEVEL}")
            if(_SDL2_MAJOR_VERSION STREQUAL "2")
                set(SDL2_VERSION_MAJOR "${_SDL2_MAJOR_VERSION}" CACHE INTERNAL "")
                set(SDL2_VERSION_MINOR "${_SDL2_MINOR_VERSION}" CACHE INTERNAL "")
                set(SDL2_VERSION_PATCH "${_SDL2_PATCHLEVEL}" CACHE INTERNAL "")
                set(SDL2_VERSION "${SDL2_VERSION_MAJOR}.${SDL2_VERSION_MINOR}.${SDL2_VERSION_PATCH}" CACHE INTERNAL "")
            endif()
        endif()

        unset(_SDL2_VERSION_H)
        unset(_SDL2_MAJOR_VERSION)
        unset(_SDL2_MINOR_VERSION)
        unset(_SDL2_PATCHLEVEL)
    endif()

    if(SDL2_VERSION)
        set(SDL2_FOUND ON CACHE INTERNAL "")
        add_library("SDL2::SDL2" INTERFACE IMPORTED GLOBAL)
        target_include_directories("SDL2::SDL2" SYSTEM INTERFACE "${SDL2_INCLUDE_DIR}")
        target_link_libraries("SDL2::SDL2" INTERFACE "${SDL2_LIBRARY}")
    endif()

    if(SDL2_VERSION AND SDL2_main_LIBRARY)
        set(SDL2_main_FOUND ON CACHE INTERNAL "")
        add_library("SDL2::main" INTERFACE IMPORTED GLOBAL)
        target_link_libraries("SDL2::main" INTERFACE "${SDL2_main_LIBRARY}")
    endif()

    find_package_handle_standard_args("SDL2"
        REQUIRED_VARS SDL2_LIBRARY SDL2_FOUND
        VERSION_VAR SDL2_VERSION
        HANDLE_COMPONENTS HANDLE_VERSION_RANGES)
endif()
