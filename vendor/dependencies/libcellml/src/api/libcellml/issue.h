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

#include "libcellml/exportdefinitions.h"
#include "libcellml/types.h"

#ifdef ERROR
#    undef ERROR
#endif

namespace libcellml {

/**
 * @brief The Issue class
 *
 * Base class for issues used with logger derived classes.
 */
class LIBCELLML_EXPORT Issue
{
    friend class Analyser;
    friend class Annotator;
    friend class GeneratorVariableTracker;
    friend class Importer;
    friend class Parser;
    friend class Printer;
    friend class Validator;

public:
    virtual ~Issue(); /**< Destructor, @private. */
    Issue(const Issue &rhs) = delete; /**< Copy constructor, @private. */
    Issue(Issue &&rhs) noexcept = delete; /**< Move constructor, @private. */
    Issue &operator=(Issue rhs) = delete; /**< Assignment operator, @private. */

    /**
     * @brief The issue Level enum class.
     *
     * Enum to describe the level of severity that a given issue has.
     */
    enum class Level
    {
        ERROR,
        WARNING,
        MESSAGE
    };

    /**
     * @brief The issue Reference enum class.
     *
     * Enum to describe the rule which the issue addresses.
     */
    enum class ReferenceRule
    {
        UNDEFINED,

        // Specification errors:
        XML,
        XML_UNEXPECTED_ELEMENT,
        XML_UNEXPECTED_CHARACTER,
        XML_UNEXPECTED_NAMESPACE,
        XML_ATTRIBUTE_HAS_NAMESPACE,
        XML_ID_ATTRIBUTE,
        MODEL_ELEMENT,
        MODEL_NAME,
        MODEL_NAME_VALUE,
        MODEL_CHILD,
        MODEL_MORE_THAN_ONE_ENCAPSULATION,
        IMPORT_ELEMENT,
        IMPORT_HREF,
        IMPORT_HREF_LOCATOR,
        IMPORT_CHILD,
        IMPORT_EQUIVALENT_INFOSET,
        IMPORT_UNITS_ELEMENT,
        IMPORT_UNITS_NAME,
        IMPORT_UNITS_NAME_VALUE,
        IMPORT_UNITS_NAME_UNIQUE,
        IMPORT_UNITS_UNITS_REFERENCE,
        IMPORT_UNITS_UNITS_REFERENCE_VALUE,
        IMPORT_UNITS_UNITS_REFERENCE_VALUE_TARGET,
        IMPORT_COMPONENT_ELEMENT,
        IMPORT_COMPONENT_NAME,
        IMPORT_COMPONENT_NAME_VALUE,
        IMPORT_COMPONENT_NAME_UNIQUE,
        IMPORT_COMPONENT_COMPONENT_REFERENCE,
        IMPORT_COMPONENT_COMPONENT_REFERENCE_VALUE,
        IMPORT_COMPONENT_COMPONENT_REFERENCE_TARGET,
        UNITS_ELEMENT,
        UNITS_NAME,
        UNITS_NAME_VALUE,
        UNITS_NAME_UNIQUE,
        UNITS_STANDARD,
        UNITS_CHILD,
        UNIT_ELEMENT,
        UNIT_UNITS,
        UNIT_UNITS_REFERENCE,
        UNIT_UNITS_CIRCULAR_REFERENCE,
        UNIT_ATTRIBUTE_OPTIONAL,
        UNIT_ATTRIBUTE_PREFIX_VALUE,
        UNIT_ATTRIBUTE_MULTIPLIER_VALUE,
        UNIT_ATTRIBUTE_EXPONENT_VALUE,
        COMPONENT_ELEMENT,
        COMPONENT_NAME,
        COMPONENT_NAME_VALUE,
        COMPONENT_NAME_UNIQUE,
        COMPONENT_CHILD,
        VARIABLE_ELEMENT,
        VARIABLE_ATTRIBUTE_REQUIRED,
        VARIABLE_NAME_VALUE,
        VARIABLE_NAME_UNIQUE,
        VARIABLE_UNITS_VALUE,
        VARIABLE_ATTRIBUTE_OPTIONAL,
        VARIABLE_INTERFACE_VALUE,
        VARIABLE_INITIAL_VALUE_VALUE,
        RESET_ELEMENT,
        RESET_ATTRIBUTE_REQUIRED,
        RESET_VARIABLE_REFERENCE,
        RESET_TEST_VARIABLE_REFERENCE,
        RESET_ORDER_VALUE,
        RESET_ORDER_UNIQUE,
        RESET_CHILD,
        RESET_RESET_VALUE_CHILD,
        RESET_TEST_VALUE_CHILD,
        TEST_VALUE_ELEMENT,
        TEST_VALUE_CHILD,
        RESET_VALUE_ELEMENT,
        RESET_VALUE_CHILD,
        MATH_ELEMENT,
        MATH_MATHML,
        MATH_CHILD,
        MATH_CI_VARIABLE_REFERENCE,
        MATH_CN_UNITS_ATTRIBUTE,
        MATH_CN_UNITS_ATTRIBUTE_REFERENCE,
        MATH_CN_BASE10,
        MATH_CN_FORMAT,
        ENCAPSULATION_ELEMENT,
        ENCAPSULATION_CHILD,
        COMPONENT_REF_ELEMENT,
        COMPONENT_REF_COMPONENT_ATTRIBUTE,
        COMPONENT_REF_COMPONENT_ATTRIBUTE_REFERENCE,
        COMPONENT_REF_COMPONENT_ATTRIBUTE_UNIQUE,
        COMPONENT_REF_CHILD,
        CONNECTION_ELEMENT,
        CONNECTION_COMPONENT1_ATTRIBUTE,
        CONNECTION_COMPONENT1_ATTRIBUTE_REFERENCE,
        CONNECTION_COMPONENT2_ATTRIBUTE,
        CONNECTION_COMPONENT2_ATTRIBUTE_REFERENCE,
        CONNECTION_EXCLUDE_SELF,
        CONNECTION_UNIQUE,
        CONNECTION_CHILD,
        MAP_VARIABLES_ELEMENT,
        MAP_VARIABLES_VARIABLE1_ATTRIBUTE,
        MAP_VARIABLES_VARIABLE1_ATTRIBUTE_REFERENCE,
        MAP_VARIABLES_VARIABLE2_ATTRIBUTE,
        MAP_VARIABLES_VARIABLE2_ATTRIBUTE_REFERENCE,
        MAP_VARIABLES_UNIQUE,

