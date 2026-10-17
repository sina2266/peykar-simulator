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

#include "libcellml/analyser.h"

using namespace emscripten;

EMSCRIPTEN_BINDINGS(libcellml_analyser)
{
    class_<libcellml::Analyser, base<libcellml::Logger>>("Analyser")
        .smart_ptr_constructor("Analyser", &libcellml::Analyser::create)
        .function("analyseModel", &libcellml::Analyser::analyseModel)
        .function("addExternalVariableByVariable", select_overload<bool(const libcellml::VariablePtr &)>(&libcellml::Analyser::addExternalVariable))
        .function("addExternalVariableByExternalVariable", select_overload<bool(const libcellml::AnalyserExternalVariablePtr &)>(&libcellml::Analyser::addExternalVariable))
        .function("removeExternalVariableByIndex", select_overload<bool(size_t)>(&libcellml::Analyser::removeExternalVariable))
        .function("removeExternalVariableByVariable", select_overload<bool(const libcellml::VariablePtr &)>(&libcellml::Analyser::removeExternalVariable))
        .function("removeExternalVariableByExternalVariable", select_overload<bool(const libcellml::AnalyserExternalVariablePtr &)>(&libcellml::Analyser::removeExternalVariable))
        .function("removeAllExternalVariables", &libcellml::Analyser::removeAllExternalVariables)
        .function("containsExternalVariableByVariable", select_overload<bool(const libcellml::VariablePtr &) const>(&libcellml::Analyser::containsExternalVariable))
        .function("containsExternalVariableByExternalVariable", select_overload<bool(const libcellml::AnalyserExternalVariablePtr &) const>(&libcellml::Analyser::containsExternalVariable))
        .function("externalVariableByIndex", select_overload<libcellml::AnalyserExternalVariablePtr(size_t) const>(&libcellml::Analyser::externalVariable))
        .function("externalVariableByVariable", select_overload<libcellml::AnalyserExternalVariablePtr(const libcellml::VariablePtr &) const>(&libcellml::Analyser::externalVariable))
        .function("externalVariableCount", &libcellml::Analyser::externalVariableCount)
        .function("analyserModel", &libcellml::Analyser::analyserModel)
    ;
}
