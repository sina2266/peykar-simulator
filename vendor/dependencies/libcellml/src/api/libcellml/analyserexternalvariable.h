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

#include "libcellml/exportdefinitions.h"
#include "libcellml/types.h"

namespace libcellml {

/**
 * @brief The AnalyserExternalVariable class.
 *
 * The AnalyserExternalVariable class is for representing an external variable
 * in the context of a CellML Analyser, i.e. a variable which is not set or
 * computed by this model, but set externally. It is therefore the
 * responsibility of the user to set an external variable (see
 * @ref GeneratorProfile::externalVariableMethodTypeDefinitionString and
 * @ref GeneratorProfile::setExternalVariableMethodTypeDefinitionString).
 *
 * Also, say that the analysis of a model fails because the type of one or
 * several of its variables could not be determined (i.e. the model is
 * underconstrained). To add those variables as external variables will allow
 * the analysis to succeed. This is because the analyser will have considered
 * those variables as initialised. Once again, it is the responsibility of the
 * user to ensure that it all makes sense.
 */
class LIBCELLML_EXPORT AnalyserExternalVariable
{
public:
    ~AnalyserExternalVariable(); /**< Destructor, @private. */
    AnalyserExternalVariable(const AnalyserExternalVariable &rhs) = delete; /**< Copy constructor, @private. */
    AnalyserExternalVariable(AnalyserExternalVariable &&rhs) noexcept = delete; /**< Move constructor, @private. */
    AnalyserExternalVariable &operator=(AnalyserExternalVariable rhs) = delete; /**< Assignment operator, @private. */

    /**
     * @brief Create an @ref AnalyserExternalVariable object.
     *
     * Factory method to create an @ref AnalyserExternalVariable. Create an
     * analyser external variable with::
     *
     *   auto variable = libcellml::Variable::create();
     *   auto analyserExternalVariable = libcellml::AnalyserExternalVariable::create(variable);
     *
     * @return A smart pointer to a @ref AnalyserExternalVariable object.
     */
    static AnalyserExternalVariablePtr create(const VariablePtr &variable) noexcept;

    /**
     * @brief Get the @ref Variable associated with this
     * @ref AnalyserExternalVariable.
     *
     * Return a reference to the @ref Variable associated with this
     * @ref AnalyserExternalVariable.
     *
     * @return The @ref Variable associated with this @ref AnalyserExternalVariable.
     */
    VariablePtr variable() const;

    /**
     * @brief Add a dependency to this @ref AnalyserExternalVariable.
     *
     * Add the given @ref Variable as a dependency of this
     * @ref AnalyserExternalVariable, but only if the given @ref Variable or its
     * primary equivalent has not already been added.
     *
     * @param variable The @ref Variable to add as a dependency.
     *
     * @return @c true if the variable was added, @c false otherwise.
     */
    bool addDependency(const VariablePtr &variable);

    /**
     * @brief Remove the dependency at the given @p index.
     *
     * Remove the dependency with the given @p index. The @p index must be in
     * the range [0, \#dependencies).
     *
     * @param index The index of the dependency to remove.
     *
     * @return @c true if the dependency was removed, @c false otherwise.
     */
    bool removeDependency(size_t index);

    /**
     * @overload
     *
     * @brief Remove the dependency with the given @p variableName in the
     * @ref Component with the given @p componentName in the given @p model.
     *
     * Remove the dependency found that matches the given @p variableName in the
     * @ref Component with the given @p componentName in the given @p model.
     *
     * @param model The pointer to the @ref Model which contains the dependency to
     * remove.
     * @param componentName The name of the @ref Component which contains the
     * dependency to remove.
     * @param variableName The name of the dependency to remove.
     *
     * @return @c true if the dependency was removed, @c false otherwise.
     */
    bool removeDependency(const ModelPtr &model,
                          const std::string &componentName,
                          const std::string &variableName);

    /**
     * @overload
     *
     * @brief Remove the dependency with the given pointer.
     *
     * Remove the dependency with the pointer @p variable.
     *
     * @param variable The pointer to the dependency to remove.
     *
     * @return @c true if the dependency was removed, @c false otherwise.
     */
    bool removeDependency(const VariablePtr &variable);

