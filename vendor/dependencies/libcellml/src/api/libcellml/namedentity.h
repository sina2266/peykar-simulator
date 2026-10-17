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
 * @brief The NamedEntity class.
 *
 * Base class for all nameable libCellML classes.
 */
class LIBCELLML_EXPORT NamedEntity: public ParentedEntity
{
public:
    ~NamedEntity() override = default; /**< Destructor, @private. */
    NamedEntity(const NamedEntity &rhs) = delete; /**< Copy constructor, @private. */
    NamedEntity(NamedEntity &&rhs) noexcept = delete; /**< Move constructor, @private. */
    NamedEntity &operator=(NamedEntity rhs) = delete; /**< Assignment operator, @private. */

    /**
     * @brief Set the name for the Entity.
     *
     * This method sets the name parameter of the Entity.  It does not check
     * the validity of the name.
     *
     * @param name A string to represent the name.
     */
    void setName(const std::string &name);

    /**
     * @brief Get the name of the Entity.
     *
     * Gets the name of the Entity as a string.
     *
     * @return @c std::string representation of the Entity name.
     */
    std::string name() const;

    /**
     * @brief Remove the name of the Entity.
     *
     * Remove the name of the Entity.
     */
    void removeName();

protected:
    bool doEquals(const EntityPtr &other) const override; /**< Virtual implementation method for equals, @private. */

    class NamedEntityImpl; /**< Forward declaration for pImpl idiom, @private. */
    explicit NamedEntity(NamedEntityImpl *pImpl); /**< Constructor for derived classes, @private. */

private:
    NamedEntityImpl *pFunc(); /**< Getter for private implementation pointer, @private. */
    const NamedEntityImpl *pFunc() const; /**< Const getter for private implementation pointer, @private. */
};

} // namespace libcellml
