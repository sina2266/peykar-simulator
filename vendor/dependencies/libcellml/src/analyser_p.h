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

#include "libcellml/analyser.h"
#include "libcellml/generatorprofile.h"
#include "libcellml/issue.h"

#include "analysermodel_p.h"
#include "internaltypes.h"
#include "logger_p.h"
#include "utilities.h"

namespace libcellml {

struct AnalyserInternalEquation;
struct AnalyserInternalVariable;

using AnalyserInternalEquationWeakPtr = std::weak_ptr<AnalyserInternalEquation>;
using AnalyserInternalEquationPtr = std::shared_ptr<AnalyserInternalEquation>;
using AnalyserInternalVariablePtr = std::shared_ptr<AnalyserInternalVariable>;

using AnalyserInternalEquationWeakPtrs = std::vector<AnalyserInternalEquationWeakPtr>;
using AnalyserInternalEquationPtrs = std::vector<AnalyserInternalEquationPtr>;
using AnalyserInternalVariablePtrs = std::vector<AnalyserInternalVariablePtr>;

using AnalyserEquationPtrs = std::vector<AnalyserEquationPtr>;
using AnalyserVariablePtrs = std::vector<AnalyserVariablePtr>;
using AnalyserExternalVariablePtrs = std::vector<AnalyserExternalVariablePtr>;

struct AnalyserInternalVariable
{
    enum struct Type
    {
        UNKNOWN,
        SHOULD_BE_STATE,
        INITIALISED,
        VARIABLE_OF_INTEGRATION,
        STATE,
        CONSTANT,
        COMPUTED_TRUE_CONSTANT,
        COMPUTED_VARIABLE_BASED_CONSTANT,
        INITIALISED_ALGEBRAIC_VARIABLE,
        ALGEBRAIC_VARIABLE,
        UNDERCONSTRAINED,
        OVERCONSTRAINED
    };

    Type mType = Type::UNKNOWN;
    bool mIsKnownStateVariable = false;
    bool mIsExternalVariable = false;

    VariablePtr mInitialisingVariable;
    VariablePtr mVariable;
    VariablePtrs mDependencies;

    static AnalyserInternalVariablePtr create(const VariablePtr &variable);

    void setVariable(const VariablePtr &variable,
                     bool checkInitialValue = true);

    void makeVoi();
    void makeState();
    void makeConstant();
};

struct AnalyserInternalEquation
{
    enum struct Type
    {
        UNKNOWN,
        CONSTANT,
        COMPUTED_CONSTANT,
        ODE,
        NLA,
        ALGEBRAIC
    };

    Type mType = Type::UNKNOWN;

    VariablePtrs mDependencies;

    AnalyserEquationAstPtr mAst;

    ComponentPtr mComponent;

    AnalyserInternalVariablePtrs mVariables;
    AnalyserInternalVariablePtrs mStateVariables;
    AnalyserInternalVariablePtrs mAllVariables;
    AnalyserInternalVariablePtrs mUnknownVariables;

    size_t mNlaSystemIndex = MAX_SIZE_T;
    AnalyserInternalEquationWeakPtrs mNlaSiblings;

    bool mComputedTrueConstant = true;
    bool mComputedVariableBasedConstant = true;

    static AnalyserInternalEquationPtr create(const ComponentPtr &component);
    static AnalyserInternalEquationPtr create(const AnalyserInternalVariablePtr &variable);

    void addVariable(const AnalyserInternalVariablePtr &variable);
    void addStateVariable(const AnalyserInternalVariablePtr &stateVariable);

    static bool isKnownVariable(const AnalyserInternalVariablePtr &variable);
    static bool isKnownStateVariable(const AnalyserInternalVariablePtr &stateVariable);

    static bool hasKnownVariables(const AnalyserInternalVariablePtrs &variables);
    bool hasKnownVariables();

    static bool isNonConstantVariable(const AnalyserInternalVariablePtr &variable);

    static bool hasNonConstantVariables(const AnalyserInternalVariablePtrs &variables);
    bool hasNonConstantVariables();

    bool variableOnLhsRhs(const AnalyserInternalVariablePtr &variable,
                          const AnalyserEquationAstPtr &astChild);
    bool variableOnRhs(const AnalyserInternalVariablePtr &variable);
    bool variableOnLhsOrRhs(const AnalyserInternalVariablePtr &variable);

    bool check(const AnalyserModelPtr &analyserModel, bool checkNlaSystems);
};

/**
 * @brief The Analyser::AnalyserImpl class.
 *
 * The private implementation for the Analyser class.
 */
using UnitsMap = std::map<std::string, double>;
using UnitsMaps = std::vector<UnitsMap>;
using UnitsMultipliers = std::vector<double>;

class Analyser::AnalyserImpl: public Logger::LoggerImpl
{
public:
    class PowerData
    {
    public:
        bool mDimensionlessBase;
        bool mExponentValueAvailable = true;
        bool mExponentValueChangeable = false;
        double mExponentValue;
        AnalyserEquationAstPtr mExponentAst;
    };

    Analyser *mAnalyser = nullptr;

    AnalyserModelPtr mAnalyserModel = AnalyserModel::AnalyserModelImpl::create();

    AnalyserExternalVariablePtrs mExternalVariables;

    AnalyserInternalVariablePtrs mInternalVariables;
    std::unordered_map<std::uintptr_t, AnalyserInternalVariablePtr> mInternalVariableMap;
    AnalyserInternalEquationPtrs mInternalEquations;

