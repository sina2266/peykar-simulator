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
 * @brief The ImportedEntity class.
 *
 * Base class for all importable libCellML classes.
 */
class LIBCELLML_EXPORT ImportedEntity
{
public:
    virtual ~ImportedEntity() = 0; /**< Destructor. */
    ImportedEntity(const ImportedEntity &rhs) = delete; /**< Copy constructor. */
    ImportedEntity(ImportedEntity &&rhs) noexcept = delete; /**< Move constructor. */

    /**
     * @brief Test if this entity is an imported entity.
     *
     * Method to test if this entity is an imported entity.  Returns
     * @c true if the import is not the @c nullptr otherwise return @c false.
     *
     * @return @c true if the import is not the @c nullptr, @c false otherwise.
     */
    bool isImport() const;

    /**
     * @brief Get the import source.
     *
     * Get the import source, if no import source is set @c nullptr is returned.
     *
     * @sa setImportSource
     *
     * @return The shared pointer for the import source, if no import source is
     * set returns @c nullptr.
     */
    ImportSourcePtr importSource() const;

    /**
     * @brief Set the import source.
     *
     * Set the import source for the imported entity.  Set to @c nullptr to
     * unset the import source.
     *
     * @sa importSource
     *
     * @param importSource The import source to set.
     */
    void setImportSource(const ImportSourcePtr &importSource);

    /**
     * @brief Get the import reference.
     *
     * Get the reference to the entity in the imported model.
     *
     * @sa setImportReference
     *
     * @return The reference to the entity in the imported model, the empty
     * string if it is not set.
     */
    std::string importReference() const;

    /**
     * @brief Set the import reference.
     *
     * Set the import reference to an entity in the imported model.  The import
     * reference should be a Component or a Unit in the import model.
     *
     * @sa importReference
     *
     * @param reference The name of the reference to refer to in the import model.
     */
    void setImportReference(const std::string &reference);

    /**
     * @brief Test whether this entity has been resolved.
     *
     * Test whether this entity is resolved or not.
     *
     * An entity that is not imported is always resolved so this method
     * returns @c true. Alternatively, if this entity is imported then
     * it returns @c true if every entity that this imported entity requires
     * can be found. That is, return @c true if this imported entity is resolvable.
     * In all other cases, this method returns @c false.
     *
     * @return @c true if the import is resolved, @c false otherwise.
     */
    bool isResolved() const;

protected:
    ImportedEntity(); /**< Constructor, @private. */

    /**
     * @brief Virtual is resolved method to be implemented by derived classes.
     *
     * Virtual isResolved method to allow the @ref Units and @ref Component classes to
     * implement their own versions.
     *
     * @return @c true if this imported entity is resolved and @c false otherwise.
     */
    virtual bool doIsResolved() const = 0;

    bool doEquals(const ImportedEntityPtr &other) const; /**< Implementation method for equals, @private. */

private:
    struct ImportedEntityImpl;
    ImportedEntityImpl *mPimpl; /**< Private member to implementation pointer, @private. */
};

} // namespace libcellml
