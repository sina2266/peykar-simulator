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

#include <emscripten/bind.h>

#include "libcellml/enums.h"

using namespace emscripten;

EMSCRIPTEN_BINDINGS(libcellml_enums) {
    enum_<libcellml::CellmlElementType>("CellmlElementType")
        .value("COMPONENT", libcellml::CellmlElementType::COMPONENT)
        .value("COMPONENT_REF", libcellml::CellmlElementType::COMPONENT_REF)
        .value("CONNECTION", libcellml::CellmlElementType::CONNECTION)
        .value("ENCAPSULATION", libcellml::CellmlElementType::ENCAPSULATION)
        .value("IMPORT", libcellml::CellmlElementType::IMPORT)
        .value("MAP_VARIABLES", libcellml::CellmlElementType::MAP_VARIABLES)
        .value("MATH", libcellml::CellmlElementType::MATH)
        .value("MODEL", libcellml::CellmlElementType::MODEL)
        .value("RESET", libcellml::CellmlElementType::RESET)
        .value("RESET_VALUE", libcellml::CellmlElementType::RESET_VALUE)
        .value("TEST_VALUE", libcellml::CellmlElementType::TEST_VALUE)
        .value("UNDEFINED", libcellml::CellmlElementType::UNDEFINED)
        .value("UNIT", libcellml::CellmlElementType::UNIT)
        .value("UNITS", libcellml::CellmlElementType::UNITS)
        .value("VARIABLE", libcellml::CellmlElementType::VARIABLE)
    ;

    function("cellmlElementTypeAsString", &libcellml::cellmlElementTypeAsString);
}
