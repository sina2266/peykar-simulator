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

namespace libcellml {

/**
 * @brief The Strict class.
 *
 * Base class for all libCellML classes that need to have a strict interface.
 */
class LIBCELLML_EXPORT Strict
{
public:
    virtual ~Strict() = 0; /**< Destructor. */
    Strict(const Strict &rhs) = delete; /**< Copy constructor. */
    Strict(Strict &&rhs) noexcept = delete; /**< Move constructor. */

    /**
     * @brief Set the strict flag for the object.
     *
     * Set the strict flag to the value of the @p strict parameter.
     *
     * @sa isStrict
     *
     * @param strict The boolean value to set.
     */
    void setStrict(bool strict);

    /**
     * @brief Is the object in strict mode.
     *
     * Determine if the object has the strict flag set.
     *
     * @sa setStrict
     *
     * @return @c true if the object has the strict flag set, @c false otherwise.
     */
    bool isStrict() const;

protected:
    class StrictImpl; /**< Forward declaration for pImpl idiom, @private. */

    Strict(); /**< Constructor for derived classes, @private. */

    /**
     * @brief Getter for private implementation pointer.
     *
     * Getter for private implementation pointer, @private.
     *
     * @return A pointer to EntityImpl.
     */
    StrictImpl *pFunc()
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
    const StrictImpl *pFunc() const
    {
        return mPimpl;
    }

private:
    StrictImpl *mPimpl; /**< Private member to implementation pointer, @private. */
};

} // namespace libcellml
