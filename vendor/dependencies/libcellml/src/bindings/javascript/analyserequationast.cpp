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

#include "libcellml/analyserequationast.h"

using namespace emscripten;

EMSCRIPTEN_BINDINGS(libcellml_analyserequationast)
{
    enum_<libcellml::AnalyserEquationAst::Type>("AnalyserEquationAst.Type")
        .value("EQUALITY", libcellml::AnalyserEquationAst::Type::EQUALITY)
        .value("EQ", libcellml::AnalyserEquationAst::Type::EQ)
        .value("NEQ", libcellml::AnalyserEquationAst::Type::NEQ)
        .value("LT", libcellml::AnalyserEquationAst::Type::LT)
        .value("LEQ", libcellml::AnalyserEquationAst::Type::LEQ)
        .value("GT", libcellml::AnalyserEquationAst::Type::GT)
        .value("GEQ", libcellml::AnalyserEquationAst::Type::GEQ)
        .value("AND", libcellml::AnalyserEquationAst::Type::AND)
        .value("OR", libcellml::AnalyserEquationAst::Type::OR)
        .value("XOR", libcellml::AnalyserEquationAst::Type::XOR)
        .value("NOT", libcellml::AnalyserEquationAst::Type::NOT)
        .value("PLUS", libcellml::AnalyserEquationAst::Type::PLUS)
        .value("MINUS", libcellml::AnalyserEquationAst::Type::MINUS)
        .value("TIMES", libcellml::AnalyserEquationAst::Type::TIMES)
        .value("DIVIDE", libcellml::AnalyserEquationAst::Type::DIVIDE)
        .value("POWER", libcellml::AnalyserEquationAst::Type::POWER)
        .value("ROOT", libcellml::AnalyserEquationAst::Type::ROOT)
        .value("ABS", libcellml::AnalyserEquationAst::Type::ABS)
        .value("EXP", libcellml::AnalyserEquationAst::Type::EXP)
        .value("LN", libcellml::AnalyserEquationAst::Type::LN)
        .value("LOG", libcellml::AnalyserEquationAst::Type::LOG)
        .value("CEILING", libcellml::AnalyserEquationAst::Type::CEILING)
        .value("FLOOR", libcellml::AnalyserEquationAst::Type::FLOOR)
        .value("MIN", libcellml::AnalyserEquationAst::Type::MIN)
        .value("MAX", libcellml::AnalyserEquationAst::Type::MAX)
        .value("REM", libcellml::AnalyserEquationAst::Type::REM)
        .value("DIFF", libcellml::AnalyserEquationAst::Type::DIFF)
        .value("SIN", libcellml::AnalyserEquationAst::Type::SIN)
        .value("COS", libcellml::AnalyserEquationAst::Type::COS)
        .value("TAN", libcellml::AnalyserEquationAst::Type::TAN)
        .value("SEC", libcellml::AnalyserEquationAst::Type::SEC)
        .value("CSC", libcellml::AnalyserEquationAst::Type::CSC)
        .value("COT", libcellml::AnalyserEquationAst::Type::COT)
        .value("SINH", libcellml::AnalyserEquationAst::Type::SINH)
        .value("COSH", libcellml::AnalyserEquationAst::Type::COSH)
        .value("TANH", libcellml::AnalyserEquationAst::Type::TANH)
        .value("SECH", libcellml::AnalyserEquationAst::Type::SECH)
        .value("CSCH", libcellml::AnalyserEquationAst::Type::CSCH)
        .value("COTH", libcellml::AnalyserEquationAst::Type::COTH)
        .value("ASIN", libcellml::AnalyserEquationAst::Type::ASIN)
        .value("ACOS", libcellml::AnalyserEquationAst::Type::ACOS)
        .value("ATAN", libcellml::AnalyserEquationAst::Type::ATAN)
        .value("ASEC", libcellml::AnalyserEquationAst::Type::ASEC)
        .value("ACSC", libcellml::AnalyserEquationAst::Type::ACSC)
        .value("ACOT", libcellml::AnalyserEquationAst::Type::ACOT)
        .value("ASINH", libcellml::AnalyserEquationAst::Type::ASINH)
        .value("ACOSH", libcellml::AnalyserEquationAst::Type::ACOSH)
        .value("ATANH", libcellml::AnalyserEquationAst::Type::ATANH)
        .value("ASECH", libcellml::AnalyserEquationAst::Type::ASECH)
        .value("ACSCH", libcellml::AnalyserEquationAst::Type::ACSCH)
        .value("ACOTH", libcellml::AnalyserEquationAst::Type::ACOTH)
        .value("PIECEWISE", libcellml::AnalyserEquationAst::Type::PIECEWISE)
        .value("PIECE", libcellml::AnalyserEquationAst::Type::PIECE)
        .value("OTHERWISE", libcellml::AnalyserEquationAst::Type::OTHERWISE)
        .value("CI", libcellml::AnalyserEquationAst::Type::CI)
        .value("CN", libcellml::AnalyserEquationAst::Type::CN)
        .value("DEGREE", libcellml::AnalyserEquationAst::Type::DEGREE)
        .value("LOGBASE", libcellml::AnalyserEquationAst::Type::LOGBASE)
        .value("BVAR", libcellml::AnalyserEquationAst::Type::BVAR)
        .value("TRUE", libcellml::AnalyserEquationAst::Type::TRUE)
        .value("FALSE", libcellml::AnalyserEquationAst::Type::FALSE)
        .value("E", libcellml::AnalyserEquationAst::Type::E)
        .value("PI", libcellml::AnalyserEquationAst::Type::PI)
        .value("INF", libcellml::AnalyserEquationAst::Type::INF)
        .value("NAN", libcellml::AnalyserEquationAst::Type::NAN)
    ;

    class_<libcellml::AnalyserEquationAst>("AnalyserEquationAst")
        .smart_ptr_constructor("AnalyserEquationAst", &libcellml::AnalyserEquationAst::create)
        .function("type", &libcellml::AnalyserEquationAst::type)
        .class_function("typeAsString", &libcellml::AnalyserEquationAst::typeAsString)
        .function("setType", &libcellml::AnalyserEquationAst::setType)
        .function("value", &libcellml::AnalyserEquationAst::value)
        .function("setValue", &libcellml::AnalyserEquationAst::setValue)
        .function("variable", &libcellml::AnalyserEquationAst::variable)
        .function("setVariable", &libcellml::AnalyserEquationAst::setVariable)
        .function("parent", &libcellml::AnalyserEquationAst::parent)
        .function("setParent", &libcellml::AnalyserEquationAst::setParent)
        .function("leftChild", &libcellml::AnalyserEquationAst::leftChild)
        .function("setLeftChild", &libcellml::AnalyserEquationAst::setLeftChild)
        .function("rightChild", &libcellml::AnalyserEquationAst::rightChild)
        .function("setRightChild", &libcellml::AnalyserEquationAst::setRightChild)
        .function("swapLeftAndRightChildren", &libcellml::AnalyserEquationAst::swapLeftAndRightChildren)
    ;

    EM_ASM(
        Module['AnalyserEquationAst']['Type'] = Module['AnalyserEquationAst.Type'];
        delete Module['AnalyserEquationAst.Type'];
    );
}
