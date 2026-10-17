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

TEST(ModelFlattening, invalidInput)
{
    auto importer = libcellml::Importer::create();

    auto flatModel = importer->flattenModel(nullptr);

    EXPECT_EQ(nullptr, flatModel);
    EXPECT_EQ(size_t(1), importer->issueCount());
    EXPECT_EQ("The model is null.", importer->issue(0)->description());
    EXPECT_EQ(libcellml::Issue::ReferenceRule::INVALID_ARGUMENT, importer->issue(0)->referenceRule());
}

TEST(ModelFlattening, modelWithoutImports)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"units\">\n"
        "  <units name=\"ms\">\n"
        "    <unit prefix=\"milli\" units=\"second\"/>\n"
        "  </units>\n"
        "</model>\n";

    auto parser = libcellml::Parser::create();
    auto modelUnitsDefinitions = parser->parseModel(e);
    auto importer = libcellml::Importer::create();

    EXPECT_FALSE(modelUnitsDefinitions->hasUnresolvedImports());

    auto flatModel = importer->flattenModel(modelUnitsDefinitions);

    auto printer = libcellml::Printer::create();

    auto a = printer->printModel(flatModel);
    EXPECT_EQ(e, a);
}

TEST(ModelFlattening, modelWithUnresolvedImports)
{
    const std::string modelImportingUnits =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"importer_of_units\">\n"
        "  <import xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"unitsdefinitions.xml\">\n"
        "    <units units_ref=\"ms\" name=\"my_ms\"/>\n"
        "  </import>\n"
        "</model>\n";

    auto parser = libcellml::Parser::create();
    auto modelWithUnitsImports = parser->parseModel(modelImportingUnits);

    EXPECT_TRUE(modelWithUnitsImports->hasUnresolvedImports());

    auto importer = libcellml::Importer::create();
    auto flatModel = importer->flattenModel(modelWithUnitsImports);

    EXPECT_EQ(nullptr, flatModel);
}

TEST(ModelFlattening, importedUnits)
{
    const std::string unitsDefinitionModel =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" xmlns:cellml=\"http://www.cellml.org/cellml/2.0#\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" name=\"units\">\n"
        "  <units name=\"ms\">\n"
        "    <unit units=\"second\" prefix=\"milli\"/>\n"
        "  </units>\n"
        "</model>\n";

    const std::string modelImportingUnits =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model name=\"importer_of_units\" xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <import xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"unitsdefinitions.xml\">\n"
        "    <units units_ref=\"ms\" name=\"my_ms\"/>\n"
        "  </import>\n"
        "</model>\n";

    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"importer_of_units\">\n"
        "  <units name=\"my_ms\">\n"
        "    <unit prefix=\"milli\" units=\"second\"/>\n"
        "  </units>\n"
        "</model>\n";

    auto parser = libcellml::Parser::create();

    auto modelWithUnitsImports = parser->parseModel(modelImportingUnits);
    auto modelUnitsDefinitions = parser->parseModel(unitsDefinitionModel);

    EXPECT_TRUE(modelWithUnitsImports->hasUnresolvedImports());

    auto units = modelWithUnitsImports->units(0);
    auto importSource = units->importSource();
    importSource->setModel(modelUnitsDefinitions);

    EXPECT_FALSE(modelWithUnitsImports->hasUnresolvedImports());

    auto importer = libcellml::Importer::create();

    modelWithUnitsImports = importer->flattenModel(modelWithUnitsImports);

    auto printer = libcellml::Printer::create();

    auto a = printer->printModel(modelWithUnitsImports);
    EXPECT_EQ(e, a);

    // Check that we haven't changed the imported model.
    auto aModelUnitsDefinitions = printer->printModel(modelUnitsDefinitions);
    auto eModelUnitsDefinitions = printer->printModel(parser->parseModel(unitsDefinitionModel));
    EXPECT_EQ(eModelUnitsDefinitions, aModelUnitsDefinitions);
}

TEST(ModelFlattening, importedComponent)
{
    const std::string sineDefinitionModel =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" xmlns:cellml=\"http://www.cellml.org/cellml/2.0#\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" name=\"sin\" id=\"sin\">\n"
        "  <component name=\"sin\" id=\"sin\">\n"
        "    <variable name=\"x\" units=\"dimensionless\" interface=\"public_and_private\"/>\n"
        "    <variable id=\"sin\" units=\"dimensionless\" name=\"sin\" interface=\"public_and_private\"/>\n"
        "    <math xmlns=\"http://www.w3.org/1998/Math/MathML\">\n"
        "      <apply id=\"actual_sin\">\n"
        "        <eq/>\n"
        "        <ci>sin</ci>\n"
        "        <apply>\n"
        "          <sin/>\n"
        "          <ci>x</ci>\n"
        "        </apply>\n"
        "      </apply>\n"
        "    </math>\n"
        "  </component>\n"
        "</model>\n";

    const std::string modelImportingComponent =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" xmlns:cellml=\"http://www.cellml.org/cellml/2.0#\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" name=\"sin_approximations_import\" id=\"sin_approximations_import\">\n"
        "  <import xlink:href=\"sin.xml\">\n"
        "    <component name=\"actual_sin\" component_ref=\"sin\"/>\n"
        "  </import>\n"
        "  <component name=\"main\" id=\"main\"/>\n"
        "  <encapsulation>\n"
        "    <component_ref component=\"main\">\n"
        "      <component_ref component=\"actual_sin\"/>\n"
        "    </component_ref>\n"
        "  </encapsulation>\n"
        "</model>\n";

    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"sin_approximations_import\" id=\"sin_approximations_import\">\n"
        "  <component name=\"main\" id=\"main\"/>\n"
        "  <component name=\"actual_sin\" id=\"sin\">\n"
        "    <variable name=\"x\" units=\"dimensionless\" interface=\"public_and_private\"/>\n"
        "    <variable name=\"sin\" units=\"dimensionless\" interface=\"public_and_private\" id=\"sin\"/>\n"
        "    <math xmlns=\"http://www.w3.org/1998/Math/MathML\">\n"
        "      <apply id=\"actual_sin\">\n"
        "        <eq/>\n"
        "        <ci>sin</ci>\n"
        "        <apply>\n"
        "          <sin/>\n"
        "          <ci>x</ci>\n"
        "        </apply>\n"
        "      </apply>\n"
        "    </math>\n"
        "  </component>\n"
        "  <encapsulation>\n"
        "    <component_ref component=\"main\">\n"
        "      <component_ref component=\"actual_sin\"/>\n"
        "    </component_ref>\n"
        "  </encapsulation>\n"
        "</model>\n";

    auto parser = libcellml::Parser::create();

    auto modelWithComponentImport = parser->parseModel(modelImportingComponent);
    auto modelSineDefinition = parser->parseModel(sineDefinitionModel);

    EXPECT_TRUE(modelWithComponentImport->hasUnresolvedImports());

    auto component = modelWithComponentImport->component(0)->component(0);
    auto importSource = component->importSource();
    importSource->setModel(modelSineDefinition);

    EXPECT_TRUE(component->isImport());
    EXPECT_FALSE(modelWithComponentImport->hasUnresolvedImports());

    auto importer = libcellml::Importer::create();
    modelWithComponentImport = importer->flattenModel(modelWithComponentImport);

    auto printer = libcellml::Printer::create();

    auto a = printer->printModel(modelWithComponentImport);
    EXPECT_EQ(e, a);
}

TEST(ModelFlattening, importedComponentWithEquivalentVariables)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"main_model\">\n"
        "  <component name=\"main_model_component\">\n"
        "    <variable name=\"v\" units=\"dimensionless\" initial_value=\"1\" interface=\"public_and_private\"/>\n"
        "  </component>\n"
        "  <component name=\"other_component\">\n"
        "    <variable name=\"time\" units=\"dimensionless\" interface=\"public\"/>\n"
        "    <variable name=\"parameter\" units=\"dimensionless\" interface=\"public\"/>\n"
        "    <variable name=\"cosine\" units=\"dimensionless\" initial_value=\"0\" interface=\"public\"/>\n"
        "    <math xmlns=\"http://www.w3.org/1998/Math/MathML\">\n"
        "      <apply>\n"
        "        <eq/>\n"
        "        <apply>\n"
        "          <diff/>\n"
        "          <bvar>\n"
        "            <ci>time</ci>\n"
        "          </bvar>\n"
        "          <ci>cosine</ci>\n"
        "        </apply>\n"
        "        <apply>\n"
        "          <sin/>\n"
        "          <apply>\n"
        "            <times/>\n"
        "            <ci>parameter</ci>\n"
        "            <ci>time</ci>\n"
        "          </apply>\n"
        "        </apply>\n"
        "      </apply>\n"
        "    </math>\n"
        "  </component>\n"
        "  <connection component_1=\"main_model_component\" component_2=\"other_component\">\n"
        "    <map_variables variable_1=\"v\" variable_2=\"parameter\"/>\n"
        "  </connection>\n"
        "  <encapsulation>\n"
        "    <component_ref component=\"main_model_component\">\n"
        "      <component_ref component=\"other_component\"/>\n"
        "    </component_ref>\n"
        "  </encapsulation>\n"
        "</model>\n";

    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("modelflattening/equivalentimportedvariable.xml"));
    auto importer = libcellml::Importer::create();

    EXPECT_TRUE(model->hasUnresolvedImports());

    importer->resolveImports(model, resourcePath("modelflattening/"));

    EXPECT_FALSE(model->hasUnresolvedImports());

    // Want to make sure that we haven't damaged the import model after flattening.
    auto importModel = model->component(0)->component(0)->importSource()->model();

    model = importer->flattenModel(model);

    auto printer = libcellml::Printer::create();

    auto a = printer->printModel(model);
    EXPECT_EQ(e, a);

    // Check that we haven't changed the imported model.
    auto aImportModel = printer->printModel(importModel);
    auto eImportModel = printer->printModel(parser->parseModel(fileContents("modelflattening/reusable.xml")));
    EXPECT_EQ(eImportModel, aImportModel);
}

