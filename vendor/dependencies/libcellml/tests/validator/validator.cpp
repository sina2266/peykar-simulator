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

#include "gtest/gtest.h"

#include <libcellml>

#include "test_utils.h"

/*
 * The tests in this file are here to catch any branches of code that
 * are not picked up by the main tests testing the API of the library
 */

TEST(Validator, invalidInput)
{
    libcellml::ValidatorPtr validator = libcellml::Validator::create();

    validator->validateModel(nullptr);
    EXPECT_EQ(size_t(1), validator->issueCount());
    EXPECT_EQ("The model is null.", validator->issue(0)->description());
}

TEST(Validator, namedModel)
{
    libcellml::ValidatorPtr validator = libcellml::Validator::create();
    libcellml::ModelPtr model = libcellml::Model::create();
    model->setName("awesomeName");
    validator->validateModel(model);
    EXPECT_EQ(size_t(0), validator->issueCount());
}

TEST(Validator, unnamedModel)
{
    const std::vector<std::string> expectedIssues = {
        "Model '' does not have a valid name attribute. CellML identifiers must contain one or more basic Latin alphabetic characters.",
    };
    const std::vector<std::string> expectedSpecificationHeadings = {
        "2.1.1.1",
    };
    const std::vector<std::string> expectedUrls = {
        "https://cellml-specification.readthedocs.io/en/latest/reference/formal_and_informative/specB01.html?issue=MODEL_NAME_VALUE",
    };

    libcellml::ValidatorPtr validator = libcellml::Validator::create();
    libcellml::ModelPtr model = libcellml::Model::create();
    validator->validateModel(model);
    EXPECT_EQ_ISSUES_SPECIFICATION_HEADINGS_URLS(expectedIssues, expectedSpecificationHeadings, expectedUrls, validator);
}

TEST(Validator, invalidCellMLIdentifiersWithSpecificationHeading)
{
    const std::vector<std::string> expectedIssues = {
        "Model '9numbernine' does not have a valid name attribute. CellML identifiers must not begin with a European numeric character [0-9].",
        "Component 'try.this' does not have a valid name attribute. CellML identifiers must not contain any characters other than [a-zA-Z0-9_].",
        "Component '' does not have a valid name attribute. CellML identifiers must contain one or more basic Latin alphabetic characters.",
        "Component 'or this' does not have a valid name attribute. CellML identifiers must not contain any characters other than [a-zA-Z0-9_].",
        "Component '' does not have a valid name attribute. CellML identifiers must contain one or more basic Latin alphabetic characters.",
    };
    const std::vector<std::string> expectedSpecificationHeadings = {
        "2.1.1.1",
        "2.7.1.1",
        "2.7.1.1",
        "2.7.1.1",
        "2.7.1.1",
    };
    const std::vector<std::string> expectedUrls = {
        "https://cellml-specification.readthedocs.io/en/latest/reference/formal_and_informative/specB01.html?issue=MODEL_NAME_VALUE",
        "https://cellml-specification.readthedocs.io/en/latest/reference/formal_and_informative/specB07.html?issue=COMPONENT_NAME_VALUE",
        "https://cellml-specification.readthedocs.io/en/latest/reference/formal_and_informative/specB07.html?issue=COMPONENT_NAME_VALUE",
        "https://cellml-specification.readthedocs.io/en/latest/reference/formal_and_informative/specB07.html?issue=COMPONENT_NAME_VALUE",
        "https://cellml-specification.readthedocs.io/en/latest/reference/formal_and_informative/specB07.html?issue=COMPONENT_NAME_VALUE",
    };

    libcellml::ValidatorPtr v = libcellml::Validator::create();
    libcellml::ModelPtr model = libcellml::Model::create();
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    libcellml::ComponentPtr c2 = libcellml::Component::create();
    libcellml::ComponentPtr c3 = libcellml::Component::create();
    libcellml::ComponentPtr c4 = libcellml::Component::create();
    libcellml::ComponentPtr c5 = libcellml::Component::create();

    model->setName("9numbernine");
    c1->setName("try.this");
    c2->setName("");
    c3->setName("or this");
    c4->setName("nice_name");

    model->addComponent(c1);
    model->addComponent(c2);
    model->addComponent(c3);
    model->addComponent(c4);
    model->addComponent(c5);

    v->validateModel(model);

    EXPECT_EQ_ISSUES_SPECIFICATION_HEADINGS_URLS(expectedIssues, expectedSpecificationHeadings, expectedUrls, v);
}

TEST(Validator, invalidElementIdAttribute)
{
    const std::vector<std::string> expectedIssues = {
        "Model 'valid_name' does not have a valid 'id' attribute, '993-++$@'.",
    };
    const std::vector<std::string> expectedSpecificationHeadings = {
        "1.2.5.1.1",
    };
    const std::vector<std::string> expectedUrls = {
        "https://cellml-specification.readthedocs.io/en/latest/reference/formal_and_informative/specA02.html?issue=XML_ID_ATTRIBUTE",
    };

    libcellml::ModelPtr model = libcellml::Model::create("valid_name");
    model->setId("993-++$@");

    libcellml::ValidatorPtr v = libcellml::Validator::create();

    v->validateModel(model);

    EXPECT_EQ_ISSUES_SPECIFICATION_HEADINGS_URLS(expectedIssues, expectedSpecificationHeadings, expectedUrls, v);
}

TEST(Validator, namedModelWithUnnamedComponent)
{
    const std::vector<std::string> expectedIssues = {
        "Component '' does not have a valid name attribute. CellML identifiers must contain one or more basic Latin alphabetic characters.",
    };
    libcellml::ValidatorPtr validator = libcellml::Validator::create();
    libcellml::ModelPtr model = libcellml::Model::create();
    libcellml::ComponentPtr component = libcellml::Component::create();
    model->setName("awesomeName");
    model->addComponent(component);
    validator->validateModel(model);
    EXPECT_EQ_ISSUES(expectedIssues, validator);
}

TEST(Validator, unnamedModelWithUnnamedComponentWithUnnamedUnits)
{
    const std::vector<std::string> expectedIssues = {
        "Model '' does not have a valid name attribute. CellML identifiers must contain one or more basic Latin alphabetic characters.",
        "Component '' does not have a valid name attribute. CellML identifiers must contain one or more basic Latin alphabetic characters.",
        "Units '' does not have a valid name attribute. CellML identifiers must contain one or more basic Latin alphabetic characters.",
    };

    libcellml::ValidatorPtr validator = libcellml::Validator::create();
    libcellml::ModelPtr model = libcellml::Model::create();
    libcellml::ComponentPtr component = libcellml::Component::create();
    libcellml::UnitsPtr units = libcellml::Units::create();
    model->addComponent(component);
    model->addUnits(units);
    validator->validateModel(model);

    EXPECT_EQ_ISSUES(expectedIssues, validator);
}

TEST(Validator, modelWithDuplicateComponentsAndUnits)
{
    const std::vector<std::string> expectedIssues = {
        "Model 'multiplicity' contains multiple components with the name 'michael'. Valid component names must be unique to their model.",
        "Model 'multiplicity' contains multiple units with the name 'keaton'. Valid units names must be unique to their model.",
    };

    libcellml::ValidatorPtr validator = libcellml::Validator::create();
    libcellml::ModelPtr model = libcellml::Model::create();
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    libcellml::ComponentPtr c2 = libcellml::Component::create();
    libcellml::UnitsPtr u1 = libcellml::Units::create();
    libcellml::UnitsPtr u2 = libcellml::Units::create();
    model->addComponent(c1);
    model->addComponent(c2);
    model->addUnits(u1);
    model->addUnits(u2);

    model->setName("multiplicity");
    c1->setName("michael");
    c2->setName("michael");
    u1->setName("keaton");
    u2->setName("keaton");
    validator->validateModel(model);

    EXPECT_EQ_ISSUES(expectedIssues, validator);
}

TEST(Validator, unitsHasReferenceToStandardUnitsDefinedInModel)
{
    const std::string e = "Units is named 'second' which is a protected standard unit name.";

    libcellml::ValidatorPtr validator = libcellml::Validator::create();
    libcellml::ModelPtr model = libcellml::Model::create();
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    libcellml::UnitsPtr u1 = libcellml::Units::create();
    libcellml::UnitsPtr u2 = libcellml::Units::create();

    model->addComponent(c1);
    model->addUnits(u1);
    model->addUnits(u2);

    model->setName("model");
    c1->setName("michael");
    u1->setName("keaton");
    u2->setName("second");

    u1->addUnit("second");

    validator->validateModel(model);

    EXPECT_EQ(size_t(1), validator->errorCount());
    EXPECT_EQ(e, validator->error(0)->description());
}

TEST(Validator, unnamedAndDuplicateNamedVariablesWithAndWithoutValidUnits)
{
    const std::vector<std::string> expectedIssues = {
        "Variable '2cold' in component 'fargo' does not have a valid name attribute. CellML identifiers must not begin with a European numeric character [0-9].",
        "Component 'fargo' contains multiple variables with the name 'margie'. Valid variable names must be unique to their component.",
        "Variable 'margie' in component 'fargo' does not have any units specified.",
        "Variable 'ransom' in component 'fargo' has a units reference 'dollars' which is neither standard nor defined in the parent model.",
        "Variable 'mullah' in component 'fargo' does not have a valid units attribute. The attribute given is '$$'. CellML identifiers must not contain any characters other than [a-zA-Z0-9_].",
        "Variable '' in component 'fargo' does not have a valid name attribute. CellML identifiers must contain one or more basic Latin alphabetic characters.",
    };

    libcellml::ValidatorPtr validator = libcellml::Validator::create();
    libcellml::ModelPtr model = libcellml::Model::create();
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    libcellml::VariablePtr v3 = libcellml::Variable::create();
    libcellml::VariablePtr v4 = libcellml::Variable::create();
    libcellml::VariablePtr v5 = libcellml::Variable::create();
    libcellml::VariablePtr v6 = libcellml::Variable::create();

    model->addComponent(c1);
    c1->addVariable(v1);
    c1->addVariable(v2);
    c1->addVariable(v3);
    c1->addVariable(v4);
    c1->addVariable(v5);
    c1->addVariable(v6);

    model->setName("minnesota");
    c1->setName("fargo");
    v1->setName("2cold");
    v1->setUnits("ampere");
    v2->setName("margie");
    v2->setUnits("ampere");
    v3->setName("margie");
    v4->setName("ransom");
    v4->setUnits("dollars");
    v5->setName("mullah");
    v5->setUnits("$$");
    v6->setName("");
    v6->setUnits("second");

    validator->validateModel(model);

    EXPECT_EQ_ISSUES(expectedIssues, validator);
}

TEST(Validator, invalidVariableInitialValuesAndInterfaces)
{
    const std::vector<std::string> expectedIssues = {
        "Variable 'candidate' in component 'republican' has an invalid interface attribute value 'orange'.",
        "Variable 'candidate' in component 'republican' has an invalid initial value 'trump'. Initial values must be a real number string or a variable reference.",
    };

    libcellml::ValidatorPtr validator = libcellml::Validator::create();
    libcellml::ModelPtr model = libcellml::Model::create();
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    model->addComponent(c1);
    c1->addVariable(v1);

    model->setName("election");
    c1->setName("republican");
    v1->setName("candidate");
    v1->setUnits("ampere");
    v1->setInterfaceType("orange");
    v1->setInitialValue("trump");

    validator->validateModel(model);

    EXPECT_EQ_ISSUES(expectedIssues, validator);
}

libcellml::ModelPtr createImportUnitsModel()
{
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ImportSourcePtr imp = libcellml::ImportSource::create();
    libcellml::UnitsPtr importedUnits = libcellml::Units::create();

    m->setName("model_name");
    imp->setUrl("some-other-model.xml");
    importedUnits->setName("valid_imported_units_in_this_model");
    importedUnits->setSourceUnits(imp, "units_in_that_model");
    m->addUnits(importedUnits);

    return m;
}

TEST(Validator, importUnitsValid)
{
    libcellml::ValidatorPtr v = libcellml::Validator::create();
    libcellml::ModelPtr m = createImportUnitsModel();

    v->validateModel(m);
    EXPECT_EQ(size_t(0), v->issueCount());
}

TEST(Validator, importUnitsMissingRefs)
{
    const std::vector<std::string> expectedIssues = {
        "Imported units 'invalid_imported_units_in_this_model' does not have a valid units_ref attribute. CellML identifiers must contain one or more basic Latin alphabetic characters.",
        "Import of units 'invalid_imported_units_in_this_model' does not have a valid locator xlink:href attribute.",
    };

    libcellml::ValidatorPtr v = libcellml::Validator::create();
    libcellml::ModelPtr m = createImportUnitsModel();

    libcellml::UnitsPtr units = m->units(0);

    libcellml::ImportSourcePtr imp = libcellml::ImportSource::create();
    units->setName("invalid_imported_units_in_this_model");
    units->setSourceUnits(imp, "");

    v->validateModel(m);

    EXPECT_EQ_ISSUES(expectedIssues, v);
}

TEST(Validator, importUnitsDuplicateRefs)
{
    const std::vector<std::string> expectedIssues = {
        "Model 'model_name' contains multiple imported units from 'some-other-model.xml' with the same units_ref attribute 'units_in_that_model'.",
    };

    libcellml::ValidatorPtr v = libcellml::Validator::create();
    libcellml::ModelPtr m = createImportUnitsModel();

    libcellml::ImportSourcePtr imp = libcellml::ImportSource::create();
    imp->setUrl("some-other-model.xml");
    libcellml::UnitsPtr units = libcellml::Units::create();
    units->setName("duplicate_imported_units_in_this_model");
    units->setSourceUnits(imp, "units_in_that_model");
    m->addUnits(units);

    v->validateModel(m);
    EXPECT_EQ_ISSUES(expectedIssues, v);
}

TEST(Validator, importUnitsUnnamedUnits)
{
    const std::vector<std::string> expectedIssues = {
        "Imported units '' does not have a valid name attribute. CellML identifiers must contain one or more basic Latin alphabetic characters.",
    };

    libcellml::ValidatorPtr v = libcellml::Validator::create();
    libcellml::ModelPtr m = createImportUnitsModel();

    libcellml::UnitsPtr u = m->units(0);
    u->setName("");

    v->validateModel(m);

    EXPECT_EQ_ISSUES(expectedIssues, v);
}

TEST(Validator, importUnitsInvalidUrl)
{
    const std::vector<std::string> expectedIssues = {
        "Import of units 'cant_find_me' has an invalid URI in the xlink:href attribute.",
    };

    libcellml::ValidatorPtr v = libcellml::Validator::create();
    libcellml::ModelPtr m = createImportUnitsModel();

    libcellml::UnitsPtr u = m->units(0);
    libcellml::ImportSourcePtr imp = u->importSource();
    imp->setUrl("Look Ma, I've got special characters!");

    u->setName("cant_find_me");
    u->setSourceUnits(imp, "cant_find_me_anyway");

    v->validateModel(m);

    EXPECT_EQ_ISSUES(expectedIssues, v);
}

TEST(Validator, importComponents)
{
    const std::vector<std::string> expectedIssues = {
        "Imported component 'invalid_imported_component_in_this_model' does not have a valid component_ref attribute. CellML identifiers must contain one or more basic Latin alphabetic characters.",
        "Import of component 'invalid_imported_component_in_this_model' does not have a valid locator xlink:href attribute.",
        "Imported component '' does not have a valid name attribute. CellML identifiers must contain one or more basic Latin alphabetic characters.",
        "Import of component 'a_bad_imported_component' has an invalid URI in the xlink:href attribute.",
    };

    libcellml::ValidatorPtr v = libcellml::Validator::create();
    libcellml::ModelPtr m = libcellml::Model::create();
    m->setName("model_name");

    // Valid component import
    libcellml::ImportSourcePtr imp = libcellml::ImportSource::create();
    imp->setUrl("some-other-model.xml");
    libcellml::ComponentPtr importedComponent = libcellml::Component::create();
    importedComponent->setName("valid_imported_component_in_this_model");
    importedComponent->setSourceComponent(imp, "component_in_that_model");
    m->addComponent(importedComponent);
    v->validateModel(m);
    EXPECT_EQ(size_t(0), v->issueCount());

    // Another valid component import
    libcellml::ImportSourcePtr imp2 = libcellml::ImportSource::create();
    imp2->setUrl("yet-another-other-model.xml");
    libcellml::ComponentPtr importedComponent2 = libcellml::Component::create();
    importedComponent2->setName("another_valid_imported_component_in_this_model");
    importedComponent2->setSourceComponent(imp2, "new_shiny_component_ref");
    m->addComponent(importedComponent2);
    v->validateModel(m);
    EXPECT_EQ(size_t(0), v->issueCount());

    // Invalid component import - missing ref to source component
    libcellml::ImportSourcePtr imp3 = libcellml::ImportSource::create();
    libcellml::ComponentPtr importedComponent3 = libcellml::Component::create();
    importedComponent3->setName("invalid_imported_component_in_this_model");
    importedComponent3->setSourceComponent(imp3, "");
    m->addComponent(importedComponent3);
    v->validateModel(m);
    EXPECT_EQ(size_t(2), v->issueCount());

    // Valid component import - two components in the same place is allowed
    libcellml::ImportSourcePtr imp4 = libcellml::ImportSource::create();
    imp4->setUrl("some-other-model.xml");
    libcellml::ComponentPtr importedComponent4 = libcellml::Component::create();
    importedComponent4->setName("duplicate_imported_component_in_this_model");
    importedComponent4->setSourceComponent(imp4, "component_in_that_model");
    m->addComponent(importedComponent4);
    v->validateModel(m);
    EXPECT_EQ(size_t(2), v->issueCount());

    // Invalid - name missing from component
    libcellml::ImportSourcePtr imp5 = libcellml::ImportSource::create();
    imp5->setUrl("some-other-different-model.xml");
    libcellml::ComponentPtr importedComponent5 = libcellml::Component::create();
    importedComponent5->setSourceComponent(imp5, "component_in_that_model");
    m->addComponent(importedComponent5);
    v->validateModel(m);
    EXPECT_EQ(size_t(3), v->issueCount());

    // Valid - two components from the same source is allowed
    libcellml::ImportSourcePtr imp7 = libcellml::ImportSource::create();
    imp7->setUrl("yet-another-other-model.xml");
    libcellml::ComponentPtr importedComponent7 = libcellml::Component::create();
    importedComponent7->setName("another_duplicate_imported_component");
    importedComponent7->setSourceComponent(imp7, "new_shiny_component_ref");
    m->addComponent(importedComponent7);
    v->validateModel(m);
    EXPECT_EQ(size_t(3), v->issueCount());

    // Valid - duplicate component_ref from a different source
    libcellml::ImportSourcePtr imp8 = libcellml::ImportSource::create();
    imp8->setUrl("yet-another-other-model.xml"); // source used before
    libcellml::ComponentPtr importedComponent8 = libcellml::Component::create();
    importedComponent8->setName("a_good_imported_component");
    importedComponent8->setSourceComponent(imp8, "component_in_that_model");
    m->addComponent(importedComponent8);
    v->validateModel(m);
    EXPECT_EQ(size_t(3), v->issueCount());

    // Invalid: component_ref URL is not valid html
    libcellml::ImportSourcePtr imp9 = libcellml::ImportSource::create();
    imp9->setUrl("not @ valid url");
    libcellml::ComponentPtr importedComponent9 = libcellml::Component::create();
    importedComponent9->setName("a_bad_imported_component");
    importedComponent9->setSourceComponent(imp9, "component_in_some_model");
    m->addComponent(importedComponent9);
    v->validateModel(m);

    // Check for expected error messages
    EXPECT_EQ_ISSUES(expectedIssues, v);
}

