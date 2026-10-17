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

#include "libcellml/types.h"

#include <limits>

#include "anycellmlelement_p.h"
#include "internaltypes.h"

namespace libcellml {

/**
 * @brief The UnitsItem::UnitsItemImpl struct.
 *
 * The private implementation for the UnitsItem class.
 */
struct UnitsItem::UnitsItemImpl
{
    UnitsWeakPtr mUnits; /**< Units that owns this units item.*/
    size_t mIndex = std::numeric_limits<size_t>::max(); /**< Index of this units item.*/
};

UnitsItem::UnitsItem(const UnitsPtr &units, size_t index)
    : mPimpl(new UnitsItemImpl())
{
    mPimpl->mUnits = units;
    mPimpl->mIndex = index;
}

UnitsItem::~UnitsItem()
{
    delete mPimpl;
}

UnitsItemPtr UnitsItem::create(const UnitsPtr &units, size_t index) noexcept
{
    return std::shared_ptr<UnitsItem> {new UnitsItem {units, index}};
}

UnitsPtr UnitsItem::units() const
{
    return mPimpl->mUnits.lock();
}

size_t UnitsItem::index() const
{
    return mPimpl->mIndex;
}

bool UnitsItem::isValid() const
{
    auto units = mPimpl->mUnits.lock();
    if (units != nullptr) {
        return mPimpl->mIndex < units->unitCount();
    }

    return false;
}

/**
 * @brief The VariablePair::VariablePairImpl struct.
 *
 * The private implementation for the VariablePair class.
 */
struct VariablePair::VariablePairImpl
{
    VariableWeakPtr mVariable1; /**< Variable 1 for the pair.*/
    VariableWeakPtr mVariable2; /**< Variable 2 for the pair.*/
};

VariablePair::VariablePair(const VariablePtr &variable1, const VariablePtr &variable2)
    : mPimpl(new VariablePairImpl())
{
    mPimpl->mVariable1 = variable1;
    mPimpl->mVariable2 = variable2;
}

VariablePairPtr VariablePair::create(const VariablePtr &variable1, const VariablePtr &variable2) noexcept
{
    return std::shared_ptr<VariablePair> {new VariablePair {variable1, variable2}};
}

VariablePair::~VariablePair()
{
    delete mPimpl;
}

VariablePtr VariablePair::variable1() const
{
    return mPimpl->mVariable1.lock();
}

VariablePtr VariablePair::variable2() const
{
    return mPimpl->mVariable2.lock();
}

bool VariablePair::isValid() const
{
    return (mPimpl->mVariable1.lock() != nullptr) && (mPimpl->mVariable2.lock() != nullptr);
}

AnyCellmlElementPtr AnyCellmlElement::AnyCellmlElementImpl::create()
{
    return std::shared_ptr<AnyCellmlElement> {new AnyCellmlElement {}};
}

void AnyCellmlElement::AnyCellmlElementImpl::setComponent(const ComponentPtr &component, CellmlElementType type)
{
    mType = type;
    mItem = component;
}

void AnyCellmlElement::AnyCellmlElementImpl::setComponentRef(const ComponentPtr &component)
{
    mType = CellmlElementType::COMPONENT_REF;
    mItem = component;
}

void AnyCellmlElement::AnyCellmlElementImpl::setEncapsulation(const ModelPtr &model)
{
    mType = CellmlElementType::ENCAPSULATION;
    mItem = model;
}

void AnyCellmlElement::AnyCellmlElementImpl::setConnection(const VariablePairPtr &pair)
{
    setVariablePair(pair, CellmlElementType::CONNECTION);
}

void AnyCellmlElement::AnyCellmlElementImpl::setConnection(const VariablePtr &variable1, const VariablePtr &variable2)
{
    setConnection(VariablePair::create(variable1, variable2));
}

void AnyCellmlElement::AnyCellmlElementImpl::setImportSource(const ImportSourcePtr &importSource)
{
    mType = CellmlElementType::IMPORT;
    mItem = importSource;
}

void AnyCellmlElement::AnyCellmlElementImpl::setMapVariables(const VariablePairPtr &pair)
{
    setVariablePair(pair, CellmlElementType::MAP_VARIABLES);
}

void AnyCellmlElement::AnyCellmlElementImpl::setMapVariables(const VariablePtr &variable1, const VariablePtr &variable2)
{
    setMapVariables(VariablePair::create(variable1, variable2));
}

void AnyCellmlElement::AnyCellmlElementImpl::setMath(const ComponentPtr &component)
{
    mType = CellmlElementType::MATH;
    mItem = component;
}

void AnyCellmlElement::AnyCellmlElementImpl::setModel(const ModelPtr &model, CellmlElementType type)
{
    mType = type;
    mItem = model;
}

void AnyCellmlElement::AnyCellmlElementImpl::setReset(const ResetPtr &reset, CellmlElementType type)
{
    mType = type;
    mItem = reset;
}

void AnyCellmlElement::AnyCellmlElementImpl::setResetValue(const ResetPtr &reset)
{
    mType = CellmlElementType::RESET_VALUE;
    mItem = reset;
}

void AnyCellmlElement::AnyCellmlElementImpl::setTestValue(const ResetPtr &reset)
{
    mType = CellmlElementType::TEST_VALUE;
    mItem = reset;
}

void AnyCellmlElement::AnyCellmlElementImpl::setUnits(const UnitsPtr &units)
{
    mType = CellmlElementType::UNITS;
    mItem = units;
}

void AnyCellmlElement::AnyCellmlElementImpl::setUnitsItem(const UnitsItemPtr &unitsItem)
{
    mType = CellmlElementType::UNIT;
    mItem = unitsItem;
}

void AnyCellmlElement::AnyCellmlElementImpl::setVariable(const VariablePtr &variable)
{
    mType = CellmlElementType::VARIABLE;
    mItem = variable;
}

void AnyCellmlElement::AnyCellmlElementImpl::setVariablePair(const VariablePairPtr &pair, CellmlElementType type)
{
    mType = type;
    mItem = pair;
}

void AnyCellmlElement::AnyCellmlElementImpl::setVariablePair(const VariablePtr &variable1, const VariablePtr &variable2, CellmlElementType type)
{
    setVariablePair(VariablePair::create(variable1, variable2), type);
}

AnyCellmlElement::AnyCellmlElement()
    : mPimpl(new AnyCellmlElementImpl())
{
}

AnyCellmlElement::~AnyCellmlElement()
{
    delete mPimpl;
}

CellmlElementType AnyCellmlElement::type() const
{
    return mPimpl->mType;
}

ComponentPtr AnyCellmlElement::component() const
{
    if ((mPimpl->mType == CellmlElementType::COMPONENT)
        || (mPimpl->mType == CellmlElementType::COMPONENT_REF)) {
        try {
            return std::any_cast<ComponentPtr>(mPimpl->mItem);
        } catch (const std::bad_any_cast &) {
            return nullptr;
        }
    }

    return nullptr;
}

VariablePairPtr AnyCellmlElement::variablePair() const
{
    if ((mPimpl->mType == CellmlElementType::CONNECTION)
        || (mPimpl->mType == CellmlElementType::MAP_VARIABLES)) {
        try {
            return std::any_cast<VariablePairPtr>(mPimpl->mItem);
        } catch (const std::bad_any_cast &) {
            return nullptr;
        }
    }

    return nullptr;
}

ImportSourcePtr AnyCellmlElement::importSource() const
{
    if (mPimpl->mType == CellmlElementType::IMPORT) {
        try {
            return std::any_cast<ImportSourcePtr>(mPimpl->mItem);
        } catch (const std::bad_any_cast &) {
            return nullptr;
        }
    }

    return nullptr;
}

ModelPtr AnyCellmlElement::model() const
{
    if ((mPimpl->mType == CellmlElementType::ENCAPSULATION)
        || (mPimpl->mType == CellmlElementType::MODEL)) {
        try {
            return std::any_cast<ModelPtr>(mPimpl->mItem);
        } catch (const std::bad_any_cast &) {
            return nullptr;
        }
    }

    return nullptr;
}

ResetPtr AnyCellmlElement::reset() const
{
    if ((mPimpl->mType == CellmlElementType::RESET)
        || (mPimpl->mType == CellmlElementType::RESET_VALUE)
        || (mPimpl->mType == CellmlElementType::TEST_VALUE)) {
        try {
            return std::any_cast<ResetPtr>(mPimpl->mItem);
        } catch (const std::bad_any_cast &) {
            return nullptr;
        }
    }

    return nullptr;
}

UnitsPtr AnyCellmlElement::units() const
{
    if (mPimpl->mType == CellmlElementType::UNITS) {
        try {
            return std::any_cast<UnitsPtr>(mPimpl->mItem);
        } catch (const std::bad_any_cast &) {
            return nullptr;
        }
    }

    return nullptr;
}

UnitsItemPtr AnyCellmlElement::unitsItem() const
{
    if (mPimpl->mType == CellmlElementType::UNIT) {
        try {
            return std::any_cast<UnitsItemPtr>(mPimpl->mItem);
        } catch (const std::bad_any_cast &) {
            return nullptr;
        }
    }

    return nullptr;
}

VariablePtr AnyCellmlElement::variable() const
{
    if (mPimpl->mType == CellmlElementType::VARIABLE) {
        try {
            return std::any_cast<VariablePtr>(mPimpl->mItem);
        } catch (const std::bad_any_cast &) {
            return nullptr;
        }
    }

    return nullptr;
}

} // namespace libcellml
