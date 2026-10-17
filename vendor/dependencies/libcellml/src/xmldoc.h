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

#include <memory>
#include <string>

#include "xmlnode.h"

namespace libcellml {

class XmlDoc; /**< Forward declaration of the internal XmlDoc class. */
using XmlDocPtr = std::shared_ptr<XmlDoc>; /**< Type definition for shared XML doc pointer. */

/**
 * @brief The XmlDoc class.
 *
 * The XmlDoc class is a wrapper class for operations on
 * xmlDoc objects from libxml2.
 */
class XmlDoc
{
public:
    XmlDoc(); /**< Constructor, @private. */
    ~XmlDoc(); /**< Destructor. */

    /**
     * @brief Parse an XML document from a string.
     *
     * Parses the @p input @c std::string as an XML document.
     *
     * @param input The @c std::string to parse.
     */
    void parse(const std::string &input);

    /**
     * @brief Parse an XML string as MathML.
     *
     * Parses the @p input @c std::string as a MathML string.
     *
     * @param input The @c std::string to parse.
     */
    void parseMathML(const std::string &input);

    /**
     * @brief Convert this @c XmlDoc content into a pretty-print @c std::string.
     *
     * Converts the content in this @c XmlDoc into a pretty-print
     * @c std::string.
     *
     * @return The pretty-print @c std::string representation of the content for
     * this @c XmlDoc.
     */
    std::string prettyPrint() const;

    /**
     * @brief Get the root XML element of this @c XmlDoc.
     *
     * Get the root XML element for this @c XmlDoc as a returned @c XMLNodePtr.
     *
     * @return The root XML element for this @c XmlDoc.
     */
    XmlNodePtr rootNode() const;

    /**
     * @brief Add an @p error raised while parsing this @c XmlDoc.
     *
     * Adds the @p error raise while parsing this @c XmlDoc to the list of
     * @c mXmlErrors. These errors will pertain to the ability of libxml to
     * parse a given document.
     *
     * @param error The XML error string to add.
     */
    void addXmlError(const std::string &error);

    /**
     * @brief Count the number of XML errors in this @c XmlDoc.
     *
     * Returns the number of XML errors raised while parsing this @c XmlDoc.
     *
     * @return The number of XML errors.
     */
    size_t xmlErrorCount() const;

    /**
     * @brief Get a XML error at index.
     *
     * Returns the @c std::string message pertaining to the error at the
     * @p index raised by libxml during parsing.
     *
     * @param index The index of the error to get.
     *
     * @return The @c std::string form of the XML error.
     */
    std::string xmlError(size_t index) const;

private:
    struct XmlDocImpl; /**< Forward declaration for pImpl idiom, @private. */
    XmlDocImpl *mPimpl; /**< Private member to implementation pointer, @private. */
};

} // namespace libcellml
