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

#include "libcellml/componententity.h"
#include "libcellml/exportdefinitions.h"

#ifndef SWIG
template class LIBCELLML_EXPORT std::weak_ptr<libcellml::Model>;
#endif

namespace libcellml {

/**
 * @brief The Model class.
 *
 * The Model class is for representing a CellML Model.
 */
class LIBCELLML_EXPORT Model: public ComponentEntity
#ifndef SWIG
    ,
                              public std::enable_shared_from_this<Model>
#endif
{
public:
    ~Model() override; /**< Destructor, @private. */
    Model(const Model &rhs) = delete; /**< Copy constructor, @private. */
    Model(Model &&rhs) noexcept = delete; /**< Move constructor, @private. */
    Model &operator=(Model rhs) = delete; /**< Assignment operator, @private. */

    /**
     * @brief Create a @c Model object.
     *
     * Factory method to create a @c Model.  Create a
     * blank model with::
     *
     *   ModelPtr model = libcellml::Model::create();
     *
     * or a named model with name "Model" with::
     *
     *   ModelPtr model = libcellml::Model::create("Model");
     *
     * @return A smart pointer to a @c Model object.
     */
    static ModelPtr create() noexcept;

    /**
     * @overload
     */
    static ModelPtr create(const std::string &name) noexcept;

    /**
     * @brief Add a child units item to this model.
     *
     * Add units by reference to the model. If the units item was
     * previously in a different model, it is moved to this one,
     * and the previous model's units list is updated.
     *
     * The function will return @c false and no action is taken if:
     *  - The @p units is @c nullptr.
     *
     * @param units The units to add.
     *
     * @return @c true if the units item is added or @c false otherwise.
     */
    bool addUnits(const UnitsPtr &units);

    /**
     * @brief Remove the units at the given @p index.
     *
     * Remove the units from this model at the given @p index.
     * @p index must be in the range [0, \#units).
     *
     * @param index The index of the units to remove.
     *
     * @return True if the units were removed, false otherwise.
     */
    bool removeUnits(size_t index);

    /**
     * @overload
     *
     * @brief Remove the units with the given @p name.
     *
     * Remove the first units found with the given @p name.
     *
     * @param name The name of the units to remove.
     *
     * @return True if the units were removed, false otherwise.
     */
    bool removeUnits(const std::string &name);

    /**
     * @overload
     *
     * @brief Remove the units with the given pointer.
     *
     * Remove the units with the pointer @p units.
     *
     * @param units The pointer to the units to remove.
     *
     * @return True if the units were removed, false otherwise.
     */
    bool removeUnits(const UnitsPtr &units);

    /**
     * @brief Remove all units stored in this model.
     *
     * Clears all units that have been added to this model.
     */
    void removeAllUnits();

    /**
     * @brief Tests to see if the units is within this model.
     *
     * Tests to see if the units with the given @c name is contained
     * within this model.  Returns @c true if the units is in
     * the model and @c false otherwise.
     *
     * @param name The name of the units to test for existence in
     * this model.
     *
     * @return @c true if the named units is in the model
     * and @c false otherwise.
     */
    bool hasUnits(const std::string &name) const;

    /**
     * @overload
     *
     * @brief Tests to see if the units is within this model.
     *
     * Tests to see if the given @c units is contained within this model.
     * Returns @c true if the units is in the model and @c false otherwise.
     *
     * @param units The units to test for existence in this model.
     *
     * @return @c true if the units is in the model and @c false otherwise.
     */
    bool hasUnits(const UnitsPtr &units) const;

    /**
     * @brief Get the units item at the given @p index.
     *
     * Returns a reference to a units at the given @p index.  If the @p index
     * is not valid a @c nullptr is returned, the range of valid values for the
     * index is [0, \#units).
     *
     * @param index The index of the units to return.
     *
     * @return A reference to the units at the given @p index on success, @c nullptr otherwise.
     */
    UnitsPtr units(size_t index) const;

    /**
     * @overload
     *
     * @brief Get a units with the given @p name.
     *
     * Returns a reference to a units with the given @p name.  If the @p name
     * is not valid a @c nullptr is returned.
     *
     * @param name The name of the units to return.
     *
     * @return A reference to the units with the given @p name on success, @c nullptr otherwise.
     */
    UnitsPtr units(const std::string &name) const;

    /**
     * @brief Take the units at the given @p index and return it.
     *
     * Removes the units at the given @p index position and returns it.
     * @p index must be in the range [0, \#units).
     *
     * @param index The index of the units to take.
     *
     * @return A reference to the units at the given @p index if the index is valid, @c nullptr otherwise.
     */
    UnitsPtr takeUnits(size_t index);

    /**
     * @overload
     *
     * @brief Take the units with the given @p name and return it.
     *
     * Takes the first occurrence of the units with the given name @p name and returns it.
     * If no units with name @p name is found then a @c nullptr is returned.
     *
     * @param name The name of the units to take.
     *
     * @return A reference to the units identified with the given @p name if found, @c nullptr otherwise.
     */
    UnitsPtr takeUnits(const std::string &name);

    /**
     * @brief Replace a units at the given @p index.
     *
     * Replaces the units at the given @p index with @p units.
     * @p index must be in the range [0, \#units).
     *
     * @param index Index of the units to replace.
     * @param units The units to use for replacement.
     *
     * @return True if the units were replaced, false otherwise.
     */
    bool replaceUnits(size_t index, const UnitsPtr &units);

    /**
     * @overload
     *
     * @brief Replace a units with the given @p name.
     *
     * Replaces the units with the given @p name with @p units.
     *
     * @param name The name of the units to replace.
     * @param units The units to use for replacement.
     *
     * @return True if the units were replaced, false otherwise.
     */
    bool replaceUnits(const std::string &name, const UnitsPtr &units);

    /**
     * @overload
     *
     * @brief Replace a units with another units.
     *
     * Replaces one units with another.
     *
     * @param oldUnits The units to be replaced.
     * @param newUnits The units to use for replacement.
     *
     * @return True if the units were replaced, false otherwise.
     */
    bool replaceUnits(const UnitsPtr &oldUnits, const UnitsPtr &newUnits);

    /**
     * @brief Get the number of units in the model.
     *
     * Returns the number of units the model contains.
     *
     * @return The number of units.
     */
    size_t unitsCount() const;

    /**
     * @brief Link the units used in this model.
     *
     * Traverses the model looking for @c Units attached to
     * @c Variables that are not standard units and which are not
     * linked to @c Units added to the model.  This method will link
     * variable units specified by name to units in the model
     * (if they are found). Any variable units that cannot be linked
     * to units in the model are left in an unlinked state.  This means it
     * is possible to still have unlinked @c Units in the model after
     * calling this method.
     *
     * Unlinked variable units can occur when a @c Variable's units are
     * set by name.  If the @c Model to which the @c Variable belongs
     * has @c Units defined with the same name, then that @c Variable's
     * @c Units will not be linked to the @c Model's @c Units.  This
     * method will link the two units (the one from the variable and the
     * one from the model).
     *
     * If a @c Variable has units that are not found in the model
     * then the units will remain unlinked, and this will return @c false.
     *
     * @return @c true upon success; @c false if some units have not been linked.
     */
    bool linkUnits();

    /**
     * @brief Test to determine if any variable units are not linked to model units.
     *
     * Traverses the model to determine if any @c Units attached to variables
     * are not @c Units attached to the model.
     *
     * @return True if any @c Units attached to variables are not linked to
     * units in the model, false otherwise.
     */
    bool hasUnlinkedUnits();

    /**
     * @brief Test to determine if there are any import entities.
     *
     * Checks the model to determine if there are any @c Units or
     * @c Components which are imports.  Returns @c true if the
     * model has at least one @c Units or @c Component which is an
     * imported @c Units or @c Component.
     *
     * @return True if the @c Model has a @c Units or @c Component
     * that is an import, false otherwise.
     */
    bool hasImports() const;

    /**
     * @brief Test if this model has unresolved imports.
     *
     * Test all components and units of this model and determine if the imported entities are fully resolved.
     *
     * @return @c true if the @c Model has unresolved imports and @c false otherwise.
     */
    bool hasUnresolvedImports() const;

    /**
     * @brief Test if this model is fully defined.
     *
     * Test all components and units of this model and determine if they are all fully defined.
     *
     * @return @c true if the @c Model is defined, @c false otherwise.
     */
    bool isDefined() const;

    /**
     * @brief Create a clone of this model.
     *
     * Creates a full separate copy of this model.  The component
     * hierarchy and variable equivalence maps will also be copied
     * from this model to the destination model.
     *
     * @return a new @c ModelPtr to the cloned model.
     */
    ModelPtr clone() const;

    /**
     * @brief Fix @c Variable interfaces throughout the model.
     *
     * Traverses the model investigating variable equivalences to set the appropriate
     * interface type.  If all equivalent variables have the correct interface type set then
     * the method returns @c true.  If the interface type of one or more equivalent variables
     * cannot be set correctly then @c false is returned.
     *
     * If the interface type for a variable cannot be set correctly, it is left unchanged.
     *
     * @return @c true if all the variable interfaces in the @c Model are correct and
     * @c false otherwise.
     */
    bool fixVariableInterfaces();

    /**
     * @brief Remove any empty components and units from the model.
     *
     *  Remove any empty components and units from this model.
     *  In this context, "empty" means:
     *   - components with no name, identifier, resets, variables, maths, or non-empty child components; and
     *   - units which have no name, identifier, or child units.
     *  For components, this is applied recursively from the leaves of the encapsulation hierarchy.
     */
    void clean();

    /**
     * @brief Return all URLs used by imports in the model.
     *
     * Return a @c std::vector of @c std::strings representing all URLs used by imports in this model.
     *
     * @return a @c std::vector of @c std::strings.
     */
    std::vector<std::string> importRequirements() const;

private:
    Model(); /**< Constructor, @private. */
    explicit Model(const std::string &name); /**< Constructor with std::string parameter, @private. */

    bool doAddComponent(const ComponentPtr &component) override; /**< Virtual implementation method for addComponent, @private. */

    bool doEquals(const EntityPtr &other) const override; /**< Virtual implementation method for equals, @private. */

    class ModelImpl; /**< Forward declaration for pImpl idiom, @private. */

    ModelImpl *pFunc(); /**< Getter for private implementation pointer, @private. */
    const ModelImpl *pFunc() const; /**< Const getter for private implementation pointer, @private. */
};

} // namespace libcellml
