/*
Copyright libCellML Contributors

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#pragma once

#include <vector>

#include "libcellml/componententity.h"

#include "namedentity_p.h"

namespace libcellml {

/**
 * @brief The ComponentEntity::ComponentEntityImpl class.
 *
 * This class is the private implementation struct for the ComponentEntity class.  Separating
 * the implementation from the definition allows for greater flexibility when
 * distributing the code.
 */
class ComponentEntity::ComponentEntityImpl: public NamedEntityImpl
{
public:
    std::vector<ComponentPtr> mComponents;
    std::string mEncapsulationId;

    std::vector<ComponentPtr>::const_iterator findComponent(const std::string &name) const;
    std::vector<ComponentPtr>::const_iterator findComponent(const ComponentPtr &component) const;
};

} // namespace libcellml
