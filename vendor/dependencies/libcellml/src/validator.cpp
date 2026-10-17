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

#include "libcellml/validator.h"

#include <algorithm>
#include <cmath>
#include <libxml/uri.h>
#include <map>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>

#include "libcellml/importsource.h"
#include "libcellml/reset.h"
#include "libcellml/units.h"
#include "libcellml/variable.h"

#include "anycellmlelement_p.h"
#include "commonutils.h"
#include "issue_p.h"
#include "logger_p.h"
#include "namespaces.h"
#include "utilities.h"
#include "xmldoc.h"
#include "xmlutils.h"

namespace libcellml {

/**
 * Type definition for a list of issue descriptions.
 */
using IssuesList = std::vector<Strings>;

/**
 * @brief Validate that equivalent variable pairs in the @p model
 * have equivalent units.
 * Any errors will be logged in the @c Validator.
 *
 * Any difference in base units is reported as an error in the @c Validator, but the multiplier difference does not trigger a validator error.
 * Where the base units are equivalent, the multiplier may be interpreted as units_of_v1 = (10^multiplier)*units_of_v2
 *
 * @param model The model containing the variables.
 * @param v1 The variable which may contain units.
 * @param v2 The equivalent variable which may contain units.
 * @param hints String containing error messages to be passed back to the calling function for logging.
 * @param multiplier Double returning the effective multiplier mismatch between the units.
 */
bool unitsAreEquivalent(const ModelPtr &model, const VariablePtr &v1, const VariablePtr &v2, std::string &hints, double &multiplier);

/**
 * @brief Utility function used by unitsAreEquivalent to compare base units of two variables.
 *
 * Utility function used by unitsAreEquivalent to compare base units of two variables.
 *
 * @param model The model containing the variables.
 * @param unitMap A list of the exponents of base variables.
 * @param multiplier Multiplier of the current unit in its parent.
 * @param uName String name of the current variable being investigated.
 * @param uExp Exponent of the current unit in its parent.
 * @param logMult Log multiplier.
 * @param direction Specify whether we want to increment (1) or decrement (-1).
 */
void updateBaseUnitCount(const ModelPtr &model,
                         std::map<std::string, double> &unitMap,
                         double &multiplier,
                         const std::string &uName,
                         double uExp, double logMult, int direction);

/**
 * @brief Validate the provided @p name is a valid CellML identifier.
 *
 * Checks if the provided @p name is a valid CellML identifier according
 * to the CellML 2.0 specification. This requires a non-zero length Unicode
 * character sequence containing basic Latin alphanumeric characters or
 * underscores that does not begin with a number.  Returns the rule in the
 * specification that the given @p name violates.  If the name is a valid
 * name then Issue::ReferenceRule::UNDEFINED is returned.
 *
 * @param name The @c std::string identifier to check.
 *
 * @return Returns UNDEFINED if the name is a valid CellML identifier
 * otherwise one of:
 *  - DATA_REPR_IDENTIFIER_BEGIN_EURO_NUM;
 *  - DATA_REPR_IDENTIFIER_LATIN_ALPHANUM; or
 *  - DATA_REPR_IDENTIFIER_AT_LEAST_ONE_ALPHANUM.
 */
Issue::ReferenceRule validateCellmlIdentifier(const std::string &name)
{
    // One or more alphabetic characters.
    if (!name.empty()) {
        // Does not start with numeric character.
        if (isdigit(name[0]) != 0) {
            return Issue::ReferenceRule::DATA_REPR_IDENTIFIER_BEGIN_EURO_NUM;
        }
        // Basic Latin alphanumeric characters and underscores.
        if (name.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_") != std::string::npos) {
            return Issue::ReferenceRule::DATA_REPR_IDENTIFIER_LATIN_ALPHANUM;
        }
    } else {
        // Empty string.
        return Issue::ReferenceRule::DATA_REPR_IDENTIFIER_AT_LEAST_ONE_ALPHANUM;
    }
    return Issue::ReferenceRule::UNDEFINED;
}

/**
 * @brief Check if the provided @p name is a valid CellML identifier.
 *
 * Test if the given @p name is a valid CellML identifier according to
 * the CellML 2.0 specification.
 *
 * @param name The @c std::string identifier to check.
 *
 * @return @c true if @p name is a valid CellML identifier and @c false otherwise.
 */
bool isCellmlIdentifier(const std::string &name)
{
    Issue::ReferenceRule isValid = validateCellmlIdentifier(name);
    return isValid == Issue::ReferenceRule::UNDEFINED;
}

/**
 * @brief Test to determine if @p startChar is a valid XML name start character.
 *
 * A start name character is is defined here: https://www.w3.org/TR/xml11/#NT-NameStartChar.
 *
 * @param startChar The character to test.
 *
 * @return True if the character is in the allowed Unicode ranges for a start character in an XML name.
 */
bool isNameStartChar(uint32_t startChar)
{
    // ":" | [A-Z] | "_" | [a-z] | [#xC0-#xD6] | [#xD8-#xF6] | [#xF8-#x2FF] | [#x370-#x37D] | [#x37F-#x1FFF] | [#x200C-#x200D] | [#x2070-#x218F] | [#x2C00-#x2FEF] | [#x3001-#xD7FF] | [#xF900-#xFDCF] | [#xFDF0-#xFFFD] | [#x10000-#xEFFFF]
    return (startChar == 0x3AU)
           || ((0x41U <= startChar) && (startChar <= 0x5AU))
           || (startChar == 0x5FU)
           || ((0x61U <= startChar) && (startChar <= 0x7AU))
           || ((0xC380U <= startChar) && (startChar <= 0xC396U))
           || ((0xC398U <= startChar) && (startChar <= 0xC3B6U))
           || ((0xC3B8U <= startChar) && (startChar <= 0xCBBFU))
           || ((0xCDB0U <= startChar) && (startChar <= 0xCDBDU))
           || ((0xCDBFU <= startChar) && (startChar <= 0xE1BFBFU))
           || ((0xE2808CU <= startChar) && (startChar <= 0xE2808DU))
           || ((0xE281B0U <= startChar) && (startChar <= 0xE2868FU))
           || ((0xE2B080U <= startChar) && (startChar <= 0xE2BFAFU))
           || ((0xE38081U <= startChar) && (startChar <= 0xED9FBFU))
           || ((0xEFA480U <= startChar) && (startChar <= 0xEFB78FU))
           || ((0xEFB7B0U <= startChar) && (startChar <= 0xEFBFBDU))
           || ((0xF0908080U <= startChar) && (startChar <= 0xF3AFBFBFU));
}

/**
 * @brief Test to determine if @p nameChar is a valid XML name character.
 *
 * An XML name character is defined here: https://www.w3.org/TR/xml11/#NT-NameChar.
 *
 * @param nameChar The character to test.
 *
 * @return True if the character is in the allowed Unicode ranges for an XML name character.
 */
bool isNameChar(uint32_t nameChar)
{
    if (isNameStartChar(nameChar)) {
        return true;
    }

    // "-" | "." | [0-9] | #xB7 | [#x0300-#x036F] | [#x203F-#x2040]
    return ((0x30U <= nameChar) && (nameChar <= 0x39U))
           || (nameChar == 0x2DU)
           || (nameChar == 0x2EU)
           || (nameChar == 0xC2B7U)
           || ((0xCC80U <= nameChar) && (nameChar <= 0xCDAFU))
           || ((0xE280BFU <= nameChar) && (nameChar <= 0xE28180U));
}

/**
 * @brief Convert @p text to a uint32 form.
 *
 * Convert a variable-width code point to uint32 form.
 *
 * @param text The @c std::string to convert.
 * @param initialValue The starting point for the conversion.
 *
 * @return uint32 form of @p text.
 */
uint32_t convertTextToUint32(const std::string &text, size_t initialValue = 0)
{
    const std::vector<uint8_t> bitShifts = {24, 16, 8, 0};
    uint32_t value = 0;
    size_t index = 0;
    for (size_t j = initialValue; j < 4; ++j) {
        auto tempValue = static_cast<uint32_t>(static_cast<unsigned char>(text[index++])) << bitShifts[j];
        value |= tempValue;
    }

    return value;
}

/**
 * @brief Breakdown a string into code points.
 *
 * Breakdown @p text into a vector of code points as described by
 * @c uint32_t.
 *
 * @param text The std::string to breakdown.
 *
 * @return A vector of @c uint32_t of the Unicode character values.
 */
std::vector<uint32_t> characterBreakdown(const std::string &text)
{
    std::vector<uint32_t> breakdown;
    std::vector<uint8_t> bitShifts = {24, 16, 8, 0};
    for (size_t i = 0; i < text.length();) {
        size_t codePointLength = 1;
        uint32_t value = 0;
        auto unsignedText = static_cast<uint8_t>((unsigned char)text[i]);
        if ((unsignedText & 0xf8U) == 0xf0U) {
            codePointLength = 4;
            breakdown.push_back(convertTextToUint32(text.substr(i, codePointLength)));
        } else if ((unsignedText & 0xf0U) == 0xe0U) {
            codePointLength = 3;
            breakdown.push_back(convertTextToUint32(text.substr(i, codePointLength), 1));
        } else if ((unsignedText & 0xe0U) == 0xc0U) {
            codePointLength = 2;
            breakdown.push_back(convertTextToUint32(text.substr(i, codePointLength), 2));
        } else {
            auto subText = text.substr(i, codePointLength);
            value = static_cast<uint8_t>(subText[0]);
            breakdown.push_back(value);
        }

        i += codePointLength;
    }

    return breakdown;
}

/**
 * @brief Test to determine if @p name is a valid XML name.
 *
 * An XML name is defined here: https://www.w3.org/TR/xml11/#NT-Name.
 *
 * @param name The @c std::string to test.
 *
 * @return True if the name is a valid XML name.
 */
bool isValidXmlName(const std::string &name)
{
    if (!name.empty()) {
        auto breakdown = characterBreakdown(name);
        if (!isNameStartChar(breakdown[0])) {
            return false;
        }
        for (size_t i = 1; i < breakdown.size(); ++i) {
            if (!isNameChar(breakdown[i])) {
                return false;
            }
        }
    }
    return true;
}

/**
 * @brief The Validator::ValidatorImpl class.
 *
 * The private implementation for the Validator class.
 */
class Validator::ValidatorImpl: public LoggerImpl
{
public:
    Validator *mValidator = nullptr;

    /**
     * @brief Utility function to construct an @c Issue if required for a given CellML identifier string.
     *
     * Utility function to construct an @c Issue with the appropriate description for
     * a given CellML identifier string.
     *
     * @param name The @c std::string identifier to check.
     *
     * @return An @c IssuePtr if the @p name is illegal, @c nullptr otherwise.
     */
    IssuePtr makeIssueIllegalIdentifier(const std::string &name) const;

    /**
     * @brief Validate the given component's name is unique in the model.
     *
     * The @p component is checked against known names in @p names. If
     * the component's name already exists an issue is added to the validator
     * with the component passed to the issue for further reference.
     *
     * @param model The model the component is used in.
     * @param component The component to validate the name of.
     * @param names The list of component names already used in the model.
     */
    void validateUniqueName(const ModelPtr &model, const ComponentPtr &component, NameList &names);

    /**
     * @brief Validate the @p component using the CellML 2.0 Specification.
     *
     * Validate the given @p component and its encapsulated entities using
     * the CellML 2.0 Specification. Any issues will be logged in the @c Validator.
     *
     * @param component The component to validate.
     * @param history The history of visited components.
     * @param modelsVisited The list of visited models.
     */
    void validateComponent(const ComponentPtr &component, History &history, std::vector<ModelPtr> &modelsVisited);

    /**
     * @brief Validate the component tree of the given @p component.
     *
     * Validate the given component and all child components of the component.
     *
     * @param model The model the @p component comes from.
     * @param component The @c Component to validate.
     * @param componentNames The list of already used component names used
     * to track repeated component names.
     * @param history The history of visited components.
     * @param modelsVisited The list of visited models.
     */
    void validateComponentTree(const ModelPtr &model, const ComponentPtr &component, NameList &componentNames, History &history, std::vector<ModelPtr> &modelsVisited);

    /**
     * @brief Validate the @p units using the CellML 2.0 Specification.
     *
     * Validate the given @p units and its encapsulated entities using
     * the CellML 2.0 Specification. Any issues will be logged in the @c Validator.
     *
     * @param units The units to validate.
     * @param history The history of units visited.
     * @param modelsVisited The list of visited models.
     * @param sourceUrl The source URL of the @p units.
     */
    void validateUnits(const UnitsPtr &units, History &history, std::vector<ModelPtr> &modelsVisited, const std::string &sourceUrl = ORIGIN_MODEL_REF);

    /**
     * @brief Validate the variable connections in the @p model using the CellML 2.0 Specification.
     *
     * Validate the variable connections in the given @p model using
     * the CellML 2.0 Specification. Any issues will be logged in the @c Validator.
     *
     * @param model The model which may contain variable connections to validate.
     */
    void validateConnections(const ModelPtr &model);

    /**
     * @brief Validate the units of the given variables equivalent variables.
     *
     * Validate that the variables that are equivalent to the given variable all
     * have compatible units.
     *
     * @param model The model for which the variable and model belong.
     * @param variable The variable to validate.
     * @param alreadyReported A list of variable pointer pairs.
     */
    void validateEquivalenceUnits(const ModelPtr &model, const VariablePtr &variable, VariableMap &alreadyReported);

    /**
     * @brief Validate the structure of the variables equivalences.
     *
     * Validate the structure of the variables equivalences.
     *
     * @param variable The variable to validate.
     */
    void validateEquivalenceStructure(const VariablePtr &variable);

    /**
     * @brief Validate the variable interface type.
     *
     * Validate the interface type for the given variable.
     *
     * @param variable The variable to validate.
     * @param alreadyReported A list of variable pointer pairs.
     */
    void validateVariableInterface(const VariablePtr &variable, VariableMap &alreadyReported);

    /**
     * @brief Validate the @c unit at index @c index from @p units using the CellML 2.0 Specification.
     *
     * Validate the @c unit at index @c index from @p units using
     * the CellML 2.0 Specification. Any issues will be logged in the @c Validator.
     *
     * @param index The index of the @c unit to validate from @p units.
     * @param units The units to validate.
     * @param history The history of units visited.
     * @param modelsVisited The list of visited models.
     */
    void validateUnitsUnitsItem(size_t index, const UnitsPtr &units, History &history, std::vector<ModelPtr> &modelsVisited);

    /**
     * @brief Validate the @p variable using the CellML 2.0 Specification.
     *
     * Validate the given @p variable using the CellML 2.0 Specification.
     * Any issues will be logged in the @c Validator.
     *
     * @param variable The variable to validate.
     * @param variableNames A vector list of the name attributes of the @p variable and its siblings.
     */
    void validateVariable(const VariablePtr &variable, const NameList &variableNames);

