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
 * @brief The GeneratorVariableTracker class.
 *
 * The GeneratorVariableTracker class is for representing a CellML GeneratorVariableTracker.
 */
class LIBCELLML_EXPORT GeneratorVariableTracker: public Logger
{
public:
    ~GeneratorVariableTracker(); /**< Destructor, @private. */
    GeneratorVariableTracker(const GeneratorVariableTracker &rhs) = delete; /**< Copy constructor, @private. */
    GeneratorVariableTracker(GeneratorVariableTracker &&rhs) noexcept = delete; /**< Move constructor, @private. */
    GeneratorVariableTracker &operator=(GeneratorVariableTracker rhs) = delete; /**< Assignment operator, @private. */

    /**
     * @brief Create a @ref GeneratorVariableTracker object.
     *
     * Factory method to create a @ref GeneratorVariableTracker. Create a GeneratorVariableTracker with::
     *
     * @code
     *   auto GeneratorVariableTracker = libcellml::GeneratorVariableTracker::create();
     * @endcode
     *
     * @return A smart pointer to a @ref GeneratorVariableTracker object.
     */
    static GeneratorVariableTrackerPtr create() noexcept;

    /**
     * @brief Test if the given @p analyserVariable is tracked.
     *
     * Test if the given @p analyserVariable is tracked.
     *
     * @param analyserVariable The pointer to the @ref AnalyserVariable to test.
     *
     * @return @c true if the analyser variable is tracked, @c false otherwise.
     */
    bool isTrackedVariable(const AnalyserVariablePtr &analyserVariable);

    /**
     * @brief Test if the given @p analyserVariable is untracked.
     *
     * Test if the given @p analyserVariable is untracked.
     *
     * @param analyserVariable The pointer to the @ref AnalyserVariable to test.
     *
     * @return @c true if the analyser variable is untracked, @c false otherwise.
     */
    bool isUntrackedVariable(const AnalyserVariablePtr &analyserVariable);

    /**
     * @brief Track the given @p analyserVariable.
     *
     * Track the given @p analyserVariable. This will add the analyser variable to the list of tracked variables.
     *
     * @param analyserVariable The pointer to the @ref AnalyserVariable to track.
     */
    void trackVariable(const AnalyserVariablePtr &analyserVariable);

    /**
     * @brief Untrack the given @p analyserVariable.
     *
     * Untrack the given @p analyserVariable. This will remove the analyser variable from the list of tracked variables.
     *
     * @param analyserVariable The pointer to the @ref AnalyserVariable to untrack.
     */
    void untrackVariable(const AnalyserVariablePtr &analyserVariable);

    /**
     * @brief Track all the constants in the given @p analyserModel.
     *
     * Track all the constants in the given @p analyserModel. This will add all the constants in the analyser model to
     * the list of tracked variables.
     *
     * @param analyserModel The pointer to the @ref AnalyserModel which all the constants are to be tracked.
     */
    void trackAllConstants(const AnalyserModelPtr &analyserModel);

    /**
     * @brief Untrack all the constants in the given @p analyserModel.
     *
     * Untrack all the constants in the given @p analyserModel. This will remove all the constants in the analyser model
     * from the list of tracked variables.
     *
     * @param analyserModel The pointer to the @ref AnalyserModel which all the constants are to be untracked.
     */
    void untrackAllConstants(const AnalyserModelPtr &analyserModel);

    /**
     * @brief Track all the computed constants in the given @p analyserModel.
     *
     * Track all the computed constants in the given @p analyserModel. This will add all the computed constants in the
     * analyser model to the list of tracked variables.
     *
     * @param analyserModel The pointer to the @ref AnalyserModel which all the computed constants are to be tracked.
     */
    void trackAllComputedConstants(const AnalyserModelPtr &analyserModel);

    /**
     * @brief Untrack all the computed constants in the given @p analyserModel.
     *
     * Untrack all the computed constants in the given @p analyserModel. This will remove all the computed constants in
     * the analyser model from the list of tracked variables.
     *
     * @param analyserModel The pointer to the @ref AnalyserModel which all the computed constants are to be untracked.
     */
    void untrackAllComputedConstants(const AnalyserModelPtr &analyserModel);

    /**
     * @brief Track all the algebraic variables in the given @p analyserModel.
     *
     * Track all the algebraic variables in the given @p analyserModel. This will add all the algebraic variables in the
     * analyser model to the list of tracked variables.
     *
     * @param analyserModel The pointer to the @ref AnalyserModel which all the algebraic variables are to be tracked.
     */
    void trackAllAlgebraicVariables(const AnalyserModelPtr &analyserModel);

    /**
     * @brief Untrack all the algebraic variables in the given @p analyserModel.
     *
     * Untrack all the algebraic variables in the given @p analyserModel. This will remove all the algebraic variables in the
     * analyser model from the list of tracked variables.
     *
     * @param analyserModel The pointer to the @ref AnalyserModel which all the algebraic variables are to be untracked.
     */
    void untrackAllAlgebraicVariables(const AnalyserModelPtr &analyserModel);

