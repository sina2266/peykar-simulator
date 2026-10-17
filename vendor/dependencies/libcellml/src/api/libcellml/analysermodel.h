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

#include <string>
#include <vector>

#include "libcellml/analyser.h"

#ifndef SWIG
template class LIBCELLML_EXPORT std::weak_ptr<libcellml::AnalyserModel>;
#endif

namespace libcellml {

/**
 * @brief The AnalyserModel class.
 *
 * The AnalyserModel class is for representing a model in the context of a
 * CellML Analyser.
 */
class LIBCELLML_EXPORT AnalyserModel
#ifndef SWIG
    : public std::enable_shared_from_this<AnalyserModel>
#endif
{
    friend class Analyser;

public:
    /**
     * @brief The type of a model.
     *
     * The type of a model, i.e. whether it is unknown, invalid,
     * underconstrained, overconstrained, or unsuitably constrained, or whether
     * it defines a system of algebraic equations, a system of non-linear
     * algebraic equations, a system of ordinary differential equations, or
     * a system of differential algebraic equations.
     */
    enum class Type
    {
        UNKNOWN, /**< The type of the model is unknown. */
        ALGEBRAIC, /**< The model defines a system of algebraic equations that can be solved directly. */
        DAE, /**< The model defines a system of differential algebraic equations that require both an external ODE solver (e.g., CVODE) and an external NLA solver (e.g., KINSOL) to solve. */
        INVALID, /**< The model is invalid. */
        NLA, /**< The model defines a system of (potentially non-linear) algebraic equations that require an external NLA solver (e.g., KINSOL) to solve. */
        ODE, /**< The model defines a system of ordinary differential equations that require an external ODE solver (e.g., CVODE) to solve. */
        OVERCONSTRAINED, /**< The model is determined to be overconstrained as there are more equations than unknown variables. */
        UNDERCONSTRAINED, /**< The model is determined to be underconstrained as there are insufficient equations to constrain all variables. */
        UNSUITABLY_CONSTRAINED /**< The model is determined to be unsuitably constrained, i.e. both overconstrained and underconstrained. */
    };

    ~AnalyserModel(); /**< Destructor, @private. */
    AnalyserModel(const AnalyserModel &rhs) = delete; /**< Copy constructor, @private. */
    AnalyserModel(AnalyserModel &&rhs) noexcept = delete; /**< Move constructor, @private. */
    AnalyserModel &operator=(AnalyserModel rhs) = delete; /**< Assignment operator, @private. */

    /**
     * @brief Test to determine if @ref AnalyserModel is a valid model.
     *
     * Test to determine if @ref AnalyserModel is a valid model, return @c true if
     * it is a valid model (i.e. either an algebraic or an ODE model) and
     * @c false otherwise.
     *
     * @return @c true if @ref AnalyserModel is a valid model, @c false otherwise.
     */
    bool isValid() const;

    /**
     * @brief Get the @ref Type of the @ref AnalyserModel.
     *
     * Return the @ref Type of the @ref AnalyserModel.
     *
     * @return The @ref Type.
     */
    Type type() const;

    /**
     * @brief Get the string version of a @ref Type.
     *
     * Return the string version of a @ref Type.
     *
     * @param type The type for which we want the string version.
     *
     * @return The string version of the @ref Type.
     */
    static std::string typeAsString(Type type);

    /**
     * @brief Test to determine if this @ref AnalyserModel has external variables.
     *
     * Test to determine if this @ref AnalyserModel has external variables, return
     * @c true if it does and @c false otherwise.
     *
     * @return @c true if @ref AnalyserModel has external variables, @c false
     * otherwise.
     */
    bool hasExternalVariables() const;

    /**
     * @brief Get the @ref Variable of integration.
     *
     * Return the @ref Variable of integration of the @ref AnalyserModel, in the
     * case of an ODE model, @c nullptr otherwise.
     *
     * @return The @ref Variable of integration, if an ODE model, @c nullptr
     * otherwise.
     */
    AnalyserVariablePtr voi() const;

    /**
     * @brief Get the number of states.
     *
     * Return the number of states in the @ref AnalyserModel.
     *
     * @return The number of states.
     */
    size_t stateCount() const;

    /**
     * @brief Get the states.
     *
     * Return the states in the @ref AnalyserModel.
     *
     * @return The states as a @c std::vector.
     */
    std::vector<AnalyserVariablePtr> states() const;

    /**
     * @brief Get the state at @p index.
     *
     * Return the state at the index @p index in this @ref AnalyserModel.
     *
     * @param index The index of the state to return.
     *
     * @return The state at the given @p index on success, @c nullptr on
     * failure.
     */
    AnalyserVariablePtr state(size_t index) const;

    /**
     * @brief Get the number of constants.
     *
     * Return the number of constants in the @ref AnalyserModel.
     *
     * @return The number of constants.
     */
    size_t constantCount() const;

    /**
     * @brief Get the constants.
     *
     * Return the constants in the @ref AnalyserModel.
     *
     * @return The constants as a @c std::vector.
     */
    std::vector<AnalyserVariablePtr> constants() const;

    /**
     * @brief Get the constant at @p index.
     *
     * Return the constant at the index @p index in this @ref AnalyserModel.
     *
     * @param index The index of the constant to return.
     *
     * @return The constant at the given @p index on success, @c nullptr on
     * failure.
     */
    AnalyserVariablePtr constant(size_t index) const;

    /**
     * @brief Get the number of computed constants.
     *
     * Return the number of computed constants in the @ref AnalyserModel.
     *
     * @return The number of computed constants.
     */
    size_t computedConstantCount() const;

    /**
     * @brief Get the computed constants.
     *
     * Return the computed constants in the @ref AnalyserModel.
     *
     * @return The computed constants as a @c std::vector.
     */
    std::vector<AnalyserVariablePtr> computedConstants() const;

    /**
     * @brief Get the computed constant at @p index.
     *
     * Return the computed constant at the index @p index in this @ref AnalyserModel.
     *
     * @param index The index of the computed constant to return.
     *
     * @return The computed constant at the given @p index on success, @c nullptr on
     * failure.
     */
    AnalyserVariablePtr computedConstant(size_t index) const;

    /**
     * @brief Get the number of algebraic variables.
     *
     * Return the number of algebraic variables in the @ref AnalyserModel.
     *
     * @return The number of algebraic variables.
     */
    size_t algebraicVariableCount() const;

    /**
     * @brief Get the algebraic variables.
     *
     * Return the algebraic variables in the @ref AnalyserModel.
     *
     * @return The algebraic variables as a @c std::vector.
     */
    std::vector<AnalyserVariablePtr> algebraicVariables() const;

    /**
     * @brief Get the algebraic variable at @p index.
     *
     * Return the algebraic variable at the index @p index in this @ref AnalyserModel.
     *
     * @param index The index of the algebraic variable to return.
     *
     * @return The algebraic variable at the given @p index on success, @c nullptr on
     * failure.
     */
    AnalyserVariablePtr algebraicVariable(size_t index) const;

    /**
     * @brief Get the number of external variables.
     *
     * Return the number of external variables in the @ref AnalyserModel.
     *
     * @return The number of external variables.
     */
    size_t externalVariableCount() const;

    /**
     * @brief Get the external variables.
     *
     * Return the external variables in the @ref AnalyserModel.
     *
     * @return The external variables as a @c std::vector.
     */
    std::vector<AnalyserVariablePtr> externalVariables() const;

    /**
     * @brief Get the external variable at @p index.
     *
     * Return the external variable at the index @p index in this @ref AnalyserModel.
     *
     * @param index The index of the external variable to return.
     *
     * @return The external variable at the given @p index on success, @c nullptr on
     * failure.
     */
    AnalyserVariablePtr externalVariable(size_t index) const;

    /**
     * @brief Get the analyser variable for the given @p variable.
     *
     * Return the analyser variable for the given @p variable in this @ref AnalyserModel.
     *
     * @param variable The variable for which to return the analyser variable.
     *
     * @return The analyser variable for the given @p variable on success, @c nullptr on failure.
     */
    AnalyserVariablePtr analyserVariable(const VariablePtr &variable);

    /**
     * @brief Get the number of analyser equations.
     *
     * Return the number of analyser equations in the @ref AnalyserModel.
     *
     * @return The number of analyser equations.
     */
    size_t analyserEquationCount() const;

    /**
     * @brief Get the analyser equations.
     *
     * Return the analyser equations in the @ref AnalyserModel.
     *
     * @return The analyser equations as a @c std::vector.
     */
    std::vector<AnalyserEquationPtr> analyserEquations() const;

    /**
     * @brief Get the analyser equation at @p index.
     *
     * Return the analyser equation at the index @p index in this @ref AnalyserModel.
     *
     * @param index The index of the analyser equation to return.
     *
     * @return The analyser equation at the given @p index on success, @c nullptr on
     * failure.
     */
    AnalyserEquationPtr analyserEquation(size_t index) const;

    /**
     * @brief Test to determine if @ref AnalyserModel needs an "equal to"
     * function.
     *
     * Test to determine if @ref AnalyserModel needs an "equal to" function,
     * return @c true if it does and @c false otherwise.
     *
     * @return @c true if @ref AnalyserModel needs an "equal to" function,
     * @c false otherwise.
     */
    bool needEqFunction() const;

    /**
     * @brief Test to determine if @ref AnalyserModel needs a "not equal to"
     * function.
     *
     * Test to determine if @ref AnalyserModel needs a "not equal to" function,
     * return @c true if it does and @c false otherwise.
     *
     * @return @c true if @ref AnalyserModel needs a "not equal to" function,
     * @c false otherwise.
     */
    bool needNeqFunction() const;

    /**
     * @brief Test to determine if @ref AnalyserModel needs a "less than"
     * function.
     *
     * Test to determine if @ref AnalyserModel needs a "less than" function,
     * return @c true if it does and @c false otherwise.
     *
     * @return @c true if @ref AnalyserModel needs a "less than" function,
     * @c false otherwise.
     */
    bool needLtFunction() const;

    /**
     * @brief Test to determine if @ref AnalyserModel needs a "less than or equal
     * to" function.
     *
     * Test to determine if @ref AnalyserModel needs a "less than or equal to"
     * function, return @c true if it does and @c false otherwise.
     *
     * @return @c true if @ref AnalyserModel needs a "less than or equal to"
     * function, @c false otherwise.
     */
    bool needLeqFunction() const;

    /**
     * @brief Test to determine if @ref AnalyserModel needs a "greater than"
     * function.
     *
     * Test to determine if @ref AnalyserModel needs a "greater than" function,
     * return @c true if it does and @c false otherwise.
     *
     * @return @c true if @ref AnalyserModel needs a "greater than" function,
     * @c false otherwise.
     */
    bool needGtFunction() const;

    /**
     * @brief Test to determine if @ref AnalyserModel needs a "greater than or
     * equal to" function.
     *
     * Test to determine if @ref AnalyserModel needs a "greater than or equal to"
     * function, return @c true if it does and @c false otherwise.
     *
     * @return @c true if @ref AnalyserModel needs a "greater than or equal to"
     * function, @c false otherwise.
     */
    bool needGeqFunction() const;

    /**
     * @brief Test to determine if @ref AnalyserModel needs an "and" function.
     *
     * Test to determine if @ref AnalyserModel needs an "and" function, return
     * @c true if it does and @c false otherwise.
     *
     * @return @c true if @ref AnalyserModel needs an "and" function, @c false
     * otherwise.
     */
    bool needAndFunction() const;

    /**
     * @brief Test to determine if @ref AnalyserModel needs an "or" function.
     *
     * Test to determine if @ref AnalyserModel needs an "or" function, return
     * @c true if it does and @c false otherwise.
     *
     * @return @c true if @ref AnalyserModel needs an "or" function, @c false
     * otherwise.
     */
    bool needOrFunction() const;

    /**
     * @brief Test to determine if @ref AnalyserModel needs a "exclusive or"
     * function.
     *
     * Test to determine if @ref AnalyserModel needs a "exclusive or" function,
     * return @c true if it does and @c false otherwise.
     *
     * @return @c true if @ref AnalyserModel needs a "exclusive or" function,
     * @c false otherwise.
     */
    bool needXorFunction() const;

    /**
     * @brief Test to determine if @ref AnalyserModel needs a "not" function.
     *
     * Test to determine if @ref AnalyserModel needs a "not" function, return
     * @c true if it does and @c false otherwise.
     *
     * @return @c true if @ref AnalyserModel needs a "not" function, @c false
     * otherwise.
     */
    bool needNotFunction() const;

    /**
     * @brief Test to determine if @ref AnalyserModel needs a "minimum" function.
     *
     * Test to determine if @ref AnalyserModel needs a "minimum" function, return
     * @c true if it does and @c false otherwise.
     *
     * @return @c true if @ref AnalyserModel needs a "minimum" function, @c false
     * otherwise.
     */
    bool needMinFunction() const;

    /**
     * @brief Test to determine if @ref AnalyserModel needs a "maximum" function.
     *
     * Test to determine if @ref AnalyserModel needs a "maximum" function, return
     * @c true if it does and @c false otherwise.
     *
     * @return @c true if @ref AnalyserModel needs a "maximum" function, @c false
     * otherwise.
     */
    bool needMaxFunction() const;

    /**
     * @brief Test to determine if @ref AnalyserModel needs a "secant" function.
     *
     * Test to determine if @ref AnalyserModel needs a "secant" function, return
     * @c true if it does and @c false otherwise.
     *
     * @return @c true if @ref AnalyserModel needs a "secant" function, @c false
     * otherwise.
     */
    bool needSecFunction() const;

    /**
     * @brief Test to determine if @ref AnalyserModel needs a "cosecant" function.
     *
     * Test to determine if @ref AnalyserModel needs a "cosecant" function, return
     * @c true if it does and @c false otherwise.
     *
     * @return @c true if @ref AnalyserModel needs a "cosecant" function, @c false
     * otherwise.
     */
    bool needCscFunction() const;

    /**
     * @brief Test to determine if @ref AnalyserModel needs a "cotangent"
     * function.
     *
     * Test to determine if @ref AnalyserModel needs a "cotangent" function,
     * return @c true if it does and @c false otherwise.
     *
     * @return @c true if @ref AnalyserModel needs a "cotangent" function,
     * @c false otherwise.
     */
    bool needCotFunction() const;

    /**
     * @brief Test to determine if @ref AnalyserModel needs a "hyperbolic secant"
     * function.
     *
     * Test to determine if @ref AnalyserModel needs a "hyperbolic secant"
     * function, return @c true if it does and @c false otherwise.
     *
     * @return @c true if @ref AnalyserModel needs a "hyperbolic secant" function,
     * @c false otherwise.
     */
    bool needSechFunction() const;

    /**
     * @brief Test to determine if @ref AnalyserModel needs a "hyperbolic
     * cosecant" function.
     *
     * Test to determine if @ref AnalyserModel needs a "hyperbolic cosecant"
     * function, return @c true if it does and @c false otherwise.
     *
     * @return @c true if @ref AnalyserModel needs a "hyperbolic cosecant"
     * function, @c false otherwise.
     */
    bool needCschFunction() const;

    /**
     * @brief Test to determine if @ref AnalyserModel needs a "hyperbolic
     * cotangent" function.
     *
     * Test to determine if @ref AnalyserModel needs a "hyperbolic cotangent"
     * function, return @c true if it does and @c false otherwise.
     *
     * @return @c true if @ref AnalyserModel needs a "hyperbolic cotangent"
     * function, @c false otherwise.
     */
    bool needCothFunction() const;

    /**
     * @brief Test to determine if @ref AnalyserModel needs an "arc secant"
     * function.
     *
     * Test to determine if @ref AnalyserModel needs an "arc secant" function,
     * return @c true if it does and @c false otherwise.
     *
     * @return @c true if @ref AnalyserModel needs an "arc secant" function,
     * @c false otherwise.
     */
    bool needAsecFunction() const;

    /**
     * @brief Test to determine if @ref AnalyserModel needs an "arc cosecant"
     * function.
     *
     * Test to determine if @ref AnalyserModel needs an "arc cosecant" function,
     * return @c true if it does and @c false otherwise.
     *
     * @return @c true if @ref AnalyserModel needs an "arc cosecant" function,
     * @c false otherwise.
     */
    bool needAcscFunction() const;

    /**
     * @brief Test to determine if @ref AnalyserModel needs an "arc cotangent"
     * function.
     *
     * Test to determine if @ref AnalyserModel needs an "arc cotangent" function,
     * return @c true if it does and @c false otherwise.
     *
     * @return @c true if @ref AnalyserModel needs an "arc cotangent" function,
     * @c false otherwise.
     */
    bool needAcotFunction() const;

    /**
     * @brief Test to determine if @ref AnalyserModel needs an "arc hyperbolic
     * secant" function.
     *
     * Test to determine if @ref AnalyserModel needs an "arc hyperbolic secant"
     * function, return @c true if it does and @c false otherwise.
     *
     * @return @c true if @ref AnalyserModel needs an "arc hyperbolic secant"
     * function, @c false otherwise.
     */
    bool needAsechFunction() const;

    /**
     * @brief Test to determine if @ref AnalyserModel needs an "arc hyperbolic
     * cosecant" function.
     *
     * Test to determine if @ref AnalyserModel needs an "arc hyperbolic cosecant"
     * function, return @c true if it does and @c false otherwise.
     *
     * @return @c true if @ref AnalyserModel needs an "arc hyperbolic cosecant"
     * function, @c false otherwise.
     */
    bool needAcschFunction() const;

    /**
     * @brief Test to determine if @ref AnalyserModel needs an "arc hyperbolic
     * cotangent" function.
     *
     * Test to determine if @ref AnalyserModel needs an "arc hyperbolic
     * cotangent" function, return @c true if it does and @c false otherwise.
     *
     * @return @c true if @ref AnalyserModel needs an "arc hyperbolic cotangent"
     * function, @c false otherwise.
     */
    bool needAcothFunction() const;

    /**
     * @brief Test to determine if @p variable1 and @p variable2 are equivalent.
     *
     * Test to see if @p variable1 is the same as or equivalent to @p variable2.
     * Returns @c true if @p variable1 is equivalent to @p variable2 and
     * @c false otherwise.
     *
     * The function utilises caching which is constructed during the model
     * analysis phase (@ref Analyser::analyseModel). The cache may become
     * out of date if the model is changed after the model has been analysed.
     *
     * @param variable1 The @ref Variable to test if it is equivalent to
     * @p variable2.
     * @param variable2 The @ref Variable that is potentially equivalent to
     * @p variable1.
     *
     * @return @c true if @p variable1 is equivalent to @p variable2 and
     * @c false otherwise.
     */
    bool areEquivalentVariables(const VariablePtr &variable1, const VariablePtr &variable2);

private:
    AnalyserModel(const ModelPtr &model); /**< Constructor, @private. */

    struct AnalyserModelImpl;
    AnalyserModelImpl *mPimpl; /**< Private member to implementation pointer, @private. */
};

} // namespace libcellml
