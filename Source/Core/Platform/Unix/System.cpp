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

#include <err.h>
#include <stdlib.h>

#include <Core/System.h>

using namespace ArenaBuilder;

void System::ExitWithErrorMessage(const oschar_t* message)
{
    errx(EXIT_FAILURE, "%s", message);
}

void System::ExitWithErrorDialog(const oschar_t*)
{
    exit(EXIT_FAILURE);
}
