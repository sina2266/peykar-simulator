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

#include "libcellml/issue.h"

#include "issue_p.h"
#include "utilities.h"

namespace libcellml {

IssuePtr Issue::IssueImpl::create()
{
    return std::shared_ptr<Issue> {new Issue {}};
}

void Issue::IssueImpl::setDescription(const std::string &description)
{
    mDescription = description;
}

void Issue::IssueImpl::setLevel(Issue::Level level)
{
    mLevel = level;
}

void Issue::IssueImpl::setReferenceRule(Issue::ReferenceRule referenceRule)
{
    mReferenceRule = referenceRule;
}

Issue::Issue()
    : mPimpl(new IssueImpl())
{
}

Issue::~Issue()
{
    delete mPimpl;
}

std::string Issue::description() const
{
    return mPimpl->mDescription;
}

Issue::Level Issue::level() const
{
    return mPimpl->mLevel;
}

Issue::ReferenceRule Issue::referenceRule() const
{
    return mPimpl->mReferenceRule;
}

AnyCellmlElementPtr Issue::item() const
{
    return mPimpl->mItem;
}

/**
 * @brief Map ReferenceRules to their section titles.
 *
 * An internal map used to convert a Issue::ReferenceRule into its heading string.
 */
static const std::map<Issue::ReferenceRule, std::vector<std::string>> ruleToInformation = {
    {Issue::ReferenceRule::UNDEFINED, {"", "", "", ""}},

    // Validation errors related to the CellML Specification:
    {Issue::ReferenceRule::XML, {"XML", "1.2.1.1", baseSpecificationUrl, "specA02"}},
    {Issue::ReferenceRule::XML_UNEXPECTED_ELEMENT, {"XML_UNEXPECTED_ELEMENT", "1.2.2.2", baseSpecificationUrl, "specA02"}},
    {Issue::ReferenceRule::XML_UNEXPECTED_CHARACTER, {"XML_UNEXPECTED_CHARACTER", "1.2.3.2", baseSpecificationUrl, "specA02"}},
    {Issue::ReferenceRule::XML_UNEXPECTED_NAMESPACE, {"XML_UNEXPECTED_NAMESPACE", "1.2.4.1", baseSpecificationUrl, "specA02"}},
    {Issue::ReferenceRule::XML_ATTRIBUTE_HAS_NAMESPACE, {"XML_ATTRIBUTE_HAS_NAMESPACE", "1.2.4.2", baseSpecificationUrl, "specA02"}},
    {Issue::ReferenceRule::XML_ID_ATTRIBUTE, {"XML_ID_ATTRIBUTE", "1.2.5.1.1", baseSpecificationUrl, "specA02"}},
    {Issue::ReferenceRule::MODEL_ELEMENT, {"MODEL_ELEMENT", "2.1", baseSpecificationUrl, "specB01"}},
    {Issue::ReferenceRule::MODEL_NAME, {"MODEL_NAME", "2.1.1", baseSpecificationUrl, "specB01"}},
    {Issue::ReferenceRule::MODEL_NAME_VALUE, {"MODEL_NAME_VALUE", "2.1.1.1", baseSpecificationUrl, "specB01"}},
    {Issue::ReferenceRule::MODEL_CHILD, {"MODEL_CHILD", "2.1.2", baseSpecificationUrl, "specB01"}},
    {Issue::ReferenceRule::MODEL_MORE_THAN_ONE_ENCAPSULATION, {"MODEL_MORE_THAN_ONE_ENCAPSULATION", "2.1.3", baseSpecificationUrl, "specB01"}},
    {Issue::ReferenceRule::IMPORT_ELEMENT, {"IMPORT_ELEMENT", "2.2", baseSpecificationUrl, "specB02"}},
    {Issue::ReferenceRule::IMPORT_HREF, {"IMPORT_HREF", "2.2.1", baseSpecificationUrl, "specB02"}},
    {Issue::ReferenceRule::IMPORT_HREF_LOCATOR, {"IMPORT_HREF_LOCATOR", "2.2.1.1", baseSpecificationUrl, "specB02"}},
    {Issue::ReferenceRule::IMPORT_CHILD, {"IMPORT_CHILD", "2.2.2", baseSpecificationUrl, "specB02"}},
    {Issue::ReferenceRule::IMPORT_EQUIVALENT_INFOSET, {"IMPORT_EQUIVALENT_INFOSET", "2.2.3", baseSpecificationUrl, "specB02"}},
    {Issue::ReferenceRule::IMPORT_UNITS_ELEMENT, {"IMPORT_UNITS_ELEMENT", "2.3", baseSpecificationUrl, "specB03"}},
    {Issue::ReferenceRule::IMPORT_UNITS_NAME, {"IMPORT_UNITS_NAME", "2.3.1", baseSpecificationUrl, "specB03"}},
    {Issue::ReferenceRule::IMPORT_UNITS_NAME_VALUE, {"IMPORT_UNITS_NAME_VALUE", "2.3.1.1", baseSpecificationUrl, "specB03"}},
    {Issue::ReferenceRule::IMPORT_UNITS_NAME_UNIQUE, {"IMPORT_UNITS_NAME_UNIQUE", "2.3.1.2", baseSpecificationUrl, "specB03"}},
    {Issue::ReferenceRule::IMPORT_UNITS_UNITS_REFERENCE, {"IMPORT_UNITS_UNITS_REFERENCE", "2.3.2", baseSpecificationUrl, "specB03"}},
    {Issue::ReferenceRule::IMPORT_UNITS_UNITS_REFERENCE_VALUE, {"IMPORT_UNITS_UNITS_REFERENCE_VALUE", "2.3.2.1", baseSpecificationUrl, "specB03"}},
    {Issue::ReferenceRule::IMPORT_UNITS_UNITS_REFERENCE_VALUE_TARGET, {"IMPORT_UNITS_UNITS_REFERENCE_VALUE_TARGET", "2.3.2.2", baseSpecificationUrl, "specB03"}},
    {Issue::ReferenceRule::IMPORT_COMPONENT_ELEMENT, {"IMPORT_COMPONENT_ELEMENT", "2.4", baseSpecificationUrl, "specB04"}},
    {Issue::ReferenceRule::IMPORT_COMPONENT_NAME, {"IMPORT_COMPONENT_NAME", "2.4.1", baseSpecificationUrl, "specB04"}},
    {Issue::ReferenceRule::IMPORT_COMPONENT_NAME_VALUE, {"IMPORT_COMPONENT_NAME_VALUE", "2.4.1.1", baseSpecificationUrl, "specB04"}},
    {Issue::ReferenceRule::IMPORT_COMPONENT_NAME_UNIQUE, {"IMPORT_COMPONENT_NAME_UNIQUE", "2.4.1.2", baseSpecificationUrl, "specB04"}},
    {Issue::ReferenceRule::IMPORT_COMPONENT_COMPONENT_REFERENCE, {"IMPORT_COMPONENT_COMPONENT_REFERENCE", "2.4.2", baseSpecificationUrl, "specB04"}},
    {Issue::ReferenceRule::IMPORT_COMPONENT_COMPONENT_REFERENCE_VALUE, {"IMPORT_COMPONENT_COMPONENT_REFERENCE_VALUE", "2.4.2.1", baseSpecificationUrl, "specB04"}},
    {Issue::ReferenceRule::IMPORT_COMPONENT_COMPONENT_REFERENCE_TARGET, {"IMPORT_COMPONENT_COMPONENT_REFERENCE_TARGET", "2.4.2.2", baseSpecificationUrl, "specB04"}},
    {Issue::ReferenceRule::UNITS_ELEMENT, {"UNITS_ELEMENT", "2.5", baseSpecificationUrl, "specB05"}},
    {Issue::ReferenceRule::UNITS_NAME, {"UNITS_NAME", "2.5.1", baseSpecificationUrl, "specB05"}},
    {Issue::ReferenceRule::UNITS_NAME_VALUE, {"UNITS_NAME_VALUE", "2.5.1.1", baseSpecificationUrl, "specB05"}},
    {Issue::ReferenceRule::UNITS_NAME_UNIQUE, {"UNITS_NAME_UNIQUE", "2.5.1.2", baseSpecificationUrl, "specB05"}},
    {Issue::ReferenceRule::UNITS_STANDARD, {"UNITS_STANDARD", "2.5.2", baseSpecificationUrl, "specB05"}},
    {Issue::ReferenceRule::UNITS_CHILD, {"UNITS_CHILD", "2.5.3", baseSpecificationUrl, "specB05"}},
    {Issue::ReferenceRule::UNIT_ELEMENT, {"UNIT_ELEMENT", "2.6", baseSpecificationUrl, "specB06"}},
    {Issue::ReferenceRule::UNIT_UNITS, {"UNIT_UNITS", "2.6.1", baseSpecificationUrl, "specB06"}},
    {Issue::ReferenceRule::UNIT_UNITS_REFERENCE, {"UNIT_UNITS_REFERENCE", "2.6.1.1", baseSpecificationUrl, "specB06"}},
    {Issue::ReferenceRule::UNIT_UNITS_CIRCULAR_REFERENCE, {"UNIT_UNITS_CIRCULAR_REFERENCE", "2.6.1.3", baseSpecificationUrl, "specB06"}},
    {Issue::ReferenceRule::UNIT_ATTRIBUTE_OPTIONAL, {"UNIT_ATTRIBUTE_OPTIONAL", "2.6.2", baseSpecificationUrl, "specB06"}},
    {Issue::ReferenceRule::UNIT_ATTRIBUTE_PREFIX_VALUE, {"UNIT_ATTRIBUTE_PREFIX_VALUE", "2.6.2.1.1", baseSpecificationUrl, "specB06"}},
    {Issue::ReferenceRule::UNIT_ATTRIBUTE_MULTIPLIER_VALUE, {"UNIT_ATTRIBUTE_MULTIPLIER_VALUE", "2.6.2.2.1", baseSpecificationUrl, "specB06"}},
    {Issue::ReferenceRule::UNIT_ATTRIBUTE_EXPONENT_VALUE, {"UNIT_ATTRIBUTE_EXPONENT_VALUE", "2.6.2.3.1", baseSpecificationUrl, "specB06"}},
    {Issue::ReferenceRule::COMPONENT_ELEMENT, {"COMPONENT_ELEMENT", "2.7", baseSpecificationUrl, "specB07"}},
    {Issue::ReferenceRule::COMPONENT_NAME, {"COMPONENT_NAME", "2.7.1", baseSpecificationUrl, "specB07"}},
    {Issue::ReferenceRule::COMPONENT_NAME_VALUE, {"COMPONENT_NAME_VALUE", "2.7.1.1", baseSpecificationUrl, "specB07"}},
    {Issue::ReferenceRule::COMPONENT_NAME_UNIQUE, {"COMPONENT_NAME_UNIQUE", "2.7.1.2", baseSpecificationUrl, "specB07"}},
    {Issue::ReferenceRule::COMPONENT_CHILD, {"COMPONENT_CHILD", "2.7.2", baseSpecificationUrl, "specB07"}},
    {Issue::ReferenceRule::VARIABLE_ELEMENT, {"VARIABLE_ELEMENT", "2.8", baseSpecificationUrl, "specB08"}},
    {Issue::ReferenceRule::VARIABLE_ATTRIBUTE_REQUIRED, {"VARIABLE_ATTRIBUTE_REQUIRED", "2.8.1", baseSpecificationUrl, "specB08"}},
    {Issue::ReferenceRule::VARIABLE_NAME_VALUE, {"VARIABLE_NAME_VALUE", "2.8.1.1.1", baseSpecificationUrl, "specB08"}},
    {Issue::ReferenceRule::VARIABLE_NAME_UNIQUE, {"VARIABLE_NAME_UNIQUE", "2.8.1.1.2", baseSpecificationUrl, "specB08"}},
    {Issue::ReferenceRule::VARIABLE_UNITS_VALUE, {"VARIABLE_UNITS_VALUE", "2.8.1.2.1", baseSpecificationUrl, "specB08"}},
    {Issue::ReferenceRule::VARIABLE_ATTRIBUTE_OPTIONAL, {"VARIABLE_ATTRIBUTE_OPTIONAL", "2.8.2", baseSpecificationUrl, "specB08"}},
    {Issue::ReferenceRule::VARIABLE_INTERFACE_VALUE, {"VARIABLE_INTERFACE_VALUE", "2.8.2.1.1", baseSpecificationUrl, "specB08"}},
    {Issue::ReferenceRule::VARIABLE_INITIAL_VALUE_VALUE, {"VARIABLE_INITIAL_VALUE_VALUE", "2.8.2.2.1", baseSpecificationUrl, "specB08"}},
    {Issue::ReferenceRule::RESET_ELEMENT, {"RESET_ELEMENT", "2.9", baseSpecificationUrl, "specB09"}},
    {Issue::ReferenceRule::RESET_ATTRIBUTE_REQUIRED, {"RESET_ATTRIBUTE_REQUIRED", "2.9.1", baseSpecificationUrl, "specB09"}},
    {Issue::ReferenceRule::RESET_VARIABLE_REFERENCE, {"RESET_VARIABLE_REFERENCE", "2.9.1.1.1", baseSpecificationUrl, "specB09"}},
    {Issue::ReferenceRule::RESET_TEST_VARIABLE_REFERENCE, {"RESET_TEST_VARIABLE_REFERENCE", "2.9.1.2.1", baseSpecificationUrl, "specB09"}},
    {Issue::ReferenceRule::RESET_ORDER_VALUE, {"RESET_ORDER_VALUE", "2.9.1.3.1", baseSpecificationUrl, "specB09"}},
    {Issue::ReferenceRule::RESET_ORDER_UNIQUE, {"RESET_ORDER_UNIQUE", "2.9.1.3.2", baseSpecificationUrl, "specB09"}},
    {Issue::ReferenceRule::RESET_CHILD, {"RESET_CHILD", "2.9.2", baseSpecificationUrl, "specB09"}},
    {Issue::ReferenceRule::RESET_RESET_VALUE_CHILD, {"RESET_RESET_VALUE_CHILD", "2.9.2.1", baseSpecificationUrl, "specB09"}},
    {Issue::ReferenceRule::RESET_TEST_VALUE_CHILD, {"RESET_TEST_VALUE_CHILD", "2.9.2.2", baseSpecificationUrl, "specB09"}},
    {Issue::ReferenceRule::TEST_VALUE_ELEMENT, {"TEST_VALUE_ELEMENT", "2.10", baseSpecificationUrl, "specB10"}},
    {Issue::ReferenceRule::TEST_VALUE_CHILD, {"TEST_VALUE_CHILD", "2.10.1", baseSpecificationUrl, "specB10"}},
    {Issue::ReferenceRule::RESET_VALUE_ELEMENT, {"RESET_VALUE_ELEMENT", "2.11", baseSpecificationUrl, "specB11"}},
    {Issue::ReferenceRule::RESET_VALUE_CHILD, {"RESET_VALUE_CHILD", "2.11.1", baseSpecificationUrl, "specB11"}},
    {Issue::ReferenceRule::MATH_ELEMENT, {"MATH_ELEMENT", "2.12", baseSpecificationUrl, "specB12"}},
    {Issue::ReferenceRule::MATH_MATHML, {"MATH_MATHML", "2.12.1", baseSpecificationUrl, "specB12"}},
    {Issue::ReferenceRule::MATH_CHILD, {"MATH_CHILD", "2.12.2", baseSpecificationUrl, "specB12"}},
    {Issue::ReferenceRule::MATH_CI_VARIABLE_REFERENCE, {"MATH_CI_VARIABLE_REFERENCE", "2.12.3", baseSpecificationUrl, "specB12"}},
    {Issue::ReferenceRule::MATH_CN_UNITS_ATTRIBUTE, {"MATH_CN_UNITS_ATTRIBUTE", "2.12.4", baseSpecificationUrl, "specB12"}},
    {Issue::ReferenceRule::MATH_CN_UNITS_ATTRIBUTE_REFERENCE, {"MATH_CN_UNITS_ATTRIBUTE_REFERENCE", "2.12.4.1", baseSpecificationUrl, "specB12"}},
    {Issue::ReferenceRule::MATH_CN_BASE10, {"MATH_CN_BASE10", "2.12.5", baseSpecificationUrl, "specB12"}},
    {Issue::ReferenceRule::MATH_CN_FORMAT, {"MATH_CN_FORMAT", "2.12.5.1", baseSpecificationUrl, "specB12"}},
    {Issue::ReferenceRule::ENCAPSULATION_ELEMENT, {"ENCAPSULATION_ELEMENT", "2.13", baseSpecificationUrl, "specB13"}},
    {Issue::ReferenceRule::ENCAPSULATION_CHILD, {"ENCAPSULATION_CHILD", "2.13.1", baseSpecificationUrl, "specB13"}},
    {Issue::ReferenceRule::COMPONENT_REF_ELEMENT, {"COMPONENT_REF_ELEMENT", "2.14", baseSpecificationUrl, "specB14"}},
    {Issue::ReferenceRule::COMPONENT_REF_COMPONENT_ATTRIBUTE, {"COMPONENT_REF_COMPONENT_ATTRIBUTE", "2.14.1", baseSpecificationUrl, "specB14"}},
    {Issue::ReferenceRule::COMPONENT_REF_COMPONENT_ATTRIBUTE_REFERENCE, {"COMPONENT_REF_COMPONENT_ATTRIBUTE_REFERENCE", "2.14.1.1", baseSpecificationUrl, "specB14"}},
    {Issue::ReferenceRule::COMPONENT_REF_COMPONENT_ATTRIBUTE_UNIQUE, {"COMPONENT_REF_COMPONENT_ATTRIBUTE_UNIQUE", "2.14.1.2", baseSpecificationUrl, "specB14"}},
    {Issue::ReferenceRule::COMPONENT_REF_CHILD, {"COMPONENT_REF_CHILD", "2.14.2", baseSpecificationUrl, "specB14"}},
    {Issue::ReferenceRule::CONNECTION_ELEMENT, {"CONNECTION_ELEMENT", "2.15", baseSpecificationUrl, "specB14"}},
    {Issue::ReferenceRule::CONNECTION_COMPONENT1_ATTRIBUTE, {"CONNECTION_COMPONENT1_ATTRIBUTE", "2.15.1", baseSpecificationUrl, "specB15"}},
    {Issue::ReferenceRule::CONNECTION_COMPONENT1_ATTRIBUTE_REFERENCE, {"CONNECTION_COMPONENT1_ATTRIBUTE_REFERENCE", "2.15.1.1", baseSpecificationUrl, "specB15"}},
    {Issue::ReferenceRule::CONNECTION_COMPONENT2_ATTRIBUTE, {"CONNECTION_COMPONENT2_ATTRIBUTE", "2.15.2", baseSpecificationUrl, "specB15"}},
    {Issue::ReferenceRule::CONNECTION_COMPONENT2_ATTRIBUTE_REFERENCE, {"CONNECTION_COMPONENT2_ATTRIBUTE_REFERENCE", "2.15.2.1", baseSpecificationUrl, "specB15"}},
    {Issue::ReferenceRule::CONNECTION_EXCLUDE_SELF, {"CONNECTION_EXCLUDE_SELF", "2.15.3", baseSpecificationUrl, "specB15"}},
    {Issue::ReferenceRule::CONNECTION_UNIQUE, {"CONNECTION_UNIQUE", "2.15.4", baseSpecificationUrl, "specB15"}},
    {Issue::ReferenceRule::CONNECTION_CHILD, {"CONNECTION_CHILD", "2.15.5", baseSpecificationUrl, "specB15"}},
    {Issue::ReferenceRule::MAP_VARIABLES_ELEMENT, {"MAP_VARIABLES_ELEMENT", "2.16", baseSpecificationUrl, "specB16"}},
    {Issue::ReferenceRule::MAP_VARIABLES_VARIABLE1_ATTRIBUTE, {"MAP_VARIABLES_VARIABLE1_ATTRIBUTE", "2.16.1", baseSpecificationUrl, "specB16"}},
    {Issue::ReferenceRule::MAP_VARIABLES_VARIABLE1_ATTRIBUTE_REFERENCE, {"MAP_VARIABLES_VARIABLE1_ATTRIBUTE_REFERENCE", "2.16.1.1", baseSpecificationUrl, "specB16"}},
    {Issue::ReferenceRule::MAP_VARIABLES_VARIABLE2_ATTRIBUTE, {"MAP_VARIABLES_VARIABLE2", "2.16.2", baseSpecificationUrl, "specB16"}},
    {Issue::ReferenceRule::MAP_VARIABLES_VARIABLE2_ATTRIBUTE_REFERENCE, {"MAP_VARIABLES_VARIABLE2_ATTRIBUTE_REFERENCE", "2.16.2.1", baseSpecificationUrl, "specB16"}},
    {Issue::ReferenceRule::MAP_VARIABLES_UNIQUE, {"MAP_VARIABLES_UNIQUE", "2.16.3", baseSpecificationUrl, "specB16"}},

    // Secondary specification rules:
    {Issue::ReferenceRule::DATA_REPR_IDENTIFIER_AT_LEAST_ONE_ALPHANUM, {"DATA_REPR_IDENTIFIER_AT_LEAST_ONE_ALPHANUM", "1.3.1.1", baseSpecificationUrl, "specA03"}},
    {Issue::ReferenceRule::DATA_REPR_IDENTIFIER_BEGIN_EURO_NUM, {"DATA_REPR_IDENTIFIER_BEGIN_EURO_NUM", "1.3.1.1", baseSpecificationUrl, "specA03"}},
    {Issue::ReferenceRule::DATA_REPR_IDENTIFIER_LATIN_ALPHANUM, {"DATA_REPR_IDENTIFIER_LATIN_ALPHANUM", "1.3.1.1", baseSpecificationUrl, "specA03"}},

    // Not in normative specification:
    {Issue::ReferenceRule::INVALID_ARGUMENT, {"INVALID_ARGUMENT", "", docsUrl, ""}},

    // Importer class issues:
    {Issue::ReferenceRule::IMPORTER_ERROR_IMPORTING_UNITS, {"IMPORTER_ERROR_IMPORTING_UNITS", "", docsUrl, ""}},
    {Issue::ReferenceRule::IMPORTER_MISSING_FILE, {"IMPORTER_MISSING_FILE", "", docsUrl, ""}},
    {Issue::ReferenceRule::IMPORTER_MISSING_COMPONENT, {"IMPORTER_MISSING_COMPONENT", "", docsUrl, ""}},
    {Issue::ReferenceRule::IMPORTER_MISSING_UNITS, {"IMPORTER_MISSING_UNITS", "", docsUrl, ""}},
    {Issue::ReferenceRule::IMPORTER_NULL_MODEL, {"IMPORTER_NULL_MODEL", "", docsUrl, ""}},
    {Issue::ReferenceRule::IMPORTER_UNDEFINED_MODEL, {"IMPORTER_UNDEFINED_MODEL", "", docsUrl, ""}},
    {Issue::ReferenceRule::IMPORTER_UNRESOLVED_IMPORTS, {"IMPORTER_UNRESOLVED_IMPORTS", "", docsUrl, ""}},

    // Analyser issues:
    {Issue::ReferenceRule::ANALYSER_EQUATION_NOT_EQUALITY_STATEMENT, {"ANALYSER_EQUATION_NOT_EQUALITY_STATEMENT", "", docsUrl, ""}},
    {Issue::ReferenceRule::ANALYSER_UNITS, {"ANALYSER_UNITS", "", docsUrl, ""}},
    {Issue::ReferenceRule::ANALYSER_UNLINKED_UNITS, {"ANALYSER_UNLINKED_UNITS", "", docsUrl, ""}},
    {Issue::ReferenceRule::ANALYSER_VARIABLE_INITIALISED_MORE_THAN_ONCE, {"ANALYSER_VARIABLE_INITIALISED_MORE_THAN_ONCE", "", docsUrl, ""}},
    {Issue::ReferenceRule::ANALYSER_VARIABLE_INITIALISED_USING_ALGEBRAIC_VARIABLE, {"ANALYSER_VARIABLE_INITIALISED_USING_ALGEBRAIC_VARIABLE", "", docsUrl, ""}},
    {Issue::ReferenceRule::ANALYSER_VARIABLE_INITIALISED_USING_VARIABLE_WITH_DIFFERENT_UNITS, {"ANALYSER_VARIABLE_INITIALISED_USING_VARIABLE_WITH_DIFFERENT_UNITS", "", docsUrl, ""}},
    {Issue::ReferenceRule::ANALYSER_VOI_INITIALISED, {"ANALYSER_VOI_INITIALISED", "", docsUrl, ""}},
    {Issue::ReferenceRule::ANALYSER_VOI_SEVERAL, {"ANALYSER_VOI_SEVERAL", "", docsUrl, ""}},
    {Issue::ReferenceRule::ANALYSER_ODE_NOT_FIRST_ORDER, {"ANALYSER_ODE_NOT_FIRST_ORDER", "", docsUrl, ""}},
    {Issue::ReferenceRule::ANALYSER_VARIABLE_UNUSED, {"ANALYSER_VARIABLE_UNUSED", "", docsUrl, ""}},
    {Issue::ReferenceRule::ANALYSER_STATE_NOT_INITIALISED, {"ANALYSER_STATE_NOT_INITIALISED", "", docsUrl, ""}},
    {Issue::ReferenceRule::ANALYSER_STATE_RATE_AS_ALGEBRAIC_VARIABLE, {"ANALYSER_STATE_RATE_AS_ALGEBRAIC_VARIABLE", "", docsUrl, ""}},
    {Issue::ReferenceRule::ANALYSER_VARIABLE_UNDERCONSTRAINED, {"ANALYSER_VARIABLE_UNDERCONSTRAINED", "", docsUrl, ""}},
    {Issue::ReferenceRule::ANALYSER_VARIABLE_OVERCONSTRAINED, {"ANALYSER_VARIABLE_OVERCONSTRAINED", "", docsUrl, ""}},
    {Issue::ReferenceRule::ANALYSER_EXTERNAL_VARIABLE_DIFFERENT_MODEL, {"ANALYSER_EXTERNAL_VARIABLE_DIFFERENT_MODEL", "", docsUrl, ""}},
    {Issue::ReferenceRule::ANALYSER_EXTERNAL_VARIABLE_VOI, {"ANALYSER_EXTERNAL_VARIABLE_VOI", "", docsUrl, ""}},
    {Issue::ReferenceRule::ANALYSER_EXTERNAL_VARIABLE_USE_PRIMARY_VARIABLE, {"ANALYSER_EXTERNAL_VARIABLE_USE_PRIMARY_VARIABLE", "", docsUrl, ""}},

    // Annotator issues:
    {Issue::ReferenceRule::ANNOTATOR_ID_NOT_FOUND, {"ANNOTATOR_ID_NOT_FOUND", "", docsUrl, ""}},
    {Issue::ReferenceRule::ANNOTATOR_ID_NOT_UNIQUE, {"ANNOTATOR_ID_NOT_UNIQUE", "", docsUrl, ""}},
    {Issue::ReferenceRule::ANNOTATOR_NO_MODEL, {"ANNOTATOR_NO_MODEL", "", docsUrl, ""}},
    {Issue::ReferenceRule::ANNOTATOR_INCONSISTENT_TYPE, {"ANNOTATOR_INCONSISTENT_TYPE", "", docsUrl, ""}},
    {Issue::ReferenceRule::ANNOTATOR_NULL_MODEL, {"ANNOTATOR_NULL_MODEL", "", docsUrl, ""}},

};

std::string Issue::referenceHeading() const
{
    return ruleToInformation.at(referenceRule())[1];
}

std::string Issue::url() const
{
    auto search = ruleToInformation.at(referenceRule());

    if (search[1].empty()) {
        return search[2] + "?issue=" + search[0];
    }

    return search[2] + search[3] + ".html?issue=" + search[0];
}

} // namespace libcellml
