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

#include "libcellml/analyservariable.h"

using namespace emscripten;

EMSCRIPTEN_BINDINGS(libcellml_analyservariable)
{
    enum_<libcellml::AnalyserVariable::Type>("AnalyserVariable.Type")
        .value("VARIABLE_OF_INTEGRATION", libcellml::AnalyserVariable::Type::VARIABLE_OF_INTEGRATION)
        .value("STATE", libcellml::AnalyserVariable::Type::STATE)
        .value("CONSTANT", libcellml::AnalyserVariable::Type::CONSTANT)
        .value("COMPUTED_CONSTANT", libcellml::AnalyserVariable::Type::COMPUTED_CONSTANT)
        .value("ALGEBRAIC_VARIABLE", libcellml::AnalyserVariable::Type::ALGEBRAIC_VARIABLE)
        .value("EXTERNAL_VARIABLE", libcellml::AnalyserVariable::Type::EXTERNAL_VARIABLE)
    ;

    class_<libcellml::AnalyserVariable>("AnalyserVariable")
        .smart_ptr<std::shared_ptr<libcellml::AnalyserVariable>>("AnalyserVariable")
        .function("type", &libcellml::AnalyserVariable::type)
        .class_function("typeAsString", &libcellml::AnalyserVariable::typeAsString)
        .function("index", &libcellml::AnalyserVariable::index)
        .function("initialisingVariable", &libcellml::AnalyserVariable::initialisingVariable)
        .function("variable", &libcellml::AnalyserVariable::variable)
        .function("analyserModel", &libcellml::AnalyserVariable::analyserModel)
        .function("analyserEquationCount", &libcellml::AnalyserVariable::analyserEquationCount)
        .function("analyserEquations", &libcellml::AnalyserVariable::analyserEquations)
        .function("analyserEquation", &libcellml::AnalyserVariable::analyserEquation)
    ;

    EM_ASM(
        Module['AnalyserVariable']['Type'] = Module['AnalyserVariable.Type'];
        delete Module['AnalyserVariable.Type'];
    );

}
