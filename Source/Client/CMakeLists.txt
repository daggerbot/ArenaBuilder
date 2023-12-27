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

find_package("SDL2" "2.26.5" REQUIRED)

add_executable("ArenaClient" WIN32
    "Client.cpp"
    "Main.cpp"
    "RenderWindow.cpp"
)

set_target_properties("ArenaClient" PROPERTIES
    OUTPUT_NAME "ArenaBuilder"
)

target_link_libraries("ArenaClient"
    PRIVATE
        "ArenaCompilerOptions"
        "ArenaCore"
        "ArenaGame"
        "SDL2::SDL2"
)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_sources("ArenaClient"
        PRIVATE
            "Platform/Windows/System.cpp"
    )
endif()
