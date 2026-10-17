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

#include "libcellml/annotator.h"

using namespace emscripten;

EMSCRIPTEN_BINDINGS(libcellml_annotator)
{
    class_<libcellml::Annotator>("Annotator")
        .smart_ptr_constructor("Annotator", select_overload<libcellml::AnnotatorPtr()>(&libcellml::Annotator::create))
        .function("setModel", &libcellml::Annotator::setModel)
        .function("model", select_overload<libcellml::ModelPtr() const>(&libcellml::Annotator::model))
        .function("modelById", select_overload<libcellml::ModelPtr(const std::string &)>(&libcellml::Annotator::model))
        .function("modelByIdIndex", select_overload<libcellml::ModelPtr(const std::string &, size_t)>(&libcellml::Annotator::model))
        .function("item", select_overload<libcellml::AnyCellmlElementPtr(const std::string &)>(&libcellml::Annotator::item))
        .function("itemByIndex", select_overload<libcellml::AnyCellmlElementPtr(const std::string &, size_t)>(&libcellml::Annotator::item))
        .function("component", select_overload<libcellml::ComponentPtr(const std::string &)>(&libcellml::Annotator::component))
        .function("componentByIndex", select_overload<libcellml::ComponentPtr(const std::string &, size_t)>(&libcellml::Annotator::component))
        .function("componentEncapsulation", select_overload<libcellml::ComponentPtr(const std::string &)>(&libcellml::Annotator::componentEncapsulation))
        .function("componentEncapsulationByIndex", select_overload<libcellml::ComponentPtr(const std::string &, size_t)>(&libcellml::Annotator::componentEncapsulation))
        .function("encapsulation", select_overload<libcellml::ModelPtr(const std::string &)>(&libcellml::Annotator::encapsulation))
        .function("encapsulationByIndex", select_overload<libcellml::ModelPtr(const std::string &, size_t)>(&libcellml::Annotator::encapsulation))
        .function("variable", select_overload<libcellml::VariablePtr(const std::string &)>(&libcellml::Annotator::variable))
        .function("variableByIndex", select_overload<libcellml::VariablePtr(const std::string &, size_t)>(&libcellml::Annotator::variable))
        .function("reset", select_overload<libcellml::ResetPtr(const std::string &)>(&libcellml::Annotator::reset))
        .function("resetByIndex", select_overload<libcellml::ResetPtr(const std::string &, size_t)>(&libcellml::Annotator::reset))
        .function("importSource", select_overload<libcellml::ImportSourcePtr(const std::string &)>(&libcellml::Annotator::importSource))
        .function("importSourceByIndex", select_overload<libcellml::ImportSourcePtr(const std::string &, size_t)>(&libcellml::Annotator::importSource))
        .function("units", select_overload<libcellml::UnitsPtr(const std::string &)>(&libcellml::Annotator::units))
        .function("unitsByIndex", select_overload<libcellml::UnitsPtr(const std::string &, size_t)>(&libcellml::Annotator::units))
        .function("mapVariables", select_overload<libcellml::VariablePairPtr(const std::string &)>(&libcellml::Annotator::mapVariables))
        .function("mapVariablesByIndex", select_overload<libcellml::VariablePairPtr(const std::string &, size_t)>(&libcellml::Annotator::mapVariables))
        .function("connection", select_overload<libcellml::VariablePairPtr(const std::string &)>(&libcellml::Annotator::connection))
        .function("connectionByIndex", select_overload<libcellml::VariablePairPtr(const std::string &, size_t)>(&libcellml::Annotator::connection))
        .function("unitsItem", select_overload<libcellml::UnitsItemPtr(const std::string &)>(&libcellml::Annotator::unitsItem))
        .function("unitsItemByIndex", select_overload<libcellml::UnitsItemPtr(const std::string &, size_t)>(&libcellml::Annotator::unitsItem))
        .function("testValue", select_overload<libcellml::ResetPtr(const std::string &)>(&libcellml::Annotator::testValue))
        .function("testValueByIndex", select_overload<libcellml::ResetPtr(const std::string &, size_t)>(&libcellml::Annotator::testValue))
        .function("resetValue", select_overload<libcellml::ResetPtr(const std::string &)>(&libcellml::Annotator::resetValue))
        .function("resetValueByIndex", select_overload<libcellml::ResetPtr(const std::string &, size_t)>(&libcellml::Annotator::resetValue))
        .function("assignAllIds", select_overload<bool()>(&libcellml::Annotator::assignAllIds))
        .function("assignAllIdsByModel", select_overload<bool(libcellml::ModelPtr &)>(&libcellml::Annotator::assignAllIds))
        .function("assignIds", &libcellml::Annotator::assignIds)
        .function("clearAllIds", select_overload<void()>(&libcellml::Annotator::clearAllIds))
        .function("clearAllIdsByModel", select_overload<void(libcellml::ModelPtr &)>(&libcellml::Annotator::clearAllIds))
        .function("isUnique", &libcellml::Annotator::isUnique)
        .function("items", &libcellml::Annotator::items)
        .function("ids", &libcellml::Annotator::ids)
        .function("duplicateIds", &libcellml::Annotator::duplicateIds)
        .function("assignIdByComponent", select_overload<std::string(const libcellml::ComponentPtr &, libcellml::CellmlElementType)>(&libcellml::Annotator::assignId))
        .function("assignIdByImportSource", select_overload<std::string(const libcellml::ImportSourcePtr &)>(&libcellml::Annotator::assignId))
        .function("assignIdByModel", select_overload<std::string(const libcellml::ModelPtr &, libcellml::CellmlElementType)>(&libcellml::Annotator::assignId))
        .function("assignIdByReset", select_overload<std::string(const libcellml::ResetPtr &, libcellml::CellmlElementType)>(&libcellml::Annotator::assignId))
        .function("assignIdByUnits", select_overload<std::string(const libcellml::UnitsPtr &)>(&libcellml::Annotator::assignId))
        .function("assignIdByUnitsIndex", select_overload<std::string(const libcellml::UnitsPtr &, size_t)>(&libcellml::Annotator::assignId))
        .function("assignIdByUnitsItem", select_overload<std::string(const libcellml::UnitsItemPtr &)>(&libcellml::Annotator::assignId))
        .function("assignIdByVariable", select_overload<std::string(const libcellml::VariablePtr &)>(&libcellml::Annotator::assignId))
        .function("assignIdByVariablePair", select_overload<std::string(const libcellml::VariablePairPtr &, libcellml::CellmlElementType)>(&libcellml::Annotator::assignId))
        .function("assignIdByVariableVariable", select_overload<std::string(const libcellml::VariablePtr &, const libcellml::VariablePtr &, libcellml::CellmlElementType)>(&libcellml::Annotator::assignId))
        .function("itemCount", &libcellml::Annotator::itemCount)
        .function("hasModel", &libcellml::Annotator::hasModel)
    ;
}
