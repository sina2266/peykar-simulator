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

#include "libcellml/variable.h"

using namespace emscripten;

EMSCRIPTEN_BINDINGS(libcellml_variable) {

    enum_<libcellml::Variable::InterfaceType>("Variable.InterfaceType")
        .value("NONE", libcellml::Variable::InterfaceType::NONE)
        .value("PRIVATE", libcellml::Variable::InterfaceType::PRIVATE)
        .value("PUBLIC", libcellml::Variable::InterfaceType::PUBLIC)
        .value("PUBLIC_AND_PRIVATE", libcellml::Variable::InterfaceType::PUBLIC_AND_PRIVATE)
    ;

    class_<libcellml::Variable, base<libcellml::NamedEntity>>("Variable")
        .smart_ptr_constructor("Variable", select_overload<libcellml::VariablePtr()>(&libcellml::Variable::create))
        .constructor(select_overload<libcellml::VariablePtr(const std::string &)>(&libcellml::Variable::create))
        .function("removeAllEquivalences", &libcellml::Variable::removeAllEquivalences)
        .function("equivalentVariable", &libcellml::Variable::equivalentVariable)
        .function("equivalentVariableCount", &libcellml::Variable::equivalentVariableCount)
        .function("hasEquivalentVariable", &libcellml::Variable::hasEquivalentVariable)
        .function("setUnitsByName", select_overload<void(const std::string &)>(&libcellml::Variable::setUnits))
        .function("setUnitsByUnits", select_overload<void(const libcellml::UnitsPtr &)>(&libcellml::Variable::setUnits))
        .function("units", &libcellml::Variable::units)
        .function("removeUnits", &libcellml::Variable::removeUnits)
        .function("setInitialValueByString", select_overload<void(const std::string &)>(&libcellml::Variable::setInitialValue))
        .function("setInitialValueByDouble", select_overload<void(double)>(&libcellml::Variable::setInitialValue))
        .function("setInitialValueByVariable", select_overload<void(const libcellml::VariablePtr &)>(&libcellml::Variable::setInitialValue))
        .function("initialValue", &libcellml::Variable::initialValue)
        .function("removeInitialValue", &libcellml::Variable::removeInitialValue)
        .function("setInterfaceTypeByString", select_overload<void(const std::string &)>(&libcellml::Variable::setInterfaceType))
        .function("setInterfaceTypeByInterfaceType", select_overload<void(libcellml::Variable::InterfaceType)>(&libcellml::Variable::setInterfaceType))
        .function("interfaceType", &libcellml::Variable::interfaceType)
        .function("removeInterfaceType", &libcellml::Variable::removeInterfaceType)
        .function("hasInterfaceType", &libcellml::Variable::hasInterfaceType)
        .function("permitsInterfaceType", &libcellml::Variable::permitsInterfaceType)
        .function("clone", &libcellml::Variable::clone)
        .class_function("equivalenceConnectionId", &libcellml::Variable::equivalenceConnectionId)
        .class_function("equivalenceMappingId", &libcellml::Variable::equivalenceMappingId)
        .class_function("addEquivalence", select_overload<bool(const libcellml::VariablePtr &, const libcellml::VariablePtr &)>(&libcellml::Variable::addEquivalence))
        .class_function("addEquivalenceWithIds", select_overload<bool(const libcellml::VariablePtr &, const libcellml::VariablePtr &, const std::string &, const std::string &)>(&libcellml::Variable::addEquivalence))
        .class_function("removeEquivalence", &libcellml::Variable::removeEquivalence)
        .class_function("setEquivalenceConnectionId", &libcellml::Variable::setEquivalenceConnectionId)
        .class_function("setEquivalenceMappingId", &libcellml::Variable::setEquivalenceMappingId)
    ;

    EM_ASM(
        Module['Variable']['InterfaceType'] = Module['Variable.InterfaceType'];
        delete Module['Variable.InterfaceType'];
    );
}