    /**
     * @brief Validate the @p reset using the CellML 2.0 Specification.
     *
     * Examine the @p reset for conformance to the CellML 2.0 specification.  Any
     * issues will be logged in the @c Validator.
     *
     * @param reset The reset to validate.
     * @param component The component the reset belongs to.
     */
    void validateReset(const ResetPtr &reset, const ComponentPtr &component);

    /**
     * @brief Validate the math @p input @c std::string.
     *
     * Validate the math @p input @c std::string using the CellML 2.0 Specification and
     * the W3C MathML DTD. Any issues will be logged in the @c Validator.
     *
     * @param input The math @c std::string to validate.
     * @param component The component containing the math @c std::string to be validated.
     */
    void validateMath(const std::string &input, const ComponentPtr &component);

    /**
     * @brief Validate an individual MathML math element.
     *
     * Validate the MathML to determine that all MathML elements are listed in the
     * supported MathML elements table from the CellML specification 2.0 document.
     *
     * @param node The node to check children and sibling nodes.
     * @param component The component the MathML belongs to.
     */
    void validateMathMLElement(const XmlNodePtr &node, const ComponentPtr &component);

    /**
     * @brief Traverse the node tree for invalid MathML elements.
     *
     * Traverse the Xml node tree checking that all MathML elements are listed in the
     * supported MathML elements table from the CellML specification 2.0 document.
     *
     * @param node The node to check children and sibling nodes.
     * @param component The component the MathML belongs to.
     */
    void validateMathMLElements(const XmlNodePtr &node, const ComponentPtr &component);

    /**
     * @brief Validate and clean the @c cn node.
     *
     * Validate the @c cn node and clear any CellML namespace from the node.
     *
     * @param node The node @c cn element.
     * @param component The component the @p node is a part of.
     */
    void validateAndCleanCnNode(const XmlNodePtr &node, const ComponentPtr &component);

    /**
     * @brief Validate that the @c ci node has a reference to a variable.
     *
     * Validate that the @c ci node has a reference to a variable.
     *
     * @param node The node @c ci element from the document.
     * @param component The component the @p node is a part of.
     * @param variableNames A list of variable names.
     */
    void validateAndCleanCiNode(const XmlNodePtr &node, const ComponentPtr &component, const NameList &variableNames);

    /**
     * @brief Validate the text of a @c cn element.
     *
     * Validates that the @c cn element has a units attached.
     *
     * @param component The component that the @c cn element belongs to.
     * @param unitsName The name of the units.
     * @param textNode The text of the @c cn element.
     *
     * @return  @c true if the @c cn units is valid, @c false otherwise.
     */
    bool validateCnUnits(const ComponentPtr &component, const std::string &unitsName, const std::string &textNode);

    /**
     * @brief Validate CellML variables and units in MathML @c ci and @c cn variables. Removes CellML units from the @p node.
     *
     * Validates CellML variables found in MathML @c ci elements. Validates @c cellml:units
     * attributes found on @c cn elements and removes them from the @c XmlNode @p node to leave MathML that may then
     * be validated using the MathML DTD.
     *
     * @param node The @ref XmlNode to validate CellML entities on and remove @c cellml:units from.
     * @param component The component that the math @c XmlNode @p node is contained within.
     * @param variableNames A @c vector list of the names of variables found within the @p component.
     */
    void validateAndCleanMathCiCnNodes(XmlNodePtr &node, const ComponentPtr &component, const NameList &variableNames);

    /**
     * @brief Add a MathML-related issue.
     *
     * Add a MathML-related issue.
     *
     * @param description The description for the MathML-related issue.
     * @param referenceRule The reference rule for the MathML-related issue.
     * @param component The component where the MathML-related issue occurred.
     */
    void addMathmlIssue(const std::string &description,
                        Issue::ReferenceRule referenceRule,
                        const ComponentPtr &component);

    bool hasOneMathmlSibling(const XmlNodePtr &parentNode,
                             const XmlNodePtr &node,
                             const ComponentPtr &component);
    bool hasAtLeastOneMathmlSibling(const XmlNodePtr &parentNode,
                                    const XmlNodePtr &node,
                                    const ComponentPtr &component);
    bool hasTwoMathmlSiblings(const XmlNodePtr &parentNode,
                              const XmlNodePtr &node,
                              const ComponentPtr &component);
    bool hasAtLeastTwoMathmlSiblings(const XmlNodePtr &parentNode,
                                     const XmlNodePtr &node,
                                     const ComponentPtr &component);
    size_t hasOneOrTwoMathmlSiblings(const XmlNodePtr &parentNode,
                                     const XmlNodePtr &node,
                                     const ComponentPtr &component);

    bool isFirstMathmlSibling(const XmlNodePtr &parentNode,
                              const XmlNodePtr &node,
                              const ComponentPtr &component);
    bool isSecondMathmlSibling(const XmlNodePtr &parentNode,
                               const XmlNodePtr &node,
                               const ComponentPtr &component);

    bool hasFirstMathmlSiblingWithName(const XmlNodePtr &parentNode,
                                       const XmlNodePtr &node,
                                       const std::string &name,
                                       const ComponentPtr &component);

    bool hasOneMathmlChild(const XmlNodePtr &node,
                           const ComponentPtr &component);
    bool hasAtLeastOneMathmlChild(const XmlNodePtr &node,
                                  const ComponentPtr &component);
    bool hasTwoMathmlChildren(const XmlNodePtr &node,
                              const ComponentPtr &component);
    bool hasOneOrTwoMathmlChildren(const XmlNodePtr &node,
                                   const ComponentPtr &component);

    /**
     * @brief Traverse the node tree for invalid MathML elements'
     * children/siblings.
     *
     * Traverse the XML node tree checking that all MathML elements have the
     * correct number of children/siblings and that their type is as expected.
     *
     * @param node The node to check children and siblings.
     * @param component The component the MathML belongs to.
     */
    void validateMathMLElementsChildrenAndSiblings(const XmlNodePtr &node,
                                                   const ComponentPtr &component);

    /**
     * @brief Check if the provided @p node is a supported MathML element.
     *
     * Checks if the provided @p node is one of the supported MathML elements defined in the table
     * of supported MathML elements from the CellML specification version 2.0 document.
     *
     * @param node The @ref XmlNode node to check against the list of supported MathML elements.
     *
     * @return @c true if @p node is a supported MathML element and @c false otherwise.
     */
    bool isSupportedMathMLElement(const XmlNodePtr &node) const;

    /** @brief Function to check IDs within the model scope are unique.
     *
     * Function to check IDs within the model scope are unique.
     *
     * @param model The model to be checked.
     */
    void checkUniqueIds(const ModelPtr &model);

    /** @brief Function to check reset orders within the model scope are unique.
     *
     * Function to check reset orders within the model scope are unique.
     *
     * @param model The model to be checked.
     */
    void checkUniqueResetOrders(const ModelPtr &model);

    /** @brief Utility function to construct a map of identifiers used within the model.
     *
     * Utility function to construct a map of identifiers used within the model.
     *
     * @param model The model to be checked.
     * @return An IdMap of the items in the model with identifier fields.
     */
    IdMap buildModelIdMap(const ModelPtr &model);

    /** @brief Utility function to construct a map of reset orders used within the model.
     *
     * Utility function to construct a map of reset orders used within the model.
     *
     * @param model The model to be checked.
     * @return A ResetOrderMap of the items in the model with reset orders.
     */
    ResetOrderMap buildModelResetOrderMap(const ModelPtr &model);

    /**
     * @brief Traverse the component tree populating the reset order map.
     *
     * Traverse the component tree populating the reset order map.
     *
     * @param component The component to check and populate from.
     * @param resetOrderMap The ResetOrderMap object to construct.
     */
    void traverseComponentTree(const ComponentPtr &component, ResetOrderMap &resetOrderMap);

    /**
     * @brief Utility function to add an item to the resetOrderMap.
     * @param variable The variable to add.
     * @param order The order associated with the variable.
     * @param resetOrderMap The resetOrderMap under construction.
     */
    void addResetOrderMapItem(const VariablePtr &variable, int order, ResetOrderMap &resetOrderMap);

    /** @brief Utility function called recursively to construct a map of identifiers in a component.
     *
     * Utility function called recursively to construct a map of identifiers in a component.
     *
     * @param component The component to check.
     * @param idMap The IdMap object to construct.
     * @param reportedConnections A set of connection identifiers to prevent duplicate reporting.
     * @param connectionIds A map to speed up lookups for component to component connection identifiers.
     */
    void buildComponentIdMap(const ComponentPtr &component, IdMap &idMap, std::set<std::string> &reportedConnections, const ConnectionIdMap &connectionIds);

    /** @brief Utility function to add an item to the idMap.
     *
     * Utility function to add an item to the idMap.
     *
     * @param id A string identifier to add.
     * @param info A string description of the item with this identifier.
     * @param idMap The IdMap under construction.
     */
    void addIdMapItem(const std::string &id, const std::string &info, IdMap &idMap);

    /** @brief Utility function to parse MathML children and add element identifiers to idMap.
     *
     * Utility function to parse MathML children and add element identifiers to idMap.
     *
     * @param node XMLNode to read.
     * @param infoRef @c std::string reference information for the math.
     * @param idMap The IdMap under construction.
     */
    void buildMathChildIdMap(const XmlNodePtr &node, const std::string &infoRef, IdMap &idMap);

    /** @brief Utility function to parse math and add element identifiers to idMap.
     *
     * Utility function to parse math and add element identifiers to idMap.
     *
     * @param infoRef @c std::string reference information for the math.
     * @param idMap The IdMap under construction.
     * @param input The @c std::string MathML string.
     */
    void buildMathIdMap(const std::string &infoRef, IdMap &idMap, const std::string &input);

    /**
     * @brief Validate the import source xlink:href and id.
     *
     * Validate the import source xlink:href and id.
     *
     * @param importSource The import source to validate.
     * @param importName The name of the entity that the import is called.
     * @param importType The type of the entity the import is, either 'units', or 'component'.
     */
    void validateImportSource(const ImportSourcePtr &importSource, const std::string &importName, const std::string &importType);

    /**
     * @brief Check if the @p names are already part of a cyclic issues.
     *
     * Check to see if the @p names have already been reported in a cyclic issue.
     *
     * @param names The names that make up the cycle.
     *
     * @return @c true if the @p names have already been reported in a cyclic issue, @c false otherwise.
     */
    bool hasCycleAlreadyBeenReported(NameList names) const;

    /**
     * @brief Check to see if the @p description is already present in the issues.
     *
     * Check to see if the @p description has already been reported in
     * existing issues.
     *
     * @param description The description to check for prior existence.
     *
     * @return @c true if the description is already present, @c false otherwise.
     */
    bool checkIssuesForDuplications(const std::string &description) const;

    /**
     * @brief Deal with errors raised from imports.
     *
     * When an error is raised by an imported entity, track back to the originating model and
     * assign the source @ref Units or @ref Component that the error belongs to.
     *
     * @param initialErrorCount The initial number of errors in the validator.
     * @param isOriginatingModel @c true if currently validating the origin model, @c false otherwise.
     * @param type The type of import, either @c Units or @c Component.
     * @param name The name of the importing entity.
     * @param history History list of visited imported entities.
     * @param component The originating component if type is @c Component.
     * @param units The originating units if type is @c Units.
     */
    void handleErrorsFromImports(size_t initialErrorCount, bool isOriginatingModel, const std::string &type,
                                 const std::string &name, const History &history, const ComponentPtr &component,
                                 const UnitsPtr &units) const;
};

bool checkForLocalCycles(const History &history, const HistoryEpochPtr &h)
{
    return std::find_if(history.begin(), history.end(),
                        [=](const HistoryEpochPtr &i) -> bool { return (i->mName == h->mName) && (i->mSourceUrl == h->mSourceUrl); })
           != history.end();
}

Validator::ValidatorImpl *Validator::pFunc()
{
    return reinterpret_cast<Validator::ValidatorImpl *>(Logger::pFunc());
}

Validator::Validator()
    : Logger(new ValidatorImpl())
{
    pFunc()->mValidator = this;
}

Validator::~Validator()
{
    delete pFunc();
}

ValidatorPtr Validator::create() noexcept
{
    return std::shared_ptr<Validator> {new Validator {}};
}

void Validator::validateModel(const ModelPtr &model)
{
    // Clear any pre-existing issues in this validator instance.
    pFunc()->removeAllIssues();

    if (model == nullptr) {
        auto issue = Issue::IssueImpl::create();
        issue->mPimpl->setReferenceRule(Issue::ReferenceRule::INVALID_ARGUMENT);
        issue->mPimpl->setDescription("The model is null.");
        pFunc()->addIssue(issue);
    } else {
        // Check for a valid name attribute.
        if (!isCellmlIdentifier(model->name())) {
            auto issue = pFunc()->makeIssueIllegalIdentifier(model->name());
            issue->mPimpl->mItem->mPimpl->setModel(model);
            issue->mPimpl->setReferenceRule(Issue::ReferenceRule::MODEL_NAME_VALUE);
            issue->mPimpl->setDescription("Model '" + model->name() + "' does not have a valid name attribute. " + issue->description());
            pFunc()->addIssue(issue);
        }
        // Check for a valid identifier.
        if (!isValidXmlName(model->id())) {
            auto issue = Issue::IssueImpl::create();
            issue->mPimpl->setReferenceRule(Issue::ReferenceRule::XML_ID_ATTRIBUTE);
            issue->mPimpl->mItem->mPimpl->setModel(model);
            issue->mPimpl->setDescription("Model '" + model->name() + "' does not have a valid 'id' attribute, '" + model->id() + "'.");
            pFunc()->addIssue(issue);
        }
        std::vector<ModelPtr> modelsVisited = {model};
        // Check for components in this model.
        if (model->componentCount() > 0) {
            NameList componentNames;
            History history;
            for (size_t i = 0; i < model->componentCount(); ++i) {
                history.clear();
                ComponentPtr component = model->component(i);
                pFunc()->validateComponentTree(model, component, componentNames, history, modelsVisited);
            }
        }
        // Check for units in this model.
        if (model->unitsCount() > 0) {
            History history;
            for (size_t i = 0; i < model->unitsCount(); ++i) {
                history.clear();
                UnitsPtr units = model->units(i);
                pFunc()->validateUnits(units, history, modelsVisited);
            }
        }

        // Validate any connections / variable equivalence networks in the model.
        pFunc()->validateConnections(model);

        // Check identifiers across the model are unique.
        pFunc()->checkUniqueIds(model);

        pFunc()->checkUniqueResetOrders(model);
    }
}

void Validator::ValidatorImpl::validateUniqueName(const ModelPtr &model, const ComponentPtr &component, NameList &names)
{
    std::string name = component->name();
    if (!name.empty()) {
        if (std::find(names.begin(), names.end(), name) != names.end()) {
            auto issue = Issue::IssueImpl::create();
            issue->mPimpl->setDescription("Model '" + model->name() + "' contains multiple components with the name '" + name + "'. Valid component names must be unique to their model.");
            issue->mPimpl->mItem->mPimpl->setModel(model);
            if (component->isImport()) {
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::IMPORT_COMPONENT_NAME_UNIQUE);
            } else {
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::COMPONENT_NAME_UNIQUE);
            }
            addIssue(issue);
        } else {
            names.push_back(name);
        }
    }
}

