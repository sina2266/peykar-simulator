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

#include "libcellml/parentedentity.h"

#include "entity_p.h"

namespace libcellml {

using ParentedEntityWeakPtr = std::weak_ptr<ParentedEntity>; /**< Type definition for weak parented entity pointer. */

/**
 * @brief The ParentedEntity::ParentedEntityImpl class.
 *
 * The private implementation for the ParentedEntity class.
 */
class ParentedEntity::ParentedEntityImpl: public EntityImpl
{
public:
    /**
     * @brief Sets the given entity as the parent of this entity.
     *
     * Set the parent of the entity to the entity given.
     *
     * @param parent An @c Entity.
     */
    void setParent(const ParentedEntityPtr &parent);

    /**
     * @brief Clear the pointer to the parent entity.
     *
     * Clears the pointer to the parent entity.
     */
    void removeParent();

    ParentedEntityWeakPtr mParent; /**< Pointer to parent. */
};

} // namespace libcellml
