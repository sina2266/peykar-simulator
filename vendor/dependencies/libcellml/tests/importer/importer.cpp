/*
Copyright libCellML Contributors

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License->
You may obtain a copy of the License at

    http://www.apache->org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License->
*/

#include "gtest/gtest.h"

#include <libcellml>

#include "test_utils.h"

TEST(Importer, create)
{
    auto importer = libcellml::Importer::create();
    EXPECT_NE(nullptr, importer);
}

TEST(Importer, noWarningForkedImport)
{
    libcellml::ParserPtr p = libcellml::Parser::create();
    libcellml::ImporterPtr importer = libcellml::Importer::create();
    libcellml::ModelPtr model = p->parseModel(fileContents("importer/forkedImport.cellml"));

    EXPECT_EQ(size_t(0), p->errorCount());

    EXPECT_TRUE(model->hasUnresolvedImports());
    importer->resolveImports(model, resourcePath("importer/"));
    EXPECT_FALSE(model->hasUnresolvedImports());
}

TEST(Importer, noWarningDiamondImport)
{
    libcellml::ParserPtr p = libcellml::Parser::create();
    libcellml::ImporterPtr importer = libcellml::Importer::create();
    libcellml::ModelPtr model = p->parseModel(fileContents("importer/diamond.cellml"));

    EXPECT_EQ(size_t(0), p->errorCount());

    EXPECT_TRUE(model->hasUnresolvedImports());
    importer->resolveImports(model, resourcePath("importer/"));
    EXPECT_FALSE(model->hasUnresolvedImports());
    EXPECT_EQ(size_t(0), importer->errorCount());
}

TEST(Importer, warningCircularImportReferencesComponent)
{
    const std::string errorMessage =
        std::string("Cyclic dependencies were found when attempting to resolve a component in the model 'circularImport1'. The dependency loop is:\n")
        + " - component 'i_am_cyclic' specifies an import from ':this:' to '" + resourcePath("importer/") + "circularImport_2.cellml';\n"
        + " - component 'c2' specifies an import from '" + resourcePath("importer/") + "circularImport_2.cellml' to '" + resourcePath("importer/") + "circularImport_3.cellml'; and\n"
        + " - component 'c3' specifies an import from '" + resourcePath("importer/") + "circularImport_3.cellml' to '" + resourcePath("importer/") + "circularImport_1.cellml'.";

    auto parser = libcellml::Parser::create();
    auto importer = libcellml::Importer::create();
    auto model = parser->parseModel(fileContents("importer/circularImport_1.cellml"));
    EXPECT_EQ(size_t(0), parser->issueCount());
    importer->resolveImports(model, resourcePath("importer/"));

    EXPECT_EQ(size_t(1), importer->issueCount());
    EXPECT_EQ(size_t(1), importer->errorCount());
    EXPECT_EQ(errorMessage, importer->error(0)->description());
}

TEST(Importer, warningCircularImportReferencesUnits)
{
    const std::string errorMessage =
        std::string("Cyclic dependencies were found when attempting to resolve units in the model 'circularImport1'. The dependency loop is:\n")
        + " - units 'i_am_cyclic' specifies an import from ':this:' to '" + resourcePath("importer/") + "circularUnits_2.cellml';\n"
        + " - units 'u2' specifies an import from '" + resourcePath("importer/") + "circularUnits_2.cellml' to '" + resourcePath("importer/") + "circularUnits_3.cellml'; and\n"
        + " - units 'u3' specifies an import from '" + resourcePath("importer/") + "circularUnits_3.cellml' to '" + resourcePath("importer/") + "circularUnits_1.cellml'.";

    auto parser = libcellml::Parser::create();
    auto importer = libcellml::Importer::create();
    auto model = parser->parseModel(fileContents("importer/circularUnits_1.cellml"));
    EXPECT_EQ(size_t(0), parser->issueCount());
    importer->resolveImports(model, resourcePath("importer/"));
    EXPECT_EQ(size_t(1), importer->issueCount());
    EXPECT_EQ(size_t(1), importer->errorCount());
    EXPECT_EQ(errorMessage, importer->error(0)->description());
}

TEST(Importer, warningUnrequiredCircularDependencyComponent)
{
    auto parser = libcellml::Parser::create();
    auto importer = libcellml::Importer::create();
    auto model = parser->parseModel(fileContents("importer/master1.cellml"));
    EXPECT_EQ(size_t(0), parser->issueCount());
    importer->resolveImports(model, resourcePath("importer/"));
    EXPECT_FALSE(model->hasUnresolvedImports());

    // Expect no issues, as the circular component dependency is not instantiated.
    EXPECT_EQ(size_t(0), importer->issueCount());
}

TEST(Importer, warningUnrequiredCircularDependencyUnits)
{
    auto parser = libcellml::Parser::create();
    auto importer = libcellml::Importer::create();
    auto model = parser->parseModel(fileContents("importer/master2.cellml"));
    EXPECT_EQ(size_t(0), parser->issueCount());
    importer->resolveImports(model, resourcePath("importer/"));
    EXPECT_FALSE(model->hasUnresolvedImports());

    // Expect no issues as the circular dependency is not instantiated.
    EXPECT_EQ(size_t(0), importer->issueCount());
}

TEST(Importer, missingUnitsFromImportOfCnTerms)
{
    // This test is intended to show that parsing a model and importing
    // it have the same end result.
    auto validator = libcellml::Validator::create();
    auto model = libcellml::Model::create("model_from_imports");
    auto c = libcellml::Component::create("c");
    auto importer = libcellml::Importer::create();

    auto imp = libcellml::ImportSource::create();
    imp->setUrl("units_in_cn.cellml");

    c->setImportReference("myComponent");
    c->setImportSource(imp);
    model->addComponent(c);

    EXPECT_TRUE(model->hasUnresolvedImports());

    importer->resolveImports(model, resourcePath());
    EXPECT_FALSE(model->hasUnresolvedImports());
    model = importer->flattenModel(model);

    validator->validateModel(model);
    EXPECT_EQ(size_t(0), validator->errorCount());
}