void Validator::ValidatorImpl::validateComponentTree(const ModelPtr &model, const ComponentPtr &component, NameList &componentNames, History &history, std::vector<ModelPtr> &modelsVisited)
{
    validateUniqueName(model, component, componentNames);
    for (size_t i = 0; i < component->componentCount(); ++i) {
        auto childComponent = component->component(i);
        validateComponentTree(model, childComponent, componentNames, history, modelsVisited);
    }
    validateComponent(component, history, modelsVisited);
}

void Validator::ValidatorImpl::validateImportSource(const ImportSourcePtr &importSource, const std::string &importName, const std::string &importType)
{
    std::string url = importSource->url();

    // Check for a valid identifier.
    if (!isValidXmlName(importSource->id())) {
        auto issue = Issue::IssueImpl::create();
        issue->mPimpl->setReferenceRule(Issue::ReferenceRule::XML_ID_ATTRIBUTE);
        issue->mPimpl->mItem->mPimpl->setImportSource(importSource);
        issue->mPimpl->setDescription("Import of " + importType + " '" + importName + "' does not have a valid 'id' attribute, '" + importSource->id() + "'.");
        addIssue(issue);
    }

    if (url.empty()) {
        auto issue = Issue::IssueImpl::create();
        issue->mPimpl->setDescription("Import of " + importType + " '" + importName + "' does not have a valid locator xlink:href attribute.");
        issue->mPimpl->mItem->mPimpl->setImportSource(importSource);
        issue->mPimpl->setReferenceRule(Issue::ReferenceRule::IMPORT_HREF_LOCATOR);
        addIssue(issue);
    } else {
        xmlURIPtr uri = xmlParseURI(url.c_str());
        if (uri == nullptr) {
            auto issue = Issue::IssueImpl::create();
            issue->mPimpl->setDescription("Import of " + importType + " '" + importName + "' has an invalid URI in the xlink:href attribute.");
            issue->mPimpl->mItem->mPimpl->setImportSource(importSource);
            issue->mPimpl->setReferenceRule(Issue::ReferenceRule::IMPORT_HREF_LOCATOR);
            addIssue(issue);

        } else {
            xmlFreeURI(uri);
        }
    }
}

void Validator::ValidatorImpl::handleErrorsFromImports(size_t initialErrorCount, bool isOriginatingModel, const std::string &type, const std::string &name, const History &history, const ComponentPtr &component, const UnitsPtr &units) const
{
    static const std::string skipThis = "Cyclic dependencies";
    static const std::string notOriginMarker = "NOT ORIGIN: ";
    static const std::string dataBoundaryMarker = "&";
    static const std::string dataSeparator = ";";

    for (size_t i = initialErrorCount; i < mValidator->issueCount(); ++i) {
        auto issue = mValidator->issue(i);
        auto description = issue->description();
        if (description.substr(0, skipThis.length()) != skipThis) {
            std::ostringstream os;
            if (isOriginatingModel) {
                // If error is not from this model then set this component or units.
                size_t depth = 0;
                size_t pos = description.find(notOriginMarker);
                size_t originalDescriptionStart = 0;
                while (pos != std::string::npos) {
                    if (depth == 0) {
                        os << "Imported ";
                        if (type == "Component") {
                            os << "component ";
                        } else {
                            os << "units ";
                        }
                        os << "'" << name << "' is not valid because:" << std::endl;
                    }
                    size_t startMarker = description.find(dataBoundaryMarker, pos);
                    size_t endMarker = description.find(dataBoundaryMarker, startMarker + 1);
                    std::string importInfo = description.substr(startMarker + 1, endMarker - startMarker - 1);
                    auto ss = split(importInfo);
                    os << "  -> " << type << " '" << ss[0] << "' importing '" << ss[1] << "' from '" << ss[2] << "'";
                    originalDescriptionStart = endMarker + 1;
                    pos = description.find(notOriginMarker, pos + 1);
                    depth += 1;
                    if (pos == std::string::npos) {
                        if (depth > 1) {
                            os << " which";
                        }
                        os << " has an error:" << std::endl
                           << "   - ";
                    } else {
                        os << " imports:" << std::endl;
                    }
                }

                if (depth > 0) {
                    if (type == "Component") {
                        issue->mPimpl->mItem->mPimpl->setComponent(component);
                    } else {
                        issue->mPimpl->mItem->mPimpl->setUnits(units);
                    }
                }

                os << description.substr(originalDescriptionStart);
                issue->mPimpl->setDescription(os.str());
            } else {
                // Get name, reference, and import source from history.
                auto h = history.back();
                os << notOriginMarker << dataBoundaryMarker << h->mName << dataSeparator << h->mReferenceName << dataSeparator << h->mDestinationUrl << dataBoundaryMarker << description;
                issue->mPimpl->setDescription(os.str());
            }
        }
    }
}

void Validator::ValidatorImpl::validateComponent(const ComponentPtr &component, History &history, std::vector<ModelPtr> &modelsVisited)
{
    size_t initialIssueCount = mValidator->issueCount();
    bool isOriginatingModel = modelsVisited.size() == 1;

    std::string componentName = component->name();

    // Check for a valid name attribute.
    bool isImported = component->isImport();
    std::string descriptionPrefix = "Component ";
    if (isImported) {
        descriptionPrefix = "Imported component ";
    }
    if (!isCellmlIdentifier(componentName)) {
        auto issue = makeIssueIllegalIdentifier(componentName);
        issue->mPimpl->mItem->mPimpl->setComponent(component);
        issue->mPimpl->setDescription(descriptionPrefix + "'" + componentName + "' does not have a valid name attribute. " + issue->description());
        if (isImported) {
            issue->mPimpl->setReferenceRule(Issue::ReferenceRule::IMPORT_COMPONENT_NAME_VALUE);
        } else {
            issue->mPimpl->setReferenceRule(Issue::ReferenceRule::COMPONENT_NAME_VALUE);
        }
        addIssue(issue);
    }
    // Check for a valid identifier.
    if (!isValidXmlName(component->id())) {
        auto issue = Issue::IssueImpl::create();
        issue->mPimpl->setReferenceRule(Issue::ReferenceRule::XML_ID_ATTRIBUTE);
        issue->mPimpl->mItem->mPimpl->setComponent(component);
        issue->mPimpl->setDescription(descriptionPrefix + "'" + componentName + "' does not have a valid 'id' attribute, '" + component->id() + "'.");
        addIssue(issue);
    }

    if (isImported) {
        // Check for a component_ref; assumes imported if the import source is not null.
        std::string componentRef = component->importReference();

        if (!isCellmlIdentifier(componentRef)) {
            auto issue = makeIssueIllegalIdentifier(componentRef);
            issue->mPimpl->setDescription(descriptionPrefix + "'" + componentName + "' does not have a valid component_ref attribute. " + issue->description());
            issue->mPimpl->mItem->mPimpl->setComponent(component);
            issue->mPimpl->setReferenceRule(Issue::ReferenceRule::IMPORT_COMPONENT_COMPONENT_REFERENCE_VALUE);
            addIssue(issue);
        }

        validateImportSource(component->importSource(), componentName, "component");

        auto importModel = component->importSource()->model();
        if (importModel != nullptr) {
            auto importedComponent = importModel->component(componentRef);
            if (importedComponent != nullptr) {
                auto h = createHistoryEpoch(component, importeeModelUrl(history, component->importSource()->url()));
                if (checkForImportCycles(history, h)) {
                    history.push_back(h);
                    auto description = formDescriptionOfCyclicDependency(history, "resolve");
                    IssuePtr issue = Issue::IssueImpl::create();
                    issue->mPimpl->setDescription(description);
                    issue->mPimpl->setReferenceRule(Issue::ReferenceRule::IMPORT_COMPONENT_COMPONENT_REFERENCE);
                    issue->mPimpl->mItem->mPimpl->setImportSource(component->importSource());
                    addIssue(issue);
                } else {
                    history.push_back(h);
                    modelsVisited.push_back(importModel);
                    validateComponent(importedComponent, history, modelsVisited);
                    modelsVisited.pop_back();
                }
                history.pop_back();
            } else {
                auto issue = Issue::IssueImpl::create();
                issue->mPimpl->setDescription(descriptionPrefix + "'" + componentName + "' refers to component '" + componentRef + "' which does not appear in '" + component->importSource()->url() + "'.");
                issue->mPimpl->mItem->mPimpl->setComponent(component);
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::IMPORT_COMPONENT_COMPONENT_REFERENCE_TARGET);
                addIssue(issue);
            }
        }
    } else {
        // Check for variables in this component.
        NameList variableNames;
        // Validate variable(s).
        for (size_t i = 0; i < component->variableCount(); ++i) {
            VariablePtr variable = component->variable(i);
            validateVariable(variable, variableNames);
            variableNames.push_back(variable->name());
        }
        // Check for resets in this component.
        for (size_t i = 0; i < component->resetCount(); ++i) {
            ResetPtr reset = component->reset(i);
            validateReset(reset, component);
        }

        // Validate math through the private implementation (for XML handling).
        if (!component->math().empty()) {
            validateMath(component->math(), component);
        }
    }

    handleErrorsFromImports(initialIssueCount, isOriginatingModel, "Component", componentName, history, component, nullptr);
}

std::set<std::string> namesInCycle(NameList allNames)
{
    std::string cycleStartName = allNames.back();
    allNames.pop_back();
    std::reverse(allNames.begin(), allNames.end());
    std::set<std::string> namesInCycle = {cycleStartName};
    std::string name = *allNames.begin();
    while (name != cycleStartName) {
        namesInCycle.emplace(name);
        allNames.erase(allNames.begin());
        name = *allNames.begin();
    }

    return namesInCycle;
}

bool Validator::ValidatorImpl::hasCycleAlreadyBeenReported(NameList names) const
{
    std::set<std::string> testNamesInCycle = namesInCycle(std::move(names));
    bool found = false;
    for (size_t i = 0; !found && (i < mValidator->issueCount()); ++i) {
        auto issue = mValidator->issue(i);
        if (issue->description().substr(0, 20) == "Cyclic units exist: ") {
            // Remove prefix to loop information.
            auto loop = issue->description().substr(20);
            // Remove suffix to loop information.
            loop.pop_back();
            auto parts = split(loop, " -> ");
            auto existingNamesInCycle = namesInCycle(parts);
            found = testNamesInCycle == existingNamesInCycle;
        }
    }

    return found;
}

bool Validator::ValidatorImpl::checkIssuesForDuplications(const std::string &description) const
{
    size_t count = mValidator->issueCount();
    for (size_t i = 0; i < count; ++i) {
        if (mValidator->issue(count - 1 - i)->description() == description) {
            return true;
        }
    }
    return false;
}