    GeneratorProfilePtr mGeneratorProfile = GeneratorProfile::create();

    std::map<std::string, UnitsPtr> mStandardUnits;
    std::map<AnalyserEquationAstPtr, UnitsPtr> mCiCnUnits;

    AnalyserImpl();

    AnalyserInternalVariablePtr internalVariable(const VariablePtr &variable);

    VariablePtr voiFirstOccurrence(const VariablePtr &variable,
                                   const ComponentPtr &component);

    void analyseNode(const XmlNodePtr &node, AnalyserEquationAstPtr &ast,
                     const AnalyserEquationAstPtr &astParent,
                     const ComponentPtr &component,
                     const AnalyserInternalEquationPtr &equation);
    void analyseComponent(const ComponentPtr &component);
    void analyseComponentVariables(const ComponentPtr &component);

    void equivalentVariables(const VariablePtr &variable,
                             VariablePtrs &equivalentVariables) const;
    VariablePtrs equivalentVariables(const VariablePtr &variable) const;

    void analyseEquationAst(const AnalyserEquationAstPtr &ast);

    void updateUnitsMapWithStandardUnit(const std::string &unitsName,
                                        UnitsMap &unitsMap,
                                        double unitsExponent);
    void updateUnitsMap(const ModelPtr &model, const std::string &unitsName,
                        UnitsMap &unitsMap, bool userUnitsMap = false,
                        double unitsExponent = 1.0,
                        double unitsMultiplier = 0.0);
    UnitsMap multiplyDivideUnitsMaps(const UnitsMap &firstUnitsMap,
                                     const UnitsMap &secondUnitsMap,
                                     bool multiply);
    UnitsMaps multiplyDivideUnitsMaps(const UnitsMaps &firstUnitsMaps,
                                      const UnitsMaps &secondUnitsMaps,
                                      bool multiply = true);
    UnitsMaps multiplyDivideUnitsMaps(const UnitsMaps &unitsMaps,
                                      double factor, bool multiply);
    double multiplyDivideUnitsMultipliers(double firstUnitsMultiplier,
                                          double secondUnitsMultiplier,
                                          bool multiply);
    UnitsMultipliers multiplyDivideUnitsMultipliers(const UnitsMultipliers &firstUnitsMultipliers,
                                                    const UnitsMultipliers &secondUnitsMultipliers,
                                                    bool multiply = true);
    UnitsMultipliers multiplyDivideUnitsMultipliers(double firstUnitsMultiplier,
                                                    const UnitsMultipliers &secondUnitsMultipliers,
                                                    bool multiply);
    UnitsMultipliers powerRootUnitsMultipliers(const UnitsMultipliers &unitsMultipliers,
                                               double factor, bool power);
    bool areSameUnitsMaps(const UnitsMaps &firstUnitsMaps,
                          const UnitsMaps &secondUnitsMaps);
    bool isDimensionlessUnitsMaps(const UnitsMaps &unitsMaps);
    bool areSameUnitsMultipliers(const UnitsMultipliers &firstUnitsMultipliers,
                                 const UnitsMultipliers &secondUnitsMultipliers);
    void updateUnitsMultiplier(const ModelPtr &model,
                               const std::string &unitsName,
                               double &newUnitsMultiplier,
                               double unitsExponent = 1.0,
                               double unitsMultiplier = 0.0);
    std::string componentName(const AnalyserEquationAstPtr &ast);
    double powerValue(const AnalyserEquationAstPtr &ast, PowerData &powerData);
    std::string expression(const AnalyserEquationAstPtr &ast,
                           bool includeHierarchy = true);
    std::string expressionUnits(const UnitsMaps &unitsMaps,
                                const UnitsMultipliers &unitsMultipliers = {});
    std::string expressionUnits(const AnalyserEquationAstPtr &ast,
                                const UnitsMaps &unitsMaps,
                                const UnitsMaps &userUnitsMaps,
                                const UnitsMultipliers &unitsMultipliers);
    void defaultUnitsMapsAndMultipliers(UnitsMaps &unitsMaps,
                                        UnitsMaps &userUnitsMaps,
                                        UnitsMultipliers &unitsMultipliers);
    void analyseEquationUnits(const AnalyserEquationAstPtr &ast,
                              UnitsMaps &unitsMaps, UnitsMaps &userUnitsMaps,
                              UnitsMultipliers &unitsMultipliers,
                              std::string &issueDescription, PowerData &powerData);

    double scalingFactor(const VariablePtr &variable);

    void scaleAst(const AnalyserEquationAstPtr &ast,
                  const AnalyserEquationAstPtr &astParent,
                  double scalingFactor);
    void scaleEquationAst(const AnalyserEquationAstPtr &ast);

    static bool isExternalVariable(const AnalyserInternalVariablePtr &variable);

    bool isStateRateBased(const AnalyserEquationPtr &analyserEquation,
                          AnalyserEquationPtrs &checkedEquations);

    void addInvalidVariableIssue(const AnalyserInternalVariablePtr &variable,
                                 Issue::ReferenceRule referenceRule);

    void analyseModel(const ModelPtr &model);

    AnalyserExternalVariablePtrs::const_iterator findExternalVariable(const VariablePtr &variable) const;
    AnalyserExternalVariablePtrs::const_iterator findExternalVariable(const AnalyserExternalVariablePtr &externalVariable) const;
};

} // namespace libcellml
