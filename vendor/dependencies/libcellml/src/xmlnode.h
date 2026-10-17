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

#include <map>
#include <memory>
#include <string>

#include "xmlattribute.h"

namespace libcellml {

class XmlNode; /**< Forward declaration of the internal XmlNode class. */
using XmlNodePtr = std::shared_ptr<XmlNode>; /**< Type definition for shared XML node pointer. */
/**
 * Type definition for the XML namespace map using XML namespace prefix
 * for the key and the XML namespace URI for the value.
 */
using XmlNamespaceMap = std::map<std::string, std::string>;

/**
 * @brief The XmlNode class.
 *
 * The XmlNode class is a wrapper class for operations on
 * xmlNode objects from libxml2.
 */
class XmlNode
{
public:
    XmlNode(); /**< Constructor, @private. */
    ~XmlNode(); /**< Destructor. */

    /**
     * @brief Set the internal @c xmlNode for this @c XmlNode wrapper.
     *
     * Sets the @c pImpl of the libxml2 xmlNode attribute
     * for this @c XmlNode.
     *
     * @param node The libxml2 @c xmlNodePtr to set.
     */
    void setXmlNode(const xmlNodePtr &node);

    /**
     * @brief Get the namespace URI of the XML element.
     *
     * Get the namespace URI of the XML element.
     *
     * @return A @c std::string representation of the XML namespace URI.
     */
    std::string namespaceUri() const;

    /**
     * @brief Add a namespace definition to this XML element.
     *
     * Add a libXml2 namespace definition to this XML element using the given URI
     * and prefix.
     *
     * @param uri The @c std::string representation of the XML namespace URI.
     * @param prefix The @c std::string representation of the XML namespace prefix.
     */
    void addNamespaceDefinition(const std::string &uri, const std::string &prefix);

    /**
     * @brief Remove the namespace definition from this XML element.
     *
     * Remove all matching namespace definitions that match the given URI.
     * The URI is compared against the namespace href attribute and if
     * a match is found the namespace definition is removed from this XML
     * elements list of namespace definitions.
     *
     * If the given XML namespace URI does not match any of the namespaces
     * in the namespace definition list then nothing is changed.
     *
     * @param uri The @c std::string representation of the XML namespace URI.
     */
    void removeNamespaceDefinition(const std::string &uri);

    /**
     * @brief Test if this XML element has the given namespace definition.
     *
     * Test to see if this XML element has the given namespace definition defined
     * on it.  The URI is compared against the namespace definition href attribute.
     * If a match is found true is returned, otherwise false is.
     *
     * @param uri The @c std::string representation of the XML namespace URI.
     * @return true if this element has the given namespace definition, false otherwise.
     */
    bool hasNamespaceDefinition(const std::string &uri);

    /**
     * @brief Get the namespaces defined on this XML element.
     *
     * Get the namespaces defined on this XML element as a map.  The map is constructed
     * from the XML namespace prefix and XML namespace URI as key and value for the map
     * respectively.
     *
     * @return A map with a key of an XML namespace prefix and a value of an XML namespace URI.
     */
    XmlNamespaceMap definedNamespaces() const;

    /**
     * @brief Check if this @c XmlNode is an element node in the given
     * namespace with the specified local name.
     *
     * Checks whether this @c XmlNode is an element type node in the
     * given namespace with the specified local name.
     * Returns @c true if so, and @c false otherwise.
     *
     * @param name The @c char element name to check for.
     * @param ns The @c char namespace in which the element
     * node is to be defined.
     *
     * @return @c true if this @c XmlNode is an element node in the
     * given namespace @p ns with the given local name @p name;
     * and @c false otherwise.
     */
    bool isElement(const char *name, const char *ns) const;

    /**
     * @brief Test if this @c XmlNode is an XML element node.
     *
     * Test if this @c XmlNode is an XML element node.
     *
     * @return @c true if this @c XmlNode is an XML element node and @c false
     * otherwise.
     */
    bool isElement() const;

    /**
     * @brief Check if this @c XmlNode is an element node in the
     * CellML 1.X or 2.0 namespace with the given local name.
     *
     * Checks whether this @c XmlNode is an element node in
     * the CellML 1.X or 2.0 namespace with the specified local name.
     * Returns @p true if so, and @c false otherwise.
     *
     * @param name The @c char element name to check for.
     *
     * @return @c true if this @c XmlNode is an element node in the
     * CellML 1.X or 2.0 namespace with the given local name @p name; and
     * @c false otherwise.
     */
    bool isCellmlElement(const char *name = nullptr) const;

