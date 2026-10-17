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

#include "libcellml/model.h"

#include <algorithm>
#include <iterator>
#include <vector>

#include "libcellml/component.h"
#include "libcellml/importsource.h"
#include "libcellml/units.h"
#include "libcellml/variable.h"

#include "commonutils.h"
#include "component_p.h"
#include "internaltypes.h"
#include "model_p.h"
#include "units_p.h"
#include "utilities.h"

namespace libcellml {

std::vector<UnitsPtr>::const_iterator Model::ModelImpl::findUnits(const std::string &name) const
{
    return std::find_if(mUnits.begin(), mUnits.end(),
                        [=](const UnitsPtr &u) -> bool { return u->name() == name; });
}

std::vector<UnitsPtr>::const_iterator Model::ModelImpl::findUnits(const UnitsPtr &units) const
{
    auto result = std::find(mUnits.begin(), mUnits.end(), units);
    if (result != mUnits.end()) {
        return result;
    }
    return std::find_if(mUnits.begin(), mUnits.end(),
                        [=](const UnitsPtr &u) -> bool { return u->equals(units); });
}

bool Model::ModelImpl::equalUnits(const ModelPtr &other) const
{
    std::vector<EntityPtr> entities;
    std::copy(mUnits.begin(), mUnits.end(), std::back_inserter(entities));
    return equalEntities(other, entities);
}

Model::ModelImpl *Model::pFunc()
{
    return reinterpret_cast<Model::ModelImpl *>(Entity::pFunc());
}

const Model::ModelImpl *Model::pFunc() const
{
    return reinterpret_cast<Model::ModelImpl const *>(Entity::pFunc());
}

Model::Model()
    : ComponentEntity(new Model::ModelImpl())
{
}

Model::Model(const std::string &name)
    : ComponentEntity(new Model::ModelImpl())
{
    setName(name);
}

Model::~Model()
{
    delete pFunc();
}
ModelPtr Model::create() noexcept
{
    return std::shared_ptr<Model> {new Model {}};
}

ModelPtr Model::create(const std::string &name) noexcept
{
    return std::shared_ptr<Model> {new Model {name}};
}

bool Model::doAddComponent(const ComponentPtr &component)
{
    auto thisModel = shared_from_this();
    if (component->hasParent() && (thisModel != component->parent())) {
        removeComponentFromEntity(component->parent(), component);
    }
    component->pFunc()->setParent(thisModel);

    return ComponentEntity::doAddComponent(component);
}

bool Model::addUnits(const UnitsPtr &units)
{
    if (units == nullptr) {
        return false;
    }

    // Prevent adding to multiple models: move units to this model.
    auto thisModel = shared_from_this();
    if (units->hasParent() && (units->parent() != thisModel)) {
        auto otherParent = std::dynamic_pointer_cast<Model>(units->parent());
        otherParent->removeUnits(units);
    }
    pFunc()->mUnits.push_back(units);
    units->pFunc()->setParent(thisModel);

    return true;
}

bool Model::removeUnits(size_t index)
{
    bool status = false;
    if (index < pFunc()->mUnits.size()) {
        auto result = pFunc()->mUnits.begin() + ptrdiff_t(index);
        (*result)->pFunc()->removeParent();
        pFunc()->mUnits.erase(result);
        status = true;
    }

    return status;
}

bool Model::removeUnits(const std::string &name)
{
    bool status = false;
    auto result = pFunc()->findUnits(name);
    if (result != pFunc()->mUnits.end()) {
        (*result)->pFunc()->removeParent();
        pFunc()->mUnits.erase(result);
        status = true;
    }

    return status;
}

bool Model::removeUnits(const UnitsPtr &units)
{
    bool status = false;
    auto result = pFunc()->findUnits(units);
    if (result != pFunc()->mUnits.end()) {
        units->pFunc()->removeParent();
        pFunc()->mUnits.erase(result);
        status = true;
    }

    return status;
}

void Model::removeAllUnits()
{
    for (const auto &u : pFunc()->mUnits) {
        u->pFunc()->removeParent();
    }
    pFunc()->mUnits.clear();
}

bool Model::hasUnits(const std::string &name) const
{
    return pFunc()->findUnits(name) != pFunc()->mUnits.end();
}

bool Model::hasUnits(const UnitsPtr &units) const
{
    return pFunc()->findUnits(units) != pFunc()->mUnits.end();
}

UnitsPtr Model::units(size_t index) const
{
    UnitsPtr units = nullptr;
    if (index < pFunc()->mUnits.size()) {
        units = pFunc()->mUnits.at(index);
    }

    return units;
}

UnitsPtr Model::units(const std::string &name) const
{
    UnitsPtr units = nullptr;
    auto result = pFunc()->findUnits(name);
    if (result != pFunc()->mUnits.end()) {
        units = *result;
    }

    return units;
}

UnitsPtr Model::takeUnits(size_t index)
{
    UnitsPtr units = nullptr;
    if (index < pFunc()->mUnits.size()) {
        units = pFunc()->mUnits.at(index);
        removeUnits(index);
        units->pFunc()->removeParent();
    }

    return units;
}

UnitsPtr Model::takeUnits(const std::string &name)
{
    return takeUnits(size_t(pFunc()->findUnits(name) - pFunc()->mUnits.begin()));
}

bool Model::replaceUnits(size_t index, const UnitsPtr &units)
{
    bool status = false;
    if (removeUnits(index)) {
        pFunc()->mUnits.insert(pFunc()->mUnits.begin() + ptrdiff_t(index), units);
        units->pFunc()->setParent(shared_from_this());
        status = true;
    }

    return status;
}

bool Model::replaceUnits(const std::string &name, const UnitsPtr &units)
{
    return replaceUnits(size_t(pFunc()->findUnits(name) - pFunc()->mUnits.begin()), units);
}

bool Model::replaceUnits(const UnitsPtr &oldUnits, const UnitsPtr &newUnits)
{
    return replaceUnits(size_t(pFunc()->findUnits(oldUnits) - pFunc()->mUnits.begin()), newUnits);
}

size_t Model::unitsCount() const
{
    return pFunc()->mUnits.size();
}

bool Model::linkUnits()
{
    return traverseComponentEntityTreeLinkingUnits(shared_from_this());
}

bool traverseComponentTreeForUnlinkedUnits(const ComponentPtr &component)
{
    bool unlinkedUnits = areComponentVariableUnitsUnlinked(component);
    for (size_t index = 0; index < component->componentCount() && !unlinkedUnits; ++index) {
        auto c = component->component(index);
        unlinkedUnits = traverseComponentTreeForUnlinkedUnits(c);
    }
    return unlinkedUnits;
}

bool Model::hasUnlinkedUnits()
{
    bool unlinkedUnits = false;
    for (size_t index = 0; index < componentCount() && !unlinkedUnits; ++index) {
        auto c = component(index);
        unlinkedUnits = traverseComponentTreeForUnlinkedUnits(c);
    }
    return unlinkedUnits;
}

bool Model::isDefined() const
{
    bool defined = true;
    for (size_t index = 0; (index < unitsCount()) && defined; ++index) {
        defined = units(index)->isDefined();
    }
    for (size_t index = 0; (index < componentCount()) && defined; ++index) {
        defined = component(index)->isDefined();
    }
    return defined;
}

bool Model::hasUnresolvedImports() const
{
    bool unresolvedImports = false;
    for (size_t index = 0; (index < unitsCount()) && !unresolvedImports; ++index) {
        unresolvedImports = !units(index)->isResolved();
    }
    for (size_t index = 0; (index < componentCount()) && !unresolvedImports; ++index) {
        unresolvedImports = !component(index)->isResolved();
    }

    return unresolvedImports;
}

bool hasComponentImports(const ComponentEntityConstPtr &componentEntity)
{
    bool importsPresent = false;
    for (size_t index = 0; (index < componentEntity->componentCount()) && !importsPresent; ++index) {
        libcellml::ComponentPtr childComponent = componentEntity->component(index);
        importsPresent = childComponent->isImport();
        if (!importsPresent) {
            importsPresent = hasComponentImports(childComponent);
        }
    }
    return importsPresent;
}

bool hasUnitsImports(const UnitsPtr &units)
{
    bool importPresent = units->isImport();
    auto model = owningModel(units);
    size_t unistCount = units->unitCount();
    for (size_t index = 0; !importPresent && (index < unistCount); ++index) {
        std::string reference = units->unitAttributeReference(index);
        if (!reference.empty() && !isStandardUnitName(reference)) {
            if (model->hasUnits(reference)) {
                importPresent = hasUnitsImports(model->units(reference));
            }
        }
    }
    return importPresent;
}

bool Model::hasImports() const
{
    bool importsPresent = false;
    for (size_t index = 0; (index < unitsCount()) && !importsPresent; ++index) {
        libcellml::UnitsPtr units = Model::units(index);
        importsPresent = hasUnitsImports(units);
    }

    if (!importsPresent) {
        importsPresent = hasComponentImports(shared_from_this());
    }

    return importsPresent;
}

void fixComponentUnits(const ModelPtr &model, const ComponentPtr &component)
{
    for (size_t v = 0; v < component->variableCount(); ++v) {
        auto variable = component->variable(v);
        if (variable->units() != nullptr) {
            // Find the units in the model and switch out.
            auto units = model->units(variable->units()->name());
            if (units != nullptr) {
                variable->setUnits(units);
            }
        }
    }
    for (size_t c = 0; c < component->componentCount(); ++c) {
        fixComponentUnits(model, component->component(c));
    }
}

ModelPtr Model::clone() const
{
    auto m = create();

    m->setId(id());
    m->setName(name());

    m->setEncapsulationId(encapsulationId());

    for (size_t index = 0; index < pFunc()->mUnits.size(); ++index) {
        m->addUnits(units(index)->clone());
    }

    for (size_t index = 0; index < componentCount(); ++index) {
        m->addComponent(component(index)->clone());
    }

    for (size_t index = 0; index < m->componentCount(); ++index) {
        fixComponentUnits(m, m->component(index));
    }

    // Generate equivalence map starting from the models components.
    EquivalenceMap map;
    IndexStack indexStack;
    for (size_t index = 0; index < componentCount(); ++index) {
        indexStack.push_back(index);
        auto c = component(index);
        recordVariableEquivalences(c, map, indexStack);
        generateEquivalenceMap(c, map, indexStack);
        indexStack.pop_back();
    }
    applyEquivalenceMapToModel(map, m);

    return m;
}

bool Model::fixVariableInterfaces()
{
    VariablePtrs variables;

    for (size_t index = 0; index < componentCount(); ++index) {
        findAllVariablesWithEquivalences(component(index), variables);
    }

    bool allOk = true;
    for (const auto &variable : variables) {
        Variable::InterfaceType interfaceType = determineInterfaceType(variable);
        if (interfaceType == Variable::InterfaceType::NONE) {
            allOk = false;
        } else if (!variable->permitsInterfaceType(interfaceType)) {
            variable->setInterfaceType(interfaceType);
        }
    }

    return allOk;
}

bool traverseHierarchyAndRemoveIfEmpty(const ComponentPtr &component)
{
    for (size_t i = component->componentCount() - 1; i != MAX_SIZE_T; --i) {
        if (traverseHierarchyAndRemoveIfEmpty(component->component(i))) {
            component->removeComponent(i);
        }
    }

    return (component->variableCount() + component->resetCount() + component->componentCount() == 0)
           && component->math().empty()
           && !component->isImport()
           && component->name().empty()
           && component->id().empty();
}

void Model::clean()
{
    // Remove empty components.
    for (size_t i = componentCount() - 1; i != MAX_SIZE_T; --i) {
        if (traverseHierarchyAndRemoveIfEmpty(component(i))) {
            removeComponent(i);
        }
    }

    // Remove empty units.
    for (size_t i = unitsCount() - 1; i != MAX_SIZE_T; --i) {
        auto u = units(i);
        if (!u->isImport()
            && u->name().empty()
            && u->id().empty()
            && (u->unitCount() == 0)) {
            removeUnits(i);
        }
    }
}

bool Model::doEquals(const EntityPtr &other) const
{
    if (ComponentEntity::doEquals(other)) {
        auto model = std::dynamic_pointer_cast<Model>(other);
        if ((model != nullptr) && pFunc()->equalUnits(model)) {
            return true;
        }
    }
    return false;
}

std::vector<std::string> Model::importRequirements() const
{
    std::vector<std::string> requirements;

    auto importedComponents = getImportedComponents(shared_from_this());
    auto importedUnits = getImportedUnits(shared_from_this());

    for (auto &component : importedComponents) {
        auto url = component->importSource()->url();
        if (std::find(requirements.begin(), requirements.end(), url) == requirements.end()) {
            requirements.push_back(url);
        }
    }
    for (auto &units : importedUnits) {
        auto url = units->importSource()->url();
        if (std::find(requirements.begin(), requirements.end(), url) == requirements.end()) {
            requirements.push_back(url);
        }
    }

    return requirements;
}

} // namespace libcellml
