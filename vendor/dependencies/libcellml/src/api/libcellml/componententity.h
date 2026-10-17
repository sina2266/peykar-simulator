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

#include "libcellml/exportdefinitions.h"
#include "libcellml/namedentity.h"
#include "libcellml/types.h"

namespace libcellml {

class Component;
/**
 * @brief The ComponentEntity class.
 *
 * The interface class for managing Components.
 */
class LIBCELLML_EXPORT ComponentEntity: public NamedEntity
{
public:
    ~ComponentEntity() override = default; /**< Destructor, @private. */
    ComponentEntity(const ComponentEntity &rhs) = delete; /**< Copy constructor, @private. */
    ComponentEntity(ComponentEntity &&rhs) noexcept = delete; /**< Move constructor, @private. */
    ComponentEntity &operator=(ComponentEntity rhs) = delete; /**< Assignment operator, @private. */

    /**
     * @brief Add a child component to this component entity.
     *
     * Add the given component as a child component of this component entity.
     * If the given component doesn't have a parent then this component entity
     * will be set as its parent.  If the given component does have a parent
     * then the given component will have this component entity set as its parent
     * and its old parent will no longer have the given component as a child.
     *
     * If the addition of the given component creates a cycle of components
     * the given component will not be added to this component entity and it
     * will not have this component entity set as its parent.  The component
     * will keep its current parent and it will remain a child of its
     * current parent.
     *
     * @param component The component to add.
     *
     * @return True if the component was added, false otherwise.
     */
    bool addComponent(const ComponentPtr &component);

    /**
     * @brief Remove the component at the given @p index.
     *
     * Remove the component with the given @p index. @p index must
     * be in the range [0, \#components).
     *
     * @param index The index of the component to remove.
     *
     * @return True if the component was removed, false otherwise.
     */
    bool removeComponent(size_t index);

    /**
     * @overload
     *
     * @brief Remove the component with the given @p name.
     *
     * Remove the first component found that matches the given @p name.
     * If @p searchEncapsulated is @c true (default) this will also
     * search for the named component through this component's encapsulated components.
     *
     * @param name The name of the component to remove.
     * @param searchEncapsulated Boolean flag to indicate whether we should also search encapsulated
     * components for the component with the specified @p name. Default value is @c true.
     *
     * @return True if the component was removed, false otherwise.
     */
    bool removeComponent(const std::string &name, bool searchEncapsulated = true);

    /**
     * @overload
     *
     * @brief Remove the component with the given pointer.
     *
     * Remove the component with the pointer @p component. If @p searchEncapsulated is @c true (default)
     * this will also search for the component pointer through this component's encapsulated components.
     *
     * @param component The pointer to the component to remove.
     * @param searchEncapsulated Boolean flag to indicate whether we should also search encapsulated
     * components for the specified @p component pointer. Default value is @c true.
     *
     * @return True if the component was removed, false otherwise.
     */
    bool removeComponent(const ComponentPtr &component, bool searchEncapsulated = true);

    /**
     * @brief Remove all components stored in this component entity.
     *
     * Clears all components that have been added to this component entity.
     */
    void removeAllComponents();

    /**
     * @brief Tests to see if the named component is contained within this component entity.
     *
     * Tests to see if the component with the given @p name is contained
     * within this component entity. If @p searchEncapsulated is @c true (default) this will also
     * search for the named component in this component's encapsulated components.
     * Returns @c true if the component is in the component entity and @c false otherwise.
     *
     * @param name The component name to test for existence in this component entity.
     * @param searchEncapsulated Boolean flag to indicate whether we should also search encapsulated
     * components for the component with the specified @p name. Default value is @c true.
     *
     * @return @c true if the named component is in this component entity and @c false otherwise.
     */
    bool containsComponent(const std::string &name, bool searchEncapsulated = true) const;

    /**
     * @overload
     *
     * @brief Tests to see if the component pointer is contained within this component.
     *
     * Tests to see if the argument component pointer @p component is contained
     * within this component entity. If @p searchEncapsulated is @c true (default) this will also
     * search for the component pointer in this component's encapsulated components.
     * Returns @c true if the component is in the component entity and @c false otherwise.
     *
     * @param component The component pointer to test for existence in this component entity.
     * @param searchEncapsulated Boolean flag to indicate whether we should also search encapsulated
     * components for the specified @p component pointer. Default value is @c true.
     *
     * @return @c true if the component is in the component entity and @c false otherwise.
     */
    bool containsComponent(const ComponentPtr &component, bool searchEncapsulated = true) const;

    /**
     * @brief Get a component at the given @p index.
     *
     * Returns a reference to a component at the given @p index.  @p index must
     * be in the range [0, \#components).
     *
     * @param index The index of the Component to return.
     *
     * @return The Component at the given @p index on success, @c nullptr on failure.
     */
    ComponentPtr component(size_t index) const;

    /**
     * @overload
     *
     * @brief Get a component with the given @p name.
     *
     * Returns a component with the given @p name. If @p searchEncapsulated
     * is @c true (default) this will also search for the named component through this component's
     * encapsulated components. If the @p name is does not match a named component a @c nullptr is returned.
     *
     * @param name The name of the Component to return.
     * @param searchEncapsulated Boolean flag to indicate whether we should also search encapsulated
     * components for the component with the specified @p name. Default value is @c true.
     *
     * @return The Component with the given @p name on success, @c nullptr on failure.
     */
    ComponentPtr component(const std::string &name, bool searchEncapsulated = true) const;