TEST(Validator, importsDummyVariablesNotCheckedForUnitsInterfaces)
{
    auto validator = libcellml::Validator::create();
    auto model = libcellml::Model::create("model");

    // Create a concrete component and variable.
    // This should be checked for variable validity: units, interface.
    auto component = libcellml::Component::create("component");
    auto variable = libcellml::Variable::create("variable");
    variable->setUnits("dimensionless");
    variable->setInterfaceType("public");
    component->addVariable(variable);
    model->addComponent(component);

    // Create an imported component and dummy variable.
    // This should *not* be checked for variable validity.
    auto importer = libcellml::ImportSource::create();
    importer->setUrl("some-other-model.xml");
    auto dummyComponent = libcellml::Component::create("dummy_component");
    dummyComponent->setSourceComponent(importer, "component_in_that_model");
    auto dummyVariable = libcellml::Variable::create("I_dont_need_units_or_interface");
    dummyComponent->addVariable(dummyVariable);
    model->addComponent(dummyComponent);

    validator->validateModel(model);
    EXPECT_EQ(size_t(0), validator->issueCount());

    libcellml::Variable::addEquivalence(variable, dummyVariable);
    validator->validateModel(model);
    EXPECT_EQ(size_t(0), validator->issueCount());
}

TEST(Validator, validMath)
{
    const std::string math =
        "<math xmlns=\"http://www.w3.org/1998/Math/MathML\">\n"
        "  <apply>\n"
        "    <eq/>\n"
        "    <ci>C</ci>\n"
        "    <apply>\n"
        "      <plus/>\n"
        "      <ci>A</ci>\n"
        "      <ci>B</ci>\n"
        "    </apply>\n"
        "  </apply>\n"
        "</math>\n";

    libcellml::ValidatorPtr v = libcellml::Validator::create();
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c = libcellml::Component::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    libcellml::VariablePtr v3 = libcellml::Variable::create();

    m->setName("modelName");
    c->setName("componentName");
    v1->setName("A");
    v2->setName("B");
    v3->setName("C");
    v1->setInitialValue("1.0");
    v2->setInitialValue("-1.0");
    v1->setUnits("dimensionless");
    v2->setUnits("dimensionless");
    v3->setUnits("dimensionless");

    c->addVariable(v1);
    c->addVariable(v2);
    c->addVariable(v3);
    c->setMath(math);
    m->addComponent(c);

    v->validateModel(m);
    EXPECT_EQ(size_t(0), v->issueCount());
}

TEST(Validator, validMathInMultipleMathMLBlocks)
{
    const std::string math =
        "<math xmlns:cellml=\"http://www.cellml.org/cellml/2.0#\" xmlns=\"http://www.w3.org/1998/Math/MathML\">\n"
        "  <apply>\n"
        "    <eq/>\n"
        "    <ci>A</ci>\n"
        "    <apply>\n"
        "      <cn cellml:units=\"dimensionless\">1</cn>\n"
        "    </apply>\n"
        "  </apply>\n"
        "</math>\n"
        "<math xmlns:cellml=\"http://www.cellml.org/cellml/2.0#\" xmlns=\"http://www.w3.org/1998/Math/MathML\">\n"
        "  <apply>\n"
        "    <eq/>\n"
        "    <ci>B</ci>\n"
        "    <apply>\n"
        "      <cn cellml:units=\"dimensionless\">2</cn>\n"
        "    </apply>\n"
        "  </apply>\n"
        "</math>\n";

    libcellml::ValidatorPtr v = libcellml::Validator::create();
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c = libcellml::Component::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();

    m->setName("modelName");
    c->setName("componentName");
    v1->setName("A");
    v2->setName("B");
    v1->setUnits("dimensionless");
    v2->setUnits("dimensionless");

    c->addVariable(v1);
    c->addVariable(v2);
    c->setMath(math);
    m->addComponent(c);

    v->validateModel(m);
    EXPECT_EQ(size_t(0), v->errorCount());
}

TEST(Validator, validMathInMultipleMathMLBlocksInvalidMathTagDuplicateIDs)
{
    const std::string math =
        "<math xmlns:cellml=\"http://www.cellml.org/cellml/2.0#\" xmlns=\"http://www.w3.org/1998/Math/MathML\">\n"
        "  <apply>\n"
        "    <eq/>\n"
        "    <ci>A</ci>\n"
        "    <apply>\n"
        "      <cn id=\"myId\" cellml:units=\"dimensionless\">1</cn>\n"
        "    </apply>\n"
        "  </apply>\n"
        "</math>\n"
        "<banana/>\n"
        "<math xmlns:cellml=\"http://www.cellml.org/cellml/2.0#\" xmlns=\"http://www.w3.org/1998/Math/MathML\">\n"
        "  <apply>\n"
        "    <eq/>\n"
        "    <ci id=\"myId\">B</ci>\n"
        "    <apply>\n"
        "      <cn cellml:units=\"dimensionless\">2</cn>\n"
        "    </apply>\n"
        "  </apply>\n"
        "</math>\n";
    const std::vector<std::string> expectedIssues = {
        "Math root node is of invalid type 'banana' on component 'componentName'. A valid math root node should be of type 'math'.",
        "Duplicated identifier attribute 'myId' has been found in:\n"
        " - MathML cn element in math in component 'componentName'; and\n"
        " - MathML ci element 'B' in math in component 'componentName'.\n",
    };
    libcellml::ValidatorPtr v = libcellml::Validator::create();
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c = libcellml::Component::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();

    m->setName("modelName");
    c->setName("componentName");
    v1->setName("A");
    v2->setName("B");
    v1->setUnits("dimensionless");
    v2->setUnits("dimensionless");

    c->addVariable(v1);
    c->addVariable(v2);
    c->setMath(math);
    m->addComponent(c);

    v->validateModel(m);

    EXPECT_EQ_ISSUES(expectedIssues, v);
}

TEST(Validator, invalidMath)
{
    const std::string math1 =
        "<math>\n"
        "  <invalid_xml></not_valid>\n"
        "</math>\n";
    const std::string math2 = "<invalid_math/>\n";
    const std::vector<std::string> expectedIssues = {
        "LibXml2 error: Opening and ending tag mismatch: invalid_xml line 2 and not_valid.",
        "Could not get a valid XML root node from the math on component 'componentName1'.",
        "Math root node is of invalid type 'invalid_math' on component 'componentName2'. A valid math root node should be of type 'math'.",
    };

    libcellml::ValidatorPtr v = libcellml::Validator::create();
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    libcellml::ComponentPtr c2 = libcellml::Component::create();

    m->setName("modelName");
    c1->setName("componentName1");
    c2->setName("componentName2");

    c1->setMath(math1);
    c2->setMath(math2);
    m->addComponent(c1);
    m->addComponent(c2);

    v->validateModel(m);

    EXPECT_EQ_ISSUES(expectedIssues, v);
}

TEST(Validator, invalidMathMLElements)
{
    const std::string math =
        "<math xmlns=\"http://www.w3.org/1998/Math/MathML\">\n"
        "  <apply>\n"
        "    <equals/>\n"
        "    <ci>C</ci>\n"
        "    <apply>\n"
        "      <addition/>\n"
        "      <ci>A</ci>\n"
        "      <ci>B</ci>\n"
        "    </apply>\n"
        "  </apply>\n"
        "</math>\n";
    const std::vector<std::string> expectedIssues = {
        "Math has a 'equals' element that is not a supported MathML element.",
        "Math has a 'addition' element that is not a supported MathML element.",
        "No declaration for element equals.",
        "No declaration for element addition.",
    };
    // Note: the MathML DTD also gives issues that list every possible operator when an
    //       invalid option is given. We'll just explicitly check the less verbose issues here.

    libcellml::ValidatorPtr v = libcellml::Validator::create();
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c = libcellml::Component::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    libcellml::VariablePtr v3 = libcellml::Variable::create();

    m->setName("modelName");
    c->setName("componentName");
    v1->setName("A");
    v2->setName("B");
    v3->setName("C");
    v1->setInitialValue("1.0");
    v2->setInitialValue("-1.0");
    v1->setUnits("dimensionless");
    v2->setUnits("dimensionless");
    v3->setUnits("dimensionless");

    c->addVariable(v1);
    c->addVariable(v2);
    c->addVariable(v3);
    c->setMath(math);
    m->addComponent(c);

    v->validateModel(m);

    // Check for two expected error messages (see note above).
    for (size_t i = 0; i < 2; ++i) {
        EXPECT_EQ(expectedIssues.at(i), v->issue(i)->description());
    }
}

TEST(Validator, invalidMathMLVariables)
{
    const std::string math =
        "<math xmlns=\"http://www.w3.org/1998/Math/MathML\">\n"
        "  <apply>\n"
        "    <eq/>\n"
        "    <ci>answer</ci>\n"
        "    <partialdiff/>\n"
        "    <apply>\n"
        "      <plus/>\n"
        "      <ci>A</ci>\n"
        "      <apply>\n"
        "        <plus/>\n"
        "        <bvar>\n"
        "          <ci>new_bvar</ci>\n"
        "        </bvar>\n"
        "        <apply>\n"
        "          <plus/>\n"
        "          <ci>   </ci>\n"
        "          <apply>\n"
        "            <plus/>\n"
        "            <ci><nonsense/></ci>\n"
        "            <apply>\n"
        "              <plus/>\n"
        "              <ci/>\n"
        "              <bvar>\n"
        "                <ci>\n"
        "                  <!-- Invalid bvar -->\n"
        "                  B\n"
        "                </ci>\n"
        "              </bvar>\n"
        "            </apply>\n"
        "          </apply>\n"
        "        </apply>\n"
        "      </apply>\n"
        "    </apply>\n"
        "  </apply>\n"
        "</math>\n";
    const std::vector<std::string> expectedIssues = {
        "Math has a 'partialdiff' element that is not a supported MathML element.",
        "Math has a 'nonsense' element that is not a supported MathML element.",
        "MathML ci element has the child text 'answer' which does not correspond with any variable names present in component 'componentName'.",
        "MathML ci element has the child text 'new_bvar' which does not correspond with any variable names present in component 'componentName'.",
        "W3C MathML DTD error: Element nonsense is not declared in ci list of possible children.",
        "W3C MathML DTD error: No declaration for element nonsense.",
        "Math has a 'eq' element without exactly two MathML siblings.",
        "Math has a 'bvar' element which first sibling is not a 'diff' element.",
        "Math has a 'ci' element with no identifier as a child.",
        "Math has a 'ci' element with no identifier as a child.",
        "Math has a 'bvar' element which first sibling is not a 'diff' element.",
    };

    libcellml::ValidatorPtr v = libcellml::Validator::create();
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c = libcellml::Component::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    libcellml::VariablePtr v3 = libcellml::Variable::create();

    m->setName("modelName");
    c->setName("componentName");
    v1->setName("A");
    v2->setName("B");
    v3->setName("C");
    v1->setInitialValue("1.0");
    v2->setInitialValue("-1.0");
    v1->setUnits("dimensionless");
    v2->setUnits("dimensionless");
    v3->setUnits("dimensionless");

    c->addVariable(v1);
    c->addVariable(v2);
    c->addVariable(v3);
    c->setMath(math);
    m->addComponent(c);

    v->validateModel(m);

    EXPECT_EQ_ISSUES(expectedIssues, v);
}

TEST(Validator, invalidSimpleMathmlCellMLUnits)
{
    const std::string math =
        "<math xmlns:cellml=\"http://www.cellml.org/cellml/2.0#\" xmlns=\"http://www.w3.org/1998/Math/MathML\">\n"
        "  <apply>\""
        "    <bvar>\n"
        "      <ci cellml:units=\"dimensionless\">B</ci>\n"
        "    </bvar>\n"
        "  </apply>\n"
        "</math>";
    const std::vector<std::string> expectedIssues = {
        "Model '' does not have a valid name attribute. CellML identifiers must contain one or more basic Latin alphabetic characters.",
        "Component '' does not have a valid name attribute. CellML identifiers must contain one or more basic Latin alphabetic characters.",
        "MathML ci element has the child text 'B' which does not correspond with any variable names present in component ''.",
        "W3C MathML DTD error: Element apply content does not follow the DTD, expecting (csymbol | ci | cn | apply | reln | lambda | condition | declare | sep | semantics | annotation | annotation-xml | integers | reals | rationals | naturalnumbers | complexes | primes | exponentiale | imaginaryi | notanumber | true | false | emptyset | pi | eulergamma | infinity | interval | list | matrix | matrixrow | set | vector | piecewise | lowlimit | uplimit | bvar | degree | logbase | momentabout | domainofapplication | inverse | ident | domain | codomain | image | abs | conjugate | exp | factorial | arg | real | imaginary | floor | ceiling | not | ln | sin | cos | tan | sec | csc | cot | sinh | cosh | tanh | sech | csch | coth | arcsin | arccos | arctan | arccosh | arccot | arccoth | arccsc | arccsch | arcsec | arcsech | arcsinh | arctanh | determinant | transpose | card | quotient | divide | power | rem | implies | vectorproduct | scalarproduct | outerproduct | setdiff | fn | compose | plus | times | max | min | gcd | lcm | and | or | xor | union | intersect | cartesianproduct | mean | sdev | variance | median | mode | selector | root | minus | log | int | diff | partialdiff | divergence | grad | curl | laplacian | sum | product | limit | moment | exists | forall | neq | factorof | in | notin | notsubset | notprsubset | tendsto | eq | leq | lt | geq | gt | equivalent | approx | subset | prsubset | mi | mn | mo | mtext | ms | mspace | mrow | mfrac | msqrt | mroot | menclose | mstyle | merror | mpadded | mphantom | mfenced | msub | msup | msubsup | munder | mover | munderover | mmultiscripts | mtable | mtr | mlabeledtr | mtd | maligngroup | malignmark | maction)*, got (CDATA bvar ).",
        "W3C MathML DTD error: No declaration for attribute units of element ci.",
        "Math has a 'bvar' element without exactly two MathML siblings.",
    };
    libcellml::ValidatorPtr v = libcellml::Validator::create();
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c = libcellml::Component::create();

    c->setMath(math);
    m->addComponent(c);

    v->validateModel(m);
    EXPECT_EQ_ISSUES(expectedIssues, v);
}

TEST(Validator, invalidMathmlCellMLNsOnNode)
{
    const std::string math =
        "<math  xmlns:cellml=\"http://www.cellml.org/cellml/2.0#\" xmlns=\"http://www.w3.org/1998/Math/MathML\"><apply><cellml:bvar><ci cellml:units=\"dimensionless\">B</ci></cellml:bvar></apply></math>";
    const std::vector<std::string> expectedIssues = {
        "Model '' does not have a valid name attribute. CellML identifiers must contain one or more basic Latin alphabetic characters.",
        "Component '' does not have a valid name attribute. CellML identifiers must contain one or more basic Latin alphabetic characters.",
        "Math has a 'bvar' element that is not a supported MathML element.",
        "MathML ci element has the child text 'B' which does not correspond with any variable names present in component ''.",
        "W3C MathML DTD error: No declaration for attribute units of element ci.",
        "Math has a 'bvar' element without exactly two MathML siblings.",
    };
    libcellml::ValidatorPtr v = libcellml::Validator::create();
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c = libcellml::Component::create();

    c->setMath(math);
    m->addComponent(c);

    v->validateModel(m);
    EXPECT_EQ_ISSUES(expectedIssues, v);
}

TEST(Validator, invalidMathMLCiAndCnElementsWithCellMLUnits)
{
    const std::string math =
        "<math xmlns:cellml=\"http://www.cellml.org/cellml/2.0#\" xmlns=\"http://www.w3.org/1998/Math/MathML\">\n"
        "  <apply>\n"
        "    <eq/>\n"
        "    <cn cellml:units=\"invalid\" cellml:value=\"zero\">oops</cn>\n"
        "    <apply>\n"
        "      <plus/>\n"
        "      <ci>A</ci>\n"
        "      <apply>\n"
        "        <plus/>\n"
        "        <bvar>\n"
        "          <ci cellml:units=\"dimensionless\">new_bvar</ci>\n"
        "        </bvar>\n"
        "        <apply>\n"
        "          <plus/>\n"
        "          <ci>   </ci>\n"
        "          <apply>\n"
        "            <plus/>\n"
        "            <ci>undefined_variable</ci>\n"
        "            <apply>\n"
        "              <plus/>\n"
        "              <ci/>\n"
        "              <bvar>\n"
        "                <ci cellml:units=\"9wayswrong\">B</ci>\n"
        "              </bvar>\n"
        "              <apply>\n"
        "                <plus/>\n"
        "                <cn>2.0</cn>\n"
        "              </apply>\n"
        "            </apply>\n"
        "          </apply>\n"
        "        </apply>\n"
        "      </apply>\n"
        "    </apply>\n"
        "  </apply>\n"
        "</math>\n";
    const std::vector<std::string> expectedIssues = {
        "Math cn element has an invalid attribute type 'value' in the cellml namespace. Attribute 'units' is the only CellML namespace attribute allowed.",
        "Math has a cn element with a cellml:units attribute 'invalid' that is not a valid reference to units in the model 'modelName' or a standard unit.",
        "MathML ci element has the child text 'new_bvar' which does not correspond with any variable names present in component 'componentName'.",
        "MathML ci element has the child text 'undefined_variable' which does not correspond with any variable names present in component 'componentName'.",
        "Math cn element with the value '2.0' does not have a valid cellml:units attribute. CellML identifiers must contain one or more basic Latin alphabetic characters.",
        "W3C MathML DTD error: No declaration for attribute units of element ci.",
        "W3C MathML DTD error: No declaration for attribute units of element ci.",
        "Math has a 'cn' element of 'real' type with no valid text node (representing a basic number) as a child.",
        "Math has a 'bvar' element which first sibling is not a 'diff' element.",
        "Math has a 'ci' element with no identifier as a child.",
        "Math has a 'ci' element with no identifier as a child.",
        "Math has a 'bvar' element without exactly two MathML siblings.",
    };

    libcellml::ValidatorPtr v = libcellml::Validator::create();
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c = libcellml::Component::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    libcellml::VariablePtr v3 = libcellml::Variable::create();

    m->setName("modelName");
    c->setName("componentName");
    v1->setName("A");
    v2->setName("B");
    v3->setName("C");
    v1->setInitialValue("1.0");
    v2->setInitialValue("-1.0");
    v1->setUnits("dimensionless");
    v2->setUnits("dimensionless");
    v3->setUnits("dimensionless");

    c->addVariable(v1);
    c->addVariable(v2);
    c->addVariable(v3);
    c->setMath(math);
    m->addComponent(c);

    v->validateModel(m);

    EXPECT_EQ_ISSUES(expectedIssues, v);
}