TEST(Importer, importingComponentWithCnUnitsThatAreAlreadyDefinedInImportingModel)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"myModel\">\n"
        "  <units name=\"myUnitsThatIUse\">\n"
        "    <unit units=\"metre\"/>\n"
        "  </units>\n"
        "  <units name=\"myUnitsThatIUse_1\">\n"
        "    <unit units=\"second\"/>\n"
        "  </units>\n"
        "  <component name=\"c\">\n"
        "    <variable name=\"a\" units=\"second\"/>\n"
        "    <math xmlns=\"http://www.w3.org/1998/Math/MathML\" xmlns:cellml=\"http://www.cellml.org/cellml/2.0#\">\n"
        "      <apply>\n"
        "        <eq/>\n"
        "        <ci>a</ci>\n"
        "        <cn cellml:units=\"myUnitsThatIUse_1\">1</cn>\n"
        "      </apply>\n"
        "    </math>\n"
        "  </component>\n"
        "</model>\n";

    const std::string in =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"myModel\">\n"
        "  <units name=\"myUnitsThatIUse\">\n"
        "    <unit units=\"second\"/>\n"
        "  </units>\n"
        "   <component name=\"myComponent\">\n"
        "       <variable name=\"a\" units=\"second\"/>\n"
        "       <math xmlns=\"http://www.w3.org/1998/Math/MathML\" xmlns:cellml=\"http://www.cellml.org/cellml/2.0#\">\n"
        "           <apply>"
        "             <eq/>\n"
        "             <ci>a</ci>\n"
        "             <cn cellml:units=\"myUnitsThatIUse\">1</cn>\n"
        "           </apply>\n"
        "       </math>\n"
        "   </component>\n"
        "</model>";

    // Create the model by parsing the string above.
    auto parser = libcellml::Parser::create();
    auto importedModel = parser->parseModel(in);
    auto importer = libcellml::Importer::create();
    auto validator = libcellml::Validator::create();

    // No problems with the imported model.
    validator->validateModel(importedModel);
    EXPECT_EQ(size_t(0), validator->errorCount());

    // The model importedModel already has myUnitsThatIUse defined: the units here will clash.
    auto model = libcellml::Model::create("myModel");
    auto u = libcellml::Units::create("myUnitsThatIUse");
    u->addUnit("metre");
    model->addUnits(u);

    auto c = libcellml::Component::create("c");

    auto importSource = libcellml::ImportSource::create();
    importSource->setUrl("not_required_resolving_import_manually");
    importSource->setModel(importedModel);

    c->setImportReference("myComponent");
    c->setImportSource(importSource);
    model->addComponent(c);

    EXPECT_FALSE(model->hasUnresolvedImports());
    model = importer->flattenModel(model);

    validator->validateModel(model);
    EXPECT_EQ(size_t(0), validator->errorCount());

    // I would expect that the name of the cn elements units would need
    // to change as the importing model already has units of that name.
    auto printer = libcellml::Printer::create();
    EXPECT_EQ(e, printer->printModel(model));
}

TEST(Importer, importUnitsNotDuplicated)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"model\">\n"
        "  <units name=\"millisecond\">\n"
        "    <unit prefix=\"milli\" units=\"second\"/>\n"
        "  </units>\n"
        "  <units name=\"per_millisecond\">\n"
        "    <unit exponent=\"-1\" units=\"millisecond\"/>\n"
        "  </units>\n"
        "  <component name=\"c\">\n"
        "    <variable name=\"alpha_n\" units=\"per_millisecond\" initial_value=\"1\"/>\n"
        "    <variable name=\"beta_n\" units=\"per_millisecond\" initial_value=\"2\"/>\n"
        "  </component>\n"
        "</model>\n";

    const std::string in =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"myImportedModel\">\n"
        "  <units name=\"millisecond\">\n"
        "    <unit prefix=\"milli\" units=\"second\"/>\n"
        "  </units>\n"
        "  <units name=\"per_millisecond\">\n"
        "    <unit exponent=\"-1\" units=\"millisecond\"/>\n"
        "  </units>\n"
        "  <component name=\"myComponent\">\n"
        "    <variable name=\"alpha_n\" units=\"per_millisecond\" initial_value=\"1\"/>\n"
        "    <variable name=\"beta_n\" units=\"per_millisecond\" initial_value=\"2\"/>\n"
        "  </component>\n"
        "</model>";

    // Create the model by parsing the string above.
    auto parser = libcellml::Parser::create();
    auto importedModel = parser->parseModel(in);
    auto importer = libcellml::Importer::create();
    auto validator = libcellml::Validator::create();
    validator->validateModel(importedModel);
    EXPECT_EQ(size_t(0), validator->errorCount());

    auto model = libcellml::Model::create("model");
    auto c = libcellml::Component::create("c");

    auto importSource = libcellml::ImportSource::create();
    importSource->setUrl("not_required_resolving_import_manually");
    importSource->setModel(importedModel);

    c->setImportReference("myComponent");
    c->setImportSource(importSource);
    model->addComponent(c);

    EXPECT_FALSE(model->hasUnresolvedImports());
    model = importer->flattenModel(model);

    EXPECT_EQ(size_t(2), model->unitsCount());

    validator->validateModel(model);
    EXPECT_EQ(size_t(0), validator->errorCount());

    auto printer = libcellml::Printer::create();
    EXPECT_EQ(e, printer->printModel(model));
}

TEST(Importer, duplicatesImportedOnceOnly)
{
    std::string in =
        "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n"
        "<model name=\"forked_import\" xmlns=\"http://www.cellml.org/cellml/2.0#\" xmlns:cellml=\"http://www.cellml.org/cellml/2.0#\" xmlns:xlink=\"http://www.w3.org/1999/xlink\">\n"
        "  <import xlink:href=\"prong.cellml\">\n"
        "    <component name=\"left\" component_ref=\"component1\" />\n"
        "  </import>\n"
        "  <import xlink:href=\"prong.cellml\">\n"
        "    <component name=\"right\" component_ref=\"component1\" />\n"
        "  </import>\n"
        "  <import xlink:href=\"prong.cellml\">\n"
        "    <component name=\"centre\" component_ref=\"component1\" />\n"
        "  </import>\n"
        "</model>";
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(in);
    auto importer = libcellml::Importer::create();

    importer->resolveImports(model, resourcePath("importer/"));
    EXPECT_EQ(size_t(3), model->componentCount()); // Three imported components ...
    EXPECT_EQ(size_t(1), importer->libraryCount()); // ... but just one imported model.
    EXPECT_FALSE(model->hasUnresolvedImports());
}