TEST(ModelFlattening, importedComponentWithInternalEquivalentVariables)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"main_model\">\n"
        "  <component name=\"main_model_component\">\n"
        "    <variable name=\"v\" units=\"dimensionless\" initial_value=\"1\" interface=\"public_and_private\"/>\n"
        "  </component>\n"
        "  <component name=\"other_component\">\n"
        "    <variable name=\"time\" units=\"dimensionless\" interface=\"public_and_private\"/>\n"
        "    <variable name=\"parameter\" units=\"dimensionless\" interface=\"public\"/>\n"
        "    <variable name=\"cosine\" units=\"dimensionless\" initial_value=\"0\" interface=\"public\"/>\n"
        "    <math xmlns=\"http://www.w3.org/1998/Math/MathML\">\n"
        "      <apply>\n"
        "        <eq/>\n"
        "        <apply>\n"
        "          <diff/>\n"
        "          <bvar>\n"
        "            <ci>time</ci>\n"
        "          </bvar>\n"
        "          <ci>cosine</ci>\n"
        "        </apply>\n"
        "        <apply>\n"
        "          <sin/>\n"
        "          <apply>\n"
        "            <times/>\n"
        "            <ci>parameter</ci>\n"
        "            <ci>time</ci>\n"
        "          </apply>\n"
        "        </apply>\n"
        "      </apply>\n"
        "    </math>\n"
        "  </component>\n"
        "  <component name=\"reusable_child_1\">\n"
        "    <variable name=\"time\" units=\"dimensionless\" interface=\"public\"/>\n"
        "  </component>\n"
        "  <component name=\"reusable_child_2\">\n"
        "    <variable name=\"time\" units=\"dimensionless\" interface=\"public\"/>\n"
        "  </component>\n"
        "  <connection component_1=\"other_component\" component_2=\"reusable_child_1\">\n"
        "    <map_variables variable_1=\"time\" variable_2=\"time\"/>\n"
        "  </connection>\n"
        "  <connection component_1=\"reusable_child_1\" component_2=\"reusable_child_2\">\n"
        "    <map_variables variable_1=\"time\" variable_2=\"time\"/>\n"
        "  </connection>\n"
        "  <encapsulation>\n"
        "    <component_ref component=\"main_model_component\">\n"
        "      <component_ref component=\"other_component\">\n"
        "        <component_ref component=\"reusable_child_1\"/>\n"
        "        <component_ref component=\"reusable_child_2\"/>\n"
        "      </component_ref>\n"
        "    </component_ref>\n"
        "  </encapsulation>\n"
        "</model>\n";

    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("modelflattening/internalequivalentvariables.xml"));
    auto importer = libcellml::Importer::create();

    EXPECT_TRUE(model->hasUnresolvedImports());
    importer->resolveImports(model, resourcePath("modelflattening/"));
    EXPECT_FALSE(model->hasUnresolvedImports());

    // Want to make sure that we haven't damaged the import model after flattening.
    auto importModel = model->component(0)->component(0)->importSource()->model();

    model = importer->flattenModel(model);

    auto printer = libcellml::Printer::create();

    auto a = printer->printModel(model);
    EXPECT_EQ(e, a);

    // Check that we haven't changed the imported model.
    auto aImportModel = printer->printModel(importModel);
    auto eImportModel = printer->printModel(parser->parseModel(fileContents("modelflattening/reusable2.xml")));
    EXPECT_EQ(eImportModel, aImportModel);
}

TEST(ModelFlattening, importedComponentWithEquivalentVariablesReferencingVariablesOutsideOfImportedComponent)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"main_model\">\n"
        "  <component name=\"main_model_component\">\n"
        "    <variable name=\"v\" units=\"dimensionless\" initial_value=\"1\" interface=\"public_and_private\"/>\n"
        "  </component>\n"
        "  <component name=\"other_component\">\n"
        "    <variable name=\"time\" units=\"dimensionless\" interface=\"public_and_private\"/>\n"
        "    <variable name=\"parameter\" units=\"dimensionless\" interface=\"public\"/>\n"
        "    <variable name=\"cosine\" units=\"dimensionless\" initial_value=\"0\" interface=\"public\"/>\n"
        "    <math xmlns=\"http://www.w3.org/1998/Math/MathML\">\n"
        "      <apply>\n"
        "        <eq/>\n"
        "        <apply>\n"
        "          <diff/>\n"
        "          <bvar>\n"
        "            <ci>time</ci>\n"
        "          </bvar>\n"
        "          <ci>cosine</ci>\n"
        "        </apply>\n"
        "        <apply>\n"
        "          <sin/>\n"
        "          <apply>\n"
        "            <times/>\n"
        "            <ci>parameter</ci>\n"
        "            <ci>time</ci>\n"
        "          </apply>\n"
        "        </apply>\n"
        "      </apply>\n"
        "    </math>\n"
        "  </component>\n"
        "  <component name=\"reusable_child_1\">\n"
        "    <variable name=\"time\" units=\"dimensionless\" interface=\"public\"/>\n"
        "  </component>\n"
        "  <component name=\"reusable_child_2\">\n"
        "    <variable name=\"time\" units=\"dimensionless\" interface=\"public\"/>\n"
        "  </component>\n"
        "  <connection component_1=\"other_component\" component_2=\"reusable_child_1\">\n"
        "    <map_variables variable_1=\"time\" variable_2=\"time\"/>\n"
        "  </connection>\n"
        "  <connection component_1=\"reusable_child_1\" component_2=\"reusable_child_2\">\n"
        "    <map_variables variable_1=\"time\" variable_2=\"time\"/>\n"
        "  </connection>\n"
        "  <encapsulation>\n"
        "    <component_ref component=\"main_model_component\">\n"
        "      <component_ref component=\"other_component\">\n"
        "        <component_ref component=\"reusable_child_1\"/>\n"
        "        <component_ref component=\"reusable_child_2\"/>\n"
        "      </component_ref>\n"
        "    </component_ref>\n"
        "  </encapsulation>\n"
        "</model>\n";

    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("modelflattening/outsidecomponentequivalentvariables.xml"));
    auto importer = libcellml::Importer::create();

    EXPECT_TRUE(model->hasUnresolvedImports());
    importer->resolveImports(model, resourcePath("modelflattening/"));
    EXPECT_FALSE(model->hasUnresolvedImports());

    model = importer->flattenModel(model);

    auto printer = libcellml::Printer::create();

    auto a = printer->printModel(model);
    EXPECT_EQ(e, a);
}

TEST(ModelFlattening, importedComponentUsingUnitsDefinedInImportedModel)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"main_model\">\n"
        "  <units name=\"fergie_time\">\n"
        "    <unit prefix=\"mega\" units=\"second\"/>\n"
        "  </units>\n"
        "  <component name=\"my_component\">\n"
        "    <variable name=\"time\" units=\"fergie_time\" interface=\"public_and_private\"/>\n"
        "  </component>\n"
        "</model>\n";

    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("modelflattening/unitsdefinedinimportedmodel.xml"));
    auto importer = libcellml::Importer::create();

    EXPECT_TRUE(model->hasUnresolvedImports());
    importer->resolveImports(model, resourcePath("modelflattening/"));
    EXPECT_FALSE(model->hasUnresolvedImports());

    model = importer->flattenModel(model);

    auto printer = libcellml::Printer::create();

    auto a = printer->printModel(model);
    EXPECT_EQ(e, a);
}

TEST(ModelFlattening, importedComponentUsingImportedComponent)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"main_model\">\n"
        "  <component name=\"my_component\">\n"
        "    <variable name=\"time\" units=\"dimensionless\" interface=\"public_and_private\"/>\n"
        "    <variable name=\"parameter\" units=\"dimensionless\" interface=\"public\"/>\n"
        "    <variable name=\"cosine\" units=\"dimensionless\" initial_value=\"0\" interface=\"public\"/>\n"
        "    <math xmlns=\"http://www.w3.org/1998/Math/MathML\">\n"
        "      <apply>\n"
        "        <eq/>\n"
        "        <apply>\n"
        "          <diff/>\n"
        "          <bvar>\n"
        "            <ci>time</ci>\n"
        "          </bvar>\n"
        "          <ci>cosine</ci>\n"
        "        </apply>\n"
        "        <apply>\n"
        "          <sin/>\n"
        "          <apply>\n"
        "            <times/>\n"
        "            <ci>parameter</ci>\n"
        "            <ci>time</ci>\n"
        "          </apply>\n"
        "        </apply>\n"
        "      </apply>\n"
        "    </math>\n"
        "  </component>\n"
        "</model>\n";

    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("modelflattening/importedcomponentusingimportedcomponent.xml"));
    auto importer = libcellml::Importer::create();

    EXPECT_TRUE(model->hasUnresolvedImports());

    importer->resolveImports(model, resourcePath("modelflattening/"));
    EXPECT_FALSE(model->hasUnresolvedImports());
    EXPECT_TRUE(model->hasImports());

    model = importer->flattenModel(model);
    EXPECT_FALSE(model->hasImports());

    auto printer = libcellml::Printer::create();

    auto a = printer->printModel(model);
    EXPECT_EQ(e, a);
}

