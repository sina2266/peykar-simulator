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

#include "libxml2issues.h"

TEST(Maths, setAndGetMath)
{
    libcellml::ComponentPtr c = libcellml::Component::create();
    c->setMath(EMPTY_MATH);
    EXPECT_EQ(EMPTY_MATH, c->math());
}

TEST(Maths, appendAndSerialiseMathComponent)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component>\n"
        "    <math xmlns=\"http://www.w3.org/1998/Math/MathML\"/>\n"
        "  </component>\n"
        "</model>\n";

    libcellml::ModelPtr m = createModelWithComponent();
    libcellml::ComponentPtr c = m->component(0);
    c->appendMath(EMPTY_MATH);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

TEST(Maths, appendAndRemoveMathFromComponent)
{
    const std::string eNoMath =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component/>\n"
        "</model>\n";
    const std::string eWithMath =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component>\n"
        "    <math xmlns=\"http://www.w3.org/1998/Math/MathML\"/>\n"
        "  </component>\n"
        "</model>\n";
    libcellml::ModelPtr m = createModelWithComponent();
    libcellml::ComponentPtr c = m->component(0);
    libcellml::PrinterPtr printer = libcellml::Printer::create();

    c->appendMath(EMPTY_MATH);
    std::string a = printer->printModel(m);
    EXPECT_EQ(eWithMath, a);

    c->removeMath();
    a = printer->printModel(m);
    EXPECT_EQ(eNoMath, a);

    c->appendMath(EMPTY_MATH);
    a = printer->printModel(m);
    EXPECT_EQ(eWithMath, a);

    c->setMath("");
    a = printer->printModel(m);
    EXPECT_EQ(eNoMath, a);
}

TEST(Maths, appendSerialiseAndParseMathInComponent)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component>\n"
        "    <math xmlns=\"http://www.w3.org/1998/Math/MathML\"/>\n"
        "  </component>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c = libcellml::Component::create();
    m->addComponent(c);
    c->appendMath(EMPTY_MATH);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);

    // Parse
    libcellml::ParserPtr parser = libcellml::Parser::create();
    libcellml::ModelPtr model = parser->parseModel(e);
    a = printer->printModel(model);
    EXPECT_EQ(e, a);
}

TEST(Maths, modelWithTwoVariablesWithInitialValuesAndInvalidMath)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"component\">\n"
        "    <variable name=\"variable1\" initial_value=\"1.0\"/>\n"
        "    <variable name=\"variable2\" initial_value=\"-1.0\"/>\n"
        "    <math xmlns=\"http://www.w3.org/1998/Math/MathML\"/>\n"
        "  </component>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c = libcellml::Component::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    c->setName("component");
    v1->setName("variable1");
    v2->setName("variable2");
    v1->setInitialValue("1.0");
    v2->setInitialValue("-1.0");
    c->addVariable(v1);
    c->addVariable(v2);
    c->appendMath(EMPTY_MATH);
    m->addComponent(c);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

// 1.xiii.a
TEST(Maths, modelWithTwoVariablesWithInitialValuesAndValidMath)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"component\">\n"
        "    <variable name=\"A\" initial_value=\"1.0\"/>\n"
        "    <variable name=\"B\" initial_value=\"-1.0\"/>\n"
        "    <math xmlns=\"http://www.w3.org/1998/Math/MathML\">\n"
        "      <apply>\n"
        "        <eq/>\n"
        "        <ci>C</ci>\n"
        "        <apply>\n"
        "          <plus/>\n"
        "          <ci>A</ci>\n"
        "          <ci>B</ci>\n"
        "        </apply>\n"
        "      </apply>\n"
        "    </math>\n"
        "  </component>\n"
        "</model>\n";
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

    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c = libcellml::Component::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    c->setName("component");
    v1->setName("A");
    v2->setName("B");
    v1->setInitialValue("1.0");
    v2->setInitialValue("-1.0");
    c->addVariable(v1);
    c->addVariable(v2);
    c->appendMath(math);
    m->addComponent(c);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