TEST(Validator, validMathMLCiAndCnElementsWithCellMLUnits)
{
    const std::string math =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<math xmlns=\"http://www.w3.org/1998/Math/MathML\" xmlns:cellml=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <apply>\n"
        "    <eq/>\n"
        "    <cn cellml:units=\"dimensionless\">3</cn>\n"
        "    <apply>\n"
        "      <plus/>\n"
        "      <ci>A</ci>\n"
        "      <apply>\n"
        "        <plus/>\n"
        "        <ci>C</ci>\n"
        "        <cn cellml:units=\"dimensionless\">7</cn>\n"
        "      </apply>\n"
        "    </apply>\n"
        "  </apply>\n"
        "</math>\n";

    libcellml::ValidatorPtr v = libcellml::Validator::create();
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c = libcellml::Component::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    libcellml::VariablePtr v3 = libcellml::Variable::create();

    m->setName("modelName");
    c->setName("componentName");
    v1->setName("A");
    v2->setName("B");
    v3->setName("C");
    v1->setInitialValue("1.0");
    v2->setInitialValue("-1.0");
    v1->setUnits("dimensionless");
    v2->setUnits("dimensionless");
    v3->setUnits("dimensionless");

    c->addVariable(v1);
    c->addVariable(v2);
    c->addVariable(v3);
    c->setMath(math);
    m->addComponent(c);

    v->validateModel(m);
    EXPECT_EQ(size_t(0), v->issueCount());
}

TEST(Validator, parseAndValidateInvalidUnitIssues)
{
    const std::string in =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"asoiaf\">\n"
        "  <units name=\"ampere\"/>\n"
        "  <units name=\"north\"/>\n"
        "  <units name=\"stark\">\n"
        "    <unit units=\"volt\" prefix=\"mega\" multiplier=\"1000.0\"/>\n"
        "    <unit units=\"north\"/>\n"
        "    <unit units=\"ned\"/>\n"
        "    <unit units=\"king in the north\"/>\n"
        "    <unit prefix=\"wolf\" units=\"metre\"/>\n"
        "  </units>\n"
        "</model>\n";
    const std::vector<std::string> expectedIssues = {
        "Units is named 'ampere' which is a protected standard unit name.",
        "Units reference 'ned' in units 'stark' is not a valid reference to a local units or a standard unit type.",
        "Unit in units 'stark' does not have a valid units reference. The reference given is 'king in the north'. CellML identifiers must not contain any characters other than [a-zA-Z0-9_].",
        "Prefix 'wolf' of a unit referencing 'metre' in units 'stark' is not a valid integer or an SI prefix.",
    };

    libcellml::ParserPtr p = libcellml::Parser::create();
    libcellml::ModelPtr m = p->parseModel(in);
    EXPECT_EQ(size_t(0), p->issueCount());

    libcellml::ValidatorPtr v = libcellml::Validator::create();
    v->validateModel(m);
    EXPECT_EQ_ISSUES(expectedIssues, v);
}

TEST(Validator, validateInvalidConnectionsVariableWithoutParentComponent)
{
    const std::vector<std::string> expectedIssues = {
        "Variable 'variable2' is an equivalent variable to 'variable1' but 'variable2' has no parent component.",
    };
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr comp1 = libcellml::Component::create();
    libcellml::ComponentPtr comp2 = libcellml::Component::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();

    m->setName("modelName");
    comp1->setName("component1");
    comp2->setName("component2");

    v1->setName("variable1");
    v2->setName("variable2");
    v1->setUnits("dimensionless");
    v2->setUnits("dimensionless");

    comp1->addVariable(v1);
    comp2->addVariable(v2);

    m->addComponent(comp1);
    m->addComponent(comp2);

    libcellml::Variable::addEquivalence(v1, v2);

    comp2->removeVariable(v2);

    libcellml::ValidatorPtr v = libcellml::Validator::create();
    v->validateModel(m);
    EXPECT_EQ_ISSUES(expectedIssues, v);
}

TEST(Validator, integerStrings)
{
    const std::string in =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"asoiaf\">\n"
        "  <component name=\"component\">\n"
        "    <variable name=\"variable\" units=\"dimensionless\"/>\n"
        "    <variable name=\"other_variable\" units=\"dimensionless\"/>\n"
        "    <reset variable=\"variable\" test_variable=\"other_variable\" order=\"1\">\n"
        "      <test_value>\n"
        "        <math xmlns=\"http://www.w3.org/1998/Math/MathML\">\n"
        "        </math>\n"
        "      </test_value>\n"
        "      <reset_value>\n"
        "        <math xmlns=\"http://www.w3.org/1998/Math/MathML\">\n"
        "        </math>\n"
        "      </reset_value>\n"
        "    </reset>\n"
        "    <reset variable=\"variable\" test_variable=\"other_variable\" order=\"+2\">\n"
        "      <test_value>\n"
        "        <math xmlns=\"http://www.w3.org/1998/Math/MathML\">\n"
        "        </math>\n"
        "      </test_value>\n"
        "      <reset_value>\n"
        "        <math xmlns=\"http://www.w3.org/1998/Math/MathML\">\n"
        "        </math>\n"
        "      </reset_value>\n"
        "    </reset>\n"
        "    <reset variable=\"variable\" test_variable=\"other_variable\" order=\"\">\n"
        "      <test_value>\n"
        "        <math xmlns=\"http://www.w3.org/1998/Math/MathML\">\n"
        "        </math>\n"
        "      </test_value>\n"
        "      <reset_value>\n"
        "        <math xmlns=\"http://www.w3.org/1998/Math/MathML\">\n"
        "        </math>\n"
        "      </reset_value>\n"
        "    </reset>\n"
        "    <reset variable=\"variable\" test_variable=\"other_variable\" order=\"-\">\n"
        "      <test_value>\n"
        "        <math xmlns=\"http://www.w3.org/1998/Math/MathML\">\n"
        "        </math>\n"
        "      </test_value>\n"
        "      <reset_value>\n"
        "        <math xmlns=\"http://www.w3.org/1998/Math/MathML\">\n"
        "        </math>\n"
        "      </reset_value>\n"
        "    </reset>\n"
        "    <reset variable=\"variable\" test_variable=\"other_variable\" order=\"odd\">\n"
        "      <test_value>\n"
        "        <math xmlns=\"http://www.w3.org/1998/Math/MathML\">\n"
        "        </math>\n"
        "      </test_value>\n"
        "      <reset_value>\n"
        "        <math xmlns=\"http://www.w3.org/1998/Math/MathML\">\n"
        "        </math>\n"
        "      </reset_value>\n"
        "    </reset>\n"
        "    <reset variable=\"variable\" test_variable=\"other_variable\" order=\"12345678901\">\n"
        "      <test_value>\n"
        "        <math xmlns=\"http://www.w3.org/1998/Math/MathML\">\n"
        "        </math>\n"
        "      </test_value>\n"
        "      <reset_value>\n"
        "        <math xmlns=\"http://www.w3.org/1998/Math/MathML\">\n"
        "        </math>\n"
        "      </reset_value>\n"
        "    </reset>\n"
        "  </component>\n"
        "</model>\n";

    const std::vector<std::string> expectedParsingErrors = {
        "Reset in component 'component' referencing variable 'variable' has a non-integer order value ''.",
        "Reset in component 'component' referencing variable 'variable' has a non-integer order value '-'.",
        "Reset in component 'component' referencing variable 'variable' has a non-integer order value 'odd'.",
        "Reset in component 'component' referencing variable 'variable' has an out of range integer order value '12345678901'.",
    };

    const std::vector<std::string> expectedValidationErrors = {
        "Reset in component 'component' with variable 'variable', with test_variable 'other_variable', does not have an order set.",
        "Reset in component 'component' with variable 'variable', with test_variable 'other_variable', does not have an order set.",
        "Reset in component 'component' with variable 'variable', with test_variable 'other_variable', does not have an order set.",
        "Reset in component 'component' with variable 'variable', with test_variable 'other_variable', does not have an order set.",
    };

    libcellml::ParserPtr p = libcellml::Parser::create();
    libcellml::ModelPtr m = p->parseModel(in);

    EXPECT_EQ_ISSUES(expectedParsingErrors, p);

    libcellml::ValidatorPtr v = libcellml::Validator::create();
    v->validateModel(m);

    EXPECT_EQ_ISSUES(expectedValidationErrors, v);
}

TEST(Validator, resetValid)
{
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c = libcellml::Component::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    libcellml::ResetPtr r = libcellml::Reset::create();

    r->setVariable(v1);
    r->setTestVariable(v2);
    r->setOrder(1);
    r->setResetValue(NON_EMPTY_MATH);
    r->setTestValue(NON_EMPTY_MATH);

    c->setName("comp");
    v1->setName("var");
    v1->setUnits("second");
    v2->setName("var2");
    v2->setUnits("second");

    c->addVariable(v1);
    c->addVariable(v2);

    c->addReset(r);

    m->setName("main");
    m->addComponent(c);

    libcellml::ValidatorPtr validator = libcellml::Validator::create();
    validator->validateModel(m);

    EXPECT_EQ(size_t(0), validator->issueCount());
}

TEST(Validator, resetNoVariable)
{
    const std::string expectedError =
        "Reset in component 'comp' with order '3', with test_variable 'var2', does not reference a variable.";

    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c = libcellml::Component::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    libcellml::ResetPtr r = libcellml::Reset::create();

    // No variable
    r->setTestVariable(v2);
    r->setOrder(3);
    r->setResetValue(NON_EMPTY_MATH);
    r->setTestValue(NON_EMPTY_MATH);

    c->setName("comp");
    v1->setName("var");
    v1->setUnits("second");
    v2->setName("var2");
    v2->setUnits("second");

    c->addVariable(v1);
    c->addVariable(v2);

    c->addReset(r);

    m->setName("main");
    m->addComponent(c);

    libcellml::ValidatorPtr validator = libcellml::Validator::create();
    validator->validateModel(m);

    EXPECT_EQ(size_t(1), validator->issueCount());
    EXPECT_EQ(expectedError, validator->issue(0)->description());
}

TEST(Validator, resetNoTestVariable)
{
    const std::string expectedError =
        "Reset in component 'comp' with order '4', with variable 'var', does not reference a test_variable.";

    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c = libcellml::Component::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    libcellml::ResetPtr r = libcellml::Reset::create();

    // No test_variable
    r->setVariable(v1);
    r->setOrder(4);
    r->setResetValue(NON_EMPTY_MATH);
    r->setTestValue(NON_EMPTY_MATH);

    c->setName("comp");
    v1->setName("var");
    v1->setUnits("second");
    v2->setName("var2");
    v2->setUnits("second");

    c->addVariable(v1);
    c->addVariable(v2);

    c->addReset(r);

    m->setName("main");
    m->addComponent(c);

    libcellml::ValidatorPtr validator = libcellml::Validator::create();
    validator->validateModel(m);

    EXPECT_EQ(size_t(1), validator->issueCount());
    EXPECT_EQ(expectedError, validator->issue(0)->description());
}

TEST(Validator, resetNoOrder)
{
    const std::string expectedError =
        "Reset in component 'comp' with variable 'var', with test_variable 'var2', does not have an order set.";

    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c = libcellml::Component::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    libcellml::ResetPtr r = libcellml::Reset::create();

    // No order
    r->setVariable(v1);
    r->setTestVariable(v2);
    r->setResetValue(NON_EMPTY_MATH);
    r->setTestValue(NON_EMPTY_MATH);

    c->setName("comp");
    v1->setName("var");
    v1->setUnits("second");
    v2->setName("var2");
    v2->setUnits("second");

    c->addVariable(v1);
    c->addVariable(v2);

    c->addReset(r);

    m->setName("main");
    m->addComponent(c);

    libcellml::ValidatorPtr validator = libcellml::Validator::create();
    validator->validateModel(m);
    EXPECT_EQ(size_t(1), validator->issueCount());
    EXPECT_EQ(expectedError, validator->issue(0)->description());
}

TEST(Validator, resetNoResetValue)
{
    const std::string expectedError = "Reset in component 'comp' with order '6', with variable 'var', with test_variable 'var2', does not have a reset_value specified.";

    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c = libcellml::Component::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    libcellml::ResetPtr r = libcellml::Reset::create();

    // No reset_value
    r->setVariable(v1);
    r->setTestVariable(v2);
    r->setOrder(6);
    r->setTestValue(NON_EMPTY_MATH);

    c->setName("comp");
    v1->setName("var");
    v1->setUnits("second");
    v2->setName("var2");
    v2->setUnits("second");

    c->addVariable(v1);
    c->addVariable(v2);

    c->addReset(r);

    m->setName("main");
    m->addComponent(c);

    libcellml::ValidatorPtr validator = libcellml::Validator::create();
    validator->validateModel(m);

    EXPECT_EQ(size_t(1), validator->issueCount());
    EXPECT_EQ(expectedError, validator->issue(0)->description());
}

TEST(Validator, resetMultipleMathMLInResetValue)
{
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c = libcellml::Component::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    libcellml::ResetPtr r = libcellml::Reset::create();

    // Multiple MathML in reset_value
    r->setVariable(v1);
    r->setTestVariable(v2);
    r->setOrder(6);
    r->setResetValue(NON_EMPTY_MATH + NON_EMPTY_MATH);
    r->setTestValue(NON_EMPTY_MATH);

    c->setName("comp");
    v1->setName("var");
    v1->setUnits("second");
    v2->setName("var2");
    v2->setUnits("second");

    c->addVariable(v1);
    c->addVariable(v2);

    c->addReset(r);

    m->setName("main");
    m->addComponent(c);

    libcellml::ValidatorPtr validator = libcellml::Validator::create();
    validator->validateModel(m);
    EXPECT_EQ(size_t(0), validator->errorCount());
}

TEST(Validator, resetNoTestValue)
{
    const std::vector<std::string> expectedIssues = {
        "Reset in component 'comp' with order '7', with variable 'var', with test_variable 'var2', does not have a test_value specified.",
    };

    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c = libcellml::Component::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    libcellml::ResetPtr r = libcellml::Reset::create();

    // No test_value
    r->setVariable(v1);
    r->setTestVariable(v2);
    r->setOrder(7);
    r->setResetValue(NON_EMPTY_MATH);

    c->setName("comp");
    v1->setName("var");
    v1->setUnits("second");
    v2->setName("var2");
    v2->setUnits("second");

    c->addVariable(v1);
    c->addVariable(v2);

    c->addReset(r);

    m->setName("main");
    m->addComponent(c);

    libcellml::ValidatorPtr validator = libcellml::Validator::create();
    validator->validateModel(m);

    EXPECT_EQ_ISSUES(expectedIssues, validator);
}

TEST(Validator, resetMultipleMathMLInTestValue)
{
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c = libcellml::Component::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    libcellml::ResetPtr r = libcellml::Reset::create();

    // Multiple MathML blocks in test_value
    r->setVariable(v1);
    r->setTestVariable(v2);
    r->setOrder(6);
    r->setResetValue(NON_EMPTY_MATH);
    r->setTestValue(NON_EMPTY_MATH + NON_EMPTY_MATH);

    c->setName("comp");
    v1->setName("var");
    v1->setUnits("second");
    v2->setName("var2");
    v2->setUnits("second");

    c->addVariable(v1);
    c->addVariable(v2);

    c->addReset(r);

    m->setName("main");
    m->addComponent(c);

    libcellml::ValidatorPtr validator = libcellml::Validator::create();
    validator->validateModel(m);
    EXPECT_EQ(size_t(0), validator->errorCount());
}

TEST(Validator, resetWhitespaceAsMaths)
{
    const std::vector<std::string> expectedIssues = {
        "Reset in component 'comp' with order '8', with variable 'var', with test_variable 'var2', does not have a test_value specified.",
        "Reset in component 'comp' with order '8', with variable 'var', with test_variable 'var2', does not have a reset_value specified.",
    };

    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c = libcellml::Component::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    libcellml::ResetPtr r = libcellml::Reset::create();

    // Whitespace test_value and reset_value
    r->setVariable(v1);
    r->setTestVariable(v2);
    r->setOrder(8);
    r->setResetValue(" ");
    r->setTestValue(" ");

    c->setName("comp");
    v1->setName("var");
    v1->setUnits("second");
    v2->setName("var2");
    v2->setUnits("second");

    c->addVariable(v1);
    c->addVariable(v2);

    c->addReset(r);

    m->setName("main");
    m->addComponent(c);

    libcellml::ValidatorPtr validator = libcellml::Validator::create();
    validator->validateModel(m);

    EXPECT_EQ_ISSUES(expectedIssues, validator);
}

TEST(Validator, resetEmptyMathML)
{
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c = libcellml::Component::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    libcellml::ResetPtr r = libcellml::Reset::create();

    // Empty test_value and reset_value math block
    // TODO: For now, empty maths blocks are valid. Issue #365.
    r->setVariable(v1);
    r->setTestVariable(v2);
    r->setOrder(9);
    r->setResetValue(EMPTY_MATH);
    r->setTestValue(EMPTY_MATH);

    c->setName("comp");
    v1->setName("var");
    v1->setUnits("second");
    v2->setName("var2");
    v2->setUnits("second");

    c->addVariable(v1);
    c->addVariable(v2);

    c->addReset(r);

    m->setName("main");
    m->addComponent(c);

    libcellml::ValidatorPtr validator = libcellml::Validator::create();
    validator->validateModel(m);

    EXPECT_EQ(size_t(0), validator->issueCount());
}

TEST(Validator, resetNegativeOrder)
{
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c = libcellml::Component::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    libcellml::ResetPtr r = libcellml::Reset::create();

    // Negative order value should be allowed
    r->setVariable(v1);
    r->setTestVariable(v2);
    r->setOrder(-100);
    r->setResetValue(NON_EMPTY_MATH);
    r->setTestValue(NON_EMPTY_MATH);

    c->setName("comp");
    v1->setName("var");
    v1->setUnits("second");
    v2->setName("var2");
    v2->setUnits("second");

    c->addVariable(v1);
    c->addVariable(v2);

    c->addReset(r);

    m->setName("main");
    m->addComponent(c);

    libcellml::ValidatorPtr validator = libcellml::Validator::create();
    validator->validateModel(m);

    EXPECT_EQ(size_t(0), validator->issueCount());
}

TEST(Validator, resetVariableOutsideComponent)
{
    const std::vector<std::string> expectedIssues = {
        "Reset in component 'c1' with order '1', with variable 'v2', with test_variable 'v1', refers to a variable 'v2' in a different component 'c2'.",
        "Reset in component 'c2' with order '2', with variable 'v2', with test_variable 'v1', refers to a test_variable 'v1' in a different component 'c1'.",
    };

    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    libcellml::ComponentPtr c2 = libcellml::Component::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    libcellml::ResetPtr r1 = libcellml::Reset::create();
    libcellml::ResetPtr r2 = libcellml::Reset::create();
    libcellml::ValidatorPtr validator = libcellml::Validator::create();

    v1->setName("v1");
    v1->setUnits("dimensionless");
    v2->setName("v2");
    v2->setUnits("dimensionless");

    c1->setName("c1");
    c2->setName("c2");

    c1->addVariable(v1);
    c2->addVariable(v2);

    c1->addReset(r1);
    c2->addReset(r2);

    r1->setVariable(v2); // variable outside parent component
    r1->setTestVariable(v1);
    r1->setOrder(1);
    r1->setResetValue(EMPTY_MATH);
    r1->setTestValue(EMPTY_MATH);

    r2->setVariable(v2);
    r2->setTestVariable(v1); // test_variable outside parent component
    r2->setOrder(2);
    r2->setResetValue(EMPTY_MATH);
    r2->setTestValue(EMPTY_MATH);

    m->setName("model");
    m->addComponent(c1);
    m->addComponent(c2);

    validator->validateModel(m);

    EXPECT_EQ_ISSUES(expectedIssues, validator);
}

