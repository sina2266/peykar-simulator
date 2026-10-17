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

#include <algorithm>
#include <cctype>
#include <limits>
#include <map>
#include <string>
#include <vector>

#include "libcellml/types.h"
#include "libcellml/variable.h"

#include "internaltypes.h"
#include "xmldoc.h"

namespace libcellml {

/**
 * Base URLs of sites from which Issue::url() strings will be constructed.
 */
static const std::string baseSpecificationUrl = "https://cellml-specification.readthedocs.io/en/latest/reference/formal_and_informative/";
static const std::string docsUrl = "https://libcellml.org/documentation/guides/latest/runtime_codes/index";

static const size_t MAX_SIZE_T = std::numeric_limits<size_t>::max();

/**
 * Vector of base units.
 */
static const NameList baseUnitsList = {
    "ampere",
    "candela",
    "dimensionless",
    "kelvin",
    "kilogram",
    "metre",
    "mole",
    "second"};

/**
 *  Map connecting standard built-in units to their base unit components and their exponents.
 */
static const std::map<std::string, std::map<std::string, double>> standardUnitsList = {
    {"ampere", {{"ampere", 1.0}}},
    {"becquerel", {{"second", -1.0}}},
    {"candela", {{"candela", 1.0}}},
    {"coulomb", {{"ampere", 1.0}, {"second", 1.0}}},
    {"dimensionless", {{"dimensionless", 1.0}}},
    {"farad", {{"ampere", 2.0}, {"kilogram", -1.0}, {"metre", -2.0}, {"second", 4.0}}},
    {"gram", {{"kilogram", 1.0}}},
    {"gray", {{"metre", 2.0}, {"second", -2.0}}},
    {"henry", {{"ampere", -2.0}, {"kilogram", 1.0}, {"metre", 2.0}, {"second", -2.0}}},
    {"hertz", {{"second", -1.0}}},
    {"joule", {{"kilogram", 1.0}, {"metre", 2.0}, {"second", -2.0}}},
    {"katal", {{"mole", 1.0}, {"second", -1.0}}},
    {"kelvin", {{"kelvin", 1.0}}},
    {"kilogram", {{"kilogram", 1.0}}},
    {"litre", {{"metre", 3.0}}},
    {"lumen", {{"candela", 1.0}}},
    {"lux", {{"candela", 1.0}, {"metre", -2.0}}},
    {"metre", {{"metre", 1.0}}},
    {"mole", {{"mole", 1.0}}},
    {"newton", {{"kilogram", 1.0}, {"metre", 1.0}, {"second", -2.0}}},
    {"ohm", {{"ampere", -2.0}, {"kilogram", 1.0}, {"metre", 2.0}, {"second", -3.0}}},
    {"pascal", {{"kilogram", 1.0}, {"metre", -1.0}, {"second", -2.0}}},
    {"radian", {{"dimensionless", 1.0}}},
    {"second", {{"second", 1.0}}},
    {"siemens", {{"ampere", 2.0}, {"kilogram", -1.0}, {"metre", -2.0}, {"second", 3.0}}},
    {"sievert", {{"metre", 2.0}, {"second", -2.0}}},
    {"steradian", {{"dimensionless", 1.0}}},
    {"tesla", {{"ampere", -1.0}, {"kilogram", 1.0}, {"second", -2.0}}},
    {"volt", {{"ampere", -1.0}, {"kilogram", 1.0}, {"metre", 2.0}, {"second", -3.0}}},
    {"watt", {{"kilogram", 1.0}, {"metre", 2.0}, {"second", -3.0}}},
    {"weber", {{"ampere", -1.0}, {"kilogram", 1.0}, {"metre", 2.0}, {"second", -2.0}}}};

/**
 * Map connecting standard built-in units to the multiplier exponent of their base unit components.
 */
static const std::map<std::string, double> standardMultiplierList = {
    {"ampere", 0.0},
    {"becquerel", 0.0},
    {"candela", 0.0},
    {"coulomb", 0.0},
    {"dimensionless", 0.0},
    {"farad", 0.0},
    {"gram", -3.0},
    {"gray", 0.0},
    {"henry", 0.0},
    {"hertz", 0.0},
    {"joule", 0.0},
    {"katal", 0.0},
    {"kelvin", 0.0},
    {"kilogram", 0.0},
    {"litre", -3.0},
    {"lumen", 0.0},
    {"lux", 0.0},
    {"metre", 0.0},
    {"mole", 0.0},
    {"newton", 0.0},
    {"ohm", 0.0},
    {"pascal", 0.0},
    {"radian", 0.0},
    {"second", 0.0},
    {"siemens", 0.0},
    {"sievert", 0.0},
    {"steradian", 0.0},
    {"tesla", 0.0},
    {"volt", 0.0},
    {"watt", 0.0},
    {"weber", 0.0}};

/**
 * List of MathML elements supported by CellML.
 */
static const NameList supportedMathMLElements = {
    "ci", "cn", "sep", "apply", "piecewise", "piece", "otherwise", "eq", "neq", "gt", "lt", "geq", "leq", "and", "or",
    "xor", "not", "plus", "minus", "times", "divide", "power", "root", "abs", "exp", "ln", "log", "floor",
    "ceiling", "min", "max", "rem", "diff", "bvar", "logbase", "degree", "sin", "cos", "tan", "sec", "csc",
    "cot", "sinh", "cosh", "tanh", "sech", "csch", "coth", "arcsin", "arccos", "arctan", "arcsec", "arccsc",
    "arccot", "arcsinh", "arccosh", "arctanh", "arcsech", "arccsch", "arccoth", "pi", "exponentiale",
    "notanumber", "infinity", "true", "false"};

/**
 * @brief Map to convert an interface type into its string form.
 *
 * An internal map used to convert a Variable InterfaceType enum class member into its string form.
 */
static const std::map<Variable::InterfaceType, std::string> interfaceTypeToString = {
    {Variable::InterfaceType::NONE, "none"},
    {Variable::InterfaceType::PRIVATE, "private"},
    {Variable::InterfaceType::PUBLIC, "public"},
    {Variable::InterfaceType::PUBLIC_AND_PRIVATE, "public_and_private"}};

/**
 * @brief Check whether the @p in @c std::string can be converted to a basic @c double.
 *
 * Try to convert the @p in @c std::string to a basic @c double. Return true if the conversion succeeded and @c false
 * if it didn't.
 *
 * If the @p in is not a CellML basic real the conversion will not succeed.
 *
 * @sa isCellMLBasicReal
 *
 * @param in The @c std::string value to convert to a basic @c double.
 *
 * @return @c true if @p in could be converted to a basic @c double and @c false otherwise.
 */
bool canConvertToBasicDouble(const std::string &in);

/**
 * @brief Convert the @p in @c std::string to a @c double.
 *
 * Convert the @p in @c std::string to a @c double.
 * Returns @c true if the conversion succeeded
 * and @c false if it didn't.
 *
 * If the @p in is not a CellML real the conversion will not succeed.
 * If the @p in is a number bigger than a double can represent the
 * conversion will also not succeed.
 *
 * @sa isCellMLReal
 *
 * @param in The @c std::string value to convert to a @c double.
 * @param out The value of the @p in as a double if the conversion was successful,
 * left unchanged if the conversion fails.
 *
 * @return @c true if the conversion succeeded, @c false otherwise.
 */
bool convertToDouble(const std::string &in, double &out);

/**
 * @brief Convert the @p in @c std::string to an @c int.
 *
 * Convert the @p in @c std::string to an @c int.
 * Returns @c true if the conversion succeeded
 * and @c false if it didn't.
 *
 * If @p in is not a CellML integer the conversion will not succeed.
 * If @p in is a number bigger than an int can represent the conversion
 * will also not succeed.
 *
 * @sa isCellMLInteger
 *
 * @param in The @c std::string value to convert to an @c int.
 * @param out The integer value of the @p in.
 *
 * @return @c true if the conversion was successful and @c false if it wasn't.
 */
bool convertToInt(const std::string &in, int &out);

/**
 * @brief Convert a units prefix to an int.
 *
 * Converts the given units prefix (@p in) into its equivalent integer value.
 * If given, sets the parameter @p ok to @c true if the conversion succeeded
 * and @c false if it didn't.
 *
 * @param in The @c std::string value to convert to an @c int.
 * @param ok Optional parameter returns @c true if the conversion was successful and @c false if it wasn't.
 *
 * @return The integer value of the @p prefix.
 */
int convertPrefixToInt(const std::string &in, bool *ok = nullptr);

/**
 * @brief Convert a @c int to @c std::string format.
 *
 * Convert the @p value to @c std::string representation.
 *
 * @param value The @c int value of the integer to convert.
 *
 * @return @c std::string representation of the @p value.
 */
std::string convertToString(int value);

/**
 * @brief Convert a @c size_t to @c std::string format.
 *
 * Convert the @p value to @c std::string representation.
 *
 * @overload std::string convertToString(int value)
 *
 * @param value The @c size_t value of the integer to convert.
 *
 * @return @c std::string representation of the @p value.
 */
std::string convertToString(size_t value);

/**
 * @brief Convert a @c double to @c std::string format.
 *
 * @overload
 *
 * Convert the @p value to @c std::string representation.
 *
 * @param value The @c double value number to convert.
 * @param fullPrecision Whether the @p value is converted using full precision.
 *
 * @return @c std::string representation of the @p value.
 */
std::string convertToString(double value, bool fullPrecision = true);

/**
 * @brief Check if the @p input @c std::string has any non-whitespace characters.
 *
 * If the @p input @c std::string has any non-whitespace characters, return
 * @c true, otherwise return @c false.
 *
 * @param input The string to check for non-whitespace characters.
 *
 * @return @c true if @p input contains non-whitespace characters and @c false otherwise.
 */
bool hasNonWhitespaceCharacters(const std::string &input);

/**
 * @brief Test if the @p candidate @c std::string is a valid non-negative CellML integer.
 *
 * The candidate string must consist entirely of european numeric characters and it must also be
 * expressed in base 10 if it is to be considered a non-negative CellML integer.
 *
 * @param candidate The string to check if it is a non-negative CellML integer.
 * @return @c true if the @p candidate is a non-negative CellML integer and @c false otherwise.
 */
bool isNonNegativeCellMLInteger(const std::string &candidate);

/**
 * @brief Test if the @p candidate @c std::string is a valid CellML integer.
 *
 * The candidate string must consist of european numeric characters and optionally the
 * basic Latin hyphen character '-'.  When the integer being represented is negative
 * the candidate string must start with the '-' character followed by the absolute
 * value of the integer.
 *
 * @param candidate The string to test and determine whether or not it is a CellML integer.
 * @return @c true if the @p candidate is a CellML integer and @c false otherwise.
 */
bool isCellMLInteger(const std::string &candidate);

/**
 * @brief Test if the @p candidate @c std::string is in the form of a CellML basic real.
 *
 * The candidate string must consist of european numeric characters.  It may optionally
 * have a basic Latin hyphen character '-' to indicate sign.  It may also optionally
 * use the basic Latin fullstop character '.' to indicate a decimal point.  The candidate
 * string must represent a number in base 10.
 *
 * @param candidate The string to test and determine whether or not it is a CellML basic real.
 * @return @c true if the @p candidate is a CellML basic real and @c false otherwise.
 */
bool isCellMLBasicReal(const std::string &candidate);

/**
 * @brief Test if the @p candidate @c std::string is in the form of a CellML real.
 *
 * The candidate string must consist of european numeric characters.  It may optionally
 * have a basic Latin hyphen character '-' to indicate sign.  It may also optianally
 * use the basic Latin fullstop character '.' to indicate a decimal point.  The candidate
 * string must represent a number in base 10.  The basic Latin 'e' or 'E' character
 * may be used to indicate the exponent.  The exponent must be described as a
 * CellML integer.
 *
 * @param candidate The string to test and determine whether or not it is a CellML real.
 * @return @c true if the @p candidate is a CellML real and @c false otherwise.
 */
bool isCellMLReal(const std::string &candidate);

/**
 * @brief Test if @p a @c double and @p b @c double are equal.
 *
 * Return @c true if @p a @c double and @p b @c double are equal,
 * otherwise return @c false.
 *
 * @param a The first @c double value to test.
 * @param b The second @c double value to test.
 *
 * @return @c true if @p a and @p b are equal and @c false otherwise.
 */
bool areEqual(double a, double b);

/**
 * @brief Decide if two doubles are nearly equal.
 *
 * Test two doubles to determine if they are close enough
 * to be considered equal.
 *
 * Uses a modified form of comparing floats:
 *
 *   https://bitbashing.io/comparing-floats.html
 *
 * @param a A @c double to test.
 * @param b A @c double to test.
 *
 * @return @c true if the given doubles are considered close, @c false otherwise.
 */
bool areNearlyEqual(double a, double b);

/**
 * @brief Compare strings to determine if they are equal.
 *
 * Compare the given strings to determine if they are equal or not.
 * The current test is a simplistic comparison of string equality.
 *
 * @param str1 The first parameter to compare against parameter two.
 * @param str2 The second parameter to compare against parameter one.
 *
 * @return Return @c true if the @p str1 is equal to @p str2, @c false otherwise.
 */
bool areEqual(const std::string &str1, const std::string &str2);

/**
 * @brief Get all the imported components from the given component entity.
 *
 * Get all the imported components from the given component entity.  Tracing
 * through the component hierarchy to find them.
 *
 * @param componentEntity The component entity to search.
 *
 * @return A vector of @ref ComponentPtr that are imported components.
 */
std::vector<ComponentPtr> getImportedComponents(const ComponentEntityConstPtr &componentEntity);

/**
 * @brief Get all imported units from a model.
 *
 * Get all imported units from a model.
 *
 * @param model The model to search for imported units.
 *
 * @return A vector of @ref UnitsPtr that are imported units.
 */
std::vector<UnitsPtr> getImportedUnits(const ModelConstPtr &model);

/**
 * @brief Remove the given component from the given entity.
 *
 * The entity given can be either a @c Model or @c Component and as such is
 * expected to be derived from @c ComponentEntity.  The component to be removed
 * is expected to be a direct child of the given entity.  That is to say the component
 * will only be removed if it is a direct child of the entity itself, the component
 * hierarchy of the entity will *not* be searched for the location of the component.
 *
 * @param entity The entity to remove the component from.
 * @param component The component to remove from the entity.
 */
void removeComponentFromEntity(const EntityPtr &entity, const ComponentPtr &component);

/**
 * @brief Check if the provided @p name is a standard unit.
 *
 * Checks if the provided @p name is one of the standard units in the
 * @ref libcellml::Units::StandardUnit @c enum. Returns @c true if @p name is a standard unit
 * and @c false otherwise.
 *
 * @param name The @c std::string name to check against the list of standard units.
 *
 * @return @c true if @name is a standard unit and @c false otherwise.
 */
bool isStandardUnitName(const std::string &name);

/**
 * @brief Test if the provided @ref Units is a standard unit.
 *
 * Tests to determine if the provided @p units is equivalent to
 * a standard unit.  Returns @c true if the @p units is a standard unit
 * and @c false otherwise.
 *
 * @param units The @ref Units to test.
 *
 * @return @c true if @p units is a standard unit, @c false otherwise.
 */
bool isStandardUnit(const UnitsPtr &units);

/**
 * @brief Check if the provided @p name is a standard prefix.
 *
 * Checks if the provided @p name is one of the standard prefixes in the
 * @c Prefix @c enum. Returns @c true if @name is a standard prefix
 * and @c false otherwise.
 *
 * @param name The @c std::string name to check against the list of standard prefixes.
 *
 * @return @c true if @p name is a standard prefix and @c false otherwise.
 */
bool isStandardPrefixName(const std::string &name);

/**
 * @brief Get the index of the @p variable in the @p component.
 *
 * Searches through the @p component for the @p variable returning the index
 * of the @p variable if it was found.  If the @p variable was not found then
 * the number of variables in the @p component is returned.
 *
 * @param component The @c ComponentPtr to search for the @c VariablePtr in.
 * @param variable The @c VariablePtr to return the index of.
 *
 * @return The index of the @p variable found in the component.  Returns the
 * number of variables in the component if the variable was not found.
 */
size_t indexOf(const VariablePtr &variable, const ComponentConstPtr &component);

/**
 * @brief Test to determine if @p variable1 and @p variable2 are equivalent.
 *
 * Test to see if @p variable1 is the same as or equivalent to @p variable2.
 * Returns @c true if @p variable1 is the same as or equivalent to @p variable2
 * and @c false otherwise.
 *
 * @param variable1 The @c Variable to test if it is equivalent to @p variable2.
 * @param variable2 The @c Variable that is potentially equivalent to
 * @p variable1.
 *
 * @return @c true if @p variable1 is equivalent to @p variable2 and @c false
 * otherwise.
 */
bool areEquivalentVariables(const VariablePtr &variable1,
                            const VariablePtr &variable2);

/**
 * @brief Test to determine if @p entity1 is a child of @p entity2.
 *
 * Test to see if @p entity1 is a child of @p entity2.  Returns @c true if
 * @p entity1 is a child of @p entity2 and @c false otherwise.
 *
 * @param entity1 The @c ParentedEntity to test if it is a child of @p entity2.
 * @param entity2 The @c ParentedEntity that is potentially the parent of @p entity1.
 *
 * @return @c true if @p entity1 is a child of @p entity2 and @c false otherwise.
 */
bool isEntityChildOf(const ParentedEntityPtr &entity1, const ParentedEntityPtr &entity2);

/**
 * @brief Test to determine if @p entity1 and @p entity2 are siblings.
 *
 * Test to determine if @p entity1 and @p entity2 are siblings.  Returns
 * @c true if @p entity1 and @p entity2 are siblings, @c false otherwise.
 *
 * @param entity1 An @c ParentedEntity to test if it is a sibling to @p entity2.
 * @param entity2 An @c ParentedEntity to test if it is a sibling to @p entity1.
 *
 * @return @c true if @p entity1 and @p entity2 are siblings, @c false otherwise.
 */
bool areEntitiesSiblings(const ParentedEntityPtr &entity1, const ParentedEntityPtr &entity2);

/**
 * @brief Determine the interface type of the @p variable.
 *
 * Determine the interface type of the given @p variable. For variables with
 * at least one equivalent variable, returning an interface type of
 * Variable::InterfaceType::NONE indicates an error.
 *
 * @param variable The variable to determine the interface type for.
 *
 * @return The @p variable's interface type.
 */
Variable::InterfaceType determineInterfaceType(const VariablePtr &variable);

/**
 * @brief Traverse the component tree looking for variables with equivalences.
 *
 * Search through the component tree starting at @p component looking for variables
 * with equivalences.  Variables found in the component tree with equivalences are added
 * to the @p variables list.
 *
 * @param component The @c Component to search.
 * @param variables The list of equivalent variables found.
 */
void findAllVariablesWithEquivalences(const ComponentPtr &component, VariablePtrs &variables);

/**
 * @brief Split a string.
 *
 * Split the given string with the given delimiter.  If a delimiter is not given
 * then ';' is used.  If the delimiter is not found a copy of @p content is returned.
 *
 * @param content The @c std::string to split.
 * @param delimiter The delimiter to split the string with, default ';'.
 *
 * @return A @c std::vector of @c std::strings.
 */
Strings split(const std::string &content, const std::string &delimiter = ";");

/**
 * @brief Trim whitespace from the front of a string (in place).
 *
 * Remove whitespace from the front of a string, modifying the passed string.
 *
 * @param s The @c std::string to trim.
 */
static inline void leftTrim(std::string &s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
                return std::isspace(ch) == 0;
            }));
}

