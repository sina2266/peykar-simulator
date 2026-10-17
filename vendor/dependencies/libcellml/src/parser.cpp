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

#include "libcellml/parser.h"

#include <algorithm>
#include <string>
#include <vector>

#include "libcellml/component.h"
#include "libcellml/importsource.h"
#include "libcellml/issue.h"
#include "libcellml/model.h"
#include "libcellml/reset.h"
#include "libcellml/variable.h"

#include "anycellmlelement_p.h"
#include "issue_p.h"
#include "logger_p.h"
#include "namespaces.h"
#include "utilities.h"
#include "xmldoc.h"
#include "xmlutils.h"

namespace libcellml {

/**
 * @brief The Parser::ParserImpl class.
 *
 * The private implementation for the Parser class.
 */
class Parser::ParserImpl: public Logger::LoggerImpl
{
public:
    Parser *mParser = nullptr;
    bool mParsing1XVersion = false;
    bool mParsing20Version = true;

    /**
     * @brief Update the @p model with attributes parsed from a @c std::string.
     *
     * Update the @p model with attributes and entities parsed from
     * the @c std::string @p input. Any entities or attributes in @p model with names
     * matching those in @p input will be overwritten.
     *
     * @param model The @c ModelPtr to update.
     * @param input The string to parse and update the @p model with.
     */
    void loadModel(const ModelPtr &model, const std::string &input);

    /**
     * @brief Create and populate a new model from a @c std::string.
     *
     * Takes a @c std::string and attempts to parse it into CellML 2.0 data structures.
     * Returns @c nullptr if the @p input is not a @c std::string representation of
     * a CellML model.
     *
     * @param input The string to parse into a model.
     *
     * @return The new @c ModelPtr deserialised from the input string.
     */
    ModelPtr parseModel(const std::string &input);

    /**
     * @brief Update the @p component with attributes parsed from @p node.
     *
     * Update the @p component with attributes and entities parsed from
     * the XML @p node. Any entities or attributes in @p component with names
     * matching those in @p node will be overwritten.
     *
     * @param component The @c ComponentPtr to update.
     * @param node The @c XmlNodePtr to parse and update the @p component with.
     */
    void loadComponent(const ComponentPtr &component, const XmlNodePtr &node);

    /**
     * @brief Update the @p model with a connection parsed from @p node.
     *
     * Update the @p model with connection information parsed from
     * the XML @p node. Connection information from @p node will be added
     * to any variable equivalence relationships already existing in @p model.
     *
     * @param model The @c ModelPtr to update.
     * @param node The @c XmlNodePtr to parse and update the model with.
     * @param usedConnections A list of connections that have already been used.
     */
    void loadConnection(const ModelPtr &model, const XmlNodePtr &node, NamePairList &usedConnections);

    /**
     * @brief Update the @p model with an encapsulation parsed from @p node.
     *
     * Update the @p model with encapsulation information parsed from
     * the XML @p node. Encapsulation relationships from @p node will be added
     * to any encapsulations relationships already in @p model.
     *
     * @param model The @c ModelPtr to update.
     * @param node The @c XmlNodePtr to parse and update the model with.
     */
    void loadEncapsulation(const ModelPtr &model, const XmlNodePtr &node);

    /**
     * @brief Recursively update the @p model with the encapsulation parsed from the @p node.
     *
     * Recursively parse through the @p node to create the component hierarchy.  Returns the
     * root component of the hierarchy to the calling method.
     *
     * @param model The @c ModelPtr to update.
     * @param node The @c XmlNodePtr to parse and update the model with.
     * @param usedNames List of components already used in loading component hierarchy.
     *
     * @return A @c ComponentPtr which is the root of the component hierarchy.
     */
    ComponentPtr loadComponentRef(const ModelPtr &model, const XmlNodePtr &node, NameList &usedNames);

    /**
     * @brief Update the @p import source with attributes parsed from @p node and add any imported
     * components or units it to the @p model.
     *
     * Update the @p import source with attributes parsed from @p node and add any imported
     * components or units to the @p model. If any attributes exist in @p import source
     * with names matching those in @p node, they will be overwritten. Likewise,
     * any imported components or units of the same name already in @p model will
     * be overwritten by those parsed from @p node.
     *
     * @param importSource The @c ImportSourcePtr to update.
     * @param model The @c ModelPtr to add imported components/units to.
     * @param node The @c XmlNodePtr to parse and update the @p import source with.
     */
    void loadImport(ImportSourcePtr &importSource, const ModelPtr &model, const XmlNodePtr &node);

    /**
     * @brief Update the @p units with attributes parsed from @p node.
     *
     * Update the @p units by parsing the XML @p node.
     * Existing attributes in @p units with names
     * matching those in @p node will be overwritten.
     *
     * @param units The @c UnitsPtr to update.
     * @param node The @c XmlNodePtr to parse and update the @p units with.
     */
    void loadUnits(const UnitsPtr &units, const XmlNodePtr &node);

    /**
     * @brief Load any units defined in a component into the model.
     *
     * The method is for transforming CellML 1.0 and CellML 1.1 models into CellML 2.0.
     * It searches through the components in the model looking for units definitions and
     * adding them to the model.
     *
     * @param model The @c ModelPtr to add any units to.
     * @param node The @c XmlNodePtr to search for units children of components.
     */
    void loadUnitsFromComponent(const ModelPtr &model, const XmlNodePtr &node);

    /**
     * @brief Update the @p units with a unit parsed from @p node.
     *
     * Update the @p units with a unit parsed from the XML @p node.
     * If a unit with the same name exists in @p units, it will be
     * overwritten by the unit from @p node.
     *
     * @param units The @c UnitsPtr to update.
     * @param node The unit @c XmlNodePtr to parse and update the @p units with.
     */
    void loadUnit(const UnitsPtr &units, const XmlNodePtr &node);

    /**
     * @brief Update the @p variable with attributes parsed from @p node.
     *
     * Update the @p variable with attributes parsed from
     * the XML @p node. Existing attributes in @p variable with names
     * matching those in @p node will be overwritten.
     *
     * @param variable The @c VariablePtr to update.
     * @param node The @c XmlNodePtr to parse and update the @p variable with.
     */
    void loadVariable(const VariablePtr &variable, const XmlNodePtr &node);

    /**
     * @brief Update the @p reset with attributes parsed from the @p node.
     *
     * Update the @p reset with attributes parsed from
     * the XML @p node. Existing attributes in @p reset with names
     * matching those in @p node will be overwritten.
     *
     * @param reset The @c ResetPtr to update.
     * @param component The @c ComponentPtr the reset belongs to.
     * @param node The @c XmlNodePtr to parse and update the @p variable with.
     */
    void loadReset(const ResetPtr &reset, const ComponentPtr &component, const XmlNodePtr &node);

    /**
     * @brief Update the @p reset with the child parsed from the @p node.
     *
     * Takes either a 'test_value' node or a 'reset_value' node that is
     * a child of a @c Reset.  Updates the @p reset with the information in the
     * @p node.
     *
     * @param childType The @c std::string type of child which is either 'test_value' or 'reset_value'.
     * @param reset The @c ResetPtr to update.
     * @param component The @c ComponentPtr the reset belongs to.
     * @param node The @c XmlNodePtr to parse and update the @p variable with.
     */
    void loadResetChild(const std::string &childType, const ResetPtr &reset, const ComponentPtr &component, const XmlNodePtr &node);

    /**
     * @brief Checks the multiplicity of the @p childType.
     *
     * Checks to determine if the @p childType has the correct number of entries
     * for a @c Reset.  Adds an issue to the @c Parser if the @p count is not correct
     * for the @p childType.
     *
     * @param count The number of children of the type defined by @p childType.
     * @param childType The @c std::string type of child which is either 'test_value' or 'reset_value'.
     * @param reset The @c ResetPtr to update.
     * @param component The @c ComponentPtr the reset belongs to.
     */
    void checkResetChildMultiplicity(size_t count, const std::string &childType, const ResetPtr &reset, const ComponentPtr &component);