        // Secondary specification errors:
        DATA_REPR_IDENTIFIER_AT_LEAST_ONE_ALPHANUM,
        DATA_REPR_IDENTIFIER_BEGIN_EURO_NUM,
        DATA_REPR_IDENTIFIER_LATIN_ALPHANUM,

        // Issues not present in the normative specification:
        INVALID_ARGUMENT,

        // Importer class issues:
        IMPORTER_ERROR_IMPORTING_UNITS,
        IMPORTER_MISSING_FILE,
        IMPORTER_MISSING_COMPONENT,
        IMPORTER_MISSING_UNITS,
        IMPORTER_NULL_MODEL,
        IMPORTER_UNDEFINED_MODEL,
        IMPORTER_UNRESOLVED_IMPORTS,

        // Analyser issues:
        ANALYSER_EQUATION_NOT_EQUALITY_STATEMENT,
        ANALYSER_UNITS,
        ANALYSER_UNLINKED_UNITS,
        ANALYSER_VARIABLE_INITIALISED_MORE_THAN_ONCE,
        ANALYSER_VARIABLE_INITIALISED_USING_ALGEBRAIC_VARIABLE,
        ANALYSER_VARIABLE_INITIALISED_USING_VARIABLE_WITH_DIFFERENT_UNITS,
        ANALYSER_VOI_INITIALISED,
        ANALYSER_VOI_SEVERAL,
        ANALYSER_ODE_NOT_FIRST_ORDER,
        ANALYSER_VARIABLE_UNUSED,
        ANALYSER_STATE_NOT_INITIALISED,
        ANALYSER_STATE_RATE_AS_ALGEBRAIC_VARIABLE,
        ANALYSER_VARIABLE_UNDERCONSTRAINED,
        ANALYSER_VARIABLE_OVERCONSTRAINED,
        ANALYSER_EXTERNAL_VARIABLE_DIFFERENT_MODEL,
        ANALYSER_EXTERNAL_VARIABLE_VOI,
        ANALYSER_EXTERNAL_VARIABLE_USE_PRIMARY_VARIABLE,