void Validator::ValidatorImpl::validateUnits(const UnitsPtr &units, History &history, std::vector<ModelPtr> &modelsVisited, const std::string &sourceUrl)
{
    auto h = createHistoryEpoch(units, sourceUrl);
    if (checkForLocalCycles(history, h)) {
        history.push_back(h);
        std::string des;
        Strings names;
        for (const auto &entry : history) {
            if (!des.empty()) {
                des += " -> ";
            }
            auto tmp = "'" + entry->mName + "'";
            names.push_back(tmp);
            des += tmp;
        }
        if (!hasCycleAlreadyBeenReported(names)) {
            auto issue = Issue::IssueImpl::create();
            issue->mPimpl->setDescription("Cyclic units exist: " + des + ".");
            issue->mPimpl->mItem->mPimpl->setUnits(units);
            issue->mPimpl->setReferenceRule(Issue::ReferenceRule::UNIT_UNITS_CIRCULAR_REFERENCE);
            addIssue(issue);
        }
        history.pop_back();
        return;
    }

    std::string unitsName = units->name();
    size_t initialIssueCount = mValidator->issueCount();
    bool isOriginatingModel = modelsVisited.size() == 1;

    std::string unitsRef;
    std::string unitsImportUrl;
    if (units->isImport()) {
        unitsRef = units->importReference();
        unitsImportUrl = units->importSource()->url();
        h->mReferenceName = unitsRef;
        h->mSourceUrl = importeeModelUrl(history, units->importSource()->url());
    }

    auto model = owningModel(units);

    size_t unitsWithNameCount = 0;
    size_t unitsWithImportSource = 0;
    for (size_t i = 0; (i < model->unitsCount()) && (modelsVisited.size() == 1); ++i) {
        auto tmpUnits = model->units(i);
        if (tmpUnits->name() == unitsName) {
            unitsWithNameCount += 1;
        }
        if (tmpUnits->isImport()) {
            if (tmpUnits->importReference() == unitsRef && tmpUnits->importSource()->url() == unitsImportUrl) {
                unitsWithImportSource += 1;
            }
        }
    }

    if (units->isImport()) {
        // Check for a units_ref.
        size_t currentIssueCount = mValidator->issueCount();
        if (!isCellmlIdentifier(unitsRef)) {
            auto issue = makeIssueIllegalIdentifier(unitsRef);
            issue->mPimpl->setDescription("Imported units '" + unitsName + "' does not have a valid units_ref attribute. " + issue->description());
            issue->mPimpl->mItem->mPimpl->setUnits(units);
            issue->mPimpl->setReferenceRule(Issue::ReferenceRule::IMPORT_UNITS_UNITS_REFERENCE_VALUE);
            addIssue(issue);
        }

        auto importSource = units->importSource();
        validateImportSource(importSource, unitsName, "units");

        bool foundImportIssue = mValidator->issueCount() > currentIssueCount;

        // Check if we already have another import from the same source with the same units_ref.
        // (This looks for matching entries at the same position in the source and ref vectors).
        if (!foundImportIssue && (unitsWithImportSource > 1)) {
            auto description = "Model '" + model->name() + "' contains multiple imported units from '" + unitsImportUrl + "' with the same units_ref attribute '" + unitsRef + "'.";
            if (!checkIssuesForDuplications(description)) {
                auto issue = Issue::IssueImpl::create();
                issue->mPimpl->setDescription(description);
                issue->mPimpl->mItem->mPimpl->setModel(model);
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::IMPORT_UNITS_UNITS_REFERENCE);
                addIssue(issue);
            }
        }

        auto importSourceModel = importSource->model();
        if (importSourceModel != nullptr) {
            auto importedUnits = importSourceModel->units(unitsRef);
            if (importedUnits != nullptr) {
                Strings importUnitsNames;
                Strings importUnitsRefs;
                Strings importUnitsImportSources;
                IssuesList importIssueList;

                if (checkForImportCycles(history, h)) {
                    history.push_back(h);
                    auto description = formDescriptionOfCyclicDependency(history, "resolve");
                    IssuePtr issue = Issue::IssueImpl::create();
                    issue->mPimpl->setDescription(description);
                    issue->mPimpl->mItem->mPimpl->setUnits(units);
                    issue->mPimpl->setReferenceRule(Issue::ReferenceRule::IMPORT_UNITS_UNITS_REFERENCE);
                    addIssue(issue);
                } else {
                    modelsVisited.push_back(importSourceModel);
                    history.push_back(h);
                    validateUnits(importedUnits, history, modelsVisited, importSource->url());
                    modelsVisited.pop_back();
                }
            } else {
                auto issue = Issue::IssueImpl::create();
                issue->mPimpl->setDescription("Imported units '" + units->name() + "' refers to units '" + unitsRef + "' which does not appear in '" + importSource->url() + "'.");
                issue->mPimpl->mItem->mPimpl->setUnits(units);
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::IMPORT_UNITS_UNITS_REFERENCE_VALUE_TARGET);
                addIssue(issue);
            }
        }
    }
    // Check for duplicate units names in this model.
    if (unitsWithNameCount > 1) {
        std::string description = "Model '" + model->name() + "' contains multiple units with the name '" + unitsName + "'. Valid units names must be unique to their model.";
        if (!checkIssuesForDuplications(description)) {
            auto issue = Issue::IssueImpl::create();
            issue->mPimpl->setDescription(description);
            issue->mPimpl->mItem->mPimpl->setModel(model);
            if (units->isImport()) {
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::IMPORT_UNITS_NAME_UNIQUE);
            } else {
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::UNITS_NAME_UNIQUE);
            }

            addIssue(issue);
        }
    }
    // Check for a valid name attribute.
    if (!isCellmlIdentifier(unitsName)) {
        auto issue = makeIssueIllegalIdentifier(unitsName);
        issue->mPimpl->mItem->mPimpl->setUnits(units);
        if (units->isImport()) {
            issue->mPimpl->setDescription("Imported units '" + unitsName + "' does not have a valid name attribute. " + issue->description());
            issue->mPimpl->setReferenceRule(Issue::ReferenceRule::IMPORT_UNITS_NAME_VALUE);
        } else {
            issue->mPimpl->setDescription("Units '" + unitsName + "' does not have a valid name attribute. " + issue->description());
            issue->mPimpl->setReferenceRule(Issue::ReferenceRule::UNITS_NAME_VALUE);
        }
        addIssue(issue);
    } else {
        // Check for a matching standard units.
        if (isStandardUnitName(unitsName)) {
            auto issue = Issue::IssueImpl::create();
            issue->mPimpl->setDescription("Units is named '" + unitsName + "' which is a protected standard unit name.");
            issue->mPimpl->mItem->mPimpl->setUnits(units);
            issue->mPimpl->setReferenceRule(Issue::ReferenceRule::UNITS_STANDARD);
            addIssue(issue);
        }
    }
    // Check for a valid identifier.
    if (!isValidXmlName(units->id())) {
        auto issue = Issue::IssueImpl::create();
        issue->mPimpl->setReferenceRule(Issue::ReferenceRule::XML_ID_ATTRIBUTE);
        issue->mPimpl->mItem->mPimpl->setUnits(units);
        std::string descriptionStart = "Units";
        if (units->isImport()) {
            descriptionStart = "Imported units";
        }
        issue->mPimpl->setDescription(descriptionStart + " '" + unitsName + "' does not have a valid 'id' attribute, '" + units->id() + "'.");
        addIssue(issue);
    }

    if (units->unitCount() > 0) {
        history.push_back(h);
        for (size_t i = 0; i < units->unitCount(); ++i) {
            validateUnitsUnitsItem(i, units, history, modelsVisited);
        }
        history.pop_back();
    }

    handleErrorsFromImports(initialIssueCount, isOriginatingModel, "Units", unitsName, history, nullptr, units);
}

void Validator::ValidatorImpl::validateUnitsUnitsItem(size_t index, const UnitsPtr &units, History &history, std::vector<ModelPtr> &modelsVisited)
{
    // Validate the unit at the given index.
    std::string reference;
    std::string prefix;
    std::string id;
    double exponent;
    double multiplier;

    units->unitAttributes(index, reference, prefix, exponent, multiplier, id);
    if (isCellmlIdentifier(reference)) {
        ModelPtr model = owningModel(units);
        if (model->hasUnits(reference) && !isStandardUnitName(reference)) {
            validateUnits(model->units(reference), history, modelsVisited);
        } else if (!model->hasUnits(reference) && !isStandardUnitName(reference)) {
            auto issue = Issue::IssueImpl::create();
            issue->mPimpl->setDescription("Units reference '" + reference + "' in units '" + units->name() + "' is not a valid reference to a local units or a standard unit type.");
            issue->mPimpl->mItem->mPimpl->setUnitsItem(UnitsItem::create(units, index));
            issue->mPimpl->setReferenceRule(Issue::ReferenceRule::UNIT_UNITS_REFERENCE);
            addIssue(issue);
        }
    } else {
        auto issue = makeIssueIllegalIdentifier(reference);
        issue->mPimpl->setDescription("Unit in units '" + units->name() + "' does not have a valid units reference. The reference given is '" + reference + "'. " + issue->description());
        issue->mPimpl->mItem->mPimpl->setUnitsItem(UnitsItem::create(units, index));
        issue->mPimpl->setReferenceRule(Issue::ReferenceRule::UNIT_UNITS_REFERENCE);
        addIssue(issue);
    }
    // Check for a valid identifier.
    if (!isValidXmlName(id)) {
        auto issue = Issue::IssueImpl::create();
        issue->mPimpl->setReferenceRule(Issue::ReferenceRule::XML_ID_ATTRIBUTE);
        issue->mPimpl->mItem->mPimpl->setUnitsItem(UnitsItem::create(units, index));
        issue->mPimpl->setDescription("Unit in units '" + units->name() + "' does not have a valid 'id' attribute, '" + units->id() + "'.");
        addIssue(issue);
    }
    if (!prefix.empty()) {
        if (!isStandardPrefixName(prefix)) {
            if (!isCellMLInteger(prefix)) {
                auto issue = Issue::IssueImpl::create();
                issue->mPimpl->setDescription("Prefix '" + prefix + "' of a unit referencing '" + reference + "' in units '" + units->name() + "' is not a valid integer or an SI prefix.");
                issue->mPimpl->mItem->mPimpl->setUnitsItem(UnitsItem::create(units, index));
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::UNIT_ATTRIBUTE_PREFIX_VALUE);
                addIssue(issue);
            } else {
                try {
                    int test = std::stoi(prefix);
                    (void)test;
                } catch (std::out_of_range &) {
                    auto issue = Issue::IssueImpl::create();
                    issue->mPimpl->setDescription("Prefix '" + prefix + "' of a unit referencing '" + reference + "' in units '" + units->name() + "' is out of the integer range.");
                    issue->mPimpl->mItem->mPimpl->setUnitsItem(UnitsItem::create(units, index));
                    issue->mPimpl->setReferenceRule(Issue::ReferenceRule::UNIT_ATTRIBUTE_PREFIX_VALUE);
                    addIssue(issue);
                }
            }
        }
    }
}

void Validator::ValidatorImpl::validateVariable(const VariablePtr &variable, const NameList &variableNames)
{
    ComponentPtr component = owningComponent(variable);
    auto variableName = variable->name();
    if (!variableName.empty()) {
        if (std::find(variableNames.begin(), variableNames.end(), variableName) != variableNames.end()) {
            auto issue = Issue::IssueImpl::create();
            issue->mPimpl->setDescription("Component '" + component->name() + "' contains multiple variables with the name '" + variableName + "'. Valid variable names must be unique to their component.");
            issue->mPimpl->mItem->mPimpl->setComponent(component);
            issue->mPimpl->setReferenceRule(Issue::ReferenceRule::VARIABLE_NAME_UNIQUE);
            addIssue(issue);
        }
    }

    // Check for a valid name attribute.
    if (!isCellmlIdentifier(variableName)) {
        auto issue = makeIssueIllegalIdentifier(variableName);
        issue->mPimpl->setDescription("Variable '" + variableName + "' in component '" + component->name() + "' does not have a valid name attribute. " + issue->description());
        issue->mPimpl->mItem->mPimpl->setVariable(variable);
        issue->mPimpl->setReferenceRule(Issue::ReferenceRule::VARIABLE_NAME_VALUE);
        addIssue(issue);
    }
    // Check for a valid identifier.
    if (!isValidXmlName(variable->id())) {
        auto issue = Issue::IssueImpl::create();
        issue->mPimpl->setReferenceRule(Issue::ReferenceRule::XML_ID_ATTRIBUTE);
        issue->mPimpl->mItem->mPimpl->setVariable(variable);
        issue->mPimpl->setDescription("Variable '" + variableName + "' does not have a valid 'id' attribute, '" + variable->id() + "'.");
        addIssue(issue);
    }
    // Check for a valid units attribute.
    if (variable->units() == nullptr) {
        auto issue = Issue::IssueImpl::create();
        issue->mPimpl->setDescription("Variable '" + variableName + "' in component '" + component->name() + "' does not have any units specified.");
        issue->mPimpl->mItem->mPimpl->setVariable(variable);
        issue->mPimpl->setReferenceRule(Issue::ReferenceRule::VARIABLE_UNITS_VALUE);
        addIssue(issue);
    } else {
        std::string unitsName = variable->units()->name();
        if (!isCellmlIdentifier(unitsName)) {
            auto issue = makeIssueIllegalIdentifier(unitsName);
            issue->mPimpl->setDescription("Variable '" + variableName + "' in component '" + component->name() + "' does not have a valid units attribute. The attribute given is '" + unitsName + "'. " + issue->description());
            issue->mPimpl->mItem->mPimpl->setVariable(variable);
            issue->mPimpl->setReferenceRule(Issue::ReferenceRule::VARIABLE_UNITS_VALUE);
            addIssue(issue);
        } else if (!isStandardUnitName(unitsName)) {
            ModelPtr model = owningModel(component);
            if (!model->hasUnits(unitsName)) {
                auto issue = Issue::IssueImpl::create();
                issue->mPimpl->setDescription("Variable '" + variableName + "' in component '" + component->name() + "' has a units reference '" + unitsName + "' which is neither standard nor defined in the parent model.");
                issue->mPimpl->mItem->mPimpl->setVariable(variable);
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::VARIABLE_UNITS_VALUE);
                addIssue(issue);
            }
        }
    }
    // Check for a valid interface attribute.
    if (!variable->interfaceType().empty()) {
        std::string interfaceType = variable->interfaceType();
        if ((interfaceType != "public") && (interfaceType != "private") && (interfaceType != "none") && (interfaceType != "public_and_private")) {
            auto issue = Issue::IssueImpl::create();
            issue->mPimpl->setDescription("Variable '" + variableName + "' in component '" + component->name() + "' has an invalid interface attribute value '" + interfaceType + "'.");
            issue->mPimpl->mItem->mPimpl->setVariable(variable);
            issue->mPimpl->setReferenceRule(Issue::ReferenceRule::VARIABLE_INTERFACE_VALUE);
            addIssue(issue);
        }
    }
    // Check for a valid initial value attribute.
    if (!variable->initialValue().empty()) {
        std::string initialValue = variable->initialValue();
        // Check if initial value is a variable reference
        if (!component->hasVariable(initialValue)) {
            // Otherwise, check that the initial value can be converted to a double
            if (!isCellMLReal(initialValue)) {
                auto issue = Issue::IssueImpl::create();
                issue->mPimpl->setDescription("Variable '" + variableName + "' in component '" + component->name() + "' has an invalid initial value '" + initialValue + "'. Initial values must be a real number string or a variable reference.");
                issue->mPimpl->mItem->mPimpl->setVariable(variable);
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::VARIABLE_INITIAL_VALUE_VALUE);
                addIssue(issue);
            }
        }
    }
}