/**
 * @brief Trim whitespace from the end of a string (in place).
 *
 * Remove whitespace from the end of a string, modifying the passed string.
 *
 * @param s The @c std::string to trim.
 */
static inline void rightTrim(std::string &s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
                return std::isspace(ch) == 0;
            }).base(),
            s.end());
}

/**
 * @brief Trim whitespace from the beginning and end of a string(in place).
 *
 * Remove whitespace from the beginning and end of a string, modifying the passed
 * string.
 *
 * @param s The @c std::string to trim.
 */
static inline void trim(std::string &s)
{
    leftTrim(s);
    rightTrim(s);
}

/**
 * @brief Trim whitespace from the beginning and end of a string.
 *
 * Remove whitespace from the beginning and end of a string
 * returning the result.
 *
 * @param s The @c std::string to trim.
 *
 * @return The trimmed string.
 */
static inline std::string trimCopy(std::string s)
{
    trim(s);
    return s;
}

/**
 * @brief Replace text in string.
 *
 * Replace the @c std::string @p from in @p string with @c std::string @p to.
 * If the string @p from is not found in @p string then the @p string is returned unchanged.
 *
 * @param string The string to make the substution in.
 * @param from The string to replace.
 * @param to The replacement string.
 *
 * @return The modified string.
 */
