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

#include "libcellml/component.h"

#include <algorithm>
#include <iterator>
#include <string>
#include <vector>

#include "libcellml/importsource.h"
#include "libcellml/model.h"
#include "libcellml/reset.h"
#include "libcellml/variable.h"

#include "commonutils.h"
#include "component_p.h"
#include "reset_p.h"
#include "utilities.h"
#include "variable_p.h"

namespace libcellml {

std::vector<VariablePtr>::const_iterator Component::ComponentImpl::findVariable(const std::string &name) const
{
    return std::find_if(mVariables.begin(), mVariables.end(),
                        [=](const VariablePtr &v) -> bool { return v->name() == name; });
}

std::vector<VariablePtr>::const_iterator Component::ComponentImpl::findVariable(const VariablePtr &variable) const
{
    return std::find_if(mVariables.begin(), mVariables.end(),
                        [=](const VariablePtr &v) -> bool { return v->equals(variable); });
}

std::vector<ResetPtr>::const_iterator Component::ComponentImpl::findReset(const ResetPtr &reset) const
{
    return std::find_if(mResets.begin(), mResets.end(),
                        [=](const ResetPtr &r) -> bool { return r->equals(reset); });
}

bool Component::ComponentImpl::equalVariables(const ComponentPtr &other) const
{
    std::vector<EntityPtr> entities;
    std::copy(mVariables.begin(), mVariables.end(), std::back_inserter(entities));
    return equalEntities(other, entities);
}

bool Component::ComponentImpl::equalResets(const ComponentPtr &other) const
{
    std::vector<EntityPtr> entities;
    std::copy(mResets.begin(), mResets.end(), std::back_inserter(entities));
    return equalEntities(other, entities);
}

Component::ComponentImpl *Component::pFunc()
{
    return reinterpret_cast<Component::ComponentImpl *>(Entity::pFunc());
}

const Component::ComponentImpl *Component::pFunc() const
{
    return reinterpret_cast<Component::ComponentImpl const *>(Entity::pFunc());
}

Component::Component()
    : ComponentEntity(new Component::ComponentImpl())
{
    pFunc()->mComponent = this;
}

Component::Component(const std::string &name)
    : ComponentEntity(new Component::ComponentImpl())
{
    pFunc()->mComponent = this;
    setName(name);
}

Component::~Component()
{
    delete pFunc();
}

ComponentPtr Component::create() noexcept
{
    return std::shared_ptr<Component> {new Component {}};
}

ComponentPtr Component::create(const std::string &name) noexcept
{
    return std::shared_ptr<Component> {new Component {name}};
}

bool Component::ComponentImpl::performTestWithHistory(History &history, const ComponentConstPtr &component, TestType type) const
{
    if (mComponent->isImport()) {
        auto model = mComponent->importSource()->model();
        if (model == nullptr) {
            return false;
        }

        auto importedComponent = model->component(mComponent->importReference());
        if (importedComponent == nullptr) {
            return false;
        }

        auto h = createHistoryEpoch(component, importeeModelUrl(history, mComponent->importSource()->url()));
        if (checkForImportCycles(history, h)) {
            return false;
        }

        history.push_back(h);
        bool result = importedComponent->pFunc()->performTestWithHistory(history, importedComponent, type);
        history.pop_back();
        return result;
    }

    auto model = std::dynamic_pointer_cast<libcellml::Model>(mComponent->parent());
    if ((model == nullptr) && (mComponent->parent() != nullptr)) {
        model = owningModel(mComponent->parent());
    }

    auto tmpComponent = mComponent->clone();
    auto units = unitsUsed(model, tmpComponent);
    for (const auto &u : units) {
        if ((type == TestType::RESOLVED) && !u->isResolved()) {
            return false;
        } else if (type == TestType::DEFINED) {
            if (!u->isDefined() || (model == nullptr) || !model->hasUnits(u)) {
                return false;
            }
        }
    }

    for (size_t i = 0; i < mComponent->componentCount(); ++i) {
        auto currentComponent = mComponent->component(i);
        if (!currentComponent->pFunc()->performTestWithHistory(history, currentComponent, type)) {
            return false;
        }
    }

    return true;
}

bool Component::doAddComponent(const ComponentPtr &component)
{
    auto newParent = shared_from_this();
    bool hasParent = component->hasParent();
    if (hasParent) {
        if (hasAncestor(component)) {
            return false;
        }
        auto parent = component->parent();
        if (parent != newParent) {
            removeComponentFromEntity(parent, component);
        }
    } else if (hasAncestor(component)) {
        return false;
    } else if (newParent == component) {
        return false;
    }
    component->pFunc()->setParent(newParent);

    return ComponentEntity::doAddComponent(component);
}

void Component::setSourceComponent(ImportSourcePtr &importSource, const std::string &name)
{
    setImportSource(importSource);
    setImportReference(name);
}

void Component::appendMath(const std::string &math)
{
    pFunc()->mMath.append(math);
}

std::string Component::math() const
{
    return pFunc()->mMath;
}

void Component::setMath(const std::string &math)
{
    pFunc()->mMath = math;
}

void Component::removeMath()
{
    pFunc()->mMath.clear();
}

bool Component::addVariable(const VariablePtr &variable)
{
    // Prevent adding null pointer.
    if (variable == nullptr) {
        return false;
    }

    // Prevent adding to multiple components.
    auto thisComponent = shared_from_this();
    if (variable->hasParent() && (thisComponent != variable->parent())) {
        auto otherParent = std::dynamic_pointer_cast<Component>(variable->parent());
        otherParent->removeVariable(variable);
    }

    variable->pFunc()->setParent(thisComponent);
    pFunc()->mVariables.push_back(variable);
    return true;
}

bool Component::removeVariable(size_t index)
{
    if (index < pFunc()->mVariables.size()) {
        auto variable = pFunc()->mVariables[index];
        pFunc()->mVariables.erase(pFunc()->mVariables.begin() + ptrdiff_t(index));
        variable->pFunc()->removeParent();
        return true;
    }

    return false;
}

bool Component::removeVariable(const std::string &name)
{
    auto result = pFunc()->findVariable(name);
    if (result != pFunc()->mVariables.end()) {
        (*result)->pFunc()->removeParent();
        pFunc()->mVariables.erase(result);
        return true;
    }

    return false;
}

bool Component::removeVariable(const VariablePtr &variable)
{
    auto result = pFunc()->findVariable(variable);
    if (result != pFunc()->mVariables.end()) {
        pFunc()->mVariables.erase(result);
        variable->pFunc()->removeParent();
        return true;
    }

    return false;
}

void Component::removeAllVariables()
{
    for (const auto &variable : pFunc()->mVariables) {
        variable->pFunc()->removeParent();
    }
    pFunc()->mVariables.clear();
}

VariablePtr Component::variable(size_t index) const
{
    if (index < pFunc()->mVariables.size()) {
        return pFunc()->mVariables.at(index);
    }

    return nullptr;
}

VariablePtr Component::variable(const std::string &name) const
{
    auto result = pFunc()->findVariable(name);
    if (result != pFunc()->mVariables.end()) {
        return *result;
    }

    return nullptr;
}

VariablePtr Component::takeVariable(size_t index)
{
    VariablePtr variable = nullptr;
    if (index < pFunc()->mVariables.size()) {
        variable = pFunc()->mVariables.at(index);
        removeVariable(index);
    }

    return variable;
}

VariablePtr Component::takeVariable(const std::string &name)
{
    VariablePtr res = variable(name);
    removeVariable(name);

    return res;
}

size_t Component::variableCount() const
{
    return pFunc()->mVariables.size();
}

bool Component::hasVariable(const VariablePtr &variable) const
{
    return pFunc()->findVariable(variable) != pFunc()->mVariables.end();
}

bool Component::hasVariable(const std::string &name) const
{
    return pFunc()->findVariable(name) != pFunc()->mVariables.end();
}

bool Component::addReset(const ResetPtr &reset)
{
    // Prevent adding null pointer.
    if (reset == nullptr) {
        return false;
    }

    // Prevent adding to multiple components.
    auto thisComponent = shared_from_this();
    if (reset->hasParent() && (thisComponent != reset->parent())) {
        auto otherParent = std::dynamic_pointer_cast<Component>(reset->parent());
        otherParent->removeReset(reset);
    }
    reset->pFunc()->setParent(thisComponent);
    pFunc()->mResets.push_back(reset);
    return true;
}

bool Component::removeReset(size_t index)
{
    if (index < pFunc()->mResets.size()) {
        pFunc()->mResets.at(index)->pFunc()->removeParent();
        pFunc()->mResets.erase(pFunc()->mResets.begin() + ptrdiff_t(index));
        return true;
    }
    return false;
}

bool Component::removeReset(const ResetPtr &reset)
{
    auto result = pFunc()->findReset(reset);
    if (result != pFunc()->mResets.end()) {
        (*result)->pFunc()->removeParent();
        pFunc()->mResets.erase(result);
        return true;
    }
    return false;
}

void Component::removeAllResets()
{
    for (const auto &reset : pFunc()->mResets) {
        reset->pFunc()->removeParent();
    }
    pFunc()->mResets.clear();
}

ResetPtr Component::takeReset(size_t index)
{
    ResetPtr reset = nullptr;
    if (index < pFunc()->mResets.size()) {
        reset = pFunc()->mResets.at(index);
        removeReset(index);
        reset->pFunc()->removeParent();
    }

    return reset;
}

ResetPtr Component::reset(size_t index) const
{
    if (index < pFunc()->mResets.size()) {
        return pFunc()->mResets.at(index);
    }

    return nullptr;
}

size_t Component::resetCount() const
{
    return pFunc()->mResets.size();
}

bool Component::hasReset(const ResetPtr &reset) const
{
    return pFunc()->findReset(reset) != pFunc()->mResets.end();
}

ComponentPtr Component::clone() const
{
    auto c = create();

    c->setId(id());
    c->setName(name());
    c->setMath(math());

    if (isImport()) {
        c->setImportSource(importSource());
    }

    c->setImportReference(importReference());

    for (size_t index = 0; index < variableCount(); ++index) {
        auto v = variable(index);
        c->addVariable(v->clone());
    }

    for (size_t index = 0; index < resetCount(); ++index) {
        auto r = reset(index);
        auto rClone = r->clone();
        c->addReset(rClone);
        size_t variableIndex = indexOf(r->variable(), shared_from_this());
        if (variableIndex < variableCount()) {
            auto v = c->variable(variableIndex);
            rClone->setVariable(v);
        }
        size_t testVariableIndex = indexOf(r->testVariable(), shared_from_this());
        if (testVariableIndex < variableCount()) {
            auto v = c->variable(testVariableIndex);
            rClone->setTestVariable(v);
        }
    }

    for (size_t index = 0; index < componentCount(); ++index) {
        auto cChild = component(index);
        c->addComponent(cChild->clone());
    }

    return c;
}

bool doRequiresImport(const ComponentConstPtr &thisComponent)
{
    if (thisComponent->isImport()) {
        return true;
    }
    for (size_t c = 0; c < thisComponent->componentCount(); ++c) {
        if (doRequiresImport(thisComponent->component(c))) {
            return true;
        }
    }
    return false;
}

bool Component::requiresImports() const
{
    return doRequiresImport(shared_from_this());
}

bool Component::isDefined() const
{
    History history;
    return pFunc()->performTestWithHistory(history, shared_from_this(), TestType::DEFINED);
}

bool Component::doIsResolved() const
{
    History history;
    return pFunc()->performTestWithHistory(history, shared_from_this(), TestType::RESOLVED);
}

bool Component::doEquals(const EntityPtr &other) const
{
    if (ComponentEntity::doEquals(other)) {
        auto component = std::dynamic_pointer_cast<Component>(other);
        return (component != nullptr) && areEqual(pFunc()->mMath, component->math())
               && pFunc()->equalResets(component) && pFunc()->equalVariables(component)
               && ImportedEntity::doEquals(component);
    }
    return false;
}

} // namespace libcellml
