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

#include "libcellml/generator.h"

#include <regex>

#include "libcellml/analyserequation.h"
#include "libcellml/analyserequationast.h"
#include "libcellml/analysermodel.h"
#include "libcellml/analyservariable.h"
#include "libcellml/generatorvariabletracker.h"
#include "libcellml/units.h"
#include "libcellml/version.h"

#include "analyserequation_p.h"
#include "commonutils.h"
#include "generator_p.h"
#include "generatorprofilesha1values.h"
#include "generatorprofiletools.h"
#include "utilities.h"

#include "libcellml/undefines.h"

namespace libcellml {

void Generator::GeneratorImpl::reset()
{
    mCode = {};
}

std::string Generator::GeneratorImpl::analyserVariableIndexString(const AnalyserVariablePtr &analyserVariable)
{
    // Determine the actual index of the analyser variable in the list of analyser variables by accounting for the fact
    // that some analyser variables may be untracked.

    auto analyserVariables = libcellml::analyserVariables(analyserVariable);

    if (analyserVariables.empty()) {
        return convertToString(analyserVariable->index());
    }

    size_t i = MAX_SIZE_T;
    size_t res = MAX_SIZE_T;

    for (;;) {
        auto analyserVar = analyserVariables[++i];

        if (isTrackedVariable(analyserVar, true)) {
            ++res;
        }

        if (analyserVariable == analyserVar) {
            break;
        }
    }

    return convertToString(res);
}

bool Generator::GeneratorImpl::isTrackedEquation(const AnalyserEquationPtr &analyserEquation, bool tracked)
{
    AnalyserVariablePtr analyserVariable;

    switch (analyserEquation->type()) {
    case AnalyserEquation::Type::COMPUTED_CONSTANT:
        analyserVariable = analyserEquation->computedConstants().front();

        return isTrackedVariable(analyserVariable, tracked);
    case AnalyserEquation::Type::ALGEBRAIC:
        analyserVariable = analyserEquation->algebraicVariables().front();

        return isTrackedVariable(analyserVariable, tracked);
    default:
        return true;
    }
}

bool Generator::GeneratorImpl::isTrackedVariable(const AnalyserVariablePtr &analyserVariable, bool tracked)
{
    return tracked ? (mVariableTracker == nullptr) || mVariableTracker->isTrackedVariable(analyserVariable) : (mVariableTracker != nullptr) && mVariableTracker->isUntrackedVariable(analyserVariable);
}

bool Generator::GeneratorImpl::modelHasOdes(const AnalyserModelPtr &analyserModel) const
{
    switch (analyserModel->type()) {
    case AnalyserModel::Type::ODE:
    case AnalyserModel::Type::DAE:
        return true;
    default:
        return false;
    }
}

double Generator::GeneratorImpl::scalingFactor(const AnalyserModelPtr &analyserModel, const VariablePtr &variable) const
{
    // Return the scaling factor for the given variable, accounting for the fact that a constant may be initialised by
    // another variable which initial value may be defined in a different component.

    auto analyserVariable = analyserModel->analyserVariable(variable);

    if ((analyserVariable->type() == AnalyserVariable::Type::CONSTANT)
        && !isCellMLReal(variable->initialValue())) {
        auto variableComponent = owningComponent(variable);
        auto initialValueVariable = variableComponent->variable(variable->initialValue());
        auto initialValueAnalyserVariable = analyserModel->analyserVariable(initialValueVariable);

        if (variableComponent != owningComponent(initialValueAnalyserVariable->variable())) {
            return Units::scalingFactor(initialValueVariable->units(), variable->units());
        }
    }

    return Units::scalingFactor(analyserVariable->variable()->units(), variable->units());
}

bool Generator::GeneratorImpl::isNegativeNumber(const AnalyserEquationAstPtr &ast) const
{
    if (ast->type() == AnalyserEquationAst::Type::CN) {
        double doubleValue;

        convertToDouble(ast->value(), doubleValue);

        return doubleValue < 0.0;
    }

    return false;
}

bool Generator::GeneratorImpl::isRelationalOperator(const AnalyserEquationAstPtr &ast) const
{
    switch (ast->type()) {
    case AnalyserEquationAst::Type::EQ:
        return mProfile->hasEqOperator();
    case AnalyserEquationAst::Type::NEQ:
        return mProfile->hasNeqOperator();
    case AnalyserEquationAst::Type::LT:
        return mProfile->hasLtOperator();
    case AnalyserEquationAst::Type::LEQ:
        return mProfile->hasLeqOperator();
    case AnalyserEquationAst::Type::GT:
        return mProfile->hasGtOperator();
    case AnalyserEquationAst::Type::GEQ:
        return mProfile->hasGeqOperator();
    default:
        return false;
    }
}

bool Generator::GeneratorImpl::isAndOperator(const AnalyserEquationAstPtr &ast) const
{
    return (ast->type() == AnalyserEquationAst::Type::AND)
           && mProfile->hasAndOperator();
}

bool Generator::GeneratorImpl::isOrOperator(const AnalyserEquationAstPtr &ast) const
{
    return (ast->type() == AnalyserEquationAst::Type::OR)
           && mProfile->hasOrOperator();
}

bool Generator::GeneratorImpl::isXorOperator(const AnalyserEquationAstPtr &ast) const
{
    return (ast->type() == AnalyserEquationAst::Type::XOR)
           && mProfile->hasXorOperator();
}

bool Generator::GeneratorImpl::isLogicalOperator(const AnalyserEquationAstPtr &ast) const
{
    // Note: AnalyserEquationAst::Type::NOT is a unary logical operator, hence
    //       we don't include it here since this method is only used to
    //       determine whether parentheses should be added around some code.

    return isAndOperator(ast) || isOrOperator(ast) || isXorOperator(ast);
}

bool Generator::GeneratorImpl::isPlusOperator(const AnalyserEquationAstPtr &ast) const
{
    return ast->type() == AnalyserEquationAst::Type::PLUS;
}

bool Generator::GeneratorImpl::isMinusOperator(const AnalyserEquationAstPtr &ast) const
{
    return ast->type() == AnalyserEquationAst::Type::MINUS;
}

bool Generator::GeneratorImpl::isTimesOperator(const AnalyserEquationAstPtr &ast) const
{
    return ast->type() == AnalyserEquationAst::Type::TIMES;
}

bool Generator::GeneratorImpl::isDivideOperator(const AnalyserEquationAstPtr &ast) const
{
    return ast->type() == AnalyserEquationAst::Type::DIVIDE;
}

bool Generator::GeneratorImpl::isPowerOperator(const AnalyserEquationAstPtr &ast) const
{
    return (ast->type() == AnalyserEquationAst::Type::POWER)
           && mProfile->hasPowerOperator();
}

bool Generator::GeneratorImpl::isRootOperator(const AnalyserEquationAstPtr &ast) const
{
    return (ast->type() == AnalyserEquationAst::Type::ROOT)
           && mProfile->hasPowerOperator();
}

bool Generator::GeneratorImpl::isPiecewiseStatement(const AnalyserEquationAstPtr &ast) const
{
    return (ast->type() == AnalyserEquationAst::Type::PIECEWISE)
           && mProfile->hasConditionalOperator();
}

void Generator::GeneratorImpl::updateVariableInfoSizes(size_t &componentSize,
                                                       size_t &nameSize,
                                                       size_t &unitsSize,
                                                       const AnalyserVariablePtr &analyserVariable) const
{
    auto analyserVariableVariable = analyserVariable->variable();
    auto variableComponentSize = owningComponent(analyserVariableVariable)->name().length() + 1;
    auto variableNameSize = analyserVariableVariable->name().length() + 1;
    auto variableUnitsSize = analyserVariableVariable->units()->name().length() + 1;
    // Note: +1 to account for the end of string termination.

    componentSize = (componentSize > variableComponentSize) ? componentSize : variableComponentSize;
    nameSize = (nameSize > variableNameSize) ? nameSize : variableNameSize;
    unitsSize = (unitsSize > variableUnitsSize) ? unitsSize : variableUnitsSize;
}

bool Generator::GeneratorImpl::modifiedProfile() const
{
    std::string profileContents = generatorProfileAsString(mProfile);

    return (mProfile->profile() == GeneratorProfile::Profile::C) ?
               sha1(profileContents) != C_GENERATOR_PROFILE_SHA1 :
               sha1(profileContents) != PYTHON_GENERATOR_PROFILE_SHA1;
}

std::string Generator::GeneratorImpl::newLineIfNeeded()
{
    return mCode.empty() ? "" : "\n";
}

void Generator::GeneratorImpl::addOriginCommentCode()
{
    if (!mProfile->commentString().empty()
        && !mProfile->originCommentString().empty()) {
        std::string profileInformation = modifiedProfile() ?
                                             "a modified " :
                                             "the ";

        profileInformation += (mProfile->profile() == GeneratorProfile::Profile::C) ?
                                  "C" :
                                  "Python";
        profileInformation += " profile of";

        mCode += newLineIfNeeded()
                 + replace(mProfile->commentString(),
                           "[CODE]", replace(replace(mProfile->originCommentString(), "[PROFILE_INFORMATION]", profileInformation), "[LIBCELLML_VERSION]", versionString()));
    }
}

void Generator::GeneratorImpl::addInterfaceHeaderCode()
{
    if (!mProfile->interfaceHeaderString().empty()) {
        mCode += newLineIfNeeded()
                 + mProfile->interfaceHeaderString();
    }
}

void Generator::GeneratorImpl::addImplementationHeaderCode()
{
    auto hasInterfaceFileName = mProfile->implementationHeaderString().empty() ?
                                    false :
                                    (mProfile->implementationHeaderString().find("[INTERFACE_FILE_NAME]") != std::string::npos);

    if (!mProfile->implementationHeaderString().empty()
        && ((hasInterfaceFileName && !mProfile->interfaceFileNameString().empty())
            || !hasInterfaceFileName)) {
        mCode += newLineIfNeeded()
                 + replace(mProfile->implementationHeaderString(),
                           "[INTERFACE_FILE_NAME]", mProfile->interfaceFileNameString());
    }
}

void Generator::GeneratorImpl::addVersionAndLibcellmlVersionCode(bool interface)
{
    std::string code;

    if ((interface && !mProfile->interfaceVersionString().empty())
        || (!interface && !mProfile->implementationVersionString().empty())) {
        if (interface) {
            code += mProfile->interfaceVersionString();
        } else {
            if (modifiedProfile()) {
                static const std::regex regEx("([0-9]+\\.[0-9]+\\.[0-9]+)");

                code += std::regex_replace(mProfile->implementationVersionString(), regEx, "$1.post0");
            } else {
                code += mProfile->implementationVersionString();
            }
        }
    }

    if ((interface && !mProfile->interfaceLibcellmlVersionString().empty())
        || (!interface && !mProfile->implementationLibcellmlVersionString().empty())) {
        code += interface ?
                    mProfile->interfaceLibcellmlVersionString() :
                    replace(mProfile->implementationLibcellmlVersionString(),
                            "[LIBCELLML_VERSION]", versionString());
    }

    if (!code.empty()) {
        mCode += newLineIfNeeded()
                 + code;
    }
}

void Generator::GeneratorImpl::addStateAndVariableCountCode(bool interface)
{
    std::string code;

    if (modelHasOdes(mAnalyserModel)
        && ((interface && !mProfile->interfaceStateCountString().empty())
            || (!interface && !mProfile->implementationStateCountString().empty()))) {
        code += interface ?
                    mProfile->interfaceStateCountString() :
                    replace(mProfile->implementationStateCountString(),
                            "[STATE_COUNT]", std::to_string(mAnalyserModel->stateCount()));
    }

    if ((interface && !mProfile->interfaceConstantCountString().empty())
        || (!interface && !mProfile->implementationConstantCountString().empty())) {
        code += interface ?
                    mProfile->interfaceConstantCountString() :
                    replace(mProfile->implementationConstantCountString(),
                            "[CONSTANT_COUNT]", std::to_string((mVariableTracker != nullptr) ? mVariableTracker->trackedConstantCount(mAnalyserModel) : mAnalyserModel->constantCount()));
    }

    if ((interface && !mProfile->interfaceComputedConstantCountString().empty())
        || (!interface && !mProfile->implementationComputedConstantCountString().empty())) {
        code += interface ?
                    mProfile->interfaceComputedConstantCountString() :
                    replace(mProfile->implementationComputedConstantCountString(),
                            "[COMPUTED_CONSTANT_COUNT]", std::to_string((mVariableTracker != nullptr) ? mVariableTracker->trackedComputedConstantCount(mAnalyserModel) : mAnalyserModel->computedConstantCount()));
    }

    if ((interface && !mProfile->interfaceAlgebraicVariableCountString().empty())
        || (!interface && !mProfile->implementationAlgebraicVariableCountString().empty())) {
        code += interface ?
                    mProfile->interfaceAlgebraicVariableCountString() :
                    replace(mProfile->implementationAlgebraicVariableCountString(),
                            "[ALGEBRAIC_VARIABLE_COUNT]", std::to_string((mVariableTracker != nullptr) ? mVariableTracker->trackedAlgebraicVariableCount(mAnalyserModel) : mAnalyserModel->algebraicVariableCount()));
    }

    if ((mAnalyserModel->externalVariableCount() != 0)
        && ((interface && !mProfile->interfaceExternalVariableCountString().empty())
            || (!interface && !mProfile->implementationExternalVariableCountString().empty()))) {
        code += interface ?
                    mProfile->interfaceExternalVariableCountString() :
                    replace(mProfile->implementationExternalVariableCountString(),
                            "[EXTERNAL_VARIABLE_COUNT]", std::to_string(mAnalyserModel->externalVariableCount()));
    }

    if (!code.empty()) {
        mCode += newLineIfNeeded()
                 + code;
    }
}

std::string Generator::GeneratorImpl::generateVariableInfoObjectCode(const std::string &objectString)
{
    size_t componentSize = 0;
    size_t nameSize = 0;
    size_t unitsSize = 0;

    for (const auto &analyserVariable : analyserVariables(mAnalyserModel)) {
        if (isTrackedVariable(analyserVariable, true)) {
            updateVariableInfoSizes(componentSize, nameSize, unitsSize, analyserVariable);
        }
    }

    return replace(replace(replace(objectString,
                                   "[COMPONENT_SIZE]", std::to_string(componentSize)),
                           "[NAME_SIZE]", std::to_string(nameSize)),
                   "[UNITS_SIZE]", std::to_string(unitsSize));
}

void Generator::GeneratorImpl::addVariableInfoObjectCode()
{
    if (!mProfile->variableInfoObjectString().empty()) {
        mCode += newLineIfNeeded()
                 + generateVariableInfoObjectCode(mProfile->variableInfoObjectString());
    }
}

std::string Generator::GeneratorImpl::generateVariableInfoEntryCode(const std::string &name,
                                                                    const std::string &units,
                                                                    const std::string &component) const
{
    return replace(replace(replace(mProfile->variableInfoEntryString(),
                                   "[NAME]", name),
                           "[UNITS]", units),
                   "[COMPONENT]", component);
}

void Generator::GeneratorImpl::addInterfaceVariableInfoCode()
{
    std::string code;

    if (modelHasOdes(mAnalyserModel)
        && !mProfile->interfaceVoiInfoString().empty()) {
        code += mProfile->interfaceVoiInfoString();
    }

    if (modelHasOdes(mAnalyserModel)
        && !mProfile->interfaceStateInfoString().empty()) {
        code += mProfile->interfaceStateInfoString();
    }

    if (!mProfile->interfaceConstantInfoString().empty()) {
        code += mProfile->interfaceConstantInfoString();
    }

    if (!mProfile->interfaceComputedConstantInfoString().empty()) {
        code += mProfile->interfaceComputedConstantInfoString();
    }

    if (!mProfile->interfaceAlgebraicVariableInfoString().empty()) {
        code += mProfile->interfaceAlgebraicVariableInfoString();
    }

    if (mAnalyserModel->hasExternalVariables()
        && !mProfile->interfaceExternalVariableInfoString().empty()) {
        code += mProfile->interfaceExternalVariableInfoString();
    }

    if (!code.empty()) {
        mCode += newLineIfNeeded()
                 + code;
    }
}

void Generator::GeneratorImpl::addImplementationVariableInfoCode(const std::string &variableInfoString,
                                                                 const std::vector<AnalyserVariablePtr> &analyserVariables,
                                                                 bool voiVariable)
{
    if (!variableInfoString.empty()
        && !mProfile->variableInfoEntryString().empty()
        && !mProfile->arrayElementSeparatorString().empty()) {
        std::string infoElementsCode;

        for (const auto &analyserVariable : analyserVariables) {
            if (isTrackedVariable(analyserVariable, true)) {
                if (!infoElementsCode.empty()) {
                    infoElementsCode += mProfile->arrayElementSeparatorString() + "\n";
                }

                auto analyserVariableVariable = analyserVariable->variable();

                infoElementsCode += (voiVariable ? "" : mProfile->indentString())
                                    + generateVariableInfoEntryCode(analyserVariableVariable->name(),
                                                                    analyserVariableVariable->units()->name(),
                                                                    owningComponent(analyserVariableVariable)->name());
            }
        }

        if (!voiVariable && !infoElementsCode.empty()) {
            infoElementsCode += "\n";
        }

        mCode += newLineIfNeeded()
                 + replace(variableInfoString, "[CODE]", infoElementsCode);
    }
}

void Generator::GeneratorImpl::addImplementationVariableInfoCode()
{
    if (modelHasOdes(mAnalyserModel)) {
        addImplementationVariableInfoCode(mProfile->implementationVoiInfoString(), {mAnalyserModel->voi()}, true);
        addImplementationVariableInfoCode(mProfile->implementationStateInfoString(), mAnalyserModel->states(), false);
    }

    addImplementationVariableInfoCode(mProfile->implementationConstantInfoString(), mAnalyserModel->constants(), false);
    addImplementationVariableInfoCode(mProfile->implementationComputedConstantInfoString(), mAnalyserModel->computedConstants(), false);
    addImplementationVariableInfoCode(mProfile->implementationAlgebraicVariableInfoString(), mAnalyserModel->algebraicVariables(), false);

    if (mAnalyserModel->hasExternalVariables()) {
        addImplementationVariableInfoCode(mProfile->implementationExternalVariableInfoString(), mAnalyserModel->externalVariables(), false);
    }
}

void Generator::GeneratorImpl::addArithmeticFunctionsCode()
{
    if (mAnalyserModel->needEqFunction() && !mProfile->hasEqOperator()
        && !mProfile->eqFunctionString().empty()) {
        mCode += newLineIfNeeded()
                 + mProfile->eqFunctionString();
    }

    if (mAnalyserModel->needNeqFunction() && !mProfile->hasNeqOperator()
        && !mProfile->neqFunctionString().empty()) {
        mCode += newLineIfNeeded()
                 + mProfile->neqFunctionString();
    }

    if (mAnalyserModel->needLtFunction() && !mProfile->hasLtOperator()
        && !mProfile->ltFunctionString().empty()) {
        mCode += newLineIfNeeded()
                 + mProfile->ltFunctionString();
    }

    if (mAnalyserModel->needLeqFunction() && !mProfile->hasLeqOperator()
        && !mProfile->leqFunctionString().empty()) {
        mCode += newLineIfNeeded()
                 + mProfile->leqFunctionString();
    }

    if (mAnalyserModel->needGtFunction() && !mProfile->hasGtOperator()
        && !mProfile->gtFunctionString().empty()) {
        mCode += newLineIfNeeded()
                 + mProfile->gtFunctionString();
    }

    if (mAnalyserModel->needGeqFunction() && !mProfile->hasGeqOperator()
        && !mProfile->geqFunctionString().empty()) {
        mCode += newLineIfNeeded()
                 + mProfile->geqFunctionString();
    }

    if (mAnalyserModel->needAndFunction() && !mProfile->hasAndOperator()
        && !mProfile->andFunctionString().empty()) {
        mCode += newLineIfNeeded()
                 + mProfile->andFunctionString();
    }

    if (mAnalyserModel->needOrFunction() && !mProfile->hasOrOperator()
        && !mProfile->orFunctionString().empty()) {
        mCode += newLineIfNeeded()
                 + mProfile->orFunctionString();
    }

    if (mAnalyserModel->needXorFunction() && !mProfile->hasXorOperator()
        && !mProfile->xorFunctionString().empty()) {
        mCode += newLineIfNeeded()
                 + mProfile->xorFunctionString();
    }

    if (mAnalyserModel->needNotFunction() && !mProfile->hasNotOperator()
        && !mProfile->notFunctionString().empty()) {
        mCode += newLineIfNeeded()
                 + mProfile->notFunctionString();
    }

    if (mAnalyserModel->needMinFunction()
        && !mProfile->minFunctionString().empty()) {
        mCode += newLineIfNeeded()
                 + mProfile->minFunctionString();
    }

    if (mAnalyserModel->needMaxFunction()
        && !mProfile->maxFunctionString().empty()) {
        mCode += newLineIfNeeded()
                 + mProfile->maxFunctionString();
    }
}

void Generator::GeneratorImpl::addTrigonometricFunctionsCode()
{
    if (mAnalyserModel->needSecFunction()
        && !mProfile->secFunctionString().empty()) {
        mCode += newLineIfNeeded()
                 + mProfile->secFunctionString();
    }

    if (mAnalyserModel->needCscFunction()
        && !mProfile->cscFunctionString().empty()) {
        mCode += newLineIfNeeded()
                 + mProfile->cscFunctionString();
    }

    if (mAnalyserModel->needCotFunction()
        && !mProfile->cotFunctionString().empty()) {
        mCode += newLineIfNeeded()
                 + mProfile->cotFunctionString();
    }

    if (mAnalyserModel->needSechFunction()
        && !mProfile->sechFunctionString().empty()) {
        mCode += newLineIfNeeded()
                 + mProfile->sechFunctionString();
    }

    if (mAnalyserModel->needCschFunction()
        && !mProfile->cschFunctionString().empty()) {
        mCode += newLineIfNeeded()
                 + mProfile->cschFunctionString();
    }

    if (mAnalyserModel->needCothFunction()
        && !mProfile->cothFunctionString().empty()) {
        mCode += newLineIfNeeded()
                 + mProfile->cothFunctionString();
    }

    if (mAnalyserModel->needAsecFunction()
        && !mProfile->asecFunctionString().empty()) {
        mCode += newLineIfNeeded()
                 + mProfile->asecFunctionString();
    }

    if (mAnalyserModel->needAcscFunction()
        && !mProfile->acscFunctionString().empty()) {
        mCode += newLineIfNeeded()
                 + mProfile->acscFunctionString();
    }

    if (mAnalyserModel->needAcotFunction()
        && !mProfile->acotFunctionString().empty()) {
        mCode += newLineIfNeeded()
                 + mProfile->acotFunctionString();
    }

    if (mAnalyserModel->needAsechFunction()
        && !mProfile->asechFunctionString().empty()) {
        mCode += newLineIfNeeded()
                 + mProfile->asechFunctionString();
    }

    if (mAnalyserModel->needAcschFunction()
        && !mProfile->acschFunctionString().empty()) {
        mCode += newLineIfNeeded()
                 + mProfile->acschFunctionString();
    }

    if (mAnalyserModel->needAcothFunction()
        && !mProfile->acothFunctionString().empty()) {
        mCode += newLineIfNeeded()
                 + mProfile->acothFunctionString();
    }
}

void Generator::GeneratorImpl::addInterfaceCreateDeleteArrayMethodsCode()
{
    std::string code;

    if (modelHasOdes(mAnalyserModel)
        && !mProfile->interfaceCreateStatesArrayMethodString().empty()) {
        code += mProfile->interfaceCreateStatesArrayMethodString();
    }

    if (!mProfile->interfaceCreateConstantsArrayMethodString().empty()) {
        code += mProfile->interfaceCreateConstantsArrayMethodString();
    }

    if (!mProfile->interfaceCreateComputedConstantsArrayMethodString().empty()) {
        code += mProfile->interfaceCreateComputedConstantsArrayMethodString();
    }

    if (!mProfile->interfaceCreateAlgebraicVariablesArrayMethodString().empty()) {
        code += mProfile->interfaceCreateAlgebraicVariablesArrayMethodString();
    }

    if (mAnalyserModel->hasExternalVariables()
        && !mProfile->interfaceCreateExternalVariablesArrayMethodString().empty()) {
        code += mProfile->interfaceCreateExternalVariablesArrayMethodString();
    }

    if (!mProfile->interfaceDeleteArrayMethodString().empty()) {
        code += "\n"
                + mProfile->interfaceDeleteArrayMethodString();
    }

    if (!code.empty()) {
        mCode += newLineIfNeeded()
                 + code;
    }
}

void Generator::GeneratorImpl::addImplementationCreateDeleteArrayMethodsCode()
{
    if (modelHasOdes(mAnalyserModel)
        && !mProfile->implementationCreateStatesArrayMethodString().empty()) {
        mCode += newLineIfNeeded()
                 + mProfile->implementationCreateStatesArrayMethodString();
    }

    if (!mProfile->implementationCreateConstantsArrayMethodString().empty()) {
        mCode += newLineIfNeeded()
                 + mProfile->implementationCreateConstantsArrayMethodString();
    }

    if (!mProfile->implementationCreateComputedConstantsArrayMethodString().empty()) {
        mCode += newLineIfNeeded()
                 + mProfile->implementationCreateComputedConstantsArrayMethodString();
    }

    if (!mProfile->implementationCreateAlgebraicVariablesArrayMethodString().empty()) {
        mCode += newLineIfNeeded()
                 + mProfile->implementationCreateAlgebraicVariablesArrayMethodString();
    }

    if (mAnalyserModel->hasExternalVariables()
        && !mProfile->implementationCreateExternalVariablesArrayMethodString().empty()) {
        mCode += newLineIfNeeded()
                 + mProfile->implementationCreateExternalVariablesArrayMethodString();
    }

    if (!mProfile->implementationDeleteArrayMethodString().empty()) {
        mCode += newLineIfNeeded()
                 + mProfile->implementationDeleteArrayMethodString();
    }
}

void Generator::GeneratorImpl::addExternalVariableMethodTypeDefinitionCode()
{
    if (mAnalyserModel->hasExternalVariables()) {
        auto externalVariableMethodTypeDefinitionString = mProfile->externalVariableMethodTypeDefinitionString(modelHasOdes(mAnalyserModel));

        if (!externalVariableMethodTypeDefinitionString.empty()) {
            mCode += newLineIfNeeded()
                     + externalVariableMethodTypeDefinitionString;
        }
    }
}

void Generator::GeneratorImpl::addRootFindingInfoObjectCode()
{
    if (!mProfile->rootFindingInfoObjectString(modelHasOdes(mAnalyserModel), mAnalyserModel->hasExternalVariables()).empty()) {
        mCode += newLineIfNeeded()
                 + mProfile->rootFindingInfoObjectString(modelHasOdes(mAnalyserModel), mAnalyserModel->hasExternalVariables());
    }
}

void Generator::GeneratorImpl::addExternNlaSolveMethodCode()
{
    if (!mProfile->externNlaSolveMethodString().empty()) {
        mCode += newLineIfNeeded()
                 + mProfile->externNlaSolveMethodString();
    }
}

void Generator::GeneratorImpl::addNlaSystemsCode()
{
    if (!mProfile->objectiveFunctionMethodString(modelHasOdes(mAnalyserModel), mAnalyserModel->hasExternalVariables()).empty()
        && !mProfile->findRootMethodString(modelHasOdes(mAnalyserModel), mAnalyserModel->hasExternalVariables()).empty()
        && !mProfile->nlaSolveCallString(modelHasOdes(mAnalyserModel), mAnalyserModel->hasExternalVariables()).empty()) {
        // Note: only states and algebraic variables can be computed through an NLA system. Constants, computed
        //       constants, and external variables cannot, by definition, be computed through an NLA system.

        std::vector<AnalyserEquationPtr> handledNlaAnalyserEquations;

        for (const auto &analyserEquation : mAnalyserModel->analyserEquations()) {
            if ((analyserEquation->type() == AnalyserEquation::Type::NLA)
                && (std::find(handledNlaAnalyserEquations.begin(), handledNlaAnalyserEquations.end(), analyserEquation) == handledNlaAnalyserEquations.end())) {
                // 1) Generate some code for the objectiveFunction[INDEX]() method.
                //     a) Retrieve the values from our NLA solver's u array.

                std::string methodBody;
                auto i = MAX_SIZE_T;
                auto analyserVariables = libcellml::analyserVariables(analyserEquation);

                for (const auto &analyserVariable : analyserVariables) {
                    auto arrayString = (analyserVariable->type() == AnalyserVariable::Type::STATE) ?
                                           mProfile->ratesArrayString() :
                                           mProfile->algebraicVariablesArrayString();

                    methodBody += mProfile->indentString()
                                  + arrayString + mProfile->openArrayString() + analyserVariableIndexString(analyserVariable) + mProfile->closeArrayString()
                                  + mProfile->equalityString()
                                  + mProfile->uArrayString() + mProfile->openArrayString() + convertToString(++i) + mProfile->closeArrayString()
                                  + mProfile->commandSeparatorString() + "\n";
                }

                //     b) Initialise any untracked constant, computed constant, or algebraic variable that is needed by
                //        our NLA system.

                methodBody += "\n";

                auto methodBodySize = methodBody.size();

                for (const auto &constantDependency : analyserEquation->mPimpl->mConstantDependencies) {
                    if (isTrackedVariable(constantDependency, false)) {
                        methodBody += generateInitialisationCode(constantDependency, true);
                    }
                }

                std::vector<AnalyserEquationPtr> dummyRemainingAnalyserEquations = mAnalyserModel->analyserEquations();
                std::vector<AnalyserEquationPtr> dummyAnalyserEquationsForDependencies;
                std::vector<AnalyserVariablePtr> dummyGeneratedConstantDependencies;

                for (const auto &dependency : analyserEquation->dependencies()) {
                    if (((dependency->type() == AnalyserEquation::Type::COMPUTED_CONSTANT)
                         || (dependency->type() == AnalyserEquation::Type::ALGEBRAIC))
                        && isTrackedEquation(dependency, false)) {
                        methodBody += generateEquationCode(dependency, dummyRemainingAnalyserEquations,
                                                           dummyAnalyserEquationsForDependencies,
                                                           dummyGeneratedConstantDependencies, false,
                                                           GenerateEquationCodeTarget::OBJECTIVE_FUNCTION);
                    }
                }

                //     c) Generate our NLA system's objective functions.

                methodBody += (methodBody.size() == methodBodySize) ? "" : "\n";

                i = MAX_SIZE_T;

                methodBody += mProfile->indentString()
                              + mProfile->fArrayString() + mProfile->openArrayString() + convertToString(++i) + mProfile->closeArrayString()
                              + mProfile->equalityString()
                              + generateCode(analyserEquation->ast())
                              + mProfile->commandSeparatorString() + "\n";

                handledNlaAnalyserEquations.push_back(analyserEquation);

                for (const auto &nlaSibling : analyserEquation->nlaSiblings()) {
                    methodBody += mProfile->indentString()
                                  + mProfile->fArrayString() + mProfile->openArrayString() + convertToString(++i) + mProfile->closeArrayString()
                                  + mProfile->equalityString()
                                  + generateCode(nlaSibling->ast())
                                  + mProfile->commandSeparatorString() + "\n";

                    handledNlaAnalyserEquations.push_back(nlaSibling);
                }

                mCode += newLineIfNeeded()
                         + replace(replace(mProfile->objectiveFunctionMethodString(modelHasOdes(mAnalyserModel), mAnalyserModel->hasExternalVariables()),
                                           "[INDEX]", convertToString(analyserEquation->nlaSystemIndex())),
                                   "[CODE]", generateMethodBodyCode(methodBody));

                // 2) Generate some code for the findRoot[INDEX]() method.
                //     a) Assign the values to our NLA solver's u array.

                methodBody = {};

                i = MAX_SIZE_T;

                for (const auto &analyserVariable : analyserVariables) {
                    auto arrayString = (analyserVariable->type() == AnalyserVariable::Type::STATE) ?
                                           mProfile->ratesArrayString() :
                                           mProfile->algebraicVariablesArrayString();

                    methodBody += mProfile->indentString()
                                  + mProfile->uArrayString() + mProfile->openArrayString() + convertToString(++i) + mProfile->closeArrayString()
                                  + mProfile->equalityString()
                                  + arrayString + mProfile->openArrayString() + analyserVariableIndexString(analyserVariable) + mProfile->closeArrayString()
                                  + mProfile->commandSeparatorString() + "\n";
                }

                //     b) Call our NLA solver.

                auto analyserVariablesCount = analyserVariables.size();

                methodBody += "\n"
                              + mProfile->indentString()
                              + replace(replace(mProfile->nlaSolveCallString(modelHasOdes(mAnalyserModel), mAnalyserModel->hasExternalVariables()),
                                                "[INDEX]", convertToString(analyserEquation->nlaSystemIndex())),
                                        "[SIZE]", convertToString(analyserVariablesCount));

                //     c) Retrieve the values from our NLA solver's u array.

                i = MAX_SIZE_T;

                methodBody += "\n";

                for (const auto &analyserVariable : analyserVariables) {
                    auto arrayString = (analyserVariable->type() == AnalyserVariable::Type::STATE) ?
                                           mProfile->ratesArrayString() :
                                           mProfile->algebraicVariablesArrayString();

                    methodBody += mProfile->indentString()
                                  + arrayString + mProfile->openArrayString() + analyserVariableIndexString(analyserVariable) + mProfile->closeArrayString()
                                  + mProfile->equalityString()
                                  + mProfile->uArrayString() + mProfile->openArrayString() + convertToString(++i) + mProfile->closeArrayString()
                                  + mProfile->commandSeparatorString() + "\n";
                }

                mCode += newLineIfNeeded()
                         + replace(replace(replace(mProfile->findRootMethodString(modelHasOdes(mAnalyserModel), mAnalyserModel->hasExternalVariables()),
                                                   "[INDEX]", convertToString(analyserEquation->nlaSystemIndex())),
                                           "[SIZE]", convertToString(analyserVariablesCount)),
                                   "[CODE]", generateMethodBodyCode(methodBody));
            }
        }
    }
}

std::string Generator::GeneratorImpl::generateMethodBodyCode(const std::string &methodBody) const
{
    return methodBody.empty() ?
               mProfile->emptyMethodString().empty() ?
               "" :
               mProfile->indentString() + mProfile->emptyMethodString() :
               methodBody;
}

std::string generateDoubleCode(const std::string &value)
{
    if (value.find('.') != std::string::npos) {
        return value;
    }

    auto ePos = value.find_first_of("eE");

    if (ePos == std::string::npos) {
        return value + ".0";
    }

    return value.substr(0, ePos) + ".0" + value.substr(ePos);
}

std::string Generator::GeneratorImpl::generateDoubleOrVariableNameCode(const VariablePtr &variable)
{
    if (isCellMLReal(variable->initialValue())) {
        return generateDoubleCode(variable->initialValue());
    }

    auto initialValueVariable = owningComponent(variable)->variable(variable->initialValue());
    auto initialValueAnalyserVariable = mAnalyserModel->analyserVariable(initialValueVariable);
    std::string arrayName;

    switch (initialValueAnalyserVariable->type()) {
    case AnalyserVariable::Type::STATE:
        arrayName = mProfile->statesArrayString();

        break;
    case AnalyserVariable::Type::CONSTANT:
        arrayName = mProfile->constantsArrayString();

        break;
    case AnalyserVariable::Type::COMPUTED_CONSTANT:
        arrayName = mProfile->computedConstantsArrayString();

        break;
    default: // If it is not one of the above types then it has to be an algebraic variable.
        arrayName = mProfile->algebraicVariablesArrayString();

        break;
    }

    return arrayName + mProfile->openArrayString() + analyserVariableIndexString(initialValueAnalyserVariable) + mProfile->closeArrayString();
}

std::string Generator::GeneratorImpl::generateVariableNameCode(const VariablePtr &variable, bool state)
{
    // Generate some code for a variable name, but only if we have an analyser model. If we don't have an analyser
    // model, it means that we are using the generator from the analyser, in which case we just want to return the
    // original name of the variable.

    if (mAnalyserModel == nullptr) {
        return variable->name();
    }

    auto analyserVariable = mAnalyserModel->analyserVariable(variable);

    if (analyserVariable->type() == AnalyserVariable::Type::VARIABLE_OF_INTEGRATION) {
        return mProfile->voiString();
    }

    if (isTrackedVariable(analyserVariable, false)) {
        return owningComponent(analyserVariable->variable())->name() + "_" + analyserVariable->variable()->name();
    }

    std::string arrayName;

    if (analyserVariable->type() == AnalyserVariable::Type::STATE) {
        arrayName = state ?
                        mProfile->statesArrayString() :
                        mProfile->ratesArrayString();
    } else if (analyserVariable->type() == AnalyserVariable::Type::CONSTANT) {
        arrayName = mProfile->constantsArrayString();
    } else if (analyserVariable->type() == AnalyserVariable::Type::COMPUTED_CONSTANT) {
        arrayName = mProfile->computedConstantsArrayString();
    } else if (analyserVariable->type() == AnalyserVariable::Type::ALGEBRAIC_VARIABLE) {
        arrayName = mProfile->algebraicVariablesArrayString();
    } else {
        arrayName = mProfile->externalVariablesArrayString();
    }

    return arrayName + mProfile->openArrayString() + analyserVariableIndexString(analyserVariable) + mProfile->closeArrayString();
}

std::string Generator::GeneratorImpl::generateOperatorCode(const std::string &op, const AnalyserEquationAstPtr &ast)
{
    // Generate the code for the left and right branches of the given AST.

    std::string res;
    auto astLeftChild = ast->leftChild();
    auto astRightChild = ast->rightChild();
    auto astLeftChildCode = generateCode(astLeftChild);
    auto astRightChildCode = generateCode(astRightChild);

    // Determine whether parentheses should be added around the left and/or
    // right piece of code, and this based on the precedence of the operators
    // used in CellML, which are listed below from higher to lower precedence:
    //  1. Parentheses                                           [Left to right]
    //  2. POWER (as an operator, not as a function, i.e.        [Left to right]
    //            as in Matlab and not in C, for example)
    //  3. Unary PLUS, Unary MINUS, NOT                          [Right to left]
    //  4. TIMES, DIVIDE                                         [Left to right]
    //  5. PLUS, MINUS                                           [Left to right]
    //  6. LT, LEQ, GT, GEQ                                      [Left to right]
    //  7. EQ, NEQ                                               [Left to right]
    //  8. XOR (bitwise)                                         [Left to right]
    //  9. AND (logical)                                         [Left to right]
    // 10. OR (logical)                                          [Left to right]
    // 11. PIECEWISE (as an operator)                            [Right to left]

    if (isPlusOperator(ast)) {
        if (isRelationalOperator(astLeftChild)
            || isLogicalOperator(astLeftChild)
            || isPiecewiseStatement(astLeftChild)) {
            astLeftChildCode = "(" + astLeftChildCode + ")";
        }

        if (isRelationalOperator(astRightChild)
            || isLogicalOperator(astRightChild)
            || isPiecewiseStatement(astRightChild)) {
            astRightChildCode = "(" + astRightChildCode + ")";
        }
    } else if (isMinusOperator(ast)) {
        if (isRelationalOperator(astLeftChild)
            || isLogicalOperator(astLeftChild)
            || isPiecewiseStatement(astLeftChild)) {
            astLeftChildCode = "(" + astLeftChildCode + ")";
        }

        if (isNegativeNumber(astRightChild)
            || isRelationalOperator(astRightChild)
            || isLogicalOperator(astRightChild)
            || isMinusOperator(astRightChild)
            || isPiecewiseStatement(astRightChild)
            || (astRightChildCode.rfind(mProfile->minusString(), 0) == 0)) {
            astRightChildCode = "(" + astRightChildCode + ")";
        } else if (isPlusOperator(astRightChild)) {
            if (astRightChild->rightChild() != nullptr) {
                astRightChildCode = "(" + astRightChildCode + ")";
            }
        }
    } else if (isTimesOperator(ast)) {
        if (isRelationalOperator(astLeftChild)
            || isLogicalOperator(astLeftChild)
            || isPiecewiseStatement(astLeftChild)) {
            astLeftChildCode = "(" + astLeftChildCode + ")";
        } else if (isPlusOperator(astLeftChild)
                   || isMinusOperator(astLeftChild)) {
            if (astLeftChild->rightChild() != nullptr) {
                astLeftChildCode = "(" + astLeftChildCode + ")";
            }
        }

        if (isRelationalOperator(astRightChild)
            || isLogicalOperator(astRightChild)
            || isPiecewiseStatement(astRightChild)) {
            astRightChildCode = "(" + astRightChildCode + ")";
        } else if (isPlusOperator(astRightChild)
                   || isMinusOperator(astRightChild)) {
            if (astRightChild->rightChild() != nullptr) {
                astRightChildCode = "(" + astRightChildCode + ")";
            }
        }
    } else if (isDivideOperator(ast)) {
        if (isRelationalOperator(astLeftChild)
            || isLogicalOperator(astLeftChild)
            || isPiecewiseStatement(astLeftChild)) {
            astLeftChildCode = "(" + astLeftChildCode + ")";
        } else if (isPlusOperator(astLeftChild)
                   || isMinusOperator(astLeftChild)) {
            if (astLeftChild->rightChild() != nullptr) {
                astLeftChildCode = "(" + astLeftChildCode + ")";
            }
        }

        if (isRelationalOperator(astRightChild)
            || isLogicalOperator(astRightChild)
            || isTimesOperator(astRightChild)
            || isDivideOperator(astRightChild)
            || isPiecewiseStatement(astRightChild)) {
            astRightChildCode = "(" + astRightChildCode + ")";
        } else if (isPlusOperator(astRightChild)
                   || isMinusOperator(astRightChild)) {
            if (astRightChild->rightChild() != nullptr) {
                astRightChildCode = "(" + astRightChildCode + ")";
            }
        }
    } else if (isAndOperator(ast)) {
        // Note: according to the precedence rules above, we only need to
        //       add parentheses around OR and PIECEWISE. However, it looks
        //       better/clearer to have some around some other operators
        //       (agreed, this is somewhat subjective).

        if (isRelationalOperator(astLeftChild)
            || isOrOperator(astLeftChild)
            || isXorOperator(astLeftChild)
            || isPiecewiseStatement(astLeftChild)) {
            astLeftChildCode = "(" + astLeftChildCode + ")";
        } else if (isPlusOperator(astLeftChild)
                   || isMinusOperator(astLeftChild)) {
            if (astLeftChild->rightChild() != nullptr) {
                astLeftChildCode = "(" + astLeftChildCode + ")";
            }
        } else if (isPowerOperator(astLeftChild)) {
            astLeftChildCode = "(" + astLeftChildCode + ")";
        } else if (isRootOperator(astLeftChild)) {
            astLeftChildCode = "(" + astLeftChildCode + ")";
        }

        if (isRelationalOperator(astRightChild)
            || isOrOperator(astRightChild)
            || isXorOperator(astRightChild)
            || isPiecewiseStatement(astRightChild)) {
            astRightChildCode = "(" + astRightChildCode + ")";
        } else if (isPlusOperator(astRightChild)
                   || isMinusOperator(astRightChild)) {
            if (astRightChild->rightChild() != nullptr) {
                astRightChildCode = "(" + astRightChildCode + ")";
            }
        } else if (isPowerOperator(astRightChild)) {
            astRightChildCode = "(" + astRightChildCode + ")";
        } else if (isRootOperator(astRightChild)) {
            astRightChildCode = "(" + astRightChildCode + ")";
        }
    } else if (isOrOperator(ast)) {
        // Note: according to the precedence rules above, we only need to
        //       add parentheses around PIECEWISE. However, it looks
        //       better/clearer to have some around some other operators
        //       (agreed, this is somewhat subjective).

        if (isRelationalOperator(astLeftChild)
            || isAndOperator(astLeftChild)
            || isXorOperator(astLeftChild)
            || isPiecewiseStatement(astLeftChild)) {
            astLeftChildCode = "(" + astLeftChildCode + ")";
        } else if (isPlusOperator(astLeftChild)
                   || isMinusOperator(astLeftChild)) {
            if (astLeftChild->rightChild() != nullptr) {
                astLeftChildCode = "(" + astLeftChildCode + ")";
            }
        } else if (isPowerOperator(astLeftChild)) {
            astLeftChildCode = "(" + astLeftChildCode + ")";
        } else if (isRootOperator(astLeftChild)) {
            astLeftChildCode = "(" + astLeftChildCode + ")";
        }

        if (isRelationalOperator(astRightChild)
            || isAndOperator(astRightChild)
            || isXorOperator(astRightChild)
            || isPiecewiseStatement(astRightChild)) {
            astRightChildCode = "(" + astRightChildCode + ")";
        } else if (isPlusOperator(astRightChild)
                   || isMinusOperator(astRightChild)) {
            if (astRightChild->rightChild() != nullptr) {
                astRightChildCode = "(" + astRightChildCode + ")";
            }
        } else if (isPowerOperator(astRightChild)) {
            astRightChildCode = "(" + astRightChildCode + ")";
        } else if (isRootOperator(astRightChild)) {
            astRightChildCode = "(" + astRightChildCode + ")";
        }
    } else if (isXorOperator(ast)) {
        // Note: according to the precedence rules above, we only need to
        //       add parentheses around AND, OR and PIECEWISE. However, it
        //       looks better/clearer to have some around some other
        //       operators (agreed, this is somewhat subjective).

        if (isRelationalOperator(astLeftChild)
            || isAndOperator(astLeftChild)
            || isOrOperator(astLeftChild)
            || isPiecewiseStatement(astLeftChild)) {
            astLeftChildCode = "(" + astLeftChildCode + ")";
        } else if (isPlusOperator(astLeftChild)
                   || isMinusOperator(astLeftChild)) {
            if (astLeftChild->rightChild() != nullptr) {
                astLeftChildCode = "(" + astLeftChildCode + ")";
            }
        } else if (isPowerOperator(astLeftChild)) {
            astLeftChildCode = "(" + astLeftChildCode + ")";
        } else if (isRootOperator(astLeftChild)) {
            astLeftChildCode = "(" + astLeftChildCode + ")";
        }

        if (isRelationalOperator(astRightChild)
            || isAndOperator(astRightChild)
            || isOrOperator(astRightChild)
            || isPiecewiseStatement(astRightChild)) {
            astRightChildCode = "(" + astRightChildCode + ")";
        } else if (isPlusOperator(astRightChild)
                   || isMinusOperator(astRightChild)) {
            if (astRightChild->rightChild() != nullptr) {
                astRightChildCode = "(" + astRightChildCode + ")";
            }
        } else if (isPowerOperator(astRightChild)) {
            astRightChildCode = "(" + astRightChildCode + ")";
        } else if (isRootOperator(astRightChild)) {
            astRightChildCode = "(" + astRightChildCode + ")";
        }
    } else if (isPowerOperator(ast)) {
        if (isRelationalOperator(astLeftChild)
            || isLogicalOperator(astLeftChild)
            || isMinusOperator(astLeftChild)
            || isTimesOperator(astLeftChild)
            || isDivideOperator(astLeftChild)
            || isPiecewiseStatement(astLeftChild)) {
            astLeftChildCode = "(" + astLeftChildCode + ")";
        } else if (isPlusOperator(astLeftChild)) {
            if (astLeftChild->rightChild() != nullptr) {
                astLeftChildCode = "(" + astLeftChildCode + ")";
            }
        }

        if (isRelationalOperator(astRightChild)
            || isLogicalOperator(astRightChild)
            || isMinusOperator(astLeftChild)
            || isTimesOperator(astRightChild)
            || isDivideOperator(astRightChild)
            || isPowerOperator(astRightChild)
            || isRootOperator(astRightChild)
            || isPiecewiseStatement(astRightChild)) {
            astRightChildCode = "(" + astRightChildCode + ")";
        } else if (isPlusOperator(astRightChild)) {
            if (astRightChild->rightChild() != nullptr) {
                astRightChildCode = "(" + astRightChildCode + ")";
            }
        }
    } else if (isRootOperator(ast)) {
        if (isRelationalOperator(astRightChild)
            || isLogicalOperator(astRightChild)
            || isMinusOperator(astRightChild)
            || isTimesOperator(astRightChild)
            || isDivideOperator(astRightChild)
            || isPiecewiseStatement(astRightChild)) {
            astRightChildCode = "(" + astRightChildCode + ")";
        } else if (isPlusOperator(astRightChild)) {
            if (astRightChild->rightChild() != nullptr) {
                astRightChildCode = "(" + astRightChildCode + ")";
            }
        }

        auto astLeftChildLeftChild = astLeftChild->leftChild();

        if (isRelationalOperator(astLeftChildLeftChild)
            || isLogicalOperator(astLeftChildLeftChild)
            || isMinusOperator(astLeftChildLeftChild)
            || isTimesOperator(astLeftChildLeftChild)
            || isDivideOperator(astLeftChildLeftChild)
            || isPowerOperator(astLeftChildLeftChild)
            || isRootOperator(astLeftChildLeftChild)
            || isPiecewiseStatement(astLeftChildLeftChild)) {
            astLeftChildCode = "(" + astLeftChildCode + ")";
        } else if (isPlusOperator(astLeftChildLeftChild)) {
            if (astLeftChildLeftChild->rightChild() != nullptr) {
                astLeftChildCode = "(" + astLeftChildCode + ")";
            }
        }

        return astRightChildCode + op + "(1.0/" + astLeftChildCode + ")";
    }

    return astLeftChildCode + op + astRightChildCode;
}

std::string Generator::GeneratorImpl::generateMinusUnaryCode(const AnalyserEquationAstPtr &ast)
{
    // Generate the code for the left branch of the given AST.

    auto astLeftChild = ast->leftChild();
    auto code = generateCode(astLeftChild);

    // Determine whether parentheses should be added around the left code.

    if (isRelationalOperator(astLeftChild)
        || isLogicalOperator(astLeftChild)
        || isPlusOperator(astLeftChild)
        || isMinusOperator(astLeftChild)
        || isPiecewiseStatement(astLeftChild)) {
        code = "(" + code + ")";
    }

    return mProfile->minusString() + code;
}

std::string Generator::GeneratorImpl::generateOneParameterFunctionCode(const std::string &function,
                                                                       const AnalyserEquationAstPtr &ast)
{
    return function + "(" + generateCode(ast->leftChild()) + ")";
}

std::string Generator::GeneratorImpl::generateTwoParameterFunctionCode(const std::string &function,
                                                                       const AnalyserEquationAstPtr &ast)
{
    return function + "(" + generateCode(ast->leftChild()) + ", " + generateCode(ast->rightChild()) + ")";
}

std::string Generator::GeneratorImpl::generatePiecewiseIfCode(const std::string &condition,
                                                              const std::string &value) const
{
    return replace(replace(mProfile->hasConditionalOperator() ?
                               mProfile->conditionalOperatorIfString() :
                               mProfile->piecewiseIfString(),
                           "[CONDITION]", condition),
                   "[IF_STATEMENT]", value);
}

std::string Generator::GeneratorImpl::generatePiecewiseElseCode(const std::string &value) const
{
    return replace(mProfile->hasConditionalOperator() ?
                       mProfile->conditionalOperatorElseString() :
                       mProfile->piecewiseElseString(),
                   "[ELSE_STATEMENT]", value);
}

std::string Generator::GeneratorImpl::generateCode(const AnalyserEquationAstPtr &ast)
{
    // Generate the code for the given AST.
    // Note: AnalyserEquationAst::Type::BVAR is only relevant when there is no analyser model (in which case we want to
    //       generate something like dx/dt, as is in the case of the analyser when we want to mention an equation) since
    //       otherwise we don't need to generate any code for it (since we will, instead, want to generate something
    //       like rates[0]).

    std::string code;

    switch (ast->type()) {
    case AnalyserEquationAst::Type::EQUALITY:
        code = generateOperatorCode(mProfile->equalityString(), ast);

        break;
    case AnalyserEquationAst::Type::EQ:
        if (mProfile->hasEqOperator()) {
            code = generateOperatorCode(mProfile->eqString(), ast);
        } else {
            code = generateTwoParameterFunctionCode(mProfile->eqString(), ast);
        }

        break;
    case AnalyserEquationAst::Type::NEQ:
        if (mProfile->hasNeqOperator()) {
            code = generateOperatorCode(mProfile->neqString(), ast);
        } else {
            code = generateTwoParameterFunctionCode(mProfile->neqString(), ast);
        }

        break;
    case AnalyserEquationAst::Type::LT:
        if (mProfile->hasLtOperator()) {
            code = generateOperatorCode(mProfile->ltString(), ast);
        } else {
            code = generateTwoParameterFunctionCode(mProfile->ltString(), ast);
        }

        break;
    case AnalyserEquationAst::Type::LEQ:
        if (mProfile->hasLeqOperator()) {
            code = generateOperatorCode(mProfile->leqString(), ast);
        } else {
            code = generateTwoParameterFunctionCode(mProfile->leqString(), ast);
        }

        break;
    case AnalyserEquationAst::Type::GT:
        if (mProfile->hasGtOperator()) {
            code = generateOperatorCode(mProfile->gtString(), ast);
        } else {
            code = generateTwoParameterFunctionCode(mProfile->gtString(), ast);
        }

        break;
    case AnalyserEquationAst::Type::GEQ:
        if (mProfile->hasGeqOperator()) {
            code = generateOperatorCode(mProfile->geqString(), ast);
        } else {
            code = generateTwoParameterFunctionCode(mProfile->geqString(), ast);
        }

        break;
    case AnalyserEquationAst::Type::AND:
        if (mProfile->hasAndOperator()) {
            code = generateOperatorCode(mProfile->andString(), ast);
        } else {
            code = generateTwoParameterFunctionCode(mProfile->andString(), ast);
        }

        break;
    case AnalyserEquationAst::Type::OR:
        if (mProfile->hasOrOperator()) {
            code = generateOperatorCode(mProfile->orString(), ast);
        } else {
            code = generateTwoParameterFunctionCode(mProfile->orString(), ast);
        }

        break;
    case AnalyserEquationAst::Type::XOR:
        if (mProfile->hasXorOperator()) {
            code = generateOperatorCode(mProfile->xorString(), ast);
        } else {
            code = generateTwoParameterFunctionCode(mProfile->xorString(), ast);
        }

        break;
    case AnalyserEquationAst::Type::NOT:
        if (mProfile->hasNotOperator()) {
            code = mProfile->notString() + generateCode(ast->leftChild());
        } else {
            code = generateOneParameterFunctionCode(mProfile->notString(), ast);
        }

        break;
    case AnalyserEquationAst::Type::PLUS:
        if (ast->rightChild() != nullptr) {
            code = generateOperatorCode(mProfile->plusString(), ast);
        } else {
            code = generateCode(ast->leftChild());
        }

        break;
    case AnalyserEquationAst::Type::MINUS:
        if (ast->rightChild() != nullptr) {
            code = generateOperatorCode(mProfile->minusString(), ast);
        } else {
            code = generateMinusUnaryCode(ast);
        }

        break;
    case AnalyserEquationAst::Type::TIMES:
        code = generateOperatorCode(mProfile->timesString(), ast);

        break;
    case AnalyserEquationAst::Type::DIVIDE:
        code = generateOperatorCode(mProfile->divideString(), ast);

        break;
    case AnalyserEquationAst::Type::POWER: {
        auto stringValue = generateCode(ast->rightChild());
        double doubleValue;
        auto validConversion = convertToDouble(stringValue, doubleValue);

        if (validConversion && areEqual(doubleValue, 0.5)) {
            code = generateOneParameterFunctionCode(mProfile->squareRootString(), ast);
        } else if (validConversion && areEqual(doubleValue, 2.0)
                   && !mProfile->squareString().empty()) {
            code = generateOneParameterFunctionCode(mProfile->squareString(), ast);
        } else {
            code = mProfile->hasPowerOperator() ?
                       generateOperatorCode(mProfile->powerString(), ast) :
                       mProfile->powerString() + "(" + generateCode(ast->leftChild()) + ", " + stringValue + ")";
        }
    } break;
    case AnalyserEquationAst::Type::ROOT: {
        auto astRightChild = ast->rightChild();

        if (astRightChild != nullptr) {
            auto astLeftChild = ast->leftChild();
            double doubleValue;

            if (convertToDouble(generateCode(astLeftChild), doubleValue)
                && areEqual(doubleValue, 2.0)) {
                code = mProfile->squareRootString() + "(" + generateCode(astRightChild) + ")";
            } else {
                if (mProfile->hasPowerOperator()) {
                    code = generateOperatorCode(mProfile->powerString(), ast);
                } else {
                    auto rootValueAst = AnalyserEquationAst::create();

                    rootValueAst->setType(AnalyserEquationAst::Type::DIVIDE);
                    rootValueAst->setParent(ast);

                    auto leftChild = AnalyserEquationAst::create();

                    leftChild->setType(AnalyserEquationAst::Type::CN);
                    leftChild->setValue("1.0");
                    leftChild->setParent(rootValueAst);

                    rootValueAst->setLeftChild(leftChild);
                    rootValueAst->setRightChild(astLeftChild->leftChild());

                    code = mProfile->powerString() + "(" + generateCode(astRightChild) + ", " + generateOperatorCode(mProfile->divideString(), rootValueAst) + ")";
                }
            }
        } else {
            code = generateOneParameterFunctionCode(mProfile->squareRootString(), ast);
        }
    } break;
    case AnalyserEquationAst::Type::ABS:
        code = generateOneParameterFunctionCode(mProfile->absoluteValueString(), ast);

        break;
    case AnalyserEquationAst::Type::EXP:
        code = generateOneParameterFunctionCode(mProfile->exponentialString(), ast);

        break;
    case AnalyserEquationAst::Type::LN:
        code = generateOneParameterFunctionCode(mProfile->naturalLogarithmString(), ast);

        break;
    case AnalyserEquationAst::Type::LOG: {
        auto astRightChild = ast->rightChild();

        if (astRightChild != nullptr) {
            auto stringValue = generateCode(ast->leftChild());
            double doubleValue;

            if (convertToDouble(stringValue, doubleValue)
                && areEqual(doubleValue, 10.0)) {
                code = mProfile->commonLogarithmString() + "(" + generateCode(astRightChild) + ")";
            } else {
                code = mProfile->naturalLogarithmString() + "(" + generateCode(astRightChild) + ")/" + mProfile->naturalLogarithmString() + "(" + stringValue + ")";
            }
        } else {
            code = generateOneParameterFunctionCode(mProfile->commonLogarithmString(), ast);
        }
    } break;
    case AnalyserEquationAst::Type::CEILING:
        code = generateOneParameterFunctionCode(mProfile->ceilingString(), ast);

        break;
    case AnalyserEquationAst::Type::FLOOR:
        code = generateOneParameterFunctionCode(mProfile->floorString(), ast);

        break;
    case AnalyserEquationAst::Type::MIN:
        code = generateTwoParameterFunctionCode(mProfile->minString(), ast);

        break;
    case AnalyserEquationAst::Type::MAX:
        code = generateTwoParameterFunctionCode(mProfile->maxString(), ast);

        break;
    case AnalyserEquationAst::Type::REM:
        code = generateTwoParameterFunctionCode(mProfile->remString(), ast);

        break;
    case AnalyserEquationAst::Type::DIFF:
        if (mAnalyserModel != nullptr) {
            code = generateCode(ast->rightChild());
        } else {
            code = "d" + generateCode(ast->rightChild()) + "/d" + generateCode(ast->leftChild());
        }

        break;
    case AnalyserEquationAst::Type::SIN:
        code = generateOneParameterFunctionCode(mProfile->sinString(), ast);

        break;
    case AnalyserEquationAst::Type::COS:
        code = generateOneParameterFunctionCode(mProfile->cosString(), ast);

        break;
    case AnalyserEquationAst::Type::TAN:
        code = generateOneParameterFunctionCode(mProfile->tanString(), ast);

        break;
    case AnalyserEquationAst::Type::SEC:
        code = generateOneParameterFunctionCode(mProfile->secString(), ast);

        break;
    case AnalyserEquationAst::Type::CSC:
        code = generateOneParameterFunctionCode(mProfile->cscString(), ast);

        break;
    case AnalyserEquationAst::Type::COT:
        code = generateOneParameterFunctionCode(mProfile->cotString(), ast);

        break;
    case AnalyserEquationAst::Type::SINH:
        code = generateOneParameterFunctionCode(mProfile->sinhString(), ast);

        break;
    case AnalyserEquationAst::Type::COSH:
        code = generateOneParameterFunctionCode(mProfile->coshString(), ast);

        break;
    case AnalyserEquationAst::Type::TANH:
        code = generateOneParameterFunctionCode(mProfile->tanhString(), ast);

        break;
    case AnalyserEquationAst::Type::SECH:
        code = generateOneParameterFunctionCode(mProfile->sechString(), ast);

        break;
    case AnalyserEquationAst::Type::CSCH:
        code = generateOneParameterFunctionCode(mProfile->cschString(), ast);

        break;
    case AnalyserEquationAst::Type::COTH:
        code = generateOneParameterFunctionCode(mProfile->cothString(), ast);

        break;
    case AnalyserEquationAst::Type::ASIN:
        code = generateOneParameterFunctionCode(mProfile->asinString(), ast);

        break;
    case AnalyserEquationAst::Type::ACOS:
        code = generateOneParameterFunctionCode(mProfile->acosString(), ast);

        break;
    case AnalyserEquationAst::Type::ATAN:
        code = generateOneParameterFunctionCode(mProfile->atanString(), ast);

        break;
    case AnalyserEquationAst::Type::ASEC:
        code = generateOneParameterFunctionCode(mProfile->asecString(), ast);

        break;
    case AnalyserEquationAst::Type::ACSC:
        code = generateOneParameterFunctionCode(mProfile->acscString(), ast);

        break;
    case AnalyserEquationAst::Type::ACOT:
        code = generateOneParameterFunctionCode(mProfile->acotString(), ast);

        break;
    case AnalyserEquationAst::Type::ASINH:
        code = generateOneParameterFunctionCode(mProfile->asinhString(), ast);

        break;
    case AnalyserEquationAst::Type::ACOSH:
        code = generateOneParameterFunctionCode(mProfile->acoshString(), ast);

        break;
    case AnalyserEquationAst::Type::ATANH:
        code = generateOneParameterFunctionCode(mProfile->atanhString(), ast);

        break;
    case AnalyserEquationAst::Type::ASECH:
        code = generateOneParameterFunctionCode(mProfile->asechString(), ast);

        break;
    case AnalyserEquationAst::Type::ACSCH:
        code = generateOneParameterFunctionCode(mProfile->acschString(), ast);

        break;
    case AnalyserEquationAst::Type::ACOTH:
        code = generateOneParameterFunctionCode(mProfile->acothString(), ast);

        break;
    case AnalyserEquationAst::Type::PIECEWISE: {
        auto astRightChild = ast->rightChild();

        if (astRightChild != nullptr) {
            if (astRightChild->type() == AnalyserEquationAst::Type::PIECE) {
                code = generateCode(ast->leftChild()) + generatePiecewiseElseCode(generateCode(astRightChild) + generatePiecewiseElseCode(mProfile->nanString()));
            } else {
                code = generateCode(ast->leftChild()) + generatePiecewiseElseCode(generateCode(astRightChild));
            }
        } else {
            code = generateCode(ast->leftChild()) + generatePiecewiseElseCode(mProfile->nanString());
        }
    } break;
    case AnalyserEquationAst::Type::PIECE:
        code = generatePiecewiseIfCode(generateCode(ast->rightChild()), generateCode(ast->leftChild()));

        break;
    case AnalyserEquationAst::Type::OTHERWISE:
        code = generateCode(ast->leftChild());

        break;
    case AnalyserEquationAst::Type::CI: {
        code = generateVariableNameCode(ast->variable(), ast->parent()->type() != AnalyserEquationAst::Type::DIFF);

        auto astParent = ast->parent();

        if ((mAnalyserModel != nullptr)
            && (astParent->type() == AnalyserEquationAst::Type::EQUALITY)
            && (astParent->leftChild() == ast)
            && isTrackedVariable(mAnalyserModel->analyserVariable(ast->variable()), false)) {
            // Note: we want this AST to be its parent's left child since a declaration is always of the form x = RHS,
            //       not LHS = x.

            code = replace(mProfile->variableDeclarationString(), "[CODE]", code);
        }
    }

    break;
    case AnalyserEquationAst::Type::CN:
        code = generateDoubleCode(ast->value());

        break;
    case AnalyserEquationAst::Type::DEGREE:
    case AnalyserEquationAst::Type::LOGBASE:
        code = generateCode(ast->leftChild());

        break;
    case AnalyserEquationAst::Type::BVAR:
        code = generateCode(ast->leftChild());

        break;
    case AnalyserEquationAst::Type::TRUE:
        code = mProfile->trueString();

        break;
    case AnalyserEquationAst::Type::FALSE:
        code = mProfile->falseString();

        break;
    case AnalyserEquationAst::Type::E:
        code = mProfile->eString();

        break;
    case AnalyserEquationAst::Type::PI:
        code = mProfile->piString();

        break;
    case AnalyserEquationAst::Type::INF:
        code = mProfile->infString();

        break;
    default: // AnalyserEquationAst::Type::NAN.
        code = mProfile->nanString();

        break;
    }

    return code;
}

bool Generator::GeneratorImpl::isToBeComputedAgain(const AnalyserEquationPtr &analyserEquation)
{
    // NLA and algebraic equations that are state/rate-based and external equations are to be computed again (in the
    // computeVariables() method) unless the variables they compute are not tracked.

    switch (analyserEquation->type()) {
    case AnalyserEquation::Type::NLA:
    case AnalyserEquation::Type::ALGEBRAIC:
        if (analyserEquation->isStateRateBased()) {
            for (const auto &analyserVariable : analyserVariables(analyserEquation)) {
                if (isTrackedVariable(analyserVariable, true)) {
                    return true;
                }
            }

            return false;
        }

        return false;
    case AnalyserEquation::Type::EXTERNAL:
        return isTrackedEquation(analyserEquation, true);
    default:
        return false;
    }
}

bool Generator::GeneratorImpl::isSomeConstant(const AnalyserEquationPtr &analyserEquation,
                                              bool includeComputedConstants) const
{
    return (analyserEquation->type() == AnalyserEquation::Type::CONSTANT)
           || (!includeComputedConstants && (analyserEquation->type() == AnalyserEquation::Type::COMPUTED_CONSTANT));
}

std::string Generator::GeneratorImpl::generateZeroInitialisationCode(const AnalyserVariablePtr &analyserVariable)
{
    return mProfile->indentString()
           + generateVariableNameCode(analyserVariable->variable(), false)
           + mProfile->equalityString()
           + "0.0"
           + mProfile->commandSeparatorString() + "\n";
}

std::string Generator::GeneratorImpl::generateInitialisationCode(const AnalyserVariablePtr &analyserVariable, bool force)
{
    if (!force && isTrackedVariable(analyserVariable, false)) {
        return {};
    }

    auto initialisingVariable = analyserVariable->initialisingVariable();
    auto scalingFactor = Generator::GeneratorImpl::scalingFactor(mAnalyserModel, initialisingVariable);
    std::string scalingFactorCode;

    if (!areNearlyEqual(scalingFactor, 1.0)) {
        scalingFactorCode = generateDoubleCode(convertToString(scalingFactor)) + mProfile->timesString();
    }

    auto code = generateVariableNameCode(analyserVariable->variable())
                + mProfile->equalityString()
                + scalingFactorCode + generateDoubleOrVariableNameCode(initialisingVariable)
                + mProfile->commandSeparatorString() + "\n";

    if (isTrackedVariable(analyserVariable, false)) {
        code = replace(mProfile->variableDeclarationString(), "[CODE]", code);
    }

    return mProfile->indentString()
           + code;
}

std::string Generator::GeneratorImpl::generateEquationCode(const AnalyserEquationPtr &analyserEquation,
                                                           std::vector<AnalyserEquationPtr> &remainingAnalyserEquations,
                                                           std::vector<AnalyserEquationPtr> &analyserEquationsForDependencies,
                                                           std::vector<AnalyserVariablePtr> &generatedConstantDependencies,
                                                           bool includeComputedConstants,
                                                           GenerateEquationCodeTarget target)
{
    std::string res;

    if (std::find(remainingAnalyserEquations.begin(), remainingAnalyserEquations.end(), analyserEquation) != remainingAnalyserEquations.end()) {
        // Stop tracking the analyser equation and its NLA siblings, if any.
        // Note: we need to do this as soon as possible to avoid recursive
        //       calls, something that would happen if we were to do this at the
        //       end of this if statement.

        remainingAnalyserEquations.erase(std::find(remainingAnalyserEquations.begin(), remainingAnalyserEquations.end(), analyserEquation));

        for (const auto &nlaSibling : analyserEquation->nlaSiblings()) {
            remainingAnalyserEquations.erase(std::find(remainingAnalyserEquations.begin(), remainingAnalyserEquations.end(), nlaSibling));
        }

        // Generate any dependency that this analyser equation may have.

        for (const auto &constantDependency : analyserEquation->mPimpl->mConstantDependencies) {
            if ((analyserEquation->type() != AnalyserEquation::Type::NLA)
                && isTrackedVariable(constantDependency, false)
                && (std::find(generatedConstantDependencies.begin(), generatedConstantDependencies.end(), constantDependency) == generatedConstantDependencies.end())) {
                res += generateInitialisationCode(constantDependency, true);

                generatedConstantDependencies.push_back(constantDependency);
            }
        }

        if (!isSomeConstant(analyserEquation, includeComputedConstants)) {
            for (const auto &dependency : analyserEquation->dependencies()) {
                if (((analyserEquation->type() != AnalyserEquation::Type::NLA)
                     && (dependency->type() == AnalyserEquation::Type::COMPUTED_CONSTANT)
                     && isTrackedEquation(dependency, false))
                    || (((target == GenerateEquationCodeTarget::NORMAL)
                         || ((target == GenerateEquationCodeTarget::COMPUTE_VARIABLES)
                             && ((dependency->type() != AnalyserEquation::Type::NLA)
                                 || isToBeComputedAgain(dependency)
                                 || (std::find(analyserEquationsForDependencies.begin(), analyserEquationsForDependencies.end(), dependency) != analyserEquationsForDependencies.end()))))
                        && (dependency->type() != AnalyserEquation::Type::ODE)
                        && (isTrackedEquation(dependency, true)
                            || (analyserEquation->type() != AnalyserEquation::Type::NLA))
                        && !isSomeConstant(dependency, includeComputedConstants)
                        && (analyserEquationsForDependencies.empty()
                            || isToBeComputedAgain(dependency)
                            || (std::find(analyserEquationsForDependencies.begin(), analyserEquationsForDependencies.end(), dependency) != analyserEquationsForDependencies.end())))) {
                    res += generateEquationCode(dependency, remainingAnalyserEquations, analyserEquationsForDependencies,
                                                generatedConstantDependencies, includeComputedConstants, target);
                }
            }
        }

        // Generate the analyser equation code itself, based on the analyser equation type.

        switch (analyserEquation->type()) {
        case AnalyserEquation::Type::EXTERNAL:
            for (const auto &analyserVariable : analyserVariables(analyserEquation)) {
                res += mProfile->indentString()
                       + generateVariableNameCode(analyserVariable->variable())
                       + mProfile->equalityString()
                       + replace(mProfile->externalVariableMethodCallString(modelHasOdes(mAnalyserModel)),
                                 "[INDEX]", analyserVariableIndexString(analyserVariable))
                       + mProfile->commandSeparatorString() + "\n";
            }

            break;
        case AnalyserEquation::Type::NLA: {
            auto modelHasOdes = this->modelHasOdes(mAnalyserModel);

            if (!mProfile->findRootCallString(modelHasOdes, mAnalyserModel->hasExternalVariables()).empty()) {
                res += mProfile->indentString()
                       + replace(mProfile->findRootCallString(modelHasOdes, mAnalyserModel->hasExternalVariables()),
                                 "[INDEX]", convertToString(analyserEquation->nlaSystemIndex()));
            }
        } break;
        default:
            res += mProfile->indentString() + generateCode(analyserEquation->ast()) + mProfile->commandSeparatorString() + "\n";

            break;
        }
    }

    return res;
}

std::string Generator::GeneratorImpl::generateEquationCode(const AnalyserEquationPtr &analyserEquation,
                                                           std::vector<AnalyserEquationPtr> &remainingAnalyserEquations,
                                                           std::vector<AnalyserVariablePtr> &generatedConstantDependencies)
{
    std::vector<AnalyserEquationPtr> dummyAnalyserEquationsForComputeVariables;

    return generateEquationCode(analyserEquation, remainingAnalyserEquations, dummyAnalyserEquationsForComputeVariables,
                                generatedConstantDependencies, true);
}

bool Generator::GeneratorImpl::hasComputedConstantDependency(const AnalyserVariablePtr &analyserVariable)
{
    // Check if the analyser variable has a direct or indirect dependency on a computed constant.

    if (analyserVariable->type() == AnalyserVariable::Type::COMPUTED_CONSTANT) {
        return true;
    }

    auto initialisingVariable = analyserVariable->initialisingVariable();
    auto initialValueVariable = owningComponent(initialisingVariable)->variable(initialisingVariable->initialValue());

    if (initialValueVariable == nullptr) {
        return false;
    }

    return hasComputedConstantDependency(mAnalyserModel->analyserVariable(initialValueVariable));
}

std::string Generator::GeneratorImpl::generateInitialiseVariableCode(const AnalyserVariablePtr &analyserVariable,
                                                                     std::vector<AnalyserEquationPtr> &remainingAnalyserEquations,
                                                                     std::vector<AnalyserVariablePtr> &remainingStates,
                                                                     std::vector<AnalyserVariablePtr> &remainingConstants,
                                                                     std::vector<AnalyserVariablePtr> &remainingComputedConstants,
                                                                     std::vector<AnalyserVariablePtr> &remainingAlgebraicVariables,
                                                                     std::vector<AnalyserVariablePtr> *generatedConstantDependencies)
{
    std::string res;

    // Check if the analyser variable is initialised using a constant value or an initialising variable.

    auto initialisingVariable = analyserVariable->initialisingVariable();
    auto initialValueVariable = (initialisingVariable != nullptr) ? owningComponent(initialisingVariable)->variable(initialisingVariable->initialValue()) : nullptr;
    auto initialiseAnalyserVariable = true;

    if (initialValueVariable != nullptr) {
        // The initial value references a state, a constant, a computed constant, or an algebraic variable, so generate
        // initialisation code for that variable first, if conditions are met.

        auto initialValueAnalyserVariable = mAnalyserModel->analyserVariable(initialValueVariable);
        auto &remainingVariables = (initialValueAnalyserVariable->type() == AnalyserVariable::Type::STATE) ?
                                       remainingStates :
                                   (initialValueAnalyserVariable->type() == AnalyserVariable::Type::CONSTANT) ?
                                       remainingConstants :
                                   (initialValueAnalyserVariable->type() == AnalyserVariable::Type::COMPUTED_CONSTANT) ?
                                       remainingComputedConstants :
                                       remainingAlgebraicVariables;

        if (((generatedConstantDependencies == nullptr) && !hasComputedConstantDependency(initialValueAnalyserVariable))
            || (generatedConstantDependencies != nullptr)) {
            auto initialisingAnalyserVariable = std::find_if(remainingVariables.begin(), remainingVariables.end(),
                                                             [&](const AnalyserVariablePtr &av) {
                                                                 return mAnalyserModel->areEquivalentVariables(initialValueVariable, av->variable());
                                                             });

            if (initialisingAnalyserVariable != remainingVariables.end()) {
                res += generateInitialiseVariableCode(AnalyserVariablePtr(*initialisingAnalyserVariable),
                                                      remainingAnalyserEquations, remainingStates, remainingConstants,
                                                      remainingComputedConstants, remainingAlgebraicVariables,
                                                      generatedConstantDependencies);
            }
        } else {
            initialiseAnalyserVariable = false;
        }
    }

    // Now initialise the analyser variable itself, if we can.

    if (initialiseAnalyserVariable) {
        auto &remainingVariables = (analyserVariable->type() == AnalyserVariable::Type::STATE) ?
                                       remainingStates :
                                   (analyserVariable->type() == AnalyserVariable::Type::CONSTANT) ?
                                       remainingConstants :
                                   (analyserVariable->type() == AnalyserVariable::Type::COMPUTED_CONSTANT) ?
                                       remainingComputedConstants :
                                       remainingAlgebraicVariables;
        auto remainingVariable = std::find(remainingVariables.begin(), remainingVariables.end(), analyserVariable);

        if (remainingVariable != remainingVariables.end()) {
            if (remainingVariables != remainingComputedConstants) {
                res += generateInitialisationCode(AnalyserVariablePtr(*remainingVariable));
            } else {
                res += generateEquationCode(analyserVariable->analyserEquation(0), remainingAnalyserEquations, *generatedConstantDependencies);
            }

            remainingVariables.erase(remainingVariable);
        }
    }

    return res;
}

void Generator::GeneratorImpl::addInterfaceComputeModelMethodsCode()
{
    auto interfaceInitialiseArraysMethodString = mProfile->interfaceInitialiseArraysMethodString(modelHasOdes(mAnalyserModel));
    std::string code;

    if (!interfaceInitialiseArraysMethodString.empty()) {
        code += interfaceInitialiseArraysMethodString;
    }

    if (!mProfile->interfaceComputeComputedConstantsMethodString(modelHasOdes(mAnalyserModel)).empty()) {
        code += mProfile->interfaceComputeComputedConstantsMethodString(modelHasOdes(mAnalyserModel));
    }

    auto interfaceComputeRatesMethodString = mProfile->interfaceComputeRatesMethodString(mAnalyserModel->hasExternalVariables());

    if (modelHasOdes(mAnalyserModel)
        && !interfaceComputeRatesMethodString.empty()) {
        code += interfaceComputeRatesMethodString;
    }

    auto interfaceComputeVariablesMethodString = mProfile->interfaceComputeVariablesMethodString(modelHasOdes(mAnalyserModel),
                                                                                                 mAnalyserModel->hasExternalVariables());

    if (!interfaceComputeVariablesMethodString.empty()) {
        code += interfaceComputeVariablesMethodString;
    }

    if (!code.empty()) {
        mCode += newLineIfNeeded()
                 + code;
    }
}

void Generator::GeneratorImpl::addImplementationInitialiseArraysMethodCode(std::vector<AnalyserEquationPtr> &remainingAnalyserEquations,
                                                                           std::vector<AnalyserVariablePtr> &remainingStates,
                                                                           std::vector<AnalyserVariablePtr> &remainingConstants,
                                                                           std::vector<AnalyserVariablePtr> &remainingComputedConstants,
                                                                           std::vector<AnalyserVariablePtr> &remainingAlgebraicVariables)
{
    // Note: we must always generate the method body (even if we don't end up generating the method itself) because
    //       addImplementationComputeComputedConstantsMethodCode() expects our "remaining" parameters to be updated
    //       correctly.

    // Initialise our states.

    std::string methodBody;

    for (const auto &state : mAnalyserModel->states()) {
        methodBody += generateInitialiseVariableCode(state,
                                                     remainingAnalyserEquations, remainingStates, remainingConstants,
                                                     remainingComputedConstants, remainingAlgebraicVariables);
    }

    // Use an initial guess of zero for rates computed using an NLA system (see the note below).

    for (const auto &state : mAnalyserModel->states()) {
        if (state->analyserEquation(0)->type() == AnalyserEquation::Type::NLA) {
            methodBody += generateZeroInitialisationCode(state);
        }
    }

    // Initialise our remaining constants.

    while (!remainingConstants.empty()) {
        methodBody += generateInitialiseVariableCode(AnalyserVariablePtr(*remainingConstants.begin()),
                                                     remainingAnalyserEquations, remainingStates, remainingConstants,
                                                     remainingComputedConstants, remainingAlgebraicVariables);
    }

    // Initialise our computed constants that are initialised using an equation (e.g., x = 3 rather than x with an
    // initial value of 3).

    std::vector<AnalyserVariablePtr> generatedConstantDependencies;

    for (const auto &equation : mAnalyserModel->analyserEquations()) {
        if (equation->type() == AnalyserEquation::Type::CONSTANT) {
            methodBody += generateEquationCode(equation, remainingAnalyserEquations, generatedConstantDependencies);
        }
    }

    // Initialise our algebraic variables that have an initial value. Also use an initial guess of zero for algebraic
    // variables computed using an NLA system.
    // Note: a variable which is the only unknown in an equation, but which is not on its own on either the LHS or RHS
    //       of that equation (e.g., x = y+z with x and y known and z unknown) is (currently) to be computed using an
    //       NLA system for which we need an initial guess. We use an initial guess of zero, which is fine since such an
    //       NLA system has only one solution.

    for (const auto &algebraicVariable : mAnalyserModel->algebraicVariables()) {
        if (algebraicVariable->initialisingVariable() != nullptr) {
            methodBody += generateInitialiseVariableCode(algebraicVariable,
                                                         remainingAnalyserEquations, remainingStates, remainingConstants,
                                                         remainingComputedConstants, remainingAlgebraicVariables);
        } else if (algebraicVariable->analyserEquation(0)->type() == AnalyserEquation::Type::NLA) {
            methodBody += generateZeroInitialisationCode(algebraicVariable);
        }
    }

    // Generate the method itself, if needed.

    auto implementationInitialiseArraysMethodString = mProfile->implementationInitialiseArraysMethodString(modelHasOdes(mAnalyserModel));

    if (!implementationInitialiseArraysMethodString.empty()) {
        mCode += newLineIfNeeded()
                 + replace(implementationInitialiseArraysMethodString,
                           "[CODE]", generateMethodBodyCode(methodBody));
    }
}

void Generator::GeneratorImpl::addImplementationComputeComputedConstantsMethodCode(std::vector<AnalyserEquationPtr> &remainingAnalyserEquations,
                                                                                   std::vector<AnalyserVariablePtr> &remainingStates,
                                                                                   std::vector<AnalyserVariablePtr> &remainingConstants,
                                                                                   std::vector<AnalyserVariablePtr> &remainingComputedConstants,
                                                                                   std::vector<AnalyserVariablePtr> &remainingAlgebraicVariables)
{
    if (!mProfile->implementationComputeComputedConstantsMethodString(modelHasOdes(mAnalyserModel)).empty()) {
        // Initialise our remaining states (which are initialised using a computed constant).

        std::string methodBody;
        std::vector<AnalyserVariablePtr> generatedConstantDependencies;

        for (const auto &state : mAnalyserModel->states()) {
            methodBody += generateInitialiseVariableCode(state,
                                                         remainingAnalyserEquations, remainingStates, remainingConstants,
                                                         remainingComputedConstants, remainingAlgebraicVariables,
                                                         &generatedConstantDependencies);
        }

        // Initialise our remaining computed constants.

        for (const auto &analyserEquation : mAnalyserModel->analyserEquations()) {
            if ((analyserEquation->type() == AnalyserEquation::Type::COMPUTED_CONSTANT)
                && isTrackedEquation(analyserEquation, true)) {
                methodBody += generateInitialiseVariableCode(analyserEquation->computedConstant(0),
                                                             remainingAnalyserEquations, remainingStates, remainingConstants,
                                                             remainingComputedConstants, remainingAlgebraicVariables,
                                                             &generatedConstantDependencies);
            }
        }

        // Initialise our algebraic variables that are initialised using a computed constant.

        for (const auto &algebraicVariable : mAnalyserModel->algebraicVariables()) {
            if (algebraicVariable->initialisingVariable() != nullptr) {
                methodBody += generateInitialiseVariableCode(algebraicVariable,
                                                             remainingAnalyserEquations, remainingStates, remainingConstants,
                                                             remainingComputedConstants, remainingAlgebraicVariables,
                                                             &generatedConstantDependencies);
            }
        }

        mCode += newLineIfNeeded()
                 + replace(mProfile->implementationComputeComputedConstantsMethodString(modelHasOdes(mAnalyserModel)),
                           "[CODE]", generateMethodBodyCode(methodBody));
    }
}

void Generator::GeneratorImpl::addImplementationComputeRatesMethodCode(std::vector<AnalyserEquationPtr> &remainingAnalyserEquations)
{
    auto implementationComputeRatesMethodString = mProfile->implementationComputeRatesMethodString(mAnalyserModel->hasExternalVariables());

    if (modelHasOdes(mAnalyserModel)
        && !implementationComputeRatesMethodString.empty()) {
        std::string methodBody;
        std::vector<AnalyserVariablePtr> generatedConstantDependencies;

        for (const auto &analyserEquation : mAnalyserModel->analyserEquations()) {
            // A rate is computed either through an ODE equation or through an
            // NLA equation in case the rate is not on its own on either the LHS
            // or RHS of the equation.

            auto analyserVariables = libcellml::analyserVariables(analyserEquation);

            if ((analyserEquation->type() == AnalyserEquation::Type::ODE)
                || ((analyserEquation->type() == AnalyserEquation::Type::NLA)
                    && (analyserVariables.size() == 1)
                    && (analyserVariables[0]->type() == AnalyserVariable::Type::STATE))) {
                methodBody += generateEquationCode(analyserEquation, remainingAnalyserEquations, generatedConstantDependencies);
            }
        }

        mCode += newLineIfNeeded()
                 + replace(implementationComputeRatesMethodString,
                           "[CODE]", generateMethodBodyCode(methodBody));
    }
}

void Generator::GeneratorImpl::addImplementationComputeVariablesMethodCode(std::vector<AnalyserEquationPtr> &remainingAnalyserEquations)
{
    auto implementationComputeVariablesMethodString = mProfile->implementationComputeVariablesMethodString(modelHasOdes(mAnalyserModel),
                                                                                                           mAnalyserModel->hasExternalVariables());

    if (!implementationComputeVariablesMethodString.empty()) {
        std::string methodBody;
        auto analyserEquations = mAnalyserModel->analyserEquations();
        auto newRemainingAnalyserEquations = analyserEquations;
        std::vector<AnalyserVariablePtr> generatedConstantDependencies;

        for (const auto &analyserEquation : analyserEquations) {
            if (((std::find(remainingAnalyserEquations.begin(), remainingAnalyserEquations.end(), analyserEquation) != remainingAnalyserEquations.end())
                 || isToBeComputedAgain(analyserEquation))
                && isTrackedEquation(analyserEquation, true)) {
                methodBody += generateEquationCode(analyserEquation, newRemainingAnalyserEquations, remainingAnalyserEquations,
                                                   generatedConstantDependencies, false,
                                                   GenerateEquationCodeTarget::COMPUTE_VARIABLES);
            }
        }

        mCode += newLineIfNeeded()
                 + replace(implementationComputeVariablesMethodString,
                           "[CODE]", generateMethodBodyCode(methodBody));
    }
}

Generator::GeneratorImpl *Generator::pFunc()
{
    return reinterpret_cast<Generator::GeneratorImpl *>(Logger::pFunc());
}

/*TODO
const Generator::GeneratorImpl *Generator::pFunc() const
{
    return reinterpret_cast<Generator::GeneratorImpl const *>(Logger::pFunc());
}
*/

Generator::Generator()
    : Logger(new GeneratorImpl())
{
}

Generator::~Generator()
{
    delete pFunc();
}

GeneratorPtr Generator::create() noexcept
{
    return std::shared_ptr<Generator> {new Generator {}};
}

std::string Generator::interfaceCode(const AnalyserModelPtr &analyserModel, const GeneratorProfilePtr &generatorProfile,
                                     const GeneratorVariableTrackerPtr &generatorVariableTracker)
{
    if ((analyserModel == nullptr)
        || (generatorProfile == nullptr)
        || !analyserModel->isValid()
        || !generatorProfile->hasInterface()) {
        return {};
    }

    // Keep track of some objects.

    pFunc()->mAnalyserModel = analyserModel;
    pFunc()->mProfile = generatorProfile;
    pFunc()->mVariableTracker = generatorVariableTracker;

    // Get ourselves ready.

    pFunc()->reset();

    // Add code for the origin comment.

    pFunc()->addOriginCommentCode();

    // Add code for the header.

    pFunc()->addInterfaceHeaderCode();

    // Add code for the interface of the version of the profile and libCellML.

    pFunc()->addVersionAndLibcellmlVersionCode(true);

    // Add code for the interface of the number of states and variables.

    pFunc()->addStateAndVariableCountCode(true);

    // Add code for the variable information related objects.

    pFunc()->addVariableInfoObjectCode();

    // Add code for the interface of the information about the variable of integration, states, constants, computed
    // constants, algebraic variables, and external variables.

    pFunc()->addInterfaceVariableInfoCode();

    // Add code for the interface to create and delete arrays.

    pFunc()->addInterfaceCreateDeleteArrayMethodsCode();

    // Add code for the external variable method type definition.

    pFunc()->addExternalVariableMethodTypeDefinitionCode();

    // Add code for the interface to compute the model.

    pFunc()->addInterfaceComputeModelMethodsCode();

    pFunc()->mAnalyserModel = nullptr;

    return pFunc()->mCode;
}

std::string Generator::interfaceCode(const AnalyserModelPtr &analyserModel, const GeneratorProfilePtr &generatorProfile)
{
    return interfaceCode(analyserModel, generatorProfile, nullptr);
}

std::string Generator::interfaceCode(const AnalyserModelPtr &analyserModel, GeneratorProfile::Profile profile)
{
    return interfaceCode(analyserModel, GeneratorProfile::create(profile), nullptr);
}

std::string Generator::interfaceCode(const AnalyserModelPtr &analyserModel,
                                     const GeneratorVariableTrackerPtr &generatorVariableTracker)
{
    return interfaceCode(analyserModel, pFunc()->mDefaultProfile, generatorVariableTracker);
}

std::string Generator::interfaceCode(const AnalyserModelPtr &analyserModel)
{
    return interfaceCode(analyserModel, pFunc()->mDefaultProfile, nullptr);
}

std::string Generator::implementationCode(const AnalyserModelPtr &analyserModel,
                                          const GeneratorProfilePtr &generatorProfile,
                                          const GeneratorVariableTrackerPtr &generatorVariableTracker)
{
    if ((analyserModel == nullptr)
        || (generatorProfile == nullptr)
        || !analyserModel->isValid()) {
        return {};
    }

    // Keep track of some objects.

    pFunc()->mAnalyserModel = analyserModel;
    pFunc()->mProfile = generatorProfile;
    pFunc()->mVariableTracker = generatorVariableTracker;

    // Get ourselves ready.

    pFunc()->reset();

    // Add code for the origin comment.

    pFunc()->addOriginCommentCode();

    // Add code for the header.

    pFunc()->addImplementationHeaderCode();

    // Add code for the implementation of the version of the profile and
    // libCellML.

    pFunc()->addVersionAndLibcellmlVersionCode();

    // Add code for the implementation of the number of states and variables.

    pFunc()->addStateAndVariableCountCode();

    // Add code for the variable information related objects.

    if (!pFunc()->mProfile->hasInterface()) {
        pFunc()->addVariableInfoObjectCode();
    }

    // Add code for the implementation of the information about the variable of integration, states, constants, computed
    // constants, algebraic variables, and external variables.

    pFunc()->addImplementationVariableInfoCode();

    // Add code for the arithmetic and trigonometric functions.

    pFunc()->addArithmeticFunctionsCode();
    pFunc()->addTrigonometricFunctionsCode();

    // Add code for the implementation to create and delete arrays.

    pFunc()->addImplementationCreateDeleteArrayMethodsCode();

    // Add code for the NLA solver.

    auto needNlaSolving = false;

    for (const auto &analyserEquation : pFunc()->mAnalyserModel->analyserEquations()) {
        if (analyserEquation->type() == AnalyserEquation::Type::NLA) {
            needNlaSolving = true;

            break;
        }
    }

    if (needNlaSolving) {
        pFunc()->addRootFindingInfoObjectCode();
        pFunc()->addExternNlaSolveMethodCode();
        pFunc()->addNlaSystemsCode();
    }

    // Add code for the implementation to initialise our arrays.

    auto remainingAnalyserEquations = pFunc()->mAnalyserModel->analyserEquations();
    auto remainingStates = pFunc()->mAnalyserModel->states();
    auto remainingConstants = pFunc()->mAnalyserModel->constants();
    auto remainingComputedConstants = pFunc()->mAnalyserModel->computedConstants();
    auto remainingAlgebraicVariables = pFunc()->mAnalyserModel->algebraicVariables();

    pFunc()->addImplementationInitialiseArraysMethodCode(remainingAnalyserEquations, remainingStates,
                                                         remainingConstants, remainingComputedConstants,
                                                         remainingAlgebraicVariables);

    // Add code for the implementation to compute our computed constants.

    pFunc()->addImplementationComputeComputedConstantsMethodCode(remainingAnalyserEquations, remainingStates,
                                                                 remainingConstants, remainingComputedConstants,
                                                                 remainingAlgebraicVariables);

    // Add code for the implementation to compute our rates (and any variables
    // on which they depend).

    pFunc()->addImplementationComputeRatesMethodCode(remainingAnalyserEquations);

    // Add code for the implementation to compute our variables.
    // Note: this method computes the remaining variables, i.e. the ones not
    //       needed to compute our rates, but also the variables that depend on
    //       the value of some states/rates and all the external variables.
    //       This method is typically called after having integrated a model,
    //       thus ensuring that variables that rely on the value of some
    //       states/rates are up to date.

    pFunc()->addImplementationComputeVariablesMethodCode(remainingAnalyserEquations);

    pFunc()->mAnalyserModel = nullptr;

    return pFunc()->mCode;
}

std::string Generator::implementationCode(const AnalyserModelPtr &analyserModel, const GeneratorProfilePtr &generatorProfile)
{
    return implementationCode(analyserModel, generatorProfile, nullptr);
}

std::string Generator::implementationCode(const AnalyserModelPtr &analyserModel, GeneratorProfile::Profile profile)
{
    return implementationCode(analyserModel, GeneratorProfile::create(profile), nullptr);
}

std::string Generator::implementationCode(const AnalyserModelPtr &analyserModel, const GeneratorVariableTrackerPtr &generatorVariableTracker)
{
    return implementationCode(analyserModel, pFunc()->mDefaultProfile, generatorVariableTracker);
}

std::string Generator::implementationCode(const AnalyserModelPtr &analyserModel)
{
    return implementationCode(analyserModel, pFunc()->mDefaultProfile, nullptr);
}

std::string Generator::equationCode(const AnalyserEquationAstPtr &ast,
                                    const GeneratorProfilePtr &generatorProfile)
{
    GeneratorPtr generator = Generator::create();

    generator->pFunc()->mProfile = (generatorProfile != nullptr) ? generatorProfile : generator->pFunc()->mDefaultProfile;

    return generator->pFunc()->generateCode(ast);
}

std::string Generator::equationCode(const AnalyserEquationAstPtr &ast)
{
    return Generator::equationCode(ast, nullptr);
}

} // namespace libcellml
