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

#include "utilities.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <limits>
#include <numeric>
#include <set>
#include <sstream>
#include <vector>

#include "libcellml/analyserequation.h"
#include "libcellml/analysermodel.h"
#include "libcellml/analyservariable.h"
#include "libcellml/component.h"
#include "libcellml/importsource.h"
#include "libcellml/model.h"
#include "libcellml/reset.h"
#include "libcellml/units.h"
#include "libcellml/variable.h"

#include "commonutils.h"
#include "xmldoc.h"
#include "xmlutils.h"

namespace libcellml {

static const std::map<std::string, int> standardPrefixList = {
    {"yotta", 24},
    {"zetta", 21},
    {"exa", 18},
    {"peta", 15},
    {"tera", 12},
    {"giga", 9},
    {"mega", 6},
    {"kilo", 3},
    {"hecto", 2},
    {"deca", 1},
    {"deci", -1},
    {"centi", -2},
    {"milli", -3},
    {"micro", -6},
    {"nano", -9},
    {"pico", -12},
    {"femto", -15},
    {"atto", -18},
    {"zepto", -21},
    {"yocto", -24}};

bool stringToDouble(const std::string &in, double &out)
{
    try {
        out = std::stod(in);
    } catch (std::out_of_range &) {
        return false;
    }

    return true;
}

bool canConvertToBasicDouble(const std::string &in)
{
    if (!isCellMLBasicReal(in)) {
        return false;
    }

    double temp;
    return stringToDouble(in, temp);
}

bool convertToDouble(const std::string &in, double &out)
{
    if (!isCellMLReal(in)) {
        return false;
    }

    return stringToDouble(in, out);
}

bool hasNonWhitespaceCharacters(const std::string &input)
{
    return input.find_first_not_of(" \t\n\v\f\r") != std::string::npos;
}

Strings split(const std::string &content, const std::string &delimiter)
{
    Strings strings;
    size_t current;
    size_t previous = 0;
    current = content.find(delimiter);
    while (current != std::string::npos) {
        strings.push_back(content.substr(previous, current - previous));
        previous = current + delimiter.size();
        current = content.find(delimiter, previous);
    }
    strings.push_back(content.substr(previous, current - previous));

    return strings;
}

std::string convertToString(double value, bool fullPrecision)
{
    if (std::isnan(value)) {
        // Always return "nan" whether we are dealing with +NAN or -NAN. This is to ensure that
        // the string representation of a NAN is consistent across compilers.
        return "nan";
    }
    std::ostringstream strs;
    if (fullPrecision) {
        strs << std::setprecision(std::numeric_limits<double>::digits10) << value;
    } else {
        strs << value;
    }
    return strs.str();
}

bool convertToInt(const std::string &in, int &out)
{
    if (!isCellMLInteger(in)) {
        return false;
    }

    try {
        out = std::stoi(in);
    } catch (std::out_of_range &) {
        return false;
    }
    return true;
}

int convertPrefixToInt(const std::string &in, bool *ok)
{
    int prefixInt = 0;

    if (ok != nullptr) {
        *ok = true;
    }

    if (isStandardPrefixName(in)) {
        prefixInt = standardPrefixList.at(in);
    } else if (!in.empty()) {
        bool success = convertToInt(in, prefixInt);
        if (ok != nullptr) {
            *ok = success;
        }
    }
    return prefixInt;
}

std::string convertToString(size_t value)
{
    std::ostringstream strs;
    strs << value;
    return strs.str();
}

std::string convertToString(int value)
{
    std::ostringstream strs;
    strs << value;
    return strs.str();
}

bool isEuropeanNumericCharacter(char c)
{
    const std::set<char> validIntegerCharacters = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
    return validIntegerCharacters.find(c) != validIntegerCharacters.end();
}

bool isNonNegativeCellMLInteger(const std::string &candidate)
{
    if (candidate.empty()) {
        return false;
    }
    return std::all_of(candidate.begin(), candidate.end(), isEuropeanNumericCharacter);
}

bool isCellMLInteger(const std::string &candidate)
{
    if (!candidate.empty() && ((*candidate.begin() == '-') || (*candidate.begin() == '+'))) {
        return isNonNegativeCellMLInteger(candidate.substr(1));
    }
    return isNonNegativeCellMLInteger(candidate);
}

bool isCellMLExponent(const std::string &candidate)
{
    return isCellMLInteger(candidate);
}

std::vector<size_t> findOccurrences(const std::string &candidate, const std::string &sub)
{
    std::vector<size_t> occurrences;
    size_t pos = candidate.find(sub, 0);
    while (pos != std::string::npos) {
        occurrences.push_back(pos);
        pos = candidate.find(sub, pos + 1);
    }
    return occurrences;
}

bool isCellMLBasicReal(const std::string &candidate)
{
    if (!candidate.empty()) {
        std::vector<size_t> decimalOccurrences = findOccurrences(candidate, ".");
        if (decimalOccurrences.size() < 2) {
            bool beginsMinus = *candidate.begin() == '-';
            std::string numbersOnlyCandidate = candidate;
            if (decimalOccurrences.size() == 1) {
                numbersOnlyCandidate.erase(decimalOccurrences.at(0), 1);
            }
            if (beginsMinus) {
                numbersOnlyCandidate.erase(0, 1);
            }
            return std::all_of(numbersOnlyCandidate.begin(), numbersOnlyCandidate.end(), isEuropeanNumericCharacter);
        }
    }
    return false;
}

bool isCellMLReal(const std::string &candidate)
{
    bool isReal = false;
    if (!candidate.empty()) {
        std::string normalisedCandidate = candidate;
        std::vector<size_t> eOccurrences = findOccurrences(candidate, "E");
        for (const auto &ePos : eOccurrences) {
            normalisedCandidate.replace(ePos, 1, "e");
        }
        std::vector<size_t> lowerEOccurrences = findOccurrences(normalisedCandidate, "e");
        size_t eIndicatorCount = lowerEOccurrences.size();
        if (eIndicatorCount < 2) {
            if (eIndicatorCount == 1) {
                size_t ePos = lowerEOccurrences.at(0);
                std::string significand = normalisedCandidate.substr(0, ePos);
                std::string exponent = normalisedCandidate.substr(ePos + 1, std::string::npos);
                if (isCellMLBasicReal(significand) && isCellMLExponent(exponent)) {
                    isReal = true;
                }
            } else {
                isReal = isCellMLBasicReal(normalisedCandidate);
            }
        }
    }
    return isReal;
}

bool areEqual(double a, double b)
{
    // Note: we add 0.0 in case a is, for instance, equal to 0.0 and b is equal
    //       to -0.0.
    return convertToString(a + 0.0) == convertToString(b + 0.0);
}

uint64_t ulpsDistance(double a, double b)
{
    static const auto max = std::numeric_limits<uint64_t>::max();

    // Max distance for NaN.
    if (std::isnan(a) || std::isnan(b)) {
        return max;
    }

    // If one's infinite and they're not equal, max distance.
    if (std::isinf(a) != std::isinf(b)) {
        return max;
    }

    static const int SIZE_OF_DOUBLE = sizeof(double);

    uint64_t ia;
    uint64_t ib;
    memcpy(&ia, &a, SIZE_OF_DOUBLE);
    memcpy(&ib, &b, SIZE_OF_DOUBLE);

    // Return the absolute value of the distance in ULPs.
    uint64_t distance = max;
    if (ia < ib) {
        distance = ib + ~ia + 1;
    } else {
        distance = ia + ~ib + 1;
    }
    return distance;
}

bool areNearlyEqual(double a, double b)
{
    static const double fixedEpsilon = std::numeric_limits<double>::epsilon();
    static const ptrdiff_t ulpsEpsilon = 1;

    if (fabs(a - b) <= fixedEpsilon) {
        return true;
    }

    // If they are not the same sign then return false.
    if ((a < 0.0) != (b < 0.0)) {
        return false;
    }

    return ulpsDistance(a, b) <= ulpsEpsilon;
}

std::vector<ComponentPtr> getImportedComponents(const ComponentEntityConstPtr &componentEntity)
{
    std::vector<ComponentPtr> importedComponents;
    for (size_t i = 0; i < componentEntity->componentCount(); ++i) {
        auto component = componentEntity->component(i);
        if (component->isImport()) {
            importedComponents.push_back(component);
        }

        auto childImportedComponents = getImportedComponents(component);
        importedComponents.reserve(importedComponents.size() + childImportedComponents.size());
        importedComponents.insert(importedComponents.end(), childImportedComponents.begin(), childImportedComponents.end());
    }

    return importedComponents;
}

std::vector<UnitsPtr> getImportedUnits(const ModelConstPtr &model)
{
    std::vector<UnitsPtr> importedUnits;
    for (size_t i = 0; i < model->unitsCount(); ++i) {
        auto units = model->units(i);
        if (units->isImport()) {
            importedUnits.push_back(units);
        }
    }

    return importedUnits;
}

std::vector<ImportSourcePtr> getAllImportSources(const ModelConstPtr &model)
{
    std::vector<ImportSourcePtr> importSources;

    auto importedComponents = getImportedComponents(model);
    auto importedUnits = getImportedUnits(model);

    importSources.reserve(importedComponents.size() + importedUnits.size());
    for (auto &component : importedComponents) {
        importSources.push_back(component->importSource());
    }
    for (auto &units : importedUnits) {
        importSources.push_back(units->importSource());
    }

    return importSources;
}

bool isStandardUnitName(const std::string &name)
{
    return standardUnitsList.count(name) != 0;
}

bool isStandardUnit(const UnitsPtr &units)
{
    return (units->unitCount() == 0) && isStandardUnitName(units->name());
}

bool isStandardPrefixName(const std::string &name)
{
    return standardPrefixList.count(name) != 0;
}

void removeComponentFromEntity(const EntityPtr &entity, const ComponentPtr &component)
{
    auto componentEntity = std::dynamic_pointer_cast<ComponentEntity>(entity);
    componentEntity->removeComponent(component, false);
}

size_t indexOf(const VariablePtr &variable, const ComponentConstPtr &component)
{
    size_t index = 0;
    bool found = false;
    while (index < component->variableCount() && !found) {
        if (component->variable(index) == variable) {
            found = true;
        } else {
            ++index;
        }
    }

    return index;
}

bool areEquivalentVariables(const VariablePtr &variable1,
                            const VariablePtr &variable2)
{
    return (variable1 == variable2) || variable1->hasEquivalentVariable(variable2, true);
}

bool isEntityChildOf(const ParentedEntityPtr &entity1, const ParentedEntityPtr &entity2)
{
    return entity1->parent() == entity2;
}

bool areEntitiesSiblings(const ParentedEntityPtr &entity1, const ParentedEntityPtr &entity2)
{
    auto entity1Parent = entity1->parent();
    return entity1Parent == entity2->parent();
}

using PublicPrivateRequiredPair = std::pair<bool, bool>;

/**
 * @brief Determine whether a public and/or private interface is required for the given @p variable.
 *
 * Determine whether a public and/or private interface is required for the given @p variable.  Returns
 * a pair of booleans where the first item in the pair indicates whether a public interface is required,
 * the second item in the pair indicates whether a private interface is required, and if both items
 * in the pair are @c false then this indicates an error has occurred and the interface type cannot be
 * determined.
 *
 * @param variable The variable to detect the interface type required.
 *
 * @return A pair of booleans.
 */
PublicPrivateRequiredPair publicAndOrPrivateInterfaceTypeRequired(const VariablePtr &variable)
{
    PublicPrivateRequiredPair pair = std::make_pair(false, false);
    for (size_t index = 0; index < variable->equivalentVariableCount() && !(pair.first && pair.second); ++index) {
        auto equivalentVariable = variable->equivalentVariable(index);
        auto componentOfVariable = variable->parent();
        auto componentOfEquivalentVariable = equivalentVariable->parent();
        if (componentOfEquivalentVariable == nullptr) {
            return std::make_pair(false, false);
        }
        if (areEntitiesSiblings(componentOfVariable, componentOfEquivalentVariable)
            || isEntityChildOf(componentOfVariable, componentOfEquivalentVariable)) {
            pair.first = true;
        } else if (isEntityChildOf(componentOfEquivalentVariable, componentOfVariable)) {
            pair.second = true;
        } else {
            return std::make_pair(false, false);
        }
    }
    return pair;
}

/**
 * @brief Get the interface type for the given public private pair.
 *
 * Get the interface type for the @p pair.  The default return type is
 * Variable::InterfaceType::NONE.
 *
 * @param pair The pair for which the interface type is determined.
 *
 * @return The interface type as specified in the @p pair.
 */
Variable::InterfaceType interfaceTypeFor(const PublicPrivateRequiredPair &pair)
{
    Variable::InterfaceType interfaceType = Variable::InterfaceType::NONE;
    if (pair.first && pair.second) {
        interfaceType = Variable::InterfaceType::PUBLIC_AND_PRIVATE;
    } else if (pair.first) {
        interfaceType = Variable::InterfaceType::PUBLIC;
    } else if (pair.second) {
        interfaceType = Variable::InterfaceType::PRIVATE;
    }

    return interfaceType;
}

Variable::InterfaceType determineInterfaceType(const VariablePtr &variable)
{
    auto publicAndOrPrivatePair = publicAndOrPrivateInterfaceTypeRequired(variable);

    return interfaceTypeFor(publicAndOrPrivatePair);
}

void findAllVariablesWithEquivalences(const ComponentPtr &component, VariablePtrs &variables)
{
    for (size_t index = 0; index < component->variableCount(); ++index) {
        auto variable = component->variable(index);
        if (variable->equivalentVariableCount() > 0) {
            variables.push_back(variable);
        }
    }
    for (size_t index = 0; index < component->componentCount(); ++index) {
        findAllVariablesWithEquivalences(component->component(index), variables);
    }
}

/**
 * @brief Return a set of names taken from MathML cn units attribute.
 *
 * Search the given @p node for MathML @c cn elements.
 * For all @c cn elements return the units reference if it is not empty
 * or a reference to a standard unit.
 *
 * @param node The node to search for MathML @c cn elements.
 * @return A set of units references.
 */
UniqueNames findCnUnitsNames(const XmlNodePtr &node);

/**
 * @brief Find all MathML @c cn elements units attributes in the given component's math string.
 *
 * Search through the @p component's math string and return a list of units references found
 * on MathML @c cn elements units attribute.
 *
 * @param component The component to search.
 * @return A list of units references.
 */
NameList findComponentCnUnitsNames(const ComponentConstPtr &component);

void findAndReplaceCnUnitsNames(const XmlNodePtr &node, const std::string &oldName, const std::string &newName);
void findAndReplaceComponentCnUnitsNames(const ComponentPtr &component, const std::string &oldName, const std::string &newName);
size_t getComponentIndexInComponentEntity(const ComponentEntityPtr &componentParent, const ComponentEntityPtr &component);
IndexStack indexStackOf(const VariablePtr &variable);
VariablePtr getVariableLocatedAt(const IndexStack &stack, const ModelPtr &model);
void makeEquivalence(const IndexStack &stack1, const IndexStack &stack2, const ModelPtr &model);
IndexStack rebaseIndexStack(const IndexStack &stack, const IndexStack &originStack, const IndexStack &destinationStack);
void componentNames(const ComponentPtr &component, NameList &names);

UniqueNames findCnUnitsNames(const XmlNodePtr &node)
{
    UniqueNames names;
    XmlNodePtr childNode = node->firstChild();
    while (childNode != nullptr) {
        if (childNode->isMathmlElement("cn")) {
            std::string u = childNode->attribute("units");
            if (!u.empty() && !isStandardUnitName(u)) {
                names.insert(u);
            }
        }
        names.merge(findCnUnitsNames(childNode));
        childNode = childNode->next();
    }

    return names;
}

NameList findComponentCnUnitsNames(const ComponentConstPtr &component)
{
    UniqueNames nodeUnitsNames;
    // Inspect the MathML in this component for any specified constant <cn> units.
    std::string mathContent = component->math();
    if (mathContent.empty()) {
        return {};
    }
    std::vector<XmlDocPtr> mathDocs = multiRootXml(mathContent);
    for (const auto &doc : mathDocs) {
        auto rootNode = doc->rootNode();
        if (rootNode->isMathmlElement("math")) {
            nodeUnitsNames.merge(findCnUnitsNames(rootNode));
        }
    }

    NameList unitsNames(nodeUnitsNames.size());
    std::copy(nodeUnitsNames.begin(), nodeUnitsNames.end(), unitsNames.begin());
    return unitsNames;
}

void findAndReplaceCnUnitsNames(const XmlNodePtr &node, const std::string &oldName, const std::string &newName)
{
    XmlNodePtr childNode = node->firstChild();
    while (childNode != nullptr) {
        if (childNode->isMathmlElement("cn")) {
            std::string unitsName = childNode->attribute("units");
            if (unitsName == oldName) {
                childNode->setAttribute("units", newName.c_str());
            }
        }
        findAndReplaceCnUnitsNames(childNode, oldName, newName);
        childNode = childNode->next();
    }
}

void findAndReplaceComponentCnUnitsNames(const ComponentPtr &component, const std::string &oldName, const std::string &newName)
{
    std::string mathContent = component->math();
    if (mathContent.empty()) {
        return;
    }
    bool contentModified = false;
    std::string newMathContent;
    std::vector<XmlDocPtr> mathDocs = multiRootXml(mathContent);
    for (const auto &doc : mathDocs) {
        auto rootNode = doc->rootNode();
        if (rootNode->isMathmlElement("math")) {
            auto originalMath = rootNode->convertToString();
            findAndReplaceCnUnitsNames(rootNode, oldName, newName);
            auto newMath = rootNode->convertToString();
            newMathContent += newMath;
            if (newMath != originalMath) {
                contentModified = true;
            }
        }
    }

    if (contentModified) {
        component->setMath(newMathContent);
    }
}

void findAndReplaceComponentsCnUnitsNames(const ComponentPtr &component, const std::string &oldName, const std::string &newName)
{
    findAndReplaceComponentCnUnitsNames(component, oldName, newName);
    for (size_t index = 0; index < component->componentCount(); ++index) {
        auto childComponent = component->component(index);
        findAndReplaceComponentCnUnitsNames(childComponent, oldName, newName);
    }
}

size_t getComponentIndexInComponentEntity(const ComponentEntityPtr &componentParent, const ComponentEntityPtr &component)
{
    size_t index = 0;
    bool found = false;
    while (!found) {
        if (componentParent->component(index) == component) {
            found = true;
        } else {
            ++index;
        }
    }

    return index;
}

IndexStack indexStackOf(const ComponentPtr &component)
{
    auto dummyVariable = Variable::create();
    component->addVariable(dummyVariable);
    IndexStack indexStack = indexStackOf(dummyVariable);
    indexStack.pop_back();
    component->removeVariable(dummyVariable);

    return indexStack;
}

IndexStack rebaseIndexStack(const IndexStack &stack, const IndexStack &originStack, const IndexStack &destinationStack)
{
    auto rebasedStack = stack;

    rebasedStack.resize(originStack.size(), SIZE_MAX);
    if (rebasedStack == originStack) {
        rebasedStack = destinationStack;
        auto offsetIt = stack.begin() + ptrdiff_t(originStack.size());
        rebasedStack.insert(rebasedStack.end(), offsetIt, stack.end());
    } else {
        rebasedStack.clear();
    }

    return rebasedStack;
}

EquivalenceMap rebaseEquivalenceMap(const EquivalenceMap &map, const IndexStack &originStack, const IndexStack &destinationStack)
{
    EquivalenceMap rebasedMap;
    for (const auto &entry : map) {
        auto key = entry.first;
        auto rebasedKey = rebaseIndexStack(key, originStack, destinationStack);
        auto vector = entry.second;
        std::vector<IndexStack> rebasedVector;
        for (auto stack : vector) {
            // Temporarily remove the variable index whilst we rebase the component part of the stack.
            size_t variableIndex = stack.back();
            stack.pop_back();
            auto rebasedTarget = rebaseIndexStack(stack, originStack, destinationStack);
            if (!rebasedTarget.empty()) {
                rebasedTarget.push_back(variableIndex);
                rebasedVector.push_back(rebasedTarget);
            }
        }

        if (!rebasedVector.empty()) {
            rebasedMap.emplace(rebasedKey, rebasedVector);
        }
    }

    return rebasedMap;
}

void componentNames(const ComponentPtr &component, NameList &names)
{
    for (size_t index = 0; index < component->componentCount(); ++index) {
        auto c = component->component(index);
        names.push_back(c->name());
        componentNames(c, names);
    }
}

NameList componentNames(const ModelPtr &model)
{
    NameList names;
    for (size_t index = 0; index < model->componentCount(); ++index) {
        auto component = model->component(index);
        names.push_back(component->name());
        componentNames(component, names);
    }
    return names;
}

ComponentNameMap createComponentNamesMap(const ComponentPtr &component)
{
    ComponentNameMap nameMap;
    for (size_t index = 0; index < component->componentCount(); ++index) {
        auto c = component->component(index);
        nameMap.emplace(c->name(), c);
        ComponentNameMap childrenNameMap = createComponentNamesMap(c);
        nameMap.insert(childrenNameMap.begin(), childrenNameMap.end());
    }

    return nameMap;
}

std::vector<UnitsPtr> referencedUnits(const ModelPtr &model, const UnitsPtr &units)
{
    std::vector<UnitsPtr> requiredUnits;

    for (size_t index = 0; index < units->unitCount(); ++index) {
        const std::string ref = units->unitAttributeReference(index);
        if (!isStandardUnitName(ref)) {
            auto refUnits = model->units(ref);
            auto requiredUnitsUnits = referencedUnits(model, refUnits);
            requiredUnits.insert(requiredUnits.end(), requiredUnitsUnits.begin(), requiredUnitsUnits.end());
            requiredUnits.push_back(refUnits);
        }
    }

    return requiredUnits;
}

std::vector<UnitsPtr> unitsUsed(const ModelPtr &model, const ComponentConstPtr &component)
{
    std::vector<UnitsPtr> usedUnits;

    // Get all the units used by variables in this component.

    for (size_t i = 0; i < component->variableCount(); ++i) {
        auto v = component->variable(i);
        auto u = v->units();
        if ((u != nullptr) && !isStandardUnitName(u->name()) && (model != nullptr)) {
            auto modelUnits = model->units(u->name());
            auto availableUnits = modelUnits ? modelUnits : u;
            auto requiredUnits = referencedUnits(model, availableUnits);
            usedUnits.insert(usedUnits.end(), requiredUnits.begin(), requiredUnits.end());
            usedUnits.push_back(availableUnits);
        } else if (model == nullptr) {
            usedUnits.push_back(u);
        }
    }

    // Get all the units used by cn elements in the components maths.

    auto componentCnUnitsNames = findComponentCnUnitsNames(component);
    for (const auto &unitsName : componentCnUnitsNames) {
        auto u = model->units(unitsName);
        if (u == nullptr) {
            // We have used a units in the math but it is not defined in the given model, so send back a units that isn't defined.
            u = Units::create(unitsName);
        } else {
            auto requiredUnits = referencedUnits(model, u);
            usedUnits.insert(usedUnits.end(), requiredUnits.begin(), requiredUnits.end());
        }
        usedUnits.push_back(u);
    }

    // Get all the units used by child components of this component.

    for (size_t i = 0; i < component->componentCount(); ++i) {
        auto childComponent = component->component(i);
        auto childUsedUnits = unitsUsed(model, childComponent);
        usedUnits.insert(usedUnits.end(), childUsedUnits.begin(), childUsedUnits.end());
    }

    return usedUnits;
}

NameList unitsNamesUsed(const ComponentPtr &component)
{
    auto unitNames = findComponentCnUnitsNames(component);
    for (size_t i = 0; i < component->variableCount(); ++i) {
        auto u = component->variable(i)->units();
        if ((u != nullptr) && !isStandardUnitName(u->name())) {
            unitNames.push_back(u->name());
        }
    }
    return unitNames;
}

IndexStack indexStackOf(const VariablePtr &variable)
{
    IndexStack indexStack;
    ComponentPtr component = std::dynamic_pointer_cast<Component>(variable->parent());
    indexStack.push_back(indexOf(variable, component));

    ComponentEntityPtr parent = component;
    ComponentEntityPtr grandParent = std::dynamic_pointer_cast<ComponentEntity>(parent->parent());
    while (grandParent != nullptr) {
        indexStack.push_back(getComponentIndexInComponentEntity(grandParent, parent));
        parent = grandParent;
        grandParent = std::dynamic_pointer_cast<ComponentEntity>(parent->parent());
    }

    std::reverse(std::begin(indexStack), std::end(indexStack));

    return indexStack;
}

void recordVariableEquivalences(const ComponentPtr &component, EquivalenceMap &equivalenceMap, IndexStack &indexStack)
{
    for (size_t index = 0; index < component->variableCount(); ++index) {
        auto variable = component->variable(index);
        for (size_t j = 0; j < variable->equivalentVariableCount(); ++j) {
            if (j == 0) {
                indexStack.push_back(index);
            }
            auto equivalentVariable = variable->equivalentVariable(j);
            auto equivalentVariableIndexStack = indexStackOf(equivalentVariable);
            if (equivalenceMap.count(indexStack) == 0) {
                equivalenceMap.emplace(indexStack, std::vector<IndexStack>());
            }
            equivalenceMap[indexStack].push_back(equivalentVariableIndexStack);
        }
        if (variable->equivalentVariableCount() > 0) {
            indexStack.pop_back();
        }
    }
}

void generateEquivalenceMap(const ComponentPtr &component, EquivalenceMap &map, IndexStack &indexStack)
{
    for (size_t index = 0; index < component->componentCount(); ++index) {
        indexStack.push_back(index);
        auto c = component->component(index);
        recordVariableEquivalences(c, map, indexStack);
        generateEquivalenceMap(c, map, indexStack);
        indexStack.pop_back();
    }
}

VariablePtr getVariableLocatedAt(const IndexStack &stack, const ModelPtr &model)
{
    ComponentPtr component;
    for (size_t index = 0; index < stack.size() - 1; ++index) {
        if (index == 0) {
            component = model->component(stack.at(index));
        } else {
            component = component->component(stack.at(index));
        }
    }

    return component->variable(stack.back());
}

void makeEquivalence(const IndexStack &stack1, const IndexStack &stack2, const ModelPtr &model)
{
    auto v1 = getVariableLocatedAt(stack1, model);
    auto v2 = getVariableLocatedAt(stack2, model);
    Variable::addEquivalence(v1, v2);
}

void applyEquivalenceMapToModel(const EquivalenceMap &map, const ModelPtr &model)
{
    for (const auto &iter : map) {
        auto key = iter.first;
        auto vector = iter.second;
        for (auto vectorIter = vector.begin(); vectorIter < vector.end(); ++vectorIter) {
            makeEquivalence(key, *vectorIter, model);
        }
    }
}

void listComponentIds(const ComponentPtr &component, IdList &idList)
{
    std::string id = component->id();
    if (!id.empty()) {
        idList.insert(id);
    }
    // Imports.
    auto importSource = component->importSource();
    if (importSource != nullptr) {
        id = importSource->id();
        if (!id.empty()) {
            idList.insert(id);
        }
    }
    // Component reference in encapsulation structure.
    id = component->encapsulationId();
    if (!id.empty()) {
        idList.insert(id);
    }
    // Variables.
    for (size_t v = 0; v < component->variableCount(); ++v) {
        auto variable = component->variable(v);
        id = variable->id();
        if (!id.empty()) {
            idList.insert(id);
        }

        for (size_t e = 0; e < variable->equivalentVariableCount(); ++e) {
            // Equivalent variable mappings.
            id = Variable::equivalenceMappingId(variable, variable->equivalentVariable(e));
            if (!id.empty()) {
                idList.insert(id);
            }
            // Connections.
            id = Variable::equivalenceConnectionId(variable, variable->equivalentVariable(e));
            if (!id.empty()) {
                idList.insert(id);
            }
        }
    }
    // Resets.
    for (size_t r = 0; r < component->resetCount(); ++r) {
        auto reset = component->reset(r);
        id = reset->id();
        if (!id.empty()) {
            idList.insert(id);
        }
        id = reset->testValueId();
        if (!id.empty()) {
            idList.insert(id);
        }
        id = reset->resetValueId();
        if (!id.empty()) {
            idList.insert(id);
        }
    }

    // Note: identifiers on component and reset MathML blocks and their children are not yet included.

    for (size_t c = 0; c < component->componentCount(); ++c) {
        listComponentIds(component->component(c), idList);
    }
}

IdList listIds(const ModelPtr &model)
{
    // Collect all existing identifiers in a list and return. NB can't use a map or a set as we need to be able to print
    // invalid models (with duplicated identifiers) too.

    std::unordered_set<std::string> idList;
    // Model.
    std::string id = model->id();
    if (!id.empty()) {
        idList.insert(id);
    }
    // Units.
    for (size_t u = 0; u < model->unitsCount(); ++u) {
        auto units = model->units(u);
        id = units->id();
        if (!id.empty()) {
            idList.insert(id);
        }
        // Imports.
        auto importSource = units->importSource();
        if (importSource != nullptr) {
            id = importSource->id();
            if (!id.empty()) {
                idList.insert(id);
            }
        }
        for (size_t i = 0; i < units->unitCount(); ++i) {
            std::string prefix;
            std::string reference;
            double exponent;
            double multiplier;
            units->unitAttributes(i, reference, prefix, exponent, multiplier, id);
            if (!id.empty()) {
                idList.insert(id);
            }
        }
    }
    // Components.
    for (size_t c = 0; c < model->componentCount(); ++c) {
        listComponentIds(model->component(c), idList);
    }
    // Encapsulation.
    id = model->encapsulationId();
    if (!id.empty()) {
        idList.insert(id);
    }

    return idList;
}

std::string makeUniqueId(IdList &idList)
{
    // Because the hexadecimal counter starts high enough that it will always have a letter as the first character,
    // we don't need to prefix it with any other string to be valid.
    int counter = 0xb4da55;
    std::stringstream stream;
    stream << std::hex << counter;
    std::string id = stream.str();
    stream.str(std::string());

    while (idList.count(id) != 0) {
        ++counter;
        stream << std::hex << counter;
        id = stream.str();
        stream.str(std::string());
    }
    idList.insert(id);
    return id;
}

ConnectionMap createConnectionMap(const VariablePtr &variable1, const VariablePtr &variable2)
{
    ConnectionMap map;

    ComponentPtr component1 = owningComponent(variable1);
    ComponentPtr component2 = owningComponent(variable2);
    if ((component1 != nullptr) && (component2 != nullptr)) {
        for (size_t i = 0; i < component1->variableCount(); ++i) {
            auto v = component1->variable(i);
            for (const auto &vEquiv : equivalentVariables(v)) {
                if (owningComponent(vEquiv) == component2) {
                    map.insert(std::make_pair(v, vEquiv));
                }
            }
        }
    }

    return map;
}

void recursiveEquivalentVariables(const VariablePtr &variable, std::vector<VariablePtr> &equivalentVariables)
{
    for (size_t i = 0; i < variable->equivalentVariableCount(); ++i) {
        VariablePtr equivalentVariable = variable->equivalentVariable(i);

        if (std::find(equivalentVariables.begin(), equivalentVariables.end(), equivalentVariable) == equivalentVariables.end()) {
            equivalentVariables.push_back(equivalentVariable);

            recursiveEquivalentVariables(equivalentVariable, equivalentVariables);
        }
    }
}

std::vector<VariablePtr> equivalentVariables(const VariablePtr &variable)
{
    std::vector<VariablePtr> res = {variable};

    recursiveEquivalentVariables(variable, res);

    return res;
}

bool linkComponentVariableUnits(const ComponentPtr &component, DescriptionList &descriptionList)
{
    bool status = true;
    for (size_t index = 0; index < component->variableCount(); ++index) {
        auto v = component->variable(index);
        auto u = v->units();

        if (u != nullptr) {
            auto model = owningModel(u);
            if (model == owningModel(v)) {
                // Units are already linked, and exist in this model.
                continue;
            }
            if ((model == nullptr) && !isStandardUnit(u)) {
                model = owningModel(component);
                if (model->hasUnits(u->name())) {
                    v->setUnits(model->units(u->name()));
                } else {
                    descriptionList.push_back(std::make_pair(v, "Model does not contain the units '" + u->name() + "' required by variable '" + v->name() + "' in component '" + component->name() + "'."));
                    status = false;
                }
            } else if (model != nullptr) {
                descriptionList.push_back(std::make_pair(v, "The units '" + u->name() + "' assigned to variable '" + v->name() + "' in component '" + component->name() + "' belong to a different model, '" + model->name() + "'."));
                status = false;
            }
        }
    }
    return status;
}

bool traverseComponentEntityTreeLinkingUnits(const ComponentEntityPtr &componentEntity)
{
    DescriptionList descriptionList;
    return traverseComponentEntityTreeLinkingUnits(componentEntity, descriptionList);
}

bool traverseComponentEntityTreeLinkingUnits(const ComponentEntityPtr &componentEntity, DescriptionList &descriptionList)
{
    auto component = std::dynamic_pointer_cast<Component>(componentEntity);
    bool status = (component != nullptr) ?
                      linkComponentVariableUnits(component, descriptionList) :
                      true;
    for (size_t index = 0; index < componentEntity->componentCount(); ++index) {
        auto c = componentEntity->component(index);
        status = traverseComponentEntityTreeLinkingUnits(c, descriptionList) && status;
    }
    return status;
}

bool areComponentVariableUnitsUnlinked(const ComponentPtr &component)
{
    bool unlinked = false;
    for (size_t index = 0; index < component->variableCount() && !unlinked; ++index) {
        auto v = component->variable(index);
        auto u = v->units();
        if ((u != nullptr) && !isStandardUnit(u)) {
            auto model = owningModel(u);
            unlinked = (model == nullptr) || (owningModel(component) != model);
        }
    }
    return unlinked;
}

std::string replace(std::string string, const std::string &from, const std::string &to)
{
    auto index = string.find(from);

    return (index == std::string::npos) ?
               string :
               string.replace(index, from.length(), to);
}

bool equalEntities(const EntityPtr &owner, const std::vector<EntityPtr> &entities)
{
    std::vector<size_t> unmatchedIndex(entities.size());
    std::iota(unmatchedIndex.begin(), unmatchedIndex.end(), 0);
    for (const auto &entity : entities) {
        bool entityFound = false;
        size_t index = 0;
        for (index = 0; index < unmatchedIndex.size() && !entityFound; ++index) {
            size_t currentIndex = unmatchedIndex.at(index);
            auto model = std::dynamic_pointer_cast<Model>(owner);
            if (model != nullptr) {
                auto unitsOther = model->units(currentIndex);
                if (entity->equals(unitsOther)) {
                    entityFound = true;
                }
            } else {
                auto component = std::dynamic_pointer_cast<Component>(owner);
                auto variable = std::dynamic_pointer_cast<Variable>(entity);
                if (variable != nullptr) {
                    auto variableOther = component->variable(currentIndex);
                    if (variable->equals(variableOther)) {
                        entityFound = true;
                    }
                } else {
                    auto reset = std::dynamic_pointer_cast<Reset>(entity);
                    auto resetOther = component->reset(currentIndex);
                    if (reset->equals(resetOther)) {
                        entityFound = true;
                    }
                }
            }
        }
        if (entityFound) {
            // We are going to assume here that nobody is going to add more
            // than 2,147,483,647 units to this component. And much more than
            // that in a 64-bit environment.
            unmatchedIndex.erase(unmatchedIndex.begin() + ptrdiff_t(index) - 1);
        } else {
            return false;
        }
    }
    return true;
}

bool areEqual(const std::string &str1, const std::string &str2)
{
    return str1 == str2;
}

void recordUrl(const HistoryEpochPtr &historyEpoch, const ImportedEntityConstPtr &importedEntity)
{
    if (importedEntity->isImport()) {
        historyEpoch->mDestinationUrl = importedEntity->importSource()->url();
    }
}

HistoryEpochPtr createHistoryEpoch(const UnitsConstPtr &units, const std::string &sourceUrl, const std::string &destinationUrl)
{
    auto h = std::make_shared<HistoryEpoch>(units, sourceUrl, destinationUrl);
    if (destinationUrl.empty()) {
        recordUrl(h, units);
    }
    return h;
}

HistoryEpochPtr createHistoryEpoch(const ComponentConstPtr &component, const std::string &sourceUrl, const std::string &destinationUrl)
{
    auto h = std::make_shared<HistoryEpoch>(component, sourceUrl, destinationUrl);
    if (destinationUrl.empty()) {
        recordUrl(h, component);
    }
    return h;
}

std::string importeeModelUrl(const History &history, const std::string &url)
{
    for (auto i = history.size(); i-- > 0;) {
        auto historyEpoch = history[i];
        if (historyEpoch->mDestinationUrl != url) {
            return historyEpoch->mDestinationUrl;
        }
    }

    return ORIGIN_MODEL_REF;
}

bool checkForImportCycles(const History &history, const HistoryEpochPtr &h)
{
    return std::any_of(history.begin(), history.end(), [h](const auto &entry) {
        return ((h->mDestinationUrl == entry->mSourceUrl) || ((entry->mSourceUrl == ORIGIN_MODEL_REF) && (entry->mSourceModel != nullptr) && (entry->mSourceModel->equals(h->mDestinationModel))));
    });
}

std::string formDescriptionOfCyclicDependency(const History &history, const std::string &action)
{
    auto origin = history.front();
    auto model = origin->mSourceModel;
    bool isComponent = origin->mType == "component";
    std::string typeStringPrefix = isComponent ? "a " : "";
    std::string msgHeader = "Cyclic dependencies were found when attempting to " + action + " "
                            + typeStringPrefix + origin->mType + " in the model '"
                            + model->name() + "'. The dependency loop is:\n";
    HistoryEpochPtr h;
    size_t i = 0;
    std::string msgHistory;
    while (i < history.size()) {
        h = history[i];
        msgHistory += " - " + h->mType + " '" + h->mName + "' specifies an import from '" + h->mSourceUrl + "' to '" + h->mDestinationUrl + "'";
        if (i == history.size() - 2) {
            msgHistory += "; and\n";
        } else if (i == history.size() - 1) {
            msgHistory += ".";
        } else {
            msgHistory += ";\n";
        }
        ++i;
    }

    return msgHeader + msgHistory;
}

size_t nonCommentChildCount(const XmlNodePtr &node)
{
    size_t res = 0;
    auto childNode = node->firstChild();

    while (childNode != nullptr) {
        if (!childNode->isComment()) {
            ++res;
        }

        childNode = childNode->next();
    }

    return res;
}

XmlNodePtr nonCommentChildNode(const XmlNodePtr &node, size_t index)
{
    // Note: we assume that there is always a non-comment child at the given
    //       index, hence we never test res for nullptr.

    auto res = node->firstChild();
    auto childNodeIndex = res->isComment() ? MAX_SIZE_T : 0;

    while (childNodeIndex != index) {
        res = res->next();

        if (!res->isComment()) {
            ++childNodeIndex;
        }
    }

    return res;
}

size_t mathmlChildCount(const XmlNodePtr &node)
{
    size_t res = 0;
    auto childNode = node->firstChild();

    while (childNode != nullptr) {
        if (childNode->isMathmlElement()) {
            ++res;
        }

        childNode = childNode->next();
    }

    return res;
}

XmlNodePtr mathmlChildNode(const XmlNodePtr &node, size_t index)
{
    auto res = node->firstChild();
    auto childNodeIndex = res->isMathmlElement() ? 0 : MAX_SIZE_T;

    while ((res != nullptr) && (childNodeIndex != index)) {
        res = res->next();

        if ((res != nullptr) && res->isMathmlElement()) {
            ++childNodeIndex;
        }
    }

    return res;
}

std::vector<AnalyserVariablePtr> analyserVariables(const AnalyserVariablePtr &analyserVariable)
{
    std::vector<AnalyserVariablePtr> res;

    switch (analyserVariable->type()) {
    case AnalyserVariable::Type::CONSTANT:
        return analyserVariable->analyserModel()->constants();
    case AnalyserVariable::Type::COMPUTED_CONSTANT:
        return analyserVariable->analyserModel()->computedConstants();

    case AnalyserVariable::Type::ALGEBRAIC_VARIABLE:
        return analyserVariable->analyserModel()->algebraicVariables();
    default:
        break;
    }

    return {};
}

std::vector<AnalyserVariablePtr> analyserVariables(const AnalyserModelPtr &analyserModel)
{
    std::vector<AnalyserVariablePtr> res;

    if (analyserModel->voi() != nullptr) {
        res.push_back(analyserModel->voi());
    }

    auto states = analyserModel->states();

    res.insert(res.end(), states.begin(), states.end());

    auto constants = analyserModel->constants();
    auto computedConstants = analyserModel->computedConstants();
    auto algebraicVariables = analyserModel->algebraicVariables();
    auto externalVariables = analyserModel->externalVariables();

    res.insert(res.end(), constants.begin(), constants.end());
    res.insert(res.end(), computedConstants.begin(), computedConstants.end());
    res.insert(res.end(), algebraicVariables.begin(), algebraicVariables.end());
    res.insert(res.end(), externalVariables.begin(), externalVariables.end());

    return res;
}

std::vector<AnalyserVariablePtr> analyserVariables(const AnalyserEquationPtr &analyserEquation)
{
    auto res = analyserEquation->states();
    auto computedConstants = analyserEquation->computedConstants();
    auto algebraicVariables = analyserEquation->algebraicVariables();
    auto externalVariables = analyserEquation->externalVariables();

    res.insert(res.end(), computedConstants.begin(), computedConstants.end());
    res.insert(res.end(), algebraicVariables.begin(), algebraicVariables.end());
    res.insert(res.end(), externalVariables.begin(), externalVariables.end());

    return res;
}

} // namespace libcellml