        // Annotator issues:
        ANNOTATOR_ID_NOT_FOUND,
        ANNOTATOR_ID_NOT_UNIQUE,
        ANNOTATOR_NO_MODEL,
        ANNOTATOR_INCONSISTENT_TYPE,
        ANNOTATOR_NULL_MODEL,

        // Generator issues:
        GENERATOR_NULL_MODEL,
        GENERATOR_NULL_VARIABLE,
        GENERATOR_VOI_VARIABLE_ALWAYS_TRACKED,
        GENERATOR_VOI_VARIABLE_NOT_UNTRACKABLE,
        GENERATOR_STATE_VARIABLE_ALWAYS_TRACKED,
        GENERATOR_STATE_VARIABLE_NOT_UNTRACKABLE,
        GENERATOR_EXTERNAL_VARIABLE_ALWAYS_TRACKED,
        GENERATOR_EXTERNAL_VARIABLE_NOT_UNTRACKABLE,
        GENERATOR_NLA_BASED_VARIABLE_ALWAYS_TRACKED,
        GENERATOR_NLA_BASED_VARIABLE_NOT_UNTRACKABLE,
        GENERATOR_EXTERNALLY_NEEDED_VARIABLE_ALWAYS_TRACKED,
        GENERATOR_EXTERNALLY_NEEDED_VARIABLE_NOT_UNTRACKABLE,

        // Placeholder for further references:
        UNSPECIFIED
    };

    /**
     * @brief Get the description for this issue.
     *
     * Get the @c std::string description for why this issue was raised.
     *
     * @return The @c std::string description of the issue.
     */
    std::string description() const;

    /**
     * @brief Get the level of this issue.
     *
     * Get the @c Issue::Level of this issue. If no level has been set for
     * this issue, will return Level::ERROR.
     *
     * @return The @c Issue::Level set for this issue.
     */
    Level level() const;

    /**
     * @brief Get the @c enum ReferenceRule of this issue.
     *
     * Get the @c enum @c ReferenceRule value of this issue. If no reference
     * rule has been set for this issue, return ReferenceRule::UNDEFINED.
     *
     * @return The @c ReferenceRule for this issue.
     */
    ReferenceRule referenceRule() const;

    /**
     * @brief Get the item for this issue.
     *
     * Get the item for this issue as an @ref AnyCellmlElementPtr.
     *
     * @return The @ref AnyCellmlElementPtr for this issue.
     */
    AnyCellmlElementPtr item() const;

    /**
     * @brief Get the @c url of this issue.
     *
     * Get the @c url to which the user should refer for more information on this issue.
     *
     * @return The @c url for this issue.
     */
    std::string url() const;

    /**
     * @brief Get the @c std::string heading associated with the @c enum ReferenceRule for this issue.
     *
     * Get the @c std::string libCellML Reference heading associated with the @c enum ReferenceRule
     * for this issue. If no rule has been set for this issue, will return an empty string.
     *
     * @return The @c std::string libCellML Reference heading relevant to this issue.
     */
    std::string referenceHeading() const;

private:
    Issue(); /**< Constructor, @private. */

    struct IssueImpl; /**< Forward declaration for pImpl idiom, @private. */
    IssueImpl *mPimpl; /**< Private member to implementation pointer, @private. */
};

} // namespace libcellml
