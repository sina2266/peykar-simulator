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

#include "xmlnode.h"

#include <algorithm>
#include <libxml/tree.h>
#include <string>

#include "namespaces.h"
#include "utilities.h"
#include "xmlattribute.h"

namespace libcellml {

/**
 * @brief The XmlNode::XmlNodeImpl struct.
 *
 * This struct is the private implementation struct for the XmlNode class.  Separating
 * the implementation from the definition allows for greater flexibility when
 * distributing the code.
 */
struct XmlNode::XmlNodeImpl
{
    xmlNodePtr mXmlNodePtr;
};

XmlNode::XmlNode()
    : mPimpl(new XmlNodeImpl())
{
}

XmlNode::~XmlNode()
{
    delete mPimpl;
}

void XmlNode::setXmlNode(const xmlNodePtr &node)
{
    mPimpl->mXmlNodePtr = node;
}

std::string XmlNode::namespaceUri() const
{
    if (mPimpl->mXmlNodePtr->ns == nullptr) {
        return {};
    }
    return reinterpret_cast<const char *>(mPimpl->mXmlNodePtr->ns->href);
}

void XmlNode::addNamespaceDefinition(const std::string &uri, const std::string &prefix)
{
    xmlNsPtr nsPtr = xmlNewNs(mPimpl->mXmlNodePtr, reinterpret_cast<const xmlChar *>(uri.c_str()), reinterpret_cast<const xmlChar *>(prefix.c_str()));
    auto last = mPimpl->mXmlNodePtr->nsDef;
    while (last != nullptr) {
        last = last->next;
    }
    last = nsPtr;
}

void clearNamespace(const xmlNodePtr &node, xmlNsPtr ns)
{
    if (node->ns == ns) {
        node->ns = nullptr;
    }
    xmlAttrPtr attr = node->properties;
    while (attr != nullptr) {
        if (attr->ns == ns) {
            attr->ns = nullptr;
        }
        attr = attr->next;
    }
    if (node->children != nullptr) {
        clearNamespace(node->children, ns);
    }
    if (node->next != nullptr) {
        clearNamespace(node->next, ns);
    }
}

void XmlNode::removeNamespaceDefinition(const std::string &uri)
{
    xmlNsPtr previous = nullptr;
    xmlNsPtr next = nullptr;
    xmlNsPtr namespaceToRemove = nullptr;
    auto current = mPimpl->mXmlNodePtr->nsDef;
    while (current != nullptr) {
        next = current->next;
        namespaceToRemove = nullptr;
        if (xmlStrcmp(reinterpret_cast<const xmlChar *>(uri.c_str()), reinterpret_cast<const xmlChar *>(current->href)) == 0) {
            namespaceToRemove = current;
        } else {
            previous = current;
        }
        current = current->next;
        if (namespaceToRemove != nullptr) {
            if (previous == nullptr) {
                mPimpl->mXmlNodePtr->nsDef = next;
            } else {
                previous->next = next;
            }
            namespaceToRemove->next = nullptr;
            // Search subtree of this node and clear uses of the namespace.
            clearNamespace(mPimpl->mXmlNodePtr, namespaceToRemove);
            xmlFreeNs(namespaceToRemove);
        }
    }
}

bool XmlNode::hasNamespaceDefinition(const std::string &uri)
{
    if (mPimpl->mXmlNodePtr->nsDef != nullptr) {
        auto next = mPimpl->mXmlNodePtr->nsDef;
        while (next != nullptr) {
            // If you have a namespace, the href cannot be empty.
            std::string href = std::string(reinterpret_cast<const char *>(next->href));
            if (href == uri) {
                return true;
            }
            next = next->next;
        }
    }
    return false;
}

XmlNamespaceMap XmlNode::definedNamespaces() const
{
    XmlNamespaceMap namespaceMap;
    if (mPimpl->mXmlNodePtr->nsDef != nullptr) {
        auto next = mPimpl->mXmlNodePtr->nsDef;
        while (next != nullptr) {
            std::string prefix;
            if (next->prefix != nullptr) {
                prefix = std::string(reinterpret_cast<const char *>(next->prefix));
            }
            // If you have a namespace, the href cannot be empty.
            std::string href = std::string(reinterpret_cast<const char *>(next->href));
            namespaceMap.emplace(prefix, href);
            next = next->next;
        }
    }
    return namespaceMap;
}

bool XmlNode::isElement(const char *name, const char *ns) const
{
    bool found = false;
    if ((mPimpl->mXmlNodePtr->type == XML_ELEMENT_NODE)
        && (xmlStrcmp(reinterpret_cast<const xmlChar *>(namespaceUri().c_str()), reinterpret_cast<const xmlChar *>(ns)) == 0)
        && ((name == nullptr) || (xmlStrcmp(mPimpl->mXmlNodePtr->name, reinterpret_cast<const xmlChar *>(name)) == 0))) {
        found = true;
    }
    return found;
}

bool XmlNode::isElement() const
{
    return mPimpl->mXmlNodePtr->type == XML_ELEMENT_NODE;
}

bool XmlNode::isCellmlElement(const char *name) const
{
    return isCellml20Element(name) || isCellml11Element(name) || isCellml10Element(name);
}

bool XmlNode::isCellml20Element(const char *name) const
{
    return isElement(name, CELLML_2_0_NS);
}

bool XmlNode::isCellml10Element(const char *name) const
{
    return isElement(name, CELLML_1_0_NS);
}

bool XmlNode::isCellml11Element(const char *name) const
{
    return isElement(name, CELLML_1_1_NS);
}

bool XmlNode::isCellml1XElement(const char *name) const
{
    return isCellml10Element(name) || isCellml11Element(name);
}

bool XmlNode::isMathmlElement(const char *name) const
{
    return isElement(name, MATHML_NS);
}

bool XmlNode::isText() const
{
    return mPimpl->mXmlNodePtr->type == XML_TEXT_NODE;
}

bool XmlNode::isBasicReal() const
{
    return canConvertToBasicDouble(convertToStrippedString());
}

bool XmlNode::isInteger() const
{
    int dummyInt;
    return convertToInt(convertToStrippedString(), dummyInt);
}

bool XmlNode::isComment() const
{
    return mPimpl->mXmlNodePtr->type == XML_COMMENT_NODE;
}

std::string XmlNode::name() const
{
    return reinterpret_cast<const char *>(mPimpl->mXmlNodePtr->name);
}

bool XmlNode::hasAttribute(const char *attributeName) const
{
    xmlAttrPtr attribute = xmlHasProp(mPimpl->mXmlNodePtr, reinterpret_cast<const xmlChar *>(attributeName));
    return attribute != nullptr;
}

xmlNsPtr getAttributeNamespace(const xmlNodePtr &node, const char *attributeName)
{
    return xmlHasProp(node, reinterpret_cast<const xmlChar *>(attributeName))->ns;
}

std::string XmlNode::attribute(const char *attributeName) const
{
    std::string attributeValueString;
    if (hasAttribute(attributeName)) {
        xmlChar *attributeValue = xmlGetProp(mPimpl->mXmlNodePtr, reinterpret_cast<const xmlChar *>(attributeName));
        attributeValueString = std::string(reinterpret_cast<const char *>(attributeValue));
        xmlFree(attributeValue);
    }
    return attributeValueString;
}

void XmlNode::setAttribute(const char *attributeName, const char *attributeValue)
{
    if (hasAttribute(attributeName)) {
        auto ns = getAttributeNamespace(mPimpl->mXmlNodePtr, attributeName);
        xmlSetNsProp(mPimpl->mXmlNodePtr, ns, reinterpret_cast<const xmlChar *>(attributeName), reinterpret_cast<const xmlChar *>(attributeValue));
    }
}

XmlAttributePtr XmlNode::firstAttribute() const
{
    xmlAttrPtr attribute = mPimpl->mXmlNodePtr->properties;
    XmlAttributePtr attributeHandle = nullptr;
    if (attribute != nullptr) {
        attributeHandle = std::make_shared<XmlAttribute>();
        attributeHandle->setXmlAttribute(attribute);
    }
    return attributeHandle;
}

bool XmlNode::equals(const XmlNodePtr &node) const
{
    return mPimpl->mXmlNodePtr == node->mPimpl->mXmlNodePtr;
}

XmlNodePtr XmlNode::firstChild() const
{
    xmlNodePtr child = mPimpl->mXmlNodePtr->children;
    XmlNodePtr childHandle = nullptr;
    while (child != nullptr) {
        childHandle = std::make_shared<XmlNode>();
        childHandle->setXmlNode(child);
        bool textNode = childHandle->isText();
        if (!textNode || !childHandle->convertToStrippedString().empty()) {
            break;
        }
        child = child->next;
    }
    return childHandle;
}

XmlNodePtr XmlNode::next() const
{
    xmlNodePtr next = mPimpl->mXmlNodePtr->next;
    XmlNodePtr nextHandle = nullptr;
    if (next != nullptr) {
        nextHandle = std::make_shared<XmlNode>();
        nextHandle->setXmlNode(next);
    }
    return nextHandle;
}

XmlNodePtr XmlNode::parent() const
{
    xmlNodePtr parent = mPimpl->mXmlNodePtr->parent;
    XmlNodePtr parentHandle = std::make_shared<XmlNode>();
    parentHandle->setXmlNode(parent);
    return parentHandle;
}

std::string XmlNode::convertToString() const
{
    xmlKeepBlanksDefault(1);
    xmlBufferPtr buffer = xmlBufferCreate();
    xmlNodeDump(buffer, mPimpl->mXmlNodePtr->doc, mPimpl->mXmlNodePtr, 0, 0);
    std::string contentString = std::string(reinterpret_cast<const char *>(buffer->content));
    xmlBufferFree(buffer);
    return contentString;
}

std::string XmlNode::convertToStrippedString() const
{
    std::string contentString = convertToString();
    contentString.erase(contentString.begin(), find_if_not(contentString.begin(), contentString.end(), [](int c) { return isspace(c); }));
    contentString.erase(find_if_not(contentString.rbegin(), contentString.rend(), [](int c) { return isspace(c); }).base(), contentString.end());
    return contentString;
}

} // namespace libcellml
