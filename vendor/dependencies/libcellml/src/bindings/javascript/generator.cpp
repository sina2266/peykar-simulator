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

#include "libcellml/analyserequationast.h"
#include "libcellml/generator.h"
#include "libcellml/generatorprofile.h"
#include "libcellml/generatorvariabletracker.h"

using namespace emscripten;

EMSCRIPTEN_BINDINGS(libcellml_generator)
{
    class_<libcellml::Generator, base<libcellml::Logger>>("Generator")
        .smart_ptr_constructor("Generator", &libcellml::Generator::create)
        .function("interfaceCode", select_overload<std::string(const libcellml::AnalyserModelPtr &)>(&libcellml::Generator::interfaceCode))
        .function("interfaceCodeByProfile", select_overload<std::string(const libcellml::AnalyserModelPtr &, const libcellml::GeneratorProfilePtr &)>(&libcellml::Generator::interfaceCode))
        .function("interfaceCodeByProfileEnumeration", select_overload<std::string(const libcellml::AnalyserModelPtr &, libcellml::GeneratorProfile::Profile)>(&libcellml::Generator::interfaceCode))
        .function("interfaceCodeByVariableTracker", select_overload<std::string(const libcellml::AnalyserModelPtr &, const libcellml::GeneratorVariableTrackerPtr &)>(&libcellml::Generator::interfaceCode))
        .function("interfaceCodeByProfileAndVariableTracker", select_overload<std::string(const libcellml::AnalyserModelPtr &, const libcellml::GeneratorProfilePtr &, const libcellml::GeneratorVariableTrackerPtr &)>(&libcellml::Generator::interfaceCode))
        .function("implementationCode", select_overload<std::string(const libcellml::AnalyserModelPtr &)>(&libcellml::Generator::implementationCode))
        .function("implementationCodeByProfile", select_overload<std::string(const libcellml::AnalyserModelPtr &, const libcellml::GeneratorProfilePtr &)>(&libcellml::Generator::implementationCode))
        .function("implementationCodeByProfileEnumeration", select_overload<std::string(const libcellml::AnalyserModelPtr &, libcellml::GeneratorProfile::Profile)>(&libcellml::Generator::implementationCode))
        .function("implementationCodeByVariableTracker", select_overload<std::string(const libcellml::AnalyserModelPtr &, const libcellml::GeneratorVariableTrackerPtr &)>(&libcellml::Generator::implementationCode))
        .function("implementationCodeByProfileAndVariableTracker", select_overload<std::string(const libcellml::AnalyserModelPtr &, const libcellml::GeneratorProfilePtr &, const libcellml::GeneratorVariableTrackerPtr &)>(&libcellml::Generator::implementationCode))
        .class_function("equationCode", select_overload<std::string(const libcellml::AnalyserEquationAstPtr &)>(&libcellml::Generator::equationCode))
        .class_function("equationCodeByProfile", select_overload<std::string(const libcellml::AnalyserEquationAstPtr &, const libcellml::GeneratorProfilePtr &)>(&libcellml::Generator::equationCode))
    ;
}
