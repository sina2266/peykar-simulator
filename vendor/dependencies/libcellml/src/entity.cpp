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

#include "libcellml/entity.h"

#include "entity_p.h"

namespace libcellml {

using EntityWeakPtr = std::weak_ptr<Entity>; /**< Type definition for weak entity pointer. */

Entity::Entity(Entity::EntityImpl *derivedPimpl)
    : mPimpl(derivedPimpl)
{
}

Entity::~Entity() = default;

void Entity::setId(const std::string &id)
{
    pFunc()->mId = id;
}

std::string Entity::id() const
{
    return pFunc()->mId;
}

void Entity::removeId()
{
    pFunc()->mId = "";
}

bool Entity::equals(const EntityPtr &other) const
{
    return doEquals(other);
}

bool Entity::doEquals(const EntityPtr &other) const
{
    if (other == nullptr) {
        return false;
    }

    return pFunc()->mId == other->id();
}

} // namespace libcellml
