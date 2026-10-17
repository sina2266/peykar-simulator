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
#include <emscripten/emscripten.h>

#include "libcellml/issue.h"

using namespace emscripten;

EMSCRIPTEN_BINDINGS(libcellml_issue) {

    enum_<libcellml::Issue::Level>("Issue.Level")
        .value("ERROR", libcellml::Issue::Level::ERROR)
        .value("WARNING", libcellml::Issue::Level::WARNING)
        .value("MESSAGE", libcellml::Issue::Level::MESSAGE)
    ;

    enum_<libcellml::Issue::ReferenceRule>("Issue.ReferenceRule")
        .value("ANALYSER_EQUATION_NOT_EQUALITY_STATEMENT", libcellml::Issue::ReferenceRule::ANALYSER_EQUATION_NOT_EQUALITY_STATEMENT)
        .value("ANALYSER_EXTERNAL_VARIABLE_DIFFERENT_MODEL", libcellml::Issue::ReferenceRule::ANALYSER_EXTERNAL_VARIABLE_DIFFERENT_MODEL)
        .value("ANALYSER_EXTERNAL_VARIABLE_USE_PRIMARY_VARIABLE", libcellml::Issue::ReferenceRule::ANALYSER_EXTERNAL_VARIABLE_USE_PRIMARY_VARIABLE)
        .value("ANALYSER_EXTERNAL_VARIABLE_VOI", libcellml::Issue::ReferenceRule::ANALYSER_EXTERNAL_VARIABLE_VOI)
        .value("ANALYSER_ODE_NOT_FIRST_ORDER", libcellml::Issue::ReferenceRule::ANALYSER_ODE_NOT_FIRST_ORDER)
        .value("ANALYSER_STATE_NOT_INITIALISED", libcellml::Issue::ReferenceRule::ANALYSER_STATE_NOT_INITIALISED)
        .value("ANALYSER_STATE_RATE_AS_ALGEBRAIC_VARIABLE", libcellml::Issue::ReferenceRule::ANALYSER_STATE_RATE_AS_ALGEBRAIC_VARIABLE)
        .value("ANALYSER_UNITS", libcellml::Issue::ReferenceRule::ANALYSER_UNITS)
        .value("ANALYSER_UNLINKED_UNITS", libcellml::Issue::ReferenceRule::ANALYSER_UNLINKED_UNITS)
        .value("ANALYSER_VARIABLE_UNDERCONSTRAINED", libcellml::Issue::ReferenceRule::ANALYSER_VARIABLE_UNDERCONSTRAINED)
        .value("ANALYSER_VARIABLE_OVERCONSTRAINED", libcellml::Issue::ReferenceRule::ANALYSER_VARIABLE_OVERCONSTRAINED)
        .value("ANALYSER_VARIABLE_INITIALISED_MORE_THAN_ONCE", libcellml::Issue::ReferenceRule::ANALYSER_VARIABLE_INITIALISED_MORE_THAN_ONCE)
        .value("ANALYSER_VARIABLE_INITIALISED_USING_ALGEBRAIC_VARIABLE", libcellml::Issue::ReferenceRule::ANALYSER_VARIABLE_INITIALISED_USING_ALGEBRAIC_VARIABLE)
        .value("ANALYSER_VARIABLE_INITIALISED_USING_VARIABLE_WITH_DIFFERENT_UNITS", libcellml::Issue::ReferenceRule::ANALYSER_VARIABLE_INITIALISED_USING_VARIABLE_WITH_DIFFERENT_UNITS)
        .value("ANALYSER_VARIABLE_UNUSED", libcellml::Issue::ReferenceRule::ANALYSER_VARIABLE_UNUSED)
        .value("ANALYSER_VOI_INITIALISED", libcellml::Issue::ReferenceRule::ANALYSER_VOI_INITIALISED)
        .value("ANALYSER_VOI_SEVERAL", libcellml::Issue::ReferenceRule::ANALYSER_VOI_SEVERAL)
        .value("ANNOTATOR_ID_NOT_FOUND", libcellml::Issue::ReferenceRule::ANNOTATOR_ID_NOT_FOUND)
        .value("ANNOTATOR_ID_NOT_UNIQUE", libcellml::Issue::ReferenceRule::ANNOTATOR_ID_NOT_UNIQUE)
        .value("ANNOTATOR_INCONSISTENT_TYPE", libcellml::Issue::ReferenceRule::ANNOTATOR_INCONSISTENT_TYPE)
        .value("ANNOTATOR_NO_MODEL", libcellml::Issue::ReferenceRule::ANNOTATOR_NO_MODEL)
        .value("ANNOTATOR_NULL_MODEL", libcellml::Issue::ReferenceRule::ANNOTATOR_NULL_MODEL)
        .value("COMPONENT_CHILD", libcellml::Issue::ReferenceRule::COMPONENT_CHILD)
        .value("COMPONENT_ELEMENT", libcellml::Issue::ReferenceRule::COMPONENT_ELEMENT)
        .value("COMPONENT_NAME", libcellml::Issue::ReferenceRule::COMPONENT_NAME)
        .value("COMPONENT_NAME_UNIQUE", libcellml::Issue::ReferenceRule::COMPONENT_NAME_UNIQUE)
        .value("COMPONENT_NAME_VALUE", libcellml::Issue::ReferenceRule::COMPONENT_NAME_VALUE)
        .value("COMPONENT_REF_CHILD", libcellml::Issue::ReferenceRule::COMPONENT_REF_CHILD)
        .value("COMPONENT_REF_COMPONENT_ATTRIBUTE", libcellml::Issue::ReferenceRule::COMPONENT_REF_COMPONENT_ATTRIBUTE)
        .value("COMPONENT_REF_COMPONENT_ATTRIBUTE_REFERENCE", libcellml::Issue::ReferenceRule::COMPONENT_REF_COMPONENT_ATTRIBUTE_REFERENCE)
        .value("COMPONENT_REF_COMPONENT_ATTRIBUTE_UNIQUE", libcellml::Issue::ReferenceRule::COMPONENT_REF_COMPONENT_ATTRIBUTE_UNIQUE)
        .value("COMPONENT_REF_ELEMENT", libcellml::Issue::ReferenceRule::COMPONENT_REF_ELEMENT)
        .value("CONNECTION_CHILD", libcellml::Issue::ReferenceRule::CONNECTION_CHILD)
        .value("CONNECTION_COMPONENT1_ATTRIBUTE", libcellml::Issue::ReferenceRule::CONNECTION_COMPONENT1_ATTRIBUTE)
        .value("CONNECTION_COMPONENT1_ATTRIBUTE_REFERENCE", libcellml::Issue::ReferenceRule::CONNECTION_COMPONENT1_ATTRIBUTE_REFERENCE)
        .value("CONNECTION_COMPONENT2_ATTRIBUTE", libcellml::Issue::ReferenceRule::CONNECTION_COMPONENT2_ATTRIBUTE)
        .value("CONNECTION_COMPONENT2_ATTRIBUTE_REFERENCE", libcellml::Issue::ReferenceRule::CONNECTION_COMPONENT2_ATTRIBUTE_REFERENCE)
        .value("CONNECTION_ELEMENT", libcellml::Issue::ReferenceRule::CONNECTION_ELEMENT)
        .value("CONNECTION_EXCLUDE_SELF", libcellml::Issue::ReferenceRule::CONNECTION_EXCLUDE_SELF)
        .value("CONNECTION_UNIQUE", libcellml::Issue::ReferenceRule::CONNECTION_UNIQUE)
        .value("DATA_REPR_IDENTIFIER_AT_LEAST_ONE_ALPHANUM", libcellml::Issue::ReferenceRule::DATA_REPR_IDENTIFIER_AT_LEAST_ONE_ALPHANUM)
        .value("DATA_REPR_IDENTIFIER_BEGIN_EURO_NUM", libcellml::Issue::ReferenceRule::DATA_REPR_IDENTIFIER_BEGIN_EURO_NUM)
        .value("DATA_REPR_IDENTIFIER_LATIN_ALPHANUM", libcellml::Issue::ReferenceRule::DATA_REPR_IDENTIFIER_LATIN_ALPHANUM)
        .value("ENCAPSULATION_CHILD", libcellml::Issue::ReferenceRule::ENCAPSULATION_CHILD)
        .value("ENCAPSULATION_ELEMENT", libcellml::Issue::ReferenceRule::ENCAPSULATION_ELEMENT)
        .value("IMPORTER_ERROR_IMPORTING_UNITS", libcellml::Issue::ReferenceRule::IMPORTER_ERROR_IMPORTING_UNITS)
        .value("IMPORTER_MISSING_COMPONENT", libcellml::Issue::ReferenceRule::IMPORTER_MISSING_COMPONENT)
        .value("IMPORTER_MISSING_FILE", libcellml::Issue::ReferenceRule::IMPORTER_MISSING_FILE)
        .value("IMPORTER_MISSING_UNITS", libcellml::Issue::ReferenceRule::IMPORTER_MISSING_UNITS)
        .value("IMPORTER_NULL_MODEL", libcellml::Issue::ReferenceRule::IMPORTER_NULL_MODEL)
        .value("IMPORTER_UNDEFINED_MODEL", libcellml::Issue::ReferenceRule::IMPORTER_UNDEFINED_MODEL)
        .value("IMPORTER_UNRESOLVED_IMPORTS", libcellml::Issue::ReferenceRule::IMPORTER_UNRESOLVED_IMPORTS)
        .value("IMPORT_CHILD", libcellml::Issue::ReferenceRule::IMPORT_CHILD)
        .value("IMPORT_COMPONENT_COMPONENT_REFERENCE", libcellml::Issue::ReferenceRule::IMPORT_COMPONENT_COMPONENT_REFERENCE)
        .value("IMPORT_COMPONENT_COMPONENT_REFERENCE_TARGET", libcellml::Issue::ReferenceRule::IMPORT_COMPONENT_COMPONENT_REFERENCE_TARGET)
        .value("IMPORT_COMPONENT_COMPONENT_REFERENCE_VALUE", libcellml::Issue::ReferenceRule::IMPORT_COMPONENT_COMPONENT_REFERENCE_VALUE)
        .value("IMPORT_COMPONENT_ELEMENT", libcellml::Issue::ReferenceRule::IMPORT_COMPONENT_ELEMENT)
        .value("IMPORT_COMPONENT_NAME", libcellml::Issue::ReferenceRule::IMPORT_COMPONENT_NAME)
        .value("IMPORT_COMPONENT_NAME_UNIQUE", libcellml::Issue::ReferenceRule::IMPORT_COMPONENT_NAME_UNIQUE)
        .value("IMPORT_COMPONENT_NAME_VALUE", libcellml::Issue::ReferenceRule::IMPORT_COMPONENT_NAME_VALUE)
        .value("IMPORT_ELEMENT", libcellml::Issue::ReferenceRule::IMPORT_ELEMENT)
        .value("IMPORT_EQUIVALENT_INFOSET", libcellml::Issue::ReferenceRule::IMPORT_EQUIVALENT_INFOSET)
        .value("IMPORT_HREF", libcellml::Issue::ReferenceRule::IMPORT_HREF)
        .value("IMPORT_HREF_LOCATOR", libcellml::Issue::ReferenceRule::IMPORT_HREF_LOCATOR)
        .value("IMPORT_UNITS_ELEMENT", libcellml::Issue::ReferenceRule::IMPORT_UNITS_ELEMENT)
        .value("IMPORT_UNITS_NAME", libcellml::Issue::ReferenceRule::IMPORT_UNITS_NAME)
        .value("IMPORT_UNITS_NAME_UNIQUE", libcellml::Issue::ReferenceRule::IMPORT_UNITS_NAME_UNIQUE)
        .value("IMPORT_UNITS_NAME_VALUE", libcellml::Issue::ReferenceRule::IMPORT_UNITS_NAME_VALUE)
        .value("IMPORT_UNITS_UNITS_REFERENCE", libcellml::Issue::ReferenceRule::IMPORT_UNITS_UNITS_REFERENCE)
        .value("IMPORT_UNITS_UNITS_REFERENCE_VALUE", libcellml::Issue::ReferenceRule::IMPORT_UNITS_UNITS_REFERENCE_VALUE)
        .value("IMPORT_UNITS_UNITS_REFERENCE_VALUE_TARGET", libcellml::Issue::ReferenceRule::IMPORT_UNITS_UNITS_REFERENCE_VALUE_TARGET)
        .value("INVALID_ARGUMENT", libcellml::Issue::ReferenceRule::INVALID_ARGUMENT)
        .value("MAP_VARIABLES_ELEMENT", libcellml::Issue::ReferenceRule::MAP_VARIABLES_ELEMENT)
        .value("MAP_VARIABLES_UNIQUE", libcellml::Issue::ReferenceRule::MAP_VARIABLES_UNIQUE)
        .value("MAP_VARIABLES_VARIABLE1_ATTRIBUTE", libcellml::Issue::ReferenceRule::MAP_VARIABLES_VARIABLE1_ATTRIBUTE)
        .value("MAP_VARIABLES_VARIABLE1_ATTRIBUTE_REFERENCE", libcellml::Issue::ReferenceRule::MAP_VARIABLES_VARIABLE1_ATTRIBUTE_REFERENCE)
        .value("MAP_VARIABLES_VARIABLE2_ATTRIBUTE", libcellml::Issue::ReferenceRule::MAP_VARIABLES_VARIABLE2_ATTRIBUTE)
        .value("MAP_VARIABLES_VARIABLE2_ATTRIBUTE_REFERENCE", libcellml::Issue::ReferenceRule::MAP_VARIABLES_VARIABLE2_ATTRIBUTE_REFERENCE)
        .value("MATH_CHILD", libcellml::Issue::ReferenceRule::MATH_CHILD)
        .value("MATH_CI_VARIABLE_REFERENCE", libcellml::Issue::ReferenceRule::MATH_CI_VARIABLE_REFERENCE)
        .value("MATH_CN_BASE10", libcellml::Issue::ReferenceRule::MATH_CN_BASE10)
        .value("MATH_CN_FORMAT", libcellml::Issue::ReferenceRule::MATH_CN_FORMAT)
        .value("MATH_CN_UNITS_ATTRIBUTE", libcellml::Issue::ReferenceRule::MATH_CN_UNITS_ATTRIBUTE)
        .value("MATH_CN_UNITS_ATTRIBUTE_REFERENCE", libcellml::Issue::ReferenceRule::MATH_CN_UNITS_ATTRIBUTE_REFERENCE)
        .value("MATH_ELEMENT", libcellml::Issue::ReferenceRule::MATH_ELEMENT)
        .value("MATH_MATHML", libcellml::Issue::ReferenceRule::MATH_MATHML)
        .value("MODEL_CHILD", libcellml::Issue::ReferenceRule::MODEL_CHILD)
        .value("MODEL_ELEMENT", libcellml::Issue::ReferenceRule::MODEL_ELEMENT)
        .value("MODEL_MORE_THAN_ONE_ENCAPSULATION", libcellml::Issue::ReferenceRule::MODEL_MORE_THAN_ONE_ENCAPSULATION)
        .value("MODEL_NAME", libcellml::Issue::ReferenceRule::MODEL_NAME)
        .value("MODEL_NAME_VALUE", libcellml::Issue::ReferenceRule::MODEL_NAME_VALUE)
        .value("RESET_ATTRIBUTE_REQUIRED", libcellml::Issue::ReferenceRule::RESET_ATTRIBUTE_REQUIRED)
        .value("RESET_CHILD", libcellml::Issue::ReferenceRule::RESET_CHILD)
        .value("RESET_ELEMENT", libcellml::Issue::ReferenceRule::RESET_ELEMENT)
        .value("RESET_ORDER_UNIQUE", libcellml::Issue::ReferenceRule::RESET_ORDER_UNIQUE)
        .value("RESET_ORDER_VALUE", libcellml::Issue::ReferenceRule::RESET_ORDER_VALUE)
        .value("RESET_RESET_VALUE_CHILD", libcellml::Issue::ReferenceRule::RESET_RESET_VALUE_CHILD)
        .value("RESET_TEST_VALUE_CHILD", libcellml::Issue::ReferenceRule::RESET_TEST_VALUE_CHILD)
        .value("RESET_TEST_VARIABLE_REFERENCE", libcellml::Issue::ReferenceRule::RESET_TEST_VARIABLE_REFERENCE)
        .value("RESET_VALUE_CHILD", libcellml::Issue::ReferenceRule::RESET_VALUE_CHILD)
        .value("RESET_VALUE_ELEMENT", libcellml::Issue::ReferenceRule::RESET_VALUE_ELEMENT)
        .value("RESET_VARIABLE_REFERENCE", libcellml::Issue::ReferenceRule::RESET_VARIABLE_REFERENCE)
        .value("TEST_VALUE_CHILD", libcellml::Issue::ReferenceRule::TEST_VALUE_CHILD)
        .value("TEST_VALUE_ELEMENT", libcellml::Issue::ReferenceRule::TEST_VALUE_ELEMENT)
        .value("UNDEFINED", libcellml::Issue::ReferenceRule::UNDEFINED)
        .value("UNITS_CHILD", libcellml::Issue::ReferenceRule::UNITS_CHILD)
        .value("UNITS_ELEMENT", libcellml::Issue::ReferenceRule::UNITS_ELEMENT)
        .value("UNITS_NAME", libcellml::Issue::ReferenceRule::UNITS_NAME)
        .value("UNITS_NAME_UNIQUE", libcellml::Issue::ReferenceRule::UNITS_NAME_UNIQUE)
        .value("UNITS_NAME_VALUE", libcellml::Issue::ReferenceRule::UNITS_NAME_VALUE)
        .value("UNITS_STANDARD", libcellml::Issue::ReferenceRule::UNITS_STANDARD)
        .value("UNIT_ATTRIBUTE_EXPONENT_VALUE", libcellml::Issue::ReferenceRule::UNIT_ATTRIBUTE_EXPONENT_VALUE)
        .value("UNIT_ATTRIBUTE_MULTIPLIER_VALUE", libcellml::Issue::ReferenceRule::UNIT_ATTRIBUTE_MULTIPLIER_VALUE)
        .value("UNIT_ATTRIBUTE_OPTIONAL", libcellml::Issue::ReferenceRule::UNIT_ATTRIBUTE_OPTIONAL)
        .value("UNIT_ATTRIBUTE_PREFIX_VALUE", libcellml::Issue::ReferenceRule::UNIT_ATTRIBUTE_PREFIX_VALUE)
        .value("UNIT_ELEMENT", libcellml::Issue::ReferenceRule::UNIT_ELEMENT)
        .value("UNIT_UNITS", libcellml::Issue::ReferenceRule::UNIT_UNITS)
        .value("UNIT_UNITS_CIRCULAR_REFERENCE", libcellml::Issue::ReferenceRule::UNIT_UNITS_CIRCULAR_REFERENCE)
        .value("UNIT_UNITS_REFERENCE", libcellml::Issue::ReferenceRule::UNIT_UNITS_REFERENCE)
        .value("UNSPECIFIED", libcellml::Issue::ReferenceRule::UNSPECIFIED)
        .value("VARIABLE_ATTRIBUTE_OPTIONAL", libcellml::Issue::ReferenceRule::VARIABLE_ATTRIBUTE_OPTIONAL)
        .value("VARIABLE_ATTRIBUTE_REQUIRED", libcellml::Issue::ReferenceRule::VARIABLE_ATTRIBUTE_REQUIRED)
        .value("VARIABLE_ELEMENT", libcellml::Issue::ReferenceRule::VARIABLE_ELEMENT)
        .value("VARIABLE_INITIAL_VALUE_VALUE", libcellml::Issue::ReferenceRule::VARIABLE_INITIAL_VALUE_VALUE)
        .value("VARIABLE_INTERFACE_VALUE", libcellml::Issue::ReferenceRule::VARIABLE_INTERFACE_VALUE)
        .value("VARIABLE_NAME_UNIQUE", libcellml::Issue::ReferenceRule::VARIABLE_NAME_UNIQUE)
        .value("VARIABLE_NAME_VALUE", libcellml::Issue::ReferenceRule::VARIABLE_NAME_VALUE)
        .value("VARIABLE_UNITS_VALUE", libcellml::Issue::ReferenceRule::VARIABLE_UNITS_VALUE)
        .value("XML_ID_ATTRIBUTE", libcellml::Issue::ReferenceRule::XML_ID_ATTRIBUTE)
        .value("XML_UNEXPECTED_CHARACTER", libcellml::Issue::ReferenceRule::XML_UNEXPECTED_CHARACTER)
        .value("XML_UNEXPECTED_ELEMENT", libcellml::Issue::ReferenceRule::XML_UNEXPECTED_ELEMENT)
        .value("XML_UNEXPECTED_NAMESPACE", libcellml::Issue::ReferenceRule::XML_UNEXPECTED_NAMESPACE)
        .value("XML_ATTRIBUTE_HAS_NAMESPACE", libcellml::Issue::ReferenceRule::XML_ATTRIBUTE_HAS_NAMESPACE)
    ;

    class_<libcellml::Issue>("Issue")
        .smart_ptr<std::shared_ptr<libcellml::Issue>>("Issue")
        .function("description", &libcellml::Issue::description)
        .function("level", &libcellml::Issue::level)
        .function("referenceRule", &libcellml::Issue::referenceRule)
        .function("item", &libcellml::Issue::item)
        .function("url", &libcellml::Issue::url)
        .function("referenceHeading", &libcellml::Issue::referenceHeading)
    ;

    EM_ASM(
        Module['Issue']['ReferenceRule'] = Module['Issue.ReferenceRule'];
        delete Module['Issue.ReferenceRule'];
        Module['Issue']['Level'] = Module['Issue.Level'];
        delete Module['Issue.Level'];
    );
}