TEST(ModelFlattening, repeatedImportOfSameUnitsViaDifferentComponents)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"main_model\">\n"
        "  <units name=\"fergie_time\">\n"
        "    <unit prefix=\"mega\" units=\"second\"/>\n"
        "  </units>\n"
        "  <component name=\"average_component\">\n"
        "    <variable name=\"time\" units=\"fergie_time\" interface=\"public_and_private\"/>\n"
        "  </component>\n"
        "  <component name=\"good_component\">\n"
        "    <variable name=\"sometime\" units=\"fergie_time\" interface=\"public_and_private\"/>\n"
        "  </component>\n"
        "</model>\n";

    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("modelflattening/repeatedimportofsameunitsviadifferentcomponents.xml"));
    auto importer = libcellml::Importer::create();

    EXPECT_TRUE(model->hasUnresolvedImports());
    importer->resolveImports(model, resourcePath("modelflattening/"));
    EXPECT_FALSE(model->hasUnresolvedImports());

    model = importer->flattenModel(model);

    auto printer = libcellml::Printer::create();

    auto a = printer->printModel(model);
    EXPECT_EQ(e, a);
}

TEST(ModelFlattening, importedUnitsWithNameClashes)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"main_model\">\n"
        "  <units name=\"common_units\">\n"
        "    <unit units=\"second\"/>\n"
        "  </units>\n"
        "  <units name=\"common_units_1\">\n"
        "    <unit prefix=\"hecto\" units=\"second\"/>\n"
        "  </units>\n"
        "  <component name=\"my_component\">\n"
        "    <variable name=\"v\" units=\"common_units_1\" interface=\"public_and_private\"/>\n"
        "  </component>\n"
        "  <component name=\"best_component\">\n"
        "    <variable name=\"v\" units=\"common_units\" initial_value=\"1\" interface=\"public_and_private\"/>\n"
        "  </component>\n"
        "</model>\n";

    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("modelflattening/importedunitswithnameclashes.xml"));
    auto importer = libcellml::Importer::create();

    EXPECT_TRUE(model->hasUnresolvedImports());
    importer->resolveImports(model, resourcePath("modelflattening/"));
    EXPECT_FALSE(model->hasUnresolvedImports());

    model = importer->flattenModel(model);

    auto printer = libcellml::Printer::create();

    auto a = printer->printModel(model);
    EXPECT_EQ(e, a);
}

TEST(ModelFlattening, importedComponentWithNameClashes)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"main_model\">\n"
        "  <units name=\"common_units\">\n"
        "    <unit units=\"second\"/>\n"
        "  </units>\n"
        "  <component name=\"my_component\"/>\n"
        "  <component name=\"best_component_1\"/>\n"
        "  <component name=\"best_component\">\n"
        "    <variable name=\"v\" units=\"common_units\" initial_value=\"1\" interface=\"public_and_private\"/>\n"
        "  </component>\n"
        "  <encapsulation>\n"
        "    <component_ref component=\"my_component\">\n"
        "      <component_ref component=\"best_component_1\"/>\n"
        "    </component_ref>\n"
        "  </encapsulation>\n"
        "</model>\n";

    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("modelflattening/importedcomponentswithnameclashes.xml"));
    auto importer = libcellml::Importer::create();

    EXPECT_TRUE(model->hasUnresolvedImports());
    importer->resolveImports(model, resourcePath("modelflattening/"));
    EXPECT_FALSE(model->hasUnresolvedImports());

    model = importer->flattenModel(model);

    auto printer = libcellml::Printer::create();

    auto a = printer->printModel(model);
    EXPECT_EQ(e, a);
}

TEST(ModelFlattening, importingComponentThatAlsoHasAnImportedComponentAsAChild)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"a_model\">\n"
        "  <component name=\"membrane\"/>\n"
        "  <component name=\"sodium_channel_1\"/>\n"
        "  <encapsulation>\n"
        "    <component_ref component=\"membrane\">\n"
        "      <component_ref component=\"sodium_channel_1\"/>\n"
        "    </component_ref>\n"
        "  </encapsulation>\n"
        "</model>\n";

    auto model = libcellml::Model::create("a_model");
    auto importer = libcellml::Importer::create();

    // Import the membrane component from a file.
    auto membrane = libcellml::Component::create("membrane");
    model->addComponent(membrane);

    auto membraneImporter = libcellml::ImportSource::create();
    membraneImporter->setUrl("basic_membrane_model.cellml");

    membrane->setImportSource(membraneImporter);
    membrane->setImportReference("membrane_for_importing");

    // Import the sodium channel from a file, and set as encapsulated child of the membrane.
    auto sodiumChannel = libcellml::Component::create("sodium_channel");
    membrane->addComponent(sodiumChannel);

    auto sodiumImporter = libcellml::ImportSource::create();
    sodiumImporter->setUrl("basic_sodium_channel.cellml");

    sodiumChannel->setImportSource(sodiumImporter);
    sodiumChannel->setImportReference("sodium_channel_for_importing");

    EXPECT_TRUE(model->hasUnresolvedImports());
    importer->resolveImports(model, resourcePath("modelflattening/"));
    EXPECT_FALSE(model->hasUnresolvedImports());

    model = importer->flattenModel(model);
    auto printer = libcellml::Printer::create();
    auto a = printer->printModel(model);
    EXPECT_EQ(e, a);
}

TEST(ModelFlattening, unitsUsedByVariableNotInDirectlyImportedComponent)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"a_model\">\n"
        "  <units name=\"per_ms\">\n"
        "    <unit exponent=\"-1\" prefix=\"milli\" units=\"second\"/>\n"
        "  </units>\n"
        "  <component name=\"imported_hierarchy\"/>\n"
        "  <component name=\"mGate\">\n"
        "    <variable name=\"alpha_m\" units=\"per_ms\"/>\n"
        "  </component>\n"
        "  <encapsulation>\n"
        "    <component_ref component=\"imported_hierarchy\">\n"
        "      <component_ref component=\"mGate\"/>\n"
        "    </component_ref>\n"
        "  </encapsulation>\n"
        "</model>\n";

    auto model = libcellml::Model::create("a_model");
    auto importer = libcellml::Importer::create();
    // Import the component hierarchy from a file
    auto channel = libcellml::Component::create("imported_hierarchy");
    model->addComponent(channel);

    auto channelImporter = libcellml::ImportSource::create();
    channelImporter->setUrl("imported_hierarchy_model.cellml");

    channel->setImportSource(channelImporter);
    channel->setImportReference("sodium_channel");

    EXPECT_TRUE(model->hasUnresolvedImports());
    importer->resolveImports(model, resourcePath("modelflattening/"));
    EXPECT_FALSE(model->hasUnresolvedImports());

    model = importer->flattenModel(model);

    auto printer = libcellml::Printer::create();

    auto a = printer->printModel(model);
    EXPECT_EQ(e, a);
}

// The variable ordering changes when using the HH model defined with imports.
TEST(ModelFlattening, hodgkinHuxleyDefinedUsingImports)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("modelflattening/hodgkin_huxley_squid_axon_model_1952/model.cellml"));
    auto importer = libcellml::Importer::create();

    EXPECT_TRUE(model->hasUnresolvedImports());
    importer->resolveImports(model, resourcePath("modelflattening/hodgkin_huxley_squid_axon_model_1952/"));
    EXPECT_FALSE(model->hasUnresolvedImports());

    model = importer->flattenModel(model);

    auto printer = libcellml::Printer::create();

    auto a = printer->printModel(model);
    auto modelNonImportVersion = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));
    auto e = printer->printModel(modelNonImportVersion);
    EXPECT_EQ(e, a);

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/model.h", generator->interfaceCode(analyserModel));
    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/model.c", generator->implementationCode(analyserModel));

    libcellml::GeneratorProfilePtr profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ("", generator->interfaceCode(analyserModel, profile));
    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/model.py", generator->implementationCode(analyserModel, profile));
}

