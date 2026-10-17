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

/**
 * @todo Need a way to have resources which can be used in tests, such as the
 * source models used in these tests. But I guess not until we start validating
 * models...
 *
 * While this is needed eventually, for now we are not instantiating the imports
 * so don't need it just yet.
 */

TEST(ComponentImport, basics)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <import xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"a-model.xml\">\n"
        "    <component component_ref=\"bob\" name=\"\"/>\n"
        "  </import>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ImportSourcePtr imp = libcellml::ImportSource::create();
    imp->setUrl("a-model.xml");

    libcellml::ComponentPtr c = libcellml::Component::create();
    EXPECT_EQ(nullptr, c->importSource());
    EXPECT_EQ("", c->importReference());

    c->setImportSource(imp);
    c->setImportReference("bob");

    EXPECT_EQ(imp, c->importSource());
    EXPECT_EQ("bob", c->importReference());

    m->addComponent(c);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);

    EXPECT_EQ(e, a);
}

TEST(ComponentImport, singleImportA)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <import xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"some-other-model.xml\">\n"
        "    <component component_ref=\"a_component_in_that_model\" name=\"component_in_this_model\"/>\n"
        "  </import>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ImportSourcePtr imp = libcellml::ImportSource::create();
    imp->setUrl("some-other-model.xml");

    libcellml::ComponentPtr importedComponent = libcellml::Component::create();

    EXPECT_EQ(importedComponent->importSource(), nullptr);

    importedComponent->setName("component_in_this_model");
    importedComponent->setSourceComponent(imp, "a_component_in_that_model");

    EXPECT_EQ(importedComponent->importSource(), imp);

    EXPECT_EQ(size_t(0), m->componentCount());
    m->addComponent(importedComponent);
    EXPECT_EQ(size_t(1), m->componentCount());

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

TEST(ComponentImport, singleImportB)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <import xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"some-other-model.xml\">\n"
        "    <component component_ref=\"a_component_in_that_model\" name=\"component_in_this_model\"/>\n"
        "  </import>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ImportSourcePtr imp = libcellml::ImportSource::create();
    imp->setUrl("some-other-model.xml");

    libcellml::ComponentPtr importedComponent = libcellml::Component::create();
    importedComponent->setName("component_in_this_model");
    importedComponent->setImportSource(imp);
    importedComponent->setImportReference("a_component_in_that_model");
    m->addComponent(importedComponent);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

TEST(ComponentImport, nonExistentUrlAndParse)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <import xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"http://someplace.world/cellml/model.xml\">\n"
        "    <component component_ref=\"na_channel\" name=\"noble_na_channel\"/>\n"
        "  </import>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ImportSourcePtr imp = libcellml::ImportSource::create();
    imp->setUrl("http://someplace.world/cellml/model.xml");

    libcellml::ComponentPtr importedComponent = libcellml::Component::create();

    EXPECT_EQ(importedComponent->importSource(), nullptr);

    importedComponent->setName("noble_na_channel");
    importedComponent->setSourceComponent(imp, "na_channel");

    EXPECT_EQ(importedComponent->importSource(), imp);

    EXPECT_EQ(size_t(0), m->componentCount());
    m->addComponent(importedComponent);
    EXPECT_EQ(size_t(1), m->componentCount());

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);

    // Parse
    libcellml::ParserPtr parser = libcellml::Parser::create();
    libcellml::ModelPtr model = parser->parseModel(e);
    EXPECT_EQ(size_t(1), model->componentCount());
    a = printer->printModel(model);
    EXPECT_EQ(e, a);
}

TEST(ComponentImport, multipleImportAndParse)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <import xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"some-other-model.xml\">\n"
        "    <component component_ref=\"cc1\" name=\"c1\"/>\n"
        "    <component component_ref=\"cc2\" name=\"c2\"/>\n"
        "  </import>\n"
        "  <import xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"some-other-model.xml\">\n"
        "    <component component_ref=\"cc1\" name=\"c3\"/>\n"
        "  </import>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ImportSourcePtr imp = libcellml::ImportSource::create();
    imp->setUrl("some-other-model.xml");
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    c1->setName("c1");
    c1->setSourceComponent(imp, "cc1");
    m->addComponent(c1);
    libcellml::ComponentPtr c2 = libcellml::Component::create();
    c2->setName("c2");
    c2->setSourceComponent(imp, "cc2");
    m->addComponent(c2);

    // These have the same URL but a different ImportSource object, so should be separated.
    libcellml::ImportSourcePtr imp2 = libcellml::ImportSource::create();
    imp2->setUrl("some-other-model.xml");
    libcellml::ComponentPtr c3 = libcellml::Component::create();
    c3->setName("c3");
    c3->setSourceComponent(imp2, "cc1");
    m->addComponent(c3);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);

    // Parse
    libcellml::ParserPtr parser = libcellml::Parser::create();
    libcellml::ModelPtr model = parser->parseModel(e);
    EXPECT_EQ(size_t(3), model->componentCount());
    a = printer->printModel(model);
    EXPECT_EQ(e, a);
}

