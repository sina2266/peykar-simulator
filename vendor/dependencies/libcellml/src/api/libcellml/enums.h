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

#include <string>

#include "libcellml/exportdefinitions.h"

namespace libcellml {

/**
 * @brief The CellmlElementType enum class.
 *
 * This enum describes CellML elements.
 * A CellML element identifies elements in
 * the underlying CellML specification format.
 * The enum also adds an UNDEFINED entry for
 * not specifying an underlying CellML element.
 */
enum class CellmlElementType
{
    COMPONENT,
    COMPONENT_REF,
    CONNECTION,
    ENCAPSULATION,
    IMPORT,
    MAP_VARIABLES,
    MATH,
    MODEL,
    RESET,
    RESET_VALUE,
    TEST_VALUE,
    UNDEFINED,
    UNIT,
    UNITS,
    VARIABLE,
};

/**
 * @brief Convert a CellmlElementType enumeration value into its @c std::string form.
 *
 * Convert a CellmlElementType enumeration value into its @c std::string form.
 *
 * @param value A CellmlElementType enumeration.
 * @return A @c std::string.
 */
LIBCELLML_EXPORT std::string cellmlElementTypeAsString(CellmlElementType value);

} // namespace libcellml
