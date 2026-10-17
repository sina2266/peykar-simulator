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

#include "libcellml/logger.h"
#include "libcellml/types.h"

namespace libcellml {

/**
 * @brief The Validator class.
 *
 * The Validator class is for representing a CellML Validator.
 */
class LIBCELLML_EXPORT Validator: public Logger
{
public:
    ~Validator() override; /**< Destructor, @private. */
    Validator(const Validator &rhs) = delete; /**< Copy constructor, @private. */
    Validator(Validator &&rhs) noexcept = delete; /**< Move constructor, @private. */
    Validator &operator=(Validator rhs) = delete; /**< Assignment operator, @private. */

    /**
     * @brief Create a @c Validator object.
     *
     * Factory method to create a @c Validator.  Create a
     * validator with::
     *
     *   ValidatorPtr validator = libcellml::Validator::create();
     *
     * @return A smart pointer to a @c Validator object.
     */
    static ValidatorPtr create() noexcept;

    /**
     * @brief Validate the @p model using the CellML 2.0 Specification.
     *
     * Validate the given @p model and its encapsulated entities using the
     * CellML 2.0 Specification. Any errors will be logged in the @c Validator.
     *
     * All existing issues will be removed before the model is validated.
     *
     * @param model The model to validate.
     */
    void validateModel(const ModelPtr &model);

private:
    Validator(); /**< Constructor, @private. */

    class ValidatorImpl; /**< Forward declaration for pImpl idiom, @private. */

    ValidatorImpl *pFunc(); /**< Getter for private implementation pointer, @private. */
};

} // namespace libcellml