std::string replace(std::string string, const std::string &from, const std::string &to);

/**
 * @brief Collect all existing identifier attributes within the given model.
 *
 * @param model The @c ModelPtr to interrogate.
 *
 * @return An @c IdList collection of existing identifiers.
 */
IdList listIds(const ModelPtr &model);

/**
 * @brief Creates an identifier string for a "type" object, unique in the context of @p idList.
 *
 * The identifier format is a 6-digit hexadecimal string.
 *
 * @return A string representing a unique identifier.
 */
std::string makeUniqueId(IdList &idList);

/**
 * Function to support linking units names to their corresponding @ref Units items.
 *
 * @param component The component to check.
 * @param descriptionList A @c std::vector of variables paired with a description.
 *
 * @return @c true if all variables have been successfully linked to units, @c false otherwise.
 */
bool linkComponentVariableUnits(const ComponentPtr &component, DescriptionList &descriptionList);

/**
 * @overload
 *
 * @brief Utility function used when linking units names to their corresponding @ref Units items.
 *
 * Returns @c true if all variables in the component can be linked to their units, or
 * @c false otherwise.
 *
 * @param componentEntity The component entity to check.
 *
 * @return @c true upon success; @c false if not all variables could be linked to units.
 */
bool traverseComponentEntityTreeLinkingUnits(const ComponentEntityPtr &componentEntity);