    /**
     * @brief Check if this @c XmlNode is an element node in the
     * CellML 2.0 namespace with the given local name.
     *
     * Checks whether this @c XmlNode is an element node in
     * the CellML 2.0 namespace with the specified local name.
     * Returns @p true if so, and @c false otherwise.
     *
     * @param name The @c char element name to check for.
     *
     * @return @c true if this @c XmlNode is an element node in the
     * CellML 2.0 namespace with the given local name @p name; and
     * @c false otherwise.
     */
    bool isCellml20Element(const char *name = nullptr) const;

    /**
     * @brief Check if this @c XmlNode is an element node in the
     * CellML 1.0 namespace with the given local name.
     *
     * Checks whether this @c XmlNode is an element node in
     * the CellML 1.0 namespace with the specified local name.
     * Returns @p true if so, and @c false otherwise.
     *
     * @param name The @c char element name to check for.
     *
     * @return @c true if this @c XmlNode is an element node in the
     * CellML 1.0 namespace with the given local name @p name; and
     * @c false otherwise.
     */
    bool isCellml10Element(const char *name = nullptr) const;

    /**
     * @brief Check if this @c XmlNode is an element node in the
     * CellML 1.1 namespace with the given local name.
     *
     * Checks whether this @c XmlNode is an element node in
     * the CellML 1.1 namespace with the specified local name.
     * Returns @p true if so, and @c false otherwise.
     *
     * @param name The @c char element name to check for.
     *
     * @return @c true if this @c XmlNode is an element node in the
     * CellML 1.1 namespace with the given local name @p name; and
     * @c false otherwise.
     */
    bool isCellml11Element(const char *name = nullptr) const;

    /**
     * @brief Check if this @c XmlNode is an element node in the
     * CellML 1.0 or CellML 1.1 namespace with the given local name.
     *
     * Checks whether this @c XmlNode is an element node in
     * the CellML 1.0 or CellML 1.1 namespace with the specified local name.
     * Returns @p true if so, and @c false otherwise.
     *
     * @param name The @c char element name to check for.
     *
     * @return @c true if this @c XmlNode is an element node in the
     * CellML 1.0 or CellML 1.1 namespace with the given local name @p name; and
     * @c false otherwise.
     */
    bool isCellml1XElement(const char *name = nullptr) const;

    /**
     * @brief Check if this @c XmlNode is an element node in the
     * MathML namespace with the given local name.
     *
     * Checks whether this @c XmlNode is an element node in
     * the MathML namespace with the specified local name.
     * Returns @p true if so, and @c false otherwise.
     *
     * @param name The @c char element name to check for.
     *
     * @return @c true if this @c XmlNode is an element node in the
     * MathML namespace with the given local name @p name; and
     * @c false otherwise.
     */
    bool isMathmlElement(const char *name = nullptr) const;

    /**
     * @brief Check if this @c XmlNode is a text node.
     *
     * Checks whether this @c XmlNode is a text node.
     * Returns @c true if so, and @c false otherwise.
     *
     * @return @c true if this @c XmlNode is a text node and @c false otherwise.
     */
    bool isText() const;

    /**
     * @brief Check if this @c XmlNode is a text node representing a basic real number.
     *
     * Checks whether this @c XmlNode is a text node representing a basic real number.
     * Returns @c true if so, and @c false otherwise.
     *
     * @return @c true if this @c XmlNode is a text node representing a basic real number
     * and @c false otherwise.
     */
    bool isBasicReal() const;

    /**
     * @brief Check if this @c XmlNode is a text node representing an integer.
     *
     * Checks whether this @c XmlNode is a text node representing an integer.
     * Returns @c true if so, and @c false otherwise.
     *
     * @return @c true if this @c XmlNode is a text node representing an integer
     * and @c false otherwise.
     */
    bool isInteger() const;

    /**
     * @brief Check if this @c XmlNode is a comment node.
     *
     * Checks whether this @c XmlNode is a comment node.
     * Returns @c true if so, and @c false otherwise.
     *
     * @return @c true if this @c XmlNode is a comment node and @c false
     * otherwise.
     */
    bool isComment() const;