    /**
     * @brief Track all the variables in the given @p analyserModel.
     *
     * Track all the variables in the given @p analyserModel. This will add all the variables in the analyser model to
     * the list of tracked variables. This includes all the constants, computed constants, and algebraic variables.
     *
     * @param analyserModel The pointer to the @ref AnalyserModel which all the variables are to be tracked.
     */
    void trackAllVariables(const AnalyserModelPtr &analyserModel);

    /**
     * @brief Untrack all the variables in the given @p analyserModel.
     *
     * Untrack all the variables in the given @p analyserModel. This will remove all the variables in the analyser model
     * from the list of tracked variables. This includes all the constants, computed constants, and algebraic variables.
     *
     * @param analyserModel The pointer to the @ref AnalyserModel which all the variables are to be untracked.
     */
    void untrackAllVariables(const AnalyserModelPtr &analyserModel);

    /**
     * @brief Get the number of tracked constants in the given @p analyserModel.
     *
     * Get the number of tracked constants in the given @p analyserModel.
     *
     * @param analyserModel The pointer to the @ref AnalyserModel for which to get the number of tracked constants.
     *
     * @return The number of tracked constants in the analyser model.
     */
    size_t trackedConstantCount(const AnalyserModelPtr &analyserModel);

    /**
     * @brief Get the number of untracked constants in the given @p analyserModel.
     *
     * Get the number of untracked constants in the given @p analyserModel.
     *
     * @param analyserModel The pointer to the @ref AnalyserModel for which to get the number of untracked constants.
     *
     * @return The number of untracked constants in the analyser model.
     */
    size_t untrackedConstantCount(const AnalyserModelPtr &analyserModel);

    /**
     * @brief Get the number of tracked computed constants in the given @p analyserModel.
     *
     * Get the number of tracked computed constants in the given @p analyserModel.
     *
     * @param analyserModel The pointer to the @ref AnalyserModel for which to get the number of tracked computed
     * constants.
     *
     * @return The number of tracked computed constants in the analyser model.
     */
    size_t trackedComputedConstantCount(const AnalyserModelPtr &analyserModel);

    /**
     * @brief Get the number of untracked computed constants in the given @p analyserModel.
     *
     * Get the number of untracked computed constants in the given @p analyserModel.
     *
     * @param analyserModel The pointer to the @ref AnalyserModel for which to get the number of untracked computed
     * constants.
     *
     * @return The number of untracked computed constants in the analyser model.
     */
    size_t untrackedComputedConstantCount(const AnalyserModelPtr &analyserModel);

    /**
     * @brief Get the number of tracked algebraic variables in the given @p analyserModel.
     *
     * Get the number of tracked algebraic variables in the given @p analyserModel.
     *
     * @param analyserModel The pointer to the @ref AnalyserModel for which to get the number of tracked algebraic
     * variables.
     *
     * @return The number of tracked algebraic variables in the analyser model.
     */
    size_t trackedAlgebraicVariableCount(const AnalyserModelPtr &analyserModel);

    /**
     * @brief Get the number of untracked algebraic variables in the given @p analyserModel.
     *
     * Get the number of untracked algebraic variables in the given @p analyserModel.
     *
     * @param analyserModel The pointer to the @ref AnalyserModel for which to get the number of untracked algebraic
     * variables.
     *
     * @return The number of untracked algebraic variables in the analyser model.
     */
    size_t untrackedAlgebraicVariableCount(const AnalyserModelPtr &analyserModel);

    /**
     * @brief Get the number of tracked variables in the given @p analyserModel.
     *
     * Get the number of tracked variables in the given @p analyserModel.
     *
     * @param analyserModel The pointer to the @ref AnalyserModel for which to get the number of tracked variables.
     *
     * @return The number of tracked variables in the analyser model.
     */
    size_t trackedVariableCount(const AnalyserModelPtr &analyserModel);

    /**
     * @brief Get the number of untracked variables in the given @p analyserModel.
     *
     * Get the number of untracked variables in the given @p analyserModel.
     *
     * @param analyserModel The pointer to the @ref AnalyserModel for which to get the number of untracked variables.
     *
     * @return The number of untracked variables in the analyser model.
     */
    size_t untrackedVariableCount(const AnalyserModelPtr &analyserModel);

private:
    GeneratorVariableTracker(); /**< Constructor, @private. */

    struct GeneratorVariableTrackerImpl;

    GeneratorVariableTrackerImpl *pFunc(); /**< Getter for private implementation pointer, @private. */
    const GeneratorVariableTrackerImpl *pFunc() const; /**< Const getter for private implementation pointer, @private. */
};

} // namespace libcellml
