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

#include "test_utils.h"

#include "gtest/gtest.h"

#include <libcellml>

TEST(ImportSource, createImportSource)
{
    auto imp1 = libcellml::ImportSource::create();
    EXPECT_NE(nullptr, imp1);
}

TEST(ImportSource, addImportSourceBadInput)
{
    auto importer = libcellml::Importer::create();

    EXPECT_FALSE(importer->addImportSource(nullptr));
}

TEST(ImportSource, addToImporter)
{
    auto importer = libcellml::Importer::create();

    // Add import source to importer:
    auto imp1 = libcellml::ImportSource::create();
    std::string url1 = "http://www.example.com#hello";
    imp1->setUrl(url1);

    EXPECT_TRUE(importer->addImportSource(imp1));
    EXPECT_EQ(size_t(1), importer->importSourceCount());
    EXPECT_EQ(imp1, importer->importSource(0));

    // Add the same one again:
    EXPECT_FALSE(importer->addImportSource(imp1));
    EXPECT_EQ(size_t(1), importer->importSourceCount());
    EXPECT_EQ(imp1, importer->importSource(0));

    // Add another with the same URL:
    auto imp2 = libcellml::ImportSource::create();
    imp2->setUrl(url1);

    EXPECT_TRUE(importer->addImportSource(imp2));
    EXPECT_EQ(size_t(2), importer->importSourceCount());
    EXPECT_EQ(imp2, importer->importSource(1));

    // Add another with a new URL:
    auto imp3 = libcellml::ImportSource::create();
    auto url3 = "http://www.example.com#bonjour";
    imp3->setUrl(url3);

    EXPECT_TRUE(importer->addImportSource(imp3));
    EXPECT_EQ(size_t(3), importer->importSourceCount());
    EXPECT_EQ(imp3, importer->importSource(2));
}

TEST(ImportSource, hasImportSource)
{
    auto importer = libcellml::Importer::create();

    auto importSource = libcellml::ImportSource::create();

    EXPECT_FALSE(importer->hasImportSource(importSource));

    importer->addImportSource(importSource);

    EXPECT_TRUE(importer->hasImportSource(importSource));
}

TEST(ImportSource, hasImportSourceDifferentImportSources)
{
    auto importer = libcellml::Importer::create();

    auto importSource1 = libcellml::ImportSource::create();
    importSource1->setUrl("some_url");
    auto importSource2 = libcellml::ImportSource::create();
    importSource2->setUrl("some_other_url");

    EXPECT_FALSE(importer->hasImportSource(importSource1));
    EXPECT_FALSE(importer->hasImportSource(importSource2));

    importer->addImportSource(importSource1);

    EXPECT_TRUE(importer->hasImportSource(importSource1));
    EXPECT_FALSE(importer->hasImportSource(importSource2));
}

TEST(ImportSource, hasImportSourceEqualImportSources)
{
    auto importer = libcellml::Importer::create();

    auto importSource1 = libcellml::ImportSource::create();
    importSource1->setUrl("same_url");
    auto importSource2 = libcellml::ImportSource::create();
    importSource2->setUrl("same_url");

    EXPECT_FALSE(importer->hasImportSource(importSource1));
    EXPECT_FALSE(importer->hasImportSource(importSource2));

    importer->addImportSource(importSource1);

    EXPECT_TRUE(importer->hasImportSource(importSource1));
    EXPECT_TRUE(importer->hasImportSource(importSource2));
}

TEST(ImportSource, removeImportSourceByIndex)
{
    auto importer = libcellml::Importer::create();

    auto importSource = libcellml::ImportSource::create();

    importer->addImportSource(importSource);

    EXPECT_EQ(size_t(1), importer->importSourceCount());

    EXPECT_TRUE(importer->removeImportSource(0));

    EXPECT_EQ(size_t(0), importer->importSourceCount());

    EXPECT_FALSE(importer->removeImportSource(0));
}

TEST(ImportSource, removeImportSourceByReference)
{
    auto importer = libcellml::Importer::create();

    auto importSource = libcellml::ImportSource::create();

    importer->addImportSource(importSource);

    EXPECT_EQ(size_t(1), importer->importSourceCount());

    EXPECT_TRUE(importer->removeImportSource(importSource));

    EXPECT_EQ(size_t(0), importer->importSourceCount());

    EXPECT_FALSE(importer->removeImportSource(importSource));
}