TEST(ModelFlattening, importedComponentsWithConnectionsToChildren)
{
    const std::string modelConnectionsParentChild =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" xmlns:cellml=\"http://www.cellml.org/cellml/2.0#\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" name=\"units\">\n"
        "  <units name=\"ms\">\n"
        "    <unit units=\"second\" prefix=\"milli\"/>\n"
        "  </units>\n"
        "    <units name=\"per_ms\">\n"
        "    <unit exponent=\"-1\" prefix=\"milli\" units=\"second\"/>\n"
        "  </units>\n"
        "  <component name=\"gate\">\n"
        "    <variable name=\"t\" units=\"ms\" interface=\"public_and_private\"/>\n"
        "    <variable name=\"X\" units=\"dimensionless\" interface=\"public_and_private\"/>\n"
        "  </component>\n"
        "  <component name=\"gateEquations\">\n"
        "    <variable name=\"t\" units=\"ms\" interface=\"public\"/>\n"
        "    <variable name=\"alpha_X\" units=\"per_ms\" interface=\"public\"/>\n"
        "    <variable name=\"X\" units=\"dimensionless\" interface=\"public\"/>\n"
        "  </component>\n"
        "  <connection component_1=\"gate\" component_2=\"gateEquations\">\n"
        "    <map_variables variable_1=\"t\" variable_2=\"t\"/>\n"
        "    <map_variables variable_1=\"X\" variable_2=\"X\"/>\n"
        "  </connection>\n"
        "  <encapsulation>\n"
        "    <component_ref component=\"gate\">\n"
        "      <component_ref component=\"gateEquations\"/>\n"
        "    </component_ref>\n"
        "  </encapsulation>\n"
        "</model>\n";

    const std::string importingModel =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model name=\"importer_of_units\" xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <import xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"GateModel.cellml\">\n"
        "    <component component_ref=\"gateEquations\" name=\"importedGateH\"/>\n"
        "  </import>\n"
        "  <units name=\"mV\">\n"
        "    <unit prefix=\"milli\" units=\"volt\"/>\n"
        "  </units>\n"
        "  <component name=\"hGateEquations\">\n"
        "    <variable name=\"V\" units=\"mV\" interface=\"public\"/>\n"
        "  </component>\n"
        "  <encapsulation>\n"
        "    <component_ref component=\"hGateEquations\">\n"
        "      <component_ref component=\"importedGateH\"/>\n"
        "    </component_ref>\n"
        "  </encapsulation>\n"
        "</model>\n";

    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"importer_of_units\">\n"
        "  <units name=\"mV\">\n"
        "    <unit prefix=\"milli\" units=\"volt\"/>\n"
        "  </units>\n"
        "  <units name=\"ms\">\n"
        "    <unit prefix=\"milli\" units=\"second\"/>\n"
        "  </units>\n"
        "  <units name=\"per_ms\">\n"
        "    <unit exponent=\"-1\" prefix=\"milli\" units=\"second\"/>\n"
        "  </units>\n"
        "  <component name=\"hGateEquations\">\n"
        "    <variable name=\"V\" units=\"mV\" interface=\"public\"/>\n"
        "  </component>\n"
        "  <component name=\"importedGateH\">\n"
        "    <variable name=\"t\" units=\"ms\" interface=\"public\"/>\n"
        "    <variable name=\"alpha_X\" units=\"per_ms\" interface=\"public\"/>\n"
        "    <variable name=\"X\" units=\"dimensionless\" interface=\"public\"/>\n"
        "  </component>\n"
        "  <encapsulation>\n"
        "    <component_ref component=\"hGateEquations\">\n"
        "      <component_ref component=\"importedGateH\"/>\n"
        "    </component_ref>\n"
        "  </encapsulation>\n"
        "</model>\n";

    auto parser = libcellml::Parser::create();

    auto modelUsingImports = parser->parseModel(importingModel);
    auto modelComponentDefinition = parser->parseModel(modelConnectionsParentChild);

    EXPECT_TRUE(modelUsingImports->hasUnresolvedImports());

    auto component = modelUsingImports->component(0)->component(0);
    auto importSource = component->importSource();
    importSource->setModel(modelComponentDefinition);

    EXPECT_FALSE(modelUsingImports->hasUnresolvedImports());

    auto importer = libcellml::Importer::create();

    modelUsingImports = importer->flattenModel(modelUsingImports);

    auto printer = libcellml::Printer::create();

    auto a = printer->printModel(modelUsingImports);
    EXPECT_EQ(e, a);
}

TEST(ModelFlattening, resolveFlattenCircularImportsComponents)
{
    const std::string resolveError =
        std::string("Cyclic dependencies were found when attempting to resolve a component in the model 'importExample2b'. The dependency loop is:\n")
        + " - component 'sideB' specifies an import from ':this:' to '" + resourcePath("modelflattening/") + "circularImport1.cellml';\n"
        + " - component 'shared' specifies an import from '" + resourcePath("modelflattening/") + "circularImport1.cellml' to '" + resourcePath("modelflattening/") + "circularImport2.cellml'; and\n"
        + " - component 'circular2' specifies an import from '" + resourcePath("modelflattening/") + "circularImport2.cellml' to '" + resourcePath("modelflattening/") + "circularImport1.cellml'.";

    const std::string flattenError =
        std::string("Cyclic dependencies were found when attempting to flatten a component in the model 'importExample2b'. The dependency loop is:\n")
        + " - component 'sideB' specifies an import from ':this:' to '" + resourcePath("modelflattening/") + "circularImport1.cellml';\n"
        + " - component 'shared' specifies an import from '" + resourcePath("modelflattening/") + "circularImport1.cellml' to '" + resourcePath("modelflattening/") + "circularImport2.cellml'; and\n"
        + " - component 'circular2' specifies an import from '" + resourcePath("modelflattening/") + "circularImport2.cellml' to '" + resourcePath("modelflattening/") + "circularImport1.cellml'.";

    auto parser = libcellml::Parser::create();
    auto originalModel = parser->parseModel(fileContents("modelflattening/importExample2b.cellml"));
    auto importer = libcellml::Importer::create();

    // Resolve the imports.
    importer->resolveImports(originalModel, resourcePath("modelflattening/"));

    // Check for issues: expect one reporting the circular import.
    EXPECT_EQ(size_t(1), importer->issueCount());
    EXPECT_EQ(resolveError, importer->issue(0)->description());

    // Create a flattened version to demonstrate the diagnostics.
    auto flatModel = importer->flattenModel(originalModel);
    EXPECT_EQ(size_t(1), importer->issueCount());
    EXPECT_EQ(flattenError, importer->issue(0)->description());
    EXPECT_EQ(nullptr, flatModel);
}

TEST(ModelFlattening, resolveFlattenNonCircularImportsUnits)
{
    auto importer = libcellml::Importer::create();
    auto model = libcellml::Model::create("model");
    auto importModel = libcellml::Model::create("importModel");

    auto unitsWithImportedUnit = libcellml::Units::create("one_imported_unit");
    unitsWithImportedUnit->addUnit("metre");
    unitsWithImportedUnit->addUnit("importedUnit");

    auto fancyUnits = libcellml::Units::create("fancyUnits");
    fancyUnits->addUnit("per_becquerel");

    auto perBecquerel = libcellml::Units::create("per_becquerel");
    perBecquerel->addUnit(libcellml::Units::StandardUnit::BECQUEREL, -1.0);

    importModel->addUnits(fancyUnits);
    importModel->addUnits(perBecquerel);

    auto importSource = libcellml::ImportSource::create();
    importSource->setUrl("here.cellml");
    importSource->setModel(importModel);

    auto importedUnit = libcellml::Units::create("importedUnit");
    importedUnit->setImportReference("fancyUnits");
    importedUnit->setImportSource(importSource);

    model->addUnits(unitsWithImportedUnit);
    model->addUnits(importedUnit);

    EXPECT_FALSE(model->hasUnresolvedImports());

    auto flatModel = importer->flattenModel(model);
    EXPECT_EQ(size_t(0), importer->issueCount());
}

TEST(ModelFlattening, resolveFlattenCircularImportsUnits)
{
    const std::string resolveError =
        std::string("Cyclic dependencies were found when attempting to resolve units in the model 'importExampleUnits'. The dependency loop is:\n")
        + " - units 'sideB' specifies an import from ':this:' to '" + resourcePath("modelflattening/") + "circularImport1units.cellml';\n"
        + " - units 'shared' specifies an import from '" + resourcePath("modelflattening/") + "circularImport1units.cellml' to '" + resourcePath("modelflattening/") + "circularImport2units.cellml'; and\n"
        + " - units 'circular2' specifies an import from '" + resourcePath("modelflattening/") + "circularImport2units.cellml' to '" + resourcePath("modelflattening/") + "circularImport1units.cellml'.";

    const std::string flattenError =
        std::string("Cyclic dependencies were found when attempting to flatten units in the model 'importExampleUnits'. The dependency loop is:\n")
        + " - units 'sideB' specifies an import from ':this:' to '" + resourcePath("modelflattening/") + "circularImport1units.cellml';\n"
        + " - units 'shared' specifies an import from '" + resourcePath("modelflattening/") + "circularImport1units.cellml' to '" + resourcePath("modelflattening/") + "circularImport2units.cellml'; and\n"
        + " - units 'circular2' specifies an import from '" + resourcePath("modelflattening/") + "circularImport2units.cellml' to '" + resourcePath("modelflattening/") + "circularImport1units.cellml'.";

    auto parser = libcellml::Parser::create();
    auto originalModel = parser->parseModel(fileContents("modelflattening/importExampleUnits.cellml"));
    auto importer = libcellml::Importer::create();

    // Resolve the imports.
    importer->resolveImports(originalModel, resourcePath("modelflattening/"));

    // Check for issues: expect one reporting the circular import.
    EXPECT_EQ(size_t(1), importer->issueCount());
    EXPECT_EQ(resolveError, importer->issue(0)->description());

    // Create a flattened version to demonstrate the diagnostics.
    auto flatModel = importer->flattenModel(originalModel);
    EXPECT_EQ(size_t(1), importer->issueCount());
    EXPECT_EQ(flattenError, importer->issue(0)->description());
    EXPECT_EQ(nullptr, flatModel);
}