TEST(Importer, accessImportedModelLibrary)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("importer/diamond.cellml"));
    auto importer = libcellml::Importer::create();
    auto printer = libcellml::Printer::create();

    importer->resolveImports(model, resourcePath("importer/"));
    EXPECT_FALSE(model->hasUnresolvedImports());

    // Library should contain left, right, and one instance (not two) of the point.
    EXPECT_EQ(size_t(3), importer->libraryCount());

    // Test that the library items have the expected keys.
    EXPECT_EQ(resourcePath("importer/diamond_left.cellml"), importer->key(0));
    EXPECT_EQ(resourcePath("importer/diamond_point.cellml"), importer->key(1));
    EXPECT_EQ(resourcePath("importer/diamond_right.cellml"), importer->key(2));
    EXPECT_EQ("", importer->key(999));

    // Access library items by their URL.
    auto left = importer->library(resourcePath("importer/diamond_left.cellml"));
    auto right = importer->library(resourcePath("importer/diamond_right.cellml"));
    auto point = importer->library(resourcePath("importer/diamond_point.cellml"));

    // Test that the library items are the same as those in the files.
    EXPECT_EQ_FILE_CONTENTS("importer/diamond_left.cellml", printer->printModel(left));
    EXPECT_EQ_FILE_CONTENTS("importer/diamond_right.cellml", printer->printModel(right));
    EXPECT_EQ_FILE_CONTENTS("importer/diamond_point.cellml", printer->printModel(point));
}

TEST(Importer, multipleModelResolution)
{
    // This test is intended to show how the Importer class can hold multiple imported models in its library, and that
    // these can be used to resolve the imports of more than one importing model.
    // The example given has two models A, B, each importing the same four components, a, b, c, d from a
    // third model D. We expect that model D is parsed and instantiated just once, rather than 8 times.

    auto parser = libcellml::Parser::create();
    auto modelA = parser->parseModel(fileContents("importer/generic.cellml"));
    auto modelB = parser->parseModel(fileContents("importer/generic.cellml"));

    // Model D will be imported into the library as it's included as a dependency in the models here.
    // Passing in one of the models for resolution will load model D into the library.
    auto importer = libcellml::Importer::create();
    importer->resolveImports(modelA, resourcePath("importer/"));

    EXPECT_EQ(size_t(1), importer->libraryCount());

    // Now resolve the other models and expect the library contents to be unchanged.
    importer->resolveImports(modelB, resourcePath("importer/"));

    EXPECT_EQ(size_t(1), importer->libraryCount());
}

TEST(Importer, addModelToLibrary)
{
    // This test shows how a model instance can be manually added to the import library by the user,
    // and will be used to resolve imports, rather than from an external file/URL.
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("importer/generic_no_source.cellml"));
    auto importer = libcellml::Importer::create();

    auto sourceModel = libcellml::Model::create("source");
    sourceModel->addComponent(libcellml::Component::create("a"));
    sourceModel->addComponent(libcellml::Component::create("b"));
    sourceModel->addComponent(libcellml::Component::create("c"));
    sourceModel->addComponent(libcellml::Component::create("d"));

    // Add a model manually to the library, including the URL that it will replace in future imports.
    EXPECT_TRUE(importer->addModel(sourceModel, "i_dont_exist.cellml"));

    // Can't add to the same URL key more than once.
    EXPECT_FALSE(importer->addModel(sourceModel, "i_dont_exist.cellml"));

    importer->resolveImports(model, "");

    EXPECT_EQ(size_t(0), importer->issueCount());
    EXPECT_FALSE(model->hasUnresolvedImports());
}

TEST(Importer, replaceModel)
{
    // This test shows how a model instance can be replaced in the import library by the user,
    // and will be used to resolve imports.
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("importer/generic_no_source.cellml"));
    auto importer = libcellml::Importer::create();

    auto wrongSourceModel = libcellml::Model::create("wrong");

    auto rightSourceModel = libcellml::Model::create("source");
    rightSourceModel->addComponent(libcellml::Component::create("a"));
    rightSourceModel->addComponent(libcellml::Component::create("b"));
    rightSourceModel->addComponent(libcellml::Component::create("c"));
    rightSourceModel->addComponent(libcellml::Component::create("d"));

    // Add a model manually to the library, including the URL that it will replace in future imports.
    EXPECT_TRUE(importer->addModel(wrongSourceModel, "i_dont_exist.cellml"));
    // Can't add to the same URL key more than once.
    EXPECT_FALSE(importer->addModel(rightSourceModel, "i_dont_exist.cellml"));

    // Use replaceModel instead.
    EXPECT_FALSE(importer->replaceModel(rightSourceModel, "not_in_library"));
    EXPECT_TRUE(importer->replaceModel(rightSourceModel, "i_dont_exist.cellml"));

    importer->resolveImports(model, "");

    EXPECT_EQ(size_t(0), importer->issueCount());
    EXPECT_FALSE(model->hasUnresolvedImports());
}

TEST(Importer, getNonexistentModel)
{
    auto importer = libcellml::Importer::create();
    auto model = libcellml::Model::create("model");
    importer->addModel(model, "howdy");
    EXPECT_EQ(nullptr, importer->library("bonjour"));
}

TEST(Importer, library)
{
    auto importer = libcellml::Importer::create();
    auto model1 = libcellml::Model::create("model1");
    auto model2 = libcellml::Model::create("model2");
    auto model3 = libcellml::Model::create("model3");
    auto model4 = libcellml::Model::create("model4");

    EXPECT_TRUE(importer->addModel(model1, "a_model_the_first.cellml"));
    EXPECT_TRUE(importer->addModel(model2, "b_tweede_voorbeeld.cellml"));
    EXPECT_TRUE(importer->addModel(model3, "c_troisieme_modele.cellml"));

    // Add another under the first key, expect false.
    EXPECT_FALSE(importer->addModel(model4, "a_model_the_first.cellml"));
    EXPECT_EQ(size_t(3), importer->libraryCount());

    // Access library by key.
    EXPECT_EQ(model1, importer->library("a_model_the_first.cellml"));
    EXPECT_EQ(model2, importer->library("b_tweede_voorbeeld.cellml"));
    EXPECT_EQ(model3, importer->library("c_troisieme_modele.cellml"));
    EXPECT_EQ(nullptr, importer->library("d_tuawha.cellml"));

    // Access library by index.
    EXPECT_EQ(model1, importer->library(0));
    EXPECT_EQ(model2, importer->library(1));
    EXPECT_EQ(model3, importer->library(2));
    EXPECT_EQ(nullptr, importer->library(999));
}

