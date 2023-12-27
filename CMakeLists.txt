# Copyright (c) 2023 Martin Mills <daggerbot@gmail.com>
#
# This Source Code Form is subject to the terms of the Mozilla Public License
# version 2.0 (the "License"). If a copy of the License was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations
# under the License.

cmake_minimum_required(VERSION "3.25")
list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake/Modules")
project("ArenaBuilder" C CXX)

set(BINDIR "${CMAKE_CURRENT_BINARY_DIR}/bin")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${BINDIR}")

#---------------------------------------------------------------------------------------------------
# Configure compiler options

add_library("ArenaCompilerOptions" INTERFACE)

target_compile_definitions("ArenaCompilerOptions" INTERFACE
    $<$<CONFIG:MinSizeRel,Release,RelWithDebInfo>:NDEBUG>
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options("ArenaCompilerOptions" INTERFACE $<$<COMPILE_LANGUAGE:CXX>:
        "-std=c++17"
        "-pedantic"
        "-Wall"
        "-Wconversion"
        "-Wextra"
        "-Wsuggest-override"
    >)
else()
    set(CMAKE_CXX_STANDARD "17")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_compile_definitions("ArenaCompilerOptions" INTERFACE
        "_UNICODE"
        "_WIN32_WINNT=0x0A00"
        "NOMINMAX"
        "UNICODE"
        "WINVER=0x0A00"
    )
    if(MINGW)
        target_link_options("ArenaCompilerOptions" INTERFACE "-municode")
    endif()
endif()

#---------------------------------------------------------------------------------------------------
# Build components

add_subdirectory("Source")