void Validator::ValidatorImpl::validateReset(const ResetPtr &reset, const ComponentPtr &component)
{
    bool noOrder = false;
    bool noVariable = false;
    bool noTestVariable = false;
    bool noTestValue = false;
    bool noResetValue = false;
    bool varOutsideComponent = false;
    bool testVarOutsideComponent = false;
    std::string varParentName;
    std::string testVarParentName;

    std::string description = "Reset in component '" + component->name() + "' ";

    if (reset->isOrderSet()) {
        description += "with order '" + convertToString(reset->order()) + "', ";
    } else {
        noOrder = true;
    }

    // Check for a valid identifier.
    if (!isValidXmlName(reset->id())) {
        auto issue = Issue::IssueImpl::create();
        issue->mPimpl->setReferenceRule(Issue::ReferenceRule::XML_ID_ATTRIBUTE);
        issue->mPimpl->mItem->mPimpl->setReset(reset);
        issue->mPimpl->setDescription(description + "' does not have a valid 'id' attribute, '" + reset->id() + "'.");
        addIssue(issue);
    }

    if (reset->variable() == nullptr) {
        noVariable = true;
    } else {
        description += "with variable '" + reset->variable()->name() + "', ";
        auto var = reset->variable();
        auto varParent = owningComponent(var);
        varParentName = varParent->name();
        if (varParentName != component->name()) {
            varOutsideComponent = true;
        }
    }

    if (reset->testVariable() == nullptr) {
        noTestVariable = true;
    } else {
        description += "with test_variable '" + reset->testVariable()->name() + "', ";

        auto var = reset->testVariable();
        auto varParent = owningComponent(var);
        testVarParentName = varParent->name();
        if (testVarParentName != component->name()) {
            testVarOutsideComponent = true;
        }
    }

    // Check that the contents of test_value and reset_value are valid MathML
    std::string testValueString = reset->testValue();
    std::string resetValueString = reset->resetValue();

    if ((testValueString.empty()) || (std::all_of(testValueString.begin(), testValueString.end(), isspace))) {
        noTestValue = true;
    } else {
        validateMath(testValueString, component);
    }
    if ((resetValueString.empty()) || (std::all_of(resetValueString.begin(), resetValueString.end(), isspace))) {
        noResetValue = true;
    } else {
        validateMath(resetValueString, component);
    }

    // Check for a valid identifier.
    if (!isValidXmlName(reset->testValueId())) {
        auto issue = Issue::IssueImpl::create();
        issue->mPimpl->setReferenceRule(Issue::ReferenceRule::XML_ID_ATTRIBUTE);
        issue->mPimpl->mItem->mPimpl->setReset(reset);
        issue->mPimpl->setDescription(description + "' does not have a valid test_value 'id' attribute, '" + reset->testValueId() + "'.");
        addIssue(issue);
    }
    // Check for a valid identifier.
    if (!isValidXmlName(reset->resetValueId())) {
        auto issue = Issue::IssueImpl::create();
        issue->mPimpl->setReferenceRule(Issue::ReferenceRule::XML_ID_ATTRIBUTE);
        issue->mPimpl->mItem->mPimpl->setReset(reset);
        issue->mPimpl->setDescription(description + "' does not have a valid reset_value 'id' attribute, '" + reset->resetValueId() + "'.");
        addIssue(issue);
    }

    if (noOrder) {
        auto issue = Issue::IssueImpl::create();
        issue->mPimpl->setDescription(description + "does not have an order set.");
        issue->mPimpl->mItem->mPimpl->setComponent(component);
        issue->mPimpl->setReferenceRule(Issue::ReferenceRule::RESET_ORDER_VALUE);
        addIssue(issue);
    }
    if (noVariable) {
        auto issue = Issue::IssueImpl::create();
        issue->mPimpl->setDescription(description + "does not reference a variable.");
        issue->mPimpl->mItem->mPimpl->setReset(reset);
        issue->mPimpl->setReferenceRule(Issue::ReferenceRule::RESET_VARIABLE_REFERENCE);
        addIssue(issue);
    }
    if (noTestVariable) {
        auto issue = Issue::IssueImpl::create();
        issue->mPimpl->setDescription(description + "does not reference a test_variable.");
        issue->mPimpl->mItem->mPimpl->setReset(reset);
        issue->mPimpl->setReferenceRule(Issue::ReferenceRule::RESET_TEST_VARIABLE_REFERENCE);
        addIssue(issue);
    }
    if (noTestValue) {
        auto issue = Issue::IssueImpl::create();
        issue->mPimpl->setDescription(description + "does not have a test_value specified.");
        issue->mPimpl->mItem->mPimpl->setReset(reset);
        issue->mPimpl->setReferenceRule(Issue::ReferenceRule::TEST_VALUE_ELEMENT);
        addIssue(issue);
    }
    if (noResetValue) {
        auto issue = Issue::IssueImpl::create();
        issue->mPimpl->setDescription(description + "does not have a reset_value specified.");
        issue->mPimpl->mItem->mPimpl->setReset(reset);
        issue->mPimpl->setReferenceRule(Issue::ReferenceRule::RESET_VALUE_ELEMENT);
        addIssue(issue);
    }
    if (varOutsideComponent) {
        auto issue = Issue::IssueImpl::create();
        issue->mPimpl->setDescription(description + "refers to a variable '" + reset->variable()->name() + "' in a different component '" + varParentName + "'.");
        issue->mPimpl->mItem->mPimpl->setReset(reset);
        issue->mPimpl->setReferenceRule(Issue::ReferenceRule::RESET_VARIABLE_REFERENCE);
        addIssue(issue);
    }
    if (testVarOutsideComponent) {
        auto issue = Issue::IssueImpl::create();
        issue->mPimpl->setDescription(description + "refers to a test_variable '" + reset->testVariable()->name() + "' in a different component '" + testVarParentName + "'.");
        issue->mPimpl->mItem->mPimpl->setReset(reset);
        issue->mPimpl->setReferenceRule(Issue::ReferenceRule::RESET_TEST_VARIABLE_REFERENCE);
        addIssue(issue);
    }
}

void Validator::ValidatorImpl::validateMath(const std::string &input, const ComponentPtr &component)
{
    // Parse as XML first.
    std::vector<XmlDocPtr> docs = multiRootXml(input);
    for (const auto &doc : docs) {
        // Copy any XML parsing issues into the common validator issue handler.
        if (doc->xmlErrorCount() > 0) {
            for (size_t i = 0; i < doc->xmlErrorCount(); ++i) {
                auto issue = Issue::IssueImpl::create();
                issue->mPimpl->setDescription("LibXml2 error: " + doc->xmlError(i));
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::XML);
                addIssue(issue);
            }
        }
        XmlNodePtr node = doc->rootNode();
        if (node == nullptr) {
            auto issue = Issue::IssueImpl::create();
            issue->mPimpl->setDescription("Could not get a valid XML root node from the math on component '" + component->name() + "'.");
            issue->mPimpl->mItem->mPimpl->setComponent(component);
            issue->mPimpl->setReferenceRule(Issue::ReferenceRule::XML);
            addIssue(issue);
            return;
        }
        if (!node->isMathmlElement("math")) {
            auto issue = Issue::IssueImpl::create();
            issue->mPimpl->setDescription("Math root node is of invalid type '" + node->name() + "' on component '" + component->name() + "'. A valid math root node should be of type 'math'.");
            issue->mPimpl->mItem->mPimpl->setComponent(component);
            issue->mPimpl->setReferenceRule(Issue::ReferenceRule::MATH_ELEMENT);
            addIssue(issue);
            return;
        }

        XmlNodePtr nodeCopy = node;
        NameList variableNames;
        for (size_t i = 0; i < component->variableCount(); ++i) {
            std::string variableName = component->variable(i)->name();
            if (std::find(variableNames.begin(), variableNames.end(), variableName) == variableNames.end()) {
                variableNames.push_back(variableName);
            }
        }

        validateMathMLElements(nodeCopy, component);

        // Iterate through ci/cn elements and remove cellml units attributes.
        XmlNodePtr mathNode = node;
        validateAndCleanMathCiCnNodes(node, component, variableNames);

        // Remove the cellml namespace definition.
        if (mathNode->hasNamespaceDefinition(CELLML_2_0_NS)) {
            mathNode->removeNamespaceDefinition(CELLML_2_0_NS);
        }

        // Get the MathML string with cellml:units attributes and namespace already removed.
        std::string cleanMathml = mathNode->convertToString();

        // Parse/validate the clean math string with the W3C MathML DTD.
        XmlDocPtr mathmlDoc = std::make_shared<XmlDoc>();
        mathmlDoc->parseMathML(cleanMathml);
        // Copy any MathML validation errors into the common validator error handler.
        if (mathmlDoc->xmlErrorCount() > 0) {
            for (size_t i = 0; i < mathmlDoc->xmlErrorCount(); ++i) {
                auto issue = Issue::IssueImpl::create();
                issue->mPimpl->setDescription("W3C MathML DTD error: " + mathmlDoc->xmlError(i));
                issue->mPimpl->mItem->mPimpl->setMath(component);
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::MATH_MATHML);
                addIssue(issue);
            }
        }

        // Make sure that the different MathML elements for the right number of
        // children/siblings, type, etc.

        mathNode = mathmlDoc->rootNode();

        auto childCount = mathmlChildCount(mathNode);

        for (size_t i = 0; i < childCount; ++i) {
            validateMathMLElementsChildrenAndSiblings(mathmlChildNode(mathNode, i), component);
        }
    }
}

bool Validator::ValidatorImpl::validateCnUnits(const ComponentPtr &component, const std::string &unitsName, const std::string &textNode)
{
    if (isCellmlIdentifier(unitsName)) {
        return true;
    }

    IssuePtr issue = makeIssueIllegalIdentifier(unitsName);
    issue->mPimpl->setDescription("Math cn element with the value '" + textNode + "' does not have a valid cellml:units attribute. " + issue->description());
    issue->mPimpl->mItem->mPimpl->setMath(component);
    issue->mPimpl->setReferenceRule(Issue::ReferenceRule::MATH_CN_UNITS_ATTRIBUTE);
    addIssue(issue);

    return false;
}

std::string text(const XmlNodePtr &node)
{
    if (node != nullptr) {
        if (node->isText()) {
            return node->convertToStrippedString();
        }
    }
    return {};
}

void Validator::ValidatorImpl::validateAndCleanCnNode(const XmlNodePtr &node, const ComponentPtr &component)
{
    // Get cellml:units attribute.
    XmlAttributePtr attribute = node->firstAttribute();
    std::string unitsName;
    XmlAttributePtr unitsAttribute = nullptr;
    std::vector<XmlAttributePtr> cellmlAttributesToRemove;
    while (attribute != nullptr) {
        if (!attribute->value().empty()) {
            if (attribute->isCellmlType("units")) {
                unitsName = attribute->value();
                unitsAttribute = attribute;
                cellmlAttributesToRemove.push_back(attribute);
            } else if (attribute->inNamespaceUri(CELLML_2_0_NS)) {
                cellmlAttributesToRemove.push_back(attribute);
                auto issue = Issue::IssueImpl::create();
                issue->mPimpl->setDescription("Math " + node->name() + " element has an invalid attribute type '" + attribute->name() + "' in the cellml namespace. Attribute 'units' is the only CellML namespace attribute allowed.");
                issue->mPimpl->mItem->mPimpl->setMath(component);
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::MATH_MATHML);
                addIssue(issue);
            }
        }
        attribute = attribute->next();
    }

    XmlNodePtr childNode = node->firstChild();
    std::string textInNode = text(childNode);
    // Check that cellml:units has been set.
    bool checkUnitsIsInModel = validateCnUnits(component, unitsName, textInNode);

    // Check that a specified units is valid.
    if (checkUnitsIsInModel) {
        // Check for a matching units in this model.
        ModelPtr model = owningModel(component);
        if (!model->hasUnits(unitsName)) {
            // Check for a matching standard units.
            if (!isStandardUnitName(unitsName)) {
                auto issue = Issue::IssueImpl::create();
                issue->mPimpl->setDescription("Math has a " + node->name() + " element with a cellml:units attribute '" + unitsName + "' that is not a valid reference to units in the model '" + model->name() + "' or a standard unit.");
                issue->mPimpl->mItem->mPimpl->setMath(component);
                issue->mPimpl->setReferenceRule(Issue::ReferenceRule::MATH_CN_UNITS_ATTRIBUTE_REFERENCE);
                addIssue(issue);
            }
        }
    }
    // Now that we've validated this XML node's cellml:units attribute, remove it from the node.
    // This is done so we can validate a "clean" MathML string using the MathML DTD. The math
    // string stored on the component will not be affected.
    for (const auto &cellmlAttribute : cellmlAttributesToRemove) {
        cellmlAttribute->removeAttribute();
    }
    if (node->hasNamespaceDefinition(CELLML_2_0_NS)) {
        node->removeNamespaceDefinition(CELLML_2_0_NS);
    }
}

void Validator::ValidatorImpl::validateAndCleanCiNode(const XmlNodePtr &node, const ComponentPtr &component, const NameList &variableNames)
{
    XmlNodePtr childNode = node->firstChild();
    std::string textInNode = text(childNode);
    if (!textInNode.empty()) {
        // Check whether we can find this text as a variable name in this component.
        if (std::find(variableNames.begin(), variableNames.end(), textInNode) == variableNames.end()) {
            auto issue = Issue::IssueImpl::create();
            issue->mPimpl->setDescription("MathML ci element has the child text '" + textInNode + "' which does not correspond with any variable names present in component '" + component->name() + "'.");
            issue->mPimpl->mItem->mPimpl->setMath(component);
            issue->mPimpl->setReferenceRule(Issue::ReferenceRule::MATH_CI_VARIABLE_REFERENCE);
            addIssue(issue);
        }
    }
}

void Validator::ValidatorImpl::validateAndCleanMathCiCnNodes(XmlNodePtr &node, const ComponentPtr &component, const NameList &variableNames)
{
    if (node->isMathmlElement("cn")) {
        validateAndCleanCnNode(node, component);
    } else if (node->isMathmlElement("ci")) {
        validateAndCleanCiNode(node, component, variableNames);
    }
    // Check children for ci/cn.
    XmlNodePtr childNode = node->firstChild();
    if (childNode != nullptr) {
        validateAndCleanMathCiCnNodes(childNode, component, variableNames);
    }
    // Check siblings for ci/cn.
    node = node->next();
    if (node != nullptr) {
        validateAndCleanMathCiCnNodes(node, component, variableNames);
    }
}

void Validator::ValidatorImpl::validateMathMLElement(const XmlNodePtr &node, const ComponentPtr &component)
{
    if (node != nullptr) {
        if (!node->isComment() && !node->isText() && !isSupportedMathMLElement(node)) {
            auto issue = Issue::IssueImpl::create();
            issue->mPimpl->setDescription("Math has a '" + node->name() + "' element that is not a supported MathML element.");
            issue->mPimpl->mItem->mPimpl->setMath(component);
            issue->mPimpl->setReferenceRule(Issue::ReferenceRule::MATH_CHILD);
            addIssue(issue);
        }
        validateMathMLElements(node, component);
    }
}

void Validator::ValidatorImpl::validateMathMLElements(const XmlNodePtr &node, const ComponentPtr &component)
{
    validateMathMLElement(node->firstChild(), component);
    validateMathMLElement(node->next(), component);
}

void Validator::ValidatorImpl::addMathmlIssue(const std::string &description,
                                              Issue::ReferenceRule referenceRule,
                                              const ComponentPtr &component)
{
    auto issue = Issue::IssueImpl::create();

    issue->mPimpl->setDescription(description);
    issue->mPimpl->mItem->mPimpl->setMath(component);
    issue->mPimpl->setReferenceRule(referenceRule);

    addIssue(issue);
}

bool Validator::ValidatorImpl::hasOneMathmlSibling(const XmlNodePtr &parentNode,
                                                   const XmlNodePtr &node,
                                                   const ComponentPtr &component)
{
    if (mathmlChildCount(parentNode) != 2) {
        addMathmlIssue("Math has a '" + node->name() + "' element without exactly one MathML sibling.",
                       Issue::ReferenceRule::MATH_MATHML,
                       component);

        return false;
    }

    return true;
}

bool Validator::ValidatorImpl::hasAtLeastOneMathmlSibling(const XmlNodePtr &parentNode,
                                                          const XmlNodePtr &node,
                                                          const ComponentPtr &component)
{
    if (mathmlChildCount(parentNode) < 2) {
        addMathmlIssue("Math has a '" + node->name() + "' element without at least one MathML sibling.",
                       Issue::ReferenceRule::MATH_MATHML,
                       component);

        return false;
    }

    return true;
}

bool Validator::ValidatorImpl::hasTwoMathmlSiblings(const XmlNodePtr &parentNode,
                                                    const XmlNodePtr &node,
                                                    const ComponentPtr &component)
{
    if (mathmlChildCount(parentNode) != 3) {
        addMathmlIssue("Math has a '" + node->name() + "' element without exactly two MathML siblings.",
                       Issue::ReferenceRule::MATH_MATHML,
                       component);

        return false;
    }

    return true;
}

