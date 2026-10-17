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

#include "xmlutils.h"

#include "namespaces.h"
#include "utilities.h"
#include "xmldoc.h"

namespace libcellml {

/**
 * @brief Return a list of namespaces on attributes for this node.
 *
 * Scans all attributes of the node and records any associated
 * non-empty namespace attached to the attribute in an
 * @c NodeAttributeNamespaceInfo list.
 *
 * @param node The @c XmlNode to scan attributes of.
 * @return @c NodeAttributeNamespaceInfo of namespaces on attributes for the given @p node.
 */
NodeAttributeNamespaceInfo attributeNamespaces(const XmlNodePtr &node)
{
    NodeAttributeNamespaceInfo namespaceMap;
    auto tempAttribute = node->firstAttribute();
    while (tempAttribute != nullptr) {
        if (!tempAttribute->namespacePrefix().empty()) {
            namespaceMap.emplace_back(std::make_tuple(node->name(), tempAttribute->name(), tempAttribute->namespacePrefix(), tempAttribute->namespaceUri(), node->namespaceUri()));
        }
        tempAttribute = tempAttribute->next();
    }
    return namespaceMap;
}

XmlNamespaceMap determineMissingNamespaces(const XmlNamespaceMap &namespaceMap1, const XmlNamespaceMap &namespaceMap2)
{
    XmlNamespaceMap undefinedNamespaces;
    XmlNamespaceMap::const_iterator it;
    for (it = namespaceMap1.begin(); it != namespaceMap1.end(); ++it) {
        auto result = namespaceMap2.find(it->first);
        if (result == namespaceMap2.end()) {
            undefinedNamespaces.emplace(it->first, it->second);
        }
    }
    return undefinedNamespaces;
}

XmlNamespaceMap traverseTreeForElementNamespaces(const XmlNodePtr &node)
{
    XmlNamespaceMap nodeNamespaceMap;
    auto tempNode = node;
    while (tempNode != nullptr) {
        if (!tempNode->isText() && !tempNode->isComment()) {
            nodeNamespaceMap.emplace(tempNode->name(), tempNode->namespaceUri());
        }

        auto subNodeNamespaceMap = traverseTreeForElementNamespaces(tempNode->firstChild());

        nodeNamespaceMap.insert(subNodeNamespaceMap.begin(), subNodeNamespaceMap.end());

        tempNode = tempNode->next();
    }

    return nodeNamespaceMap;
}

NodeAttributeNamespaceInfo traverseTreeForAttributeNamespaces(const XmlNodePtr &node)
{
    NodeAttributeNamespaceInfo nodeAttributeNamespaceInfo;
    auto tempNode = node;
    while (tempNode != nullptr) {
        auto attributeNamespaceList = attributeNamespaces(tempNode);
        nodeAttributeNamespaceInfo.insert(nodeAttributeNamespaceInfo.end(), attributeNamespaceList.begin(), attributeNamespaceList.end());

        auto subNodeAttributeNamespaceInfo = traverseTreeForAttributeNamespaces(tempNode->firstChild());

        nodeAttributeNamespaceInfo.insert(nodeAttributeNamespaceInfo.end(), subNodeAttributeNamespaceInfo.begin(), subNodeAttributeNamespaceInfo.end());

        tempNode = tempNode->next();
    }

    return nodeAttributeNamespaceInfo;
}

XmlNamespaceMap traverseTreeForUndefinedNamespaces(const XmlNodePtr &node)
{
    XmlNamespaceMap undefinedNamespaces;
    auto tempNode = node;
    while (tempNode != nullptr) {
        auto definedNamespaces = tempNode->definedNamespaces();
        auto usedNamespaces = attributeNamespaces(tempNode);
        XmlNamespaceMap usedNamespaceMap;
        for (const auto &entry : usedNamespaces) {
            usedNamespaceMap.emplace(std::get<2>(entry), std::get<3>(entry));
        }

        auto missingNamespaces = determineMissingNamespaces(usedNamespaceMap, definedNamespaces);

        // Update undefined namespaces with missing namespaces.
        missingNamespaces.insert(undefinedNamespaces.begin(), undefinedNamespaces.end());
        std::swap(undefinedNamespaces, missingNamespaces);

        auto subUndefineNamespaces = traverseTreeForUndefinedNamespaces(tempNode->firstChild());

        // Update undefined namespaces with undefined namespaces in children.
        subUndefineNamespaces.insert(undefinedNamespaces.begin(), undefinedNamespaces.end());
        std::swap(undefinedNamespaces, subUndefineNamespaces);

        tempNode = tempNode->next();
    }

    return undefinedNamespaces;
}

std::vector<XmlAttributePtr> attributesWithCellml1XNamespace(const XmlNodePtr &node)
{
    std::vector<XmlAttributePtr> attributes;

    auto tempNode = node;
    while (tempNode != nullptr) {
        auto tempAttribute = tempNode->firstAttribute();
        // Find attributes using old CellML namespace.
        while (tempAttribute != nullptr) {
            if (tempAttribute->namespaceUri() == CELLML_1_0_NS || tempAttribute->namespaceUri() == CELLML_1_1_NS) {
                attributes.push_back(tempAttribute);
            }
            tempAttribute = tempAttribute->next();
        }

        auto subAttributes = attributesWithCellml1XNamespace(tempNode->firstChild());

        // Append attributes found on child nodes.
        attributes.insert(attributes.end(), subAttributes.begin(), subAttributes.end());

        tempNode = tempNode->next();
    }

    return attributes;
}

void removeCellml1XNamespaces(const XmlNodePtr &node, bool childrenOnly)
{
    auto tempNode = node;
    while (tempNode != nullptr) {
        auto definedNamespaces = tempNode->definedNamespaces();
        XmlNamespaceMap::const_iterator it;
        for (it = definedNamespaces.begin(); it != definedNamespaces.end(); ++it) {
            if (it->second == CELLML_1_0_NS) {
                tempNode->removeNamespaceDefinition(CELLML_1_0_NS);
            } else if (it->second == CELLML_1_1_NS) {
                tempNode->removeNamespaceDefinition(CELLML_1_1_NS);
            }
        }

        removeCellml1XNamespaces(tempNode->firstChild());

        if (childrenOnly) {
            tempNode = nullptr;
        } else {
            tempNode = tempNode->next();
        }
    }
}

std::vector<XmlDocPtr> multiRootXml(const std::string &content)
{
    std::vector<XmlDocPtr> childDocs;

    // Wrap potentially multiple nodes in our own root node.
    auto wrappedContent = "<root>" + trimCopy(content) + "</root>";

    // Parse this new string as a document and turn the child nodes
    // into their own document.
    XmlDocPtr doc = std::make_shared<XmlDoc>();
    doc->parse(wrappedContent);
    XmlNodePtr rootNode = doc->rootNode();
    if (rootNode != nullptr) {
        XmlNodePtr child = rootNode->firstChild();
        while (child != nullptr) {
            if (child->isElement()) {
                auto childContent = child->convertToString();
                XmlDocPtr childDoc = std::make_shared<XmlDoc>();
                childDoc->parse(childContent);
                childDocs.push_back(childDoc);
            }
            child = child->next();
        }
    } else {
        XmlDocPtr originalContentDoc = std::make_shared<XmlDoc>();
        originalContentDoc->parse(content);
        childDocs.push_back(originalContentDoc);
    }

    return childDocs;
}

} // namespace libcellml
