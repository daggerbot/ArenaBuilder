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

set(GL_VERSION "3.3")

string(REGEX REPLACE "([0-9]+)\\.([0-9]+)" "\\1" GL_MAJOR_VERSION "${GL_VERSION}")
string(REGEX REPLACE "([0-9]+)\\.([0-9]+)" "\\2" GL_MINOR_VERSION "${GL_VERSION}")

configure_file("GL/Version.h.in" "Include/Render/GL/Version.h" @ONLY)

#---------------------------------------------------------------------------------------------------
# Generate OpenGL API with glad

set(VENV_GLAD "${VENV_BINDIR}/glad${EXE_SUFFIX}")
set(GLAD_DIR "${CMAKE_CURRENT_BINARY_DIR}/glad")
set(GLAD_INCLUDEDIR "${GLAD_DIR}/include")
set(GLAD_HEADER "${GLAD_INCLUDEDIR}/glad/gl.h")
set(GLAD_SRCDIR "${GLAD_DIR}/src")
set(GLAD_SOURCE "${GLAD_SRCDIR}/gl.c")

add_custom_command(
    OUTPUT "${VENV_GLAD}"
    COMMAND "${VENV_PIP}" "install" "glad2==2.0.4"
    DEPENDS "venv")

add_custom_command(
    OUTPUT "${GLAD_HEADER}" "${GLAD_SOURCE}"
    COMMAND "${VENV_GLAD}"
            "--api=gl:core=${GL_VERSION}"
            "--extensions=${CMAKE_CURRENT_SOURCE_DIR}/GL/Extensions.txt"
            "--out-path=${GLAD_DIR}"
            "--quiet"
            "--reproducible"
    DEPENDS "${VENV_GLAD}")

add_library("glad" STATIC "${GLAD_SOURCE}")
target_include_directories("glad" PUBLIC "${GLAD_INCLUDEDIR}")

#---------------------------------------------------------------------------------------------------
# ArenaRender

add_library("ArenaRender" STATIC
    "GL/System.cpp"
)

target_include_directories("ArenaRender"
    PUBLIC
        "Include"
        "${CMAKE_CURRENT_BINARY_DIR}/Include"
)

target_link_libraries("ArenaRender"
    PUBLIC
        "ArenaCore"
    PRIVATE
        "ArenaCompilerOptions"
        "glad"
)