libcellml::ModelPtr setupResetOrderModel()
{
    const std::string in =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"reset_model\">\n"
        "  <component name=\"reset_component1\">\n"
        "    <variable name=\"A\" units=\"dimensionless\" interface=\"public\"/>\n"
        "    <variable name=\"B\" units=\"dimensionless\"/>\n"
        "    <variable name=\"C\" units=\"dimensionless\"/>\n"
        "    <reset variable=\"A\" test_variable=\"B\" order=\"1\">\n"
        "      <reset_value>\n"
        "        <math xmlns=\"http://www.w3.org/1998/Math/MathML\"/>\n"
        "      </reset_value>\n"
        "      <test_value>\n"
        "        <math xmlns=\"http://www.w3.org/1998/Math/MathML\"/>\n"
        "      </test_value>\n"
        "    </reset>\n"
        "    <reset variable=\"A\" test_variable=\"C\" order=\"2\">\n"
        "      <reset_value>\n"
        "        <math xmlns=\"http://www.w3.org/1998/Math/MathML\"/>\n"
        "      </reset_value>\n"
        "      <test_value>\n"
        "        <math xmlns=\"http://www.w3.org/1998/Math/MathML\"/>\n"
        "      </test_value>\n"
        "    </reset>\n"
        "  </component>\n"
        "  <component name=\"reset_component2\">\n"
        "    <variable name=\"X\" units=\"dimensionless\" interface=\"public\"/>\n"
        "    <variable name=\"Y\" units=\"dimensionless\"/>\n"
        "    <variable name=\"Z\" units=\"dimensionless\"/>\n"
        "    <reset variable=\"X\" test_variable=\"Y\" order=\"3\">\n"
        "      <reset_value>\n"
        "        <math xmlns=\"http://www.w3.org/1998/Math/MathML\"/>\n"
        "      </reset_value>\n"
        "      <test_value>\n"
        "        <math xmlns=\"http://www.w3.org/1998/Math/MathML\"/>\n"
        "      </test_value>\n"
        "    </reset>\n"
        "    <reset variable=\"Z\" test_variable=\"Y\" order=\"4\">\n"
        "      <reset_value>\n"
        "        <math xmlns=\"http://www.w3.org/1998/Math/MathML\"/>\n"
        "      </reset_value>\n"
        "      <test_value>\n"
        "        <math xmlns=\"http://www.w3.org/1998/Math/MathML\"/>\n"
        "      </test_value>\n"
        "    </reset>\n"
        "  </component>\n"
        "  <connection component_1=\"reset_component1\" component_2=\"reset_component2\">\n"
        "    <map_variables variable_1=\"A\" variable_2=\"X\"/>\n"
        "  </connection>\n"
        "</model>\n";

    libcellml::ParserPtr p = libcellml::Parser::create();
    EXPECT_EQ(size_t(0), p->errorCount());

    return p->parseModel(in);
}

TEST(Validator, resetOrderSameVariableSetUnique)
{
    libcellml::ValidatorPtr v = libcellml::Validator::create();

    auto m = setupResetOrderModel();

    v->validateModel(m);

    EXPECT_EQ(size_t(0), v->errorCount());
}

TEST(Validator, resetOrderVariableSetInComponentNotUnique)
{
    const std::vector<std::string> expectedIssues = {
        "Variable 'A' used in resets does not have unique order values across the equivalent variable set.",
    };

    libcellml::ValidatorPtr v = libcellml::Validator::create();

    auto m = setupResetOrderModel();

    auto c = m->component(0);
    auto r = c->reset(1);
    r->setOrder(1);

    v->validateModel(m);

    EXPECT_EQ_ISSUES(expectedIssues, v);
}

TEST(Validator, resetOrderVariableSetInEquivalentSetNotUnique)
{
    const std::vector<std::string> expectedIssues = {
        "Variable 'A' used in resets does not have unique order values across the equivalent variable set.",
    };

    libcellml::ValidatorPtr v = libcellml::Validator::create();

    auto m = setupResetOrderModel();

    auto c = m->component(1);
    auto r = c->reset(0);
    r->setOrder(1);

    v->validateModel(m);

    EXPECT_EQ_ISSUES(expectedIssues, v);
}

TEST(Validator, validMathCnElements)
{
    const std::string math =
        "<math xmlns:cellml=\"http://www.cellml.org/cellml/2.0#\" xmlns=\"http://www.w3.org/1998/Math/MathML\">\n"
        "  <apply>\n"
        "    <eq/>\n"
        "    <ci>C</ci>\n"
        "    <apply>\n"
        "      <plus/>\n"
        "      <cn cellml:units=\"dimensionless\" type=\"e-notation\">3.44<sep/>2</cn>\n"
        "      <cn cellml:units=\"dimensionless\">-9.612</cn>\n"
        "    </apply>\n"
        "  </apply>\n"
        "</math>\n";

    libcellml::ValidatorPtr v = libcellml::Validator::create();
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c = libcellml::Component::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();

    m->setName("modelName");
    c->setName("componentName");
    v1->setName("C");
    v1->setInitialValue("3.5");
    v1->setUnits("dimensionless");

    c->addVariable(v1);
    c->setMath(math);
    m->addComponent(c);

    v->validateModel(m);
    EXPECT_EQ(size_t(0), v->issueCount());
}

TEST(Validator, validMathCnElementsMissingCellMLNamespace)
{
    const std::string math =
        "<math xmlns=\"http://www.w3.org/1998/Math/MathML\">\n"
        "  <apply>\n"
        "    <eq/>\n"
        "    <ci>C</ci>\n"
        "    <apply>\n"
        "      <plus/>\n"
        "      <cn cellml:units=\"dimensionless\" type=\"e-notation\">3.44<sep/>2</cn>\n"
        "      <cn cellml:units=\"dimensionless\">-9.612</cn>\n"
        "    </apply>\n"
        "  </apply>\n"
        "</math>\n";

    const std::vector<std::string> expectedIssues {
        "LibXml2 error: Namespace prefix cellml for units on cn is not defined.",
        "LibXml2 error: Namespace prefix cellml for units on cn is not defined.",
        "Math cn element with the value '3.44' does not have a valid cellml:units attribute. CellML identifiers must contain one or more basic Latin alphabetic characters.",
        "Math cn element with the value '-9.612' does not have a valid cellml:units attribute. CellML identifiers must contain one or more basic Latin alphabetic characters.",
        "W3C MathML DTD error: Namespace prefix cellml for units on cn is not defined.",
        "W3C MathML DTD error: Namespace prefix cellml for units on cn is not defined.",
        "W3C MathML DTD error: No declaration for attribute cellml:units of element cn.",
        "W3C MathML DTD error: No declaration for attribute cellml:units of element cn.",
    };

    libcellml::ValidatorPtr v = libcellml::Validator::create();
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c = libcellml::Component::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();

    m->setName("modelName");
    c->setName("componentName");
    v1->setName("C");
    v1->setInitialValue("3.5");
    v1->setUnits("dimensionless");

    c->addVariable(v1);
    c->setMath(math);
    m->addComponent(c);

    v->validateModel(m);
    EXPECT_EQ_ISSUES(expectedIssues, v);
}

TEST(Validator, unitAmericanSpellingOfUnitsRemoved)
{
    const std::vector<std::string> expectedIssues = {
        "Units reference 'meter' in units 'testunit2' is not a valid reference to a local units or a standard unit type.",
        "Variable 'tomayto' in component 'comp1' has units of 'testunit1' and an equivalent variable 'tomahto' in component 'comp2' with non-matching units of 'testunit2'. The mismatch is: metre^1.",
    };

    libcellml::ValidatorPtr validator = libcellml::Validator::create();
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr comp1 = libcellml::Component::create();
    libcellml::ComponentPtr comp2 = libcellml::Component::create();

    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();

    v1->setName("tomayto");
    v1->setInterfaceType("public");
    v2->setName("tomahto");
    v2->setInterfaceType("public");
    m->setName("callthewholethingoff");
    comp1->addVariable(v1);
    comp2->addVariable(v2);
    comp1->setName("comp1");
    comp2->setName("comp2");
    m->addComponent(comp1);
    m->addComponent(comp2);

    // u1 = u2: different spelling of meter/metre.
    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("testunit1");
    u1->addUnit("metre");
    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("testunit2");
    u2->addUnit("meter");

    v1->setUnits(u1);
    v2->setUnits(u2);
    m->addUnits(u1);
    m->addUnits(u2);

    // This one is now an issue.
    libcellml::Variable::addEquivalence(v1, v2);
    validator->validateModel(m);

    EXPECT_EQ_ISSUES(expectedIssues, validator);
}