    /**
     * @brief Test to determine if the node should be parsed.
     *
     * Tests whether the given @p node should be parsed if it matches
     * the given @p name.  The test ensures that the namespace of the
     * given @p node matches the namespace of the originating model element.
     *
     * @param node The node to test.
     * @param name The name to match to the test node name.
     *
     * @return @c true if the node should be parsed, @c false otherwise.
     */
    bool parseNode(const XmlNodePtr &node, const char *name);
};

Parser::ParserImpl *Parser::pFunc()
{
    return reinterpret_cast<Parser::ParserImpl *>(Logger::pFunc());
}

Parser::Parser()
    : Logger(new ParserImpl())
{
    pFunc()->mParser = this;
    setStrict(true);
}

Parser::~Parser()
{
    delete pFunc();
}

ParserPtr Parser::create(bool strict) noexcept
{
    auto parser = std::shared_ptr<Parser> {new Parser {}};
    parser->setStrict(strict);
    return parser;
}

ModelPtr Parser::parseModel(const std::string &input)
{
    return pFunc()->parseModel(input);
}

ModelPtr Parser::ParserImpl::parseModel(const std::string &input)
{
    removeAllIssues();
    ModelPtr model = nullptr;
    if (input.empty()) {
        auto issue = Issue::IssueImpl::create();
        issue->mPimpl->setDescription("Model string is empty.");
        issue->mPimpl->setReferenceRule(Issue::ReferenceRule::XML);
        addIssue(issue);
    } else {
        model = Model::create();
        loadModel(model, input);
    }
    return model;
}

/**
 * @brief Test to determine if the attribute is an XML identifier.
 *
 * Test to determine if the attribute is an XML identifier. When
 * transforming from CellML 1.0 and CellML 1.1 it will also test for
 * XML identifiers in the CMETA 1.0 namespace.
 *
 * @param attribute The attribute to test.
 * @param transforming @c true if transfomring from CellML 1.0 or CellML 1.1, @c false otherwise.
 *
 * @return @c true if the given attribute is an XML identifier attribute, @c false otherwise.
 */
bool isIdAttribute(const XmlAttributePtr &attribute, bool transforming)
{
    return attribute->isType("id") || (transforming && attribute->isType("id", CMETA_1_0_NS));
}

/**
 * @brief Convert non-SI units to SI units.
 *
 * Converts:
 *  * liter -> litre
 *  * meter -> metre
 *
 * @param unitsName Name of the units to convert.
 *
 * @return The SI units name.
 */
std::string convertNonSiUnits(const std::string &unitsName)
{
    if (unitsName == "liter") {
        return "litre";
    }

    if (unitsName == "meter") {
        return "metre";
    }

    return unitsName;
}

/**
 * @brief Test to determine if the @p node describes an encapsulation relationship.
 *
 * Test to determine if the @p node describes an encapsulation relationship.
 *
 * @param node The XML node to test.
 *
 * @return @c true if the @p node has a child node with a relationship that is an encapsulation, @c false otherwise.
 */
bool isEncapsulationRelationship(const XmlNodePtr &node)
{
    XmlNodePtr childNode = node->firstChild();
    while (childNode != nullptr) {
        if (childNode->isCellml1XElement("relationship_ref")) {
            XmlAttributePtr attribute = childNode->firstAttribute();
            while (attribute != nullptr) {
                if (attribute->isType("relationship") && (attribute->value() == "encapsulation")) {
                    return true;
                }
                attribute = attribute->next();
            }
        }
        childNode = childNode->next();
    }

    return false;
}

/**
 * @brief Get the CellML version of the given node.
 *
 * Get the CellML version of the node.  Only returns
 * the version for CellML 1.0 or 1.1 nodes.
 *
 * @param node The node to return the CellML version for.
 *
 * @return A @c std::string version of the given node.
 */
std::string nodesCellMl1XVersion(const XmlNodePtr &node)
{
    if (node->isCellml10Element()) {
        return "1.0";
    }

    return "1.1";
}

void Parser::ParserImpl::loadModel(const ModelPtr &model, const std::string &input)
{
    XmlDocPtr doc = std::make_shared<XmlDoc>();
    doc->parse(input);
    // Copy any XML parsing issues into the common parser issue handler.
    if (doc->xmlErrorCount() > 0) {
        for (size_t i = 0; i < doc->xmlErrorCount(); ++i) {
            auto issue = Issue::IssueImpl::create();
            issue->mPimpl->setDescription("LibXml2 error: " + doc->xmlError(i));
            issue->mPimpl->setReferenceRule(Issue::ReferenceRule::XML);
            addIssue(issue);
        }
    }
    const XmlNodePtr node = doc->rootNode();
    if (!node) {
        auto issue = Issue::IssueImpl::create();
        issue->mPimpl->setDescription("Could not get a valid XML root node from the provided input.");
        if (mParser->isStrict()) {
            issue->mPimpl->setReferenceRule(Issue::ReferenceRule::XML);
        }
        addIssue(issue);
        return;
    }

    mParsing20Version = node->isCellml20Element("model");

    auto elementNamespaceMap = traverseTreeForElementNamespaces(node);
    if (mParsing20Version) {
        for (const auto &e : elementNamespaceMap) {
            std::string name = e.first;
            std::string uri = e.second;
            if ((uri != CELLML_2_0_NS) && (uri != MATHML_NS)) {
                auto issue = Issue::IssueImpl::create();
                issue->mPimpl->setDescription("Element '" + name + "' uses namespace '" + uri + "' which does not belong to an allowed namespace. ");
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::XML_UNEXPECTED_NAMESPACE);
                addIssue(issue);
            }
        }
    }

    auto attributeNamespaceMap = traverseTreeForAttributeNamespaces(node);
    if (mParsing20Version) {
        for (const auto &e : attributeNamespaceMap) {
            std::string nodeName = std::get<0>(e);
            std::string nodeUri = std::get<4>(e);
            std::string attributeName = std::get<1>(e);
            std::string uri = std::get<3>(e);
            if ((nodeName == "cn") && (nodeUri == MATHML_NS) && (attributeName == "units") && (uri == CELLML_2_0_NS)) {
                // Explicitly allowed attribute namespace prefix.
            } else if ((nodeName == "import") && (nodeUri == CELLML_2_0_NS) && (attributeName == "href") && (uri == XLINK_NS)) {
                // Explicitly allowed attribute namespace prefix.
            } else {
                auto issue = Issue::IssueImpl::create();
                issue->mPimpl->setDescription("Element '" + nodeName + "' attribute '" + attributeName + "' has a namespace '" + uri + "' specified.");
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::XML_ATTRIBUTE_HAS_NAMESPACE);
                addIssue(issue);
            }
        }
    }

    if ((mParser->isStrict() && !mParsing20Version) || !node->isCellmlElement("model")) {
        auto issue = Issue::IssueImpl::create();
        if (node->name() == "model") {
            std::string nodeNamespace = node->namespaceUri();
            if (nodeNamespace.empty()) {
                nodeNamespace = "null";
            }
            if (mParser->isStrict() && node->isCellml1XElement("model")) {
                issue->mPimpl->setDescription("Given model is a CellML " + nodesCellMl1XVersion(node) + " model but strict parsing mode is on.");
            } else {
                std::string message = "Model element is in an invalid namespace '" + nodeNamespace + "'.";
                if (mParser->isStrict()) {
                    message += " A valid CellML root node should be in the namespace '" + std::string(CELLML_2_0_NS) + "'.";
                }
                issue->mPimpl->setDescription(message);
            }
        } else {
            issue->mPimpl->setDescription("Model element is of invalid type '" + node->name() + "'. A valid CellML root node should be of type 'model'.");
        }
        issue->mPimpl->mItem->mPimpl->setModel(model);
        if (mParser->isStrict()) {
            issue->mPimpl->setReferenceRule(Issue::ReferenceRule::XML_UNEXPECTED_ELEMENT);
        }
        addIssue(issue);
        return;
    }
    mParsing1XVersion = node->isCellml1XElement("model");
    if (mParsing1XVersion) {
        auto issue = Issue::IssueImpl::create();
        issue->mPimpl->setDescription("Given model is a CellML " + nodesCellMl1XVersion(node) + " model, the parser will try to represent this model in CellML 2.0.");
        issue->mPimpl->setLevel(Issue::Level::MESSAGE);
        issue->mPimpl->mItem->mPimpl->setModel(model);
        addIssue(issue);
    }
    // Get model attributes.
    bool nameAttributePresent = false;
    XmlAttributePtr attribute = node->firstAttribute();
    while (attribute != nullptr) {
        if (attribute->isType("name")) {
            model->setName(attribute->value());
            nameAttributePresent = true;
        } else if (isIdAttribute(attribute, mParsing1XVersion)) {
            model->setId(attribute->value());
        } else {
            auto issue = Issue::IssueImpl::create();
            if (mParsing1XVersion) {
                issue->mPimpl->setDescription("Model '" + node->attribute("name") + "' ignoring attribute '" + attribute->name() + "'.");
                issue->mPimpl->setLevel(Issue::Level::MESSAGE);
            } else {
                issue->mPimpl->setDescription("Model '" + node->attribute("name") + "' has an invalid attribute '" + attribute->name() + "'.");
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::MODEL_ELEMENT);
            }
            issue->mPimpl->mItem->mPimpl->setModel(model);
            addIssue(issue);
        }
        attribute = attribute->next();
    }

    if (!mParsing1XVersion && !nameAttributePresent) {
        auto issue = Issue::IssueImpl::create();
        issue->mPimpl->setDescription("Model does not have a name attribute.");
        issue->mPimpl->setReferenceRule(Issue::ReferenceRule::MODEL_NAME);
        issue->mPimpl->mItem->mPimpl->setModel(model);
        addIssue(issue);
    }

    // Get model children (CellML entities).
    XmlNodePtr childNode = node->firstChild();
    std::vector<XmlNodePtr> connectionNodes;
    std::vector<XmlNodePtr> encapsulationNodes;
    while (childNode != nullptr) {
        if (parseNode(childNode, "component")) {
            auto component = Component::create();
            loadComponent(component, childNode);
            model->addComponent(component);
            if (mParsing1XVersion) {
                loadUnitsFromComponent(model, childNode);
            }
        } else if (parseNode(childNode, "units")) {
            UnitsPtr units = Units::create();
            loadUnits(units, childNode);
            model->addUnits(units);
        } else if (parseNode(childNode, "import")) {
            ImportSourcePtr importSource = ImportSource::create();
            loadImport(importSource, model, childNode);
        } else if (childNode->isCellml20Element("encapsulation")) {
            // An encapsulation should not have attributes other than an 'id' attribute.
            if (childNode->firstAttribute()) {
                XmlAttributePtr childAttribute = childNode->firstAttribute();
                while (childAttribute) {
                    if (isIdAttribute(childAttribute, false)) {
                        model->setEncapsulationId(childAttribute->value());
                    } else {
                        auto issue = Issue::IssueImpl::create();
                        issue->mPimpl->setDescription("Encapsulation in model '" + model->name() + "' has an invalid attribute '" + childAttribute->name() + "'.");
                        issue->mPimpl->mItem->mPimpl->setEncapsulation(model);
                        issue->mPimpl->setReferenceRule(Issue::ReferenceRule::ENCAPSULATION_ELEMENT);
                        addIssue(issue);
                    }
                    childAttribute = childAttribute->next();
                }
            }
            // Load encapsulated component_refs.
            XmlNodePtr componentRefNode = childNode->firstChild();
            if (componentRefNode != nullptr) {
                // This component_ref and its child and sibling elements will be loaded
                // and issue-checked in loadEncapsulation().
                encapsulationNodes.push_back(childNode);
            } else {
                // Empty encapsulations are valid, but may not be intended.
                auto issue = Issue::IssueImpl::create();
                issue->mPimpl->setDescription("Encapsulation in model '" + model->name() + "' does not contain any child elements.");
                issue->mPimpl->mItem->mPimpl->setEncapsulation(model);
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::ENCAPSULATION_CHILD);
                issue->mPimpl->setLevel(libcellml::Issue::Level::WARNING);
                addIssue(issue);
            }
        } else if (childNode->isCellml20Element("connection")) {
            connectionNodes.push_back(childNode);
        } else if (childNode->isText()) {
            std::string textNode = childNode->convertToString();
            // Ignore whitespace when parsing.
            if (hasNonWhitespaceCharacters(textNode)) {
                auto issue = Issue::IssueImpl::create();
                issue->mPimpl->setDescription("Model '" + model->name() + "' has an invalid non-whitespace child text element '" + textNode + "'.");
                issue->mPimpl->mItem->mPimpl->setModel(model);
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::XML_UNEXPECTED_CHARACTER);
                addIssue(issue);
            }
        } else if (mParsing1XVersion && childNode->isCellml1XElement("group")) {
            if (isEncapsulationRelationship(childNode)) {
                encapsulationNodes.push_back(childNode);
            }
        } else if (mParsing1XVersion && childNode->isCellml1XElement("connection")) {
            connectionNodes.push_back(childNode);
        } else if (childNode->isComment()) {
            // Do nothing.
        } else {
            auto issue = Issue::IssueImpl::create();
            if (mParsing1XVersion) {
                issue->mPimpl->setDescription("Model '" + model->name() + "' ignoring child element '" + childNode->name() + "'.");
                issue->mPimpl->setLevel(Issue::Level::MESSAGE);
            } else {
                issue->mPimpl->setDescription("Model '" + model->name() + "' has an invalid child element '" + childNode->name() + "'.");
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::XML_UNEXPECTED_ELEMENT);
            }
            issue->mPimpl->mItem->mPimpl->setModel(model);
            addIssue(issue);
        }
        childNode = childNode->next();
    }

    if (!encapsulationNodes.empty()) {
        loadEncapsulation(model, encapsulationNodes.at(0));
        if (encapsulationNodes.size() > 1) {
            auto issue = Issue::IssueImpl::create();
            issue->mPimpl->setDescription("Model '" + model->name() + "' has more than one encapsulation element.");
            issue->mPimpl->mItem->mPimpl->setEncapsulation(model);
            issue->mPimpl->setReferenceRule(Issue::ReferenceRule::MODEL_MORE_THAN_ONE_ENCAPSULATION);
            addIssue(issue);
        }
    }

    NamePairList usedConnections;
    for (const auto &connectionNode : connectionNodes) {
        loadConnection(model, connectionNode, usedConnections);
    }

    // Link units to their names.
    DescriptionList issueList;
    traverseComponentEntityTreeLinkingUnits(model, issueList);
    for (const auto &entry : issueList) {
        auto issue = Issue::IssueImpl::create();
        issue->mPimpl->setDescription(entry.second);
        issue->mPimpl->setLevel(Issue::Level::WARNING);
        issue->mPimpl->setReferenceRule(Issue::ReferenceRule::VARIABLE_ELEMENT);
        issue->mPimpl->mItem->mPimpl->setVariable(entry.first);
        addIssue(issue);
    }
}

