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

TEST(ParserTransform, emptyCellml10)
{
    const std::string e = "Given model is a CellML 1.0 model, the parser will try to represent this model in CellML 2.0.";
    const std::string in =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/1.0#\"/>\n";

    libcellml::ParserPtr parser = libcellml::Parser::create(false);
    libcellml::ModelPtr model = parser->parseModel(in);
    EXPECT_EQ(size_t(1), parser->issueCount());
    EXPECT_EQ(libcellml::Issue::Level::MESSAGE, parser->issue(0)->level());
    EXPECT_EQ(e, parser->issue(0)->description());
}

TEST(ParserTransform, attemptToParseUnknownXmlModelWithPermissiveParser)
{
    const std::string e = "Model element is in an invalid namespace 'http://www.cellml.org/cellml/1.3#'.";
    const std::string in =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/1.3#\" name=\"name\"/>\n";

    libcellml::ParserPtr parser = libcellml::Parser::create(false);
    libcellml::ModelPtr model = parser->parseModel(in);

    EXPECT_EQ("", model->name());
    EXPECT_EQ(size_t(1), parser->issueCount());
    EXPECT_EQ(libcellml::Issue::Level::ERROR, parser->issue(0)->level());
    EXPECT_EQ(e, parser->issue(0)->description());
}

TEST(ParserTransform, attemptToParseUnknownXmlModelWith20Parser)
{
    const std::string e = "Model element is in an invalid namespace 'http://www.cellml.org/cellml/1.3#'. A valid CellML root node should be in the namespace 'http://www.cellml.org/cellml/2.0#'.";
    const std::string in =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/1.3#\" name=\"name\"/>\n";

    libcellml::ParserPtr parser = libcellml::Parser::create();
    libcellml::ModelPtr model = parser->parseModel(in);

    EXPECT_EQ("", model->name());
    EXPECT_EQ(size_t(1), parser->issueCount());
    EXPECT_EQ(libcellml::Issue::Level::ERROR, parser->issue(0)->level());
    EXPECT_EQ(e, parser->issue(0)->description());
}

TEST(ParserTransform, attemptToParseCellml1XModelWith20Parser)
{
    const std::string e = "Given model is a CellML 1.1 model but strict parsing mode is on.";
    const std::string in =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/1.1#\" name=\"name\"/>\n";

    libcellml::ParserPtr parser = libcellml::Parser::create();
    libcellml::ModelPtr model = parser->parseModel(in);

    EXPECT_EQ("", model->name());
    EXPECT_EQ(size_t(1), parser->issueCount());
    EXPECT_EQ(libcellml::Issue::Level::ERROR, parser->issue(0)->level());
    EXPECT_EQ(e, parser->issue(0)->description());
}

TEST(ParserTransform, parseNamedModelCellml10)
{
    const std::string n = "name";
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/1.0#\" name=\"name\"/>\n";

    libcellml::ParserPtr parser = libcellml::Parser::create(false);
    libcellml::ModelPtr model = parser->parseModel(e);
    EXPECT_EQ(n, model->name());
}

TEST(ParserTransform, hodgkinHuxleyCellml10)
{
    libcellml::ParserPtr parser = libcellml::Parser::create(false);
    auto model = parser->parseModel(fileContents("cellml1X/Hodgkin_Huxley_1952_modified.cellml"));

    EXPECT_EQ(size_t(7), model->unitsCount());
    EXPECT_EQ(size_t(5), model->componentCount());
    EXPECT_EQ(size_t(4), parser->issueCount());

    auto validator = libcellml::Validator::create();
    validator->validateModel(model);

    EXPECT_EQ(size_t(0), validator->issueCount());
}

TEST(ParserTransform, annotatedCellMl10Model)
{
    libcellml::ParserPtr parser = libcellml::Parser::create(false);
    auto model = parser->parseModel(fileContents("cellml1X/annotated_model.cellml"));

    EXPECT_EQ(size_t(5), model->unitsCount());
    EXPECT_EQ(size_t(2), model->componentCount());
    EXPECT_EQ(size_t(78), parser->issueCount());

    auto validator = libcellml::Validator::create();
    validator->validateModel(model);

    EXPECT_EQ(size_t(4), validator->issueCount());
}

TEST(ParserTransform, renameNonSiUnits)
{
    libcellml::ParserPtr parser = libcellml::Parser::create(false);
    auto model = parser->parseModel(fileContents("cellml1X/non_si_units.cellml"));

    EXPECT_EQ(size_t(11), model->unitsCount());
    EXPECT_EQ(size_t(2), model->componentCount());

    auto unitsMetre = model->units(0);
    auto unitsLitre = model->units(1);

    EXPECT_EQ("metre", unitsMetre->unitAttributeReference(0));
    EXPECT_EQ("litre", unitsLitre->unitAttributeReference(0));

    auto v1 = model->component(1)->variable(1);
    auto v2 = model->component(1)->variable(3);

    EXPECT_EQ("metre", v1->units()->name());
    EXPECT_EQ("litre", v2->units()->name());
}

TEST(ParserTransform, multipleMathInComponent)
{
    libcellml::ParserPtr parser = libcellml::Parser::create(false);
    auto model = parser->parseModel(fileContents("cellml1X/cardiac_constant_simplified.cellml"));

    auto validator = libcellml::Validator::create();

    validator->validateModel(model);

    EXPECT_EQ(size_t(0), validator->errorCount());
}

TEST(ParserTransform, multipleMathInComponentAlternativeFormulation)
{
    libcellml::ParserPtr parser = libcellml::Parser::create(false);
    auto model = parser->parseModel(fileContents("cellml1X/cardiac_constant_simplified_alternative.cellml"));

    auto validator = libcellml::Validator::create();

    validator->validateModel(model);

    EXPECT_EQ(size_t(0), validator->errorCount());
}