/**
 * @overload
 *
 *  Utility function used when linking units names to their corresponding @ref Units items.
 *
 * @param componentEntity The component entity to check.
 * @param descriptionList A @c std::vector of variables paired with a description.
 *
 * @return @c true upon success; @c false if not all variables could be linked to units.
 * */
bool traverseComponentEntityTreeLinkingUnits(const ComponentEntityPtr &componentEntity, DescriptionList &descriptionList);

/**
 * @brief Test whether a component contains variables naming units which have not yet
 *        been linked to @ref Units items.
 *
 * Utility function used when linking units names to their corresponding @ref Units items. It
 * will return a value of @c true when there are variables without linked units, or @c false
 * otherwise.
 *
 * @param component The component to check.
 *
 * @return @c true when unlinked variables are found, @c false otherwise.
 */
bool areComponentVariableUnitsUnlinked(const ComponentPtr &component);

/**
 * @brief Create a connection map for the given variables.
 *
 * Create a map of variables that belong to the same connection as
 * the connection created by the equivalent variables @p variable1 and @p variable2.
 *
 * @param variable1 A variable in the connection.
 * @param variable2 A variable in the connection.
 *
 * @return A map of connections.
 */
ConnectionMap createConnectionMap(const VariablePtr &variable1, const VariablePtr &variable2);

