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

#include <any>

#include "libcellml/types.h"

namespace libcellml {

/**
 * @brief The AnyCellmlElement::AnyCellmlElementImpl struct.
 *
 * The private implementation for the AnyCellmlElement class.
 */
struct AnyCellmlElement::AnyCellmlElementImpl
{
    CellmlElementType mType = CellmlElementType::UNDEFINED; /**< Type for the CellML element. */
    std::any mItem = nullptr; /**< std::any item cast for the item. */

    /**
     * @brief Create an any CellML element.
     *
     * Create an any CellML element.
     *
     * @return The any CellML element.
     */
    static AnyCellmlElementPtr create();

    void setComponent(const ComponentPtr &component, CellmlElementType type = CellmlElementType::COMPONENT);
    void setComponentRef(const ComponentPtr &component);
    void setConnection(const VariablePairPtr &pair);
    void setConnection(const VariablePtr &variable1, const VariablePtr &variable2);
    void setEncapsulation(const ModelPtr &model);
    void setImportSource(const ImportSourcePtr &importSource);
    void setMapVariables(const VariablePairPtr &pair);
    void setMapVariables(const VariablePtr &variable1, const VariablePtr &variable2);
    void setMath(const ComponentPtr &component);
    void setModel(const ModelPtr &model, CellmlElementType type = CellmlElementType::MODEL);
    void setReset(const ResetPtr &reset, CellmlElementType type = CellmlElementType::RESET);
    void setResetValue(const ResetPtr &reset);
    void setTestValue(const ResetPtr &reset);
    void setUnits(const UnitsPtr &units);
    void setUnitsItem(const UnitsItemPtr &unitsItem);
    void setVariable(const VariablePtr &variable);
    void setVariablePair(const VariablePairPtr &pair, CellmlElementType type);
    void setVariablePair(const VariablePtr &variable1, const VariablePtr &variable2, CellmlElementType type);
};

} // namespace libcellml
