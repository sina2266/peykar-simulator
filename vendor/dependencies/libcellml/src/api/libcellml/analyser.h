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

#include "libcellml/logger.h"

namespace libcellml {

/**
 * @brief The Analyser class.
 *
 * The Analyser class is for representing a CellML Analyser, which aims to check
 * whether a model makes mathematical sense. If a model makes mathematical sense
 * then an @ref AnalyserModel object can be retrieved, which can be used to
 * generate code, for instance.
 */
class LIBCELLML_EXPORT Analyser: public Logger
{
public:
    ~Analyser() override; /**< Destructor, @private. */
    Analyser(const Analyser &rhs) = delete; /**< Copy constructor, @private. */
    Analyser(Analyser &&rhs) noexcept = delete; /**< Move constructor, @private. */
    Analyser &operator=(Analyser rhs) = delete; /**< Assignment operator, @private. */

    /**
     * @brief Create an @ref Analyser object.
     *
     * Factory method to create an @ref Analyser.  Create an analyser with::
     *
     * @code
     *   auto analyser = libcellml::Analyser::create();
     * @endcode
     *
     * @return A smart pointer to an @ref Analyser object.
     */
    static AnalyserPtr create() noexcept;

    /**
     * @brief Analyse the @ref Model.
     *
     * Analyse the @ref Model using this @ref Analyser.
     *
     * @param model The @ref Model to analyse.
     */
    void analyseModel(const ModelPtr &model);

    /**
     * @brief Add a @ref VariablePtr as an external variable to this @ref Analyser.
     *
     * Add the given @ref VariablePtr as an external variable to this @ref Analyser, but only if it has not already been
     * added.
     *
     * @param variable The @ref Variable to add as an external variable.
     *
     * @return @c true if the variable was added, @c false otherwise.
     */
    bool addExternalVariable(const VariablePtr &variable);

    /**
     * @overload
     *
     * @brief Add an @ref AnalyserExternalVariable to this @ref Analyser.
     *
     * Add the given @ref AnalyserExternalVariable to this @ref Analyser, but only if it has not already been added.
     *
     * @param externalVariable The @ref AnalyserExternalVariable to add.
     *
     * @return @c true if the variable was added, @c false otherwise.
     */
    bool addExternalVariable(const AnalyserExternalVariablePtr &externalVariable);

    /**
     * @brief Remove the @ref AnalyserExternalVariable at the given @p index.
     *
     * Remove the @ref AnalyserExternalVariable with the given @p index. The @p index must be in the range
     * [0, \#externalVariables).
     *
     * @param index The index of the @ref AnalyserExternalVariable to remove.
     *
     * @return @c true if the @ref AnalyserExternalVariable was removed, @c false otherwise.
     */
    bool removeExternalVariable(size_t index);

    /**
     * @overload
     *
     * @brief Remove the @ref AnalyserExternalVariable for the given @p variable.
     *
     * Remove the @ref AnalyserExternalVariable found that matches the given @p variable.
     *
     * @param variable The @ref Variable for which to remove the @ref AnalyserExternalVariable.
     *
     * @return @c true if the @ref AnalyserExternalVariable was removed, @c false otherwise.
     */
    bool removeExternalVariable(const VariablePtr &variable);

    /**
     * @overload
     *
     * @brief Remove the @ref AnalyserExternalVariable for the given @p externalVariable.
     *
     * Remove the @ref AnalyserExternalVariable for the given @p externalVariable.
     *
     * @param externalVariable The @ref AnalyserExternalVariable to remove.
     *
     * @return @c true if the @ref AnalyserExternalVariable was removed, @c false otherwise.
     */
    bool removeExternalVariable(const AnalyserExternalVariablePtr &externalVariable);

    /**
     * @brief Remove all the @ref AnalyserExternalVariable items from this @ref Analyser.
     *
     * Remove all the @ref AnalyserExternalVariable items that were added to this @ref Analyser.
     */
    void removeAllExternalVariables();

    /**
     * @brief Test to see if an @ref AnalyserExternalVariable for the given @p variable is contained within this
     * @ref Analyser.
     *
     * Test to see if an @ref AnalyserExternalVariable for the the given @p variable is contained within this
     * @ref Analyser. Return @c true if an @ref AnalyserExternalVariable is in this @ref Analyser and @c false
     * otherwise.
     *
     * @param variable The @ref Variable to test.
     *
     * @return @c true if an @ref AnalyserExternalVariable is in this @ref Analyser, @c false otherwise.
     */
    bool containsExternalVariable(const VariablePtr &variable) const;

    /**
     * @overload
     *
     * @brief Test to see if an @ref AnalyserExternalVariable for the given @p externalVariable is contained within this
     * @ref Analyser.
     *
     * Test to see if an @ref AnalyserExternalVariable for the given @p externalVariable is contained within this
     * @ref Analyser. Return @c true if an @ref AnalyserExternalVariable is in this @ref Analyser, @c false otherwise.
     *
     * @param externalVariable The @ref AnalyserExternalVariable to test.
     *
     * @return @c true if an @ref AnalyserExternalVariable is in this @ref Analyser, @c false otherwise.
     */
    bool containsExternalVariable(const AnalyserExternalVariablePtr &externalVariable) const;

    /**
     * @brief Get the @ref AnalyserExternalVariable at the given @p index.
     *
     * Return the @ref AnalyserExternalVariable at the given @p index. The @p index must be in the range
     * [0, \#externalVariables).
     *
     * @param index The index of the @ref AnalyserExternalVariable to return.
     *
     * @return The @ref AnalyserExternalVariable at the given @p index on success, @c nullptr on failure.
     */
    AnalyserExternalVariablePtr externalVariable(size_t index) const;

    /**
     * @overload
     *
     * @brief Get the @ref AnalyserExternalVariable for the given @p variable.
     *
     * Return the @ref AnalyserExternalVariable for the given @p variable. If no such @ref AnalyserExternalVariable is
     * contained within this @ref Analyser, a @c nullptr is returned.
     *
     * @param variable The @ref Variable for which to retrieve the @ref AnalyserExternalVariable.
     *
     * @return The @ref AnalyserExternalVariable for the given @p variable on success, @c nullptr on failure.
     */
    AnalyserExternalVariablePtr externalVariable(const VariablePtr &variable) const;

    /**
     * @brief Get the number of @ref AnalyserExternalVariable items in this @ref Analyser.
     *
     * Return the number of @ref AnalyserExternalVariable items this @ref Analyser contains.
     *
     * @return The number of @ref AnalyserExternalVariable items.
     */
    size_t externalVariableCount() const;

    /**
     * @brief Get the analysed model.
     *
     * Get the analysed model that is the result of the model analysis. This
     * allows to retrieve some information about the type of the model, its
     * variable of integration, states, variables, equations, and whether it
     * needs some specific mathematical functions.
     *
     * @return The analysed model for the @ref Model analysed by this @ref Analyser.
     */
    AnalyserModelPtr analyserModel() const;

private:
    Analyser(); /**< Constructor, @private. */

    class AnalyserImpl; /**< Forward declaration for pImpl idiom, @private. */

    AnalyserImpl *pFunc(); /**< Getter for private implementation pointer, @private. */
    const AnalyserImpl *pFunc() const; /**< Const getter for private implementation pointer, @private. */
};

} // namespace libcellml
