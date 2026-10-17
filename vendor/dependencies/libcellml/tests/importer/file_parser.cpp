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

#include "test_resources.h"
#include "test_utils.h"

#include "gtest/gtest.h"

#include <libcellml>

#include <algorithm>
#include <fstream>
#include <sstream>
#include <vector>

TEST(Importer, resolveSineModelFromFile)
{
    libcellml::ParserPtr p = libcellml::Parser::create();
    libcellml::ModelPtr model = p->parseModel(fileContents("sine_approximations.xml"));

    EXPECT_EQ(size_t(0), p->issueCount());
    EXPECT_FALSE(model->hasUnresolvedImports());
}

TEST(Importer, resolveSineImportsModelFromFile)
{
    libcellml::ParserPtr p = libcellml::Parser::create();
    libcellml::ModelPtr model = p->parseModel(fileContents("sine_approximations_import.xml"));
    auto importer = libcellml::Importer::create();
    EXPECT_EQ(size_t(0), p->issueCount());

    EXPECT_TRUE(model->hasUnresolvedImports());
    importer->resolveImports(model, resourcePath());
    EXPECT_FALSE(model->hasUnresolvedImports());
}

TEST(Importer, resolveComplexImportsModelFromFile)
{
    libcellml::ParserPtr p = libcellml::Parser::create();
    libcellml::ModelPtr model = p->parseModel(fileContents("complex_imports.xml"));
    auto importer = libcellml::Importer::create();
    EXPECT_EQ(size_t(0), p->issueCount());

    EXPECT_TRUE(model->hasUnresolvedImports());
    importer->resolveImports(model, resourcePath());
    EXPECT_FALSE(model->hasUnresolvedImports());
}

TEST(Importer, resolveUnitsImportFromFile)
{
    libcellml::ParserPtr p = libcellml::Parser::create(false);
    libcellml::ModelPtr model = p->parseModel(fileContents("import_units_model.cellml"));
    auto importer = libcellml::Importer::create();

    EXPECT_EQ(size_t(0), p->issueCount());

    EXPECT_TRUE(model->hasUnresolvedImports());
    importer->resolveImports(model, resourcePath());
    EXPECT_FALSE(model->hasUnresolvedImports());
}

TEST(Importer, resolveImportsFromFileLevel0)
{
    libcellml::ParserPtr p = libcellml::Parser::create();
    libcellml::ModelPtr model = p->parseModel(fileContents("level0.xml"));
    auto importer = libcellml::Importer::create();

    EXPECT_EQ(size_t(0), p->issueCount());

    EXPECT_TRUE(model->hasUnresolvedImports());
    importer->resolveImports(model, resourcePath());
    EXPECT_FALSE(model->hasUnresolvedImports());
}

TEST(Importer, resolveImportsFromFileLevel0Unresolvable)
{
    libcellml::ParserPtr p = libcellml::Parser::create();
    libcellml::ModelPtr model = p->parseModel(fileContents("level0-broken-imports.xml"));
    auto importer = libcellml::Importer::create();

    EXPECT_EQ(size_t(0), p->issueCount());

    EXPECT_TRUE(model->hasUnresolvedImports());
    importer->resolveImports(model, resourcePath());
    EXPECT_TRUE(model->hasUnresolvedImports());
}

TEST(Importer, componentNotInResolvingModel)
{
    const std::string modelImportingComponent =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" xmlns:cellml=\"http://www.cellml.org/cellml/2.0#\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" name=\"sin_approximations_import\" id=\"sin_approximations_import\">\n"
        "  <import xlink:href=\"level0.xml\">\n"
        "    <component name=\"real_component\" component_ref=\"this_name_not_a_component_in_level0_model\"/>\n"
        "  </import>\n"
        "  <component name=\"main\" id=\"main\"/>\n"
        "  <encapsulation>\n"
        "    <component_ref component=\"main\">\n"
        "      <component_ref component=\"real_component\"/>\n"
        "    </component_ref>\n"
        "  </encapsulation>\n"
        "</model>\n";

    libcellml::ParserPtr p = libcellml::Parser::create();
    auto importer = libcellml::Importer::create();
    libcellml::ModelPtr model = p->parseModel(modelImportingComponent);

    EXPECT_EQ(size_t(0), p->issueCount());

    EXPECT_TRUE(model->hasUnresolvedImports());
    importer->resolveImports(model, resourcePath());
    EXPECT_TRUE(model->hasUnresolvedImports());
}
