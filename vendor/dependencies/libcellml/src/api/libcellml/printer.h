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
#include "libcellml/logger.h"
#include "libcellml/types.h"

namespace libcellml {

/**
 * @brief The Printer class.
 *
 * The Printer class is for representing a CellML Printer.
 */
class LIBCELLML_EXPORT Printer: public Logger
{
public:
    ~Printer(); /**< Destructor, @private. */
    Printer(const Printer &rhs) = delete; /**< Copy constructor, @private. */
    Printer(Printer &&rhs) noexcept = delete; /**< Move constructor, @private. */
    Printer &operator=(Printer rhs) = delete; /**< Assignment operator, @private. */

    /**
     * @brief Create a @c Printer object.
     *
     * Factory method to create a @c Printer.  Create a
     * printer with::
     *
     *   PrinterPtr printer = libcellml::Printer::create();
     *
     * @return A smart pointer to a @c Printer object.
     */
    static PrinterPtr create() noexcept;

    /**
     * @brief Serialise the @ref Model to @c std::string.
     *
     * Serialise the given @p model to a std::string.
     * Has an optional argument to automatically add identifiers to all elements in the resulting document.
     *
     * @param model The @ref Model to serialise.
     * @param autoIds Optional argument that when @c true will add identifiers to all elements in the resulting document.
     *
     * @return The @c std::string representation of the @ref Model.
     */
    std::string printModel(const ModelPtr &model, bool autoIds = false);

private:
    Printer(); /**< Constructor, @private. */

    class PrinterImpl; /**< Forward declaration for pImpl idiom, @private. */
    PrinterImpl *pFunc(); /**< Getter for private implementation pointer, @private. */
};

} // namespace libcellml