bool Validator::ValidatorImpl::hasAtLeastTwoMathmlSiblings(const XmlNodePtr &parentNode,
                                                           const XmlNodePtr &node,
                                                           const ComponentPtr &component)
{
    if (mathmlChildCount(parentNode) < 3) {
        addMathmlIssue("Math has a '" + node->name() + "' element without at least two MathML siblings.",
                       Issue::ReferenceRule::MATH_MATHML,
                       component);

        return false;
    }

    return true;
}

size_t Validator::ValidatorImpl::hasOneOrTwoMathmlSiblings(const XmlNodePtr &parentNode,
                                                           const XmlNodePtr &node,
                                                           const ComponentPtr &component)
{
    auto childCount = mathmlChildCount(parentNode);

    if ((childCount != 2) && (childCount != 3)) {
        addMathmlIssue("Math has a '" + node->name() + "' element without exactly one or two MathML siblings.",
                       Issue::ReferenceRule::MATH_MATHML,
                       component);

        return 0;
    }

    return childCount - 1;
}

bool Validator::ValidatorImpl::isFirstMathmlSibling(const XmlNodePtr &parentNode,
                                                    const XmlNodePtr &node,
                                                    const ComponentPtr &component)
{
    if (!mathmlChildNode(parentNode, 0)->equals(node)) {
        addMathmlIssue("Math has a '" + node->name() + "' element which is not the first MathML sibling.",
                       Issue::ReferenceRule::MATH_MATHML,
                       component);

        return false;
    }

    return true;
}

bool Validator::ValidatorImpl::isSecondMathmlSibling(const XmlNodePtr &parentNode,
                                                     const XmlNodePtr &node,
                                                     const ComponentPtr &component)
{
    if (!mathmlChildNode(parentNode, 1)->equals(node)) {
        addMathmlIssue("Math has a '" + node->name() + "' element which is not the second MathML sibling.",
                       Issue::ReferenceRule::MATH_MATHML,
                       component);

        return false;
    }

    return true;
}

bool Validator::ValidatorImpl::hasFirstMathmlSiblingWithName(const XmlNodePtr &parentNode,
                                                             const XmlNodePtr &node,
                                                             const std::string &name,
                                                             const ComponentPtr &component)
{
    auto childNode = mathmlChildNode(parentNode, 0);

    if (childNode->equals(node)) {
        childNode = mathmlChildNode(parentNode, 1);
    }

    if (childNode->name() != name) {
        addMathmlIssue("Math has a '" + node->name() + "' element which first sibling is not a '" + name + "' element.",
                       Issue::ReferenceRule::MATH_MATHML,
                       component);

        return false;
    }

    return true;
}

bool Validator::ValidatorImpl::hasOneMathmlChild(const XmlNodePtr &node,
                                                 const ComponentPtr &component)
{
    if (mathmlChildCount(node) != 1) {
        addMathmlIssue("Math has a '" + node->name() + "' element without exactly one MathML child.",
                       Issue::ReferenceRule::MATH_MATHML,
                       component);

        return false;
    }

    return true;
}

bool Validator::ValidatorImpl::hasAtLeastOneMathmlChild(const XmlNodePtr &node,
                                                        const ComponentPtr &component)
{
    if (mathmlChildCount(node) < 1) {
        addMathmlIssue("Math has a '" + node->name() + "' element without at least one MathML child.",
                       Issue::ReferenceRule::MATH_MATHML,
                       component);

        return false;
    }

    return true;
}

bool Validator::ValidatorImpl::hasTwoMathmlChildren(const XmlNodePtr &node,
                                                    const ComponentPtr &component)
{
    if (mathmlChildCount(node) != 2) {
        addMathmlIssue("Math has a '" + node->name() + "' element without exactly two MathML children.",
                       Issue::ReferenceRule::MATH_MATHML,
                       component);

        return false;
    }

    return true;
}

bool Validator::ValidatorImpl::hasOneOrTwoMathmlChildren(const XmlNodePtr &node,
                                                         const ComponentPtr &component)
{
    auto childCount = mathmlChildCount(node);

    if ((childCount != 1) && (childCount != 2)) {
        addMathmlIssue("Math has a '" + node->name() + "' element without exactly one or two MathML children.",
                       Issue::ReferenceRule::MATH_MATHML,
                       component);

        return false;
    }

    return true;
}

void Validator::ValidatorImpl::validateMathMLElementsChildrenAndSiblings(const XmlNodePtr &node,
                                                                         const ComponentPtr &component)
{
    // Check the current node against the MathML elements listed in
    // supportedMathMLElements.
    // Note: we don't have to do anything for 'true', 'false, 'exponentiale',
    //       'pi', 'infinity', and 'notanumber' since they must be empty and if
    //       they are not then this will be caught by the DTD.

    // Basic content elements.

    if (node->isMathmlElement("apply")) {
        if (hasAtLeastOneMathmlChild(node, component)) {
            for (size_t i = 0, iMax = mathmlChildCount(node); i < iMax; ++i) {
                validateMathMLElementsChildrenAndSiblings(mathmlChildNode(node, i), component);
            }
        }

        // Relational and logical operators.

    } else if (node->isMathmlElement("eq")
               || node->isMathmlElement("neq")
               || node->isMathmlElement("lt")
               || node->isMathmlElement("leq")
               || node->isMathmlElement("gt")
               || node->isMathmlElement("geq")) {
        auto parentNode = node->parent();

        hasTwoMathmlSiblings(parentNode, node, component)
            && isFirstMathmlSibling(parentNode, node, component);
    } else if (node->isMathmlElement("and")
               || node->isMathmlElement("or")
               || node->isMathmlElement("xor")) {
        auto parentNode = node->parent();

        hasAtLeastTwoMathmlSiblings(parentNode, node, component)
            && isFirstMathmlSibling(parentNode, node, component);
    } else if (node->isMathmlElement("not")) {
        auto parentNode = node->parent();

        hasOneMathmlSibling(parentNode, node, component)
            && isFirstMathmlSibling(parentNode, node, component);

        // Arithmetic operators.

    } else if (node->isMathmlElement("plus")) {
        auto parentNode = node->parent();

        hasAtLeastOneMathmlSibling(parentNode, node, component)
            && isFirstMathmlSibling(parentNode, node, component);
    } else if (node->isMathmlElement("minus")) {
        auto parentNode = node->parent();

        hasOneOrTwoMathmlSiblings(parentNode, node, component)
            && isFirstMathmlSibling(parentNode, node, component);
    } else if (node->isMathmlElement("times")) {
        auto parentNode = node->parent();

        hasAtLeastTwoMathmlSiblings(parentNode, node, component)
            && isFirstMathmlSibling(parentNode, node, component);
    } else if (node->isMathmlElement("divide")) {
        auto parentNode = node->parent();

        hasTwoMathmlSiblings(parentNode, node, component)
            && isFirstMathmlSibling(parentNode, node, component);
    } else if (node->isMathmlElement("power")) {
        auto parentNode = node->parent();

        hasTwoMathmlSiblings(parentNode, node, component)
            && isFirstMathmlSibling(parentNode, node, component);
    } else if (node->isMathmlElement("root")) {
        // A 'root' element can have either one or two siblings, depending on
        // whether a 'degree' element is specified, e.g.
        //
        //   <apply>
        //     <root/>
        //     <ci>a</ci>
        //   </apply>
        //
        // and
        //
        //   <apply>
        //     <root/>
        //     <degree>
        //       <cn cellml:units="dimensionless">3</cn>
        //     </degree>
        //     <ci>a</ci>
        //   </apply>

        auto parentNode = node->parent();
        auto siblingCount = hasOneOrTwoMathmlSiblings(parentNode, node, component);

        if ((siblingCount != 0)
            && isFirstMathmlSibling(parentNode, node, component)) {
            (siblingCount == 2)
                && hasFirstMathmlSiblingWithName(parentNode, node, "degree", component);
        }
    } else if (node->isMathmlElement("abs")
               || node->isMathmlElement("exp")
               || node->isMathmlElement("ln")) {
        auto parentNode = node->parent();

        hasOneMathmlSibling(parentNode, node, component)
            && isFirstMathmlSibling(parentNode, node, component);
    } else if (node->isMathmlElement("log")) {
        // A 'log' element can have either one or two siblings, depending on
        // whether a 'logbase' element is specified, e.g.
        //
        //   <apply>
        //     <log/>
        //     <ci>a</ci>
        //   </apply>
        //
        // and
        //
        //   <apply>
        //     <log/>
        //     <logbase>
        //       <cn cellml:units="dimensionless">3</cn>
        //     </logbase>
        //     <ci>a</ci>
        //   </apply>

        auto parentNode = node->parent();
        auto siblingCount = hasOneOrTwoMathmlSiblings(parentNode, node, component);

        if ((siblingCount != 0)
            && isFirstMathmlSibling(parentNode, node, component)) {
            (siblingCount == 2)
                && hasFirstMathmlSiblingWithName(parentNode, node, "logbase", component);
        }
    } else if (node->isMathmlElement("ceiling")
               || node->isMathmlElement("floor")) {
        auto parentNode = node->parent();

        hasOneMathmlSibling(parentNode, node, component)
            && isFirstMathmlSibling(parentNode, node, component);
    } else if (node->isMathmlElement("min")) {
    } else if (node->isMathmlElement("max")) {
    } else if (node->isMathmlElement("rem")) {
        // Calculus elements.

    } else if (node->isMathmlElement("diff")) {
        auto parentNode = node->parent();

        hasTwoMathmlSiblings(parentNode, node, component)
            && isFirstMathmlSibling(parentNode, node, component)
            && hasFirstMathmlSiblingWithName(parentNode, node, "bvar", component);

        // Trigonometric operators.

    } else if (node->isMathmlElement("sin")
               || node->isMathmlElement("cos")
               || node->isMathmlElement("tan")
               || node->isMathmlElement("sec")
               || node->isMathmlElement("csc")
               || node->isMathmlElement("cot")
               || node->isMathmlElement("sinh")
               || node->isMathmlElement("cosh")
               || node->isMathmlElement("tanh")
               || node->isMathmlElement("sech")
               || node->isMathmlElement("csch")
               || node->isMathmlElement("coth")
               || node->isMathmlElement("arcsin")
               || node->isMathmlElement("arccos")
               || node->isMathmlElement("arctan")
               || node->isMathmlElement("arcsec")
               || node->isMathmlElement("arccsc")
               || node->isMathmlElement("arccot")
               || node->isMathmlElement("arcsinh")
               || node->isMathmlElement("arccosh")
               || node->isMathmlElement("arctanh")
               || node->isMathmlElement("arcsech")
               || node->isMathmlElement("arccsch")
               || node->isMathmlElement("arccoth")) {
        auto parentNode = node->parent();

        hasOneMathmlSibling(parentNode, node, component)
            && isFirstMathmlSibling(parentNode, node, component);

        // Piecewise statement.

    } else if (node->isMathmlElement("piecewise")) {
        for (size_t i = 0, iMax = mathmlChildCount(node); i < iMax; ++i) {
            validateMathMLElementsChildrenAndSiblings(mathmlChildNode(node, i), component);
        }
    } else if (node->isMathmlElement("piece")) {
        if (hasTwoMathmlChildren(node, component)) {
            validateMathMLElementsChildrenAndSiblings(mathmlChildNode(node, 0), component);
            validateMathMLElementsChildrenAndSiblings(mathmlChildNode(node, 1), component);
        }
    } else if (node->isMathmlElement("otherwise")) {
        if (hasOneMathmlChild(node, component)) {
            validateMathMLElementsChildrenAndSiblings(mathmlChildNode(node, 0), component);
        }

        // Token elements.

    } else if (node->isMathmlElement("ci")) {
        auto ok = (nonCommentChildCount(node) != 1) ? false : !nonCommentChildNode(node, 0)->convertToStrippedString().empty();

        if (!ok) {
            addMathmlIssue("Math has a 'ci' element with no identifier as a child.",
                           Issue::ReferenceRule::MATH_CI_VARIABLE_REFERENCE,
                           component);
        }
    } else if (node->isMathmlElement("cn")) {
        auto cnBase = node->attribute("base");

        if (!cnBase.empty() && (cnBase != "10")) {
            addMathmlIssue("Math has a 'cn' element which is not in base 10.",
                           Issue::ReferenceRule::MATH_CN_BASE10,
                           component);

            return;
        }

        auto cnType = node->attribute("type");

        if (cnType.empty() || (cnType == "real")) {
            auto ok = (nonCommentChildCount(node) != 1) ? false : nonCommentChildNode(node, 0)->isBasicReal();

            if (!ok) {
                addMathmlIssue("Math has a 'cn' element of 'real' type with no valid text node (representing a basic number) as a child.",
                               Issue::ReferenceRule::MATH_CN_FORMAT,
                               component);
            }
        } else if (cnType == "e-notation") {
            auto ok = false;

            if (nonCommentChildCount(node) == 3) {
                ok = nonCommentChildNode(node, 0)->isBasicReal()
                     && nonCommentChildNode(node, 1)->isMathmlElement("sep")
                     && nonCommentChildNode(node, 2)->isInteger();
            }

            if (!ok) {
                addMathmlIssue("Math has a 'cn' element of 'e-notation' type with no valid text node (representing a basic number), no 'sep' element, and/or no valid text node (representing an integer) as children.",
                               Issue::ReferenceRule::MATH_CN_FORMAT,
                               component);
            }
        } else {
            addMathmlIssue("Math has a 'cn' element which is not of 'real' or 'e-notation' type.",
                           Issue::ReferenceRule::MATH_CN_FORMAT,
                           component);
        }

        // Qualifier elements.

    } else if (node->isMathmlElement("degree")) {
        // A 'degree' element can be used either with a 'root' element or within
        // a 'bvar' element, e.g.
        //
        //   <apply>
        //     <diff/>
        //     <bvar>
        //       <ci>t</ci>
        //       <degree>
        //         <cn cellml:units="dimensionless">2</cn>
        //       </degree>
        //     </bvar>
        //     <ci>x</ci>
        //   </apply>
        //
        // and
        //
        //   <apply>
        //     <root/>
        //     <degree>
        //       <cn cellml:units="dimensionless">3</cn>
        //     </degree>
        //     <ci>a</ci>
        //   </apply>

        auto parentNode = node->parent();
        auto siblingCount = hasOneOrTwoMathmlSiblings(parentNode, node, component);

        if (siblingCount == 1) {
            // Used with a 'degree' element.

            isSecondMathmlSibling(parentNode, node, component)
                && hasOneMathmlChild(node, component);
        } else if (siblingCount == 2) {
            // Used with a 'root' element.

            hasFirstMathmlSiblingWithName(parentNode, node, "root", component)
                && isSecondMathmlSibling(parentNode, node, component)
                && hasOneMathmlChild(node, component);
        }
    } else if (node->isMathmlElement("logbase")) {
        auto parentNode = node->parent();

        hasTwoMathmlSiblings(parentNode, node, component)
            && hasFirstMathmlSiblingWithName(parentNode, node, "log", component)
            && isSecondMathmlSibling(parentNode, node, component)
            && hasOneMathmlChild(node, component);
    } else if (node->isMathmlElement("bvar")) {
        // A 'bvar' element can have one or two children, e.g.
        //
        //   <apply>
        //     <diff/>
        //     <bvar>
        //       <ci>t</ci>
        //     </bvar>
        //     <ci>x</ci>
        //   </apply>
        //
        // and
        //
        //   <apply>
        //     <diff/>
        //     <bvar>
        //       <ci>t</ci>
        //       <degree>
        //         <cn cellml:units="dimensionless">2</cn>
        //       </degree>
        //     </bvar>
        //     <ci>x</ci>
        //   </apply>

        auto parentNode = node->parent();

        hasTwoMathmlSiblings(parentNode, node, component)
            && hasFirstMathmlSiblingWithName(parentNode, node, "diff", component)
            && isSecondMathmlSibling(parentNode, node, component)
            && hasOneOrTwoMathmlChildren(node, component);
    }
}

