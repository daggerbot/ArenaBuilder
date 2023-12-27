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

find_package("fmt" "10.1.1...<11" REQUIRED)

#---------------------------------------------------------------------------------------------------
# ArenaCore

add_library("ArenaCore" STATIC
    "CommandLine.cpp"
    "Debug.cpp"
)

target_include_directories("ArenaCore"
    PUBLIC
        "Include"
)

target_compile_definitions("ArenaCore"
    PRIVATE
        "ARENABUILDER_SOURCE_DIR=\"${ARENABUILDER_SOURCE_DIR}\""
)

target_link_libraries("ArenaCore"
    PUBLIC
        "fmt::fmt"
    PRIVATE
        "ArenaCompilerOptions"
)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_sources("ArenaCore"
        PRIVATE
            "Platform/Windows/Encoding.cpp"
            "Platform/Windows/Mutex.cpp"
            "Platform/Windows/System.cpp"
    )
elseif(UNIX AND NOT CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_sources("ArenaCore"
        PRIVATE
            "Platform/Unix/Mutex.cpp"
            "Platform/Unix/System.cpp"
    )
else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

#---------------------------------------------------------------------------------------------------
# ArenaCoreGui

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_library("ArenaCoreGui" STATIC "Platform/Windows/Gui/System.cpp")

    target_link_libraries("ArenaCoreGui"
        PUBLIC
            "ArenaCore"
        PRIVATE
            "ArenaCompilerOptions"
    )
else()
    add_library("ArenaCoreGui" ALIAS "ArenaCore")
endif()