TEST(ModelFlattening, resolveFlattenMissingModel)
{
    const std::string expectedError = "Component 'left' requires a model imported from 'diamond_point.cellml' which is not available in the importer.";
    auto parser = libcellml::Parser::create();
    auto originalModel = parser->parseModel(fileContents("importer/diamond.cellml"));
    auto importer = libcellml::Importer::create();

    // Resolve imports.
    importer->resolveImports(originalModel, resourcePath("importer/"));
    EXPECT_EQ(size_t(0), importer->issueCount());

    // Corrupt the importer library by removing a model that's required.
    importer->replaceModel(nullptr, importer->key(1));

    // Attempt to flatten the model.
    auto flatModel = importer->flattenModel(originalModel);
    EXPECT_EQ(size_t(1), importer->issueCount());
    EXPECT_EQ(expectedError, importer->issue(0)->description());
}

TEST(ModelFlattening, resolveFlattenMissingComponent)
{
    const std::string e =
        "Component 'left' imports a component named 'pointyBit' from the model imported from '" + resourcePath("importer/") + "diamond_point.cellml'. The component could not be found.";
    auto parser = libcellml::Parser::create();
    auto originalModel = parser->parseModel(fileContents("importer/diamond.cellml"));
    auto importer = libcellml::Importer::create();

    // Resolve imports.
    importer->resolveImports(originalModel, resourcePath("importer/"));
    EXPECT_EQ(size_t(0), importer->issueCount());

    // Corrupt the importer library by removing a component from a model that's required.
    auto pointModel = importer->library(1);
    pointModel->removeComponent("pointyBit");

    // Attempt to flatten the model.
    auto flatModel = importer->flattenModel(originalModel);
    EXPECT_EQ(size_t(1), importer->issueCount());
    EXPECT_EQ(e, importer->issue(0)->description());
}

TEST(ModelFlattening, resolveFlattenMissingUnits)
{
    const std::string e =
        "Units 'units1_imported' imports units named 'units1' from the model imported from '" + resourcePath("importer/") + "units_source.cellml'. The units could not be found.";

    auto parser = libcellml::Parser::create();
    auto originalModel = parser->parseModel(fileContents("importer/units_imported.cellml"));
    auto importer = libcellml::Importer::create();

    // Resolve imports.
    importer->resolveImports(originalModel, resourcePath("importer/"));
    EXPECT_EQ(size_t(0), importer->issueCount());

    // Corrupt the importer library by renaming units from a model that's required.
    auto importedModel = importer->library(0);
    importedModel->units(0)->setName("someOtherName");

    // Attempt to flatten the model.
    auto flatModel = importer->flattenModel(originalModel);
    EXPECT_EQ(size_t(1), importer->issueCount());
    EXPECT_EQ(e, importer->issue(0)->description());
}

TEST(ModelFlattening, importSimilarEntitiesMultipleTimes)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"model\">\n"
        "  <units name=\"similar_units\">\n"
        "    <unit exponent=\"-1\" units=\"second\"/>\n"
        "  </units>\n"
        "  <units name=\"similar_units_1\">\n"
        "    <unit exponent=\"-2\" units=\"second\"/>\n"
        "  </units>\n"
        "  <units name=\"similar_units_2\">\n"
        "    <unit exponent=\"-0.5\" units=\"second\"/>\n"
        "  </units>\n"
        "  <component name=\"base_component\">\n"
        "    <variable name=\"variable_similar\" units=\"similar_units\"/>\n"
        "  </component>\n"
        "  <component name=\"component1\"/>\n"
        "  <component name=\"base_component_1\">\n"
        "    <variable name=\"variable_similar\" units=\"similar_units_1\"/>\n"
        "  </component>\n"
        "  <component name=\"component2\"/>\n"
        "  <component name=\"base_component_2\">\n"
        "    <variable name=\"variable_similar\" units=\"similar_units_2\"/>\n"
        "  </component>\n"
        "  <encapsulation>\n"
        "    <component_ref component=\"component1\">\n"
        "      <component_ref component=\"base_component_1\"/>\n"
        "    </component_ref>\n"
        "    <component_ref component=\"component2\">\n"
        "      <component_ref component=\"base_component_2\"/>\n"
        "    </component_ref>\n"
        "  </encapsulation>\n"
        "</model>\n";

    auto importer = libcellml::Importer::create();

    auto model = libcellml::Model::create("model");
    auto importModel1 = libcellml::Model::create("importModel1");
    auto importModel2 = libcellml::Model::create("importModel2");

    auto similarUnits = libcellml::Units::create("similar_units");
    similarUnits->addUnit("second", -1.0);

    auto similarUnits1 = libcellml::Units::create("similar_units");
    similarUnits1->addUnit("second", -2.0);

    auto similarUnits2 = libcellml::Units::create("similar_units");
    similarUnits2->addUnit("second", -0.5);

    auto component = libcellml::Component::create("base_component");
    auto variable = libcellml::Variable::create("variable_similar");
    variable->setUnits("similar_units");
    component->addVariable(variable);

    model->addUnits(similarUnits);
    model->addComponent(component);

    auto component1 = libcellml::Component::create("component_1");
    component1->addComponent(component->clone());
    importModel1->addUnits(similarUnits1);
    importModel1->addComponent(component1);
    importModel1->linkUnits();

    auto component2 = libcellml::Component::create("component_2");
    component2->addComponent(component->clone());
    importModel2->addUnits(similarUnits2);
    importModel2->addComponent(component2);
    importModel2->linkUnits();

    auto importedComponent1 = libcellml::Component::create("component1");
    auto importedComponent2 = libcellml::Component::create("component2");

    model->addComponent(importedComponent1);
    model->addComponent(importedComponent2);

    auto printer = libcellml::Printer::create();

    auto importSource1 = libcellml::ImportSource::create();
    importSource1->setUrl("model1.cellml");
    importSource1->setModel(importModel1);

    auto importSource2 = libcellml::ImportSource::create();
    importSource2->setUrl("model2.cellml");
    importSource2->setModel(importModel2);

    importedComponent1->setImportSource(importSource1);
    importedComponent1->setImportReference("component_1");

    importedComponent2->setImportSource(importSource2);
    importedComponent2->setImportReference("component_2");

    EXPECT_FALSE(model->hasUnresolvedImports());

    auto flatModel = importer->flattenModel(model);
    EXPECT_EQ(size_t(0), importer->issueCount());

    const std::string a = printer->printModel(flatModel);
    EXPECT_EQ(e, a);
}

TEST(ModelFlattening, proposedImportedUnitsAlreadyDefinedInModel)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"main_model\">\n"
        "  <units name=\"common_units\">\n"
        "    <unit units=\"second\"/>\n"
        "  </units>\n"
        "  <units name=\"common_units_1\">\n"
        "    <unit prefix=\"hecto\" units=\"second\"/>\n"
        "  </units>\n"
        "  <component name=\"my_component\">\n"
        "    <variable name=\"v\" units=\"common_units_1\" interface=\"public_and_private\"/>\n"
        "  </component>\n"
        "  <component name=\"best_component\">\n"
        "    <variable name=\"v\" units=\"common_units\" initial_value=\"1\" interface=\"public_and_private\"/>\n"
        "  </component>\n"
        "</model>\n";

    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("modelflattening/importedunitswithnameclashes.xml"));
    auto importer = libcellml::Importer::create();

    auto u = libcellml::Units::create("common_units_1");
    u->addUnit(libcellml::Units::StandardUnit::SECOND, libcellml::Units::Prefix::HECTO);
    model->addUnits(u);

    EXPECT_TRUE(model->hasUnresolvedImports());
    importer->resolveImports(model, resourcePath("modelflattening/"));
    EXPECT_FALSE(model->hasUnresolvedImports());

    model = importer->flattenModel(model);

    auto printer = libcellml::Printer::create();

    auto a = printer->printModel(model);
    EXPECT_EQ(e, a);
}

TEST(ModelFlattening, cascadedUnitsManuallyImported)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"model\">\n"
        "  <units name=\"first_units\">\n"
        "    <unit exponent=\"-1\" units=\"second\"/>\n"
        "    <unit units=\"units1_import\"/>\n"
        "  </units>\n"
        "  <units name=\"units1_import\">\n"
        "    <unit exponent=\"-2\" units=\"second\"/>\n"
        "    <unit units=\"units2_import\"/>\n"
        "  </units>\n"
        "  <units name=\"units2_import\">\n"
        "    <unit exponent=\"-0.5\" units=\"second\"/>\n"
        "  </units>\n"
        "  <component name=\"base_component\">\n"
        "    <variable name=\"variable\" units=\"first_units\"/>\n"
        "  </component>\n"
        "</model>\n";

    auto importer = libcellml::Importer::create();

    auto model = libcellml::Model::create("model");
    auto importModel1 = libcellml::Model::create("importModel1");
    auto importModel2 = libcellml::Model::create("importModel2");

    auto firstUnits = libcellml::Units::create("first_units");
    firstUnits->addUnit("second", -1.0);
    firstUnits->addUnit("units1_import");

    auto secondUnits = libcellml::Units::create("second_units");
    secondUnits->addUnit("second", -2.0);
    secondUnits->addUnit("units2_import");

    auto thirdUnits = libcellml::Units::create("third_units");
    thirdUnits->addUnit("second", -0.5);

    auto component1 = libcellml::Component::create("base_component");
    auto variable = libcellml::Variable::create("variable");
    variable->setUnits(firstUnits);
    component1->addVariable(variable);

    model->addUnits(firstUnits);
    model->addComponent(component1);

    auto importedUnits1 = libcellml::Units::create("units1_import");
    auto importedUnits2 = libcellml::Units::create("units2_import");

    model->addUnits(importedUnits1);
    importModel1->addUnits(secondUnits);
    importModel1->addUnits(importedUnits2);
    importModel2->addUnits(thirdUnits);

    auto printer = libcellml::Printer::create();

    auto importSource1 = libcellml::ImportSource::create();
    importSource1->setUrl("model1.cellml");
    importSource1->setModel(importModel1);

    auto importSource2 = libcellml::ImportSource::create();
    importSource2->setUrl("model2.cellml");
    importSource2->setModel(importModel2);

    importedUnits1->setImportSource(importSource1);
    importedUnits1->setImportReference("second_units");

    importedUnits2->setImportSource(importSource2);
    importedUnits2->setImportReference("third_units");

    EXPECT_FALSE(model->hasUnresolvedImports());

    importer->addModel(importModel1, "model1.cellml");
    importer->addModel(importModel2, "model2.cellml");

    auto flatModel = importer->flattenModel(model);
    EXPECT_EQ(size_t(0), importer->issueCount());

    const std::string a = printer->printModel(flatModel);
    EXPECT_EQ(e, a);
}

