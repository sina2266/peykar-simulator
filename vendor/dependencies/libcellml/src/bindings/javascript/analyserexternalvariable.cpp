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

#include "libcellml/analyserexternalvariable.h"

using namespace emscripten;

EMSCRIPTEN_BINDINGS(libcellml_analyserexternalvariable)
{
    class_<libcellml::AnalyserExternalVariable>("AnalyserExternalVariable")
        .smart_ptr_constructor("AnalyserExternalVariable", &libcellml::AnalyserExternalVariable::create)
        .function("variable", &libcellml::AnalyserExternalVariable::variable)
        .function("addDependency", &libcellml::AnalyserExternalVariable::addDependency)
        .function("removeDependencyByIndex", select_overload<bool(size_t)>(&libcellml::AnalyserExternalVariable::removeDependency))
        .function("removeDependencyByModel", select_overload<bool(const libcellml::ModelPtr &, const std::string &, const std::string &)>(&libcellml::AnalyserExternalVariable::removeDependency))
        .function("removeDependencyByVariable", select_overload<bool(const libcellml::VariablePtr &)>(&libcellml::AnalyserExternalVariable::removeDependency))
        .function("removeAllDependencies", &libcellml::AnalyserExternalVariable::removeAllDependencies)
        .function("containsDependencyByModel", select_overload<bool(const libcellml::ModelPtr &, const std::string &, const std::string &) const>(&libcellml::AnalyserExternalVariable::containsDependency))
        .function("containsDependencyByVariable", select_overload<bool(const libcellml::VariablePtr &) const>(&libcellml::AnalyserExternalVariable::containsDependency))
        .function("dependencyByIndex", select_overload<libcellml::VariablePtr(size_t) const>(&libcellml::AnalyserExternalVariable::dependency))
        .function("dependencyByModel", select_overload<libcellml::VariablePtr(const libcellml::ModelPtr &, const std::string &, const std::string &) const>(&libcellml::AnalyserExternalVariable::dependency))
        .function("dependencies", &libcellml::AnalyserExternalVariable::dependencies)
        .function("dependencyCount", &libcellml::AnalyserExternalVariable::dependencyCount)
    ;
}
