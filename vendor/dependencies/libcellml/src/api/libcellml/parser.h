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

#include "libcellml/logger.h"
#include "libcellml/strict.h"
#include "libcellml/types.h"

namespace libcellml {

/**
 * @brief The Parser class.
 *
 * The Parser class is for representing a CellML Parser.
 */
class LIBCELLML_EXPORT Parser: public Logger, public Strict
{
public:
    ~Parser() override; /**< Destructor, @private. */
    Parser(const Parser &rhs) = delete; /**< Copy constructor, @private. */
    Parser(Parser &&rhs) noexcept = delete; /**< Move constructor, @private. */
    Parser &operator=(Parser rhs) = delete; /**< Assignment operator, @private. */

    /**
     * @brief Create a @c Parser object.
     *
     * Factory method to create a @c Parser.  Create a
     * strict parser with:
     *
     * @code
     *   ParserPtr parser = libcellml::Parser::create();
     * @endcode
     *
     * Create a parser with the strict flag set to @c false with:
     *
     * @code
     *   ParserPtr parser = libcellml::Parser::create(false);
     * @endcode
     *
     * @param strict [optional] A boolean value to set the strict flag to.
     *
     * @return A smart pointer to a @c Parser object.
     */
    static ParserPtr create(bool strict = true) noexcept;

    /**
     * @brief Create and populate a new model from a @c std::string.
     *
     * Creates and populates a new model pointer by parsing CellML
     * entities and attributes from the @p input @c std::string.
     *
     * All existing issues will be removed before the input is parsed.
     *
     * Returns a @c nullptr if the @p input is not a @c std::string representation
     * of a CellML model.
     *
     * @param input The string to parse into a model.
     *
     * @return The new @c ModelPtr deserialised from the input string.
     */
    ModelPtr parseModel(const std::string &input);

#ifdef JAVASCRIPT_BINDINGS
#    include "strict.impl"
#endif

private:
    Parser(); /**< Constructor, @private. */

    class ParserImpl; /**< Forward declaration for pImpl idiom, @private. */

    ParserImpl *pFunc(); /**< Getter for private implementation pointer, @private. */
};

} // namespace libcellml