TEST(Importer, rangeOfValidSituations)
{
    // This is a test to figure out whether we need to restrict the library keys to absolute URLs, or
    // whether they could be any string.  This test creates a collection of models from different sources,
    // some which need imports, some not, and uses them to resolve imports on another model.
    auto parser = libcellml::Parser::create();

    // Parsing concrete units and components.
    auto concreteUnits = parser->parseModel(fileContents("importer/units_concrete.cellml"));
    EXPECT_EQ(size_t(0), parser->issueCount());

    auto concreteComponents = parser->parseModel(fileContents("importer/components_concrete.cellml"));
    EXPECT_EQ(size_t(0), parser->issueCount());

    // Parsing a model which imports components and units from files called units_source.cellml
    // and components_source.cellml respectively.  These "hidden" files will also be added to the importer
    // library when they're resolved.
    auto importedUnits = parser->parseModel(fileContents("importer/units_imported.cellml"));
    EXPECT_EQ(size_t(0), parser->issueCount());

    auto importedComponents = parser->parseModel(fileContents("importer/components_imported.cellml"));
    EXPECT_EQ(size_t(0), parser->issueCount());

    // Create models through the API. These don't need import resolution.
    auto localConcreteUnits = libcellml::Model::create("localConcreteUnits");
    localConcreteUnits->addUnits(libcellml::Units::create("units5"));

    // Add all the concrete models to the Importer. These don't need resolving as they have no imports of their own.
    auto importer = libcellml::Importer::create();
    importer->addModel(concreteUnits, "i_dont_exist_yet.cellml"); // add with key that's not a file (yet).
    importer->addModel(concreteComponents, "https://www.example.com/myComponents.cellml"); // add with a key that is a remote URL.

    // Add models which have imports to the Importer by resolving them. They are automatically saved into the
    // Importer's library, along with any dependencies, under the names used to resolve them.
    importer->resolveImports(importedUnits, resourcePath("importer/"));
    importer->resolveImports(importedComponents, resourcePath("importer/"));

    EXPECT_FALSE(importedUnits->hasUnresolvedImports());
    EXPECT_FALSE(importedComponents->hasUnresolvedImports());

    // Now create a funky model that imports from all over the place and see what happens.
    const std::string funkyString =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"funky\">\n"
        // Status quo functionality, key is relative URL + base file path as the relative URL doesn't exist as a key.
        "  <import xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"units_imported.cellml\">\n"
        "    <units units_ref=\"units1_imported\" name=\"units1FromLibrary\"/>\n"
        "  </import>\n"
        // Use the relative URL for a hidden child source component.
        "  <import xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"components_source.cellml\">\n"
        "    <component component_ref=\"component1\" name=\"component1FromLibrary\"/>\n"
        "  </import>\n"
        // Use the absolute URL for a hidden child source component.
        "  <import xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\""
        + resourcePath("importer/components_source.cellml")
        + "\">\n"
          "    <component component_ref=\"component2\" name=\"component2FromLibrary\"/>\n"
          "  </import>\n"
          // Remote website URL used for key.
          "  <import xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"https://www.example.com/myComponents.cellml\">\n"
          "    <component component_ref=\"component3\" name=\"component3FromLibrary\"/>\n"
          "  </import>\n"
          // Currently non-existent file used for the key. For this CellML file to function outside of the
          // the Importer instance, this model needs to be written to this location.
          "  <import xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"i_dont_exist_yet.cellml\">\n"
          "    <units units_ref=\"units4\" name=\"units4FromLibrary\"/>\n"
          "  </import>\n"
          "</model>";

    auto funkyModel = parser->parseModel(funkyString);
    EXPECT_EQ(size_t(0), parser->issueCount());

    // Pass in a base file path, this will be used to resolve URLs if they don't already exist as keys.
    // This allows the model to be resolved against different locations where local files are specified.
    importer->resolveImports(funkyModel, resourcePath("importer/"));
    EXPECT_FALSE(funkyModel->hasUnresolvedImports());
    EXPECT_EQ(size_t(0), importer->issueCount());

    // Funky hrefs are valid ...
    auto validator = libcellml::Validator::create();
    validator->validateModel(funkyModel);
    EXPECT_EQ(size_t(0), validator->issueCount());

    // ... and can be flattened as expected too.
    auto flattenedFunkyModel = importer->flattenModel(funkyModel);
    EXPECT_EQ(size_t(0), importer->issueCount());

    flattenedFunkyModel->setName("flattenedFunkyModel");
    validator->validateModel(flattenedFunkyModel);
    EXPECT_EQ(size_t(0), validator->issueCount());
}

TEST(Importer, resolveImportsUsingDifferentAPIModels)
{
    auto importer = libcellml::Importer::create();

    // Create a source model, and add units and components for use elsewhere.
    auto source1 = libcellml::Model::create("source1");
    source1->addComponent(libcellml::Component::create("componentThatINeed"));
    source1->addUnits(libcellml::Units::create("unitsThatINeed"));
    auto vanilla = libcellml::Variable::create("vanilla");
    source1->component(0)->addVariable(vanilla);

    // Create a model which consumes "componentThatINeed" and "unitsThatINeed".
    auto model = libcellml::Model::create("model");
    model->addComponent(libcellml::Component::create("componentIWillImport"));
    model->addUnits(libcellml::Units::create("unitsThatIWillImport"));
    auto imp1 = libcellml::ImportSource::create();

    imp1->setUrl("flavourOfTheMonth"); // Set URL to be an arbitrary string.
    model->component(0)->setImportSource(imp1); // Set the same import source for both units and component.
    model->units(0)->setImportSource(imp1);
    model->component(0)->setImportReference("componentThatINeed");
    model->units(0)->setImportReference("unitsThatINeed");

    // Add the source model to the importer library, as "flavour of the month".
    importer->addModel(source1, "flavourOfTheMonth");

    // Resolve the model against the importer library and check it contains vanilla.
    importer->resolveImports(model, "");
    EXPECT_EQ(size_t(0), importer->issueCount());
    EXPECT_FALSE(model->hasUnresolvedImports());
    auto flatModel = importer->flattenModel(model);
    ASSERT_NE(nullptr, flatModel);
    EXPECT_EQ("vanilla", flatModel->component(0)->variable(0)->name());

    // But now the flavour of the month changes, and you have a better definition for componentThatINeed
    // and unitsThatINeed.
    auto source2 = libcellml::Model::create("source2");
    source2->addComponent(libcellml::Component::create("componentThatINeed"));
    source2->addUnits(libcellml::Units::create("unitsThatINeed"));
    auto chocolate = libcellml::Variable::create("chocolate");
    source2->component(0)->addVariable(chocolate);

    // Replace the flavour of the month with the new source model.
    EXPECT_TRUE(importer->replaceModel(source2, "flavourOfTheMonth"));
    // Resolve the model's imports again against the new "flavour".
    importer->clearImports(model);
    EXPECT_TRUE(importer->resolveImports(model, ""));
    EXPECT_EQ(size_t(0), importer->issueCount());
    EXPECT_FALSE(model->hasUnresolvedImports());

    // Check that we now have a variable called "chocolate" in the flattened model.
    auto flatModel2 = importer->flattenModel(model);
    ASSERT_NE(nullptr, flatModel2);
    EXPECT_EQ("chocolate", flatModel2->component(0)->variable(0)->name());
}

