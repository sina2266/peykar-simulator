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

#include "libcellml/reset.h"

#include "reset_p.h"
#include "utilities.h"

namespace libcellml {

Reset::ResetImpl *Reset::pFunc()
{
    return reinterpret_cast<Reset::ResetImpl *>(Entity::pFunc());
}

const Reset::ResetImpl *Reset::pFunc() const
{
    return reinterpret_cast<Reset::ResetImpl const *>(Entity::pFunc());
}

Reset::Reset()
    : ParentedEntity(new Reset::ResetImpl())
{
}

Reset::Reset(int order)
    : ParentedEntity(new Reset::ResetImpl())
{
    setOrder(order);
}

Reset::~Reset()
{
    delete pFunc();
}

ResetPtr Reset::create() noexcept
{
    return std::shared_ptr<Reset> {new Reset {}};
}

ResetPtr Reset::create(int order) noexcept
{
    return std::shared_ptr<Reset> {new Reset {order}};
}

void Reset::setOrder(int order)
{
    pFunc()->mOrder = order;
    pFunc()->mOrderSet = true;
}

int Reset::order() const
{
    return pFunc()->mOrder;
}

void Reset::removeOrder()
{
    pFunc()->mOrderSet = false;
    pFunc()->mOrder = 0;
}

bool Reset::isOrderSet()
{
    return pFunc()->mOrderSet;
}

void Reset::setVariable(const VariablePtr &variable)
{
    pFunc()->mVariable = variable;
}

VariablePtr Reset::variable() const
{
    return pFunc()->mVariable;
}

void Reset::setTestVariable(const VariablePtr &variable)
{
    pFunc()->mTestVariable = variable;
}

VariablePtr Reset::testVariable() const
{
    return pFunc()->mTestVariable;
}

void Reset::appendTestValue(const std::string &math)
{
    pFunc()->mTestValue.append(math);
}

std::string Reset::testValue() const
{
    return pFunc()->mTestValue;
}

void Reset::setTestValueId(const std::string &id)
{
    pFunc()->mTestValueId = id;
}

void Reset::removeTestValueId()
{
    pFunc()->mTestValueId = "";
}

std::string Reset::testValueId() const
{
    return pFunc()->mTestValueId;
}

void Reset::setTestValue(const std::string &math)
{
    pFunc()->mTestValue = math;
}

void Reset::removeTestValue()
{
    pFunc()->mTestValue = "";
}

void Reset::appendResetValue(const std::string &math)
{
    pFunc()->mResetValue.append(math);
}

std::string Reset::resetValue() const
{
    return pFunc()->mResetValue;
}

void Reset::setResetValue(const std::string &math)
{
    pFunc()->mResetValue = math;
}

void Reset::removeResetValue()
{
    pFunc()->mResetValue = "";
}

void Reset::setResetValueId(const std::string &id)
{
    pFunc()->mResetValueId = id;
}

void Reset::removeResetValueId()
{
    pFunc()->mResetValueId = "";
}

std::string Reset::resetValueId() const
{
    return pFunc()->mResetValueId;
}

ResetPtr Reset::clone() const
{
    auto r = create();

    r->setId(id());
    r->setOrder(order());
    r->setResetValue(resetValue());
    r->setResetValueId(resetValueId());
    r->setTestValue(testValue());
    r->setTestValueId(testValueId());
    if (pFunc()->mVariable != nullptr) {
        r->setVariable(pFunc()->mVariable->clone());
    }
    if (pFunc()->mTestVariable != nullptr) {
        r->setTestVariable(pFunc()->mTestVariable->clone());
    }

    return r;
}

bool Reset::doEquals(const EntityPtr &other) const
{
    if (Entity::doEquals(other)) {
        auto reset = std::dynamic_pointer_cast<Reset>(other);
        if ((reset != nullptr) && pFunc()->mOrder == reset->order()
            && areEqual(pFunc()->mResetValue, reset->resetValue())
            && pFunc()->mResetValueId == reset->resetValueId()
            && areEqual(pFunc()->mTestValue, reset->testValue())
            && pFunc()->mTestValueId == reset->testValueId()) {
            if (pFunc()->mTestVariable != nullptr
                && !pFunc()->mTestVariable->equals(reset->testVariable())) {
                return false;
            }
            if ((pFunc()->mTestVariable == nullptr)
                && reset->testVariable() != nullptr) {
                return false;
            }
            if ((pFunc()->mVariable != nullptr)
                && !pFunc()->mVariable->equals(reset->variable())) {
                return false;
            }
            if ((pFunc()->mVariable == nullptr)
                && reset->variable() != nullptr) {
                return false;
            }
            return true;
        }
    }
    return false;
}

} // namespace libcellml