void Parser::ParserImpl::loadComponent(const ComponentPtr &component, const XmlNodePtr &node)
{
    XmlAttributePtr attribute = node->firstAttribute();
    bool nameAttributePresent = false;
    while (attribute != nullptr) {
        if (attribute->isType("name")) {
            component->setName(attribute->value());
            nameAttributePresent = true;
        } else if (isIdAttribute(attribute, mParsing1XVersion)) {
            component->setId(attribute->value());
        } else {
            auto issue = Issue::IssueImpl::create();
            if (mParsing1XVersion) {
                issue->mPimpl->setDescription("Component '" + node->attribute("name") + "' ignoring attribute '" + attribute->name() + "'.");
                issue->mPimpl->setLevel(Issue::Level::MESSAGE);
            } else {
                issue->mPimpl->setDescription("Component '" + node->attribute("name") + "' has an invalid attribute '" + attribute->name() + "'.");
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::COMPONENT_ELEMENT);
            }
            issue->mPimpl->mItem->mPimpl->setComponent(component);
            addIssue(issue);
        }
        attribute = attribute->next();
    }

    if (!nameAttributePresent) {
        auto issue = Issue::IssueImpl::create();
        issue->mPimpl->setDescription("Component does not specify a name attribute.");
        issue->mPimpl->mItem->mPimpl->setComponent(component);
        issue->mPimpl->setReferenceRule(Issue::ReferenceRule::COMPONENT_NAME);
        addIssue(issue);
    }

    XmlNodePtr childNode = node->firstChild();
    while (childNode != nullptr) {
        if (childNode->isCellmlElement("variable")) {
            VariablePtr variable = Variable::create();
            loadVariable(variable, childNode);
            component->addVariable(variable);
        } else if (childNode->isCellml20Element("reset")) {
            ResetPtr reset = Reset::create();
            loadReset(reset, component, childNode);
            component->addReset(reset);
        } else if (childNode->isMathmlElement("math")) {
            // If transforming, manipulate the math sub-document CellML namespaces.
            if (mParsing1XVersion) {
                // Find all attributes using old CellML namespace.
                auto cellmlAttributes = attributesWithCellml1XNamespace(childNode->firstChild());

                // Remove all old CellML namespace definitions and references.
                removeCellml1XNamespaces(childNode, true);

                if (!cellmlAttributes.empty()) {
                    // Add CellML 2.0 namespace to MathML element.
                    childNode->addNamespaceDefinition(CELLML_2_0_NS, "cellml");

                    // Set all attributes that had an old CellML namespace with CellML 2.0 namespace.
                    for (const auto &cellmlAttribute : cellmlAttributes) {
                        cellmlAttribute->setNamespacePrefix("cellml");
                    }
                }
            }
            // Copy any namespaces that do not feature as a namespace definition
            // of the math node into the math node.
            auto mathElementDefinedNamespaces = childNode->definedNamespaces();
            auto possiblyUndefinedNamespaces = traverseTreeForUndefinedNamespaces(childNode->firstChild());
            auto undefinedNamespaces = determineMissingNamespaces(possiblyUndefinedNamespaces, mathElementDefinedNamespaces);
            XmlNamespaceMap::const_iterator it;
            for (it = undefinedNamespaces.begin(); it != undefinedNamespaces.end(); ++it) {
                childNode->addNamespaceDefinition(it->second, it->first);
            }

            // Append a self contained math XML document to the component.
            std::string math = childNode->convertToString() + "\n";
            component->appendMath(math);
        } else if (childNode->isText()) {
            std::string textNode = childNode->convertToString();
            // Ignore whitespace when parsing.
            if (hasNonWhitespaceCharacters(textNode)) {
                auto issue = Issue::IssueImpl::create();
                issue->mPimpl->setDescription("Component '" + component->name() + "' has an invalid non-whitespace child text element '" + textNode + "'.");
                issue->mPimpl->mItem->mPimpl->setComponent(component);
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::XML_UNEXPECTED_CHARACTER);
                addIssue(issue);
            }
        } else if (childNode->isComment()) {
            // Do nothing.
        } else if (mParsing1XVersion && (childNode->name() == "units")) {
            // Do nothing.
        } else {
            auto issue = Issue::IssueImpl::create();
            if (mParsing1XVersion) {
                issue->mPimpl->setDescription("Component '" + component->name() + "' ignoring child element '" + childNode->name() + "'.");
                issue->mPimpl->setLevel(Issue::Level::MESSAGE);
            } else {
                issue->mPimpl->setDescription("Component '" + component->name() + "' has an invalid child element '" + childNode->name() + "'.");
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::XML_UNEXPECTED_ELEMENT);
            }
            issue->mPimpl->mItem->mPimpl->setComponent(component);
            addIssue(issue);
        }
        childNode = childNode->next();
    }
}

void Parser::ParserImpl::loadUnitsFromComponent(const ModelPtr &model, const XmlNodePtr &node)
{
    XmlNodePtr childNode = node->firstChild();
    while (childNode != nullptr) {
        if (childNode->isCellml1XElement("units")) {
            UnitsPtr units = Units::create();
            loadUnits(units, childNode);
            model->addUnits(units);
        }
        childNode = childNode->next();
    }
}

void Parser::ParserImpl::loadUnits(const UnitsPtr &units, const XmlNodePtr &node)
{
    XmlAttributePtr attribute = node->firstAttribute();
    bool nameAttributePresent = false;
    while (attribute != nullptr) {
        if (attribute->isType("name")) {
            units->setName(attribute->value());
            nameAttributePresent = true;
        } else if (isIdAttribute(attribute, mParsing1XVersion)) {
            units->setId(attribute->value());
        } else {
            auto issue = Issue::IssueImpl::create();
            if (mParsing1XVersion) {
                issue->mPimpl->setDescription("Units '" + units->name() + "' ignoring attribute '" + attribute->name() + "'.");
                issue->mPimpl->setLevel(Issue::Level::MESSAGE);
            } else {
                issue->mPimpl->setDescription("Units '" + units->name() + "' has an invalid attribute '" + attribute->name() + "'.");
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::UNITS_ELEMENT);
            }
            issue->mPimpl->mItem->mPimpl->setUnits(units);
            addIssue(issue);
        }
        attribute = attribute->next();
    }

    if (!nameAttributePresent) {
        auto issue = Issue::IssueImpl::create();
        issue->mPimpl->setDescription("Units does not specify a name attribute.");
        issue->mPimpl->mItem->mPimpl->setUnits(units);
        issue->mPimpl->setReferenceRule(Issue::ReferenceRule::UNITS_NAME);
        addIssue(issue);
    }

    XmlNodePtr childNode = node->firstChild();
    while (childNode != nullptr) {
        if (parseNode(childNode, "unit")) {
            loadUnit(units, childNode);
        } else if (childNode->isText()) {
            std::string textNode = childNode->convertToString();
            // Ignore whitespace when parsing.
            if (hasNonWhitespaceCharacters(textNode)) {
                auto issue = Issue::IssueImpl::create();
                issue->mPimpl->setDescription("Units '" + units->name() + "' has an invalid non-whitespace child text element '" + textNode + "'.");
                issue->mPimpl->mItem->mPimpl->setUnits(units);
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::XML_UNEXPECTED_CHARACTER);
                addIssue(issue);
            }
        } else if (childNode->isComment()) {
            // Do nothing.
        } else {
            auto issue = Issue::IssueImpl::create();
            issue->mPimpl->setDescription("Units '" + units->name() + "' has an invalid child element '" + childNode->name() + "'.");
            issue->mPimpl->mItem->mPimpl->setUnits(units);
            issue->mPimpl->setReferenceRule(Issue::ReferenceRule::XML_UNEXPECTED_ELEMENT);
            addIssue(issue);
        }
        childNode = childNode->next();
    }
}

