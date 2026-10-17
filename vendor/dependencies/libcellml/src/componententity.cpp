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

#include "libcellml/componententity.h"

#include <algorithm>
#include <memory>
#include <vector>

#include "libcellml/component.h"

#include "component_p.h"

namespace libcellml {

std::vector<ComponentPtr>::const_iterator ComponentEntity::ComponentEntityImpl::findComponent(const std::string &name) const
{
    return std::find_if(mComponents.begin(), mComponents.end(),
                        [=](const ComponentPtr &c) -> bool { return c->name() == name; });
}

std::vector<ComponentPtr>::const_iterator ComponentEntity::ComponentEntityImpl::findComponent(const ComponentPtr &component) const
{
    return std::find_if(mComponents.begin(), mComponents.end(),
                        [=](const ComponentPtr &c) -> bool { return c->equals(component); });
}

ComponentEntity::ComponentEntityImpl *ComponentEntity::pFunc()
{
    return reinterpret_cast<ComponentEntity::ComponentEntityImpl *>(Entity::pFunc());
}

const ComponentEntity::ComponentEntityImpl *ComponentEntity::pFunc() const
{
    return reinterpret_cast<ComponentEntity::ComponentEntityImpl const *>(Entity::pFunc());
}

ComponentEntity::ComponentEntity(ComponentEntityImpl *pImpl)
    : NamedEntity(pImpl)
{
}

bool ComponentEntity::addComponent(const ComponentPtr &component)
{
    if (component == nullptr) {
        return false;
    }

    return doAddComponent(component);
}

bool ComponentEntity::doAddComponent(const ComponentPtr &component)
{
    pFunc()->mComponents.push_back(component);
    return true;
}

bool ComponentEntity::removeComponent(const std::string &name, bool searchEncapsulated)
{
    bool status = false;
    auto result = pFunc()->findComponent(name);
    if (result != pFunc()->mComponents.end()) {
        (*result)->pFunc()->removeParent();
        pFunc()->mComponents.erase(result);
        status = true;
    } else if (searchEncapsulated) {
        for (size_t i = 0; i < componentCount() && !status; ++i) {
            status = component(i)->removeComponent(name, searchEncapsulated);
        }
    }

    return status;
}

bool ComponentEntity::removeComponent(size_t index)
{
    bool status = false;
    if (index < pFunc()->mComponents.size()) {
        auto component = pFunc()->mComponents[index];
        pFunc()->mComponents.erase(pFunc()->mComponents.begin() + ptrdiff_t(index));
        component->pFunc()->removeParent();
        status = true;
    }

    return status;
}

bool ComponentEntity::removeComponent(const ComponentPtr &component, bool searchEncapsulated)
{
    bool status = false;
    auto result = pFunc()->findComponent(component);
    if (result != pFunc()->mComponents.end()) {
        component->pFunc()->removeParent();
        pFunc()->mComponents.erase(result);
        status = true;
    } else if (searchEncapsulated) {
        for (size_t i = 0; i < componentCount() && !status; ++i) {
            status = ComponentEntity::component(i)->removeComponent(component, searchEncapsulated);
        }
    }

    return status;
}

void ComponentEntity::removeAllComponents()
{
    for (auto &component : pFunc()->mComponents) {
        component->pFunc()->removeParent();
    }
    pFunc()->mComponents.clear();
}

size_t ComponentEntity::componentCount() const
{
    return pFunc()->mComponents.size();
}

bool ComponentEntity::containsComponent(const std::string &name, bool searchEncapsulated) const
{
    bool status = false;
    auto result = pFunc()->findComponent(name);
    if (result != pFunc()->mComponents.end()) {
        status = true;
    } else if (searchEncapsulated) {
        for (size_t i = 0; i < componentCount() && !status; ++i) {
            status = component(i)->containsComponent(name, searchEncapsulated);
        }
    }

    return status;
}

bool ComponentEntity::containsComponent(const ComponentPtr &component, bool searchEncapsulated) const
{
    bool status = false;
    auto result = pFunc()->findComponent(component);
    if (result != pFunc()->mComponents.end()) {
        status = true;
    } else if (searchEncapsulated) {
        for (size_t i = 0; i < componentCount() && !status; ++i) {
            status = ComponentEntity::component(i)->containsComponent(component, searchEncapsulated);
        }
    }

    return status;
}

ComponentPtr ComponentEntity::component(size_t index) const
{
    ComponentPtr component = nullptr;
    if (index < pFunc()->mComponents.size()) {
        component = pFunc()->mComponents.at(index);
    }

    return component;
}

ComponentPtr ComponentEntity::component(const std::string &name, bool searchEncapsulated) const
{
    ComponentPtr foundComponent = nullptr;
    auto result = pFunc()->findComponent(name);
    if (result != pFunc()->mComponents.end()) {
        foundComponent = *result;
    } else if (searchEncapsulated) {
        for (size_t i = 0; i < componentCount() && !foundComponent; ++i) {
            foundComponent = component(i)->component(name, searchEncapsulated);
        }
    }

    return foundComponent;
}

ComponentPtr ComponentEntity::takeComponent(size_t index)
{
    ComponentPtr component = nullptr;
    if (index < pFunc()->mComponents.size()) {
        component = pFunc()->mComponents.at(index);
        pFunc()->mComponents.erase(pFunc()->mComponents.begin() + ptrdiff_t(index));
        component->pFunc()->removeParent();
    }

    return component;
}

ComponentPtr ComponentEntity::takeComponent(const std::string &name, bool searchEncapsulated)
{
    ComponentPtr foundComponent = nullptr;
    auto result = pFunc()->findComponent(name);
    if (result != pFunc()->mComponents.end()) {
        foundComponent = *result;
        pFunc()->mComponents.erase(result);
        foundComponent->pFunc()->removeParent();
    } else if (searchEncapsulated) {
        for (size_t i = 0; i < componentCount() && !foundComponent; ++i) {
            foundComponent = component(i)->takeComponent(name, searchEncapsulated);
        }
    }

    return foundComponent;
}

bool ComponentEntity::replaceComponent(size_t index, const ComponentPtr &newComponent)
{
    bool status = false;
    auto oldComponent = component(index);
    ParentedEntityPtr parent = nullptr;
    if (oldComponent != nullptr) {
        parent = oldComponent->parent();
    }

    if (removeComponent(index)) {
        pFunc()->mComponents.insert(pFunc()->mComponents.begin() + ptrdiff_t(index), newComponent);
        newComponent->pFunc()->setParent(parent);
        status = true;
    }

    return status;
}

bool ComponentEntity::replaceComponent(const std::string &name, const ComponentPtr &component, bool searchEncapsulated)
{
    bool status = replaceComponent(size_t(pFunc()->findComponent(name) - pFunc()->mComponents.begin()), component);
    if (searchEncapsulated && !status) {
        for (size_t i = 0; i < componentCount() && !status; ++i) {
            status = ComponentEntity::component(i)->replaceComponent(name, component, searchEncapsulated);
        }
    }

    return status;
}

bool ComponentEntity::replaceComponent(const ComponentPtr &oldComponent, const ComponentPtr &newComponent, bool searchEncapsulated)
{
    bool status = replaceComponent(size_t(pFunc()->findComponent(oldComponent) - pFunc()->mComponents.begin()), newComponent);

    if (searchEncapsulated && !status) {
        for (size_t i = 0; i < componentCount() && !status; ++i) {
            status = component(i)->replaceComponent(oldComponent, newComponent, searchEncapsulated);
        }
    }

    return status;
}

void ComponentEntity::setEncapsulationId(const std::string &id)
{
    pFunc()->mEncapsulationId = id;
}

std::string ComponentEntity::encapsulationId() const
{
    return pFunc()->mEncapsulationId;
}

void ComponentEntity::removeEncapsulationId()
{
    pFunc()->mEncapsulationId = "";
}

bool ComponentEntity::doEquals(const EntityPtr &other) const
{
    if (NamedEntity::doEquals(other)) {
        auto componentEntity = std::dynamic_pointer_cast<ComponentEntity>(other);
        if ((componentEntity != nullptr)
            && pFunc()->mEncapsulationId == componentEntity->encapsulationId()
            && pFunc()->mComponents.size() == componentEntity->componentCount()) {
            for (const auto &component : pFunc()->mComponents) {
                if (!componentEntity->containsComponent(component, false)) {
                    return false;
                }
            }
            return true;
        }
    }

    return false;
}

} // namespace libcellml