/**
 * @brief Make a list of all variables equivalent to the given variable.
 *
 * Collect all the equivalent variables of the given @p variable and return
 * them as a list of @ref VariablePtr.
 *
 * @param variable The variable to find equivalent variables of.
 *
 * @return A @c std::vector of @ref VariablePtr.
 */
std::vector<VariablePtr> equivalentVariables(const VariablePtr &variable);

/**
 * @brief Test the given @p entities are equal to entities in @p owner.
 *
 * Test to see if all the entities given in @p entities are equal to
 * entities in @p owner.  The order that the entities appear in is not
 * taken into account.
 *
 * @param owner The owner to compare entities with.
 * @param entities The list of entities to equate.
 *
 * @return @c true if all the entities in @p entities are equal to entities in the @p owner.
 */
bool equalEntities(const EntityPtr &owner, const std::vector<EntityPtr> &entities);

/**
 * @brief Get all the import sources in the @p model.
 *
 * Get all the import sources from the imported @ref Component s
 * and @ref Units in the given @p model.
 *
 * @param model The model to find all import sources from.
 *
 * @return A @c std::vector of all the @ref ImportSource s found in the model.
 */
std::vector<ImportSourcePtr> getAllImportSources(const ModelConstPtr &model);

/**
 * @brief Return the @ref IndexStack for the given @p component.
 *
 * Return the @ref IndexStack for the given @p component.
 *
 * @param component The component to find the index stack for.
 *
 * @return An @ref IndexStack.
 */
