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

namespace libcellml {

/**
 * @brief The AnalyserEquation class.
 *
 * The AnalyserEquation class is for representing an equation in the context of
 * a CellML Analyser.
 */
class LIBCELLML_EXPORT AnalyserEquation
{
    friend class Analyser;
    friend class Generator;
    friend class GeneratorVariableTracker;

public:
    /**
     * @brief The type of an equation.
     *
     * The type of an equation, i.e. whether it is used to compute a (true) constant, a computed constant, a rate, an
     * algebraic variable, or whether it is a placeholder for an external variable.
     */
    enum class Type
    {
        CONSTANT, /**< An equation that computes a (true) constant (e.g., x = 3). */
        COMPUTED_CONSTANT, /**< An equation that computes a computed constant (e.g., x = 3+5, x = 3+z, x = y+z where y and z are (true or computed) constants). */
        ODE, /**< An equation that computes an ordinary differential equation (e.g., d(y)/dt = f(t, x)). */
        NLA, /**< An equation that computes one or several algebraic variables (e.g., x+y = f(z) where x and y are known, but not z). */
        ALGEBRAIC, /**< An equation that computes an algebraic variable (e.g., y = f(x)). */
        EXTERNAL /**< A placeholder equation that captures the dependency of the model on an external variable. */
    };

    ~AnalyserEquation(); /**< Destructor, @private. */
    AnalyserEquation(const AnalyserEquation &rhs) = delete; /**< Copy constructor, @private. */
    AnalyserEquation(AnalyserEquation &&rhs) noexcept = delete; /**< Move constructor, @private. */
    AnalyserEquation &operator=(AnalyserEquation rhs) = delete; /**< Assignment operator, @private. */

    /**
     * @brief Get the @ref Type of this @ref AnalyserEquation.
     *
     * Return the @ref Type of this @ref AnalyserEquation.
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
     * @brief Get the @ref AnalyserEquationAst for this @ref AnalyserEquation.
     *
     * Return the @ref AnalyserEquationAst for this @ref AnalyserEquation.
     *
     * @return The @ref AnalyserEquationAst.
     */
    AnalyserEquationAstPtr ast() const;

    /**
     * @brief Get the number of dependencies for this @ref AnalyserEquation.
     *
     * Return the number of dependencies for this @ref AnalyserEquation.
     *
     * @return The number of dependencies for this @ref AnalyserEquation.
     */
    size_t dependencyCount() const;

    /**
     * @brief Get the list of @ref AnalyserEquation dependencies.
     *
     * Return the list of @ref AnalyserEquation items which correspond to the
     * equations on which this @ref AnalyserEquation depends.
     *
     * @return The dependencies as a @c std::vector.
     */
    std::vector<AnalyserEquationPtr> dependencies() const;

    /**
     * @brief Get the dependency, at @p index, for this @ref AnalyserEquation.
     *
     * Return the dependency, at @p index, for this @ref AnalyserEquation.
     *
     * @param index The index of the dependency to return.
     *
     * @return The dependency, at @p index, on success, @c nullptr on failure.
     */
    AnalyserEquationPtr dependency(size_t index) const;

    /**
     * @brief Get the index of the NLA system of this @ref AnalyserEquation.
     *
     * Return the index of the NLA system where this @ref AnalyserEquation is
     * used. A value of MAX_SIZE_T indicates that this @ref AnalyserEquation is
     * not used in an NLA system.
     *
     * @return The index of the NLA system.
     */
    size_t nlaSystemIndex() const;

    /**
     * @brief Get the number of NLA siblings for this @ref AnalyserEquation.
     *
     * Return the number of NLA siblings for this @ref AnalyserEquation.
     *
     * @return The number of NLA siblings for this @ref AnalyserEquation.
     */
    size_t nlaSiblingCount() const;

    /**
     * @brief Get the list of @ref AnalyserEquation NLA siblings.
     *
     * Return the list of @ref AnalyserEquation items which are NLA siblings of
     * this @ref AnalyserEquation.
     *
     * @return The NLA siblings as a @c std::vector.
     */
    std::vector<AnalyserEquationPtr> nlaSiblings() const;

    /**
     * @brief Get the NLA sibling, at @p index, for this @ref AnalyserEquation.
     *
     * Return the NLA sibling, at @p index, for this @ref AnalyserEquation.
     *
     * @param index The index of the NLA sibling to return.
     *
     * @return The NLA sibling, at @p index, on success, @c nullptr on failure.
     */
    AnalyserEquationPtr nlaSibling(size_t index) const;