TEST(ImportSource, importSourceSharedAcrossImporters)
{
    auto i1 = libcellml::Importer::create();
    auto imp = libcellml::ImportSource::create();

    // Add import source to an importer.
    EXPECT_TRUE(i1->addImportSource(imp));

    // Add import source to another importer.
    auto i2 = libcellml::Importer::create();
    EXPECT_TRUE(i2->addImportSource(imp));

    // Expect that first importer has an import source
    EXPECT_EQ(size_t(1), i1->importSourceCount());
    EXPECT_EQ(imp, i1->importSource(0));
    EXPECT_TRUE(i1->removeImportSource(imp));

    // and that we can get it and remove it from the second.
    EXPECT_EQ(size_t(1), i2->importSourceCount());
    EXPECT_EQ(imp, i2->importSource(0));
    EXPECT_TRUE(i2->removeImportSource(imp));
}

TEST(ImportSource, addRemoveFromImporter)
{
    auto imp = libcellml::ImportSource::create();
    auto importer = libcellml::Importer::create();

    importer->addImportSource(imp);
    EXPECT_EQ(size_t(1), importer->importSourceCount());
    EXPECT_EQ(imp, importer->importSource(0));

    importer->removeImportSource(imp);
    EXPECT_EQ(size_t(0), importer->importSourceCount());
    EXPECT_EQ(nullptr, importer->importSource(0));

    auto imp1 = libcellml::ImportSource::create();
    auto imp2 = libcellml::ImportSource::create();
    auto c1 = libcellml::Component::create("c1");
    auto c2 = libcellml::Component::create("c2");
    auto u1 = libcellml::Units::create("u1");
    auto u2 = libcellml::Units::create("u2");

    c1->setSourceComponent(imp1, "cc1");
    c2->setSourceComponent(imp1, "cc2");
    u1->setSourceUnits(imp2, "uu1");
    u2->setSourceUnits(imp2, "uu2");

    EXPECT_TRUE(c1->isImport());
    EXPECT_TRUE(c2->isImport());
    EXPECT_TRUE(u1->isImport());
    EXPECT_TRUE(u2->isImport());

    importer->addImportSource(imp1);
    importer->addImportSource(imp2);

    EXPECT_EQ(size_t(2), importer->importSourceCount());

    importer->removeAllImportSources();

    EXPECT_EQ(size_t(0), importer->importSourceCount());

    EXPECT_TRUE(c1->isImport());
    EXPECT_TRUE(c2->isImport());
    EXPECT_TRUE(u1->isImport());
    EXPECT_TRUE(u2->isImport());
}

TEST(ImportSource, addImportComponentBeforeAddingToModel)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <import xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"this_place.cellml\">\n"
        "    <component component_ref=\"c_other\" name=\"c2\"/>\n"
        "  </import>\n"
        "  <component name=\"c1\"/>\n"
        "  <encapsulation>\n"
        "    <component_ref component=\"c1\">\n"
        "      <component_ref component=\"c2\"/>\n"
        "    </component_ref>\n"
        "  </encapsulation>\n"
        "</model>\n";

    auto printer = libcellml::Printer::create();
    auto model = libcellml::Model::create();
    auto importSource = libcellml::ImportSource::create();

    importSource->setUrl("this_place.cellml");

    auto c1 = libcellml::Component::create("c1");
    auto c2 = libcellml::Component::create("c2");

    c2->setImportSource(importSource);
    c2->setImportReference("c_other");

    c1->addComponent(c2);
    model->addComponent(c1);

    EXPECT_EQ(e, printer->printModel(model));
}

TEST(ImportSource, addImportComponentAfterAddingToModel)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <import xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"this_place.cellml\">\n"
        "    <component component_ref=\"c_other\" name=\"c2\"/>\n"
        "  </import>\n"
        "  <component name=\"c1\"/>\n"
        "  <encapsulation>\n"
        "    <component_ref component=\"c1\">\n"
        "      <component_ref component=\"c2\"/>\n"
        "    </component_ref>\n"
        "  </encapsulation>\n"
        "</model>\n";

    auto printer = libcellml::Printer::create();
    auto model = libcellml::Model::create();
    auto importSource = libcellml::ImportSource::create();

    importSource->setUrl("this_place.cellml");

    auto c1 = libcellml::Component::create("c1");
    auto c2 = libcellml::Component::create("c2");

    c2->setImportSource(importSource);
    c2->setImportReference("c_other");

    model->addComponent(c1);
    c1->addComponent(c2);

    EXPECT_EQ(e, printer->printModel(model));
}
