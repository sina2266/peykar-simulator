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

#include "libcellml/enums.h"

#include <map>

namespace libcellml {

static const std::map<CellmlElementType, std::string> cellmlElementTypeToString = {
    {CellmlElementType::COMPONENT, "component"},
    {CellmlElementType::COMPONENT_REF, "component_ref"},
    {CellmlElementType::CONNECTION, "connection"},
    {CellmlElementType::ENCAPSULATION, "encapsulation"},
    {CellmlElementType::IMPORT, "import"},
    {CellmlElementType::MATH, "math"},
    {CellmlElementType::MAP_VARIABLES, "map_variables"},
    {CellmlElementType::MODEL, "model"},
    {CellmlElementType::RESET, "reset"},
    {CellmlElementType::RESET_VALUE, "reset_value"},
    {CellmlElementType::TEST_VALUE, "test_value"},
    {CellmlElementType::UNDEFINED, "undefined"},
    {CellmlElementType::UNIT, "unit"},
    {CellmlElementType::UNITS, "units"},
    {CellmlElementType::VARIABLE, "variable"}};

std::string cellmlElementTypeAsString(CellmlElementType value)
{
    return cellmlElementTypeToString.at(value);
}

} // namespace libcellml
