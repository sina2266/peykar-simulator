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

#include "libcellml/analysermodel.h"

using namespace emscripten;

EMSCRIPTEN_BINDINGS(libcellml_analysermodel)
{
    enum_<libcellml::AnalyserModel::Type>("AnalyserModel.Type")
        .value("UNKNOWN", libcellml::AnalyserModel::Type::UNKNOWN)
        .value("ODE", libcellml::AnalyserModel::Type::ODE)
        .value("DAE", libcellml::AnalyserModel::Type::DAE)
        .value("NLA", libcellml::AnalyserModel::Type::NLA)
        .value("ALGEBRAIC", libcellml::AnalyserModel::Type::ALGEBRAIC)
        .value("INVALID", libcellml::AnalyserModel::Type::INVALID)
        .value("UNDERCONSTRAINED", libcellml::AnalyserModel::Type::UNDERCONSTRAINED)
        .value("OVERCONSTRAINED", libcellml::AnalyserModel::Type::OVERCONSTRAINED)
        .value("UNSUITABLY_CONSTRAINED", libcellml::AnalyserModel::Type::UNSUITABLY_CONSTRAINED)
    ;

    class_<libcellml::AnalyserModel>("AnalyserModel")
        .smart_ptr<std::shared_ptr<libcellml::AnalyserModel>>("AnalyserModel")
        .function("isValid", &libcellml::AnalyserModel::isValid)
        .function("type", &libcellml::AnalyserModel::type)
        .class_function("typeAsString", &libcellml::AnalyserModel::typeAsString)
        .function("hasExternalVariables", &libcellml::AnalyserModel::hasExternalVariables)
        .function("voi", &libcellml::AnalyserModel::voi)
        .function("stateCount", &libcellml::AnalyserModel::stateCount)
        .function("states", &libcellml::AnalyserModel::states)
        .function("state", &libcellml::AnalyserModel::state)
        .function("constantCount", &libcellml::AnalyserModel::constantCount)
        .function("constants", &libcellml::AnalyserModel::constants)
        .function("constant", &libcellml::AnalyserModel::constant)
        .function("computedConstantCount", &libcellml::AnalyserModel::computedConstantCount)
        .function("computedConstants", &libcellml::AnalyserModel::computedConstants)
        .function("computedConstant", &libcellml::AnalyserModel::computedConstant)
        .function("algebraicVariableCount", &libcellml::AnalyserModel::algebraicVariableCount)
        .function("algebraicVariables", &libcellml::AnalyserModel::algebraicVariables)
        .function("algebraicVariable", &libcellml::AnalyserModel::algebraicVariable)
        .function("externalVariableCount", &libcellml::AnalyserModel::externalVariableCount)
        .function("externalVariables", &libcellml::AnalyserModel::externalVariables)
        .function("externalVariable", &libcellml::AnalyserModel::externalVariable)
        .function("analyserVariable", &libcellml::AnalyserModel::analyserVariable)
        .function("analyserEquationCount", &libcellml::AnalyserModel::analyserEquationCount)
        .function("analyserEquations", &libcellml::AnalyserModel::analyserEquations)
        .function("analyserEquation", &libcellml::AnalyserModel::analyserEquation)
        .function("needEqFunction", &libcellml::AnalyserModel::needEqFunction)
        .function("needNeqFunction", &libcellml::AnalyserModel::needNeqFunction)
        .function("needLtFunction", &libcellml::AnalyserModel::needLtFunction)
        .function("needLeqFunction", &libcellml::AnalyserModel::needLeqFunction)
        .function("needGtFunction", &libcellml::AnalyserModel::needGtFunction)
        .function("needGeqFunction", &libcellml::AnalyserModel::needGeqFunction)
        .function("needAndFunction", &libcellml::AnalyserModel::needAndFunction)
        .function("needOrFunction", &libcellml::AnalyserModel::needOrFunction)
        .function("needXorFunction", &libcellml::AnalyserModel::needXorFunction)
        .function("needNotFunction", &libcellml::AnalyserModel::needNotFunction)
        .function("needMinFunction", &libcellml::AnalyserModel::needMinFunction)
        .function("needMaxFunction", &libcellml::AnalyserModel::needMaxFunction)
        .function("needSecFunction", &libcellml::AnalyserModel::needSecFunction)
        .function("needCscFunction", &libcellml::AnalyserModel::needCscFunction)
        .function("needCotFunction", &libcellml::AnalyserModel::needCotFunction)
        .function("needSechFunction", &libcellml::AnalyserModel::needSechFunction)
        .function("needCschFunction", &libcellml::AnalyserModel::needCschFunction)
        .function("needCothFunction", &libcellml::AnalyserModel::needCothFunction)
        .function("needAsecFunction", &libcellml::AnalyserModel::needAsecFunction)
        .function("needAcscFunction", &libcellml::AnalyserModel::needAcscFunction)
        .function("needAcotFunction", &libcellml::AnalyserModel::needAcotFunction)
        .function("needAsechFunction", &libcellml::AnalyserModel::needAsechFunction)
        .function("needAcschFunction", &libcellml::AnalyserModel::needAcschFunction)
        .function("needAcothFunction", &libcellml::AnalyserModel::needAcothFunction)
        .function("areEquivalentVariables", &libcellml::AnalyserModel::areEquivalentVariables)
    ;

    EM_ASM(
        Module['AnalyserModel']['Type'] = Module['AnalyserModel.Type'];
        delete Module['AnalyserModel.Type'];
    );
}