IndexStack indexStackOf(const ComponentPtr &component);

/**
 * @brief Create a history epoch for a @ref Units with optional destination URL.
 *
 * Create a history epoch for a @ref Units.  If a destination URL is not given
 * then it will be taken from the import source if the @p units is an imported units.
 *
 * @param units The @ref Units to create a history entry for.
 * @param sourceUrl The source URL for the units.
 * @param destinationUrl The optional destination URL for the units.
 *
 * @return The history epoch.
 */
HistoryEpochPtr createHistoryEpoch(const UnitsConstPtr &units, const std::string &sourceUrl, const std::string &destinationUrl = "");

/**
 * @brief Create a history epoch for a @ref Component with optional destination URL.
 *
 * Create a history epoch for a @ref Component.  If a destination URL is not given
 * then it will be taken from the import source if the @p component is an imported component.
 *
 * @param component The @ref Component to create a history entry for.
 * @param sourceUrl The source URL for the component.
 * @param destinationUrl The optional destination URL for the component.
 *
 * @return The history epoch.
 */
HistoryEpochPtr createHistoryEpoch(const ComponentConstPtr &component, const std::string &sourceUrl, const std::string &destinationUrl = "");

/**
 * @brief Figure out the URL of the importee from the history.
 *
 * Look through the history to figure out the importee import URL.
 * @ref ORIGIN_MODEL_REF is returned by default.
 *
 * @param history The history of locations visited.
 * @param url The destination URL of the imported entity.
 * @return A std::string.
 */