TEST(Importer, importEncapsulatedChildren)
{
    // Test to make sure that the Importer is correctly importing child components,
    // and not duplicating them.
    const std::string flatModelString =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"trunkModel\">\n"
        "  <component name=\"branch1\"/>\n"
        "  <component name=\"leaf1\"/>\n"
        "  <component name=\"leaf2\"/>\n"
        "  <component name=\"leaf3\"/>\n"
        "  <component name=\"branch2\"/>\n"
        "  <component name=\"leaf4\"/>\n"
        "  <component name=\"flower1\"/>\n"
        "  <component name=\"flower2\"/>\n"
        "  <component name=\"branch3\"/>\n"
        "  <component name=\"leaf5\"/>\n"
        "  <component name=\"leaf6\"/>\n"
        "  <component name=\"fruit\"/>\n"
        "  <encapsulation>\n"
        "    <component_ref component=\"branch1\">\n"
        "      <component_ref component=\"leaf1\"/>\n"
        "      <component_ref component=\"leaf2\"/>\n"
        "      <component_ref component=\"leaf3\"/>\n"
        "    </component_ref>\n"
        "    <component_ref component=\"branch2\">\n"
        "      <component_ref component=\"leaf4\"/>\n"
        "      <component_ref component=\"flower1\"/>\n"
        "      <component_ref component=\"flower2\"/>\n"
        "    </component_ref>\n"
        "    <component_ref component=\"branch3\">\n"
        "      <component_ref component=\"leaf5\"/>\n"
        "      <component_ref component=\"leaf6\"/>\n"
        "      <component_ref component=\"fruit\"/>\n"
        "    </component_ref>\n"
        "  </encapsulation>\n"
        "</model>\n";

    auto importer = libcellml::Importer::create();
    auto parser = libcellml::Parser::create();
    auto printer = libcellml::Printer::create();
    auto model = parser->parseModel(fileContents("importer/trunk.cellml"));

    importer->resolveImports(model, resourcePath("importer/"));
    EXPECT_FALSE(model->hasUnresolvedImports());
    EXPECT_EQ(size_t(2), importer->libraryCount());

    auto flat = importer->flattenModel(model);
    EXPECT_EQ(flatModelString, printer->printModel(flat));
}

TEST(Importer, importFilesWithSameName)
{
    // This test is to see whether the importer correctly resolves imported sources, where
    // the imported files all have the same file name.

    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("importer/model.cellml"));
    EXPECT_EQ(size_t(0), parser->issueCount());

    auto importer = libcellml::Importer::create();

    // Manually add a model to the library with the "model.cellml" key.
    auto anotherModel = libcellml::Model::create("anotherModelWithKey_model.cellml");
    anotherModel->addComponent(libcellml::Component::create("b"));
    anotherModel->addUnits(libcellml::Units::create("a"));
    importer->addModel(anotherModel, "model.cellml");

    importer->resolveImports(model, resourcePath("importer/"));
    EXPECT_EQ(size_t(0), importer->issueCount());

    // 4 items in the library.
    EXPECT_EQ(size_t(4), importer->libraryCount());

    EXPECT_FALSE(model->hasUnresolvedImports());

    // Compare the library items against their model name to be sure they're different.
    std::vector<std::string> modelNames = {"layer3_model", "layer2_model", "layer1_model", "anotherModelWithKey_model.cellml"};
    for (size_t i = 0; i < importer->libraryCount(); ++i) {
        EXPECT_EQ(modelNames[i], importer->library(i)->name());
    }
}

TEST(Importer, coverageNestedImportsClearingModels)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("importer/nested_components.cellml"));
    EXPECT_EQ(size_t(0), parser->issueCount());

    auto importer = libcellml::Importer::create();
    importer->resolveImports(model, resourcePath("importer/"));

    EXPECT_EQ(size_t(0), importer->issueCount());

    EXPECT_FALSE(model->hasUnresolvedImports());
    importer->clearImports(model);
    EXPECT_TRUE(model->hasUnresolvedImports());
}

TEST(Importer, importSourceGetSetModel)
{
    auto model = libcellml::Model::create("m");
    auto component = libcellml::Component::create("c");
    auto importsource = libcellml::ImportSource::create();

    component->setImportSource(importsource);
    EXPECT_EQ(nullptr, importsource->model());

    importsource->setModel(nullptr);
    EXPECT_EQ(nullptr, importsource->model());

    importsource->setModel(model);
    EXPECT_EQ(model, importsource->model());

    importsource->removeModel();
    EXPECT_EQ(nullptr, importsource->model());
}

TEST(Importer, resolveWithMissingItems)
{
    std::vector<std::string> e = {
        "Import of units 'i_dont_exist' from '" + resourcePath("importer/") + "units_source.cellml' requires units named 'i_dont_exist' which cannot be found.",
        "Import of component 'i_dont_exist' from '" + resourcePath("importer/") + "components_source.cellml' requires component named 'i_dont_exist' which cannot be found.",
    };
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("importer/importing_nonexistent_items.cellml"));
    EXPECT_EQ(size_t(0), parser->issueCount());

    auto importer = libcellml::Importer::create();
    importer->resolveImports(model, resourcePath("importer/"));
    EXPECT_EQ(size_t(2), importer->issueCount());
    EXPECT_EQ_ISSUES(e, importer);
}

TEST(Importer, resolveWithMissingChildComponents)
{
    std::string e = "Import of component 'child' from '" + resourcePath("importer/") + "components_source.cellml' requires component named 'i_dont_exist' which cannot be found.";
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("importer/importing_component_with_missing_children.cellml"));
    EXPECT_EQ(size_t(0), parser->issueCount());

    auto importer = libcellml::Importer::create();
    importer->resolveImports(model, resourcePath("importer/"));
    EXPECT_EQ(size_t(1), importer->issueCount());
    EXPECT_EQ(e, importer->issue(0)->description());
}

TEST(Importer, resolveWithPresentChildUnits)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("importer/importing_units_with_present_children.cellml"));
    EXPECT_EQ(size_t(0), parser->issueCount());

    auto importer = libcellml::Importer::create();
    importer->resolveImports(model, resourcePath("importer/"));
    EXPECT_EQ(size_t(0), importer->issueCount());
}

TEST(Importer, resolveWithMissingChildUnits)
{
    std::string e = "Import of units 'units1' from '" + resourcePath("importer/") + "units_children.cellml' requires units named 'units_with_imaginary_children', which relies on child units named 'I_dont_exist', which cannot be found.";
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("importer/importing_units_with_missing_children.cellml"));
    EXPECT_EQ(size_t(0), parser->issueCount());

    auto importer = libcellml::Importer::create();
    importer->resolveImports(model, resourcePath("importer/"));
    EXPECT_EQ(size_t(1), importer->issueCount());
    EXPECT_EQ(e, importer->issue(0)->description());
}

