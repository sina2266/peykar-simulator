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

#pragma once

#include "libcellml/generator.h"

#include "libcellml/generatorprofile.h"

#include "issue_p.h"
#include "logger_p.h"
#include "utilities.h"

namespace libcellml {

std::string generateDoubleCode(const std::string &value);

/**
 * @brief The Generator::GeneratorImpl struct.
 *
 * The private implementation for the Generator class.
 */
struct Generator::GeneratorImpl: public Logger::LoggerImpl
{
    std::string mCode;

    AnalyserModelPtr mAnalyserModel;
    GeneratorProfilePtr mDefaultProfile = GeneratorProfile::create();
    GeneratorProfilePtr mProfile;
    GeneratorVariableTrackerPtr mVariableTracker;

    void reset();

    std::string analyserVariableIndexString(const AnalyserVariablePtr &analyserVariable);

    bool isTrackedEquation(const AnalyserEquationPtr &analyserEquation, bool tracked);
    bool isTrackedVariable(const AnalyserVariablePtr &analyserVariable, bool tracked);

    bool modelHasOdes(const AnalyserModelPtr &analyserModel) const;

    double scalingFactor(const AnalyserModelPtr &analyserModel, const VariablePtr &variable) const;

    bool isNegativeNumber(const AnalyserEquationAstPtr &ast) const;

    bool isRelationalOperator(const AnalyserEquationAstPtr &ast) const;
    bool isAndOperator(const AnalyserEquationAstPtr &ast) const;
    bool isOrOperator(const AnalyserEquationAstPtr &ast) const;
    bool isXorOperator(const AnalyserEquationAstPtr &ast) const;
    bool isLogicalOperator(const AnalyserEquationAstPtr &ast) const;
    bool isPlusOperator(const AnalyserEquationAstPtr &ast) const;
    bool isMinusOperator(const AnalyserEquationAstPtr &ast) const;
    bool isTimesOperator(const AnalyserEquationAstPtr &ast) const;
    bool isDivideOperator(const AnalyserEquationAstPtr &ast) const;
    bool isPowerOperator(const AnalyserEquationAstPtr &ast) const;
    bool isRootOperator(const AnalyserEquationAstPtr &ast) const;
    bool isPiecewiseStatement(const AnalyserEquationAstPtr &ast) const;

    void updateVariableInfoSizes(size_t &componentSize, size_t &nameSize,
                                 size_t &unitsSize,
                                 const AnalyserVariablePtr &analyserVariable) const;

    bool modifiedProfile() const;

    std::string newLineIfNeeded();

    void addOriginCommentCode();

    void addInterfaceHeaderCode();
    void addImplementationHeaderCode();

    void addVersionAndLibcellmlVersionCode(bool interface = false);

    void addStateAndVariableCountCode(bool interface = false);

    std::string generateVariableInfoObjectCode(const std::string &objectString);

    void addVariableInfoObjectCode();

    std::string generateVariableInfoEntryCode(const std::string &name,
                                              const std::string &units,
                                              const std::string &component) const;

    void addInterfaceVariableInfoCode();

    void addImplementationVariableInfoCode(const std::string &variableInfoString,
                                           const std::vector<AnalyserVariablePtr> &analyserVariables, bool voiVariable);
    void addImplementationVariableInfoCode();

    void addArithmeticFunctionsCode();
    void addTrigonometricFunctionsCode();

    void addInterfaceCreateDeleteArrayMethodsCode();
    void addImplementationCreateDeleteArrayMethodsCode();

    void addExternalVariableMethodTypeDefinitionCode();

    void addRootFindingInfoObjectCode();
    void addExternNlaSolveMethodCode();
    void addNlaSystemsCode();

    std::string generateMethodBodyCode(const std::string &methodBody) const;

    std::string generateDoubleOrVariableNameCode(const VariablePtr &variable);
    std::string generateVariableNameCode(const VariablePtr &variable, bool state = true);

