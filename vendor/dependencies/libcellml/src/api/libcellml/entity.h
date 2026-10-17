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
#include "libcellml/types.h"

namespace libcellml {

/**
 * @brief The Entity class.
 *
 * Base class for all libCellML core classes.
 */
class LIBCELLML_EXPORT Entity
{
public:
    virtual ~Entity() = 0; /**< Destructor. */
    Entity(const Entity &rhs) = delete; /**< Copy constructor. */
    Entity(Entity &&rhs) noexcept = delete; /**< Move constructor. */

    /**
     * @brief Set the @p id document identifier for this entity.
     *
     * Set the @p id document identifier for this entity using a @c std::string.
     *
     * @sa id
     *
     * @param id The @c std::string document identifier to set.
     */
    void setId(const std::string &id);

    /**
     * @brief Get the document identifier for this entity.
     *
     * Get the string corresponding with the @c id document identifier for this entity.
     * If no @c id has been set, returns an empty string.
     *
     * @sa setId
     *
     * @return The @c std::string document identifier for this entity.
     */
    std::string id() const;

    /**
     * @brief Remove the identifier for this entity.
     *
     * Clear the identifier for this entity. Future calls to id() will
     * return an empty string.
     */
    void removeId();

    /**
     * @brief Test to see if this entity is equal to the @p other entity.
     *
     * The attributes and properties of the entity are tested against the @p other
     * entity's attributes and properties, and compared for equality.
     *
     * If an entity has any children associated with it, then the order that children appear in
     * is **not** taken into account.
     *
     * The parents of either entity (if they exist) are ignored when determining equality.
     *
     * @param other The entity to test against.
     * @return @c true if this entity is equal to the @p other entity, @c false otherwise.
     */
    bool equals(const EntityPtr &other) const;

protected:
    /**
     * @brief Virtual equals(const EntityPtr &other) method to be implemented by derived classes.
     *
     * Virtual equals(const EntityPtr &other) method to allow entity classes to
     * implement their own versions. @private.
     *
     * @param other The other @ref Entity to perform equality against.
     *
     * @return True if the entities are equal, false otherwise.
     */
    virtual bool doEquals(const EntityPtr &other) const;

    class EntityImpl; /**< Forward declaration for pImpl idiom, @private. */

    explicit Entity(EntityImpl *derivedPimpl); /**< Constructor for derived classes, @private. */

    /**
     * @brief Getter for private implementation pointer.
     *
     * Getter for private implementation pointer, @private.
     *
     * @return A pointer to EntityImpl.
     */
    EntityImpl *pFunc()
    {
        return mPimpl;
    }

    /**
     * @brief Const getter for private implementation pointer.
     *
     * Const getter for private implementation pointer, @private.
     *
     * @return A pointer to const EntityImpl.
     */
    const EntityImpl *pFunc() const
    {
        return mPimpl;
    }

private:
    EntityImpl *mPimpl; /**< Private member to implementation pointer, @private. */
};

} // namespace libcellml