TEST(ComponentImport, hierarchicalImportAndParse)
{
    const std::string expected =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <import xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"some-other-model.xml\">\n"
        "    <component component_ref=\"cc1\" name=\"importedComponent\"/>\n"
        "  </import>\n"
        "  <component name=\"dave\"/>\n"
        "  <component name=\"bob\"/>\n"
        "  <encapsulation>\n"
        "    <component_ref component=\"dave\">\n"
        "      <component_ref component=\"bob\">\n"
        "        <component_ref component=\"importedComponent\"/>\n"
        "      </component_ref>\n"
        "    </component_ref>\n"
        "  </encapsulation>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ImportSourcePtr imp = libcellml::ImportSource::create();
    imp->setUrl("some-other-model.xml");

    libcellml::ComponentPtr component1 = libcellml::Component::create();
    component1->setName("dave");
    m->addComponent(component1);

    libcellml::ComponentPtr component2 = libcellml::Component::create();
    component2->setName("bob");
    component1->addComponent(component2);

    EXPECT_FALSE(component1->isImport());

    libcellml::ComponentPtr importedComponent = libcellml::Component::create();
    importedComponent->setName("importedComponent");
    importedComponent->setSourceComponent(imp, "cc1");

    EXPECT_TRUE(importedComponent->isImport());

    EXPECT_EQ(size_t(0), component2->componentCount());
    component2->addComponent(importedComponent);
    EXPECT_EQ(size_t(1), component2->componentCount());

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    std::string actual = printer->printModel(m);
    EXPECT_EQ(expected, actual);

    // Parse
    libcellml::ParserPtr parser = libcellml::Parser::create();
    libcellml::ModelPtr model = parser->parseModel(expected);
    EXPECT_EQ(size_t(1), model->componentCount());
    EXPECT_TRUE(model->component("importedComponent", true)->isImport());
    actual = printer->printModel(model);
    EXPECT_EQ(expected, actual);
}

TEST(ComponentImport, complexImportAndParse)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <import xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"some-other-model.xml\">\n"
        "    <component component_ref=\"cc1\" name=\"c1\"/>\n"
        "  </import>\n"
        "  <component name=\"dave\"/>\n"
        "  <component name=\"bob\"/>\n"
        "  <component name=\"angus\"/>\n"
        "  <encapsulation>\n"
        "    <component_ref component=\"dave\">\n"
        "      <component_ref component=\"bob\">\n"
        "        <component_ref component=\"c1\"/>\n"
        "        <component_ref component=\"angus\"/>\n"
        "      </component_ref>\n"
        "    </component_ref>\n"
        "  </encapsulation>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ImportSourcePtr imp = libcellml::ImportSource::create();
    imp->setUrl("some-other-model.xml");

    libcellml::ComponentPtr dave = libcellml::Component::create();
    dave->setName("dave");
    m->addComponent(dave);

    libcellml::ComponentPtr bob = libcellml::Component::create();
    bob->setName("bob");
    dave->addComponent(bob);

    EXPECT_FALSE(dave->isImport());

    libcellml::ComponentPtr i1 = libcellml::Component::create();
    i1->setName("c1");
    i1->setSourceComponent(imp, "cc1");

    EXPECT_TRUE(i1->isImport());

    bob->addComponent(i1);

    libcellml::ComponentPtr angus = libcellml::Component::create();
    angus->setName("angus");
    bob->addComponent(angus);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);

    // Parse
    libcellml::ParserPtr parser = libcellml::Parser::create();
    libcellml::ModelPtr model = parser->parseModel(e);
    a = printer->printModel(model);
    EXPECT_EQ(e, a);

    // check component counts
    const libcellml::ComponentPtr constDave = model->component("dave");
    EXPECT_EQ(size_t(1), constDave->componentCount());
    const libcellml::ComponentPtr constBob = constDave->component("bob");
    EXPECT_EQ(size_t(2), constBob->componentCount());
}

TEST(ComponentImport, noNameAttribute)
{
    const std::string in =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"model\">\n"
        "  <import xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"http://someplace.world/cellml/model.xml\">\n"
        "    <component component_ref=\"holey\"/>\n"
        "  </import>\n"
        "</model>\n";

    libcellml::ParserPtr p = libcellml::Parser::create();

    libcellml::ModelPtr m = p->parseModel(in);

    EXPECT_EQ(size_t(1), p->errorCount());
    EXPECT_EQ("Import of component does not specify a name attribute.", p->error(0)->description());
}

TEST(ComponentImport, notUniqueImportName)
{
    const std::string in =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"model\">\n"
        "  <import xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"http://someplace.world/cellml/model.xml\">\n"
        "    <component component_ref=\"holey\" name=\"bob\"/>\n"
        "    <component component_ref=\"moley\" name=\"bob\"/>\n"
        "  </import>\n"
        "</model>\n";

    libcellml::ParserPtr p = libcellml::Parser::create();
    libcellml::ValidatorPtr v = libcellml::Validator::create();

    libcellml::ModelPtr m = p->parseModel(in);

    EXPECT_EQ(size_t(0), p->errorCount());

    v->validateModel(m);

    EXPECT_EQ(size_t(1), v->errorCount());
    EXPECT_EQ("Model 'model' contains multiple components with the name 'bob'. Valid component names must be unique to their model.", v->error(0)->description());
}