/**
 * @brief Test to see if the given variables are reachable from their parent components.
 *
 * Determine if the variables parents are reachable from one another.  That is they either
 * have a parent/child relationship or a sibling relationship in the model's component
 * hierarchy.
 *
 * Both variables must have a valid parent.
 *
 * @param variable1 The first variable.
 * @param variable2 The second variable.
 *
 * @return @c true if the parents of the given variables are reachable in the model component
 * hierarchy, @c false otherwise.
 */
bool reachableEquivalence(const VariablePtr &variable1, const VariablePtr &variable2)
{
    auto parent1 = variable1->parent();
    auto parent2 = variable2->parent();

    return isEntityChildOf(parent1, parent2)
           || isEntityChildOf(parent2, parent1)
           || areEntitiesSiblings(parent1, parent2);
}

bool interfaceTypeIsCompatible(Variable::InterfaceType interfaceTypeMinimumRequired, const std::string &interfaceTypeCompatibleWith)
{
    return interfaceTypeCompatibleWith.find(interfaceTypeToString.at(interfaceTypeMinimumRequired)) != std::string::npos;
}

void Validator::ValidatorImpl::validateVariableInterface(const VariablePtr &variable, VariableMap &alreadyReported)
{
    Variable::InterfaceType interfaceType = determineInterfaceType(variable);
    auto component = owningComponent(variable);
    std::string componentName = component->name();
    if (interfaceType == Variable::InterfaceType::NONE) {
        for (size_t index = 0; index < variable->equivalentVariableCount(); ++index) {
            const auto equivalentVariable = variable->equivalentVariable(index);
            auto equivalentComponent = owningComponent(equivalentVariable);
            if (equivalentComponent != nullptr && !reachableEquivalence(variable, equivalentVariable)) {
                auto it = std::find_if(alreadyReported.begin(), alreadyReported.end(),
                                       [equivalentVariable, variable](const VariablePairPtr &in) {
                                           return (in->variable1() == equivalentVariable) && (in->variable2() == variable);
                                       });
                if (it == alreadyReported.end()) {
                    VariablePairPtr pair = VariablePair::create(variable, equivalentVariable);
                    alreadyReported.push_back(pair);
                    std::string equivalentComponentName = equivalentComponent->name();

                    IssuePtr err = Issue::IssueImpl::create();
                    err->mPimpl->setDescription("The equivalence between '" + variable->name() + "' in component '" + componentName + "'  and '" + equivalentVariable->name() + "' in component '" + equivalentComponentName + "' is invalid. Component '" + componentName + "' and '" + equivalentComponentName + "' are neither siblings nor in a parent/child relationship.");
                    err->mPimpl->mItem->mPimpl->setMapVariables(variable, equivalentVariable);
                    err->mPimpl->setReferenceRule(Issue::ReferenceRule::MAP_VARIABLES_ELEMENT);
                    addIssue(err);
                }
            }
        }
    } else {
        auto interfaceTypeString = variable->interfaceType();
        if (!interfaceTypeIsCompatible(interfaceType, interfaceTypeString)) {
            IssuePtr err = Issue::IssueImpl::create();
            if (interfaceTypeString.empty()) {
                err->mPimpl->setDescription("Variable '" + variable->name() + "' in component '" + componentName + "' has no interface type set. The interface type required is '" + interfaceTypeToString.find(interfaceType)->second + "'.");
            } else {
                err->mPimpl->setDescription("Variable '" + variable->name() + "' in component '" + componentName + "' has an interface type set to '" + interfaceTypeString + "' which is not the correct interface type for this variable. The interface type required is '" + interfaceTypeToString.find(interfaceType)->second + "'.");
            }
            err->mPimpl->mItem->mPimpl->setVariable(variable);
            err->mPimpl->setReferenceRule(Issue::ReferenceRule::MAP_VARIABLES_ELEMENT);
            addIssue(err);
        }
    }
}

void Validator::ValidatorImpl::validateEquivalenceUnits(const ModelPtr &model, const VariablePtr &variable, VariableMap &alreadyReported)
{
    std::string hints;
    ComponentPtr parentComponent = owningComponent(variable);

    if (variable->units() == nullptr) {
        // If the variable has no units, don't check it.
        return;
    }

    for (size_t index = 0; index < variable->equivalentVariableCount(); ++index) {
        auto equivalentVariable = variable->equivalentVariable(index);

        // If the parent component of the variable is nonexistent or imported, don't check it.
        auto equivalentComponent = owningComponent(equivalentVariable);
        if ((equivalentComponent == nullptr) || equivalentComponent->isImport()) {
            continue;
        }

        // If the equivalent variable has no units, don't check it.
        if (equivalentVariable->units() == nullptr) {
            continue;
        }

        double multiplier = 0.0;
        if (!unitsAreEquivalent(model, variable, equivalentVariable, hints, multiplier)) {
            auto it = std::find_if(alreadyReported.begin(), alreadyReported.end(),
                                   [equivalentVariable, variable](const VariablePairPtr &in) {
                                       return (in->variable1() == equivalentVariable) && (in->variable2() == variable);
                                   });
            if (it == alreadyReported.end()) {
                VariablePairPtr pair = VariablePair::create(variable, equivalentVariable);
                alreadyReported.push_back(pair);
                IssuePtr err = Issue::IssueImpl::create();
                err->mPimpl->setDescription("Variable '" + variable->name() + "' in component '" + parentComponent->name() + "' has units of '" + variable->units()->name() + "' and an equivalent variable '" + equivalentVariable->name() + "' in component '" + equivalentComponent->name() + "' with non-matching units of '" + equivalentVariable->units()->name() + "'. The mismatch is: " + hints);
                err->mPimpl->mItem->mPimpl->setMapVariables(variable, equivalentVariable);
                err->mPimpl->setReferenceRule(Issue::ReferenceRule::MAP_VARIABLES_ELEMENT);
                addIssue(err);
            }
        }
    }
}

void Validator::ValidatorImpl::validateEquivalenceStructure(const VariablePtr &variable)
{
    for (size_t index = 0; index < variable->equivalentVariableCount(); ++index) {
        auto equivalentVariable = variable->equivalentVariable(index);
        auto component = owningComponent(equivalentVariable);
        if (component == nullptr) {
            IssuePtr err = Issue::IssueImpl::create();
            err->mPimpl->setDescription("Variable '" + equivalentVariable->name() + "' is an equivalent variable to '" + variable->name() + "' but '" + equivalentVariable->name() + "' has no parent component.");
            err->mPimpl->mItem->mPimpl->setMapVariables(variable, equivalentVariable);
            err->mPimpl->setReferenceRule(Issue::ReferenceRule::MAP_VARIABLES_VARIABLE1_ATTRIBUTE);
            addIssue(err);
        }
    }
}

void Validator::ValidatorImpl::validateConnections(const ModelPtr &model)
{
    VariableMap interfaceErrorsAlreadyReported;
    VariableMap equivalentUnitErrorsAlreadyReported;

    VariablePtrs variables;

    for (size_t index = 0; index < model->componentCount(); ++index) {
        findAllVariablesWithEquivalences(model->component(index), variables);
    }

    for (const VariablePtr &variable : variables) {
        auto parentComponent = owningComponent(variable);
        if (parentComponent->isImport()) {
            continue;
        }
        validateVariableInterface(variable, interfaceErrorsAlreadyReported);
        validateEquivalenceUnits(model, variable, equivalentUnitErrorsAlreadyReported);
        validateEquivalenceStructure(variable);
    }
}

bool Validator::ValidatorImpl::isSupportedMathMLElement(const XmlNodePtr &node) const
{
    return (node->namespaceUri() == MATHML_NS)
           && std::find(supportedMathMLElements.begin(), supportedMathMLElements.end(), node->name()) != supportedMathMLElements.end();
}

IssuePtr Validator::ValidatorImpl::makeIssueIllegalIdentifier(const std::string &name) const
{
    auto issue = Issue::IssueImpl::create();
    auto referenceRule = validateCellmlIdentifier(name);
    issue->mPimpl->setReferenceRule(Issue::ReferenceRule::XML_ID_ATTRIBUTE);

    if (referenceRule == Issue::ReferenceRule::DATA_REPR_IDENTIFIER_AT_LEAST_ONE_ALPHANUM) {
        // One or more alphabetic characters.
        issue->mPimpl->setDescription("CellML identifiers must contain one or more basic Latin alphabetic characters.");
    } else if (referenceRule == Issue::ReferenceRule::DATA_REPR_IDENTIFIER_BEGIN_EURO_NUM) {
        // Does not start with numeric character.
        issue->mPimpl->setDescription("CellML identifiers must not begin with a European numeric character [0-9].");
    } else { /* referenceRule == Issue::ReferenceRule::DATA_REPR_IDENTIFIER_LATIN_ALPHANUM */
        // Basic Latin alphanumeric characters and underscores.
        issue->mPimpl->setDescription("CellML identifiers must not contain any characters other than [a-zA-Z0-9_].");
    }

    return issue;
}

bool unitsAreEquivalent(const ModelPtr &model,
                        const VariablePtr &v1,
                        const VariablePtr &v2,
                        std::string &hints,
                        double &multiplier)
{
    std::map<std::string, double> unitMap = {};

    for (const auto &baseUnits : baseUnitsList) {
        unitMap.emplace(baseUnits, 0.0);
    }

    hints = "";
    multiplier = 0.0;

    std::string v1UnitsName = v1->units()->name();
    if (model->hasUnits(v1UnitsName)) {
        UnitsPtr u1 = Units::create();
        u1 = model->units(v1UnitsName);
        updateBaseUnitCount(model, unitMap, multiplier, u1->name(), 1, 0, 1);
    } else if (unitMap.find(v1UnitsName) != unitMap.end()) {
        unitMap.at(v1UnitsName) += 1.0;
    } else if (isStandardUnitName(v1UnitsName)) {
        updateBaseUnitCount(model, unitMap, multiplier, v1UnitsName, 1, 0, 1);
    }

    std::string v2UnitsName = v2->units()->name();
    if (model->hasUnits(v2UnitsName)) {
        UnitsPtr u2 = Units::create();
        u2 = model->units(v2UnitsName);
        updateBaseUnitCount(model, unitMap, multiplier, u2->name(), 1, 0, -1);
    } else if (unitMap.find(v2UnitsName) != unitMap.end()) {
        unitMap.at(v2UnitsName) -= 1.0;
    } else if (isStandardUnitName(v2UnitsName)) {
        updateBaseUnitCount(model, unitMap, multiplier, v2UnitsName, 1, 0, -1);
    }

    // Remove "dimensionless" from base unit testing.
    unitMap.erase("dimensionless");
    static const std::regex fullStopAtEndRegex(".$");

    bool status = true;
    for (const auto &basePair : unitMap) {
        if (basePair.second != 0.0) {
            std::string num = std::to_string(basePair.second);
            num.erase(num.find_last_not_of('0') + 1, num.length());
            num = std::regex_replace(num, fullStopAtEndRegex, "");
            hints += basePair.first + "^" + num + ", ";
            status = false;
        }
    }

    if (multiplier != 0.0) {
        // NB: multiplication issues are only reported when there is a base issue mismatch too, does not trigger it alone.
        // The multiplication mismatch will be returned through the multiplier argument in all cases.

        std::string num = std::to_string(multiplier);
        num.erase(num.find_last_not_of('0') + 1, num.length());
        num = std::regex_replace(num, fullStopAtEndRegex, "");
        hints += "multiplication factor of 10^" + num + ", ";
    }

    // Remove the final trailing comma from the hints string.
    if (hints.length() > 2) {
        hints.pop_back();
        hints.back() = '.';
    }

    return status;
}

void updateBaseUnitCount(const ModelPtr &model,
                         std::map<std::string, double> &unitMap,
                         double &multiplier,
                         const std::string &uName,
                         double uExp, double logMult,
                         int direction)
{
    if (model->hasUnits(uName)) {
        UnitsPtr u = model->units(uName);
        if (u->isBaseUnit()) {
            if (unitMap.find(uName) == unitMap.end()) {
                unitMap.emplace(uName, 0.0);
            }
            unitMap[uName] += direction * uExp;
            multiplier += direction * logMult;
        } else {
            std::string ref;
            std::string pre;
            std::string id;
            double exp;
            double mult;
            double expMult;
            for (size_t i = 0; i < u->unitCount(); ++i) {
                u->unitAttributes(i, ref, pre, exp, expMult, id);
                mult = std::log10(expMult);
                if (!isStandardUnitName(ref)) {
                    updateBaseUnitCount(model, unitMap, multiplier, ref, exp * uExp, logMult + mult * uExp + convertPrefixToInt(pre) * uExp, direction);
                } else {
                    for (const auto &iter : standardUnitsList.at(ref)) {
                        unitMap.at(iter.first) += direction * (iter.second * exp * uExp);
                    }
                    multiplier += direction * (logMult + (standardMultiplierList.at(ref) + mult + convertPrefixToInt(pre)) * exp);
                }
            }
        }
    } else if (isStandardUnitName(uName)) {
        for (const auto &iter : standardUnitsList.at(uName)) {
            unitMap.at(iter.first) += direction * (iter.second * uExp);
        }
        multiplier += direction * (logMult + standardMultiplierList.at(uName));
    }
}

