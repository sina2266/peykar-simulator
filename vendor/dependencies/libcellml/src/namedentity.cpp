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

#include "libcellml/namedentity.h"

#include "namedentity_p.h"

namespace libcellml {

NamedEntity::NamedEntityImpl *NamedEntity::pFunc()
{
    return reinterpret_cast<NamedEntity::NamedEntityImpl *>(Entity::pFunc());
}

const NamedEntity::NamedEntityImpl *NamedEntity::pFunc() const
{
    return reinterpret_cast<NamedEntity::NamedEntityImpl const *>(Entity::pFunc());
}

NamedEntity::NamedEntity(NamedEntity::NamedEntityImpl *pImpl)
    : ParentedEntity(pImpl)
{
}

void NamedEntity::setName(const std::string &name)
{
    pFunc()->mName = name;
}

std::string NamedEntity::name() const
{
    return pFunc()->mName;
}

void NamedEntity::removeName()
{
    pFunc()->mName = "";
}

bool NamedEntity::doEquals(const EntityPtr &other) const
{
    if (Entity::doEquals(other)) {
        auto namedEntity = std::dynamic_pointer_cast<NamedEntity>(other);
        if (namedEntity != nullptr) {
            return pFunc()->mName == namedEntity->name();
        }
    }
    return false;
}

} // namespace libcellml
