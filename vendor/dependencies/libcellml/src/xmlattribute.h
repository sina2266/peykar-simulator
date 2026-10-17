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

#include <libxml/tree.h>
#include <memory>
#include <string>

namespace libcellml {

class XmlAttribute; /**< Forward declaration of the internal XmlAttribute class. */
using XmlAttributePtr = std::shared_ptr<XmlAttribute>; /**< Type definition for shared XML attribute pointer. */

/**
 * @brief The XmlAttribute class.
 *
 * The XmlAttribute class is a wrapper class for operations on
 * xmlAttribute objects from libxml2.
 */
class XmlAttribute
{
public:
    XmlAttribute(); /**< Constructor, @private. */
    ~XmlAttribute(); /**< Destructor. */

    /**
     * @brief Set the internal xmlAttribute for this XmlAttribute wrapper.
     *
     * Sets the @c pImpl of the libxml2 xmlAttribute attribute
     * for this XmlAttribute.
     *
     * @param attribute The libxml2 @c xmlAttributePtr to set.
     */
    void setXmlAttribute(const xmlAttrPtr &attribute);

    /**
     * @brief Get the namespace URI of this XmlAttribute.
     *
     * Get the namespace URI of this XmlAttribute.
     *
     * @return A @c std::string representation of the XML namespace URI.
     */
    std::string namespaceUri() const;

    /**
     * @brief Get the namespace prefix of this XmlAttribute.
     *
     * Get the namespace prefix of this XmlAttribute.
     *
     * @return A @c std::string representation of the XML namespace prefix.
     */
    std::string namespacePrefix() const;

    /**
     * @brief Test if this XmlAttribute is in the given namespace.
     *
     * Test if this XmlAttribute is in the given namespace. Return @c true
     * if this XmlAttribute is in the given namespace and @c false otherwise.
     *
     * @param ns The @c char namespace in which the attribute is in.
     *
     * @return @c true if this XmlAttribute is in the namespace
     * specified by @p ns and @c false otherwise.
     */
    bool inNamespaceUri(const char *ns) const;

    /**
     * @brief Check if this XmlAttribute is of the named attribute type in the
     * given namespace.
     *
     * Checks whether this XmlAttribute has the argument attribute type name in
     * the given namespace.
     * Returns @c true if so, and @c false otherwise.
     *
     * @param name The @c char attribute type name to check for.
     * @param ns The @c char namespace in which the attribute
     * type name is to be defined.
     *
     * @return @c true if this XmlAttribute is of the attribute type
     * specified by the @p name in the namespace @p ns
     * and @c false otherwise.
     */
    bool isType(const char *name, const char *ns = "") const;

    /**
     * @brief Check if this XmlAttribute is of the named attribute type in the
     * CellML 2.0 namespace.
     *
     * Checks whether this XmlAttribute has the argument attribute type name in
     * the CellML 2.0 namespace.
     * Returns @c true if so, and @c false otherwise.
     *
     * @param name The @c char attribute type name to check for.
     *
     * @return @c true if this XmlAttribute is of the attribute type
     * specified by the @p name in the CellML 2.0 namespace
     * and @c false otherwise.
     */
    bool isCellmlType(const char *name) const;

    /**
     * @brief Get the name of this XmlAttribute.
     *
     * Gets the name of this XmlAttribute and returns it as a @c std::string.
     *
     * @return The @c std::string corresponding with the name of this XmlAttribute.
     */
    std::string name() const;

    /**
     * @brief Get the value of this XmlAttribute.
     *
     * Gets the value of this XmlAttribute and returns it as a @c std::string.
     *
     * @return The @c std::string corresponding with the value of this XmlAttribute.
     */
    std::string value() const;

    /**
     * @brief Get the XmlAttribute immediately following this XmlAttribute.
     *
     * Gets the next XmlAttribute immediately following this XmlAttribute based
     * on the ordering from the parent XmlNode. If no
     * next attribute exists, returns @c nullptr.
     *
     * @return The XmlAttributePtr to the next attribute following this XmlAttribute.
     */
    XmlAttributePtr next() const;

    /**
     * @brief Remove this XmlAttribute from its parent XmlNode.
     *
     * Remove this XmlAttribute from its parent XmlNode.
     */
    void removeAttribute();

    /**
     * @brief Set the namespace prefix for this XmlAttribute.
     *
     * Set the namespace prefix for this XmlAttribute.
     * The existing attribute is removed and replaced with an attribute that has the
     * same value and the given namespace prefix.
     *
     * @param prefix A @c std::string representation of the XML namespace prefix.
     */
    void setNamespacePrefix(const std::string &prefix);

private:
    struct XmlAttributeImpl; /**< Forward declaration for pImpl idiom, @private. */
    XmlAttributeImpl *mPimpl; /**< Private member to implementation pointer, @private. */
};

} // namespace libcellml
