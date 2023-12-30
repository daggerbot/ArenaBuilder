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

#ifndef ARENABUILDER_CORE_SERVICEPROVIDER_H_INCLUDED
#define ARENABUILDER_CORE_SERVICEPROVIDER_H_INCLUDED

#include <typeinfo>

#include "Types.h"

namespace ArenaBuilder {

    // Interface for getting components without having to expose an entire class.
    class ServiceProvider {
    public:
        virtual ~ServiceProvider() = 0;

        template<typename T>
        T* GetService()
        {
            void* service = GetService(typeid(T));

            if (!service) {
                return nullptr;
            }

            return static_cast<T*>(service);
        }

        template<typename T>
        T& RequireService()
        {
            T* service = GetService<T>();

            if (!service) {
                MissingService(typeid(T).name());
            }

            return *service;
        }

        ServiceProvider& operator=(const ServiceProvider&) = delete;
        ServiceProvider& operator=(ServiceProvider&&) = delete;

    protected:
        // This is really not type safe, so be sure to always static_cast to the specified type
        // before returning, even if it's the same as the requested type. This should catch errors
        // if the inheritance tree changes.
        virtual void* GetService(const std::type_info& type) = 0;

    private:
        [[noreturn]] static void MissingService(const char* name);
    };

    inline ServiceProvider::~ServiceProvider() = default;

} // namespace ArenaBuilder

#endif // ARENABUILDER_CORE_SERVICEPROVIDER_H_INCLUDED