std::string importeeModelUrl(const History &history, const std::string &url);

/**
 * @brief Check through the @p history and determine if @p h has already been visited.
 *
 * Check through the @p history and determine if @p h has already been visited.
 *
 * @param history The history of locations visited.
 * @param h The epoch to check for existence.
 * @return @c true if @p h is already present in @p history, @c false otherwise.
 */
bool checkForImportCycles(const History &history, const HistoryEpochPtr &h);

/**
 * @brief Form the description for a cyclic dependency.
 *
 * Form the description for a cyclic dependency.
 *
 * @param history The history of the cyclic dependency.
 * @param action The action that made cyclic dependency, e.g. "resolve", "flatten".
 *
 * @return The @c std::string description of the cyclic dependency.
 */
std::string formDescriptionOfCyclicDependency(const History &history, const std::string &action);

void recordVariableEquivalences(const ComponentPtr &component, EquivalenceMap &equivalenceMap, IndexStack &indexStack);
void generateEquivalenceMap(const ComponentPtr &component, EquivalenceMap &map, IndexStack &indexStack);
void applyEquivalenceMapToModel(const EquivalenceMap &map, const ModelPtr &model);
NameList componentNames(const ModelPtr &model);
NameList unitsNamesUsed(const ComponentPtr &component);
EquivalenceMap rebaseEquivalenceMap(const EquivalenceMap &map, const IndexStack &originStack, const IndexStack &destinationStack);
std::vector<UnitsPtr> unitsUsed(const ModelPtr &model, const ComponentConstPtr &component);
ComponentNameMap createComponentNamesMap(const ComponentPtr &component);