void Parser::ParserImpl::loadUnit(const UnitsPtr &units, const XmlNodePtr &node)
{
    std::string reference;
    std::string prefix = "0";
    double exponent = 1.0;
    double multiplier = 1.0;
    std::string id;
    // A unit should not have any children.
    XmlNodePtr childNode = node->firstChild();
    while (childNode != nullptr) {
        if (childNode->isText()) {
            std::string textNode = childNode->convertToString();
            // Ignore whitespace when parsing.
            if (hasNonWhitespaceCharacters(textNode)) {
                auto issue = Issue::IssueImpl::create();
                issue->mPimpl->setDescription("Unit referencing '" + node->attribute("units") + "' in units '" + units->name() + "' has an invalid non-whitespace child text element '" + textNode + "'.");
                issue->mPimpl->mItem->mPimpl->setUnits(units);
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::XML_UNEXPECTED_CHARACTER);
                addIssue(issue);
            }
        } else if (childNode->isComment()) {
            // Do nothing.
        } else {
            auto issue = Issue::IssueImpl::create();
            issue->mPimpl->setDescription("Unit referencing '" + node->attribute("units") + "' in units '" + units->name() + "' has an invalid child element '" + childNode->name() + "'.");
            issue->mPimpl->mItem->mPimpl->setUnits(units);
            issue->mPimpl->setReferenceRule(Issue::ReferenceRule::XML_UNEXPECTED_ELEMENT);
            addIssue(issue);
        }
        childNode = childNode->next();
    }
    // Parse the unit attributes.
    XmlAttributePtr attribute = node->firstAttribute();
    bool unitsAttributePresent = false;
    while (attribute != nullptr) {
        if (attribute->isType("units")) {
            if (mParsing1XVersion) {
                reference = convertNonSiUnits(attribute->value());
            } else {
                reference = attribute->value();
            }
            unitsAttributePresent = true;
        } else if (attribute->isType("prefix")) {
            prefix = attribute->value();
        } else if (attribute->isType("exponent")) {
            if (isCellMLReal(attribute->value())) {
                if (!convertToDouble(attribute->value(), exponent)) {
                    // This value won't be saved for validation later, so it does need to be reported now.
                    auto issue = Issue::IssueImpl::create();
                    issue->mPimpl->setDescription("Unit referencing '" + node->attribute("units") + "' in units '" + units->name() + "' has an exponent with the value '" + attribute->value() + "' that is a representation of a CellML real valued number, but out of range of the 'double' type.");
                    issue->mPimpl->mItem->mPimpl->setUnits(units);
                    issue->mPimpl->setReferenceRule(Issue::ReferenceRule::UNIT_ATTRIBUTE_EXPONENT_VALUE);
                    addIssue(issue);
                }
            } else {
                // This value won't be saved for validation later, so it does need to be reported now.
                auto issue = Issue::IssueImpl::create();
                issue->mPimpl->setDescription("Unit referencing '" + node->attribute("units") + "' in units '" + units->name() + "' has an exponent with the value '" + attribute->value() + "' that is not a representation of a CellML real valued number.");
                issue->mPimpl->mItem->mPimpl->setUnits(units);
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::UNIT_ATTRIBUTE_EXPONENT_VALUE);
                addIssue(issue);
            }
        } else if (attribute->isType("multiplier")) {
            if (isCellMLReal(attribute->value())) {
                if (!convertToDouble(attribute->value(), multiplier)) {
                    // This value won't be saved for validation later, so it does need to be reported now.
                    auto issue = Issue::IssueImpl::create();
                    issue->mPimpl->setDescription("Unit referencing '" + node->attribute("units") + "' in units '" + units->name() + "' has a multiplier with the value '" + attribute->value() + "' that is a representation of a CellML real valued number, but out of range of the 'double' type.");
                    issue->mPimpl->mItem->mPimpl->setUnits(units);
                    issue->mPimpl->setReferenceRule(Issue::ReferenceRule::UNIT_ATTRIBUTE_MULTIPLIER_VALUE);
                    addIssue(issue);
                }
            } else {
                // This value won't be saved for validation later, so it does need to be reported now.
                auto issue = Issue::IssueImpl::create();
                issue->mPimpl->setDescription("Unit referencing '" + node->attribute("units") + "' in units '" + units->name() + "' has a multiplier with the value '" + attribute->value() + "' that is not a representation of a CellML real valued number.");
                issue->mPimpl->mItem->mPimpl->setUnits(units);
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::UNIT_ATTRIBUTE_MULTIPLIER_VALUE);
                addIssue(issue);
            }
        } else if (isIdAttribute(attribute, mParsing1XVersion)) {
            id = attribute->value();
        } else {
            auto issue = Issue::IssueImpl::create();
            issue->mPimpl->setDescription("Unit referencing '" + node->attribute("units") + "' in units '" + units->name() + "' has an invalid attribute '" + attribute->name() + "'.");
            issue->mPimpl->mItem->mPimpl->setUnits(units);
            issue->mPimpl->setReferenceRule(Issue::ReferenceRule::UNIT_ATTRIBUTE_OPTIONAL);
            addIssue(issue);
        }
        attribute = attribute->next();
    }

    if (!unitsAttributePresent) {
        auto issue = Issue::IssueImpl::create();
        issue->mPimpl->setDescription("Unit does not specify a units attribute.");
        issue->mPimpl->mItem->mPimpl->setUnits(units);
        issue->mPimpl->setReferenceRule(Issue::ReferenceRule::UNIT_UNITS);
        addIssue(issue);
    }

    // Add this unit to the parent units.
    units->addUnit(reference, prefix, exponent, multiplier, id);
}

void Parser::ParserImpl::loadVariable(const VariablePtr &variable, const XmlNodePtr &node)
{
    // A variable should not have any children.
    XmlNodePtr childNode = node->firstChild();
    while (childNode != nullptr) {
        if (childNode->isText()) {
            std::string textNode = childNode->convertToString();
            // Ignore whitespace when parsing.
            if (hasNonWhitespaceCharacters(textNode)) {
                auto issue = Issue::IssueImpl::create();
                issue->mPimpl->setDescription("Variable '" + node->attribute("name") + "' has an invalid non-whitespace child text element '" + textNode + "'.");
                issue->mPimpl->mItem->mPimpl->setVariable(variable);
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::XML_UNEXPECTED_CHARACTER);
                addIssue(issue);
            }
        } else if (childNode->isComment()) {
            // Do nothing.
        } else {
            auto issue = Issue::IssueImpl::create();
            if (mParsing1XVersion) {
                issue->mPimpl->setDescription("Variable '" + node->attribute("name") + "' ignoring child element '" + childNode->name() + "'.");
                issue->mPimpl->setLevel(Issue::Level::MESSAGE);
            } else {
                issue->mPimpl->setDescription("Variable '" + node->attribute("name") + "' has an invalid child element '" + childNode->name() + "'.");
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::XML_UNEXPECTED_ELEMENT);
            }
            issue->mPimpl->mItem->mPimpl->setVariable(variable);
            addIssue(issue);
        }
        childNode = childNode->next();
    }
    XmlAttributePtr attribute = node->firstAttribute();
    bool nameAttributePresent = false;
    bool unitsAttributePresent = false;
    while (attribute != nullptr) {
        if (attribute->isType("name")) {
            variable->setName(attribute->value());
            nameAttributePresent = true;
        } else if (isIdAttribute(attribute, mParsing1XVersion)) {
            variable->setId(attribute->value());
        } else if (attribute->isType("units")) {
            if (mParsing1XVersion) {
                variable->setUnits(convertNonSiUnits(attribute->value()));
            } else {
                variable->setUnits(attribute->value());
            }
            unitsAttributePresent = true;
        } else if (attribute->isType("interface")) {
            variable->setInterfaceType(attribute->value());
        } else if (attribute->isType("initial_value")) {
            variable->setInitialValue(attribute->value());
        } else if (mParsing1XVersion && attribute->isType("public_interface")) {
            if (variable->hasInterfaceType(Variable::InterfaceType::PRIVATE)) {
                variable->setInterfaceType(Variable::InterfaceType::PUBLIC_AND_PRIVATE);
            } else {
                variable->setInterfaceType(Variable::InterfaceType::PUBLIC);
            }
        } else if (mParsing1XVersion && attribute->isType("private_interface")) {
            if (variable->hasInterfaceType(Variable::InterfaceType::PUBLIC)) {
                variable->setInterfaceType(Variable::InterfaceType::PUBLIC_AND_PRIVATE);
            } else {
                variable->setInterfaceType(Variable::InterfaceType::PRIVATE);
            }
        } else {
            auto issue = Issue::IssueImpl::create();
            if (mParsing1XVersion) {
                issue->mPimpl->setDescription("Variable '" + node->attribute("name") + "' ignoring attribute '" + attribute->name() + "'.");
                issue->mPimpl->setLevel(Issue::Level::MESSAGE);
            } else {
                issue->mPimpl->setDescription("Variable '" + node->attribute("name") + "' has an invalid attribute '" + attribute->name() + "'.");
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::VARIABLE_ATTRIBUTE_OPTIONAL);
            }
            issue->mPimpl->mItem->mPimpl->setVariable(variable);
            addIssue(issue);
        }
        attribute = attribute->next();
    }

    if (!nameAttributePresent || !unitsAttributePresent) {
        auto issue = Issue::IssueImpl::create();
        std::string description = "Variable ";
        if (nameAttributePresent) {
            description += "'" + node->attribute("name") + "' does not specify a units attribute.";
        } else if (unitsAttributePresent) {
            description += "with units '" + node->attribute("units") + "' does not specify a name attribute.";
        } else {
            description += "does not specify a name attribute or a units attribute.";
        }

        issue->mPimpl->setDescription(description);
        issue->mPimpl->setReferenceRule(Issue::ReferenceRule::VARIABLE_ATTRIBUTE_REQUIRED);
        issue->mPimpl->mItem->mPimpl->setVariable(variable);
        addIssue(issue);
    }
}