    /**
     * @brief Take the component at the given @p index and return it.
     *
     * Removes the component at the given @p index position and returns it.
     * @p index must be in the range [0, \#components).
     *
     * @param index The index of the Component to take.
     *
     * @return The Component at the given @p index, @c nullptr on failure.
     */
    ComponentPtr takeComponent(size_t index);

    /**
     * @overload
     *
     * @brief Take the component with the given @p name and return it.
     *
     * Takes the component with the given @p name and returns it. If @p searchEncapsulated
     * is @c true (default) this will also search for the named component through this component's
     * encapsulated components.
     *
     * @param name The name of the Component to take.
     * @param searchEncapsulated Boolean flag to indicate whether we should also search encapsulated
     * components for the component with the specified @p name. Default value is @c true.
     *
     * @return The Component identified with the given @p name, @c nullptr on failure.
     */
    ComponentPtr takeComponent(const std::string &name, bool searchEncapsulated = true);

    /**
     * @brief Replace a component at the given @p index.
     *
     * Replaces the component at the @p index with component @p c. @p index must be in
     * the range [0, \#components).
     *
     * @param index Index of the Component to replace.
     * @param newComponent The component to be used as a replacement.
     *
     * @return True if the component was replaced, false otherwise.
     */
    bool replaceComponent(size_t index, const ComponentPtr &newComponent);

    /**
     * @overload
     *
     * @brief Replace a component with the given @p name.
     *
     * Replaces the component with the given @p name with @p component. If @p searchEncapsulated
     * is @c true (default) this will also search for the named component through this component's
     * encapsulated components. If @p name is not found in the components children then no replacement
     * is made.
     *
     * @param name The name of the Component to replace.
     * @param component The Component to use for replacement.
     * @param searchEncapsulated Boolean flag to indicate whether we should also search encapsulated
     * components for the component with the specified @p name. Default value is @c true.
     *
     * @return True if the component was replaced, false otherwise.
     */
    bool replaceComponent(const std::string &name, const ComponentPtr &component, bool searchEncapsulated = true);

    /**
     * @overload
     *
     * @brief Replace the given component.
     *
     * Replaces the given component @p oldComponent with @p newComponent. If @p searchEncapsulated
     * is @c true (default) this will also search for the component through this component's
     * encapsulated components. If @p oldComponent is not found in the components children then no replacement
     * is made.
     *
     * @param oldComponent The Component to replace.
     * @param newComponent The Component to use for replacement.
     * @param searchEncapsulated Boolean flag to indicate whether we should also search encapsulated
     * components for @p oldComponent. Default value is @c true.
     *
     * @return True if the component was replaced, false otherwise.
     */
    bool replaceComponent(const ComponentPtr &oldComponent, const ComponentPtr &newComponent, bool searchEncapsulated = true);

    /**
     * @brief Get the number of components in the component.
     *
     * Returns the number of components the component contains.
     *
     * @return The number of components.
     */
    size_t componentCount() const;

    /**
     * @brief Set the encapsulation identifier for this entity.
     *
     * The encapsulation identifier is placed on the XML element for this entity.
     * For the @c Model class this is the 'encapsulation' element that
     * is the root element for the models structure.  For the @c Component
     * class this is the 'component_ref' element that references the
     * component it represents in the structure.
     *
     * @sa encapsulationId
     *
     * @param id
     */
    void setEncapsulationId(const std::string &id);

    /**
     * @brief Get the encapsulation identifier for this entity.
     *
     * The encapsulation identifier is placed on the XML element for this entity.
     * For the @c Model class this is the 'encapsulation' element that
     * is the root element for the models structure.  For the @c Component
     * class this is the 'component_ref' element that references the
     * component it represents in the structure.
     *
     * @sa setEncapsulationId
     *
     * @return The @c std::string of the encapsulation identifier.
     */
    std::string encapsulationId() const;

    /**
     * @brief Remove the encapsulation identifier for this entity.
     *
     * Clear the encapsulation identifier for this entity.
     */
    void removeEncapsulationId();

protected:
    /**
     * @brief Virtual add component method to be implemented by derived classes.
     *
     * Virtual addComponent method to allow the model and component classes to
     * implement their own versions.
     *
     * @param component The ComponentPtr to add to the list of components.
     *
     * @return True if the component was added, false otherwise.
     */
    virtual bool doAddComponent(const ComponentPtr &component);

    bool doEquals(const EntityPtr &other) const override; /**< Virtual implementation method for equals, @private. */

    class ComponentEntityImpl; /**< Forward declaration for pImpl idiom, @private. */
    explicit ComponentEntity(ComponentEntityImpl *pImpl); /**< Constructor for derived classes, @private. */

private:
    ComponentEntityImpl *pFunc(); /**< Getter for private implementation pointer, @private. */
    const ComponentEntityImpl *pFunc() const; /**< Const getter for private implementation pointer, @private. */
};

} // namespace libcellml