TEST(ModelFlattening, cascadedUnitsManuallyImportedWithoutImportSourcesAddedToImporter)
{
    const std::string e =
        "Cyclic dependencies were found when attempting to flatten units in the model 'model'. The dependency loop is:\n"
        " - units 'units1_import' specifies an import from ':this:' to ':this:'; and\n"
        " - units 'units2_import' specifies an import from ':this:' to ':this:'.";

    auto importer = libcellml::Importer::create();

    auto model = libcellml::Model::create("model");
    auto importModel1 = libcellml::Model::create("importModel1");
    auto importModel2 = libcellml::Model::create("importModel2");

    auto firstUnits = libcellml::Units::create("first_units");
    firstUnits->addUnit("second", -1.0);
    firstUnits->addUnit("units1_import");

    auto secondUnits = libcellml::Units::create("second_units");
    secondUnits->addUnit("second", -2.0);
    secondUnits->addUnit("units2_import");

    auto thirdUnits = libcellml::Units::create("third_units");
    thirdUnits->addUnit("second", -0.5);

    auto component1 = libcellml::Component::create("base_component");
    auto variable = libcellml::Variable::create("variable");
    variable->setUnits(firstUnits);
    component1->addVariable(variable);

    model->addUnits(firstUnits);
    model->addComponent(component1);

    auto importedUnits1 = libcellml::Units::create("units1_import");
    auto importedUnits2 = libcellml::Units::create("units2_import");

    model->addUnits(importedUnits1);
    importModel1->addUnits(secondUnits);
    importModel1->addUnits(importedUnits2);
    importModel2->addUnits(thirdUnits);

    auto printer = libcellml::Printer::create();

    auto importSource1 = libcellml::ImportSource::create();
    importSource1->setUrl("model1.cellml");
    importSource1->setModel(importModel1);

    auto importSource2 = libcellml::ImportSource::create();
    importSource2->setUrl("model2.cellml");
    importSource2->setModel(importModel2);

    importedUnits1->setImportSource(importSource1);
    importedUnits1->setImportReference("second_units");

    importedUnits2->setImportSource(importSource2);
    importedUnits2->setImportReference("third_units");

    EXPECT_FALSE(model->hasUnresolvedImports());

    auto flatModel = importer->flattenModel(model);
    EXPECT_EQ(size_t(1), importer->issueCount());
    EXPECT_EQ(e, importer->issue(0)->description());
}

TEST(ModelFlattening, cascadedUnitsManuallyImportedMissingUnitReferences)
{
    const std::string e = "The model is not fully defined.";

    auto importer = libcellml::Importer::create();

    auto model = libcellml::Model::create("model");
    auto importModel1 = libcellml::Model::create("importModel1");
    auto importModel2 = libcellml::Model::create("importModel2");

    auto firstUnits = libcellml::Units::create("first_units");
    firstUnits->addUnit("second", -1.0);
    firstUnits->addUnit("units1_import");

    auto secondUnits = libcellml::Units::create("second_units");
    secondUnits->addUnit("second", -2.0);
    secondUnits->addUnit("units2_import");

    auto thirdUnits = libcellml::Units::create("third_units");
    thirdUnits->addUnit("second", -0.5);
    thirdUnits->addUnit("");
    thirdUnits->addUnit("missing_units");

    auto component1 = libcellml::Component::create("base_component");
    auto variable = libcellml::Variable::create("variable");
    variable->setUnits(firstUnits);
    component1->addVariable(variable);

    model->addUnits(firstUnits);
    model->addComponent(component1);

    auto importedUnits1 = libcellml::Units::create("units1_import");
    auto importedUnits2 = libcellml::Units::create("units2_import");

    model->addUnits(importedUnits1);
    importModel1->addUnits(secondUnits);
    importModel1->addUnits(importedUnits2);
    importModel2->addUnits(thirdUnits);

    auto printer = libcellml::Printer::create();

    auto importSource1 = libcellml::ImportSource::create();
    importSource1->setUrl("model1.cellml");
    importSource1->setModel(importModel1);

    auto importSource2 = libcellml::ImportSource::create();
    importSource2->setUrl("model2.cellml");
    importSource2->setModel(importModel2);

    importedUnits1->setImportSource(importSource1);
    importedUnits1->setImportReference("second_units");

    importedUnits2->setImportSource(importSource2);
    importedUnits2->setImportReference("third_units");

    EXPECT_FALSE(model->hasUnresolvedImports());
    EXPECT_FALSE(model->isDefined());

    importer->addModel(importModel1, "model1.cellml");
    importer->addModel(importModel2, "model2.cellml");

    auto flatModel = importer->flattenModel(model);
    EXPECT_EQ(size_t(1), importer->issueCount());
    EXPECT_EQ(e, importer->issue(0)->description());
}

TEST(ModelFlattening, importCascadingUnitsImports)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"model\">\n"
        "  <units name=\"units1_import\">\n"
        "    <unit exponent=\"-2\" units=\"second\"/>\n"
        "    <unit units=\"units2_import\"/>\n"
        "  </units>\n"
        "  <units name=\"first_units\">\n"
        "    <unit exponent=\"-1\" units=\"second\"/>\n"
        "    <unit units=\"units1_import\"/>\n"
        "  </units>\n"
        "  <units name=\"units2_import\">\n"
        "    <unit exponent=\"-0.5\" units=\"second\"/>\n"
        "  </units>\n"
        "  <component name=\"base_component\">\n"
        "    <variable name=\"variable\" units=\"first_units\"/>\n"
        "  </component>\n"
        "</model>\n";

    auto importer = libcellml::Importer::create();
    auto parser = libcellml::Parser::create();

    auto model = parser->parseModel(fileContents("importer/model_cascaded_units.cellml"));

    importer->resolveImports(model, resourcePath("importer"));
    EXPECT_EQ(size_t(0), importer->errorCount());

    auto flatModel = importer->flattenModel(model);
    EXPECT_EQ(size_t(0), importer->issueCount());

    auto printer = libcellml::Printer::create();
    const std::string a = printer->printModel(flatModel);
    EXPECT_EQ(e, a);
}

TEST(ModelFlattening, importingUnitsWithSameNameDoesntResultInRepeatedUnits)
{
    auto importer = libcellml::Importer::create();
    auto parser = libcellml::Parser::create();
    auto validator = libcellml::Validator::create();

    auto model = parser->parseModel(fileContents("importer/triangle_units_point_I.cellml"));

    EXPECT_EQ(size_t(0), importer->errorCount());

    importer->resolveImports(model, resourcePath("importer"));

    for (size_t i = 0; i < importer->libraryCount(); ++i) {
        validator->validateModel(importer->library(i));
        EXPECT_EQ(size_t(0), validator->errorCount());
    }

    auto flattenedModel = importer->flattenModel(model);

    EXPECT_EQ(size_t(1), flattenedModel->unitsCount());
    EXPECT_EQ("mm", flattenedModel->units(0)->name());
}

