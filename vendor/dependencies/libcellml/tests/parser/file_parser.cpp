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

#include <algorithm>
#include <fstream>
#include <sstream>
#include <vector>

TEST(Parser, parseSineModelFromFile)
{
    libcellml::ParserPtr p = libcellml::Parser::create();
    p->parseModel(fileContents("sine_approximations.xml"));

    EXPECT_EQ(size_t(0), p->issueCount());
}

TEST(Parser, parseSineImportsModelFromFile)
{
    libcellml::ParserPtr p = libcellml::Parser::create();
    p->parseModel(fileContents("sine_approximations_import.xml"));

    EXPECT_EQ(size_t(0), p->issueCount());
}

TEST(Parser, parseInvalidModelFromFile)
{
    const std::vector<std::string> expectedIssues = {
        "LibXml2 error: Start tag expected, '<' not found.",
        "Could not get a valid XML root node from the provided input.",
    };

    libcellml::ParserPtr p = libcellml::Parser::create();
    p->parseModel(fileContents("invalid_cellml_2.0.xml"));

    EXPECT_EQ_ISSUES(expectedIssues, p);
}

TEST(Parser, parseOrdModelFromFile)
{
    libcellml::ParserPtr p = libcellml::Parser::create();
    libcellml::ModelPtr model = p->parseModel(fileContents("Ohara_Rudy_2011.cellml"));

    EXPECT_EQ(size_t(0), p->issueCount());

    // Test some random values.
    std::string a = model->component("intracellular_ions")->variable("BSLmax")->initialValue();
    EXPECT_EQ("1.124", a);

    a = model->component("INa")->variable("mtD2")->initialValue();
    EXPECT_EQ("8.552", a);

    a = model->component("IK1")->variable("GK1_b")->initialValue();
    EXPECT_EQ("0.1908", a);

    a = model->component("SERCA")->variable("Jup")->interfaceType();
    EXPECT_EQ("public", a);
}

TEST(Parser, parseComplexEncapsulationModelFromFile)
{
    // This test resulted from https://github.com/cellml/libcellml/issues/170
    libcellml::ParserPtr p = libcellml::Parser::create();
    p->parseModel(fileContents("complex_encapsulation.xml"));

    EXPECT_EQ(size_t(0), p->issueCount());
}

TEST(Parser, parseModelWithComponentsWithMultipleMathElements)
{
    // This test resulted from https://github.com/cellml/libcellml/issues/183
    const std::string e1 =
        "<math xmlns=\"http://www.w3.org/1998/Math/MathML\">\n"
        "\t\t\t<apply><eq/>\n"
        "\t\t\t\t<ci>a1</ci>\n"
        "\t\t\t\t<apply><plus/>\n"
        "\t\t\t\t\t<ci>b1</ci>\n"
        "\t\t\t\t\t<ci>c1</ci>\n"
        "\t\t\t\t</apply>\n"
        "\t\t\t</apply>\n"
        "\t\t</math>\n";
    const std::string e2 =
        "<math xmlns=\"http://www.w3.org/1998/Math/MathML\">\n"
        "\t\t\t<apply><eq/>\n"
        "\t\t\t\t<ci>b2</ci>\n"
        "\t\t\t\t<apply><times/>\n"
        "\t\t\t\t\t<cn xmlns:cellml=\"http://www.cellml.org/cellml/2.0#\" cellml:units=\"dimensionless\">2.0</cn>\n"
        "\t\t\t\t\t<ci>d</ci>\n"
        "\t\t\t\t</apply>\n"
        "\t\t\t</apply>\n"
        "\t\t</math>\n"
        "<math xmlns=\"http://www.w3.org/1998/Math/MathML\">\n"
        "\t\t\t<apply><eq/>\n"
        "\t\t\t\t<ci>d</ci>\n"
        "\t\t\t\t<cn xmlns:cellml=\"http://www.cellml.org/cellml/2.0#\" cellml:units=\"dimensionless\" type=\"e-notation\">0.5<sep/>1</cn>\n"
        "\t\t\t</apply>\n"
        "\t\t</math>\n";

    libcellml::ParserPtr p = libcellml::Parser::create();
    libcellml::ModelPtr model = p->parseModel(fileContents("a_plus_b.cellml"));
    EXPECT_EQ(size_t(0), p->issueCount());

    std::string a = model->component("c1")->math();
    EXPECT_EQ(e1, a);

    a = model->component("c2")->math();
    EXPECT_EQ(e2, a);
}

TEST(Parser, simpleGeneratorModel)
{
    const std::string e =
        "<math xmlns=\"http://www.w3.org/1998/Math/MathML\" xmlns:cellml=\"http://www.cellml.org/cellml/2.0#\">\n"
        "            <apply>\n"
        "                <eq/>\n"
        "                <apply>\n"
        "                    <diff/>\n"
        "                    <bvar>\n"
        "                        <ci>time</ci>\n"
        "                    </bvar>\n"
        "                    <ci>x</ci>\n"
        "                </apply>\n"
        "                <cn cellml:units=\"per_second\">1</cn>\n"
        "            </apply>\n"
        "        </math>\n";

    libcellml::ParserPtr p = libcellml::Parser::create();
    libcellml::ModelPtr model = p->parseModel(fileContents("analyser/initialised_variable_of_integration.cellml"));

    EXPECT_EQ(size_t(0), p->issueCount());

    std::string a = model->component("my_component")->math();
    EXPECT_EQ(e, a);
}

TEST(Parser, parseModelWithImportedEquivVariables)
{
    auto parser = libcellml::Parser::create();
    auto modelContents = fileContents("importingModel.cellml");
    auto model = parser->parseModel(modelContents);

    auto printer = libcellml::Printer::create();
    auto serialisedModel = printer->printModel(model);

    EXPECT_EQ(modelContents, serialisedModel);
}

TEST(Parser, unitsAreLinkedOnParse)
{
    libcellml::ParserPtr p = libcellml::Parser::create();
    auto model = p->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), p->issueCount());

    EXPECT_FALSE(model->hasUnlinkedUnits());
}
