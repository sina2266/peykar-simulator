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

#include "debug.h"

#include "libcellml/analyserequation.h"
#include "libcellml/analyserequationast.h"
#include "libcellml/analysermodel.h"
#include "libcellml/analyservariable.h"
#include "libcellml/component.h"
#include "libcellml/generator.h"
#include "libcellml/generatorprofile.h"
#include "libcellml/model.h"
#include "libcellml/variable.h"

#include "commonutils.h"
#include "utilities.h"

#include "libcellml/undefines.h"

namespace libcellml {

std::string astAsCode(const AnalyserEquationAstPtr &ast)
{
    if (ast == nullptr) {
        return "nullptr";
    }

    static GeneratorProfilePtr generatorProfile = nullptr;

    if (generatorProfile == nullptr) {
        generatorProfile = GeneratorProfile::create();

        generatorProfile->setAbsoluteValueString("abs");
        generatorProfile->setNaturalLogarithmString("ln");
        generatorProfile->setCommonLogarithmString("log");
        generatorProfile->setRemString("rem");
        generatorProfile->setAsinString("arcsin");
        generatorProfile->setAcosString("arccos");
        generatorProfile->setAtanString("arctan");
        generatorProfile->setAsecString("arcsec");
        generatorProfile->setAcscString("arccsc");
        generatorProfile->setAcotString("arccot");
        generatorProfile->setAsinhString("arcsinh");
        generatorProfile->setAcoshString("arccosh");
        generatorProfile->setAtanhString("arctanh");
        generatorProfile->setAsechString("arcsech");
        generatorProfile->setAcschString("arccsch");
        generatorProfile->setAcothString("arccoth");
        generatorProfile->setTrueString("true");
        generatorProfile->setFalseString("false");
        generatorProfile->setEString("exponentiale");
        generatorProfile->setPiString("pi");
        generatorProfile->setInfString("infinity");
        generatorProfile->setNanString("notanumber");
    }

    return Generator::equationCode(ast, generatorProfile);
}

void printAnalyserModelEquations(const AnalyserModelPtr &analyserModel)
{
    size_t eqnNb = 0;

    for (const auto &eqn : analyserModel->analyserEquations()) {
        Debug() << "\n---------------------------------------[API equation #" << ++eqnNb << "]";

        if (eqn->ast() != nullptr) {
            Debug() << "\n" << astAsCode(eqn->ast());
        } else {
            Debug() << "\nNo equation";
        }

        Debug() << "\nType: " << AnalyserEquation::typeAsString(eqn->type());

        if (eqn->stateCount() != 0) {
            Debug() << "\nStates:";

            for (const auto &var : eqn->states()) {
                Debug() << " - " << var->variable()->name();
            }
        } else {
            Debug() << "\nNo states";
        }

        if (eqn->computedConstantCount() != 0) {
            Debug() << "\nComputed constants:";

            for (const auto &var : eqn->computedConstants()) {
                Debug() << " - " << var->variable()->name();
            }
        } else {
            Debug() << "\nNo computed constants";
        }

        if (eqn->algebraicVariableCount() != 0) {
            Debug() << "\nAlgebraic variables:";

            for (const auto &algVar : eqn->algebraicVariables()) {
                Debug() << " - " << algVar->variable()->name();
            }
        } else {
            Debug() << "\nNo algebraic variables";
        }

        if (eqn->externalVariableCount() != 0) {
            Debug() << "\nExternal variables:";

            for (const auto &extVar : eqn->externalVariables()) {
                Debug() << " - " << extVar->variable()->name();
            }
        } else {
            Debug() << "\nNo external variables";
        }

        if (eqn->dependencyCount() != 0) {
            Debug() << "\nDependencies:";

            for (const auto &dep : eqn->dependencies()) {
                if (dep->ast() != nullptr) {
                    Debug() << " - " << astAsCode(dep->ast());
                } else if (dep->type() == AnalyserEquation::Type::EXTERNAL) {
                    Debug() << " - External equation for '" << dep->externalVariable(0)->variable()->name() << "'";
                } else {
                    Debug() << " - ??? [" << AnalyserEquation::typeAsString(dep->type()) << "]";
                }
            }
        } else {
            Debug() << "\nNo dependencies";
        }

        if (eqn->type() == AnalyserEquation::Type::NLA) {
            if (eqn->nlaSiblingCount() != 0) {
                Debug() << "\nNLA siblings:";

                for (const auto &nlaSibling : eqn->nlaSiblings()) {
                    if (nlaSibling->ast() != nullptr) {
                        Debug() << " - " << astAsCode(nlaSibling->ast());
                    } else if (nlaSibling->type() == AnalyserEquation::Type::EXTERNAL) {
                        Debug() << " - External equation for '" << nlaSibling->externalVariable(0)->variable()->name() << "'";
                    } else {
                        Debug() << " - ??? [" << AnalyserEquation::typeAsString(nlaSibling->type()) << "]";
                    }
                }
            } else {
                Debug() << "\nNo NLA siblings";
            }
        }
    }

    Debug() << "\n---------------------------------------[END]\n";
}

void printAnalyserModelVariables(const AnalyserModelPtr &analyserModel)
{
    size_t varNb = 0;

    for (const auto &var : analyserVariables(analyserModel)) {
        Debug() << "\n---------------------------------------[API variable #" << ++varNb << "]";
        Debug() << "\nName: " << var->variable()->name();
        Debug() << "Type: " << AnalyserVariable::typeAsString(var->type());

        if (var->analyserEquationCount() != 0) {
            Debug() << "\nEquations:";

            for (const auto &eqn : var->analyserEquations()) {
                if (eqn->ast() != nullptr) {
                    Debug() << " - " << astAsCode(eqn->ast());
                } else if (eqn->type() == AnalyserEquation::Type::EXTERNAL) {
                    Debug() << " - External equation for '" << eqn->externalVariable(0)->variable()->name() << "'";
                } else {
                    Debug() << " - ??? [" << AnalyserEquation::typeAsString(eqn->type()) << "]";
                }
            }
        } else {
            Debug() << "\nNo equations";
        }
    }
}

void printHistory(const History &history)
{
    for (const auto &h : history) {
        printHistoryEpoch(h);
    }
}

void printHistoryEpoch(const HistoryEpochPtr &historyEpoch)
{
    Debug() << "'" << historyEpoch->mSourceModel->name() << "' '" << historyEpoch->mName << "' '" << historyEpoch->mSourceUrl << "' -> '" << historyEpoch->mDestinationUrl << "'";
}

void printStack(const IndexStack &stack)
{
    bool first = true;
    Debug(false) << "[";
    for (const auto &iter : stack) {
        if (!first) {
            Debug(false) << ", ";
        }
        Debug(false) << iter;
        first = false;
    }
    Debug() << "]";
}

void printEquivalenceMap(const EquivalenceMap &map)
{
    Debug() << "Print out of equivalence map";
    for (const auto &iter : map) {
        auto key = iter.first;
        Debug(false) << "key: ";
        printStack(key);
        auto vector = iter.second;
        for (const auto &vectorIt : vector) {
            Debug(false) << "value: ";
            printStack(vectorIt);
        }
    }
}

void printEquivalenceMapWithModelInfo(const EquivalenceMap &map, const ModelPtr &analyserModel)
{
    for (const auto &iter : map) {
        auto key = iter.first;
        Debug(false) << "key: ";
        printStackWithModelInfo(key, analyserModel);
        auto vector = iter.second;
        for (const auto &vectorIt : vector) {
            Debug(false) << "value: ";
            printStackWithModelInfo(vectorIt, analyserModel);
        }
    }
}

void printStackWithModelInfo(const IndexStack &stack, const ModelPtr &analyserModel)
{
    bool first = true;
    ComponentPtr entity;
    Debug(false) << "[";
    for (auto iter = stack.begin(); iter != stack.end(); ++iter) {
        if (!first) {
            Debug(false) << ", ";
        }
        auto next = iter;
        if (first) {
            entity = analyserModel->component(*iter);
            Debug(false) << entity->name();
        } else if (++next == stack.end()) {
            Debug(false) << entity->variable(*iter)->name();
        } else {
            entity = entity->component(*iter);
            Debug(false) << entity->name();
        }
        first = false;
    }
    Debug() << "]";
}

void printConnectionMap(const ConnectionMap &map)
{
    Debug() << "Print out of connection map";
    for (const auto &iter : map) {
        auto key = iter.first;
        auto value = iter.second;
        Debug() << "first: " << key->name();
        Debug() << "second: " << value->name();
    }
}

void printComponentMap(const ComponentMap &map)
{
    Debug() << "Print out of component map";
    for (const auto &iter : map) {
        auto key = iter.first;
        auto value = iter.second;
        Debug() << "first: " << key->name();
        if (value) {
            Debug() << "second: " << value->name();
        } else {
            Debug() << "second: null";
        }
    }
}

void printStringStringMap(const StringStringMap &map)
{
    Debug() << "Print out of string -> string map";
    for (const auto &iter : map) {
        Debug() << iter.first << ": " << iter.second;
    }
}

static const std::string SPACES = "  ";
static const std::string TRUNK = "  │";

struct AnalyserEquationAstTrunk
{
    AnalyserEquationAstTrunk *mPrev;
    std::string mStr;