TEST(ModelFlattening, importingAliasedUnits)
{
    const std::string importModelString =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"change_model\">\n"
        "  <units name=\"alias_mm\">\n"
        "    <unit units=\"mm\"/>\n"
        "  </units>\n"
        "  <units name=\"alias_mim\">\n"
        "    <unit units=\"mim\"/>\n"
        "  </units>\n"
        "  <units name=\"mm\">\n"
        "    <unit prefix=\"milli\" units=\"second\"/>\n"
        "  </units>\n"
        "  <units name=\"mim\">\n"
        "    <unit prefix=\"micro\" units=\"second\"/>\n"
        "  </units>\n"
        "  <component name=\"change\">\n"
        "    <variable name=\"var1\" units=\"alias_mm\"/>\n"
        "    <variable name=\"var2\" units=\"alias_mim\"/>\n"
        "    <variable name=\"var3\" units=\"mm\"/>\n"
        "    <variable name=\"var4\" units=\"mim\"/>\n"
        "  </component>\n"
        "</model>";
    const std::string modelString =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"multiple_clash\">\n"
        "  <import xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"change_model.cellml\">\n"
        "    <component component_ref=\"change\" name=\"change\"/>\n"
        "  </import>\n"
        "  <units name=\"mm\">\n"
        "    <unit prefix=\"milli\" units=\"metre\"/>\n"
        "  </units>\n"
        "  <units name=\"mim\">\n"
        "    <unit prefix=\"micro\" units=\"metre\"/>\n"
        "  </units>\n"
        "  <component name=\"opposite\">\n"
        "    <variable name=\"var1\" units=\"mm\"/>\n"
        "    <variable name=\"var2\" units=\"mim\"/>\n"
        "  </component>\n"
        "</model>";
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"multiple_clash\">\n"
        "  <units name=\"mm\">\n"
        "    <unit prefix=\"milli\" units=\"metre\"/>\n"
        "  </units>\n"
        "  <units name=\"mim\">\n"
        "    <unit prefix=\"micro\" units=\"metre\"/>\n"
        "  </units>\n"
        "  <units name=\"mm_1\">\n"
        "    <unit prefix=\"milli\" units=\"second\"/>\n"
        "  </units>\n"
        "  <units name=\"mim_1\">\n"
        "    <unit prefix=\"micro\" units=\"second\"/>\n"
        "  </units>\n"
        "  <component name=\"change\">\n"
        "    <variable name=\"var1\" units=\"mm_1\"/>\n"
        "    <variable name=\"var2\" units=\"mim_1\"/>\n"
        "    <variable name=\"var3\" units=\"mm_1\"/>\n"
        "    <variable name=\"var4\" units=\"mim_1\"/>\n"
        "  </component>\n"
        "  <component name=\"opposite\">\n"
        "    <variable name=\"var1\" units=\"mm\"/>\n"
        "    <variable name=\"var2\" units=\"mim\"/>\n"
        "  </component>\n"
        "</model>\n";

    auto importer = libcellml::Importer::create();
    auto parser = libcellml::Parser::create();
    auto validator = libcellml::Validator::create();

    auto model = parser->parseModel(modelString);
    auto importModel = parser->parseModel(importModelString);

    EXPECT_TRUE(model->hasUnresolvedImports());

    importer->addModel(importModel, "change_model.cellml");

    importer->resolveImports(model, ".");
    auto flattenedModel = importer->flattenModel(model);
    EXPECT_EQ(size_t(4), flattenedModel->unitsCount());

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(flattenedModel);
    EXPECT_EQ(e, a);
}

TEST(ModelFlattening, importingAliasedUnitsWithoutReplacing)
{
    const std::string importModelString =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"change_model\">\n"
        "  <units name=\"alias_mm\">\n"
        "    <unit units=\"mm\"/>\n"
        "  </units>\n"
        "  <units name=\"mm\">\n"
        "    <unit prefix=\"milli\" units=\"metre\"/>\n"
        "  </units>\n"
        "  <component name=\"change\">\n"
        "    <variable name=\"var1\" units=\"alias_mm\"/>\n"
        "    <variable name=\"var3\" units=\"mm\"/>\n"
        "  </component>\n"
        "</model>";
    const std::string modelString =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"multiple_clash\">\n"
        "  <import xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"change_model.cellml\">\n"
        "    <component component_ref=\"change\" name=\"change\"/>\n"
        "  </import>\n"
        "  <units name=\"mm\">\n"
        "    <unit prefix=\"milli\" units=\"metre\"/>\n"
        "  </units>\n"
        "</model>";
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"multiple_clash\">\n"
        "  <units name=\"mm\">\n"
        "    <unit prefix=\"milli\" units=\"metre\"/>\n"
        "  </units>\n"
        "  <component name=\"change\">\n"
        "    <variable name=\"var1\" units=\"mm\"/>\n"
        "    <variable name=\"var3\" units=\"mm\"/>\n"
        "  </component>\n"
        "</model>\n";

    auto importer = libcellml::Importer::create();
    auto parser = libcellml::Parser::create();
    auto validator = libcellml::Validator::create();

    auto model = parser->parseModel(modelString);
    auto importModel = parser->parseModel(importModelString);

    EXPECT_TRUE(model->hasUnresolvedImports());

    importer->addModel(importModel, "change_model.cellml");

    importer->resolveImports(model, ".");
    auto flattenedModel = importer->flattenModel(model);
    EXPECT_EQ(size_t(1), flattenedModel->unitsCount());

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(flattenedModel);
    EXPECT_EQ(e, a);
}

TEST(ModelFlattening, importingComponentUsingEmptyUnitReference)
{
    const std::string unitsDefinitionModelString =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"units_definitions\">\n"
        "  <units name=\"empty_ref\">\n"
        "    <unit units=\"\"/>\n"
        "  </units>\n"
        "  <units name=\"\">\n"
        "    <unit units=\"second\"/>\n"
        "  </units>\n"
        "</model>";
    const std::string importModelString =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"change_model\">\n"
        "  <import xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"units_definitions.cellml\">\n"
        "    <units units_ref=\"empty_ref\" name=\"empty_ref\"/>\n"
        "  </import>\n"
        "  <component name=\"change\">\n"
        "    <variable name=\"var1\" units=\"empty_ref\"/>\n"
        "  </component>\n"
        "</model>";
    const std::string modelString =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"multiple_clash\">\n"
        "  <import xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"change_model.cellml\">\n"
        "    <component component_ref=\"change\" name=\"change\"/>\n"
        "  </import>\n"
        "</model>";
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"multiple_clash\">\n"
        "  <units name=\"empty_ref\">\n"
        "    <unit units=\"\"/>\n"
        "  </units>\n"
        "  <component name=\"change\">\n"
        "    <variable name=\"var1\" units=\"empty_ref\"/>\n"
        "  </component>\n"
        "</model>\n";

    auto importer = libcellml::Importer::create();
    auto parser = libcellml::Parser::create();
    auto validator = libcellml::Validator::create();

    auto model = parser->parseModel(modelString);
    auto importModel = parser->parseModel(importModelString);
    auto unitsDefinitionModel = parser->parseModel(unitsDefinitionModelString);

    EXPECT_TRUE(model->hasUnresolvedImports());

    importer->addModel(importModel, "change_model.cellml");
    importer->addModel(unitsDefinitionModel, "units_definitions.cellml");

    importer->resolveImports(model, ".");
    auto flattenedModel = importer->flattenModel(model);
    EXPECT_EQ(size_t(1), flattenedModel->unitsCount());

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(flattenedModel);
    EXPECT_EQ(e, a);
}

TEST(ModelFlattening, importedComponentWithUnresolvedUnitsImport)
{
    const std::string unitsDefinitionsModelString =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"units_definitions\">\n"
        "  <units name=\"more_mm\">\n"
        "    <unit prefix=\"milli\" units=\"metre\"/>\n"
        "  </units>\n"
        "</model>";
    const std::string importModelString =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"base\">\n"
        "  <import xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"units_definitions.cellml\">\n"
        "    <units units_ref=\"more_mm\" name=\"more_mm\"/>\n"
        "  </import>\n"
        "  <units name=\"mm_sq\">\n"
        "    <unit units=\"mm\"/>\n"
        "    <unit units=\"more_mm\"/>\n"
        "  </units>\n"
        "  <units name=\"mm\">\n"
        "    <unit prefix=\"milli\" units=\"metre\"/>\n"
        "  </units>\n"
        "  <component name=\"base\">\n"
        "    <variable name=\"var1\" units=\"mm_sq\"/>\n"
        "  </component>\n"
        "</model>";
    const std::string modelString =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"multiple_clash\">\n"
        "  <import xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"base.cellml\">\n"
        "    <component component_ref=\"base\" name=\"base\"/>\n"
        "  </import>\n"
        "</model>";

    auto importer = libcellml::Importer::create();
    auto parser = libcellml::Parser::create();
    auto validator = libcellml::Validator::create();

    auto model = parser->parseModel(modelString);
    auto importModel = parser->parseModel(importModelString);
    auto unitsDefinitionsModel = parser->parseModel(unitsDefinitionsModelString);

    importer->addModel(importModel, "base.cellml");
    importer->addModel(unitsDefinitionsModel, "units_definitions.cellml");

    importer->resolveImports(model, ".");
    EXPECT_TRUE(model->hasUnresolvedImports());

    auto flattenedModel = importer->flattenModel(model);
    EXPECT_EQ(nullptr, flattenedModel);
}

TEST(ModelFlattening, importedUnitsUsingUnitWhichIsAnImportedUnits)
{
    const std::string unitsDefinitionsModelString =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"units_definitions\">\n"
        "  <units name=\"more_mm\">\n"
        "    <unit prefix=\"milli\" units=\"second\"/>\n"
        "  </units>\n"
        "</model>";
    const std::string importModelString =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"base\">\n"
        "  <import xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"units_definitions.cellml\">\n"
        "    <units units_ref=\"more_mm\" name=\"more_mm\"/>\n"
        "  </import>\n"
        "  <units name=\"mm_sq\">\n"
        "    <unit units=\"mm\"/>\n"
        "    <unit units=\"more_mm\"/>\n"
        "  </units>\n"
        "  <units name=\"mm\">\n"
        "    <unit prefix=\"milli\" units=\"second\"/>\n"
        "  </units>\n"
        "  <component name=\"base\">\n"
        "    <variable name=\"var1\" units=\"mm_sq\"/>\n"
        "  </component>\n"
        "</model>";
    const std::string modelString =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"multiple_clash\">\n"
        "  <import xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"base.cellml\">\n"
        "    <component component_ref=\"base\" name=\"base\"/>\n"
        "  </import>\n"
        "</model>";
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"multiple_clash\">\n"
        "  <units name=\"mm\">\n"
        "    <unit prefix=\"milli\" units=\"metre\"/>\n"
        "  </units>\n"
        "  <units name=\"mm_sq\">\n"
        "    <unit units=\"mm\"/>\n"
        "    <unit units=\"mm\"/>\n"
        "  </units>\n"
        "  <component name=\"base\">\n"
        "    <variable name=\"var1\" units=\"mm_sq\"/>\n"
        "  </component>\n"
        "</model>\n";

    auto importer = libcellml::Importer::create();
    auto parser = libcellml::Parser::create();
    auto validator = libcellml::Validator::create();

    auto model = parser->parseModel(modelString);
    auto importModel = parser->parseModel(importModelString);
    auto unitsDefinitionsModel = parser->parseModel(unitsDefinitionsModelString);

    importer->addModel(importModel, "base.cellml");
    importer->addModel(unitsDefinitionsModel, "units_definitions.cellml");

    importer->resolveImports(model, ".");
    EXPECT_TRUE(model->hasUnresolvedImports());

    auto flattenedModel = importer->flattenModel(model);
    EXPECT_EQ(nullptr, flattenedModel);
}