    /**
     * @brief Test to determine if this @ref AnalyserEquation relies on states
     * and/or rates.
     *
     * Test to determine if this @ref AnalyserEquation relies on states and/or
     * rates, return @c true if it does and @c false otherwise.
     *
     * @return @c true if this @ref AnalyserEquation relies on states and/or
     * rates, @c false otherwise.
     */
    bool isStateRateBased() const;

    /**
     * @brief Get the number of states computed by this @ref AnalyserEquation.
     *
     * Return the number of states computed by this @ref AnalyserEquation.
     *
     * @return The number of states.
     */
    size_t stateCount() const;

    /**
     * @brief Get the states computed by this @ref AnalyserEquation.
     *
     * Return the states computed by this @ref AnalyserEquation.
     *
     * @return The states as a @c std::vector.
     */
    std::vector<AnalyserVariablePtr> states() const;

    /**
     * @brief Get the state, at @p index, computed by this @ref AnalyserEquation.
     *
     * Return the state, at @p index, computed by this @ref AnalyserEquation.
     *
     * @param index The index of the state to return.
     *
     * @return The state, at @p index, on success, @c nullptr on failure.
     */
    AnalyserVariablePtr state(size_t index) const;

    /**
     * @brief Get the number of computed constants computed by this @ref AnalyserEquation.
     *
     * Return the number of computed constants computed by this @ref AnalyserEquation.
     *
     * @return The number of computed constants.
     */
    size_t computedConstantCount() const;

    /**
     * @brief Get the computed constants computed by this @ref AnalyserEquation.
     *
     * Return the computed constants computed by this @ref AnalyserEquation.
     *
     * @return The computed constants as a @c std::vector.
     */
    std::vector<AnalyserVariablePtr> computedConstants() const;

    /**
     * @brief Get the computed constant, at @p index, computed by this @ref AnalyserEquation.
     *
     * Return the computed constant, at @p index, computed by this @ref AnalyserEquation.
     *
     * @param index The index of the computed constant to return.
     *
     * @return The computed constant, at @p index, on success, @c nullptr on failure.
     */
    AnalyserVariablePtr computedConstant(size_t index) const;

    /**
     * @brief Get the number of algebraic variables computed by this @ref AnalyserEquation.
     *
     * Return the number of algebraic variables computed by this @ref AnalyserEquation.
     *
     * @return The number of algebraic variables.
     */
    size_t algebraicVariableCount() const;

    /**
     * @brief Get the algebraic variables computed by this @ref AnalyserEquation.
     *
     * Return the algebraic variables computed by this @ref AnalyserEquation.
     *
     * @return The algebraic variables as a @c std::vector.
     */
    std::vector<AnalyserVariablePtr> algebraicVariables() const;

    /**
     * @brief Get the algebraic variable, at @p index, computed by this @ref AnalyserEquation.
     *
     * Return the algebraic variable, at @p index, computed by this @ref AnalyserEquation.
     *
     * @param index The index of the algebraic variable to return.
     *
     * @return The algebraic variable, at @p index, on success, @c nullptr on failure.
     */
    AnalyserVariablePtr algebraicVariable(size_t index) const;

    /**
     * @brief Get the number of external variables computed by this @ref AnalyserEquation.
     *
     * Return the number of external variables computed by this @ref AnalyserEquation.
     *
     * @return The number of external variables.
     */
    size_t externalVariableCount() const;

    /**
     * @brief Get the external variables computed by this @ref AnalyserEquation.
     *
     * Return the external variables computed by this @ref AnalyserEquation.
     *
     * @return The external variables as a @c std::vector.
     */
    std::vector<AnalyserVariablePtr> externalVariables() const;

    /**
     * @brief Get the external variable, at @p index, computed by this @ref AnalyserEquation.
     *
     * Return the external variable, at @p index, computed by this @ref AnalyserEquation.
     *
     * @param index The index of the external variable to return.
     *
     * @return The external variable, at @p index, on success, @c nullptr on failure.
     */
    AnalyserVariablePtr externalVariable(size_t index) const;

private:
    AnalyserEquation(); /**< Constructor, @private. */

    struct AnalyserEquationImpl;
    AnalyserEquationImpl *mPimpl; /**< Private member to implementation pointer, @private. */
};

} // namespace libcellml
