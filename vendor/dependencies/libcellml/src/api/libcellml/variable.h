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

#include "libcellml/exportdefinitions.h"
#include "libcellml/namedentity.h"
#include "libcellml/types.h"

#if defined(_WIN32) && !defined(SWIG)
template class LIBCELLML_EXPORT std::weak_ptr<libcellml::Variable>;
#endif

namespace libcellml {

/**
 * @brief The Variable class.
 *
 * Class for each variable in libCellML.
 */
class LIBCELLML_EXPORT Variable: public NamedEntity
#ifndef SWIG
    ,
                                 public std::enable_shared_from_this<Variable>
#endif
{
    friend class Component;

public:
    ~Variable() override; /**< Destructor, @private. */
    Variable(const Variable &rhs) = delete; /**< Copy constructor, @private. */
    Variable(Variable &&rhs) noexcept = delete; /**< Move constructor, @private. */
    Variable &operator=(Variable rhs) = delete; /**< Assignment operator, @private. */

    /**
     * @brief Create a @c Variable object.
     *
     * Factory method to create a @c Variable.  Create a
     * blank variable with::
     *
     *   VariablePtr variable = libcellml::Variable::create();
     *
     * or a named variable with name "Variable" with::
     *
     *   VariablePtr variable = libcellml::Variable::create("Variable");
     *
     * @return A smart pointer to a @c Variable object.
     */
    static VariablePtr create() noexcept;

    /**
     * @overload
     */
    static VariablePtr create(const std::string &name) noexcept;

    /**
     * @brief The InterfaceType enum class.
     *
     * Enum describing the interface types that are available for
     * a given variable.
     */
    enum class InterfaceType
    {
        NONE,
        PRIVATE,
        PUBLIC,
        PUBLIC_AND_PRIVATE
    };

    /**
     * @brief Add each argument variable to the other's equivalent variable set.
     *
     * Add a copy of @p variable1 to the set of equivalent variables for
     * @p variable2 if not already present. Also add a copy of @p variable2 to the
     * set of equivalent variables for @p variable1 if not already present.
     *
     * @sa removeEquivalence, equivalentVariable
     *
     * @param variable1 The variable to copy to the equivalent variable set
     * for @p variable2.
     * @param variable2 The variable to copy to the equivalent variable set
     * for @p variable1.
     *
     * @return True if the equivalence was added, false otherwise.
     */
    static bool addEquivalence(const VariablePtr &variable1, const VariablePtr &variable2);

    /**
     * @overload
     *
     * @brief Add each argument variable to the other's equivalent variable set.
     *
     * Add a copy of @p variable1 to the set of equivalent variables for
     * @p variable2 if not already present. Also add a copy of @p variable2 to the
     * set of equivalent variables for @p variable1 if not already present.  Also set the
     * mapping identifier of the equivalence and also optionally the connection identifier for the
     * equivalence.
     *
     * @param variable1 The variable to copy to the equivalent variable set
     * for @p variable2.
     * @param variable2 The variable to copy to the equivalent variable set
     * for @p variable1.
     * @param mappingId The @c std::string mapping identifier.
     * @param connectionId The @c std::string connection identifier (optional).
     *
     * @return True if the equivalence was added, false otherwise.
     */
    static bool addEquivalence(const VariablePtr &variable1, const VariablePtr &variable2, const std::string &mappingId, const std::string &connectionId = "");

    /**
     * @brief Set the equivalent mapping identifier for this equivalence.
     *
     * Record the given identifier as the mapping identifier for the equivalence defined with the given
     * variables.  The variables are commutative. This identifier appears in the 'map_variables'
     * element of the model when serialised.
     *
     * To clear an equivalence mapping identifier set it to the empty string. If the two variables are
     * not equivalent the mapping identifier is not set.
     *
     * @param variable1 Variable one of the equivalence.
     * @param variable2 Variable two of the equivalence.
     * @param mappingId The @c std::string mapping identifier.
     */
    static void setEquivalenceMappingId(const VariablePtr &variable1, const VariablePtr &variable2, const std::string &mappingId);

    /**
     * @brief Set the equivalent connection identifier for this equivalence.
     *
     * Record the given identifier as the connection identifier for the equivalence defined with the given
     * variables.  The variables are commutative. This identifier appears in the 'connection'
     * element of the model when serialised.
     *
     * To clear an equivalence connection identifier set it to the empty string. If the two variables are
     * not equivalent the connection identifier is not set.
     *
     * @param variable1 Variable one of the equivalence.
     * @param variable2 Variable two of the equivalence.
     * @param connectionId The @c std::string connection identifier.
     */
    static void setEquivalenceConnectionId(const VariablePtr &variable1, const VariablePtr &variable2, const std::string &connectionId);

    /**
     * @brief Get the equivalent mapping identifier for this equivalence.
     *
     * Get the mapping identifier set for the equivalence defined with the given variables.
     * The variables are commutative.  If no mapping identifier is set the empty string is returned.
     *
     * If the two variables are not equivalent the empty string is returned.
     *
     * @param variable1 Variable one of the equivalence.
     * @param variable2 Variable two of the equivalence.
     *
     * @return the @c std::string mapping identifier.
     */
    static std::string equivalenceMappingId(const VariablePtr &variable1, const VariablePtr &variable2);

    /**
     * @brief Get the equivalent connection identifier for this equivalence.
     *
     * Get the connection identifier set for the equivalence defined with the given variables.
     * The variables are commutative.  If no connection identifier is set the empty string is returned.
     * The optional parameter @p deepSearch will traverse the equivalence network to find the connection identifier for the
     * equivalence defined by the two variables.  By default this is true.
     *
     * If the two variables are not equivalent the empty string is returned.
     *
     * @param variable1 Variable one of the equivalence.
     * @param variable2 Variable two of the equivalence.
     * @param deepSearch Optional parameter to deep search the equivalence network for the connection identifier, true by default.
     * @return the @c std::string connection identifier.
     */
    static std::string equivalenceConnectionId(const VariablePtr &variable1, const VariablePtr &variable2, bool deepSearch = true);

    /**
     * @brief Clear equivalent connection identifier for this equivalence.
     *
     * Clears the equivalent connection identifier for the equivalence defined by the two
     * variables passed as arguments.
     *
     * @param variable1 Variable one of the equivalence.
     * @param variable2 Variable two of the equivalence.
     */
    static void removeEquivalenceConnectionId(const VariablePtr &variable1, const VariablePtr &variable2);

    /**
     * @brief Clear the equivalent mapping identifier for this equivalence.
     *
     * Clears the equivalent mapping identifier for the equivalence defined by the two
     * variables passed as arguments.
     *
     * @param variable1 Variable one of the equivalence.
     * @param variable2 Variable two of the equivalence.
     */
    static void removeEquivalenceMappingId(const VariablePtr &variable1, const VariablePtr &variable2);

    /**
     * @brief Remove each argument variable from the other's equivalent variable set.
     *
     * Removes a copy of @p variable1 from the set of equivalent variables for
     * @p variable2, if present. Also removes a copy of @p variable2 from the
     * set of equivalent variables for @p variable1, if present.
     *
     * @sa addEquivalence, equivalentVariable
     *
     * @param variable1 The variable to remove from the equivalent variable set
     * of @p variable2.
     * @param variable2 The variable to remove from the equivalent variable set
     * of @p variable1.
     *
     * @return True if the equivalence was removed, false otherwise.
     */
    static bool removeEquivalence(const VariablePtr &variable1, const VariablePtr &variable2);

    /**
     * @brief Remove all equivalent variables for this variable.
     *
     * Clears all equivalences that have been added to the equivalence set for this variable.
     */
    void removeAllEquivalences();

    /**
     * @brief Get an equivalent variable at @p index.
     *
     * Returns the equivalent variable at the index @p index for this
     * variable. The index must be in the range [0, \#equivalentVariables).
     *
     * @param index The index of the variable to return.
     *
     * @return The equivalent variable at the given index.
     */
    VariablePtr equivalentVariable(size_t index) const;

    /**
     * @brief Get the number of equivalent variables for this variable.
     *
     * Returns the number of equivalent variables for this variable.
     *
     * @return the number of equivalent variables.
     */
    size_t equivalentVariableCount() const;

    /**
     * @brief Test whether the argument variable is in this variable's equivalent variables.
     *
     * Tests to see if the argument variable exists in the set of this variable's equivalent
     * variables. Returns @c true if the argument variable is in this variable's equivalent
     * variables and @c false otherwise.  By default the test will *not* traverse the equivalent
     * network to determine if the two variables are equivalent.
     *
     * If the optional parameter @p considerIndirectEquivalences is @c true then the test *will*
     * consider the entire equivalence network that this variable is a part of.
     *
     * @param equivalentVariable The variable to check for in this variable's equivalent variables.
     * @param considerIndirectEquivalences Optional parameter to expand the test to the entire
     * equivalent network of this variable.
     *
     * @return @c true if the @p equivalentVariable is in this variable's equivalent variables set or
     * if @p considerIndirectEquivalences is @c true then @c true if the @p equivalentVariable is in
     * this variable's equivalence network.  In all other cases @c false is returned.
     */
    bool hasEquivalentVariable(const VariablePtr &equivalentVariable, bool considerIndirectEquivalences = false) const;

    /**
     * @brief Set the units by @p name for this variable.
     *
     * Set the units for this variable by name. Set to an empty string
     * to unset the units.
     *
     * @sa units
     * @sa removeUnits
     *
     * @param name The name of the units to set.
     */
    void setUnits(const std::string &name);

    /**
     * @overload
     *
     * @brief Set the units for this variable using a @c UnitsPtr.
     *
     * Set the units for this variable as the name associated with the
     * argument @p units.
     *
     * @sa units
     * @sa removeUnits
     *
     * @param units The @c UnitsPtr to set.
     */
    void setUnits(const UnitsPtr &units);

    /**
     * @brief Get the name of the units for this variable.
     *
     * Get the name of the units for this variable. If no units are set
     * an empty @c std::string is returned.
     *
     * @sa setUnits
     * @sa removeUnits
     *
     * @return The @c UnitsPtr of the units for this variable.
     */
    UnitsPtr units() const;

    /**
     * @brief Clear the units from this variable.
     *
     * Clears the units from this variable.
     */
    void removeUnits();

    /**
     * @brief Set the initial value for this variable using a string.
     *
     * Set the initial value for this variable using a string.
     *
     * @sa initialValue
     *
     * @param initialValue The initial value to set.
     */
    void setInitialValue(const std::string &initialValue);

    /**
     * @overload
     *
     * @brief Set the initial value for this variable using a real number.
     *
     * Set the initial value for this variable using a real number.
     * The real number value will be converted to and stored as a string.
     *
     * @sa initialValue
     *
     * @param initialValue The initial value to set.
     */
    void setInitialValue(double initialValue);

    /**
     * @overload
     *
     * @brief Set the initial value for this variable using a variable reference.
     *
     * Set the initial value for this variable using a variable reference.
     * The initial value will be set to the name of the referenced variable.
     *
     * @sa initialValue
     *
     * @param variable The variable reference to use to set the initial value.
     */
    void setInitialValue(const VariablePtr &variable);

    /**
     * @brief Get the initial value for this variable.
     *
     * Get the string corresponding to the initial value for this variable.
     *
     * @sa setInitialValue
     *
     * @return the initial value as a @c std::string.
     */
    std::string initialValue() const;

    /**
     * @brief Clear the initial value for this variable.
     *
     * Clears the initial value for this variable.
     */
    void removeInitialValue();

    /**
     * @brief Set the interface type for this variable.
     *
     * Set the interface type for this variable using a string.
     *
     * @sa interfaceType
     *
     * @param interfaceType The string interface type to set for this variable.
     */
    void setInterfaceType(const std::string &interfaceType);

    /**
     * @brief Set the interface type for this variable.
     *
     * Set the interface type for this variable from the available
     * options in the InterfaceType enum class.
     *
     * @sa interfaceType
     *
     * @param interfaceType The enum interface type to set for this variable.
     *
     * @overload
     */
    void setInterfaceType(InterfaceType interfaceType);

    /**
     * @brief Get the interface type for this variable.
     *
     * Get the string corresponding to the interface type for this variable.
     *
     * @sa setInterfaceType
     *
     * @return the interface type as a @c std::string.
     */
    std::string interfaceType() const;

    /**
     * @brief Clear the interface type for this variable.
     *
     * Clears the interface type for this variable.
     */
    void removeInterfaceType();

    /**
     * @brief Test if this variable has the given interface type.
     *
     * Test if this variable has the same interface as @p interfaceType.
     *
     * @param interfaceType The interface type to test for.
     *
     * @return @c true if the interface type is the same as the given interface type, @c false otherwise.
     */
    bool hasInterfaceType(InterfaceType interfaceType) const;

    /**
     * @brief Test if this variable permits access through the @p interfaceType.
     *
     * Test if this variable permits access through the @p interfaceType. The results
     * will be given according to this truth table (where the parameter is the column
     * label and the stored value is the row label):
     *
     *   | Parameter / Stored value | none | public | private | public_and_private |
     *   |-------------------------:|:-----|:-------|:--------|:-------------------|
     *   |                     none | T    | F      | F       | F                  |
     *   |                   public | T    | T      | F       | F                  |
     *   |                  private | T    | F      | T       | F                  |
     *   |       public_and_private | T    | T      | T       | T                  |
     *
     * @param interfaceType The interface type to test for.
     *
     * @return @c true if the interface type is permitted, @c false otherwise.
     */
    bool permitsInterfaceType(InterfaceType interfaceType) const;

    /**
     * @brief Create a clone of this variable.
     *
     * Creates a full separate copy of this variable without copying
     * the parent.  Thus the cloned (returned) version of this variable
     * will not have a parent set even if this variable does.
     *
     * If this variable has any equivalences these equivalences will
     * *not* be cloned.
     *
     * @return a new @c VariablePtr to the cloned variable.
     */
    VariablePtr clone() const;

private:
    bool doEquals(const EntityPtr &other) const override; /**< Virtual implementation method for equals, @private. */

    Variable(); /**< Constructor, @private. */
    explicit Variable(const std::string &name); /**< Constructor with std::string parameter, @private. */

    class VariableImpl; /**< Forward declaration for pImpl idiom, @private. */

    VariableImpl *pFunc(); /**< Getter for private implementation pointer, @private. */
    const VariableImpl *pFunc() const; /**< Const getter for private implementation pointer, @private. */
};

} // namespace libcellml
