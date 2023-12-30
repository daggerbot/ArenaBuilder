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
# Set up Python venv (virtual environment)

find_package("Python" "3.11.2...<4" REQUIRED)

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(EXE_SUFFIX ".exe")
else()
    set(EXE_SUFFIX "")
endif()

set(VENV_DIR "${CMAKE_CURRENT_BINARY_DIR}/venv")
set(VENV_BINDIR "${VENV_DIR}/bin")
set(VENV_PYTHON "${VENV_BINDIR}/python3${EXE_SUFFIX}")
set(VENV_PIP "${VENV_BINDIR}/pip3${EXE_SUFFIX}")

add_custom_command(
    OUTPUT "${VENV_PYTHON}" "${VENV_PIP}"
    COMMAND "${Python_EXECUTABLE}" "-m" "venv" "${VENV_DIR}")

add_custom_target("venv" DEPENDS "${VENV_PYTHON}" "${VENV_PIP}")

#---------------------------------------------------------------------------------------------------
# Build components

add_subdirectory("Source")
add_subdirectory("Assets")

#---------------------------------------------------------------------------------------------------
# Test run

set(RUN_COMMAND "" CACHE STRING "Command used to run the game executable (e.g. wine)")
set(DEBUG_COMMAND "gdb" "--args" CACHE STRING "Command used to debug the game (e.g. gdb --args)")

set(RUN_ARGS
    "--data-dir=${BINDIR}"
)

add_custom_target("run"
    COMMAND ${RUN_COMMAND} "$<TARGET_FILE:ArenaClient>" ${RUN_ARGS}
    DEPENDS "ArenaClient"
    USES_TERMINAL)

add_custom_target("debug"
    COMMAND ${DEBUG_COMMAND} "$<TARGET_FILE:ArenaClient>" ${RUN_ARGS}
    DEPENDS "ArenaClient"
    USES_TERMINAL)
