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

TEST(ImportRequirement, create)
{
    auto model = libcellml::Model::create("model");

    auto requirements = model->importRequirements();
    EXPECT_EQ(size_t(0), requirements.size());
}

TEST(ImportRequirement, requirementsFixCircularRef)
{
    const std::vector<std::string> e = {
        "importExample2components.cellml",
        "circularImport1.cellml",
        "importExample2units.cellml",
    };

    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("import-requirements/importExample1.cellml"));

    auto requirements = model->importRequirements();
    EXPECT_EQ(e, requirements);
}

TEST(ImportRequirement, requirementsEncapsulationUnitsWithChildren)
{
    const std::vector<std::string> e = {
        "complicatedComponents.cellml",
        "complicatedUnits.cellml",
    };

    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("import-requirements/complicatedExample.cellml"));

    auto requirements = model->importRequirements();
    EXPECT_EQ(e, requirements);
}

TEST(ImportRequirement, requirementsRepeatedImports)
{
    const std::vector<std::string> e = {
        "importExample2components.cellml",
        "importExample2units.cellml",
    };

    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("import-requirements/importExample2.cellml"));

    auto requirements = model->importRequirements();
    EXPECT_EQ(e, requirements);
}
