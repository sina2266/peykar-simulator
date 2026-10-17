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

#include "libcellml/componententity.h"
#include "libcellml/exportdefinitions.h"
#include "libcellml/importedentity.h"

#ifndef SWIG
template class LIBCELLML_EXPORT std::weak_ptr<libcellml::Component>;
#endif

namespace libcellml {

/**
 * @brief The Component class.
 *
 * The Component class is for representing a CellML Component.
 */
class LIBCELLML_EXPORT Component: public ComponentEntity, public ImportedEntity
#ifndef SWIG
    ,
                                  public std::enable_shared_from_this<Component>
#endif
{
    friend class Model;
    friend class ComponentEntity;

public:
    ~Component() override; /**< Destructor, @private. */
    Component(const Component &rhs) = delete; /**< Copy constructor, @private. */
    Component(Component &&rhs) noexcept = delete; /**< Move constructor, @private. */
    Component &operator=(Component rhs) = delete; /**< Assignment operator, @private. */

    /**
     * @brief Create a @c Component object.
     *
     * Factory method to create a @c Component.  Create a
     * blank component with::
     *
     * @code
     *   ComponentPtr component = libcellml::Component::create();
     * @endcode
     *
     * @return A smart pointer to a @c Component object.
     */
    static ComponentPtr create() noexcept;

    /**
     * @brief Create a @c Component object with a name.
     *
     * Factory method to create a @c Component with a name.
     * Create a named component with name "Component" with:
     *
     * @code
     *   ComponentPtr component = libcellml::Component::create("Component");
     * @endcode
     *
     * @overload
     *
     * @param name The name of the component.
     *
     * @return A smart pointer to a @c Component object.
     */
    static ComponentPtr create(const std::string &name) noexcept;

    /**
     * @brief Set the source component for this component.
     *
     * Make this component an imported component by defining an import model
     * from which to extract the named component.  The component will be added to the
     * importSource's list of dependent entities.
     *
     * @param importSource The import source from which the named component originates.
     * @param name The name of the component in the imported model to use.
     */
    void setSourceComponent(ImportSourcePtr &importSource, const std::string &name);

    /**
     * @brief Appends the argument to the math string for this component.
     *
     * Appends @p math to the existing math string for this component.
     *
     * @param math The @c std::string to append for this component.
     */
    void appendMath(const std::string &math);

    /**
     * @brief Get the math string for this component.
     *
     * Returns a math string if one has been appended for this component. Returns
     * an empty string if math has not been appended (or has been removed).
     *
     * @return @c std::string math for this component.
     */
    std::string math() const;

    /**
     * @brief Set the math string for this component.
     *
     * Sets the math string for this component. If @p math is an empty
     * string, math will be removed from the component.
     *
     * @param math The @c std::string to append for this component.
     */
    void setMath(const std::string &math);

    /**
     * @brief Clear the math from this component.
     *
     * Clears the math string from this component.
     */
    void removeMath();

    /**
     * @brief Add a variable by reference as part of this component.
     *
     * Add a variable by reference to the component. If the variable
     * was previously in a different component, it is moved to this one, and the
     * previous component's variable list is updated.
     *
     * The function will return @c false and no action is taken if:
     *  - The @p variable is the @c nullptr.
     *
     * @sa removeVariable
     *
     * @param variable The variable to add.
     *
     * @return @c true if the variable is added or @c false otherwise.
     */
    bool addVariable(const VariablePtr &variable);

    /**
     * @brief Remove the variable at the given @p index from this component.
     *
     * Remove the variable at the given index from this component.
     * If the index is not valid @c false is returned, the valid
     * range for the index is [0, \#variables).
     * If the variable to be removed is in a connection (is equivalent to
     * another variable), this component will not be serialised in the
     * connection.
     *
     * @sa addVariable
     *
     * @param index The index of the variable to remove.
     *
     * @return True if the variable was removed, false otherwise.
     */
    bool removeVariable(size_t index);

    /**
     * @brief Remove the variable with the given @p name from this component.
     *
     * Remove the variable with the given name from this component. If the named variable to
     * be removed is in a connection (is equivalent to another variable), this
     * component will not be serialised in the connection.
     *
     * @overload
     *
     * @sa addVariable
     *
     * @param name The name of the variable to remove.
     *
     * @return True if the variable was removed, false otherwise.
     */
    bool removeVariable(const std::string &name);

    /**
     * @brief Remove the variable by the given @p variable pointer from this component.
     *
     * Remove the variable with the given pointer from this component. If the @p variable to
     * be removed is in a connection (is equivalent to another variable), this
     * component will not be serialised in the connection.
     *
     * @overload
     *
     * @sa addVariable
     *
     * @param variable The pointer to the variable to remove.
     *
     * @return True if the variable was removed, false otherwise.
     */
    bool removeVariable(const VariablePtr &variable);

    /**
     * @brief Remove all variables stored in this component.
     *
     * Clears all variables that have been added to this component. If any of the
     * variables to be removed are in connections (are equivalent to other variables),
     * this component will not be serialised in the connection.
     */
    void removeAllVariables();

    /**
     * @brief Get a variable at index.
     *
     * Returns a reference to a variable at the index @p index for this
     * component. If the index is not valid a @c nullptr is returned, the valid
     * range for the index is [0, \#variables).
     *
     * @param index The index of the variable to return.
     *
     * @return A reference to the variable at the given index on success, @c nullptr otherwise.
     */
    VariablePtr variable(size_t index) const;

    /**
     * @brief Get a variable with the given name @p name.
     *
     * Returns a reference to a variable with the name @p name for this
     * component.  If the name is not found a @c nullptr is returned.
     *
     * @overload
     *
     * @param name The name of the variable to return.
     *
     * @return A reference to the Variable with the given name on success, @c nullptr otherwise.
     */
    VariablePtr variable(const std::string &name) const;

    /**
     * @brief Take a variable at index.
     *
     * Remove the variable at the given index from this component and
     * returns a reference to a variable at the index @p index for this
     * component. If the index is not valid a @c nullptr is returned, the valid
     * range for the index is [0, \#variables).
     *
     * @param index The index of the variable to return.
     *
     * @return A reference to the variable at the given index on success, @c nullptr otherwise.
     */
    VariablePtr takeVariable(size_t index);

    /**
     * @brief Take a variable with the given name @p name.
     *
     * Remove the variable with the given name from this component and
     * returns a reference to a variable with the name @p name for this
     * component.  If the name is not found a @c nullptr is returned.
     *
     * @overload
     *
     * @param name The name of the variable to return.
     *
     * @return A reference to the Variable with the given name on success, @c nullptr otherwise.
     */
    VariablePtr takeVariable(const std::string &name);

    /**
     * @brief Get the number of variables in the component.
     *
     * Returns the number of variables the component directly contains.
     *
     * @return the number of variables.
     */
    size_t variableCount() const;

    /**
     * @brief Test whether the argument @p variable is in this component.
     *
     * Tests whether the argument @p variable exists in the set of this component's
     * variables. Returns @c true if the @p variable is in this component's
     * variables and @c false otherwise.
     *
     * @param variable The variable to check for in this component.
     *
     * @return @c true if the @p variable is in this component and @c false otherwise.
     */
    bool hasVariable(const VariablePtr &variable) const;

    /**
     * @brief Test whether the variable named @p name is in this component.
     *
     * Tests whether a variable with the argument @p name exists in the set of this
     * component's variables. Returns @c true if the named variable is in this
     * component's variables and @c false otherwise.
     *
     * @overload
     *
     * @param name The name of the variable to check for in this component.
     *
     * @return @c true if a variable named @p name is in this component
     * and @c false otherwise.
     */
    bool hasVariable(const std::string &name) const;

    /**
     * @brief Add a reset by reference as part of this component.
     *
     * Add a reset by reference to the component. If the reset
     * was previously in a different component, it is moved to this one, and the
     * previous component's reset list is updated.
     *
     * The function will return @c false with no action taken if:
     *  - the supplied @p reset pointer is @c nullptr.
     *
     * @sa removeReset
     *
     * @param reset The reset to add.
     *
     * @return @c true if the reset is added or @c false otherwise.
     */
    bool addReset(const ResetPtr &reset);

    /**
     * @brief Take a reset at index.
     *
     * Remove the reset at the given index from this component and
     * returns a reference to a reset at the index @p index for this
     * component. If the index is not valid, a @c nullptr is returned. The valid
     * range for the index is [0, \#resets).
     *
     * @param index The index of the reset to return.
     *
     * @return A reference to the reset at the given index on success, @c nullptr otherwise.
     */
    ResetPtr takeReset(size_t index);

    /**
     * @brief Remove the reset at the given @p index from this component.
     *
     * Remove the reset at the given index from this component.
     * If the index is not valid @c false is returned, the valid
     * range for the index is [0, \#resets).
     *
     * @sa addReset
     *
     * @param index The index of the reset to remove.
     *
     * @return @c true if the reset was removed, @c false otherwise.
     */
    bool removeReset(size_t index);

    /**
     * @brief Remove the reset by the given @p reset pointer from this component.
     *
     * Remove the reset with the given pointer from this component.
     *
     * @overload
     *
     * @sa addReset
     *
     * @param reset The pointer to the reset to remove.
     *
     * @return True if the reset was removed, false otherwise.
     */
    bool removeReset(const ResetPtr &reset);

    /**
     * @brief Remove all resets stored in this component.
     *
     * Clears all resets that have been added to this component.
     */
    void removeAllResets();

    /**
     * @brief Get a reset at index.
     *
     * Returns a reference to a reset at the index @p index for this
     * component. If the index is not valid a @c nullptr is returned, the valid
     * range for the index is [0, \#resets).
     *
     * @param index The index of the reset to return.
     *
     * @return A reference to the reset at the given index on success, @c nullptr otherwise.
     */
    ResetPtr reset(size_t index) const;

    /**
     * @brief Get the number of resets in the component.
     *
     * Returns the number of resets the component contains.
     *
     * @return the number of resets.
     */
    size_t resetCount() const;

    /**
     * @brief Test whether the argument @p reset is in this component.
     *
     * Tests whether the argument @p reset exists in the set of this component's
     * resets. Returns @c true if the @p reset is in this component's
     * resets and @c false otherwise.
     *
     * @param reset The reset to check for in this component.
     *
     * @return @c true if the @p reset is in this component and @c false otherwise.
     */
    bool hasReset(const ResetPtr &reset) const;

    /**
     * @brief Tests whether this component is defined or not.
     *
     * Analyses the component and determines if the component is fully defined.
     * A fully defined component will have no unresolved imports, all variables
     * will have fully defined units, and all math constant units will be fully defined.
     * If the component is fully defined it will return @c true, @c false otherwise.
     *
     * @see Units::isDefined()
     *
     * @return @c true if the component is fully defined, @c false otherwise.
     */
    bool isDefined() const;

    /**
     * @brief Create a clone of this component.
     *
     * Creates a full separate copy of this component without copying
     * the parent.  Thus the cloned (returned) version of this component
     * will not have a parent set even if this component does.  Any and
     * all child components will also be cloned recreating the full
     * component hierarchy that this component is the root of.
     *
     * @return a new @c ComponentPtr to the cloned component.
     */
    ComponentPtr clone() const;

    /**
     * @brief Determines whether this component or its descendants
     *        contain imported components.
     *
     * Determines whether this component or its descendants contain
     * imported components.
     *
     * @return @c true when imports are required, @c false otherwise.
     */
    bool requiresImports() const;

#ifdef JAVASCRIPT_BINDINGS
#    include "importedentity.impl"
#endif

private:
    Component(); /**< Constructor @private*/
    explicit Component(const std::string &name); /**< Constructor named, @private. */

    bool doAddComponent(const ComponentPtr &component) override; /**< Virtual method for implementing addComponent, @private. */

    bool doIsResolved() const override; /**< Virtual method for implementing isResolved, @private. */

    bool doEquals(const EntityPtr &other) const override; /**< Virtual implementation method for equals, @private. */

    class ComponentImpl; /**< Forward declaration for pImpl idiom, @private. */

    ComponentImpl *pFunc(); /**< Getter for private implementation pointer, @private. */
    const ComponentImpl *pFunc() const; /**< Const getter for private implementation pointer, @private. */
};

} // namespace libcellml
