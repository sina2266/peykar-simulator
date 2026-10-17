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

#include "parentedentity_p.h"

namespace libcellml {

const ParentedEntity::ParentedEntityImpl *ParentedEntity::pFunc() const
{
    return reinterpret_cast<ParentedEntity::ParentedEntityImpl const *>(Entity::pFunc());
}

ParentedEntity::ParentedEntity(ParentedEntity::ParentedEntityImpl *pImpl)
    : Entity(pImpl)
{
}

ParentedEntityPtr ParentedEntity::parent() const
{
    return pFunc()->mParent.lock();
}

void ParentedEntity::ParentedEntityImpl::removeParent()
{
    mParent = {};
}

bool ParentedEntity::hasParent() const
{
    bool hasParent = false;
    EntityPtr parent = pFunc()->mParent.lock();
    if (parent) {
        hasParent = true;
    }

    return hasParent;
}

bool ParentedEntity::hasAncestor(const ParentedEntityPtr &entity) const
{
    bool hasAncestor = false;
    ParentedEntityPtr parent = pFunc()->mParent.lock();
    if (parent == entity) {
        hasAncestor = true;
    } else if (parent) {
        hasAncestor = parent->hasAncestor(entity);
    }

    return hasAncestor;
}

void ParentedEntity::ParentedEntityImpl::setParent(const ParentedEntityPtr &parent)
{
    mParent = parent;
}

} // namespace libcellml
