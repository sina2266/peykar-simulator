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

#include "libcellml/variable.h"

#include <algorithm>
#include <cassert>
#include <map>
#include <vector>

#include "libcellml/units.h"

#include "utilities.h"
#include "variable_p.h"

namespace libcellml {

std::vector<VariableWeakPtr>::const_iterator Variable::VariableImpl::findEquivalentVariable(const VariablePtr &equivalentVariable) const
{
    return std::find_if(mEquivalentVariables.begin(), mEquivalentVariables.end(),
                        [=](const VariableWeakPtr &variableWeak) -> bool { return equivalentVariable == variableWeak.lock(); });
}

std::vector<VariableWeakPtr>::iterator Variable::VariableImpl::findEquivalentVariable(const VariablePtr &equivalentVariable)
{
    return std::find_if(mEquivalentVariables.begin(), mEquivalentVariables.end(),
                        [=](const VariableWeakPtr &variableWeak) -> bool { return equivalentVariable == variableWeak.lock(); });
}

Variable::VariableImpl *Variable::pFunc()
{
    return reinterpret_cast<Variable::VariableImpl *>(Entity::pFunc());
}

const Variable::VariableImpl *Variable::pFunc() const
{
    return reinterpret_cast<Variable::VariableImpl const *>(Entity::pFunc());
}

Variable::Variable()
    : NamedEntity(new Variable::VariableImpl())
{
    pFunc()->mVariable = this;
}

Variable::Variable(const std::string &name)
    : NamedEntity(new Variable::VariableImpl())
{
    pFunc()->mVariable = this;
    setName(name);
}

Variable::~Variable()
{
    delete pFunc();
}

VariablePtr Variable::create() noexcept
{
    return std::shared_ptr<Variable> {new Variable {}};
}

VariablePtr Variable::create(const std::string &name) noexcept
{
    return std::shared_ptr<Variable> {new Variable {name}};
}

bool Variable::doEquals(const EntityPtr &other) const
{
    if (NamedEntity::doEquals(other)) {
        auto variable = std::dynamic_pointer_cast<libcellml::Variable>(other);
        if ((variable != nullptr)
            && pFunc()->mInitialValue == variable->initialValue()
            && pFunc()->mInterfaceType == variable->interfaceType()) {
            if (pFunc()->mUnits != nullptr) {
                return pFunc()->mUnits->equals(variable->units());
            }

            return variable->units() == nullptr;
        }
    }
    return false;
}

bool Variable::addEquivalence(const VariablePtr &variable1, const VariablePtr &variable2)
{
    if ((variable1 != nullptr) && (variable2 != nullptr)) {
        bool canAdd1 = variable1->pFunc()->setEquivalentTo(variable2);
        bool canAdd2 = variable2->pFunc()->setEquivalentTo(variable1);
        if (canAdd1 && !canAdd2) {
            // Remove connection from variable1, since it can't be added to variable2.
            variable1->pFunc()->unsetEquivalentTo(variable2);
        }
        return canAdd1 && canAdd2;
    }
    return false;
}

bool Variable::addEquivalence(const VariablePtr &variable1, const VariablePtr &variable2, const std::string &mappingId, const std::string &connectionId)
{
    bool added = Variable::addEquivalence(variable1, variable2);
    variable1->pFunc()->setEquivalentMappingId(variable2, mappingId);
    variable1->pFunc()->setEquivalentConnectionId(variable2, connectionId);
    variable2->pFunc()->setEquivalentMappingId(variable1, mappingId);
    variable2->pFunc()->setEquivalentConnectionId(variable1, connectionId);

    return added;
}

bool Variable::removeEquivalence(const VariablePtr &variable1, const VariablePtr &variable2)
{
    if ((variable1 != nullptr) && (variable2 != nullptr)) {
        if (variable1->pFunc()->unsetEquivalentTo(variable2)) {
            variable2->pFunc()->unsetEquivalentTo(variable1);
            variable1->pFunc()->unsafeResetEquivalenceIds(variable2);
            variable2->pFunc()->unsafeResetEquivalenceIds(variable1);

            return true;
        }
    }

    return false;
}

void Variable::removeAllEquivalences()
{
    auto thisVariable = shared_from_this();
    for (const auto &variable : pFunc()->mEquivalentVariables) {
        auto equivalentVariable = variable.lock();
        if (equivalentVariable != nullptr) {
            equivalentVariable->pFunc()->unsetEquivalentTo(thisVariable);
        }
    }

    pFunc()->mEquivalentVariables.clear();
    pFunc()->mConnectionIdMap.clear();
    pFunc()->mMappingIdMap.clear();
}

VariablePtr Variable::equivalentVariable(size_t index) const
{
    size_t count = 0;
    for (const auto &variableWeak : pFunc()->mEquivalentVariables) {
        auto variable = variableWeak.lock();
        if (variable != nullptr) {
            if (count == index) {
                return variable;
            }
            ++count;
        }
    }

    return nullptr;
}

size_t Variable::equivalentVariableCount() const
{
    size_t count = 0;
    for (const auto &variableWeak : pFunc()->mEquivalentVariables) {
        auto variable = variableWeak.lock();
        if (variable != nullptr) {
            ++count;
        }
    }
    return count;
}

bool Variable::hasEquivalentVariable(const VariablePtr &equivalentVariable, bool considerIndirectEquivalences) const
{
    return pFunc()->hasEquivalentVariable(equivalentVariable, considerIndirectEquivalences);
}

void Variable::VariableImpl::cleanExpiredVariables()
{
    mEquivalentVariables.erase(std::remove_if(mEquivalentVariables.begin(), mEquivalentVariables.end(), [=](const VariableWeakPtr &variableWeak) -> bool { return variableWeak.expired(); }), mEquivalentVariables.end());
}

void Variable::VariableImpl::unsafeResetEquivalenceIds(const VariablePtr &equivalentVariable)
{
    setEquivalentMappingId(equivalentVariable, "");
    setEquivalentConnectionId(equivalentVariable, "");
}

bool Variable::VariableImpl::hasEquivalentVariable(const VariablePtr &equivalentVariable, bool considerIndirectEquivalences) const
{
    bool equivalent = false;
    if (considerIndirectEquivalences) {
        equivalent = hasIndirectEquivalentVariable(equivalentVariable);
    } else {
        auto it = findEquivalentVariable(equivalentVariable);
        if (it == mEquivalentVariables.end()) {
            return false;
        }
        equivalent = !it->expired();
    }

    return equivalent;
}

/**
 * @brief Test if the two variables given are equivalent, directly or indirectly.
 *
 * Traverse the variable equivalence network to determine if the two given variables
 * are equivalent.  Returns true if they are equivalent and false otherwise.
 *
 * @param variable1 The first variable to test.
 * @param variable2 The second variable to test.
 * @param testedVariables Vector of previously tested variables.
 *
 * @return True if the two given variables are equivalent, false otherwise.
 */
bool haveEquivalentVariables(const Variable *variable1,
                             const Variable *variable2,
                             std::vector<const Variable *> &testedVariables)
{
    if (variable1 == variable2) {
        return true;
    }

    if (variable2 == nullptr) {
        return false;
    }

    testedVariables.push_back(variable2);

    for (size_t i = 0; i < variable2->equivalentVariableCount(); ++i) {
        Variable *equivalentVariable2 = variable2->equivalentVariable(i).get();

        if ((std::find(testedVariables.begin(), testedVariables.end(), equivalentVariable2) == testedVariables.end())
            && haveEquivalentVariables(variable1, equivalentVariable2, testedVariables)) {
            return true;
        }
    }

    return false;
}

bool Variable::VariableImpl::hasIndirectEquivalentVariable(const VariablePtr &equivalentVariable) const
{
    if (mVariable == equivalentVariable.get()) {
        return false;
    }

    std::vector<const Variable *> testedVariables;

    return haveEquivalentVariables(mVariable, equivalentVariable.get(), testedVariables);
}

bool Variable::VariableImpl::setEquivalentTo(const VariablePtr &equivalentVariable)
{
    cleanExpiredVariables();
    if (!hasEquivalentVariable(equivalentVariable)) {
        VariableWeakPtr weakEquivalentVariable = equivalentVariable;
        mEquivalentVariables.push_back(weakEquivalentVariable);
        return true;
    }

    return false;
}

bool Variable::VariableImpl::unsetEquivalentTo(const VariablePtr &equivalentVariable)
{
    cleanExpiredVariables();
    bool status = false;
    auto result = findEquivalentVariable(equivalentVariable);
    if (result != mEquivalentVariables.end()) {
        mEquivalentVariables.erase(result);
        auto mappingIdResult = mMappingIdMap.find(equivalentVariable);
        if (mappingIdResult != mMappingIdMap.end()) {
            mMappingIdMap.erase(mappingIdResult);
        }
        auto connectionIdResult = mConnectionIdMap.find(equivalentVariable);
        if (connectionIdResult != mConnectionIdMap.end()) {
            mConnectionIdMap.erase(connectionIdResult);
        }
        status = true;
    }

    return status;
}

void Variable::VariableImpl::setEquivalentMappingId(const VariablePtr &equivalentVariable, const std::string &id)
{
    VariableWeakPtr weakEquivalentVariable = equivalentVariable;
    mMappingIdMap[weakEquivalentVariable] = id;
}

std::string Variable::VariableImpl::equivalentMappingId(const VariablePtr &equivalentVariable) const
{
    if (mMappingIdMap.find(equivalentVariable) != mMappingIdMap.end()) {
        return mMappingIdMap.at(equivalentVariable);
    }
    return "";
}

void Variable::VariableImpl::setEquivalentConnectionId(const VariablePtr &equivalentVariable, const std::string &id)
{
    VariableWeakPtr weakEquivalentVariable = equivalentVariable;
    mConnectionIdMap[weakEquivalentVariable] = id;
}

std::string Variable::VariableImpl::equivalentConnectionId(const VariablePtr &equivalentVariable) const
{
    if (mConnectionIdMap.find(equivalentVariable) != mConnectionIdMap.end()) {
        return mConnectionIdMap.at(equivalentVariable);
    }
    return "";
}

void Variable::setUnits(const std::string &name)
{
    pFunc()->mUnits = Units::create(name);
}

void Variable::setUnits(const UnitsPtr &units)
{
    pFunc()->mUnits = units;
}

void Variable::removeUnits()
{
    pFunc()->mUnits = nullptr;
}

UnitsPtr Variable::units() const
{
    return pFunc()->mUnits;
}

void Variable::setInitialValue(const std::string &initialValue)
{
    pFunc()->mInitialValue = initialValue;
}

void Variable::setInitialValue(double initialValue)
{
    pFunc()->mInitialValue = convertToString(initialValue);
}

void Variable::setInitialValue(const VariablePtr &variable)
{
    pFunc()->mInitialValue = variable->name();
}

std::string Variable::initialValue() const
{
    return pFunc()->mInitialValue;
}

void Variable::removeInitialValue()
{
    pFunc()->mInitialValue.clear();
}

void Variable::setInterfaceType(const std::string &interfaceType)
{
    pFunc()->mInterfaceType = interfaceType;
}

void Variable::setInterfaceType(Variable::InterfaceType interfaceType)
{
    setInterfaceType(interfaceTypeToString.at(interfaceType));
}

std::string Variable::interfaceType() const
{
    return pFunc()->mInterfaceType;
}

void Variable::removeInterfaceType()
{
    pFunc()->mInterfaceType.clear();
}

bool Variable::hasInterfaceType(InterfaceType interfaceType) const
{
    if (interfaceType == Variable::InterfaceType::NONE && pFunc()->mInterfaceType.empty()) {
        return true;
    }
    return pFunc()->mInterfaceType == interfaceTypeToString.at(interfaceType);
}

bool Variable::permitsInterfaceType(InterfaceType interfaceType) const
{
    std::string testString = interfaceTypeToString.at(interfaceType);

    if (testString == "none") {
        return true;
    }
    if (pFunc()->mInterfaceType == "public_and_private") {
        return true;
    }
    return testString == pFunc()->mInterfaceType;
}

void Variable::setEquivalenceMappingId(const VariablePtr &variable1, const VariablePtr &variable2, const std::string &mappingId)
{
    if ((variable1 != nullptr) && (variable2 != nullptr)) {
        if (variable1->hasEquivalentVariable(variable2, true)) {
            variable1->pFunc()->setEquivalentMappingId(variable2, mappingId);
            variable2->pFunc()->setEquivalentMappingId(variable1, mappingId);
        }
    }
}

void Variable::setEquivalenceConnectionId(const VariablePtr &variable1, const VariablePtr &variable2, const std::string &connectionId)
{
    if ((variable1 != nullptr) && (variable2 != nullptr)) {
        if (variable1->hasEquivalentVariable(variable2, true)) {
            auto map = createConnectionMap(variable1, variable2);
            for (auto &it : map) {
                it.first->pFunc()->setEquivalentConnectionId(it.second, connectionId);
                it.second->pFunc()->setEquivalentConnectionId(it.first, connectionId);
            }
            if (map.empty()) {
                variable1->pFunc()->setEquivalentConnectionId(variable2, connectionId);
                variable2->pFunc()->setEquivalentConnectionId(variable1, connectionId);
            }
        }
    }
}

std::string Variable::equivalenceMappingId(const VariablePtr &variable1, const VariablePtr &variable2)
{
    std::string id;
    if ((variable1 != nullptr) && (variable2 != nullptr)) {
        if (variable1->hasEquivalentVariable(variable2, true)) {
            id = variable1->pFunc()->equivalentMappingId(variable2);
        }
    }
    return id;
}

std::string Variable::equivalenceConnectionId(const VariablePtr &variable1, const VariablePtr &variable2, bool deepSearch)
{
    std::string id;
    if ((variable1 != nullptr) && (variable2 != nullptr)) {
        if (deepSearch) {
            if (variable1->hasEquivalentVariable(variable2, true)) {
                auto map = createConnectionMap(variable1, variable2);

                for (auto &it : map) {
                    id = it.first->pFunc()->equivalentConnectionId(it.second);

                    if (!id.empty()) {
                        return id;
                    }
                }

                id = variable1->pFunc()->equivalentConnectionId(variable2);
            }
        } else {
            id = variable1->pFunc()->equivalentConnectionId(variable2);
        }
    }
    return id;
}

void Variable::removeEquivalenceConnectionId(const VariablePtr &variable1, const VariablePtr &variable2)
{
    if ((variable1 != nullptr) && (variable2 != nullptr)) {
        if (variable1->hasEquivalentVariable(variable2, true)) {
            for (auto &it : createConnectionMap(variable1, variable2)) {
                it.first->pFunc()->setEquivalentConnectionId(it.second, "");
                it.second->pFunc()->setEquivalentConnectionId(it.first, "");
            }

            variable1->pFunc()->setEquivalentConnectionId(variable2, "");
            variable2->pFunc()->setEquivalentConnectionId(variable1, "");
        }
    }
}

void Variable::removeEquivalenceMappingId(const VariablePtr &variable1, const VariablePtr &variable2)
{
    if ((variable1 != nullptr) && (variable2 != nullptr)) {
        if (variable1->hasEquivalentVariable(variable2, true)) {
            variable1->pFunc()->setEquivalentMappingId(variable2, "");
            variable2->pFunc()->setEquivalentMappingId(variable1, "");
        }
    }
}

VariablePtr Variable::clone() const
{
    auto v = create();

    if (pFunc()->mUnits != nullptr) {
        v->setUnits(pFunc()->mUnits->clone());
    }
    v->setInitialValue(initialValue());
    v->setInterfaceType(interfaceType());
    v->setId(id());
    v->setName(name());

    return v;
}

} // namespace libcellml