    /**
     * @brief Remove all dependencies from this @ref AnalyserExternalVariable.
     *
     * Clear all dependencies that have been added to this
     * @ref AnalyserExternalVariable.
     */
    void removeAllDependencies();

    /**
     * @brief Test to see if the dependency with the given @p variableName in
     * the @ref Component with the given @p componentName in the given @p model is
     * contained within this @ref AnalyserExternalVariable's list of dependencies.
     *
     * Test to see if the dependency with the the given @p variableName in the
     * @ref Component with the given @p componentName in the given @p model is
     * contained within this @ref AnalyserExternalVariable's list of dependencies.
     * Return @c true if the dependency is in this @ref AnalyserExternalVariable's
     * list of dependencies and @c false otherwise.
     *
     * @param model The pointer to the @ref Model which contains the dependency to
     * test.
     * @param componentName The name of the @ref Component which contains the
     * dependency to test.
     * @param variableName The name of the dependency to test.
     *
     * @return @c true if the dependency is in this @ref AnalyserExternalVariable's
     * list of dependencies and @c false otherwise.
     */
    bool containsDependency(const ModelPtr &model,
                            const std::string &componentName,
                            const std::string &variableName) const;

    /**
     * @overload
     *
     * @brief Test to see if the dependency with the given pointer is contained
     * within this @ref AnalyserExternalVariable's list of dependencies.
     *
     * Test to see if the dependency with the given pointer is contained within
     * this @ref AnalyserExternalVariable's list of dependencies. Return @c true if
     * the dependency is in this @ref AnalyserExternalVariable's list of
     * dependencies and @c false otherwise.
     *
     * @param variable The pointer to the dependency to remove.
     *
     * @return @c true if the dependency is in this @ref AnalyserExternalVariable's
     * list of dependencies and @c false otherwise.
     */
    bool containsDependency(const VariablePtr &variable) const;

    /**
     * @brief Get the dependency at the given @p index.
     *
     * Return a reference to the dependency at the given @p index. The @p index
     * must be in the range [0, \#dependencies).
     *
     * @param index The index of the dependency to return.
     *
     * @return The dependency at the given @p index on success, @c nullptr on
     * failure.
     */
    VariablePtr dependency(size_t index) const;

    /**
     * @overload
     *
     * @brief Get the dependency with the given @p name.
     *
     * Return the dependency with the given @p variableName in the @ref Component
     * with the given @p componentName in the given @p model. If no such
     * dependency is contained within the @ref AnalyserExternalVariable, a
     * @c nullptr is returned.
     *
     * @param model The pointer to the @ref Model which contains the dependency to
     * retrieve.
     * @param componentName The name of the @ref Component which contains the
     * dependency to retrieve.
     * @param variableName The name of the dependency to retrieve.
     *
     * @return The dependency with the given @p variableName in the @ref Component
     * with the given @p componentName in the given @p model on success,
     * @c nullptr on failure.
     */
    VariablePtr dependency(const ModelPtr &model,
                           const std::string &componentName,
                           const std::string &variableName) const;

    /**
     * @brief Get the list of @ref Variable dependencies.
     *
     * Return the list of @ref Variable items on which this
     * @ref AnalyserExternalVariable depends.
     *
     * @return The dependencies as a @c std::vector.
     */
    std::vector<VariablePtr> dependencies() const;

    /**
     * @brief Get the number of dependencies of this @ref AnalyserExternalVariable.
     *
     * Return the number of dependencies this @ref AnalyserExternalVariable
     * contains.
     *
     * @return The number of dependencies.
     */
    size_t dependencyCount() const;

private:
    explicit AnalyserExternalVariable(const VariablePtr &variable); /**< Constructor */

    struct AnalyserExternalVariableImpl;
    AnalyserExternalVariableImpl *mPimpl; /**< Private member to implementation pointer, @private. */
};

} // namespace libcellml
