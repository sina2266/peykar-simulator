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

#include "libcellml/namedentity.h"

#include "parentedentity_p.h"

namespace libcellml {

/**
 * @brief The NamedEntity::NamedEntityImpl class.
 *
 * The private implementation for the NamedEntity class.
 */
class NamedEntity::NamedEntityImpl: public ParentedEntityImpl
{
public:
    std::string mName; /**< Entity name represented as a std::string. */
};

} // namespace libcellml
