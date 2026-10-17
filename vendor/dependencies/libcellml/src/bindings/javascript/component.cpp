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

// To work around multiple inheritance we have to create a combined Component
// and ImportedEntity class that we can bind with Emscripten.
#define JAVASCRIPT_BINDINGS
#include "libcellml/component.h"

using namespace emscripten;

EMSCRIPTEN_BINDINGS(libcellml_component) {

    class_<libcellml::Component, base<libcellml::ComponentEntity>>("Component")
        .smart_ptr<std::shared_ptr<libcellml::Component>>("ComponentPtr")
        .constructor(select_overload<libcellml::ComponentPtr()>(&libcellml::Component::create))
        .constructor(select_overload<libcellml::ComponentPtr(const std::string &)>(&libcellml::Component::create))
        .function("setSourceComponent", &libcellml::Component::setSourceComponent)
        .function("appendMath", &libcellml::Component::appendMath)
        .function("math", &libcellml::Component::math)
        .function("setMath", &libcellml::Component::setMath)
        .function("removeMath", &libcellml::Component::removeMath)
        .function("addVariable", &libcellml::Component::addVariable)
        .function("removeVariableByIndex", select_overload<bool(size_t)>(&libcellml::Component::removeVariable))
        .function("removeVariableByName", select_overload<bool(const std::string &)>(&libcellml::Component::removeVariable))
        .function("removeVariableByVariable", select_overload<bool(const libcellml::VariablePtr &)>(&libcellml::Component::removeVariable))
        .function("removeAllVariables", &libcellml::Component::removeAllVariables)
        .function("variableByIndex", select_overload<libcellml::VariablePtr(size_t) const>(&libcellml::Component::variable))
        .function("variableByName", select_overload<libcellml::VariablePtr(const std::string &) const>(&libcellml::Component::variable))
        .function("takeVariableByIndex", select_overload<libcellml::VariablePtr(size_t)>(&libcellml::Component::takeVariable))
        .function("takeVariableByName", select_overload<libcellml::VariablePtr(const std::string &)>(&libcellml::Component::takeVariable))
        .function("variableCount", &libcellml::Component::variableCount)
        .function("hasVariableByVariable", select_overload<bool(const libcellml::VariablePtr &) const>(&libcellml::Component::hasVariable))
        .function("hasVariableByName", select_overload<bool(const std::string &) const>(&libcellml::Component::hasVariable))
        .function("addReset", &libcellml::Component::addReset)
        .function("takeReset", &libcellml::Component::takeReset)
        .function("removeResetByIndex", select_overload<bool(size_t)>(&libcellml::Component::removeReset))
        .function("removeResetByReset", select_overload<bool(const libcellml::ResetPtr &)>(&libcellml::Component::removeReset))
        .function("removeAllResets", &libcellml::Component::removeAllResets)
        .function("requiresImports", &libcellml::Component::requiresImports)
        .function("reset", &libcellml::Component::reset)
        .function("resetCount", &libcellml::Component::resetCount)
        .function("hasReset", &libcellml::Component::hasReset)
        .function("clone", &libcellml::Component::clone)
        .function("requiresImports", &libcellml::Component::requiresImports)
        .function("isImport", &libcellml::Component::isImport)
        .function("importSource", &libcellml::Component::importSource)
        .function("setImportSource", &libcellml::Component::setImportSource)
        .function("importReference", &libcellml::Component::importReference)
        .function("setImportReference", &libcellml::Component::setImportReference)
        .function("isResolved", &libcellml::Component::isResolved)
        .function("isDefined", &libcellml::Component::isDefined)
    ;

}