TEST(ModelFlattening, flatteningSimpleBondGraph)
{
    auto importer = libcellml::Importer::create(false);
    auto parser = libcellml::Parser::create(false);

    auto model = parser->parseModel(fileContents("importer/simplebondgraph/cpp_coupling.cellml"));
    importer->resolveImports(model, resourcePath("importer/simplebondgraph"));

    auto flatModel = importer->flattenModel(model);
    EXPECT_EQ("C_main_vessel", flatModel->component(0)->variable(0)->name());
    EXPECT_EQ(size_t(1), flatModel->component(0)->variable(0)->equivalentVariableCount());
    EXPECT_EQ("C", flatModel->component(1)->variable(0)->name());
    EXPECT_EQ(size_t(1), flatModel->component(1)->variable(0)->equivalentVariableCount());
}

TEST(ModelFlattening, flatteningMediumBondGraph)
{
    auto importer = libcellml::Importer::create(false);
    auto parser = libcellml::Parser::create(false);

    auto model = parser->parseModel(fileContents("importer/mediumbondgraph/cpp_coupling.cellml"));
    importer->resolveImports(model, resourcePath("importer/mediumbondgraph"));

    auto flatModel = importer->flattenModel(model);
    EXPECT_EQ("C_main_vessel", flatModel->component(0)->variable(0)->name());
    EXPECT_EQ(size_t(1), flatModel->component(0)->variable(0)->equivalentVariableCount());
    EXPECT_EQ("C", flatModel->component(2)->variable(1)->name());
    EXPECT_EQ(size_t(2), flatModel->component(2)->variable(1)->equivalentVariableCount());
}

TEST(ModelFlattening, flatteningComplexBondGraph)
{
    auto importer = libcellml::Importer::create(false);
    auto parser = libcellml::Parser::create(false);

    auto model = parser->parseModel(fileContents("importer/complexbondgraph/cpp_coupling.cellml"));
    importer->resolveImports(model, resourcePath("importer/complexbondgraph"));

    auto flatModel = importer->flattenModel(model);
    EXPECT_EQ("C_main_vessel", flatModel->component(0)->variable(1)->name());
    EXPECT_EQ(size_t(1), flatModel->component(0)->variable(0)->equivalentVariableCount());
    EXPECT_EQ("C", flatModel->component(2)->variable(2)->name());
    EXPECT_EQ(size_t(2), flatModel->component(2)->variable(2)->equivalentVariableCount());
    EXPECT_EQ("C", flatModel->component(3)->variable(4)->name());
    EXPECT_EQ(size_t(1), flatModel->component(3)->variable(4)->equivalentVariableCount());
}

TEST(ModelFlattening, flatteningCheckImportModelsForChanges)
{
    const std::string eParent =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"InitializedVariables_model\">\n"
        "  <import xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"child.cellml\">\n"
        "    <component component_ref=\"not_main\" name=\"child_main\"/>\n"
        "  </import>\n"
        "  <component name=\"main\">\n"
        "    <variable name=\"kComminuteOralMin\" units=\"dimensionless\" interface=\"public\"/>\n"
        "  </component>\n"
        "  <connection component_1=\"child_main\" component_2=\"main\">\n"
        "    <map_variables variable_1=\"kComminuteOralMin\" variable_2=\"kComminuteOralMin\"/>\n"
        "  </connection>\n"
        "</model>\n";
    const std::string eChild =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"AnimalConstants_model\">\n"
        "  <units name=\"perS\">\n"
        "    <unit exponent=\"-1\" units=\"second\"/>\n"
        "  </units>\n"
        "  <component name=\"not_main\">\n"
        "    <variable name=\"kComminuteOralMax\" units=\"dimensionless\" initial_value=\"0.934\" interface=\"public\"/>\n"
        "    <variable name=\"TAveabsE\" units=\"perS\" initial_value=\"0.05\" interface=\"public\"/>\n"
        "  </component>\n"
        "</model>\n";

    auto importer = libcellml::Importer::create(false);
    auto parser = libcellml::Parser::create(false);
    auto printer = libcellml::Printer::create();

    auto model = parser->parseModel(fileContents("modelflattening/user_ex_01/parent.cellml"));
    importer->resolveImports(model, resourcePath("modelflattening/user_ex_01"));

    auto importedModel = importer->library(0);

    EXPECT_EQ(eChild, printer->printModel(importedModel));
    EXPECT_EQ(eParent, printer->printModel(model));

    auto flatModel = importer->flattenModel(model);

    EXPECT_EQ(eChild, printer->printModel(importedModel));
    EXPECT_EQ(eParent, printer->printModel(model));
}

TEST(ModelFlattening, flatteningModelsRequiringUnitsNameChangesInvolvingImportedComponents)
{
    auto importer = libcellml::Importer::create();
    auto parser = libcellml::Parser::create();

    auto model = parser->parseModel(fileContents("importer/HHComplete/MembraneModel.cellml"));
    importer->resolveImports(model, resourcePath("importer/HHComplete"));

    EXPECT_FALSE(model->hasUnresolvedImports());

    auto flatModel = importer->flattenModel(model);
    EXPECT_NE(nullptr, flatModel);
}

TEST(ModelFlattening, resolveImportsInvalidInput)
{
    auto importer = libcellml::Importer::create();

    libcellml::ModelPtr model = nullptr;

    EXPECT_FALSE(importer->resolveImports(model, "not/used"));
    EXPECT_EQ(size_t(1), importer->issueCount());
    EXPECT_EQ("Cannot resolve imports for null model.", importer->issue(0)->description());
}

TEST(ModelFlattening, flatteningImportedUnitsThatNeedEquivalence)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"test_model\">\n"
        "  <units name=\"per_fmol\">\n"
        "    <unit exponent=\"-1\" units=\"fmol\"/>\n"
        "  </units>\n"
        "  <units name=\"fmol\">\n"
        "    <unit prefix=\"femto\" units=\"mole\"/>\n"
        "  </units>\n"
        "  <units name=\"per_fmol_sec4\">\n"
        "    <unit units=\"per_fmol\"/>\n"
        "    <unit exponent=\"4\" units=\"per_sec\"/>\n"
        "  </units>\n"
        "  <units name=\"per_sec\">\n"
        "    <unit exponent=\"-1\" units=\"second\"/>\n"
        "  </units>\n"
        "</model>\n";

    auto parser = libcellml::Parser::create();
    auto importer = libcellml::Importer::create();

    auto model = parser->parseModel(fileContents("generator/cellml_slc_example/slc_model_units_only.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    importer->resolveImports(model, resourcePath("generator/cellml_slc_example"));
    EXPECT_FALSE(model->hasUnresolvedImports());

    auto flatModel = importer->flattenModel(model);

    auto printer = libcellml::Printer::create();
    EXPECT_EQ(e, printer->printModel(flatModel));
}

TEST(ModelFlattening, modelWithCnUnitsNotDefinedInImportedComponent)
{
    auto parser = libcellml::Parser::create(false);
    auto model = parser->parseModel(fileContents("modelflattening/SN_to_cAMP/SN_to_cAMP.cellml"));
    auto importer = libcellml::Importer::create(false);

    EXPECT_EQ(size_t(0), parser->errorCount());
    EXPECT_TRUE(model->hasUnresolvedImports());

    importer->resolveImports(model, resourcePath("modelflattening/SN_to_cAMP"));

    EXPECT_FALSE(model->hasUnresolvedImports());

    model = importer->flattenModel(model);

    EXPECT_EQ(size_t(1), importer->errorCount());
    EXPECT_EQ("The model is not fully defined.", importer->error(0)->description());
}

TEST(ModelFlattening, multiLayeredImportOfNonStandardUnits)
{
    auto parser = libcellml::Parser::create(false);
    auto model = parser->parseModel(fileContents("importer/periodicstimulus/experiments/periodic-stimulus.xml"));

    EXPECT_EQ(size_t(0), parser->errorCount());

    auto validator = libcellml::Validator::create();

    validator->validateModel(model);

    EXPECT_EQ(size_t(0), validator->issueCount());
    EXPECT_TRUE(model->hasUnresolvedImports());

    auto importer = libcellml::Importer::create(false);

    importer->resolveImports(model, resourcePath("importer/periodicstimulus/experiments"));

    EXPECT_FALSE(model->hasUnresolvedImports());

    auto flattenModel = importer->flattenModel(model);

    EXPECT_NE(nullptr, flattenModel);
}