// 1.xiv.a
TEST(Maths, twoComponentsWithMathAndConnectionAndParse)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"component1\">\n"
        "    <variable name=\"A1\"/>\n"
        "    <variable name=\"B1\"/>\n"
        "    <math xmlns=\"http://www.w3.org/1998/Math/MathML\">\n"
        "      <apply>\n"
        "        <eq/>\n"
        "        <ci>C1</ci>\n"
        "        <apply>\n"
        "          <plus/>\n"
        "          <ci>A1</ci>\n"
        "          <ci>B1</ci>\n"
        "        </apply>\n"
        "      </apply>\n"
        "    </math>\n"
        "  </component>\n"
        "  <component name=\"component2\">\n"
        "    <variable name=\"A2\"/>\n"
        "    <variable name=\"B2\"/>\n"
        "    <math xmlns=\"http://www.w3.org/1998/Math/MathML\">\n"
        "      <apply>\n"
        "        <eq/>\n"
        "        <ci>C2</ci>\n"
        "        <apply>\n"
        "          <plus/>\n"
        "          <ci>A2</ci>\n"
        "          <ci>B2</ci>\n"
        "        </apply>\n"
        "      </apply>\n"
        "    </math>\n"
        "  </component>\n"
        "  <connection component_1=\"component1\" component_2=\"component2\">\n"
        "    <map_variables variable_1=\"A1\" variable_2=\"A2\"/>\n"
        "  </connection>\n"
        "</model>\n";
    const std::string math1 =
        "<math xmlns=\"http://www.w3.org/1998/Math/MathML\">\n"
        "  <apply>\n"
        "    <eq/>\n"
        "    <ci>C1</ci>\n"
        "    <apply>\n"
        "      <plus/>\n"
        "      <ci>A1</ci>\n"
        "      <ci>B1</ci>\n"
        "    </apply>\n"
        "  </apply>\n"
        "</math>\n";
    const std::string math2 =
        "<math xmlns=\"http://www.w3.org/1998/Math/MathML\">\n"
        "  <apply>\n"
        "    <eq/>\n"
        "    <ci>C2</ci>\n"
        "    <apply>\n"
        "      <plus/>\n"
        "      <ci>A2</ci>\n"
        "      <ci>B2</ci>\n"
        "    </apply>\n"
        "  </apply>\n"
        "</math>\n";

    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr comp1 = libcellml::Component::create();
    libcellml::ComponentPtr comp2 = libcellml::Component::create();
    libcellml::VariablePtr v11 = libcellml::Variable::create();
    libcellml::VariablePtr v12 = libcellml::Variable::create();
    libcellml::VariablePtr v21 = libcellml::Variable::create();
    libcellml::VariablePtr v22 = libcellml::Variable::create();

    comp1->setName("component1");
    comp2->setName("component2");
    v11->setName("A1");
    v12->setName("B1");
    v21->setName("A2");
    v22->setName("B2");

    comp1->addVariable(v11);
    comp1->addVariable(v12);
    comp2->addVariable(v21);
    comp2->addVariable(v22);
    comp1->appendMath(math1);
    comp2->appendMath(math2);
    m->addComponent(comp1);
    m->addComponent(comp2);
    libcellml::Variable::addEquivalence(v11, v21);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);

    // Parse
    libcellml::ParserPtr parser = libcellml::Parser::create();
    libcellml::ModelPtr model = parser->parseModel(e);
    a = printer->printModel(model);
    EXPECT_EQ(e, a);
}

TEST(Printer, addMathMLAsCompleteXMLDoc)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"model\">\n"
        "  <component name=\"component\">\n"
        "    <math xmlns=\"http://www.w3.org/1998/Math/MathML\">\n"
        "      <apply>\n"
        "        <divide/>\n"
        "        <ci>eff</ci>\n"
        "        <ci>t_ave</ci>\n"
        "      </apply>\n"
        "    </math>\n"
        "  </component>\n"
        "</model>\n";
    const std::string math =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<math xmlns=\"http://www.w3.org/1998/Math/MathML\">\n"
        "  <apply>\n"
        "    <divide/>\n"
        "    <ci> eff </ci>\n"
        "    <ci> t_ave </ci>\n"
        "  </apply>\n"
        "</math>\n";

    auto printer = libcellml::Printer::create();
    auto model = libcellml::Model::create();
    model->setName("model");

    auto component = libcellml::Component::create("component");
    model->addComponent(component);
    component->setMath(math);
    EXPECT_EQ(e, printer->printModel(model));
}

void compareLibXml2Issues(const libcellml::PrinterPtr &printer)
{
    EXPECT_EQ(expectedLibXml2Issues.size(), printer->issueCount());
    for (size_t i = 0; i < printer->issueCount(); ++i) {
        EXPECT_EQ(expectedLibXml2Issues.at(i), printer->issue(i)->description());
    }
}
TEST(Printer, mathMLWithSyntaxError)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"model\">\n"
        "  <component name=\"component\"/>\n"
        "</model>\n";
    const std::string math =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<math xmlns=\"http://www.w3.org/1998/Math/MathML\">\n"
        "  <apply>\n"
        "    <divide/>\n"
        "    <ci> eff </ci>\n"
        "    <ci> t_ave <ci>\n"
        "  </apply>\n"
        "</math>\n";

    auto printer = libcellml::Printer::create();
    auto model = libcellml::Model::create();
    model->setName("model");

    auto component = libcellml::Component::create("component");
    model->addComponent(component);
    component->setMath(math);

    EXPECT_EQ(e, printer->printModel(model));

    compareLibXml2Issues(printer);

    auto itemComponent = printer->issue(printer->issueCount() - 1)->item()->component();
    EXPECT_NE(nullptr, itemComponent);
    EXPECT_EQ("component", itemComponent->name());
}

TEST(Printer, mathMLInResetWithSyntaxError)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"model\">\n"
        "  <component name=\"component\">\n"
        "    <reset order=\"5\">\n"
        "      <reset_value/>\n"
        "    </reset>\n"
        "  </component>\n"
        "</model>\n";
    const std::string math =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<math xmlns=\"http://www.w3.org/1998/Math/MathML\">\n"
        "  <apply>\n"
        "    <divide/>\n"
        "    <ci> eff </ci>\n"
        "    <ci> t_ave <ci>\n"
        "  </apply>\n"
        "</math>\n";

    auto printer = libcellml::Printer::create();
    auto model = libcellml::Model::create();
    model->setName("model");

    auto component = libcellml::Component::create("component");
    model->addComponent(component);
    auto reset = libcellml::Reset::create();
    reset->setResetValue(math);
    reset->setOrder(5);
    component->addReset(reset);

    EXPECT_EQ(e, printer->printModel(model));

    compareLibXml2Issues(printer);

    auto itemReset = printer->issue(printer->issueCount() - 1)->item()->reset();
    EXPECT_NE(nullptr, itemReset);
    EXPECT_EQ(5, itemReset->order());
}
