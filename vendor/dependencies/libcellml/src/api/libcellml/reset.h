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
#include "libcellml/parentedentity.h"
#include "libcellml/types.h"

namespace libcellml {

/**
 * @brief The Reset class.
 * The Reset class is for describing a CellML reset.
 */
class LIBCELLML_EXPORT Reset: public ParentedEntity
{
    friend class Component;

public:
    ~Reset() override; /**< Destructor, @private. */
    Reset(const Reset &rhs) = delete; /**< Copy constructor, @private. */
    Reset(Reset &&rhs) noexcept = delete; /**< Move constructor, @private. */
    Reset &operator=(Reset rhs) = delete; /**< Assignment operator, @private. */

    /**
     * @brief Create a @c Reset object.
     *
     * Factory method to create a @c Reset.  Create a
     * blank reset with::
     *
     *   ResetPtr reset = libcellml::Reset::create();
     *
     * or an ordered reset with order 10 with::
     *
     *   ResetPtr reset = libcellml::Reset::create(10);
     *
     * @return A smart pointer to a @c Reset object.
     */
    static ResetPtr create() noexcept;

    /**
     * @overload
     */
    static ResetPtr create(int order) noexcept;

    /**
     * @brief Set the order of this reset.
     *
     * Sets the order for this reset.
     *
     * @param order The order to set.
     */
    void setOrder(int order);

    /**
     * @brief Get the order of this reset.
     *
     * Returns the order of this reset.
     *
     * @return The order.
     */
    int order() const;

    /**
     * @brief Mark the order as unset.
     *
     * Marks the order value as unset and resets
     * the order to its default value.
     */
    void removeOrder();

    /**
     * @brief Test to determine whether the order is set or unset.
     *
     * Returns whether the order value is in a set state or not.
     *
     * @return @c true if the order is set, @c false otherwise.
     */
    bool isOrderSet();

    /**
     * @brief Set the @c Variable for this @c Reset.
     *
     * Set the @c Variable for this @c Reset.
     *
     * @param variable The @c Variable to set.
     */
    void setVariable(const VariablePtr &variable);

    /**
     * @brief Get the @c Variable for this @c Reset.
     *
     * Return the @c Variable of this @c Reset.
     *
     * @return The @c Variable.
     */
    VariablePtr variable() const;

    /**
     * @brief Set the test @c Variable for this @c Reset.
     *
     * Set the test @c Variable for this @c Reset.
     *
     * @param variable The test @c Variable to set.
     */
    void setTestVariable(const VariablePtr &variable);

    /**
     * @brief Get the test variable for this @c Reset.
     *
     * Return the @c Variable used in the test condition of this @c Reset.
     *
     * @return The @c Variable used in the test condition.
     */
    VariablePtr testVariable() const;

    /**
     * @brief Append the argument to the test value string for this reset.
     *
     * Append @p math to the existing test value string for this reset.
     *
     * @param math The @c std::string to append for this reset.
     */
    void appendTestValue(const std::string &math);

    /**
     * @brief Get the test value string for this reset.
     *
     * Return a test value string if one has been appended for this reset. Returns
     * an empty string if a test value has not been appended (or has been removed).
     *
     * @return @c std::string test value for this reset.
     */
    std::string testValue() const;

    /**
     * @brief Set the test value string for this reset.
     *
     * Set the test value string for this reset. If @p math is an empty
     * string, the test value will effectively be removed from the reset.
     *
     * @param math The @c std::string to append for this reset.
     */
    void setTestValue(const std::string &math);

    /**
     * @brief Clears the test value math for this reset.
     *
     * Clears the test value math for this reset.
     *
     * @sa setTestValue
     */
    void removeTestValue();

    /**
     * @brief Set an identifier to the test value for this reset.
     *
     * Set the identifier to the test value string for this reset.  Setting
     * the @p id to the empty string will effectively remove the identifier.
     *
     * @param id The @c std::string to set for this reset's test value.
     */
    void setTestValueId(const std::string &id);

    /**
     * @brief Clears the identifier for the test value math.
     *
     * Clears the identifier for the test value math.
     *
     * @sa setTestValueId
     */
    void removeTestValueId();

    /**
     * @brief Return the identifier of the test value for this reset.
     *
     * Get the identifier of the test value for this reset.
     *
     * @return @c std::string The identifier of the test value for this reset.
     */
    std::string testValueId() const;

    /**
     * @brief Append the argument to the reset value for this reset.
     *
     * Append @p math to the existing reset value for this reset.
     *
     * @param math The @c std::string to append to the reset value for this reset.
     */
    void appendResetValue(const std::string &math);

    /**
     * @brief Get the reset value string for this reset.
     *
     * Return a math string if one has been appended for this reset. Returns
     * an empty string if math has not been appended (or has been removed).
     *
     * @return @c std::string reset value for this reset.
     */
    std::string resetValue() const;

    /**
     * @brief Set the reset value math for this reset.
     *
     * Set the math to the reset value for this reset. If @p math is an empty
     * string, reset value will effectively be removed from the reset.
     *
     * @param math The @c std::string to append for this reset.
     */
    void setResetValue(const std::string &math);

    /**
     * @brief Clears the reset value math for this reset.
     *
     * Clears the reset value math for this reset.
     *
     * @sa setResetValue
     */
    void removeResetValue();

    /**
     * @brief Set an identifier to the reset value for this reset.
     *
     * Set an identifier to the reset value for this reset.
     *
     * @param id The @c std::string to set for this reset's reset value identifier.
     */
    void setResetValueId(const std::string &id);

    /**
     * @brief Clears the identifier for the reset value math.
     *
     * Clears the identifier for the reset value math.
     *
     * @sa setResetValueId
     */
    void removeResetValueId();

    /**
     * @brief Return the identifier of the reset value for this reset.
     *
     * Get the identifier of the reset value for this reset.
     *
     * @return @c std::string The identifier of the reset value for this reset.
     */
    std::string resetValueId() const;

    /**
     * @brief Create a clone of this reset.
     *
     * Creates a full separate copy of this reset without copying
     * the parent.
     *
     * @return a new @c ResetPtr to the cloned reset.
     */
    ResetPtr clone() const;

private:
    bool doEquals(const EntityPtr &other) const override; /**< Virtual implementation method for equals, @private. */

    Reset(); /**< Constructor, @private. */
    explicit Reset(int order); /**< Constructor with int parameter, @private. */

    class ResetImpl; /**< Forward declaration for pImpl idiom, @private. */

    ResetImpl *pFunc(); /**< Getter for private implementation pointer, @private. */
    ResetImpl const *pFunc() const; /**< Const getter for private implementation pointer, @private. */
};

} // namespace libcellml