TEST(Importer, resolveWithMissingChildDependentUnits)
{
    const std::string e = "Import of component 'component' from '" + resourcePath("importer/") + "components_missing_units.cellml' requires units named 'other_units_that_dont_exist' which cannot be found.";

    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("importer/importing_component_with_missing_units.cellml"));
    EXPECT_EQ(size_t(0), parser->issueCount());

    auto importer = libcellml::Importer::create();
    importer->resolveImports(model, resourcePath("importer/"));
    EXPECT_EQ(size_t(1), importer->issueCount());
    EXPECT_EQ(e, importer->issue(0)->description());
}

TEST(Importer, resolveWithChildUnitsImportedFromMissingModel)
{
    std::string e = "The attempt to resolve imports with the model at '" + resourcePath("importer/") + "missing_file.cellml' failed: the file could not be opened.";
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("importer/importing_units_with_child_units_missing_model.cellml"));
    EXPECT_EQ(size_t(0), parser->issueCount());

    auto importer = libcellml::Importer::create();
    importer->resolveImports(model, resourcePath("importer/"));
    EXPECT_EQ(size_t(1), importer->issueCount());
    EXPECT_EQ(e, importer->issue(0)->description());
}

TEST(Importer, resolveComponentWithUnitsMissingModel)
{
    std::string e = "The attempt to resolve imports with the model at '" + resourcePath("importer/") + "missing_model.cellml' failed: the file could not be opened.";

    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("importer/importing_component_with_imported_units_missing_model.cellml"));
    EXPECT_EQ(size_t(0), parser->issueCount());

    auto importer = libcellml::Importer::create();
    importer->resolveImports(model, resourcePath("importer/"));
    EXPECT_EQ(size_t(1), importer->issueCount());
    EXPECT_EQ(e, importer->issue(0)->description());
}

TEST(Importer, resolveImportsOfGrandchildComponents)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("importer/nested_components.cellml"));
    auto importer = libcellml::Importer::create();

    EXPECT_TRUE(model->hasUnresolvedImports());
    importer->resolveImports(model, resourcePath("importer/"));
    EXPECT_EQ(size_t(1), importer->libraryCount());
    EXPECT_EQ(resourcePath("importer/source.cellml"), importer->key(0));
    EXPECT_FALSE(model->hasUnresolvedImports());
}

TEST(Importer, resolveImportsOfGrandchildUnits)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("importer/nested_units.cellml"));
    auto importer = libcellml::Importer::create();

    EXPECT_TRUE(model->hasUnresolvedImports());
    importer->resolveImports(model, resourcePath("importer/"));
    EXPECT_EQ(size_t(1), importer->libraryCount());
    EXPECT_EQ(resourcePath("importer/source_units.cellml"), importer->key(0));
    EXPECT_FALSE(model->hasUnresolvedImports());
}

TEST(Importer, complicatedHHImportMissingGateModel)
{
    // In this test the generic gate model which is imported by deeply encapsulated components
    // through three generations of imports, is missing. It's used here to give coverage.
    auto e = "The attempt to resolve imports with the model at '" + resourcePath("importer/HH/GateModel.cellml") + "' failed: the file could not be opened.";
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("importer/HH/MembraneModel.cellml"));
    auto importer = libcellml::Importer::create();

    EXPECT_TRUE(model->hasUnresolvedImports());
    importer->resolveImports(model, resourcePath("importer/HH/"));
    EXPECT_TRUE(model->hasUnresolvedImports()); // Expect that the missing GateModel.cellml file can't be found.
    EXPECT_EQ(size_t(2), importer->errorCount());
    EXPECT_EQ(e, importer->error(0)->description());
    EXPECT_EQ(e, importer->error(1)->description());
}

TEST(Importer, clearModelImportsBeforeResolving)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("importer/ImportCircularReferences.cellml"));
    auto importer = libcellml::Importer::create();
    importer->resolveImports(model, resourcePath("importer/"));

    // Circular import issue generated.
    EXPECT_EQ(size_t(1), importer->issueCount());

    // Change URL away from circular import to the correct file.
    model->component("controller")->importSource()->setUrl("NotCircularReference.cellml");

    // Resolve imports again after clearing the issues.
    importer->resolveImports(model, resourcePath("importer/"));

    EXPECT_EQ(size_t(0), importer->issueCount());
}

TEST(Importer, isResolvedUnitsNoModel)
{
    auto units = libcellml::Units::create("units");

    EXPECT_FALSE(units->requiresImports());
}

TEST(Importer, isResolvedUnitsWithModel)
{
    auto model = libcellml::Model::create("model");
    auto units = libcellml::Units::create("units");

    model->addUnits(units);

    EXPECT_FALSE(units->requiresImports());
}

TEST(Importer, isResolvedUnitsWithChild)
{
    auto model = libcellml::Model::create("model");
    auto units = libcellml::Units::create("units");
    units->addUnit("second");

    model->addUnits(units);

    EXPECT_FALSE(units->requiresImports());
}

TEST(Importer, isResolvedUnitsWithNonExistentChild)
{
    auto model = libcellml::Model::create("model");
    auto units = libcellml::Units::create("units");
    units->addUnit("bumble");

    model->addUnits(units);

    EXPECT_FALSE(units->requiresImports());
}

TEST(Importer, isResolvedUnitsWithNonStandardChild)
{
    auto model = libcellml::Model::create("model");
    auto units = libcellml::Units::create("units");
    auto unitsBumble = libcellml::Units::create("bumble");
    units->addUnit("bumble");

    model->addUnits(units);
    model->addUnits(unitsBumble);

    EXPECT_FALSE(units->requiresImports());
}

TEST(Importer, isResolvedUnitsWithReferenceToSelf)
{
    auto model = libcellml::Model::create("model");
    auto units = libcellml::Units::create("units");
    units->addUnit("units");

    model->addUnits(units);

    EXPECT_FALSE(units->requiresImports());
}

TEST(Importer, isResolvedUnitsNotImportFullyDefined)
{
    auto model = libcellml::Model::create("standard_model");
    auto units = libcellml::Units::create("my_units");
    units->addUnit("second", -1.0);

    model->addUnits(units);

    EXPECT_FALSE(model->hasUnresolvedImports());
    EXPECT_TRUE(units->isResolved());
    EXPECT_TRUE(units->isDefined());
}