TEST(Validator, unitEquivalenceStandardUnitsToBaseUnits)
{
    libcellml::ValidatorPtr validator = libcellml::Validator::create();
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr comp1 = libcellml::Component::create();
    libcellml::ComponentPtr comp2 = libcellml::Component::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    libcellml::UnitsPtr base = libcellml::Units::create();
    libcellml::UnitsPtr standard = libcellml::Units::create();

    const std::map<std::string, std::map<std::string, double>> standardToBaseUnitList = {
        {"ampere", {{"ampere", 1.0}}},
        {"becquerel", {{"second", -1.0}}},
        {"candela", {{"candela", 1.0}}},
        {"coulomb", {{"ampere", -1.0}, {"second", 1.0}}},
        {"dimensionless", {{"dimensionless", 1.0}}},
        {"farad", {{"ampere", 2.0}, {"kilogram", -1.0}, {"metre", -2.0}, {"second", -4.0}}},
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

    v1->setName("tomayto");
    v2->setName("tomahto");
    m->setName("callthewholethingoff");
    comp1->addVariable(v1);
    comp2->addVariable(v2);
    comp1->setName("comp1");
    comp2->setName("comp2");
    m->addComponent(comp1);
    m->addComponent(comp2);
    base->setName("base");
    standard->setName("standard");
    v1->setUnits(base);
    v2->setUnits(standard);
    m->addUnits(base);
    m->addUnits(standard);

    for (const auto &line : standardToBaseUnitList) {
        standard->removeAllUnits();
        base->removeAllUnits();
        standard->addUnit(line.first);
        for (const auto &baseUnits : line.second) {
            base->addUnit(baseUnits.first, 0, baseUnits.second, 1.0);
        }
        validator->validateModel(m);
        EXPECT_EQ(size_t(0), validator->issueCount());
    }
}

TEST(Validator, unitEquivalenceBasicDimensionlessUnits)
{
    libcellml::ValidatorPtr validator = libcellml::Validator::create();
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr comp1 = libcellml::Component::create();
    libcellml::ComponentPtr comp2 = libcellml::Component::create();

    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();

    v1->setName("tomayto");
    v1->setInterfaceType("public");
    v2->setName("tomahto");
    v2->setInterfaceType("public");

    m->setName("callthewholethingoff");
    comp1->addVariable(v1);
    comp2->addVariable(v2);
    comp1->setName("comp1");
    comp2->setName("comp2");
    m->addComponent(comp1);
    m->addComponent(comp2);

    // u1 = u2: testing that cancelled units become dimensionless.
    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("metrepermetre");
    u1->addUnit("metre");
    u1->addUnit("metre", -1.0);
    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("ratio");
    u2->addUnit("dimensionless");

    v1->setUnits(u1);
    v2->setUnits(u2);

    m->addUnits(u1);
    m->addUnits(u2);

    libcellml::Variable::addEquivalence(v1, v2);

    validator->validateModel(m);
    EXPECT_EQ(size_t(0), validator->issueCount());
}

TEST(Validator, unitEquivalenceDimensionlessUnits)
{
    libcellml::ValidatorPtr validator = libcellml::Validator::create();
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr comp1 = libcellml::Component::create();
    libcellml::ComponentPtr comp2 = libcellml::Component::create();
    libcellml::ComponentPtr comp3 = libcellml::Component::create();

    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    libcellml::VariablePtr v3 = libcellml::Variable::create();

    v1->setName("tomayto");
    v2->setName("tomahto");
    v3->setName("tomaat");

    m->setName("callthewholethingoff");
    comp1->addVariable(v1);
    comp2->addVariable(v2);
    comp3->addVariable(v3);
    comp1->setName("comp1");
    comp2->setName("comp2");
    comp3->setName("comp3");
    m->addComponent(comp1);
    m->addComponent(comp2);
    m->addComponent(comp3);

    // u1 = u2 = u3: testing that cancelled units become dimensionless and equivalent to radians, steradians, etc.
    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("testunit5");
    u1->addUnit("metre", -2.0);
    u1->addUnit("metre", 2.0);
    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("testunit6");
    u2->addUnit("dimensionless");
    libcellml::UnitsPtr u3 = libcellml::Units::create();
    u3->setName("testunit7");
    u3->addUnit("steradian");

    v1->setUnits(u1);
    v2->setUnits(u2);
    v3->setUnits(u3);

    m->addUnits(u1);
    m->addUnits(u2);
    m->addUnits(u3);

    libcellml::Variable::addEquivalence(v1, v2);
    libcellml::Variable::addEquivalence(v2, v3);

    m->fixVariableInterfaces();

    validator->validateModel(m);
    EXPECT_EQ(size_t(0), validator->issueCount());
}

TEST(Validator, unitEquivalenceMultiplierPrefix)
{
    libcellml::ValidatorPtr validator = libcellml::Validator::create();
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr comp1 = libcellml::Component::create();
    libcellml::ComponentPtr comp2 = libcellml::Component::create();
    libcellml::ComponentPtr comp3 = libcellml::Component::create();

    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    libcellml::VariablePtr v3 = libcellml::Variable::create();

    v1->setName("tomayto");
    v2->setName("tomahto");
    v3->setName("tomaat");

    m->setName("callthewholethingoff");
    comp1->setName("comp1");
    comp2->setName("comp2");
    comp3->setName("comp3");
    m->addComponent(comp1);
    m->addComponent(comp2);
    m->addComponent(comp3);

    // u1 = u2 = u3: testing multiplier or prefix don't affect base unit equivalence.
    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("testunit10");
    u1->addUnit("gram", 2, 1000.0);
    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("testunit11");
    u2->addUnit("kilogram", 2.0);
    libcellml::UnitsPtr u3 = libcellml::Units::create();
    u3->setName("testunit12");
    u3->addUnit("gram", "kilo", 2.0);

    v1->setUnits(u1);
    v2->setUnits(u2);
    v3->setUnits(u3);

    m->addUnits(u1);
    m->addUnits(u2);
    m->addUnits(u3);

    libcellml::Variable::addEquivalence(v1, v2);
    libcellml::Variable::addEquivalence(v2, v3);

    validator->validateModel(m);
    EXPECT_EQ(size_t(0), validator->issueCount());
}

TEST(Validator, unitEquivalenceComplicatedNestedUnits)
{
    const std::vector<std::string> expectedIssues = {
        "Variable 'pjs' in component 'yousay' has units of 'testunit13' and an equivalent variable 'pajamas' in component 'wesay' with non-matching units of 'testunit14'. The mismatch is: metre^1, multiplication factor of 10^3.",
    };

    libcellml::ValidatorPtr validator = libcellml::Validator::create();
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr comp1 = libcellml::Component::create();
    libcellml::ComponentPtr comp2 = libcellml::Component::create();
    libcellml::ComponentPtr comp3 = libcellml::Component::create();

    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v8 = libcellml::Variable::create();
    libcellml::VariablePtr v9 = libcellml::Variable::create();
    libcellml::VariablePtr v13 = libcellml::Variable::create();
    libcellml::VariablePtr v14 = libcellml::Variable::create();

    v1->setName("tomayto");
    v8->setName("neether");
    v9->setName("nyther");
    v13->setName("pjs");
    v14->setName("pajamas");

    comp1->setName("isay");
    comp2->setName("yousay");
    comp3->setName("wesay");

    m->setName("callthewholethingoff");

    comp1->addVariable(v1);
    comp2->addVariable(v8);
    comp3->addVariable(v9);
    comp2->addVariable(v13);
    comp3->addVariable(v14);

    m->addComponent(comp1);
    m->addComponent(comp2);
    m->addComponent(comp3);

    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("testunit1");
    u1->addUnit("metre");

    // u8 = u9: testing more complicated compound units, newton/(sievert.pascal) = second^2.radian^3.steradian^-4.
    libcellml::UnitsPtr u8 = libcellml::Units::create();
    u8->setName("testunit8");
    u8->addUnit("newton", 1.0);
    u8->addUnit("pascal", -1.0);
    u8->addUnit("sievert", -1.0);
    libcellml::UnitsPtr u9 = libcellml::Units::create();
    u9->setName("testunit9");
    u9->addUnit("second", 2.0);
    u9->addUnit("radian", -4.0);
    u9->addUnit("steradian", 2.0);

    // u13 != u14: testing that the mismatch is reported correctly.
    libcellml::UnitsPtr u13 = libcellml::Units::create();
    u13->setName("testunit13");
    u13->addUnit("testunit1", "kilo", 2.0);
    u13->addUnit("testunit8", 2.0);
    libcellml::UnitsPtr u14 = libcellml::Units::create();
    u14->setName("testunit14");
    u14->addUnit("testunit1", 1.0);
    u14->addUnit("testunit9", 2.0);

    v1->setUnits(u1);
    v8->setUnits(u8);
    v9->setUnits(u9);
    v13->setUnits(u13);
    v14->setUnits(u14);

    m->addUnits(u1);
    m->addUnits(u8);
    m->addUnits(u9);
    m->addUnits(u13);
    m->addUnits(u14);

    // This one is fine but complicated: newton/(sievert.pascal) = second^2.radian^3.steradian^-4.
    libcellml::Variable::addEquivalence(v8, v9);

    // Off by (metre)^1: testing nested unit equivalence.
    libcellml::Variable::addEquivalence(v13, v14);

    m->fixVariableInterfaces();

    validator->validateModel(m);

    EXPECT_EQ_ISSUES(expectedIssues, validator);
}

TEST(Validator, unitUserCreatedUnitsBananasAndApples)
{
    const std::vector<std::string> expectedIssues = {
        "Variable 'v1' in component 'c1' has units of 'bushell_of_apples' and an equivalent variable 'v2' in component 'c2' with non-matching units of 'bunch_of_bananas'. The mismatch is: apple^10, banana^-5.",
    };

    libcellml::ValidatorPtr validator = libcellml::Validator::create();
    libcellml::ModelPtr m = createModelTwoComponentsWithOneVariableEach("m", "c1", "c2", "v1", "v2");
    auto c1 = m->component(0);
    auto c2 = m->component(1);
    auto v1 = c1->variable(0);
    auto v2 = c2->variable(0);

    libcellml::UnitsPtr uApple = libcellml::Units::create();
    uApple->setName("apple");

    libcellml::UnitsPtr uBanana = libcellml::Units::create();
    uBanana->setName("banana");

    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("bushell_of_apples");
    u1->addUnit("apple", 10.0);

    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("bunch_of_bananas");
    u2->addUnit("banana", 5.0);

    v1->setUnits(u1);
    v2->setUnits(u2);

    m->addUnits(uApple);
    m->addUnits(uBanana);
    m->addUnits(u1);
    m->addUnits(u2);

    libcellml::Variable::addEquivalence(v1, v2); // Bushell of apples != bunch of bananas.

    validator->validateModel(m);

    EXPECT_EQ_ISSUES(expectedIssues, validator);
}

TEST(Validator, unitUserCreatedUnitsCells)
{
    libcellml::ValidatorPtr validator = libcellml::Validator::create();
    libcellml::ModelPtr m = createModelTwoComponentsWithOneVariableEach("m", "c1", "c2", "v1", "v2");
    auto c1 = m->component(0);
    auto c2 = m->component(1);
    auto v1 = c1->variable(0);
    auto v2 = c2->variable(0);

    libcellml::UnitsPtr uCells = libcellml::Units::create();
    uCells->setName("cells");

    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("bushell_of_cells");
    u1->addUnit("cells", 10.0);

    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("bunch_of_cells");
    u2->addUnit("cells", 10.0);

    v1->setUnits(u1);
    v2->setUnits(u2);

    m->addUnits(uCells);
    m->addUnits(u1);
    m->addUnits(u2);

    libcellml::Variable::addEquivalence(v1, v2); // Bushell of cells == bunch of cells.

    validator->validateModel(m);

    EXPECT_EQ(size_t(0), validator->issueCount());
}

TEST(Validator, unitMissingEquivalentUnits)
{
    const std::vector<std::string> expectedIssues = {
        "Variable 'v1' in component 'c1' does not have any units specified.",
    };

    libcellml::ValidatorPtr validator = libcellml::Validator::create();
    libcellml::ModelPtr m = createModelTwoComponentsWithOneVariableEach("m", "c1", "c2", "v1", "v2");
    auto c1 = m->component(0);
    auto c2 = m->component(1);

    auto v1 = c1->variable(0);
    auto v2 = c2->variable(0);

    v2->setUnits("second");

    libcellml::Variable::addEquivalence(v1, v2);
    validator->validateModel(m);
    EXPECT_EQ_ISSUES(expectedIssues, validator);
}
TEST(Validator, unitTwoMissingEquivalentUnits)
{
    const std::vector<std::string> expectedIssues = {
        "Variable 'v1' in component 'c1' does not have any units specified.",
        "Variable 'v2' in component 'c2' does not have any units specified.",
    };

    libcellml::ValidatorPtr validator = libcellml::Validator::create();
    libcellml::ModelPtr m = createModelTwoComponentsWithOneVariableEach("m", "c1", "c2", "v1", "v2");
    auto c1 = m->component(0);
    auto c2 = m->component(1);

    auto v1 = c1->variable(0);
    auto v2 = c2->variable(0);

    libcellml::Variable::addEquivalence(v1, v2);
    validator->validateModel(m);
    EXPECT_EQ_ISSUES(expectedIssues, validator);
}

TEST(Validator, mismatchedBaseUnitsEquivalentVariables)
{
    const std::vector<std::string> expectedIssues = {
        "Variable 'v1' in component 'c1' has units of 'metre' and an equivalent variable 'v3' in component 'c3' with non-matching units of 'second'. The mismatch is: metre^1, second^-1.",
    };

    libcellml::ValidatorPtr validator = libcellml::Validator::create();
    libcellml::ModelPtr m = createModelTwoComponentsWithOneVariableEach("m", "c1", "c2", "v1", "v2");
    auto c1 = m->component(0);
    auto c2 = m->component(1);

    auto v1 = c1->variable(0);
    auto v2 = c2->variable(0);
    v1->setUnits("metre");
    v2->setUnits("metre");

    m->addComponent(libcellml::Component::create("c3"));
    auto v3 = libcellml::Variable::create("v3");
    m->component("c3")->addVariable(v3);
    v3->setUnits("second");
    v3->setInterfaceType("public");

    libcellml::Variable::addEquivalence(v1, v2);
    libcellml::Variable::addEquivalence(v3, v1);

    validator->validateModel(m);
    EXPECT_EQ_ISSUES(expectedIssues, validator);
}

TEST(Validator, unitStandardUnitsWhichAreBaseUnits)
{
    const std::vector<std::string> expectedIssues = {
        "Variable 'v1' in component 'c1' has units of 'metre' and an equivalent variable 'v2' in component 'c2' with non-matching units of 'second'. The mismatch is: metre^1, second^-1.",
    };

    libcellml::ValidatorPtr validator = libcellml::Validator::create();
    libcellml::ModelPtr m = createModelTwoComponentsWithOneVariableEach("m", "c1", "c2", "v1", "v2");
    auto c1 = m->component(0);
    auto c2 = m->component(1);
    auto v1 = c1->variable(0);
    auto v2 = c2->variable(0);

    v1->setUnits("metre");
    v2->setUnits("second");

    libcellml::Variable::addEquivalence(v1, v2); // metre != second.

    validator->validateModel(m);

    EXPECT_EQ_ISSUES(expectedIssues, validator);
}

TEST(Validator, unitStandardUnitsWhichAreNotBaseUnits)
{
    const std::vector<std::string> expectedIssues = {
        "Variable 'v1' in component 'c1' has units of 'litre' and an equivalent variable 'v2' in component 'c2' with non-matching units of 'gram'. The mismatch is: kilogram^-1, metre^3.",
    };

    libcellml::ValidatorPtr validator = libcellml::Validator::create();
    libcellml::ModelPtr m = createModelTwoComponentsWithOneVariableEach("m", "c1", "c2", "v1", "v2");
    auto c1 = m->component(0);
    auto c2 = m->component(1);
    auto v1 = c1->variable(0);
    auto v2 = c2->variable(0);

    v1->setUnits("litre");
    v2->setUnits("gram");

    libcellml::Variable::addEquivalence(v1, v2); // litre != gram.

    validator->validateModel(m);

    EXPECT_EQ_ISSUES(expectedIssues, validator);
}

TEST(Validator, unitNonStandardUnitsWhichAreBaseUnits)
{
    const std::vector<std::string> expectedIssues = {
        "Variable 'v1' in component 'c1' has a units reference 'orange' which is neither standard nor defined in the parent model.",
        "Variable 'v2' in component 'c2' has a units reference 'apple' which is neither standard nor defined in the parent model.",
    };

    libcellml::ValidatorPtr validator = libcellml::Validator::create();
    libcellml::ModelPtr m = createModelTwoComponentsWithOneVariableEach("m", "c1", "c2", "v1", "v2");
    auto c1 = m->component(0);
    auto c2 = m->component(1);
    auto v1 = c1->variable(0);
    auto v2 = c2->variable(0);

    v1->setUnits("orange");
    v2->setUnits("apple");

    libcellml::Variable::addEquivalence(v1, v2); // orange != apple.

    validator->validateModel(m);

    EXPECT_EQ_ISSUES(expectedIssues, validator);
}

TEST(Validator, multipleEquivalentUnitStandardUnitsWhichAreNotBaseUnits)
{
    const std::vector<std::string> expectedIssues = {
        "Variable 'v1' in component 'c1' has units of 'litre' and an equivalent variable 'v2' in component 'c2' with non-matching units of 'gram'. The mismatch is: kilogram^-1, metre^3.",
        "Variable 'v1' in component 'c1' has units of 'litre' and an equivalent variable 'v3' in component 'c1' with non-matching units of 'ampere'. The mismatch is: ampere^-1, metre^3, multiplication factor of 10^-3.",
    };

    libcellml::ValidatorPtr validator = libcellml::Validator::create();
    libcellml::ModelPtr m = createModelTwoComponentsWithOneVariableEach("m", "c1", "c2", "v1", "v2");
    auto c1 = m->component(0);
    auto c2 = m->component(1);
    auto v1 = c1->variable(0);
    auto v2 = c2->variable(0);
    auto v3 = libcellml::Variable::create("v3");
    v3->setInterfaceType("public");
    c1->addVariable(v3);

    v1->setUnits("litre");
    v2->setUnits("gram");
    v3->setUnits("ampere");

    libcellml::Variable::addEquivalence(v1, v2); // litre != gram.
    libcellml::Variable::addEquivalence(v1, v3); // litre != ampere.

    validator->validateModel(m);

    EXPECT_EQ_ISSUES(expectedIssues, validator);
}

TEST(Validator, unitSimpleCycle)
{
    // Testing that indirect dependence is caught in the unit cycles. The network is:
    //
    //      grandfather(u1) <- father(u2) <- child (u3) <-+
    //           |                                        |
    //           +----------------------------------------+

    const std::vector<std::string> expectedIssues = {
        "Cyclic units exist: 'grandfather' -> 'child' -> 'father' -> 'grandfather'.",
    };

    libcellml::ValidatorPtr v = libcellml::Validator::create();
    libcellml::ModelPtr m = libcellml::Model::create();

    libcellml::UnitsPtr u1 = libcellml::Units::create();
    libcellml::UnitsPtr u2 = libcellml::Units::create();
    libcellml::UnitsPtr u3 = libcellml::Units::create();

    m->setName("model");

    m->addUnits(u1);
    m->addUnits(u2);
    m->addUnits(u3);

    u1->setName("grandfather"); // Base unit.

    u2->setName("father"); // First generation.
    u2->addUnit("grandfather");

    u3->setName("child"); // Second generation.
    u3->addUnit("father");

    // Network valid at this stage.
    v->validateModel(m);
    EXPECT_EQ(size_t(0), v->issueCount());

    // Time loop Grandfather paradox created! u1 no longer a base variable: u1 -> u3 -> u2 -> u1.
    u1->addUnit("child");
    v->validateModel(m);

    EXPECT_EQ_ISSUES(expectedIssues, v);
    auto issue = v->issue(0);
    EXPECT_EQ("grandfather", issue->item()->units()->name());
}

TEST(Validator, unitSimpleCycleWithAdditionalIssue)
{
    const std::vector<std::string> expectedIssues = {
        "Model '' does not have a valid name attribute. CellML identifiers must contain one or more basic Latin alphabetic characters.",
        "Cyclic units exist: 'grandfather' -> 'child' -> 'father' -> 'grandfather'.",
    };

    libcellml::ValidatorPtr v = libcellml::Validator::create();
    libcellml::ModelPtr m = libcellml::Model::create();

    libcellml::UnitsPtr u1 = libcellml::Units::create();
    libcellml::UnitsPtr u2 = libcellml::Units::create();
    libcellml::UnitsPtr u3 = libcellml::Units::create();

    m->setName("model");

    m->addUnits(u1);
    m->addUnits(u2);
    m->addUnits(u3);

    u1->setName("grandfather"); // Base unit.

    u2->setName("father"); // First generation.
    u2->addUnit("grandfather");

    u3->setName("child"); // Second generation.
    u3->addUnit("father");

    // Network valid at this stage.
    v->validateModel(m);
    EXPECT_EQ(size_t(0), v->issueCount());

    // Time loop Grandfather paradox created! u1 no longer a base variable: u1 -> u3 -> u2 -> u1.
    u1->addUnit("child");
    // Add additional validation error.
    m->removeName();

    v->validateModel(m);

    EXPECT_EQ_ISSUES(expectedIssues, v);
    auto issue = v->issue(1);
    EXPECT_EQ("grandfather", issue->item()->units()->name());
}

libcellml::ModelPtr unitComplexCycle(bool order)
{
    // Simple testing for the directional dependency of units. The first network is:
    //
    //                            <- brotherFromAnotherMother (u4)
    //                 <- father (u2)  <-+
    //     grandfather (u1)              | <- childOfIncest_ButThatsOKApparently (u5)
    //                 <- mother (u3)  <-+
    //                            <- sisterFromAnotherFather (u6)
    //
    // There is an _undirected_ loop between u1-u2-u3-u5 but the directionality of the
    // dependencies here means the network is still valid. Keeping this here to test that
    // the directionality does indeed protect it from forming a cycle.

    libcellml::ValidatorPtr v = libcellml::Validator::create();
    libcellml::ModelPtr m = libcellml::Model::create();

    libcellml::UnitsPtr u1 = libcellml::Units::create();
    libcellml::UnitsPtr u2 = libcellml::Units::create();
    libcellml::UnitsPtr u3 = libcellml::Units::create();
    libcellml::UnitsPtr u4 = libcellml::Units::create();
    libcellml::UnitsPtr u5 = libcellml::Units::create();
    libcellml::UnitsPtr u6 = libcellml::Units::create();

    m->setName("model");

    if (order) {
        m->addUnits(u4);
        m->addUnits(u1);
        m->addUnits(u2);
        m->addUnits(u3);
    } else {
        m->addUnits(u1);
        m->addUnits(u2);
        m->addUnits(u3);
        m->addUnits(u4);
    }
    m->addUnits(u5);
    m->addUnits(u6);

    u1->setName("grandfather"); // Base unit.

    u2->setName("father"); // First generation.
    u2->addUnit("grandfather");

    u3->setName("mother"); // First generation.
    u3->addUnit("grandfather");

    u4->setName("brotherFromAnotherMother"); // Second generation.
    u4->addUnit("father");

    // Second generation depending on both first gen children, still valid, no loops because of directionality.
    u5->setName("childOfIncest_ButThatsOKApparently");
    u5->addUnit("mother");
    u5->addUnit("father");

    u6->setName("sisterFromAnotherFather"); // Second generation.
    u6->addUnit("mother");

    v->validateModel(m);
    EXPECT_EQ(size_t(0), v->issueCount());

    // As soon as a dependency of the grandfather on the brotherFromAnotherMother is added, then a
    // _directed_ loop (u1->u2->u4->u1) is created and the network is no longer valid:
    //
    //     +----------------------------------------------------------+
    //     |                                                          |
    //     |                       <- brotherFromAnotherMother (u4) <-+
    //     |            <- father (u2)  <-+
    //     +- grandfather (u1)            | <- childOfIncest (u5)
    //                  <- mother (u3)  <-+
    //                             <- sisterFromAnotherFather (u6)

    // Time loop Grandfather paradox created! u1 no longer a base variable: u1 -> u4 -> u2 -> u1.
    u1->addUnit("brotherFromAnotherMother");

    return m;
}

TEST(Validator, unitComplexCycle)
{
    const std::vector<std::string> expectedIssues = {
        "Cyclic units exist: 'grandfather' -> 'brotherFromAnotherMother' -> 'father' -> 'grandfather'.",
    };

    libcellml::ValidatorPtr v = libcellml::Validator::create();
    libcellml::ModelPtr m = unitComplexCycle(false);

    v->validateModel(m);

    EXPECT_EQ_ISSUES(expectedIssues, v);
}

TEST(Validator, unitComplexCycleChangeOrderOfUnits)
{
    // Same test as unitComplexCycle except the order of the units
    // added to the model is different.  This changes the order
    // that the loop is detected in.
    const std::vector<std::string> expectedIssues = {
        "Cyclic units exist: 'brotherFromAnotherMother' -> 'father' -> 'grandfather' -> 'brotherFromAnotherMother'.",
    };

    libcellml::ValidatorPtr v = libcellml::Validator::create();
    libcellml::ModelPtr m = unitComplexCycle(true);

    v->validateModel(m);

    EXPECT_EQ_ISSUES(expectedIssues, v);
}

TEST(Validator, duplicatedCellMLUnitsOnCiElement)
{
    const std::string math =
        "<math xmlns:cellml=\"http://www.cellml.org/cellml/2.0#\" xmlns=\"http://www.w3.org/1998/Math/MathML\">\n"
        "  <ci cellml:units=\"dimensionless\" cellml:units=\"second\">B</ci>\n"
        "</math>\n";

    const std::vector<std::string> expectedIssues = {
        "LibXml2 error: Attribute cellml:units redefined.",
        "Could not get a valid XML root node from the math on component 'componentName'.",
    };

    libcellml::ValidatorPtr v = libcellml::Validator::create();
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c = libcellml::Component::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();

    m->setName("modelName");
    c->setName("componentName");
    v1->setName("B");
    v1->setUnits("second");
    c->addVariable(v1);

    c->setMath(math);
    m->addComponent(c);

    v->validateModel(m);

    EXPECT_EQ_ISSUES(expectedIssues, v);
}

TEST(Validator, multipleDefinitionsOfCellMLNamespace)
{
    libcellml::ParserPtr parser = libcellml::Parser::create();
    libcellml::ModelPtr model = parser->parseModel(fileContents("multiplecellmlnamespaces.cellml"));

    libcellml::ValidatorPtr validator = libcellml::Validator::create();
    validator->validateModel(model);

    EXPECT_EQ(size_t(0), validator->issueCount());
}

TEST(Validator, validateModelWithoutAndWithMath)
{
    libcellml::ModelPtr model = libcellml::Model::create();
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    libcellml::ComponentPtr c2 = libcellml::Component::create();
    libcellml::ComponentPtr c3 = libcellml::Component::create();

    libcellml::ValidatorPtr validator = libcellml::Validator::create();

    model->setName("model");
    c1->setName("c1");
    c2->setName("c2");
    c3->setName("c3");

    model->addComponent(c1);
    model->addComponent(c2);
    c2->addComponent(c3);

    libcellml::VariablePtr v = libcellml::Variable::create();
    v->setName("v");
    v->setUnits("dimensionless");
    c3->addVariable(v);

    validator->validateModel(model);
    EXPECT_EQ(size_t(0), validator->issueCount());

    const std::string math =
        "<math xmlns=\"http://www.w3.org/1998/Math/MathML\" xmlns:cellml=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <apply>\n"
        "    <eq/>\n"
        "    <ci>v</ci>\n"
        "    <cn cellml:units=\"dimensionless\">1</cn>\n"
        "  </apply>\n"
        "</math>\n";

    c3->setMath(math);

    validator->validateModel(model);
    EXPECT_EQ(size_t(0), validator->issueCount());
}

TEST(Validator, unfoundUnitsInEncapsulatedComponents)
{
    const std::vector<std::string> expectedIssues = {
        "Variable 'v' in component 'c3' has a units reference 'non_existent_deep' which is neither standard nor defined in the parent model.",
        "Variable 'v' in component 'c2' has a units reference 'non_existent_shallow' which is neither standard nor defined in the parent model.",
    };

    libcellml::ModelPtr model = libcellml::Model::create();
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    libcellml::ComponentPtr c2 = libcellml::Component::create();
    libcellml::ComponentPtr c3 = libcellml::Component::create();

    libcellml::ValidatorPtr v = libcellml::Validator::create();

    model->setName("model");
    c1->setName("c1");
    c2->setName("c2");
    c3->setName("c3");

    model->addComponent(c1);
    model->addComponent(c2);
    c2->addComponent(c3);

    c1->addVariable(createVariableWithUnits("v", "dimensionless"));
    c2->addVariable(createVariableWithUnits("v", "non_existent_shallow"));
    c3->addVariable(createVariableWithUnits("v", "non_existent_deep"));

    EXPECT_TRUE(model->hasUnlinkedUnits());

    model->linkUnits();
    v->validateModel(model);

    EXPECT_EQ_ISSUES(expectedIssues, v);
}

TEST(Validator, variableEquivalenceValidNetwork)
{
    libcellml::ModelPtr model = libcellml::Model::create();
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    libcellml::ComponentPtr c2 = libcellml::Component::create();
    libcellml::ComponentPtr c3 = libcellml::Component::create();

    libcellml::ValidatorPtr validator = libcellml::Validator::create();

    model->setName("model");
    c1->setName("c1");
    c2->setName("c2");
    c3->setName("c3");

    model->addComponent(c1);
    model->addComponent(c2);
    c2->addComponent(c3);

    libcellml::VariablePtr v1 = libcellml::Variable::create();
    v1->setName("v1");
    v1->setUnits("dimensionless");

    libcellml::VariablePtr v2 = libcellml::Variable::create();
    v2->setName("v2");
    v2->setUnits("dimensionless");

    libcellml::VariablePtr v3 = libcellml::Variable::create();
    v3->setName("v3");
    v3->setUnits("dimensionless");

    c1->addVariable(v1);
    c2->addVariable(v2);
    c3->addVariable(v3);

    libcellml::Variable::addEquivalence(v1, v2);
    libcellml::Variable::addEquivalence(v2, v3);

    v1->setInterfaceType("public");
    v2->setInterfaceType("public_and_private");
    v3->setInterfaceType("public");

    validator->validateModel(model);
    EXPECT_EQ(size_t(0), validator->errorCount());
}

TEST(Validator, variableEquivalenceUnreachable)
{
    const std::vector<std::string> expectedIssues {
        "The equivalence between 'v1' in component 'c1'  and 'v3' in component 'c3' is invalid. Component 'c1' and 'c3' are neither siblings nor in a parent/child relationship.",
    };

    libcellml::ModelPtr model = libcellml::Model::create();
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    libcellml::ComponentPtr c2 = libcellml::Component::create();
    libcellml::ComponentPtr c3 = libcellml::Component::create();

    libcellml::ValidatorPtr validator = libcellml::Validator::create();

    model->setName("model");
    c1->setName("c1");
    c2->setName("c2");
    c3->setName("c3");

    model->addComponent(c1);
    model->addComponent(c2);
    c2->addComponent(c3);

    libcellml::VariablePtr v1 = libcellml::Variable::create();
    v1->setName("v1");
    v1->setUnits("dimensionless");

    libcellml::VariablePtr v2 = libcellml::Variable::create();
    v2->setName("v2");
    v2->setUnits("dimensionless");

    libcellml::VariablePtr v3 = libcellml::Variable::create();
    v3->setName("v3");
    v3->setUnits("dimensionless");

    c1->addVariable(v1);
    c2->addVariable(v2);
    c3->addVariable(v3);

    // Invalid equivalence, too far away.
    libcellml::Variable::addEquivalence(v1, v3);
    validator->validateModel(model);

    EXPECT_EQ_ISSUES(expectedIssues, validator);
}

TEST(Validator, multipleVariableEquivalencesUnreachable)
{
    const std::vector<std::string> expectedIssues {
        "The equivalence between 'v1' in component 'c1'  and 'v3' in component 'c3' is invalid. Component 'c1' and 'c3' are neither siblings nor in a parent/child relationship.",
        "The equivalence between 'v1' in component 'c1'  and 'v4' in component 'c4' is invalid. Component 'c1' and 'c4' are neither siblings nor in a parent/child relationship.",
    };

    libcellml::ModelPtr model = libcellml::Model::create();
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    libcellml::ComponentPtr c2 = libcellml::Component::create();
    libcellml::ComponentPtr c3 = libcellml::Component::create();
    libcellml::ComponentPtr c4 = libcellml::Component::create();

    libcellml::ValidatorPtr validator = libcellml::Validator::create();

    model->setName("model");
    c1->setName("c1");
    c2->setName("c2");
    c3->setName("c3");
    c4->setName("c4");

    model->addComponent(c1);
    model->addComponent(c2);
    c2->addComponent(c3);
    c2->addComponent(c4);

    libcellml::VariablePtr v1 = libcellml::Variable::create();
    v1->setName("v1");
    v1->setUnits("dimensionless");

    libcellml::VariablePtr v4 = libcellml::Variable::create();
    v4->setName("v4");
    v4->setUnits("dimensionless");

    libcellml::VariablePtr v3 = libcellml::Variable::create();
    v3->setName("v3");
    v3->setUnits("dimensionless");

    c1->addVariable(v1);
    c4->addVariable(v4);
    c3->addVariable(v3);

    // Invalid equivalence, too far away.
    libcellml::Variable::addEquivalence(v1, v3);
    libcellml::Variable::addEquivalence(v1, v4);
    validator->validateModel(model);

    EXPECT_EQ_ISSUES(expectedIssues, validator);
}

TEST(Validator, variableEquivalenceUnreachableAndReachableTogether)
{
    const std::vector<std::string> expectedIssues = {
        "Variable 'v1' in component 'c1' has an interface type set to 'public' which is not the correct interface type for this variable. The interface type required is 'private'.",
        "The equivalence between 'v2' in component 'c2'  and 'v4' in component 'c4' is invalid. Component 'c2' and 'c4' are neither siblings nor in a parent/child relationship.",
    };

    libcellml::ModelPtr model = libcellml::Model::create();
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    libcellml::ComponentPtr c2 = libcellml::Component::create();
    libcellml::ComponentPtr c3 = libcellml::Component::create();
    libcellml::ComponentPtr c4 = libcellml::Component::create();
    libcellml::ComponentPtr c5 = libcellml::Component::create();

    libcellml::ValidatorPtr validator = libcellml::Validator::create();

    model->setName("model");
    c1->setName("c1");
    c2->setName("c2");
    c3->setName("c3");
    c4->setName("c4");
    c5->setName("c5");

    model->addComponent(c1);
    c1->addComponent(c2);
    c1->addComponent(c3);
    c2->addComponent(c5);
    c3->addComponent(c4);

    libcellml::VariablePtr v1 = libcellml::Variable::create();
    v1->setName("v1");
    v1->setUnits("dimensionless");
    v1->setInterfaceType("public");

    libcellml::VariablePtr v2 = libcellml::Variable::create();
    v2->setName("v2");
    v2->setUnits("dimensionless");
    v2->setInterfaceType("public_and_private");

    libcellml::VariablePtr v3 = libcellml::Variable::create();
    v3->setName("v3");
    v3->setUnits("dimensionless");
    v3->setInterfaceType("public");

    libcellml::VariablePtr v4 = libcellml::Variable::create();
    v4->setName("v4");
    v4->setUnits("dimensionless");

    libcellml::VariablePtr v5 = libcellml::Variable::create();
    v5->setName("v5");
    v5->setUnits("dimensionless");
    v5->setInterfaceType("public");

    c1->addVariable(v1);
    c2->addVariable(v2);
    c3->addVariable(v3);
    c4->addVariable(v4);
    c5->addVariable(v5);

    libcellml::Variable::addEquivalence(v2, v4); // invalid equivalence, too far away
    libcellml::Variable::addEquivalence(v1, v2);
    libcellml::Variable::addEquivalence(v2, v3);
    libcellml::Variable::addEquivalence(v2, v5);

    validator->validateModel(model);

    EXPECT_EQ_ISSUES(expectedIssues, validator);
}

TEST(Validator, variableInterfaceShouldBePublic)
{
    const std::vector<std::string> expectedIssues = {
        "Variable 'v1' in component 'c1' has an interface type set to 'none' which is not the correct interface type for this variable. The interface type required is 'public'.",
        "Variable 'v3' in component 'c1' has no interface type set. The interface type required is 'public'.",
        "Variable 'v2' in component 'c2' has an interface type set to 'private' which is not the correct interface type for this variable. The interface type required is 'public'.",
        "Variable 'v4' in component 'c2' has an interface type set to 'private' which is not the correct interface type for this variable. The interface type required is 'public'.",
    };

    libcellml::ModelPtr model = libcellml::Model::create();
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    libcellml::ComponentPtr c2 = libcellml::Component::create();

    libcellml::ValidatorPtr validator = libcellml::Validator::create();

    model->setName("model");
    c1->setName("c1");
    c2->setName("c2");

    model->addComponent(c1);
    model->addComponent(c2);

    libcellml::VariablePtr v1 = libcellml::Variable::create();
    v1->setName("v1");
    v1->setUnits("dimensionless");

    libcellml::VariablePtr v2 = libcellml::Variable::create();
    v2->setName("v2");
    v2->setUnits("dimensionless");

    libcellml::VariablePtr v3 = libcellml::Variable::create();
    v3->setName("v3");
    v3->setUnits("dimensionless");

    libcellml::VariablePtr v4 = libcellml::Variable::create();
    v4->setName("v4");
    v4->setUnits("dimensionless");

    c1->addVariable(v1);
    c1->addVariable(v3);
    c2->addVariable(v2);
    c2->addVariable(v4);

    libcellml::Variable::addEquivalence(v1, v2);
    libcellml::Variable::addEquivalence(v3, v4);

    v1->setInterfaceType("none");
    v2->setInterfaceType(libcellml::Variable::InterfaceType::PRIVATE);
    v4->setInterfaceType(libcellml::Variable::InterfaceType::PRIVATE);

    validator->validateModel(model);

    EXPECT_EQ_ISSUES(expectedIssues, validator);
}

TEST(Validator, variableInterfaceShouldBePrivate)
{
    const std::vector<std::string> expectedIssues = {
        "Variable 'v1' in component 'c1' has an interface type set to 'public' which is not the correct interface type for this variable. The interface type required is 'private'.",
    };

    libcellml::ModelPtr model = libcellml::Model::create();
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    libcellml::ComponentPtr c2 = libcellml::Component::create();

    libcellml::ValidatorPtr validator = libcellml::Validator::create();

    model->setName("model");
    c1->setName("c1");
    c2->setName("c2");

    model->addComponent(c1);
    c1->addComponent(c2);

    libcellml::VariablePtr v1 = libcellml::Variable::create();
    v1->setName("v1");
    v1->setUnits("dimensionless");

    libcellml::VariablePtr v2 = libcellml::Variable::create();
    v2->setName("v2");
    v2->setUnits("dimensionless");

    c1->addVariable(v1);
    c2->addVariable(v2);

    libcellml::Variable::addEquivalence(v1, v2);

    v1->setInterfaceType(libcellml::Variable::InterfaceType::PUBLIC);
    v2->setInterfaceType(libcellml::Variable::InterfaceType::PUBLIC);

    validator->validateModel(model);

    EXPECT_EQ_ISSUES(expectedIssues, validator);
}

TEST(Validator, variableInterfaceCouldBePrivateButSetToPublicAndPrivate)
{
    libcellml::ModelPtr model = libcellml::Model::create();
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    libcellml::ComponentPtr c2 = libcellml::Component::create();

    libcellml::ValidatorPtr validator = libcellml::Validator::create();

    model->setName("model");
    c1->setName("c1");
    c2->setName("c2");

    model->addComponent(c1);
    c1->addComponent(c2);

    libcellml::VariablePtr v1 = libcellml::Variable::create();
    v1->setName("v1");
    v1->setUnits("dimensionless");

    libcellml::VariablePtr v2 = libcellml::Variable::create();
    v2->setName("v2");
    v2->setUnits("dimensionless");

    c1->addVariable(v1);
    c2->addVariable(v2);

    libcellml::Variable::addEquivalence(v1, v2);

    v1->setInterfaceType(libcellml::Variable::InterfaceType::PUBLIC_AND_PRIVATE);
    v2->setInterfaceType(libcellml::Variable::InterfaceType::PUBLIC_AND_PRIVATE);

    validator->validateModel(model);

    EXPECT_EQ(size_t(0), validator->issueCount());
}

TEST(Validator, variableInterfaceShouldBePublicAndPrivate)
{
    const std::vector<std::string> expectedIssues = {
        "Variable 'v2' in component 'c2' has an interface type set to 'private' which is not the correct interface type for this variable. The interface type required is 'public_and_private'.",
    };

    libcellml::ModelPtr model = libcellml::Model::create();
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    libcellml::ComponentPtr c2 = libcellml::Component::create();
    libcellml::ComponentPtr c3 = libcellml::Component::create();

    libcellml::ValidatorPtr validator = libcellml::Validator::create();

    model->setName("model");
    c1->setName("c1");
    c2->setName("c2");
    c3->setName("c3");

    model->addComponent(c1);
    c1->addComponent(c2);
    c2->addComponent(c3);

    libcellml::VariablePtr v1 = libcellml::Variable::create();
    v1->setName("v1");
    v1->setUnits("dimensionless");

    libcellml::VariablePtr v2 = libcellml::Variable::create();
    v2->setName("v2");
    v2->setUnits("dimensionless");

    libcellml::VariablePtr v3 = libcellml::Variable::create();
    v3->setName("v3");
    v3->setUnits("dimensionless");

    c1->addVariable(v1);
    c2->addVariable(v2);
    c3->addVariable(v3);

    libcellml::Variable::addEquivalence(v1, v2);
    libcellml::Variable::addEquivalence(v2, v3);

    model->fixVariableInterfaces();

    v2->setInterfaceType(libcellml::Variable::InterfaceType::PRIVATE);

    validator->validateModel(model);

    EXPECT_EQ_ISSUES(expectedIssues, validator);
}

TEST(Validator, importedUnitsFoundByValidator)
{
    auto model = libcellml::Model::create("model");
    auto component = libcellml::Component::create("component");
    auto importSource = libcellml::ImportSource::create();
    importSource->setUrl("somewhere.cellml");
    model->addComponent(component);

    auto mV = libcellml::Units::create("mV");
    mV->setImportReference("mV");
    mV->setImportSource(importSource);
    model->addUnits(mV);

    auto v = libcellml::Variable::create("variable");
    v->setUnits(mV);
    component->addVariable(v);

    EXPECT_EQ(model, mV->parent());
    EXPECT_TRUE(model->hasUnits("mV"));

    auto validator = libcellml::Validator::create();
    validator->validateModel(model);
    EXPECT_EQ(size_t(0), validator->errorCount());
}

TEST(Validator, duplicateIdSimple)
{
    std::vector<std::string> e = {"Duplicated identifier attribute 'id' has been found in:\n"
                                  " - model 'model';\n"
                                  " - component 'c1' in model 'model';\n"
                                  " - variable 'v1' in component 'c1';\n"
                                  " - component 'c2' in component 'c1'; and\n"
                                  " - variable 'v2' in component 'c2'.\n"};

    auto model = createModelTwoComponentsWithOneVariableEach("model", "c1", "c2", "v1", "v2");
    model->setId("id");
    model->component(0)->setId("id");
    model->component(1)->setId("id");
    model->component(0)->variable(0)->setId("id");
    model->component(1)->variable(0)->setId("id");
    model->component(0)->variable(0)->setUnits("dimensionless");
    model->component(1)->variable(0)->setUnits("dimensionless");
    model->component(0)->addComponent(model->component(1));

    auto validator = libcellml::Validator::create();
    validator->validateModel(model);
    EXPECT_EQ_ISSUES(e, validator);
}

TEST(Validator, duplicateIdAll)
{
    std::vector<std::string> expectedIssues;
    expectedIssues.emplace_back("W3C MathML DTD error: ID id4 already defined.");
    expectedIssues.emplace_back("W3C MathML DTD error: ID id1 already defined.");
    expectedIssues.emplace_back(
        "Duplicated identifier attribute 'id1' has been found in:\n"
        " - model 'everything';\n"
        " - units 'units2' in model 'everything';\n"
        " - encapsulation in model 'everything';\n"
        " - variable 'variable2' in component 'component2';\n"
        " - MathML apply element in test_value in reset 0 in component 'component2';\n"
        " - MathML math element in reset_value in reset 0 in component 'component2';\n"
        " - MathML cn element in reset_value in reset 0 in component 'component2'; and\n"
        " - MathML ci element 'variable4' in math in component 'component3'.\n");
    expectedIssues.emplace_back(
        "Duplicated identifier attribute 'id2' has been found in:\n"
        " - unit in units 'units2' in model 'everything';\n"
        " - import source for component 'component1';\n"
        " - variable equivalence between variable 'variable1' in component 'component2' and variable 'variable4' in component 'component3';\n"
        " - reset at index 0 in component 'component2';\n"
        " - MathML eq element in test_value in reset 0 in component 'component2';\n"
        " - MathML ci element 'variable1' in reset_value in reset 0 in component 'component2';\n"
        " - encapsulation component_ref to component 'component2';\n"
        " - component 'component3' in component 'component2'; and\n"
        " - MathML eq element in math in component 'component3'.\n");
    expectedIssues.emplace_back(
        "Duplicated identifier attribute 'id3' has been found in:\n"
        " - units 'units3' in model 'everything';\n"
        " - imported component 'component1' in model 'everything';\n"
        " - test_value in reset at index 0 in component 'component2';\n"
        " - MathML ci element 'variable1' in test_value in reset 0 in component 'component2';\n"
        " - MathML eq element in reset_value in reset 0 in component 'component2';\n"
        " - variable 'variable2' in component 'component3';\n"
        " - MathML apply element in math in component 'component3'; and\n"
        " - encapsulation component_ref to component 'component3'.\n");
    expectedIssues.emplace_back(
        "Duplicated identifier attribute 'id4' has been found in:\n"
        " - import source for units 'units1';\n"
        " - component 'component2' in model 'everything';\n"
        " - connection between components 'component2' and 'component3' because of variable equivalence between variables 'variable1' and 'variable2';\n"
        " - MathML math element in test_value in reset 0 in component 'component2';\n"
        " - MathML cn element in test_value in reset 0 in component 'component2';\n"
        " - MathML apply element in reset_value in reset 0 in component 'component2'; and\n"
        " - MathML math element in math in component 'component3'.\n");
    expectedIssues.emplace_back(
        "Duplicated identifier attribute 'id5' has been found in:\n"
        " - imported units 'units1' in model 'everything';\n"
        " - variable 'variable1' in component 'component2';\n"
        " - variable equivalence between variable 'variable1' in component 'component2' and variable 'variable2' in component 'component3';\n"
        " - reset_value in reset at index 0 in component 'component2';\n"
        " - variable 'variable4' in component 'component3'; and\n"
        " - MathML cn element in math in component 'component3'.\n");

    const std::string in =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\"  name=\"everything\" id=\"id1\">\n"
        "  <import xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"some-other-model.xml\" id=\"id2\">\n"
        "    <component component_ref=\"a_component_in_that_model\" name=\"component1\" id=\"id3\"/>\n"
        "  </import>\n"
        "  <import xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"some-other-model-2.xml\" id=\"id17\">\n"
        "    <units units_ref=\"a_units_in_that_model\" name=\"units7\" id=\"id6\"/>\n"
        "  </import>\n"
        "  <import xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"some-other-model.xml\" id=\"id4\">\n"
        "    <units units_ref=\"a_units_in_that_model\" name=\"units1\" id=\"id5\"/>\n"
        "  </import>\n"
        "  <units name=\"units2\" id=\"id1\">\n"
        "    <unit units=\"second\" id=\"id2\"/>\n"
        "  </units>\n"
        "  <units name=\"units3\" id=\"id3\"/>\n"
        "  <units name=\"blob\"/>\n"
        "  <component name=\"component2\" id=\"id4\">\n"
        "    <variable name=\"variable1\" units=\"units2\" interface=\"private\" id=\"id5\"/>\n"
        "    <variable name=\"variable2\" units=\"units2\" id=\"id1\"/>\n"
        "    <reset variable=\"variable1\" test_variable=\"variable2\" order=\"1\" id=\"id2\">\n"
        "      <test_value id=\"id3\">\n"
        "        <math xmlns=\"http://www.w3.org/1998/Math/MathML\" xmlns:cellml=\"http://www.cellml.org/cellml/2.0#\" id=\"id4\">\n"
        "          <apply id=\"id1\">\n"
        "            <eq id=\"id2\"/>\n"
        "            <ci id=\"id3\">variable1</ci>\n"
        "            <cn id=\"id4\" cellml:units=\"units2\">3.4</cn>\n"
        "          </apply>\n"
        "        </math>\n"
        "      </test_value>\n"
        "      <reset_value id=\"id5\">\n"
        "        <math xmlns=\"http://www.w3.org/1998/Math/MathML\" xmlns:cellml=\"http://www.cellml.org/cellml/2.0#\" id=\"id1\">\n"
        "          <apply id=\"id4\">\n"
        "            <eq id=\"id3\"/>\n"
        "            <ci id=\"id2\">variable1</ci>\n"
        "            <cn id=\"id1\" cellml:units=\"units2\">9.0</cn>\n"
        "          </apply>\n"
        "        </math>\n"
        "      </reset_value>\n"
        "    </reset>\n"
        "  </component>\n"
        "  <component name=\"component3\" id=\"id2\">\n"
        "    <variable name=\"variable4\" units=\"units2\" interface=\"public\" id=\"id5\"/>\n"
        "    <variable name=\"variable2\" units=\"units2\" interface=\"public\" id=\"id3\"/>\n"
        "    <math xmlns=\"http://www.w3.org/1998/Math/MathML\" xmlns:cellml=\"http://www.cellml.org/cellml/2.0#\" id=\"id4\">\n"
        "      <apply id=\"id3\">\n"
        "        <eq id=\"id2\"/>\n"
        "        <ci id=\"id1\">variable4</ci>\n"
        "        <cn id=\"id5\" cellml:units=\"units2\">9.0</cn>\n"
        "      </apply>\n"
        "    </math>\n"
        "  </component>\n"
        "  <connection component_1=\"component2\" component_2=\"component3\" id=\"id4\">\n"
        "    <map_variables variable_1=\"variable1\" variable_2=\"variable2\" id=\"id5\"/>\n"
        "    <map_variables variable_1=\"variable1\" variable_2=\"variable4\" id=\"id2\"/>\n"
        "  </connection>\n"
        "  <encapsulation id=\"id1\">\n"
        "    <component_ref component=\"component2\" id=\"id2\">\n"
        "      <component_ref component=\"component3\" id=\"id3\"/>\n"
        "    </component_ref>\n"
        "  </encapsulation>\n"
        "</model>\n";

    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(in);
    auto validator = libcellml::Validator::create();
    validator->validateModel(model);
    EXPECT_EQ_ISSUES(expectedIssues, validator);
}

TEST(Validator, invalidIdsOnEveryElement)
{
    const std::vector<std::string> errorMessages = {
        "Model 'everything' does not have a valid 'id' attribute, 'model_1*'.",
        "Imported component 'component1' does not have a valid 'id' attribute, 'component_1@'.",
        "Import of component 'component1' does not have a valid 'id' attribute, '𒀦@'.",
        "Component 'component3' does not have a valid 'id' attribute, 'component_3$'.",
        "Variable 'variable1' does not have a valid 'id' attribute, 'variable_3$'.",
        "Variable 'variable2' does not have a valid 'id' attribute, 'variable_4$'.",
        "MathML ci element has the child text 'variable4' which does not correspond with any variable names present in component 'component3'.",
        "W3C MathML DTD error: Syntax of value for attribute id of math is not valid.",
        "W3C MathML DTD error: Syntax of value for attribute id of apply is not valid.",
        "W3C MathML DTD error: Syntax of value for attribute id of eq is not valid.",
        "W3C MathML DTD error: Syntax of value for attribute id of ci is not valid.",
        "W3C MathML DTD error: Syntax of value for attribute id of cn is not valid.",
        "Component 'component2' does not have a valid 'id' attribute, 'component_2!'.",
        "Variable 'variable1' does not have a valid 'id' attribute, 'ध!'.",
        "Variable 'variable2' does not have a valid 'id' attribute, 'variable_2]'.",
        "Reset in component 'component2' with order '1', ' does not have a valid 'id' attribute, 'reset_1['.",
        "W3C MathML DTD error: Syntax of value for attribute id of math is not valid.",
        "W3C MathML DTD error: Syntax of value for attribute id of apply is not valid.",
        "W3C MathML DTD error: Syntax of value for attribute id of eq is not valid.",
        "W3C MathML DTD error: Syntax of value for attribute id of ci is not valid.",
        "W3C MathML DTD error: Syntax of value for attribute id of cn is not valid.",
        "W3C MathML DTD error: Syntax of value for attribute id of math is not valid.",
        "W3C MathML DTD error: Syntax of value for attribute id of apply is not valid.",
        "W3C MathML DTD error: Syntax of value for attribute id of eq is not valid.",
        "W3C MathML DTD error: Syntax of value for attribute id of ci is not valid.",
        "W3C MathML DTD error: Syntax of value for attribute id of cn is not valid.",
        "Reset in component 'component2' with order '1', with variable 'variable1', with test_variable 'variable2', ' does not have a valid test_value 'id' attribute, 'test_value_1;'.",
        "Reset in component 'component2' with order '1', with variable 'variable1', with test_variable 'variable2', ' does not have a valid reset_value 'id' attribute, 'reset_value_1#'.",
        "Import of units 'units1' does not have a valid 'id' attribute, '今天@'.",
        "Imported units 'units1' does not have a valid 'id' attribute, '𒃵@'.",
        "Units 'units2' does not have a valid 'id' attribute, 'ऊ?'.",
        "Unit in units 'units2' does not have a valid 'id' attribute, 'ऊ?'.",
        "Units 'units3' does not have a valid 'id' attribute, '3456eight'.",
        "Units 'blob' does not have a valid 'id' attribute, 'units_4!'.",
        "Model 'everything' does not have a valid encapsulation 'id' attribute, 'encapsulation_1%'.",
        "Variable equivalence between variable 'variable1' in component 'component2' and variable 'variable1' in component 'component3', does not have a valid map_variables 'id' attribute, 'map_variables_1%'.",
        "Connection between components 'component2' and 'component3' because of variable equivalence between variables 'variable1' and 'variable1', does not have a valid connection 'id' attribute, 'connection_1%'.",
        "Variable equivalence between variable 'variable2' in component 'component2' and variable 'variable2' in component 'component3', does not have a valid map_variables 'id' attribute, 'map_variables_2%'.",
        "Component 'component2' does not have a valid encapsulation 'id' attribute, 'component_ref_θ%'.",
        "Component 'component3' does not have a valid encapsulation 'id' attribute, 'component_ref_⁇%'.",
    };

    auto parser = libcellml::Parser::create();
    auto validator = libcellml::Validator::create();
    auto model = parser->parseModel(fileContents("annotator/invalid_ids_on_every_element.cellml"));
    EXPECT_EQ(size_t(0), parser->issueCount());

    validator->validateModel(model);
    EXPECT_EQ(size_t(40), validator->errorCount());
    EXPECT_EQ_ISSUES(errorMessages, validator);
}

TEST(Validator, circularImportReferencesComponent)
{
    const std::string errorMessageImporter =
        std::string("Cyclic dependencies were found when attempting to resolve a component in the model 'circularImport1'. The dependency loop is:\n")
        + " - component 'i_am_cyclic' specifies an import from ':this:' to '" + resourcePath("importer/") + "circularImport_2.cellml';\n"
        + " - component 'c2' specifies an import from '" + resourcePath("importer/") + "circularImport_2.cellml' to '" + resourcePath("importer/") + "circularImport_3.cellml'; and\n"
        + " - component 'c3' specifies an import from '" + resourcePath("importer/") + "circularImport_3.cellml' to '" + resourcePath("importer/") + "circularImport_1.cellml'.";
    const std::string errorMessageValidator =
        "Cyclic dependencies were found when attempting to resolve a component in the model 'circularImport1'. The dependency loop is:\n"
        " - component 'i_am_cyclic' specifies an import from ':this:' to 'circularImport_2.cellml';\n"
        " - component 'c2' specifies an import from 'circularImport_2.cellml' to 'circularImport_3.cellml'; and\n"
        " - component 'c3' specifies an import from 'circularImport_3.cellml' to 'circularImport_1.cellml'.";

    auto parser = libcellml::Parser::create();
    auto validator = libcellml::Validator::create();
    auto importer = libcellml::Importer::create();

    auto model = parser->parseModel(fileContents("importer/circularImport_1.cellml"));
    EXPECT_EQ(size_t(0), parser->issueCount());

    validator->validateModel(model);
    EXPECT_EQ(size_t(0), validator->issueCount());

    importer->resolveImports(model, resourcePath("importer/"));
    EXPECT_EQ(size_t(1), importer->errorCount());
    EXPECT_EQ(errorMessageImporter, importer->error(0)->description());
    EXPECT_EQ(model->component(0), importer->issue(0)->item()->component());

    validator->validateModel(model);
    EXPECT_EQ(size_t(1), validator->errorCount());
    EXPECT_EQ(errorMessageValidator, validator->error(0)->description());
}

TEST(Validator, circularImportReferencesUnits)
{
    const std::string errorMessageImporter =
        std::string("Cyclic dependencies were found when attempting to resolve units in the model 'circularImport1'. The dependency loop is:\n")
        + " - units 'i_am_cyclic' specifies an import from ':this:' to '" + resourcePath("importer/") + "circularUnits_2.cellml';\n"
        + " - units 'u2' specifies an import from '" + resourcePath("importer/") + "circularUnits_2.cellml' to '" + resourcePath("importer/") + "circularUnits_3.cellml'; and\n"
        + " - units 'u3' specifies an import from '" + resourcePath("importer/") + "circularUnits_3.cellml' to '" + resourcePath("importer/") + "circularUnits_1.cellml'.";
    const std::string errorMessageValidator =
        "Cyclic dependencies were found when attempting to resolve units in the model 'circularImport1'. The dependency loop is:\n"
        " - units 'i_am_cyclic' references units 'u2' in 'circularUnits_2.cellml';\n"
        " - units 'u2' references units 'u3' in 'circularUnits_3.cellml'; and\n"
        " - units 'u3' references units 'i_am_cyclic' in 'circularUnits_1.cellml'.";

    auto parser = libcellml::Parser::create();
    auto validator = libcellml::Validator::create();
    auto importer = libcellml::Importer::create();

    auto model = parser->parseModel(fileContents("importer/circularUnits_1.cellml"));
    EXPECT_EQ(size_t(0), parser->issueCount());

    validator->validateModel(model);
    EXPECT_EQ(size_t(0), validator->issueCount());

    importer->resolveImports(model, resourcePath("importer/"));
    EXPECT_EQ(size_t(1), importer->errorCount());
    EXPECT_EQ(errorMessageImporter, importer->error(0)->description());
    EXPECT_EQ(model->units(0), importer->error(0)->item()->units());

    validator->validateModel(model);
    EXPECT_EQ(size_t(1), validator->errorCount());
    EXPECT_EQ(errorMessageImporter, importer->error(0)->description());
}

TEST(Validator, circularImportedUnitsDuplicateNames)
{
    const std::string errorMessageImporter =
        std::string("Cyclic dependencies were found when attempting to resolve units in the model 'circularImport1'. The dependency loop is:\n")
        + " - units 'i_am_duplicated' specifies an import from ':this:' to '" + resourcePath("importer/") + "circularUnits_2.cellml';\n"
        + " - units 'u2' specifies an import from '" + resourcePath("importer/") + "circularUnits_2.cellml' to '" + resourcePath("importer/") + "circularUnits_3.cellml';\n"
        + " - units 'u3' specifies an import from '" + resourcePath("importer/") + "circularUnits_3.cellml' to '" + resourcePath("importer/") + "circularUnits_1.cellml'; and\n"
        + " - units 'i_am_cyclic' specifies an import from '" + resourcePath("importer/") + "circularUnits_1.cellml' to '" + resourcePath("importer/") + "circularUnits_2.cellml'.";
    const std::vector<std::string> errorMessagesValidator = {
        "Cyclic dependencies were found when attempting to resolve units in the model 'circularImport1'. The dependency loop is:\n"
        " - units 'i_am_duplicated' specifies an import from ':this:' to 'circularUnits_2.cellml';\n"
        " - units 'u2' specifies an import from 'circularUnits_2.cellml' to 'circularUnits_3.cellml';\n"
        " - units 'u3' specifies an import from 'circularUnits_3.cellml' to 'circularUnits_1.cellml'; and\n"
        " - units 'i_am_cyclic' specifies an import from 'circularUnits_1.cellml' to 'circularUnits_2.cellml'.",
        "Model 'circularImport1' contains multiple units with the name 'i_am_duplicated'. Valid units names must be unique to their model.",
    };

    auto parser = libcellml::Parser::create();
    auto validator = libcellml::Validator::create();
    auto importer = libcellml::Importer::create();

    auto model = parser->parseModel(fileContents("importer/circularUnits_1_duplicated_name.cellml"));
    EXPECT_EQ(size_t(0), parser->issueCount());

    validator->validateModel(model);
    EXPECT_EQ(size_t(1), validator->issueCount());
    EXPECT_EQ(errorMessagesValidator[1], validator->issue(0)->description());
    EXPECT_EQ(model, validator->error(0)->item()->model());

    importer->resolveImports(model, resourcePath("importer/"));
    EXPECT_EQ(size_t(1), importer->errorCount());
    EXPECT_EQ(errorMessageImporter, importer->issue(0)->description());
    EXPECT_EQ(model->units(0), importer->issue(0)->item()->units());

    validator->validateModel(model);
    EXPECT_EQ_ISSUES(errorMessagesValidator, validator);
}

TEST(Validator, circularImportedComponentsDuplicateNames)
{
    const std::string errorMessageImporter =
        std::string("Cyclic dependencies were found when attempting to resolve a component in the model 'circularImport1'. The dependency loop is:\n")
        + " - component 'i_am_duplicated' specifies an import from ':this:' to '" + resourcePath("importer/") + "circularImport_2.cellml';\n"
        + " - component 'c2' specifies an import from '" + resourcePath("importer/") + "circularImport_2.cellml' to '" + resourcePath("importer/") + "circularImport_3.cellml';\n"
        + " - component 'c3' specifies an import from '" + resourcePath("importer/") + "circularImport_3.cellml' to '" + resourcePath("importer/") + "circularImport_1.cellml'; and\n"
        + " - component 'i_am_cyclic' specifies an import from '" + resourcePath("importer/") + "circularImport_1.cellml' to '" + resourcePath("importer/") + "circularImport_2.cellml'.";
    const std::vector<std::string> errorMessagesValidator = {
        "Cyclic dependencies were found when attempting to resolve a component in the model 'circularImport1'. The dependency loop is:\n"
        " - component 'i_am_duplicated' specifies an import from ':this:' to 'circularImport_2.cellml';\n"
        " - component 'c2' specifies an import from 'circularImport_2.cellml' to 'circularImport_3.cellml';\n"
        " - component 'c3' specifies an import from 'circularImport_3.cellml' to 'circularImport_1.cellml'; and\n"
        " - component 'i_am_cyclic' specifies an import from 'circularImport_1.cellml' to 'circularImport_2.cellml'.",
        "Model 'circularImport1' contains multiple components with the name 'i_am_duplicated'. Valid component names must be unique to their model.",
    };

    auto parser = libcellml::Parser::create();
    auto validator = libcellml::Validator::create();
    auto importer = libcellml::Importer::create();

    auto model = parser->parseModel(fileContents("importer/circularImport_1_duplicated_name.cellml"));
    EXPECT_EQ(size_t(0), parser->issueCount());

    validator->validateModel(model);
    EXPECT_EQ(size_t(1), validator->issueCount());
    EXPECT_EQ(errorMessagesValidator[1], validator->issue(0)->description());
    EXPECT_EQ(model, validator->error(0)->item()->model());

    importer->resolveImports(model, resourcePath("importer/"));
    EXPECT_EQ(size_t(1), importer->errorCount());
    EXPECT_EQ(errorMessageImporter, importer->issue(0)->description());
    EXPECT_EQ(model->units(0), importer->issue(0)->item()->units());

    validator->validateModel(model);
    EXPECT_EQ(size_t(2), validator->issueCount());
    EXPECT_EQ_ISSUES(errorMessagesValidator, validator);
}

TEST(Validator, importComponentWithInvalidName)
{
    const std::vector<std::string> errorMessages = {
        "Import of component 'c' has an invalid URI in the xlink:href attribute.",
        "The attempt to resolve imports with the model at '" + resourcePath("importer/") + "i am broken and invalid.cellml' failed: the file could not be opened.",
    };

    auto parser = libcellml::Parser::create();
    auto validator = libcellml::Validator::create();
    auto importer = libcellml::Importer::create();

    auto model = parser->parseModel(fileContents("importer/invalid_import_url.cellml"));
    EXPECT_EQ(size_t(0), parser->issueCount());

    validator->validateModel(model);
    EXPECT_EQ(size_t(1), validator->issueCount());
    EXPECT_EQ(errorMessages[0], validator->issue(0)->description());
    EXPECT_EQ(model->component("c")->importSource(), validator->error(0)->item()->importSource());

    importer->resolveImports(model, resourcePath("importer/"));
    EXPECT_EQ(size_t(1), importer->errorCount());
    EXPECT_EQ(errorMessages[1], importer->issue(0)->description());

    validator->validateModel(model);
    EXPECT_EQ(size_t(1), validator->issueCount());
    EXPECT_EQ(errorMessages[0], validator->issue(0)->description());
    EXPECT_EQ(model->component("c")->importSource(), validator->error(0)->item()->importSource());
}

TEST(Validator, importSecondGenComponentWithInvalidUri)
{
    const std::string errorMessageImporter =
        "The attempt to resolve imports with the model at '" + resourcePath("importer/") + "i am broken and invalid.cellml' failed: the file could not be opened.";
    const std::string errorMessageValidator =
        "Imported component 'c' is not valid because:\n"
        "  -> Component 'c' importing 'c' from 'invalid_import_url.cellml' has an error:\n"
        "   - Import of component 'c' has an invalid URI in the xlink:href attribute.";

    auto parser = libcellml::Parser::create();
    auto validator = libcellml::Validator::create();
    auto importer = libcellml::Importer::create();

    auto model = parser->parseModel(fileContents("importer/import_invalid_import_url.cellml"));
    EXPECT_EQ(size_t(0), parser->issueCount());

    validator->validateModel(model);
    EXPECT_EQ(size_t(0), validator->issueCount());

    importer->resolveImports(model, resourcePath("importer/"));
    EXPECT_EQ(size_t(1), importer->issueCount());
    EXPECT_EQ(errorMessageImporter, importer->issue(0)->description());
    EXPECT_EQ(model->component("c", true), importer->issue(0)->item()->component());

    validator->validateModel(model);
    EXPECT_EQ(size_t(1), validator->issueCount());
    EXPECT_EQ(errorMessageValidator, validator->issue(0)->description());
}

TEST(Validator, importedUnitBasedOnNonExistingUnit)
{
    const std::string errorMessageImporter =
        "Import of units 'i_am_bad' from '" + resourcePath("importer/") + "units_library.cellml' requires units named 'ps', which relies on child units named 'seconds', which cannot be found.";
    const std::string errorMessageValidator =
        "Imported units 'i_am_bad' is not valid because:\n"
        "  -> Units 'i_am_bad' importing 'ps' from 'units_library.cellml' has an error:\n"
        "   - Units reference 'seconds' in units 'ps' is not a valid reference to a local units or a standard unit type.";

    auto parser = libcellml::Parser::create();
    auto validator = libcellml::Validator::create();
    auto importer = libcellml::Importer::create();

    auto model = parser->parseModel(fileContents("importer/import_units_that_have_unknown_children.cellml"));
    EXPECT_EQ(size_t(0), parser->issueCount());

    validator->validateModel(model);
    EXPECT_EQ(size_t(0), validator->issueCount());

    importer->resolveImports(model, resourcePath("importer/"));
    EXPECT_EQ(size_t(1), importer->errorCount());
    EXPECT_EQ(errorMessageImporter, importer->error(0)->description());
    EXPECT_EQ(model->units(1), importer->issue(0)->item()->units());

    validator->validateModel(model);
    EXPECT_EQ(size_t(1), validator->issueCount());
    EXPECT_EQ(errorMessageValidator, validator->error(0)->description());
}

TEST(Validator, importInvalidUnitsNotDirectlyDeterminedFromImport)
{
    const std::string errorMessage =
        "Imported units 'i_am_bad' is not valid because:\n"
        "  -> Units 'i_am_bad' importing 'ps2' from 'units_library.cellml' has an error:\n"
        "   - Prefix 'plinco' of a unit referencing 'second' in units 'ps2' is not a valid integer or an SI prefix.";

    auto parser = libcellml::Parser::create();
    auto validator = libcellml::Validator::create();
    auto importer = libcellml::Importer::create();

    auto model = parser->parseModel(fileContents("importer/import_units_that_are_invalid.cellml"));
    EXPECT_EQ(size_t(0), parser->issueCount());

    auto unitsLibrary = parser->parseModel(fileContents("importer/units_library.cellml"));
    EXPECT_EQ(size_t(0), parser->issueCount());

    validator->validateModel(model);
    EXPECT_EQ(size_t(0), validator->issueCount());

    validator->validateModel(unitsLibrary);
    EXPECT_EQ(size_t(2), validator->issueCount());

    importer->resolveImports(model, resourcePath("importer/"));

    validator->validateModel(model);
    EXPECT_EQ(size_t(1), validator->errorCount());
    EXPECT_EQ(errorMessage, validator->issue(0)->description());
}

TEST(Validator, importInvalidUnitsReference)
{
    const std::string errorMessage =
        "Imported units 'i_am_bad' refers to units 'ps3' which does not appear in 'units_library.cellml'.";

    auto parser = libcellml::Parser::create();
    auto validator = libcellml::Validator::create();
    auto importer = libcellml::Importer::create();

    auto model = parser->parseModel(fileContents("importer/import_units_that_are_not_there.cellml"));
    EXPECT_EQ(size_t(0), parser->issueCount());

    auto unitsLibrary = parser->parseModel(fileContents("importer/units_library.cellml"));
    EXPECT_EQ(size_t(0), parser->issueCount());

    validator->validateModel(model);
    EXPECT_EQ(size_t(0), validator->issueCount());

    validator->validateModel(unitsLibrary);
    EXPECT_EQ(size_t(2), validator->issueCount());

    importer->resolveImports(model, resourcePath("importer/"));

    validator->validateModel(model);
    EXPECT_EQ(size_t(1), validator->errorCount());
    EXPECT_EQ(errorMessage, validator->issue(0)->description());
}

TEST(Validator, importInvalidComponentNotDirectlyDeterminedFromImport)
{
    const std::string errorMessage1 =
        "Component 'invalid_component' does not have a valid 'id' attribute, '8ioisfje+'.";
    const std::string errorMessage2 =
        "Imported component 'imported_component' is not valid because:\n"
        "  -> Component 'imported_component' importing 'invalid_component' from 'component_that_is_invalid.cellml' has an error:\n"
        "   - Component 'invalid_component' does not have a valid 'id' attribute, '8ioisfje+'.";

    auto parser = libcellml::Parser::create();
    auto validator = libcellml::Validator::create();
    auto importer = libcellml::Importer::create();

    auto model = parser->parseModel(fileContents("importer/importing_a_component_that_is_invalid.cellml"));
    EXPECT_EQ(size_t(0), parser->issueCount());

    auto invalidComponentModel = parser->parseModel(fileContents("importer/component_that_is_invalid.cellml"));
    EXPECT_EQ(size_t(0), parser->issueCount());

    validator->validateModel(model);
    EXPECT_EQ(size_t(0), validator->issueCount());

    validator->validateModel(invalidComponentModel);
    EXPECT_EQ(size_t(1), validator->issueCount());
    EXPECT_EQ(errorMessage1, validator->issue(0)->description());

    importer->resolveImports(model, resourcePath("importer/"));

    validator->validateModel(model);
    EXPECT_EQ(size_t(1), validator->errorCount());
    EXPECT_EQ(errorMessage2, validator->issue(0)->description());
}

TEST(Validator, importInvalidComponentReference)
{
    const std::string errorMessage =
        "Imported component 'imported_component' refers to component 'missing_component' which does not appear in 'component_that_is_invalid.cellml'.";

    auto parser = libcellml::Parser::create();
    auto validator = libcellml::Validator::create();
    auto importer = libcellml::Importer::create();

    auto model = parser->parseModel(fileContents("importer/importing_a_component_that_is_not_there.cellml"));
    EXPECT_EQ(size_t(0), parser->issueCount());

    validator->validateModel(model);
    EXPECT_EQ(size_t(0), validator->issueCount());

    importer->resolveImports(model, resourcePath("importer/"));

    validator->validateModel(model);
    EXPECT_EQ(size_t(1), validator->errorCount());
    EXPECT_EQ(errorMessage, validator->issue(0)->description());
}

TEST(Validator, unitsErrorInImportedComponent)
{
    const std::string errorMessage =
        "Imported component 'component1' is not valid because:\n"
        "  -> Component 'component1' importing 'component2' from 'model2.cellml' imports:\n"
        "  -> Component 'component2' importing 'component3' from 'model3.cellml' which has an error:\n"
        "   - Variable 'time' in component 'component3' has a units reference 'seconds' which is neither standard nor defined in the parent model.";

    auto model1 = libcellml::Model::create("model1");
    auto model2 = libcellml::Model::create("model2");
    auto model3 = libcellml::Model::create("model3");

    auto variable1 = libcellml::Variable::create("time");

    auto units1 = libcellml::Units::create("seconds");

    auto component1 = libcellml::Component::create("component1");
    auto component2 = libcellml::Component::create("component2");
    auto component3 = libcellml::Component::create("component3");

    auto imp1 = libcellml::ImportSource::create();
    auto imp2 = libcellml::ImportSource::create();

    imp1->setModel(model2);
    imp1->setUrl("model2.cellml");
    imp2->setModel(model3);
    imp2->setUrl("model3.cellml");

    model1->addComponent(component1);
    model2->addComponent(component2);
    model3->addComponent(component3);

    component3->addVariable(variable1);

    variable1->setUnits(units1);

    component1->setSourceComponent(imp1, "component2");
    component2->setSourceComponent(imp2, "component3");

    auto validator = libcellml::Validator::create();

    validator->validateModel(model1);
    EXPECT_EQ(size_t(1), validator->errorCount());
    EXPECT_EQ(errorMessage, validator->error(0)->description());
}

TEST(Validator, cImportThatIllustratesBadPractice)
{
    auto parser = libcellml::Parser::create();
    auto validator = libcellml::Validator::create();
    auto importer = libcellml::Importer::create();

    auto model = parser->parseModel(fileContents("importer/importing_bad_design_c_shape_import_hierarchy.cellml"));
    EXPECT_EQ(size_t(0), parser->issueCount());

    validator->validateModel(model);
    EXPECT_EQ(size_t(0), validator->issueCount());

    importer->resolveImports(model, resourcePath("importer/"));
    EXPECT_EQ(size_t(0), importer->issueCount());

    validator->validateModel(model);
    EXPECT_EQ(size_t(0), validator->errorCount());
}

TEST(Validator, zImportThatIllustratesBadPractice)
{
    const std::string errorMessage =
        std::string("Cyclic dependencies were found when attempting to resolve a component in the model 'import_component_from_library_and_another_component'. The dependency loop is:\n")
        + " - component 'c1' specifies an import from ':this:' to '" + resourcePath("importer/") + "component_library.cellml'; and\n"
        + " - component 'c1_imported' specifies an import from '" + resourcePath("importer/") + "component_library.cellml' to '" + resourcePath("importer/") + "layer1/importing_bad_design_z_import_hierarchy.cellml'.";

    auto parser = libcellml::Parser::create();
    auto validator = libcellml::Validator::create();
    auto importer = libcellml::Importer::create();

    auto model = parser->parseModel(fileContents("importer/importing_bad_design_z_import_hierarchy.cellml"));
    EXPECT_EQ(size_t(0), parser->issueCount());

    validator->validateModel(model);
    EXPECT_EQ(size_t(0), validator->issueCount());

    // The importer incorrectly thinks the 2nd import is the same as the original model because it cannot determine which file the
    // original model came from.  It should be noted that the 2nd import is the same as the original model but from a different directory.
    importer->resolveImports(model, resourcePath("importer/"));
    EXPECT_EQ(size_t(1), importer->errorCount());
    EXPECT_EQ(errorMessage, importer->error(0)->description());

    validator->validateModel(model);
    EXPECT_EQ(size_t(0), validator->errorCount());
}

TEST(Validator, zOffsetImportThatIllustratesBadPractice)
{
    auto parser = libcellml::Parser::create();
    auto validator = libcellml::Validator::create();
    auto importer = libcellml::Importer::create();

    auto model = parser->parseModel(fileContents("importer/importing_bad_design_z_offset_import_hierarchy.cellml"));
    EXPECT_EQ(size_t(0), parser->issueCount());

    validator->validateModel(model);
    EXPECT_EQ(size_t(0), validator->issueCount());

    // With the import offset the importer can correctly identify that the models are in fact from different locations.
    importer->resolveImports(model, resourcePath("importer/"));
    EXPECT_EQ(size_t(0), importer->errorCount());

    validator->validateModel(model);
    EXPECT_EQ(size_t(0), validator->errorCount());
}

TEST(Validator, invalidMathMLElementsChildrenOrSiblings)
{
    const std::vector<std::string> expectedIssues = {
        "Math has a 'apply' element without at least one MathML child.",
        "Math has a 'eq' element without exactly two MathML siblings.",
        "Math has a 'eq' element which is not the first MathML sibling.",
        "Math has a 'neq' element without exactly two MathML siblings.",
        "Math has a 'neq' element which is not the first MathML sibling.",
        "Math has a 'lt' element without exactly two MathML siblings.",
        "Math has a 'lt' element which is not the first MathML sibling.",
        "Math has a 'leq' element without exactly two MathML siblings.",
        "Math has a 'leq' element which is not the first MathML sibling.",
        "Math has a 'gt' element without exactly two MathML siblings.",
        "Math has a 'gt' element which is not the first MathML sibling.",
        "Math has a 'geq' element without exactly two MathML siblings.",
        "Math has a 'geq' element which is not the first MathML sibling.",
        "Math has a 'and' element without at least two MathML siblings.",
        "Math has a 'and' element which is not the first MathML sibling.",
        "Math has a 'or' element without at least two MathML siblings.",
        "Math has a 'or' element which is not the first MathML sibling.",
        "Math has a 'xor' element without at least two MathML siblings.",
        "Math has a 'xor' element which is not the first MathML sibling.",
        "Math has a 'not' element without exactly one MathML sibling.",
        "Math has a 'not' element which is not the first MathML sibling.",
        "Math has a 'plus' element without at least one MathML sibling.",
        "Math has a 'plus' element which is not the first MathML sibling.",
        "Math has a 'minus' element without exactly one or two MathML siblings.",
        "Math has a 'minus' element which is not the first MathML sibling.",
        "Math has a 'times' element without at least two MathML siblings.",
        "Math has a 'times' element which is not the first MathML sibling.",
        "Math has a 'divide' element without exactly two MathML siblings.",
        "Math has a 'divide' element which is not the first MathML sibling.",
        "Math has a 'power' element without exactly two MathML siblings.",
        "Math has a 'power' element which is not the first MathML sibling.",
        "Math has a 'root' element without exactly one or two MathML siblings.",
        "Math has a 'root' element which is not the first MathML sibling.",
        "Math has a 'root' element which is not the first MathML sibling.",
        "Math has a 'root' element which first sibling is not a 'degree' element.",
        "Math has a 'abs' element without exactly one MathML sibling.",
        "Math has a 'abs' element which is not the first MathML sibling.",
        "Math has a 'exp' element without exactly one MathML sibling.",
        "Math has a 'exp' element which is not the first MathML sibling.",
        "Math has a 'ln' element without exactly one MathML sibling.",
        "Math has a 'ln' element which is not the first MathML sibling.",
        "Math has a 'log' element without exactly one or two MathML siblings.",
        "Math has a 'log' element which is not the first MathML sibling.",
        "Math has a 'log' element which is not the first MathML sibling.",
        "Math has a 'log' element which first sibling is not a 'logbase' element.",
        "Math has a 'ceiling' element without exactly one MathML sibling.",
        "Math has a 'ceiling' element which is not the first MathML sibling.",
        "Math has a 'floor' element without exactly one MathML sibling.",
        "Math has a 'floor' element which is not the first MathML sibling.",
        "Math has a 'diff' element without exactly two MathML siblings.",
        "Math has a 'diff' element which is not the first MathML sibling.",
        "Math has a 'diff' element which first sibling is not a 'bvar' element.",
        "Math has a 'sin' element without exactly one MathML sibling.",
        "Math has a 'sin' element which is not the first MathML sibling.",
        "Math has a 'cos' element without exactly one MathML sibling.",
        "Math has a 'cos' element which is not the first MathML sibling.",
        "Math has a 'tan' element without exactly one MathML sibling.",
        "Math has a 'tan' element which is not the first MathML sibling.",
        "Math has a 'sec' element without exactly one MathML sibling.",
        "Math has a 'sec' element which is not the first MathML sibling.",
        "Math has a 'csc' element without exactly one MathML sibling.",
        "Math has a 'csc' element which is not the first MathML sibling.",
        "Math has a 'cot' element without exactly one MathML sibling.",
        "Math has a 'cot' element which is not the first MathML sibling.",
        "Math has a 'sinh' element without exactly one MathML sibling.",
        "Math has a 'sinh' element which is not the first MathML sibling.",
        "Math has a 'cosh' element without exactly one MathML sibling.",
        "Math has a 'cosh' element which is not the first MathML sibling.",
        "Math has a 'tanh' element without exactly one MathML sibling.",
        "Math has a 'tanh' element which is not the first MathML sibling.",
        "Math has a 'sech' element without exactly one MathML sibling.",
        "Math has a 'sech' element which is not the first MathML sibling.",
        "Math has a 'csch' element without exactly one MathML sibling.",
        "Math has a 'csch' element which is not the first MathML sibling.",
        "Math has a 'coth' element without exactly one MathML sibling.",
        "Math has a 'coth' element which is not the first MathML sibling.",
        "Math has a 'arcsin' element without exactly one MathML sibling.",
        "Math has a 'arcsin' element which is not the first MathML sibling.",
        "Math has a 'arccos' element without exactly one MathML sibling.",
        "Math has a 'arccos' element which is not the first MathML sibling.",
        "Math has a 'arctan' element without exactly one MathML sibling.",
        "Math has a 'arctan' element which is not the first MathML sibling.",
        "Math has a 'arcsec' element without exactly one MathML sibling.",
        "Math has a 'arcsec' element which is not the first MathML sibling.",
        "Math has a 'arccsc' element without exactly one MathML sibling.",
        "Math has a 'arccsc' element which is not the first MathML sibling.",
        "Math has a 'arccot' element without exactly one MathML sibling.",
        "Math has a 'arccot' element which is not the first MathML sibling.",
        "Math has a 'arcsinh' element without exactly one MathML sibling.",
        "Math has a 'arcsinh' element which is not the first MathML sibling.",
        "Math has a 'arccosh' element without exactly one MathML sibling.",
        "Math has a 'arccosh' element which is not the first MathML sibling.",
        "Math has a 'arctanh' element without exactly one MathML sibling.",
        "Math has a 'arctanh' element which is not the first MathML sibling.",
        "Math has a 'arcsech' element without exactly one MathML sibling.",
        "Math has a 'arcsech' element which is not the first MathML sibling.",
        "Math has a 'arccsch' element without exactly one MathML sibling.",
        "Math has a 'arccsch' element which is not the first MathML sibling.",
        "Math has a 'arccoth' element without exactly one MathML sibling.",
        "Math has a 'arccoth' element which is not the first MathML sibling.",
        "Math has a 'piece' element without exactly two MathML children.",
        "Math has a 'otherwise' element without exactly one MathML child.",
        "Math has a 'ci' element with no identifier as a child.",
        "Math has a 'cn' element which is not in base 10.",
        "Math has a 'cn' element which is not of 'real' or 'e-notation' type.",
        "Math has a 'cn' element of 'real' type with no valid text node (representing a basic number) as a child.",
        "Math has a 'cn' element of 'real' type with no valid text node (representing a basic number) as a child.",
        "Math has a 'cn' element of 'real' type with no valid text node (representing a basic number) as a child.",
        "Math has a 'cn' element of 'e-notation' type with no valid text node (representing a basic number), no 'sep' element, and/or no valid text node (representing an integer) as children.",
        "Math has a 'cn' element of 'e-notation' type with no valid text node (representing a basic number), no 'sep' element, and/or no valid text node (representing an integer) as children.",
        "Math has a 'cn' element of 'e-notation' type with no valid text node (representing a basic number), no 'sep' element, and/or no valid text node (representing an integer) as children.",
        "Math has a 'cn' element of 'e-notation' type with no valid text node (representing a basic number), no 'sep' element, and/or no valid text node (representing an integer) as children.",
        "Math has a 'cn' element of 'e-notation' type with no valid text node (representing a basic number), no 'sep' element, and/or no valid text node (representing an integer) as children.",
        "Math has a 'cn' element of 'e-notation' type with no valid text node (representing a basic number), no 'sep' element, and/or no valid text node (representing an integer) as children.",
        "Math has a 'cn' element of 'e-notation' type with no valid text node (representing a basic number), no 'sep' element, and/or no valid text node (representing an integer) as children.",
        "Math has a 'cn' element of 'e-notation' type with no valid text node (representing a basic number), no 'sep' element, and/or no valid text node (representing an integer) as children.",
        "Math has a 'cn' element of 'e-notation' type with no valid text node (representing a basic number), no 'sep' element, and/or no valid text node (representing an integer) as children.",
        "Math has a 'degree' element without exactly one or two MathML siblings.",
        "Math has a 'degree' element which is not the second MathML sibling.",
        "Math has a 'degree' element without exactly one MathML child.",
        "Math has a 'degree' element which first sibling is not a 'root' element.",
        "Math has a 'root' element which first sibling is not a 'degree' element.",
        "Math has a 'degree' element which is not the second MathML sibling.",
        "Math has a 'degree' element without exactly one MathML child.",
        "Math has a 'logbase' element without exactly two MathML siblings.",
        "Math has a 'logbase' element which first sibling is not a 'log' element.",
        "Math has a 'log' element which first sibling is not a 'logbase' element.",
        "Math has a 'logbase' element which is not the second MathML sibling.",
        "Math has a 'logbase' element without exactly one MathML child.",
        "Math has a 'bvar' element without exactly two MathML siblings.",
        "Math has a 'bvar' element which first sibling is not a 'diff' element.",
        "Math has a 'diff' element which first sibling is not a 'bvar' element.",
        "Math has a 'bvar' element which is not the second MathML sibling.",
        "Math has a 'bvar' element without exactly one or two MathML children.",
        "W3C MathML DTD error: Element true was declared EMPTY this one has content.",
        "W3C MathML DTD error: Element false was declared EMPTY this one has content.",
        "W3C MathML DTD error: Element exponentiale was declared EMPTY this one has content.",
        "W3C MathML DTD error: Element pi was declared EMPTY this one has content.",
        "W3C MathML DTD error: Element infinity was declared EMPTY this one has content.",
        "W3C MathML DTD error: Element notanumber was declared EMPTY this one has content.",
    };

    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("invalidmathmlelementschildrenorsiblings.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto validator = libcellml::Validator::create();

    validator->validateModel(model);

    EXPECT_EQ_ISSUES(expectedIssues, validator);
}