void Validator::ValidatorImpl::checkUniqueResetOrders(const ModelPtr &model)
{
    auto resetOrderMap = buildModelResetOrderMap(model);
    for (const auto &variableOrder : resetOrderMap) {
        auto variable = variableOrder.first;
        auto orders = variableOrder.second;

        std::set<int> ordersSet(orders.begin(), orders.end());

        if (ordersSet.size() < orders.size()) {
            auto issue = Issue::IssueImpl::create();
            issue->mPimpl->setReferenceRule(Issue::ReferenceRule::RESET_ORDER_UNIQUE);
            issue->mPimpl->setDescription("Variable '" + variable->name() + "' used in resets does not have unique order values across the equivalent variable set.");
            issue->mPimpl->mItem->mPimpl->setModel(model);
            addIssue(issue);
        }
    }
}

void Validator::ValidatorImpl::addResetOrderMapItem(const VariablePtr &variable, int order, ResetOrderMap &resetOrderMap)
{
    auto currentVariable = variable;
    bool existingVariableFound = resetOrderMap.count(currentVariable) > 0;
    size_t i = 0;

    while ((i < variable->equivalentVariableCount()) && !existingVariableFound) {
        currentVariable = variable->equivalentVariable(i);
        existingVariableFound = resetOrderMap.count(currentVariable) > 0;
        ++i;
    }

    if (existingVariableFound) {
        resetOrderMap[currentVariable].emplace_back(order);
    } else {
        std::vector<int> orders = {order};
        resetOrderMap.emplace(variable, orders);
    }
}

void Validator::ValidatorImpl::traverseComponentTree(const ComponentPtr &component, ResetOrderMap &resetOrderMap)
{
    for (size_t j = 0; j < component->resetCount(); ++j) {
        auto reset = component->reset(j);
        auto currentVariable = reset->variable();
        if ((currentVariable != nullptr) && reset->isOrderSet()) {
            addResetOrderMapItem(currentVariable, reset->order(), resetOrderMap);
        }
    }

    for (size_t i = 0; i < component->componentCount(); ++i) {
        traverseComponentTree(component->component(i), resetOrderMap);
    }
}

ResetOrderMap Validator::ValidatorImpl::buildModelResetOrderMap(const ModelPtr &model)
{
    ResetOrderMap resetOrderMap;
    for (size_t i = 0; i < model->componentCount(); ++i) {
        auto component = model->component(i);
        traverseComponentTree(component, resetOrderMap);
    }

    return resetOrderMap;
}

void Validator::ValidatorImpl::checkUniqueIds(const ModelPtr &model)
{
    auto idMap = buildModelIdMap(model);

    for (const auto &id : idMap) {
        if (id.second.first > 1) {
            auto desc = "Duplicated identifier attribute '" + id.first + "' has been found in:\n";
            size_t i = 0;
            size_t iMax = id.second.second.size();
            for (const auto &item : id.second.second) {
                desc += item;
                ++i;
                if (i < iMax - 1) {
                    desc += ";\n";
                } else if (i == iMax - 1) {
                    desc += "; and\n";
                } else { /* i == iMax */
                    desc += ".\n";
                }
            }
            auto issue = Issue::IssueImpl::create();
            issue->mPimpl->setReferenceRule(Issue::ReferenceRule::XML_ID_ATTRIBUTE);
            issue->mPimpl->setDescription(desc);
            issue->mPimpl->mItem->mPimpl->setModel(model);
            addIssue(issue);
        }
    }
}

void Validator::ValidatorImpl::addIdMapItem(const std::string &id, const std::string &info, IdMap &idMap)
{
    if (idMap.count(id) > 0) {
        idMap[id].second.emplace_back(info);
        idMap[id] = std::make_pair(idMap[id].first + 1, idMap[id].second);
    } else {
        Strings infos;
        infos.emplace_back(info);
        idMap.emplace(id, std::make_pair(1, infos));
    }
}

void gatherComponents(const ComponentPtr &component, std::vector<ComponentPtr> &allComponents)
{
    std::vector<ComponentPtr> stack;

    stack.push_back(component);

    while (!stack.empty()) {
        allComponents.emplace_back(std::move(stack.back()));
        stack.pop_back();

        auto *current = allComponents.back().get();
        const auto childCount = current->componentCount();

        for (size_t i = 0; i < childCount; ++i) {
            stack.emplace_back(current->component(i));
        }
    }
}

IdMap Validator::ValidatorImpl::buildModelIdMap(const ModelPtr &model)
{
    IdMap idMap;
    std::string info;
    std::set<std::string> reportedConnections;
    std::vector<ComponentPtr> allComponents;

    for (size_t c = 0; c < model->componentCount(); ++c) {
        gatherComponents(model->component(c), allComponents);
    }

    ConnectionIdMap connectionIds;

    for (const auto &comp : allComponents) {
        auto rawPtr = comp.get();
        const size_t varCount = comp->variableCount();

        for (size_t i = 0; i < varCount; ++i) {
            auto currentVariable = comp->variable(i);

            for (size_t e = 0; e < currentVariable->equivalentVariableCount(); ++e) {
                auto equiv = currentVariable->equivalentVariable(e);
                auto equivParent = owningComponent(equiv);

                if (equivParent != nullptr) {
                    auto equivRawPtr = equivParent.get();
                    auto key = (rawPtr < equivRawPtr) ? ComponentRawPtrPair {rawPtr, equivRawPtr} : ComponentRawPtrPair {equivRawPtr, rawPtr};
                    connectionIds.try_emplace(key, Variable::equivalenceConnectionId(currentVariable, equiv, false));
                }
            }
        }
    }

    // Model.
    if (!model->id().empty()) {
        info = " - model '" + model->name() + "'";
        addIdMapItem(model->id(), info, idMap);
    }

    // Units.
    for (size_t u = 0; u < model->unitsCount(); ++u) {
        auto units = model->units(u);
        if (!units->id().empty()) {
            if (units->isImport()) {
                info = " - imported units '" + units->name() + "' in model '" + model->name() + "'";
            } else {
                info = " - units '" + units->name() + "' in model '" + model->name() + "'";
            }
            addIdMapItem(units->id(), info, idMap);
        }
        for (size_t i = 0; i < units->unitCount(); ++i) {
            std::string reference;
            std::string prefix;
            double exponent;
            double multiplier;
            std::string id;
            units->unitAttributes(i, reference, prefix, exponent, multiplier, id);
            if (!id.empty()) {
                info = " - unit in units '" + units->name() + "' in model '" + model->name() + "'";
                addIdMapItem(id, info, idMap);
            }
        }
        if ((units->importSource() != nullptr) && !units->importSource()->id().empty()) {
            info = " - import source for units '" + units->name() + "'";
            addIdMapItem(units->importSource()->id(), info, idMap);
        }
    }
    // Encapsulation.
    if (!model->encapsulationId().empty()) {
        // Check for a valid identifier.
        if (!isValidXmlName(model->encapsulationId())) {
            auto issue = Issue::IssueImpl::create();
            issue->mPimpl->setReferenceRule(Issue::ReferenceRule::XML_ID_ATTRIBUTE);
            issue->mPimpl->mItem->mPimpl->setModel(model);
            issue->mPimpl->setDescription("Model '" + model->name() + "' does not have a valid encapsulation 'id' attribute, '" + model->encapsulationId() + "'.");
            addIssue(issue);
        }

        info = " - encapsulation in model '" + model->name() + "'";
        addIdMapItem(model->encapsulationId(), info, idMap);
    }

    // Start recursion through encapsulation hierarchy.
    for (size_t c = 0; c < model->componentCount(); ++c) {
        buildComponentIdMap(model->component(c), idMap, reportedConnections, connectionIds);
    }
    return idMap;
}

void Validator::ValidatorImpl::buildComponentIdMap(const ComponentPtr &component, IdMap &idMap, std::set<std::string> &reportedConnections, const ConnectionIdMap &connectionIds)
{
    std::string info;

    // Component.
    if (!component->id().empty()) {
        std::string imported;
        std::string owning;
        if (component->isImport()) {
            imported = "imported ";
        }
        if (owningComponent(component) != nullptr) {
            owning = "' in component '" + owningComponent(component)->name() + "'";
        } else {
            owning = "' in model '" + owningModel(component)->name() + "'";
        }
        info = " - " + imported + "component '" + component->name() + owning;
        addIdMapItem(component->id(), info, idMap);
    }

    // Variables.
    for (size_t i = 0; i < component->variableCount(); ++i) {
        auto item = component->variable(i);
        if (!item->id().empty()) {
            info = " - variable '" + item->name() + "' in component '" + component->name() + "'";
            addIdMapItem(item->id(), info, idMap);
        }
        // Equivalent variables.
        for (size_t e = 0; e < item->equivalentVariableCount(); ++e) {
            auto equiv = item->equivalentVariable(e);
            auto equivParent = owningComponent(equiv);
            if (equivParent != nullptr) {
                // Skipping half of the equivalences to avoid duplicate reporting.
                std::string s1 = item->name() + component->name();
                std::string s2 = equiv->name() + equivParent->name();
                std::string mappingId = Variable::equivalenceMappingId(item, equiv);
                // Variable mapping.
                if ((s1 < s2) && !mappingId.empty()) {
                    std::string mappingDescription =
                        "between variable '" + item->name() + "' in component '" + component->name()
                        + "' and variable '" + equiv->name() + "' in component '" + equivParent->name() + "'";
                    // Check for a valid identifier.
                    if (!isValidXmlName(mappingId)) {
                        auto issue = Issue::IssueImpl::create();
                        issue->mPimpl->setReferenceRule(Issue::ReferenceRule::XML_ID_ATTRIBUTE);
                        issue->mPimpl->mItem->mPimpl->setMapVariables(item, equiv);
                        issue->mPimpl->setDescription("Variable equivalence " + mappingDescription + ", does not have a valid map_variables 'id' attribute, '" + mappingId + "'.");
                        addIssue(issue);
                    }

                    info = " - variable equivalence " + mappingDescription;
                    addIdMapItem(mappingId, info, idMap);
                }
                // Connections.
                auto key = component.get() < equivParent.get() ? ComponentRawPtrPair {component.get(), equivParent.get()} : ComponentRawPtrPair {equivParent.get(), component.get()};
                auto connectionId = connectionIds.at(key);
                std::string connection = component->name() < equivParent->name() ? component->name() + equivParent->name() : equivParent->name() + component->name();

                if ((s1 < s2) && !connectionId.empty() && (reportedConnections.count(connection) == 0)) {
                    std::string connectionDescription =
                        "between components '" + component->name() + "' and '" + equivParent->name()
                        + "' because of variable equivalence between variables '" + item->name()
                        + "' and '" + equiv->name() + "'";
                    // Check for a valid identifier.
                    if (!isValidXmlName(connectionId)) {
                        auto issue = Issue::IssueImpl::create();
                        issue->mPimpl->setReferenceRule(Issue::ReferenceRule::XML_ID_ATTRIBUTE);
                        issue->mPimpl->mItem->mPimpl->setConnection(item, equiv);
                        issue->mPimpl->setDescription("Connection " + connectionDescription + ", does not have a valid connection 'id' attribute, '" + connectionId + "'.");
                        addIssue(issue);
                    }

                    reportedConnections.insert(connection);
                    info = " - connection " + connectionDescription;
                    addIdMapItem(connectionId, info, idMap);
                }
            }
        }
    }

    // Resets.
    for (size_t i = 0; i < component->resetCount(); ++i) {
        auto item = component->reset(i);
        if (!item->id().empty()) {
            info = " - reset at index " + std::to_string(i) + " in component '" + component->name() + "'";
            addIdMapItem(item->id(), info, idMap);
        }
        if (!item->testValueId().empty()) {
            info = " - test_value in reset at index " + std::to_string(i) + " in component '" + component->name() + "'";
            addIdMapItem(item->testValueId(), info, idMap);
        }
        info = "test_value in reset " + std::to_string(i) + " in component '" + component->name() + "'";
        buildMathIdMap(info, idMap, item->testValue());
        if (!item->resetValueId().empty()) {
            info = " - reset_value in reset at index " + std::to_string(i) + " in component '" + component->name() + "'";
            addIdMapItem(item->resetValueId(), info, idMap);
        }
        info = "reset_value in reset " + std::to_string(i) + " in component '" + component->name() + "'";
        buildMathIdMap(info, idMap, item->resetValue());
    }

    // Maths.
    info = "math in component '" + component->name() + "'";
    buildMathIdMap(info, idMap, component->math());

    // Imports.
    if ((component->importSource() != nullptr) && !component->importSource()->id().empty()) {
        info = " - import source for component '" + component->name() + "'";
        addIdMapItem(component->importSource()->id(), info, idMap);
    }

    // Hierarchy.
    if (!component->encapsulationId().empty()) {
        // Check for a valid identifier.
        if (!isValidXmlName(component->encapsulationId())) {
            auto issue = Issue::IssueImpl::create();
            issue->mPimpl->setReferenceRule(Issue::ReferenceRule::XML_ID_ATTRIBUTE);
            issue->mPimpl->mItem->mPimpl->setComponent(component);
            issue->mPimpl->setDescription("Component '" + component->name() + "' does not have a valid encapsulation 'id' attribute, '" + component->encapsulationId() + "'.");
            addIssue(issue);
        }

        info = " - encapsulation component_ref to component '" + component->name() + "'";
        addIdMapItem(component->encapsulationId(), info, idMap);
    }

    // Child components.
    for (size_t c = 0; c < component->componentCount(); ++c) {
        buildComponentIdMap(component->component(c), idMap, reportedConnections, connectionIds);
    }
}

void Validator::ValidatorImpl::buildMathIdMap(const std::string &infoRef, IdMap &idMap, const std::string &input)
{
    std::vector<XmlDocPtr> docs = multiRootXml(input);

    for (const auto &doc : docs) {
        XmlNodePtr node = doc->rootNode();
        if (node == nullptr) {
            return;
        }
        if (!node->isMathmlElement("math")) {
            continue;
        }
        buildMathChildIdMap(node, infoRef, idMap);
    }
}

void Validator::ValidatorImpl::buildMathChildIdMap(const XmlNodePtr &node, const std::string &infoRef, IdMap &idMap)
{
    std::string info;
    XmlAttributePtr attribute = node->firstAttribute();
    while (attribute != nullptr) {
        if (attribute->isType("id")) {
            std::string variable;
            if (node->name() == "ci") {
                if (node->firstChild() != nullptr) {
                    variable = "'" + node->firstChild()->convertToString() + "' ";
                }
            }
            info = " - MathML " + node->name() + " element " + variable + "in " + infoRef;
            addIdMapItem(attribute->value(), info, idMap);
        }
        attribute = attribute->next();
    }
    XmlNodePtr childNode = node->firstChild();
    while (childNode != nullptr) {
        buildMathChildIdMap(childNode, infoRef, idMap);
        childNode = childNode->next();
    }
}

} // namespace libcellml