TEST(Importer, isResolvedUnitsNotImportPartiallyDefined)
{
    auto model = libcellml::Model::create("standard_model");
    auto units = libcellml::Units::create("my_units");
    units->addUnit("seconds", -1.0);

    model->addUnits(units);

    EXPECT_FALSE(model->hasUnresolvedImports());
    EXPECT_TRUE(units->isResolved());
    EXPECT_FALSE(units->isDefined());
}

TEST(Importer, isResolvedUnitsOverOneLevel)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("importer/units_definition_level_1.cellml"));
    auto importer = libcellml::Importer::create();

    EXPECT_TRUE(model->hasUnresolvedImports());
    importer->resolveImports(model, resourcePath("importer/"));
    EXPECT_EQ(size_t(1), importer->libraryCount());
    EXPECT_EQ(resourcePath("importer/units_source.cellml"), importer->key(0));

    auto units1 = model->units(0);
    auto units2 = model->units(1);

    EXPECT_TRUE(units1->isResolved());
    EXPECT_FALSE(units2->isResolved());

    EXPECT_TRUE(model->hasUnresolvedImports());
}

TEST(Importer, isResolvedUnitsOverTwoLevels)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("importer/master_units.cellml"));
    auto importer = libcellml::Importer::create();

    EXPECT_TRUE(model->hasUnresolvedImports());
    importer->resolveImports(model, resourcePath("importer/"));
    EXPECT_EQ(size_t(2), importer->libraryCount());
    EXPECT_EQ(resourcePath("importer/units_definition_level_1.cellml"), importer->key(0));
    EXPECT_EQ(resourcePath("importer/units_source.cellml"), importer->key(1));

    auto units1 = model->units(0);

    EXPECT_TRUE(units1->isResolved());

    EXPECT_FALSE(model->hasUnresolvedImports());
}

TEST(Importer, isResolvedUnitsOverTwoLevelsUnresolved)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("importer/master_units_unresolved.cellml"));
    auto importer = libcellml::Importer::create();

    EXPECT_TRUE(model->hasUnresolvedImports());
    importer->resolveImports(model, resourcePath("importer/"));
    EXPECT_EQ(size_t(2), importer->libraryCount());
    EXPECT_EQ(resourcePath("importer/units_definition_level_1_unresolved.cellml"), importer->key(0));
    EXPECT_EQ(resourcePath("importer/units_source.cellml"), importer->key(1));

    auto units1 = model->units(0);

    EXPECT_FALSE(units1->isResolved());

    EXPECT_TRUE(model->hasUnresolvedImports());
}

TEST(Importer, isResolvedUnitsChildUnresolved)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("importer/importing_units_with_unresolved_children.cellml"));
    auto importer = libcellml::Importer::create();

    EXPECT_TRUE(model->hasUnresolvedImports());
    importer->resolveImports(model, resourcePath("importer/"));
    EXPECT_EQ(size_t(1), importer->libraryCount());
    EXPECT_EQ(resourcePath("importer/units_children.cellml"), importer->key(0));

    auto units1 = model->units(0);

    EXPECT_FALSE(units1->isResolved());

    EXPECT_TRUE(model->hasUnresolvedImports());
}

TEST(Importer, isResolvedComponentOverTwoLevels)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("importer/component_importer.cellml"));
    auto importer = libcellml::Importer::create();

    EXPECT_TRUE(model->hasUnresolvedImports());
    importer->resolveImports(model, resourcePath("importer/"));
    EXPECT_EQ(size_t(2), importer->libraryCount());

    auto component = model->component(0);

    EXPECT_TRUE(component->isResolved());

    EXPECT_FALSE(model->hasUnresolvedImports());
}

TEST(Importer, isResolvedComponentOverTwoLevelsUnresolved)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("importer/component_importer_unresolved.cellml"));
    auto importer = libcellml::Importer::create();

    EXPECT_TRUE(model->hasUnresolvedImports());
    importer->resolveImports(model, resourcePath("importer/"));
    EXPECT_EQ(size_t(2), importer->libraryCount());

    auto component = model->component(0);

    EXPECT_FALSE(component->isResolved());

    EXPECT_TRUE(model->hasUnresolvedImports());
}

TEST(Importer, isResolvedReferencedUnitsMissing)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("importer/main_referenced_units_missing.cellml"));
    auto importer = libcellml::Importer::create();

    EXPECT_TRUE(model->hasUnresolvedImports());
    importer->resolveImports(model, resourcePath("importer/"));
    EXPECT_EQ(size_t(1), importer->libraryCount());

    auto units = model->units(0);

    EXPECT_TRUE(units->isResolved());
    EXPECT_FALSE(units->isDefined());

    EXPECT_FALSE(model->hasUnresolvedImports());
    EXPECT_FALSE(model->isDefined());
}

TEST(Importer, isResolvedCircularImportUnits)
{
    auto parser = libcellml::Parser::create();
    auto importer = libcellml::Importer::create();
    auto model = parser->parseModel(fileContents("importer/circularUnits_1.cellml"));
    EXPECT_EQ(size_t(0), parser->issueCount());

    auto u = model->units(0);

    EXPECT_FALSE(u->isResolved());

    importer->resolveImports(model, resourcePath("importer/"));
    EXPECT_EQ(size_t(1), importer->issueCount());

    // It is possible however difficult to make a circular import with units.
    auto importModel = u->importSource()->model();
    importModel->units("u2")->importSource()->model()->units("u3")->importSource()->model()->units("i_am_cyclic")->importSource()->setModel(importModel);

    EXPECT_FALSE(u->isResolved());
    EXPECT_TRUE(model->hasUnresolvedImports());
}

TEST(Importer, isResolvedCircularImportComponent)
{
    auto parser = libcellml::Parser::create();
    auto importer = libcellml::Importer::create();
    auto model = parser->parseModel(fileContents("importer/circularImport_1.cellml"));
    EXPECT_EQ(size_t(0), parser->issueCount());

    auto c = model->component(0);

    EXPECT_FALSE(c->isResolved());

    importer->resolveImports(model, resourcePath("importer/"));
    EXPECT_EQ(size_t(1), importer->issueCount());

    // It is possible however difficult to make a circular import with components.
    auto importModel = c->importSource()->model();
    importModel->component("c2")->importSource()->model()->component("c3")->importSource()->model()->component("i_am_cyclic")->importSource()->setModel(importModel);

    EXPECT_FALSE(c->isResolved());
    EXPECT_TRUE(model->hasUnresolvedImports());
}

TEST(Importer, removeAllModels)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("importer/diamond.cellml"));
    auto importer = libcellml::Importer::create();
    importer->resolveImports(model, resourcePath("importer/"));

    EXPECT_EQ(size_t(3), importer->libraryCount());

    importer->removeAllModels();

    EXPECT_EQ(size_t(0), importer->libraryCount());
}