/**
 * @brief Change all matching cn CellML units attributes names.
 *
 * Find all cn elements with a CellML units attribute with the value @p oldName
 * and replace it with @p newName.
 *
 * The find and replace is done recursively through the component's component
 * tree hierarchy.
 *
 * @param component The component to find and replace in.
 * @param oldName The old name to match.
 * @param newName The new name to replace the old name with.
 */
void findAndReplaceComponentsCnUnitsNames(const ComponentPtr &component, const std::string &oldName, const std::string &newName);

/**
 * @brief Return the number of non-comment children.
 *
 * Return the number of non-comment children for the given node.
 *
 * @param node The node for which we want the number of non-comment children.
 *
 * @return The number of non-comment children.
 */
size_t nonCommentChildCount(const XmlNodePtr &node);

/**
 * @brief Return the non-comment child at a given index.
 *
 * Return the non-comment child, at @p index, of the given node.
 *
 * @param node The node from which we want the non-comment child at @p index.
 * @param index The index of the non-comment child.
 *
 * @return The non-comment child at @p index.
 */
XmlNodePtr nonCommentChildNode(const XmlNodePtr &node, size_t index);

/**
 * @brief Return the number of MathML children.
 *
 * Return the number of MathML children for the given node.
 *
 * @param node The node for which we want the number of MathML children.
 *
 * @return The number of MathML children.
 */
size_t mathmlChildCount(const XmlNodePtr &node);

/**
 * @brief Return the index'th MathML child for the given node.
 *
 * Return the @p index'th MathML child for the given node.
 *
 * @param node The node from which we want the @p index'th MathML child.
 * @param index The index of the MathML child.
 *
 * @return The @p index'th MathML child.
 */
XmlNodePtr mathmlChildNode(const XmlNodePtr &node, size_t index);

/**
 * @brief Return the analyser variables of the same type as the given analyser variable.
 *
 * Return the analyser variables of the same type as the given analyser variable.
 *
 * @param analyserVariable The analyser variable for which we want the analyser variables of the same type.
 *
 * @return The analyser variables of the same type as the given analyser variable.
 */
std::vector<AnalyserVariablePtr> analyserVariables(const AnalyserVariablePtr &analyserVariable);

/**
 * @brief Return the analyser variables in the given analyser model.
 *
 * Return the analyser variables in the given analyser model.
 *
 * @param analyserModel The analyser model for which we want the analyser variables.
 *
 * @return The analyser variables in the given analyser model.
 */
std::vector<AnalyserVariablePtr> analyserVariables(const AnalyserModelPtr &analyserModel);

/**
 * @brief Return the analyser variables in the given analyser equation.
 *
 * Return the analyser variables in the given analyser equation.
 *
 * @param analyserEquation The analyser equation for which we want the analyser variables.
 *
 * @return The analyser variables in the given analyser equation.
 */
std::vector<AnalyserVariablePtr> analyserVariables(const AnalyserEquationPtr &analyserEquation);

} // namespace libcellml