    AnalyserEquationAstTrunk(AnalyserEquationAstTrunk *prev,
                             const std::string &str);
};

AnalyserEquationAstTrunk::AnalyserEquationAstTrunk(AnalyserEquationAstTrunk *prev,
                                                   const std::string &str)
    : mPrev(prev)
    , mStr(str)
{
}

std::string doPrintAstAsTree(AnalyserEquationAstTrunk *trunk)
{
    if (trunk == nullptr) {
        return {};
    }

    auto res = doPrintAstAsTree(trunk->mPrev);

    if ((trunk->mPrev != nullptr) && (trunk->mPrev->mStr == SPACES)
        && ((trunk->mStr == SPACES) || (trunk->mStr == TRUNK))) {
        res += " ";
    }

    return res + trunk->mStr;
}

std::string doPrintAstAsTree(const AnalyserEquationAstPtr &ast)
{
    std::string res;

    switch (ast->type()) {
        // Equality.

    case AnalyserEquationAst::Type::EQUALITY:
        res = "EQUALITY";

        break;

        // Relational and logical operators.

    case AnalyserEquationAst::Type::EQ:
        res = "EQ";

        break;
    case AnalyserEquationAst::Type::NEQ:
        res = "NEQ";

        break;
    case AnalyserEquationAst::Type::LT:
        res = "LT";

        break;
    case AnalyserEquationAst::Type::LEQ:
        res = "LEQ";

        break;
    case AnalyserEquationAst::Type::GT:
        res = "GT";

        break;
    case AnalyserEquationAst::Type::GEQ:
        res = "LEQ";

        break;
    case AnalyserEquationAst::Type::AND:
        res = "AND";

        break;
    case AnalyserEquationAst::Type::OR:
        res = "OR";

        break;
    case AnalyserEquationAst::Type::XOR:
        res = "XOR";

        break;
    case AnalyserEquationAst::Type::NOT:
        res = "NOT";

        break;

        // Arithmetic operators.

    case AnalyserEquationAst::Type::PLUS:
        res = "PLUS";

        break;
    case AnalyserEquationAst::Type::MINUS:
        res = "MINUS";

        break;
    case AnalyserEquationAst::Type::TIMES:
        res = "TIMES";

        break;
    case AnalyserEquationAst::Type::DIVIDE:
        res = "DIVIDE";

        break;
    case AnalyserEquationAst::Type::POWER:
        res = "POWER";

        break;
    case AnalyserEquationAst::Type::ROOT:
        res = "ROOT";

        break;
    case AnalyserEquationAst::Type::ABS:
        res = "ABS";

        break;
    case AnalyserEquationAst::Type::EXP:
        res = "EXP";

        break;
    case AnalyserEquationAst::Type::LN:
        res = "LN";

        break;
    case AnalyserEquationAst::Type::LOG:
        res = "LOG";

        break;
    case AnalyserEquationAst::Type::CEILING:
        res = "CEILING";

        break;
    case AnalyserEquationAst::Type::FLOOR:
        res = "FLOOR";

        break;
    case AnalyserEquationAst::Type::MIN:
        res = "MIN";

        break;
    case AnalyserEquationAst::Type::MAX:
        res = "MAX";

        break;
    case AnalyserEquationAst::Type::REM:
        res = "REM";

        break;

        // Calculus elements.

    case AnalyserEquationAst::Type::DIFF:
        res = "DIFF";

        break;

        // Trigonometric operators.

    case AnalyserEquationAst::Type::SIN:
        res = "SIN";

        break;
    case AnalyserEquationAst::Type::COS:
        res = "COS";

        break;
    case AnalyserEquationAst::Type::TAN:
        res = "TAN";

        break;
    case AnalyserEquationAst::Type::SEC:
        res = "SEC";

        break;
    case AnalyserEquationAst::Type::CSC:
        res = "CSC";

        break;
    case AnalyserEquationAst::Type::COT:
        res = "COT";

        break;
    case AnalyserEquationAst::Type::SINH:
        res = "SINH";

        break;
    case AnalyserEquationAst::Type::COSH:
        res = "COSH";

        break;
    case AnalyserEquationAst::Type::TANH:
        res = "TANH";

        break;
    case AnalyserEquationAst::Type::SECH:
        res = "SECH";

        break;
    case AnalyserEquationAst::Type::CSCH:
        res = "CSCH";

        break;
    case AnalyserEquationAst::Type::COTH:
        res = "COTH";

        break;
    case AnalyserEquationAst::Type::ASIN:
        res = "ASIN";

        break;
    case AnalyserEquationAst::Type::ACOS:
        res = "ACOS";

        break;
    case AnalyserEquationAst::Type::ATAN:
        res = "ATAN";

        break;
    case AnalyserEquationAst::Type::ASEC:
        res = "ASEC";

        break;
    case AnalyserEquationAst::Type::ACSC:
        res = "ACSC";

        break;
    case AnalyserEquationAst::Type::ACOT:
        res = "ACOT";

        break;
    case AnalyserEquationAst::Type::ASINH:
        res = "ASINH";

        break;
    case AnalyserEquationAst::Type::ACOSH:
        res = "ACOSH";

        break;
    case AnalyserEquationAst::Type::ATANH:
        res = "ATANH";

        break;
    case AnalyserEquationAst::Type::ASECH:
        res = "ASECH";

        break;
    case AnalyserEquationAst::Type::ACSCH:
        res = "ACSCH";

        break;
    case AnalyserEquationAst::Type::ACOTH:
        res = "ACOTH";

        break;

        // Piecewise statement.

    case AnalyserEquationAst::Type::PIECEWISE:
        res = "PIECEWISE";

        break;
    case AnalyserEquationAst::Type::PIECE:
        res = "PIECE";

        break;
    case AnalyserEquationAst::Type::OTHERWISE:
        res = "OTHERWISE";

        break;

        // Token elements.

    case AnalyserEquationAst::Type::CI: {
        auto astVariable = ast->variable();

        if (astVariable != nullptr) {
            res = astVariable->name();
        }

        break;
    }
    case AnalyserEquationAst::Type::CN:
        res = ast->value();

        break;

        // Qualifier elements.

    case AnalyserEquationAst::Type::DEGREE:
        res = "DEGREE";

        break;
    case AnalyserEquationAst::Type::LOGBASE:
        res = "LOGBASE";

        break;
    case AnalyserEquationAst::Type::BVAR:
        res = "BVAR";

        break;

        // Constants.

    case AnalyserEquationAst::Type::TRUE:
        res = "TRUE";

        break;
    case AnalyserEquationAst::Type::FALSE:
        res = "FALSE";

        break;
    case AnalyserEquationAst::Type::E:
        res = "E";

        break;
    case AnalyserEquationAst::Type::PI:
        res = "PI";

        break;
    case AnalyserEquationAst::Type::INF:
        res = "INF";

        break;
    case AnalyserEquationAst::Type::NAN:
        res = "NAN";

        break;
    }

    return res;
}

std::string doPrintAstAsTree(const AnalyserEquationAstPtr &ast,
                             AnalyserEquationAstTrunk *prevTrunk, bool isLeft)
{
    if (ast == nullptr) {
        return {};
    }

    std::string res;
    std::string prevStr = SPACES;
    AnalyserEquationAstTrunk trunk(prevTrunk, prevStr);
    auto astLeftChild = ast->leftChild();

    if (astLeftChild != nullptr) {
        res += doPrintAstAsTree(astLeftChild, &trunk, true);
    }

    if (prevTrunk == nullptr) {
        trunk.mStr = "──";
    } else if (isLeft) {
        trunk.mStr = "╭──";
        prevStr = TRUNK;
    } else {
        trunk.mStr = "╰──";
        prevTrunk->mStr = prevStr;
    }

    res += doPrintAstAsTree(&trunk) + doPrintAstAsTree(ast) + "\n";

    if (prevTrunk != nullptr) {
        prevTrunk->mStr = prevStr;
    }

    trunk.mStr = TRUNK;

    auto astRightChild = ast->rightChild();

    if (astRightChild != nullptr) {
        res += doPrintAstAsTree(astRightChild, &trunk, false);
    }

    return res;
}

void printAstAsTree(const AnalyserEquationAstPtr &ast)
{
    Debug() << doPrintAstAsTree(ast, nullptr, false);
}

void printAstAsCode(const AnalyserEquationAstPtr &ast)
{
    Debug() << astAsCode(ast);
}

void printImportLibrary(const ImportLibrary &importlibrary)
{
    Debug() << " == Import library ==";
    for (const auto &entry : importlibrary) {
        std::string name = "(null)";
        if (entry.second != nullptr) {
            name = entry.second->name();
        }
        Debug() << entry.first << " - " << name;
    }
    Debug() << " == ";
}

void printVariableMap(const VariableMap &map)
{
    Debug() << " == Variable map ==";
    for (const auto &entry : map) {
        auto v1 = entry->variable1();
        auto v2 = entry->variable2();

        Debug(false) << "v1: ";
        if (v1 == nullptr) {
            Debug(false) << "(nullptr)";
        } else {
            Debug(false) << v1->name();
        }
        Debug(false) << " - v2: ";
        if (v2 == nullptr) {
            Debug(false) << "(nullptr)";
        } else {
            Debug(false) << v2->name();
        }
        Debug() << "";
    }
}

void printNamedPath(const ParentedEntityPtr &parented)
{
    if (parented != nullptr) {
        std::vector<std::string> names;
        auto named = std::dynamic_pointer_cast<libcellml::NamedEntity>(parented);
        if (named != nullptr) {
            names.push_back(named->name());
            auto parent = named->parent();
            while (parent != nullptr) {
                auto named = std::dynamic_pointer_cast<libcellml::NamedEntity>(parent);
                if (named != nullptr) {
                    names.push_back(named->name());
                }
                parent = parent->parent();
            }
        }

        while (!names.empty()) {
            Debug(false) << "/" << names.back();
            names.pop_back();
        }
    } else {
        Debug() << "nullptr variable.";
    }
}

void printEquivalences(const VariablePtr &variable)
{
    Debug(false) << "Equivalence for: ";
    printNamedPath(variable);
    Debug();
    if (variable != nullptr) {
        for (size_t j = 0; j < variable->equivalentVariableCount(); ++j) {
            Debug(false) << " - ";
            printNamedPath(variable->equivalentVariable(j));
            Debug();
        }
    }
}

void printUnits(const UnitsPtr &units)
{
    Debug(false) << "Units: " << (units ? units->name() : "nullptr");
    if (units) {
        if (units->isImport()) {
            Debug(false) << " (is imported)";
        }
        Debug() << "";
        auto model = owningModel(units);
        for (size_t i = 0; i < units->unitCount(); ++i) {
            const std::string ref = units->unitAttributeReference(i);
            Debug(false) << " - " << (i + 1) << ": " << ref;
            if (model) {
                Debug() << ", " << model->hasUnits(ref);
            } else {
                Debug() << ", modelless units.";
            }
        }
    }
    Debug() << "*****";
}

void listModelsUnits(const ModelPtr &model)
{
    Debug() << "Model name: " << model->name() << ", units count: " << model->unitsCount();
    for (size_t i = 0; i < model->unitsCount(); ++i) {
        printUnits(model->units(i));
    }
}

} // namespace libcellml