TEST(Importer, importingCommonUnitsDefinitions)
{
    auto parser = libcellml::Parser::create();
    auto importer = libcellml::Importer::create();
    auto model = parser->parseModel(fileContents("importer/common_units_import_1.xml"));
    EXPECT_EQ(size_t(0), parser->issueCount());
    importer->resolveImports(model, resourcePath("importer/"));
    EXPECT_EQ(size_t(0), importer->issueCount());
}

TEST(Importer, basePathEndingInForwardSlash)
{
    auto importer = libcellml::Importer::create();
    auto parser = libcellml::Parser::create();

    auto importedUnits = parser->parseModel(fileContents("importer/units_imported.cellml"));

    EXPECT_TRUE(importedUnits->hasUnresolvedImports());
    importer->resolveImports(importedUnits, resourcePath("importer/"));
    EXPECT_FALSE(importedUnits->hasUnresolvedImports());
}

TEST(Importer, basePathEndingInBackSlash)
{
    auto importer = libcellml::Importer::create();
    auto parser = libcellml::Parser::create();

    auto importedUnits = parser->parseModel(fileContents("importer/units_imported.cellml"));

    EXPECT_TRUE(importedUnits->hasUnresolvedImports());
    importer->resolveImports(importedUnits, resourcePath("importer\\"));
    EXPECT_FALSE(importedUnits->hasUnresolvedImports());
}

TEST(Importer, basePathNotEndingInSlash)
{
    auto importer = libcellml::Importer::create();
    auto parser = libcellml::Parser::create();

    auto importedUnits = parser->parseModel(fileContents("importer/units_imported.cellml"));

    EXPECT_TRUE(importedUnits->hasUnresolvedImports());
    importer->resolveImports(importedUnits, resourcePath("importer"));
    EXPECT_FALSE(importedUnits->hasUnresolvedImports());
}

TEST(Importer, importInvalidXml)
{
    auto e = "The attempt to import the model at '" + resourcePath("importer/not_even_proper.xml") + "' failed: the file is not valid XML.";

    auto importer = libcellml::Importer::create();
    auto parser = libcellml::Parser::create();

    auto model = parser->parseModel(fileContents("importer/import_invalid_xml.cellml"));

    importer->resolveImports(model, resourcePath("importer"));
    EXPECT_EQ(size_t(1), importer->errorCount());
    EXPECT_EQ(e, importer->error(0)->description());
}

TEST(Importer, importInvalidUnitsFromCellmlModel)
{
    auto e = "Encountered an error when resolving units 'units1_imported' from '" + resourcePath("importer/invalid_model.cellml") + "'.";

    auto importer = libcellml::Importer::create();
    auto parser = libcellml::Parser::create();

    auto model = parser->parseModel(fileContents("importer/import_invalid_units.cellml"));

    importer->resolveImports(model, resourcePath("importer"));
    EXPECT_EQ(size_t(1), importer->errorCount());
    EXPECT_EQ(e, importer->error(0)->description());
}

TEST(Importer, importUnitsThatEncounterARelatedImportError)
{
    const std::string e = "Encountered an error when resolving units 'unitsB_imported' from '" + resourcePath("importer/simple_model.cellml") + "'.";
    auto importer = libcellml::Importer::create();
    auto parser = libcellml::Parser::create();

    auto model = parser->parseModel(fileContents("importer/import_invalid_component.cellml"));
    model->component(0)->setImportReference("component4");
    model->component(0)->setName("component4_imported");

    importer->resolveImports(model, resourcePath("importer"));
    EXPECT_EQ(size_t(1), importer->errorCount());
    EXPECT_EQ(e, importer->error(0)->description());
}

TEST(Importer, importInvalidComponentFromCellmlModelComponentError)
{
    auto e = "Encountered an error when resolving component 'component1_imported' from '" + resourcePath("importer/invalid_model.cellml") + "'.";

    auto importer = libcellml::Importer::create();
    auto parser = libcellml::Parser::create();

    auto model = parser->parseModel(fileContents("importer/import_invalid_component.cellml"));
    model->component(0)->setImportReference("component1");
    model->component(0)->setName("component1_imported");

    importer->resolveImports(model, resourcePath("importer"));
    EXPECT_EQ(size_t(1), importer->errorCount());
    EXPECT_EQ(e, importer->error(0)->description());
}

TEST(Importer, importInvalidComponentFromCellmlModelVariableError)
{
    auto e = "Encountered an error when resolving component 'component2_imported' from '" + resourcePath("importer/invalid_model.cellml") + "'.";

    auto importer = libcellml::Importer::create();
    auto parser = libcellml::Parser::create();

    auto model = parser->parseModel(fileContents("importer/import_invalid_component.cellml"));
    model->component(0)->setImportReference("component2");
    model->component(0)->setName("component2_imported");

    importer->resolveImports(model, resourcePath("importer"));
    EXPECT_EQ(size_t(1), importer->errorCount());
    EXPECT_EQ(e, importer->error(0)->description());
}

TEST(Importer, importInvalidComponentFromCellmlModelResetError)
{
    auto e = "Encountered an error when resolving component 'component3_imported' from '" + resourcePath("importer/invalid_model.cellml") + "'.";

    auto importer = libcellml::Importer::create();
    auto parser = libcellml::Parser::create();

    auto model = parser->parseModel(fileContents("importer/import_invalid_component.cellml"));
    model->component(0)->setImportReference("component3");
    model->component(0)->setName("component3_imported");

    importer->resolveImports(model, resourcePath("importer"));
    EXPECT_EQ(size_t(1), importer->errorCount());
    EXPECT_EQ(e, importer->error(0)->description());
}

void testImporterWithInvalidImportedModels(bool strict)
{
    const std::string e = "The attempt to import the model at '" + resourcePath("importer/triangle_units_opposite.cellml") + "' failed: the file is not valid XML.";

    auto importer = libcellml::Importer::create(strict);
    auto parser = libcellml::Parser::create(strict);

    auto model = parser->parseModel(fileContents("importer/triangle_units_point.cellml"));

    importer->resolveImports(model, resourcePath("importer"));
    EXPECT_EQ(size_t(1), importer->errorCount());
    EXPECT_EQ(e, importer->error(0)->description());
}

TEST(Importer, importingCommonUnitsFromTriangleImportStructurePermissive)
{
    testImporterWithInvalidImportedModels(false);
}

TEST(Importer, importingCommonUnitsFromTriangleImportStructureStrict)
{
    testImporterWithInvalidImportedModels(true);
}