    /**
     * @brief Get the name of the XML element.
     *
     * Get the name of the XML element.
     *
     * @return A @c std::string representation of the XML name.
     */
    std::string name() const;

    /**
     * @brief Check if this @c XmlNode has the specified attribute.
     *
     * Checks whether this @c XmlNode has an attribute of the type
     * specified by the argument @p attributeName. Returns @c true
     * if so, and @c false otherwise.
     *
     * @param attributeName The @c char attribute type to check for.
     *
     * @return @c true if this @c XmlNode has an attribute of the type
     * specified by the @p attributeName and @c false otherwise.
     */
    bool hasAttribute(const char *attributeName) const;

    /**
     * @brief Get the attribute of the specified type for this @c XmlNode
     *
     * Get the @c std::string corresponding with the attribute value
     * of the type @p attributeName for this @c  XmlNode.
     *
     * @param attributeName The @c char attribute type.
     *
     * @return The @c std::string form of the attribute value of the
     * specified type.
     */
    std::string attribute(const char *attributeName) const;

    /**
     * @brief Set the attribute of the given name to the given value.
     *
     * Set the attribute of the given @p attributeName to the given @p attributeValue.
     * The attribute name must already exist.
     * If the attribute name does not already exist the attribute value will not be modified.
     *
     * @param attributeName The @c char attribute type.
     * @param attributeValue The @c char value of the attribute to set.
     */
    void setAttribute(const char *attributeName, const char *attributeValue);

    /**
     * @brief Get the first attribute for this @c XmlNode
     *
     * Get the @c XmlAttributePtr corresponding with the first attribute
     * for this @c  XmlNode.
     *
     * @return The @c XmlAttributePtr form of the first attribute
     * for this @c XmlNode.
     */
    XmlAttributePtr firstAttribute() const;

    /**
     * @brief Test if this @c XmlNode is the given node.
     *
     * Test if this @c XmlNode is the given node. If it is true is returned,
     * otherwise false is.
     *
     * @param node The @c XmlNode to test against.
     *
     * @return true if this @c XmlNode is the given node, false otherwise.
     */
    bool equals(const XmlNodePtr &node) const;

    /**
     * @brief Get the first child for this @c XmlNode.
     *
     * Gets the first child @c XmlNode for this @c XmlNode based on the
     * ordering from the deserialised @c XmlDoc. If no child
     * node exists, returns @c nullptr.
     *
     * @return The @c XmlNodePtr to the first child node for this @c XmlNode.
     */
    XmlNodePtr firstChild() const;

    /**
     * @brief Get the @c XmlNode immediately following this @c XmlNode.
     *
     * Gets the next @c XmlNode immediately following this @c XmlNode based
     * on the ordering from the deserialised @c  XmlDoc. If no
     * next node exists, returns @c nullptr.
     *
     * @return The @c XmlNodePtr to the next node following this @c XmlNode.
     */
    XmlNodePtr next() const;

    /**
     * @brief Get the @c XmlNode parent of this @c XmlNode.
     *
     * Gets the parent @c XmlNode of this @c XmlNode based
     * on the ordering from the deserialised @c XmlDoc. If no
     * parent node exists, returns @c nullptr.
     *
     * @return The @c XmlNodePtr to the parent of this @c XmlNode.
     */
    XmlNodePtr parent() const;

    /**
     * @brief Convert this @c XmlNode content into a @c std::string.
     *
     * Converts the content in this @c XmlNode (including all children and
     * attributes) into a @c std::string.
     *
     * @return The @c std::string representation of the content for this @c XmlNode.
     */
    std::string convertToString() const;

    /**
     * @brief Convert this @c XmlNode content into a stripped @c std::string.
     *
     * Converts the content in this @c XmlNode (including all children and
     * attributes) into a @c std::string stripping away any whitespace from the
     * beginning and the end of the @c XmlNode.
     *
     * @return The stripped @c std::string representation of the content for this @c XmlNode.
     */
    std::string convertToStrippedString() const;

private:
    struct XmlNodeImpl; /**< Forward declaration for pImpl idiom, @private. */
    XmlNodeImpl *mPimpl; /**< Private member to implementation pointer, @private. */
};

} // namespace libcellml
