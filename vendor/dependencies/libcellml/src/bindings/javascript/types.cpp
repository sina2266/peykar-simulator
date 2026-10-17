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

#include "libcellml/types.h"

using namespace emscripten;

EMSCRIPTEN_BINDINGS(libcellml_types)
{
    register_vector<std::string>("VectorString");
    register_vector<libcellml::AnyCellmlElementPtr>("VectorAnyCellmlElementPtr");
    register_vector<libcellml::VariablePtr>("VectorVariablePtr");
    register_vector<libcellml::AnalyserVariablePtr>("VectorAnalyserVariablePtr");
    register_vector<libcellml::AnalyserEquationPtr>("VectorAnalyserEquation");

    class_<libcellml::UnitsItem>("UnitsItem")
        .smart_ptr_constructor("UnitsItem", select_overload<libcellml::UnitsItemPtr(const libcellml::UnitsPtr &, size_t)>(&libcellml::UnitsItem::create))
        .function("units", &libcellml::UnitsItem::units)
        .function("index", &libcellml::UnitsItem::index)
        .function("isValid", &libcellml::UnitsItem::isValid)
    ;

    class_<libcellml::VariablePair>("VariablePair")
        .smart_ptr_constructor("VariablePair", select_overload<libcellml::VariablePairPtr(const libcellml::VariablePtr &, const libcellml::VariablePtr &)>(&libcellml::VariablePair::create))
        .function("variable1", &libcellml::VariablePair::variable1)
        .function("variable2", &libcellml::VariablePair::variable2)
        .function("isValid", &libcellml::VariablePair::isValid)
    ;

    class_<libcellml::AnyCellmlElement>("AnyCellmlElement")
        .smart_ptr<std::shared_ptr<libcellml::AnyCellmlElement>>("AnyCellmlElement")
        .function("type", &libcellml::AnyCellmlElement::type)
        .function("component", &libcellml::AnyCellmlElement::component)
        .function("importSource", &libcellml::AnyCellmlElement::importSource)
        .function("model", &libcellml::AnyCellmlElement::model)
        .function("reset", &libcellml::AnyCellmlElement::reset)
        .function("units", &libcellml::AnyCellmlElement::units)
        .function("unitsItem", &libcellml::AnyCellmlElement::unitsItem)
        .function("variable", &libcellml::AnyCellmlElement::variable)
        .function("variablePair", &libcellml::AnyCellmlElement::variablePair)
    ;
}