    std::string generateOperatorCode(const std::string &op, const AnalyserEquationAstPtr &ast);
    std::string generateMinusUnaryCode(const AnalyserEquationAstPtr &ast);
    std::string generateOneParameterFunctionCode(const std::string &function, const AnalyserEquationAstPtr &ast);
    std::string generateTwoParameterFunctionCode(const std::string &function, const AnalyserEquationAstPtr &ast);
    std::string generatePiecewiseIfCode(const std::string &condition, const std::string &value) const;
    std::string generatePiecewiseElseCode(const std::string &value) const;
    std::string generateCode(const AnalyserEquationAstPtr &ast);

    bool isToBeComputedAgain(const AnalyserEquationPtr &analyserEquation);
    bool isSomeConstant(const AnalyserEquationPtr &analyserEquation,
                        bool includeComputedConstants) const;

    enum class GenerateEquationCodeTarget
    {
        NORMAL,
        OBJECTIVE_FUNCTION,
        COMPUTE_VARIABLES
    };

    std::string generateZeroInitialisationCode(const AnalyserVariablePtr &analyserVariable);
    std::string generateInitialisationCode(const AnalyserVariablePtr &analyserVariable, bool force = false);
    std::string generateEquationCode(const AnalyserEquationPtr &analyserEquation,
                                     std::vector<AnalyserEquationPtr> &remainingAnalyserEquations,
                                     std::vector<AnalyserEquationPtr> &analyserEquationsForDependencies,
                                     std::vector<AnalyserVariablePtr> &generatedConstantDependencies,
                                     bool includeComputedConstants,
                                     GenerateEquationCodeTarget target = GenerateEquationCodeTarget::NORMAL);
    std::string generateEquationCode(const AnalyserEquationPtr &analyserEquation,
                                     std::vector<AnalyserEquationPtr> &remainingAnalyserEquations,
                                     std::vector<AnalyserVariablePtr> &generatedConstantDependencies);
    bool hasComputedConstantDependency(const AnalyserVariablePtr &analyserVariable);
    std::string generateInitialiseVariableCode(const AnalyserVariablePtr &analyserVariable,
                                               std::vector<AnalyserEquationPtr> &remainingAnalyserEquations,
                                               std::vector<AnalyserVariablePtr> &remainingStates,
                                               std::vector<AnalyserVariablePtr> &remainingConstants,
                                               std::vector<AnalyserVariablePtr> &remainingComputedConstants,
                                               std::vector<AnalyserVariablePtr> &remainingAlgebraicVariables,
                                               std::vector<AnalyserVariablePtr> *generatedConstantDependencies = nullptr);

    void addInterfaceComputeModelMethodsCode();
    void addImplementationInitialiseArraysMethodCode(std::vector<AnalyserEquationPtr> &remainingAnalyserEquations,
                                                     std::vector<AnalyserVariablePtr> &remainingStates,
                                                     std::vector<AnalyserVariablePtr> &remainingConstants,
                                                     std::vector<AnalyserVariablePtr> &remainingComputedConstants,
                                                     std::vector<AnalyserVariablePtr> &remainingAlgebraicVariables);
    void addImplementationComputeComputedConstantsMethodCode(std::vector<AnalyserEquationPtr> &remainingAnalyserEquations,
                                                             std::vector<AnalyserVariablePtr> &remainingStates,
                                                             std::vector<AnalyserVariablePtr> &remainingConstants,
                                                             std::vector<AnalyserVariablePtr> &remainingComputedConstants,
                                                             std::vector<AnalyserVariablePtr> &remainingAlgebraicVariables);
    void addImplementationComputeRatesMethodCode(std::vector<AnalyserEquationPtr> &remainingAnalyserEquations);
    void addImplementationComputeVariablesMethodCode(std::vector<AnalyserEquationPtr> &remainingAnalyserEquations);
};

} // namespace libcellml