void Parser::ParserImpl::loadConnection(const ModelPtr &model, const XmlNodePtr &node, NamePairList &usedConnections)
{
    // Define types for variable and component pairs, and their identifiers.
    using NameInfo = std::vector<std::string>;
    using NameInfoMap = std::vector<NameInfo>;

    // Initialise name pairs and flags.
    NamePair componentNamePair;
    NameInfo variableNameInfo;
    NameInfoMap variableNameMap;
    bool mapVariablesFound = false;
    bool component1Missing = false;
    bool component2Missing = false;
    bool variable1Missing = false;
    bool variable2Missing = false;

    // Check connection for component_{1, 2} attributes and get the name pair.
    std::string component1Name;
    std::string component2Name;
    std::string mappingId;
    std::string connectionId;

    XmlNodePtr componentNode = nullptr;
    if (mParsing1XVersion) {
        XmlNodePtr childNode = node->firstChild();
        while ((childNode != nullptr) && (componentNode == nullptr)) {
            if (childNode->isCellml1XElement("map_components")) {
                componentNode = childNode;
            }
            childNode = childNode->next();
        }
        if (componentNode == nullptr) {
            auto issue = Issue::IssueImpl::create();
            issue->mPimpl->setDescription("Connection definition in model '" + model->name() + "' does not have a 'map_components' element.");
            issue->mPimpl->mItem->mPimpl->setModel(model);
            addIssue(issue);
            return;
        }
    } else {
        componentNode = node;
    }

    bool component1Attribute = false;
    bool component2Attribute = false;
    XmlAttributePtr attribute = componentNode->firstAttribute();
    while (attribute != nullptr) {
        if (attribute->isType("component_1")) {
            component1Name = attribute->value();
            component1Attribute = true;
        } else if (attribute->isType("component_2")) {
            component2Name = attribute->value();
            component2Attribute = true;
        } else if (isIdAttribute(attribute, mParsing1XVersion)) {
            connectionId = attribute->value();
        } else {
            auto issue = Issue::IssueImpl::create();
            if (mParsing1XVersion) {
                issue->mPimpl->setDescription("Connection definition in model '" + model->name() + "' ignoring attribute '" + attribute->name() + "'.");
                issue->mPimpl->setLevel(Issue::Level::MESSAGE);
            } else {
                issue->mPimpl->setDescription("Connection in model '" + model->name() + "' has an invalid connection attribute '" + attribute->name() + "'.");
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::CONNECTION_ELEMENT);
            }
            issue->mPimpl->mItem->mPimpl->setModel(model);
            addIssue(issue);
        }
        attribute = attribute->next();
    }

    // Check that both components were specified and found.
    if (!component1Attribute) {
        auto issue = Issue::IssueImpl::create();
        issue->mPimpl->setDescription("Connection in model '" + model->name() + "' does not specify a component_1 in a connection element.");
        issue->mPimpl->mItem->mPimpl->setModel(model);
        issue->mPimpl->setReferenceRule(Issue::ReferenceRule::CONNECTION_COMPONENT1_ATTRIBUTE);
        addIssue(issue);
        component1Missing = true;
    } else if (component1Name.empty()) {
        auto issue = Issue::IssueImpl::create();
        issue->mPimpl->setDescription("Connection in model '" + model->name() + "' does not have a valid component_1 in a connection element.");
        issue->mPimpl->mItem->mPimpl->setModel(model);
        issue->mPimpl->setReferenceRule(Issue::ReferenceRule::CONNECTION_COMPONENT1_ATTRIBUTE_REFERENCE);
        addIssue(issue);
        component1Missing = true;
    }

    if (!component2Attribute) {
        auto issue = Issue::IssueImpl::create();
        issue->mPimpl->setDescription("Connection in model '" + model->name() + "' does not specify a component_2 in a connection element.");
        issue->mPimpl->mItem->mPimpl->setModel(model);
        issue->mPimpl->setReferenceRule(Issue::ReferenceRule::CONNECTION_COMPONENT2_ATTRIBUTE);
        addIssue(issue);
        component2Missing = true;
    } else if (component2Name.empty()) {
        auto issue = Issue::IssueImpl::create();
        issue->mPimpl->setDescription("Connection in model '" + model->name() + "' does not have a valid component_2 in a connection element.");
        issue->mPimpl->mItem->mPimpl->setModel(model);
        issue->mPimpl->setReferenceRule(Issue::ReferenceRule::CONNECTION_COMPONENT2_ATTRIBUTE_REFERENCE);
        addIssue(issue);
        component2Missing = true;
    }

    componentNamePair = std::make_pair(component1Name, component2Name);

    if (!component1Missing && !component2Missing) {
        if (component1Name == component2Name) {
            auto issue = Issue::IssueImpl::create();
            issue->mPimpl->setDescription("Connection in model '" + model->name() + "' has a connection to itself, the at fault component is '" + component1Name + "'.");
            issue->mPimpl->mItem->mPimpl->setModel(model);
            issue->mPimpl->setReferenceRule(Issue::ReferenceRule::CONNECTION_EXCLUDE_SELF);
            addIssue(issue);
        } else {
            if (component1Name > component2Name) {
                std::string tmp = component1Name;
                component1Name = component2Name;
                component2Name = tmp;
            }

            NamePairList::const_iterator it = std::find_if(usedConnections.begin(), usedConnections.end(),
                                                           [&component1Name, &component2Name](const std::pair<std::string, std::string> &element) { return element.first == component1Name && element.second == component2Name; });

            if (it == usedConnections.end()) {
                usedConnections.emplace_back(componentNamePair);
            } else {
                auto issue = Issue::IssueImpl::create();
                issue->mPimpl->setDescription("Connection in model '" + model->name() + "' between '" + component1Name + "' and '" + component2Name + "' is not unique.");
                issue->mPimpl->mItem->mPimpl->setModel(model);
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::CONNECTION_UNIQUE);
                addIssue(issue);
            }
        }
    }

    XmlNodePtr childNode = node->firstChild();

    if (childNode == nullptr) {
        auto issue = Issue::IssueImpl::create();
        std::string des = "Connection in model '" + model->name() + "'";
        if (connectionId.empty()) {
            des += " does not contain any 'map_variables' elements and will be disregarded.";
        } else {
            des += " has an identifier of '" + connectionId + "' but does not contain any 'map_variables' elements.";
            des += " The connection will be disregarded and the associated identifier will be lost.";
        }
        issue->mPimpl->setDescription(des);
        issue->mPimpl->mItem->mPimpl->setModel(model);
        issue->mPimpl->setReferenceRule(Issue::ReferenceRule::CONNECTION_CHILD);
        issue->mPimpl->setLevel(libcellml::Issue::Level::WARNING);
        addIssue(issue);
        return;
    }

    // Iterate over connection child XML nodes.
    NamePairList usedMapVariables;
    while (childNode != nullptr) {
        // Connection map XML nodes should not have further children.
        XmlNodePtr grandchildNode = childNode->firstChild();
        while (grandchildNode) {
            if (grandchildNode->isText()) {
                std::string textNode = grandchildNode->convertToString();
                // Ignore whitespace when parsing.
                if (hasNonWhitespaceCharacters(textNode)) {
                    auto issue = Issue::IssueImpl::create();
                    issue->mPimpl->setDescription("Connection in model '" + model->name() + "' has an invalid non-whitespace child text element '" + textNode + "'.");
                    issue->mPimpl->setReferenceRule(Issue::ReferenceRule::XML_UNEXPECTED_CHARACTER);
                    issue->mPimpl->mItem->mPimpl->setModel(model);
                    addIssue(issue);
                }
            } else if (grandchildNode->isComment()) {
                // Do nothing.
            } else {
                auto issue = Issue::IssueImpl::create();
                issue->mPimpl->setDescription("Connection in model '" + model->name() + "' has an invalid child element '" + grandchildNode->name() + "' of element '" + childNode->name() + "'.");
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::XML_UNEXPECTED_ELEMENT);
                issue->mPimpl->mItem->mPimpl->setModel(model);
                addIssue(issue);
            }
            grandchildNode = grandchildNode->next();
        }

        if (parseNode(childNode, "map_variables")) {
            std::string variable1Name;
            std::string variable2Name;
            bool variable1Attribute = false;
            bool variable2Attribute = false;
            XmlAttributePtr childAttribute = childNode->firstAttribute();
            mappingId.clear();
            while (childAttribute) {
                if (childAttribute->isType("variable_1")) {
                    variable1Name = childAttribute->value();
                    variable1Attribute = true;
                } else if (childAttribute->isType("variable_2")) {
                    variable2Name = childAttribute->value();
                    variable2Attribute = true;
                } else if (isIdAttribute(childAttribute, mParsing1XVersion)) {
                    mappingId = childAttribute->value();
                } else {
                    auto issue = Issue::IssueImpl::create();
                    issue->mPimpl->setDescription("Connection in model '" + model->name() + "' has an invalid map_variables attribute '" + childAttribute->name() + "'.");
                    issue->mPimpl->setReferenceRule(Issue::ReferenceRule::MAP_VARIABLES_ELEMENT);
                    issue->mPimpl->mItem->mPimpl->setModel(model);
                    addIssue(issue);
                }
                childAttribute = childAttribute->next();
            }

            // Check that both variables were specified and found.
            if (!variable1Attribute) {
                auto issue = Issue::IssueImpl::create();
                issue->mPimpl->setDescription("Connection in model '" + model->name() + "' does not specify a variable_1 in a map_variables element.");
                issue->mPimpl->mItem->mPimpl->setModel(model);
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::MAP_VARIABLES_VARIABLE1_ATTRIBUTE);
                addIssue(issue);
                variable1Missing = true;
            } else if (variable1Name.empty()) {
                auto issue = Issue::IssueImpl::create();
                issue->mPimpl->setDescription("Connection in model '" + model->name() + "' does not have a valid variable_1 in a map_variables element.");
                issue->mPimpl->mItem->mPimpl->setModel(model);
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::MAP_VARIABLES_VARIABLE1_ATTRIBUTE_REFERENCE);
                addIssue(issue);
                variable1Missing = true;
            }
            if (!variable2Attribute) {
                auto issue = Issue::IssueImpl::create();
                issue->mPimpl->setDescription("Connection in model '" + model->name() + "' does not specify a variable_2 in a map_variables element.");
                issue->mPimpl->mItem->mPimpl->setModel(model);
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::MAP_VARIABLES_VARIABLE2_ATTRIBUTE);
                addIssue(issue);
                variable2Missing = true;
            } else if (variable2Name.empty()) {
                auto issue = Issue::IssueImpl::create();
                issue->mPimpl->setDescription("Connection in model '" + model->name() + "' does not have a valid variable_2 in a map_variables element.");
                issue->mPimpl->mItem->mPimpl->setModel(model);
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::MAP_VARIABLES_VARIABLE2_ATTRIBUTE_REFERENCE);
                addIssue(issue);
                variable2Missing = true;
            }
            // We can have multiple map_variables per connection.
            variableNameInfo = {variable1Name, variable2Name, mappingId};
            variableNameMap.push_back(variableNameInfo);
            mapVariablesFound = true;

            if (!variable1Missing && !variable2Missing) {
                if (variable1Name > variable2Name) {
                    std::string tmp = variable1Name;
                    variable1Name = variable2Name;
                    variable2Name = tmp;
                }

                auto variableNamePair = std::make_pair(variable1Name, variable2Name);

                NamePairList::const_iterator it = std::find_if(usedMapVariables.begin(), usedMapVariables.end(),
                                                               [&variableNamePair](const std::pair<std::string, std::string> &element) { return element.first == variableNamePair.first && element.second == variableNamePair.second; });

                if (it == usedMapVariables.end()) {
                    usedMapVariables.emplace_back(variableNamePair);
                } else {
                    auto issue = Issue::IssueImpl::create();
                    issue->mPimpl->setDescription("Connection in model '" + model->name() + "' between '" + variableNamePair.first + "' and '" + variableNamePair.second + "' is not unique.");
                    issue->mPimpl->mItem->mPimpl->setModel(model);
                    issue->mPimpl->setReferenceRule(Issue::ReferenceRule::MAP_VARIABLES_UNIQUE);
                    addIssue(issue);
                }
            }

        } else if (childNode->isText()) {
            const std::string textNode = childNode->convertToString();
            // Ignore whitespace when parsing.
            if (hasNonWhitespaceCharacters(textNode)) {
                auto issue = Issue::IssueImpl::create();
                issue->mPimpl->setDescription("Connection in model '" + model->name() + "' has an invalid non-whitespace child text element '" + textNode + "'.");
                issue->mPimpl->mItem->mPimpl->setModel(model);
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::XML_UNEXPECTED_CHARACTER);
                addIssue(issue);
            }
        } else if (childNode->isComment()) {
            // Do nothing.
        } else if (!mParsing1XVersion || (childNode->name() != "map_components")) {
            auto issue = Issue::IssueImpl::create();
            if (mParsing1XVersion) {
                issue->mPimpl->setDescription("Connection in model '" + model->name() + "' ignoring child element '" + childNode->name() + "'.");
                issue->mPimpl->setLevel(Issue::Level::MESSAGE);
            } else {
                issue->mPimpl->setDescription("Connection in model '" + model->name() + "' has an invalid child element '" + childNode->name() + "'.");
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::CONNECTION_CHILD);
            }
            issue->mPimpl->mItem->mPimpl->setModel(model);
            addIssue(issue);
        }

        childNode = childNode->next();
    }

    // If we have a component name pair, check that the components exist in the model.
    ComponentPtr component1 = nullptr;
    ComponentPtr component2 = nullptr;
    // Now check the objects exist in the model. TODO Remove as is validation?
    if (model->containsComponent(componentNamePair.first)) {
        component1 = model->component(componentNamePair.first);
    } else {
        if (!component1Missing) {
            auto issue = Issue::IssueImpl::create();
            issue->mPimpl->setDescription("Connection in model '" + model->name() + "' specifies '" + componentNamePair.first + "' as component_1 but it does not exist in the model.");
            issue->mPimpl->mItem->mPimpl->setModel(model);
            issue->mPimpl->setReferenceRule(Issue::ReferenceRule::CONNECTION_COMPONENT1_ATTRIBUTE_REFERENCE);
            addIssue(issue);
        }
    }
    if (model->containsComponent(componentNamePair.second)) {
        component2 = model->component(componentNamePair.second);
    } else {
        if (!component2Missing) {
            auto issue = Issue::IssueImpl::create();
            issue->mPimpl->setDescription("Connection in model '" + model->name() + "' specifies '" + componentNamePair.second + "' as component_2 but it does not exist in the model.");
            issue->mPimpl->mItem->mPimpl->setModel(model);
            issue->mPimpl->setReferenceRule(Issue::ReferenceRule::CONNECTION_COMPONENT2_ATTRIBUTE_REFERENCE);
            addIssue(issue);
        }
    }

    // If we have a map_variables, check that the variables exist in the named components. TODO Remove as is validation?
    if (mapVariablesFound) {
        for (const auto &iterInfo : variableNameMap) {
            VariablePtr variable1 = nullptr;
            VariablePtr variable2 = nullptr;
            if (component1) {
                if (component1->hasVariable(iterInfo[0])) {
                    variable1 = component1->variable(iterInfo[0]);
                } else if (component1->isImport()) {
                    // With an imported component we assume this variable exists in the imported component.
                    variable1 = Variable::create();
                    variable1->setName(iterInfo[0]);
                    component1->addVariable(variable1);
                } else {
                    if (!variable1Missing) {
                        auto issue = Issue::IssueImpl::create();
                        issue->mPimpl->setDescription("Variable '" + iterInfo[0] + "' is specified as variable_1 in a connection but it does not exist in component_1 component '" + component1->name() + "' of model '" + model->name() + "'.");
                        issue->mPimpl->mItem->mPimpl->setConnection(variable1, variable2);
                        issue->mPimpl->setReferenceRule(Issue::ReferenceRule::MAP_VARIABLES_VARIABLE1_ATTRIBUTE_REFERENCE);
                        addIssue(issue);
                    }
                }
            } else {
                auto issue = Issue::IssueImpl::create();
                issue->mPimpl->setDescription("Connection in model '" + model->name() + "' specifies '" + iterInfo[0] + "' as variable_1 but the corresponding component_1 is invalid.");
                issue->mPimpl->mItem->mPimpl->setConnection(variable1, variable2);
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::CONNECTION_COMPONENT1_ATTRIBUTE);
                addIssue(issue);
            }
            if (component2) {
                if (component2->hasVariable(iterInfo[1])) {
                    variable2 = component2->variable(iterInfo[1]);
                } else if (component2->isImport()) {
                    // With an imported component we assume this variable exists in the imported component.
                    variable2 = Variable::create();
                    variable2->setName(iterInfo[1]);
                    component2->addVariable(variable2);
                } else {
                    if (!variable2Missing) {
                        auto issue = Issue::IssueImpl::create();
                        issue->mPimpl->setDescription("Variable '" + iterInfo[1] + "' is specified as variable_2 in a connection but it does not exist in component_2 component '" + component2->name() + "' of model '" + model->name() + "'.");
                        issue->mPimpl->mItem->mPimpl->setConnection(variable1, variable2);
                        issue->mPimpl->setReferenceRule(Issue::ReferenceRule::MAP_VARIABLES_VARIABLE2_ATTRIBUTE_REFERENCE);
                        addIssue(issue);
                    }
                }
            } else {
                auto issue = Issue::IssueImpl::create();
                issue->mPimpl->setDescription("Connection in model '" + model->name() + "' specifies '" + iterInfo[1] + "' as variable_2 but the corresponding component_2 is invalid.");
                issue->mPimpl->mItem->mPimpl->setConnection(variable1, variable2);
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::CONNECTION_COMPONENT2_ATTRIBUTE);
                addIssue(issue);
            }
            // Set the variable equivalence relationship for this variable pair.
            if ((variable1) && (variable2)) {
                Variable::addEquivalence(variable1, variable2, iterInfo[2], connectionId);
            }
        }
    } else {
        auto issue = Issue::IssueImpl::create();
        issue->mPimpl->setDescription("Connection in model '" + model->name() + "' does not have a map_variables element.");
        issue->mPimpl->mItem->mPimpl->setModel(model);
        issue->mPimpl->setReferenceRule(Issue::ReferenceRule::CONNECTION_CHILD);
        addIssue(issue);
    }
}

