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

const std::vector<std::string> expectedParserIssues = {
    "Given model is a CellML 1.1 model, the parser will try to represent this model in CellML 2.0.",
    "Model 'sin_approximations_import' ignoring child element 'simulation'.",
    "Model 'sin_approximations_import' ignoring child element 'RDF'.",
};

TEST(ImporterStrict, sineCellml11Import)
{
    const std::vector<std::string> expectedImporterIssues = {
        "Given model is a CellML 1.1 model, the parser will try to represent this model in CellML 2.0.",
        "Given model is a CellML 1.1 model, the parser will try to represent this model in CellML 2.0.",
        "Given model is a CellML 1.1 model, the parser will try to represent this model in CellML 2.0.",
    };
    const std::vector<std::string> expectedImporterIssueUrls = {
        "sin.xml",
        "deriv_approx_sin.xml",
        "parabolic_approx_sin.xml",
    };

    libcellml::ParserPtr parser = libcellml::Parser::create(false);
    libcellml::ImporterPtr importer = libcellml::Importer::create(false);
    auto model = parser->parseModel(fileContents("cellml1X/sin_approximations_import.xml"));

    EXPECT_EQ(size_t(3), parser->issueCount());
    EXPECT_EQ_ISSUES(expectedParserIssues, parser);
    EXPECT_TRUE(model->hasUnresolvedImports());

    importer->resolveImports(model, resourcePath("cellml1X"));
    EXPECT_EQ(size_t(3), importer->issueCount());
    EXPECT_EQ_ISSUES(expectedImporterIssues, importer);
    EXPECT_EQ(importer->issueCount(), importer->messageCount());
    for (size_t i = 0; i < importer->messageCount(); ++i) {
        auto issue = importer->message(i);
        EXPECT_EQ(expectedImporterIssueUrls.at(i), issue->item()->importSource()->url());
    }
    EXPECT_FALSE(model->hasUnresolvedImports());
}

TEST(ImporterStrict, sineCellml11ImportInStrictMode)
{
    const std::vector<std::string> expectedImporterIssues = {
        "Import of component 'actual_sin' from '" + resourcePath("cellml1X/sin.xml") + "' requires component named 'sin' which cannot be found.",
        "Import of component 'deriv_approx_sin' from '" + resourcePath("cellml1X/deriv_approx_sin.xml") + "' requires component named 'sin' which cannot be found.",
        "Import of component 'parabolic_approx_sin' from '" + resourcePath("cellml1X/parabolic_approx_sin.xml") + "' requires component named 'sin' which cannot be found.",
    };

    libcellml::ParserPtr parser = libcellml::Parser::create(false);
    libcellml::ImporterPtr importer = libcellml::Importer::create();
    auto model = parser->parseModel(fileContents("cellml1X/sin_approximations_import.xml"));

    EXPECT_EQ(size_t(3), parser->issueCount());
    EXPECT_EQ_ISSUES(expectedParserIssues, parser);
    EXPECT_TRUE(model->hasUnresolvedImports());

    importer->resolveImports(model, resourcePath("cellml1X"));
    EXPECT_EQ(size_t(3), importer->issueCount());
    EXPECT_EQ_ISSUES(expectedImporterIssues, importer);
    EXPECT_TRUE(model->hasUnresolvedImports());
}