ComponentPtr Parser::ParserImpl::loadComponentRef(const ModelPtr &model, const XmlNodePtr &node, NameList &usedNames)
{
    ComponentPtr parentComponent = nullptr;
    std::string parentComponentName;
    std::string encapsulationId;
    // Check for a component in the parent component_ref.
    XmlAttributePtr attribute = node->firstAttribute();
    while (attribute != nullptr) {
        if (attribute->isType("component")) {
            parentComponentName = attribute->value();

            if (std::find(usedNames.begin(), usedNames.end(), parentComponentName) == usedNames.end()) {
                usedNames.emplace_back(parentComponentName);
            } else {
                auto issue = Issue::IssueImpl::create();
                issue->mPimpl->setDescription("Encapsulation in model '" + model->name() + "' specifies '" + parentComponentName + "' as a component in a component_ref but it is not unique.");
                issue->mPimpl->mItem->mPimpl->setEncapsulation(model);
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::COMPONENT_REF_COMPONENT_ATTRIBUTE_UNIQUE);
                addIssue(issue);
            }

            if (model->containsComponent(parentComponentName)) {
                // Will re-add this to the model once we encapsulate the child(ren).
                parentComponent = model->takeComponent(parentComponentName);
            } else {
                auto issue = Issue::IssueImpl::create();
                issue->mPimpl->setDescription("Encapsulation in model '" + model->name() + "' specifies '" + parentComponentName + "' as a component in a component_ref but it does not exist in the model.");
                issue->mPimpl->mItem->mPimpl->setEncapsulation(model);
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::COMPONENT_REF_COMPONENT_ATTRIBUTE_REFERENCE);
                addIssue(issue);
            }
        } else if (isIdAttribute(attribute, mParsing1XVersion)) {
            encapsulationId = attribute->value();
        } else {
            auto issue = Issue::IssueImpl::create();
            issue->mPimpl->setDescription("Encapsulation in model '" + model->name() + "' has an invalid component_ref attribute '" + attribute->name() + "'.");
            issue->mPimpl->mItem->mPimpl->setEncapsulation(model);
            issue->mPimpl->setReferenceRule(Issue::ReferenceRule::COMPONENT_REF_ELEMENT);
            addIssue(issue);
        }
        attribute = attribute->next();
    }
    if (!parentComponent && parentComponentName.empty()) {
        auto issue = Issue::IssueImpl::create();
        issue->mPimpl->setDescription("Encapsulation in model '" + model->name() + "' does not have a valid component attribute in a component_ref element.");
        issue->mPimpl->mItem->mPimpl->setEncapsulation(model);
        issue->mPimpl->setReferenceRule(Issue::ReferenceRule::COMPONENT_REF_COMPONENT_ATTRIBUTE);
        addIssue(issue);
    } else if (parentComponent) {
        parentComponent->setEncapsulationId(encapsulationId);
    }

    // Get first child of this parent component_ref.
    XmlNodePtr childComponentNode = node->firstChild();

    // Loop over encapsulated children.
    std::string childEncapsulationId;
    while (childComponentNode) {
        ComponentPtr childComponent = nullptr;
        if (parseNode(childComponentNode, "component_ref")) {
            childComponent = loadComponentRef(model, childComponentNode, usedNames);
        } else if (childComponentNode->isText()) {
            const std::string textNode = childComponentNode->convertToString();
            // Ignore whitespace when parsing.
            if (hasNonWhitespaceCharacters(textNode)) {
                auto issue = Issue::IssueImpl::create();
                issue->mPimpl->setDescription("Encapsulation in model '" + model->name() + "' has an invalid non-whitespace child text element '" + textNode + "'.");
                issue->mPimpl->mItem->mPimpl->setEncapsulation(model);
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::XML_UNEXPECTED_CHARACTER);
                addIssue(issue);
            }
        } else if (childComponentNode->isComment()) {
            // Do nothing.
        } else {
            auto issue = Issue::IssueImpl::create();
            issue->mPimpl->setDescription("Encapsulation in model '" + model->name() + "' has an invalid child element '" + childComponentNode->name() + "'.");
            issue->mPimpl->mItem->mPimpl->setEncapsulation(model);
            issue->mPimpl->setReferenceRule(Issue::ReferenceRule::COMPONENT_REF_CHILD);
            addIssue(issue);
        }

        if (childComponent) {
            // Set parent/child encapsulation relationship.
            if (parentComponent) {
                parentComponent->addComponent(childComponent);
            } else {
                // Making sure that children which were taken from parents above, but have
                // invalid parents, are still included in the model.
                model->addComponent(childComponent);
            }
        }
        childComponentNode = childComponentNode->next();
    }

    return parentComponent;
}

void Parser::ParserImpl::loadEncapsulation(const ModelPtr &model, const XmlNodePtr &node)
{
    NameList usedNames;
    XmlNodePtr componentRefNode = node->firstChild();
    while (componentRefNode != nullptr) {
        ComponentPtr parentComponent = nullptr;
        std::string encapsulationId;
        bool haveComponentRef = false;
        if (parseNode(componentRefNode, "component_ref")) {
            haveComponentRef = true;
            parentComponent = loadComponentRef(model, componentRefNode, usedNames);
        } else if (componentRefNode->isText()) {
            const std::string textNode = componentRefNode->convertToString();
            // Ignore whitespace when parsing.
            if (hasNonWhitespaceCharacters(textNode)) {
                auto issue = Issue::IssueImpl::create();
                issue->mPimpl->setDescription("Encapsulation in model '" + model->name() + "' has an invalid non-whitespace child text element '" + textNode + "'.");
                issue->mPimpl->mItem->mPimpl->setEncapsulation(model);
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::XML_UNEXPECTED_CHARACTER);
                addIssue(issue);
            } else {
                // Continue to next node if this is whitespace (don't try to parse children of whitespace).
                componentRefNode = componentRefNode->next();
                continue;
            }
        } else if (componentRefNode->isComment()) {
            // Do nothing.
        } else if (mParsing1XVersion && componentRefNode->isCellml1XElement("relationship_ref")) {
            // Do nothing.
        } else {
            auto issue = Issue::IssueImpl::create();
            issue->mPimpl->setDescription("Encapsulation in model '" + model->name() + "' has an invalid child element '" + componentRefNode->name() + "'.");
            issue->mPimpl->mItem->mPimpl->setEncapsulation(model);
            issue->mPimpl->setReferenceRule(Issue::ReferenceRule::ENCAPSULATION_CHILD);
            addIssue(issue);
        }

        // Add the parentComponent to the model with its child(ren) encapsulated.
        if (parentComponent) {
            model->addComponent(parentComponent);
            if (parentComponent->componentCount() == 0) {
                auto issue = Issue::IssueImpl::create();
                issue->mPimpl->setDescription("Encapsulation in model '" + model->name() + "' specifies '" + parentComponent->name() + "' as a parent component_ref but it does not have any children.");
                issue->mPimpl->mItem->mPimpl->setEncapsulation(model);
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::ENCAPSULATION_CHILD);
                addIssue(issue);
            }
        } else if (haveComponentRef) {
            auto issue = Issue::IssueImpl::create();
            issue->mPimpl->setDescription("Encapsulation in model '" + model->name() + "' specifies an invalid parent component_ref that also does not have any children.");
            issue->mPimpl->mItem->mPimpl->setEncapsulation(model);
            issue->mPimpl->setReferenceRule(Issue::ReferenceRule::ENCAPSULATION_CHILD);
            addIssue(issue);
        }

        componentRefNode = componentRefNode->next();
    }
}

void Parser::ParserImpl::loadImport(ImportSourcePtr &importSource, const ModelPtr &model, const XmlNodePtr &node)
{
    XmlAttributePtr attribute = node->firstAttribute();
    std::string id;
    bool hrefAttributePresent = false;
    while (attribute != nullptr) {
        if (attribute->isType("href", XLINK_NS)) {
            importSource->setUrl(attribute->value());
            hrefAttributePresent = true;
        } else if (isIdAttribute(attribute, mParsing1XVersion)) {
            id = attribute->value();
            importSource->setId(id);
        } else if (attribute->inNamespaceUri(XLINK_NS)) {
            // Allow xlink attributes but do nothing for them.
        } else {
            auto issue = Issue::IssueImpl::create();
            issue->mPimpl->setDescription("Import from '" + node->attribute("href") + "' has an invalid attribute '" + attribute->name() + "'.");
            issue->mPimpl->mItem->mPimpl->setImportSource(importSource);
            issue->mPimpl->setReferenceRule(Issue::ReferenceRule::IMPORT_ELEMENT);
            addIssue(issue);
        }
        attribute = attribute->next();
    }

    if (!hrefAttributePresent) {
        auto issue = Issue::IssueImpl::create();
        issue->mPimpl->setDescription("Import does not specify an xlink:href attribute.");
        issue->mPimpl->mItem->mPimpl->setImportSource(importSource);
        issue->mPimpl->setReferenceRule(Issue::ReferenceRule::IMPORT_HREF);
        addIssue(issue);
    }

    XmlNodePtr childNode = node->firstChild();

    if (childNode == nullptr) {
        auto issue = Issue::IssueImpl::create();
        if (id.empty()) {
            issue->mPimpl->setDescription("Import from '" + node->attribute("href") + "' is empty and will be disregarded.");
        } else {
            issue->mPimpl->setDescription("Import from '" + node->attribute("href") + "' has an identifier of '" + id + "' but is empty. The import will be disregarded and the associated identifier will be lost.");
        }
        issue->mPimpl->mItem->mPimpl->setImportSource(importSource);
        issue->mPimpl->setLevel(libcellml::Issue::Level::WARNING);
        issue->mPimpl->setReferenceRule(Issue::ReferenceRule::IMPORT_CHILD);
        addIssue(issue);
    }
    bool nameAttributePresent;
    while (childNode != nullptr) {
        if (parseNode(childNode, "component")) {
            ComponentPtr importedComponent = Component::create();
            XmlAttributePtr childAttribute = childNode->firstAttribute();
            importedComponent->setImportSource(importSource);
            nameAttributePresent = false;
            while (childAttribute) {
                if (childAttribute->isType("name")) {
                    importedComponent->setName(childAttribute->value());
                    nameAttributePresent = true;
                } else if (isIdAttribute(childAttribute, mParsing1XVersion)) {
                    importedComponent->setId(childAttribute->value());
                } else if (childAttribute->isType("component_ref")) {
                    importedComponent->setImportReference(childAttribute->value());
                } else {
                    auto issue = Issue::IssueImpl::create();
                    issue->mPimpl->setDescription("Import of component '" + childNode->attribute("name") + "' from '" + node->attribute("href") + "' has an invalid attribute '" + childAttribute->name() + "'.");
                    issue->mPimpl->mItem->mPimpl->setImportSource(importSource);
                    issue->mPimpl->setReferenceRule(Issue::ReferenceRule::IMPORT_COMPONENT_ELEMENT);
                    addIssue(issue);
                }
                childAttribute = childAttribute->next();
            }

            if (!nameAttributePresent) {
                auto issue = Issue::IssueImpl::create();
                issue->mPimpl->setDescription("Import of component does not specify a name attribute.");
                issue->mPimpl->mItem->mPimpl->setImportSource(importSource);
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::IMPORT_COMPONENT_NAME);
                addIssue(issue);
            }

            model->addComponent(importedComponent);
        } else if (parseNode(childNode, "units")) {
            UnitsPtr importedUnits = Units::create();
            XmlAttributePtr childAttribute = childNode->firstAttribute();
            importedUnits->setImportSource(importSource);
            nameAttributePresent = false;
            while (childAttribute) {
                if (childAttribute->isType("name")) {
                    importedUnits->setName(childAttribute->value());
                    nameAttributePresent = true;
                } else if (isIdAttribute(childAttribute, mParsing1XVersion)) {
                    importedUnits->setId(childAttribute->value());
                } else if (childAttribute->isType("units_ref")) {
                    importedUnits->setImportReference(childAttribute->value());
                } else {
                    auto issue = Issue::IssueImpl::create();
                    issue->mPimpl->setDescription("Import of units '" + childNode->attribute("name") + "' from '" + node->attribute("href") + "' has an invalid attribute '" + childAttribute->name() + "'.");
                    issue->mPimpl->mItem->mPimpl->setImportSource(importSource);
                    issue->mPimpl->setReferenceRule(Issue::ReferenceRule::IMPORT_UNITS_ELEMENT);
                    addIssue(issue);
                }
                childAttribute = childAttribute->next();
            }

            if (!nameAttributePresent) {
                auto issue = Issue::IssueImpl::create();
                issue->mPimpl->setDescription("Import of units does not specify a name attribute.");
                issue->mPimpl->mItem->mPimpl->setImportSource(importSource);
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::IMPORT_UNITS_NAME);
                addIssue(issue);
            }

            model->addUnits(importedUnits);
        } else if (childNode->isText()) {
            const std::string textNode = childNode->convertToString();
            // Ignore whitespace when parsing.
            if (hasNonWhitespaceCharacters(textNode)) {
                auto issue = Issue::IssueImpl::create();
                issue->mPimpl->setDescription("Import from '" + node->attribute("href") + "' has an invalid non-whitespace child text element '" + textNode + "'.");
                issue->mPimpl->mItem->mPimpl->setImportSource(importSource);
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::XML_UNEXPECTED_CHARACTER);
                addIssue(issue);
            }
        } else if (childNode->isComment()) {
            // Do nothing.
        } else {
            auto issue = Issue::IssueImpl::create();
            if (mParsing1XVersion) {
                issue->mPimpl->setDescription("Import from '" + node->attribute("href") + "' ignoring child element '" + childNode->name() + "'.");
                issue->mPimpl->setLevel(Issue::Level::MESSAGE);
            } else {
                issue->mPimpl->setDescription("Import from '" + node->attribute("href") + "' has an invalid child element '" + childNode->name() + "'.");
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::XML_UNEXPECTED_ELEMENT);
            }
            issue->mPimpl->mItem->mPimpl->setImportSource(importSource);
            addIssue(issue);
        }
        childNode = childNode->next();
    }
}

void Parser::ParserImpl::loadResetChild(const std::string &childType, const ResetPtr &reset, const ComponentPtr &component, const XmlNodePtr &node)
{
    std::string variableName;
    std::string testVariableName;

    if (reset->variable() != nullptr) {
        variableName = reset->variable()->name();
    }

    if (reset->testVariable() != nullptr) {
        testVariableName = reset->testVariable()->name();
    }

    XmlAttributePtr childAttribute = node->firstAttribute();
    while (childAttribute) {
        if (childAttribute->isType("id")) {
            if (childType == "test_value") {
                reset->setTestValueId(childAttribute->value());
            } else {
                reset->setResetValueId(childAttribute->value());
            }
        } else {
            auto issue = Issue::IssueImpl::create();
            issue->mPimpl->setDescription("Reset in component '" + component->name() + "' referencing variable '" + variableName + "' and test_variable '" + testVariableName + "' has an unexpected attribute in the " + childType + " block of '" + childAttribute->name() + "'.");
            issue->mPimpl->mItem->mPimpl->setReset(reset);
            issue->mPimpl->setReferenceRule(Issue::ReferenceRule::RESET_ELEMENT);
            addIssue(issue);
        }
        childAttribute = childAttribute->next();
    }

    XmlNodePtr mathNode = node->firstChild();
    while (mathNode != nullptr) {
        if (mathNode->isMathmlElement("math")) {
            std::string math = mathNode->convertToString() + "\n";
            if (childType == "test_value") {
                reset->appendTestValue(math);
            } else {
                reset->appendResetValue(math);
            }
        } else if (mathNode->isText()) {
            std::string textNode = mathNode->convertToString();
            // Ignore whitespace when parsing.
            if (hasNonWhitespaceCharacters(textNode)) {
                auto issue = Issue::IssueImpl::create();
                issue->mPimpl->setDescription(childType + " has an invalid non-whitespace child text element '" + textNode + "'.");
                issue->mPimpl->mItem->mPimpl->setReset(reset);
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::XML_UNEXPECTED_CHARACTER);
                addIssue(issue);
            }
        } else if (mathNode->isComment()) {
            // Do nothing
        } else {
            std::string textNode = mathNode->convertToString();
            auto issue = Issue::IssueImpl::create();
            issue->mPimpl->setDescription("The " + childType + " in the reset in component '" + component->name() + "' referencing variable '" + variableName + "' and test_variable '" + testVariableName + "' should have a MathML block as a child.");
            issue->mPimpl->mItem->mPimpl->setReset(reset);
            if (childType == "test_value") {
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::TEST_VALUE_CHILD);
            } else {
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::RESET_VALUE_CHILD);
            }
            addIssue(issue);
        }
        mathNode = mathNode->next();
    }
}

bool Parser::ParserImpl::parseNode(const XmlNodePtr &node, const char *name)
{
    if (mParsing20Version) {
        return node->isCellml20Element(name);
    }

    return node->isCellml1XElement(name);
}

void Parser::ParserImpl::checkResetChildMultiplicity(size_t count, const std::string &childType, const ResetPtr &reset, const ComponentPtr &component)
{
    std::string variableName;
    std::string testVariableName;

    if (reset->variable() != nullptr) {
        variableName = reset->variable()->name();
    }

    if (reset->testVariable() != nullptr) {
        testVariableName = reset->testVariable()->name();
    }

    if (count > 1) {
        auto issue = Issue::IssueImpl::create();
        issue->mPimpl->setDescription("Reset in component '" + component->name() + "' referencing variable '"
                                      + variableName + "' and test_variable '" + testVariableName + "' has " + convertToString(count) + " " + childType + " blocks.");
        issue->mPimpl->mItem->mPimpl->setReset(reset);
        issue->mPimpl->setReferenceRule(Issue::ReferenceRule::RESET_CHILD);
        addIssue(issue);
    }
    if (count == 0) {
        auto issue = Issue::IssueImpl::create();
        issue->mPimpl->setDescription("Reset in component '" + component->name() + "' referencing variable '"
                                      + variableName + "' and test_variable '" + testVariableName + "' does not have a " + childType + " block defined.");
        issue->mPimpl->mItem->mPimpl->setReset(reset);
        if (childType == "reset_value") {
            issue->mPimpl->setReferenceRule(Issue::ReferenceRule::RESET_RESET_VALUE_CHILD);
        } else {
            issue->mPimpl->setReferenceRule(Issue::ReferenceRule::RESET_TEST_VALUE_CHILD);
        }
        addIssue(issue);
    }
}

void Parser::ParserImpl::loadReset(const ResetPtr &reset, const ComponentPtr &component, const XmlNodePtr &node)
{
    int order = 0;
    bool orderValid = false;
    bool orderDefined = false;

    XmlAttributePtr attribute = node->firstAttribute();
    while (attribute != nullptr) {
        if (attribute->isType("variable")) {
            const std::string variableReference = attribute->value();
            VariablePtr referencedVariable = component->variable(variableReference);

            if (referencedVariable == nullptr) {
                auto issue = Issue::IssueImpl::create();
                issue->mPimpl->setDescription("Reset referencing variable '" + variableReference + "' is not a valid reference for a variable in component '" + component->name() + "'.");
                issue->mPimpl->mItem->mPimpl->setReset(reset);
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::RESET_VARIABLE_REFERENCE);
                addIssue(issue);
            } else {
                reset->setVariable(referencedVariable);
            }
        } else if (attribute->isType("test_variable")) {
            const std::string testVariableReference = attribute->value();
            VariablePtr testVariable = component->variable(testVariableReference);
            if (testVariable == nullptr) {
                auto issue = Issue::IssueImpl::create();
                issue->mPimpl->setDescription("Reset referencing test_variable '" + testVariableReference + "' is not a valid reference for a variable in component '" + component->name() + "'.");
                issue->mPimpl->mItem->mPimpl->setReset(reset);
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::RESET_TEST_VARIABLE_REFERENCE);
                addIssue(issue);
            } else {
                reset->setTestVariable(testVariable);
            }
        } else if (attribute->isType("order")) {
            orderDefined = true;
            orderValid = isCellMLInteger(attribute->value());
            if (orderValid) {
                orderValid = convertToInt(attribute->value(), order);
                if (!orderValid) {
                    std::string variableName;
                    if (reset->variable() != nullptr) {
                        variableName = reset->variable()->name();
                    }
                    auto issue = Issue::IssueImpl::create();
                    issue->mPimpl->setDescription("Reset in component '" + component->name() + "' referencing variable '" + variableName + "' has an out of range integer order value '" + attribute->value() + "'.");
                    issue->mPimpl->mItem->mPimpl->setReset(reset);
                    issue->mPimpl->setReferenceRule(Issue::ReferenceRule::RESET_ORDER_VALUE);
                    addIssue(issue);
                }
            } else { // This value won't be saved for validation later, so it does need to be reported now.
                std::string variableName;
                if (reset->variable() != nullptr) {
                    variableName = reset->variable()->name();
                }
                auto issue = Issue::IssueImpl::create();
                issue->mPimpl->setDescription("Reset in component '" + component->name() + "' referencing variable '" + variableName + "' has a non-integer order value '" + attribute->value() + "'.");
                issue->mPimpl->mItem->mPimpl->setReset(reset);
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::RESET_ORDER_VALUE);
                addIssue(issue);
            }
        } else if (attribute->isType("id")) {
            reset->setId(attribute->value());
        } else {
            auto issue = Issue::IssueImpl::create();
            issue->mPimpl->setDescription("Reset in component '" + component->name() + "' has an invalid attribute '" + attribute->name() + "'.");
            issue->mPimpl->mItem->mPimpl->setReset(reset);
            issue->mPimpl->setReferenceRule(Issue::ReferenceRule::RESET_ATTRIBUTE_REQUIRED);
            addIssue(issue);
        }
        attribute = attribute->next();
    }

    if (orderValid) {
        reset->setOrder(order);
    } else if (!orderDefined) {
        auto issue = Issue::IssueImpl::create();
        issue->mPimpl->setDescription("Reset in component '" + component->name() + "' does not have its order set.");
        issue->mPimpl->mItem->mPimpl->setReset(reset);
        issue->mPimpl->setReferenceRule(Issue::ReferenceRule::RESET_ORDER_VALUE);
        addIssue(issue);
    }

    XmlNodePtr childNode = node->firstChild();

    size_t testValueCount = 0;
    size_t resetValueCount = 0;
    while (childNode != nullptr) {
        if (childNode->isCellml20Element("test_value")) {
            loadResetChild("test_value", reset, component, childNode);
            testValueCount++;
        } else if (childNode->isCellml20Element("reset_value")) {
            loadResetChild("reset_value", reset, component, childNode);
            resetValueCount++;
        } else if (childNode->isText()) {
            std::string textNode = childNode->convertToString();
            // Ignore whitespace when parsing.
            if (hasNonWhitespaceCharacters(textNode)) {
                auto issue = Issue::IssueImpl::create();
                issue->mPimpl->setDescription("Reset has an invalid non-whitespace child text element '" + textNode + "'. Either a test_value block or a reset_value block is expected.");
                issue->mPimpl->mItem->mPimpl->setReset(reset);
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::XML_UNEXPECTED_CHARACTER);
                addIssue(issue);
            }
        } else if (childNode->isComment()) {
            // Do nothing.
        } else {
            auto issue = Issue::IssueImpl::create();
            issue->mPimpl->setDescription("Reset in component '" + component->name() + "' has an invalid child '" + childNode->name() + "'.");
            issue->mPimpl->mItem->mPimpl->setReset(reset);
            issue->mPimpl->setReferenceRule(Issue::ReferenceRule::XML_UNEXPECTED_ELEMENT);
            addIssue(issue);
        }
        childNode = childNode->next();
    }

    checkResetChildMultiplicity(testValueCount, "test_value", reset, component);
    checkResetChildMultiplicity(resetValueCount, "reset_value", reset, component);
}

} // namespace libcellml
