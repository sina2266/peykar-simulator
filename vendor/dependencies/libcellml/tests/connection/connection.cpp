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

#include "test_utils.h"
#include <libcellml>

TEST(Connection, addEquivalenceNullptrFirstParameter)
{
    libcellml::VariablePtr v1 = nullptr;
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    libcellml::Variable::addEquivalence(v1, v2);
    EXPECT_EQ(size_t(0), v2->equivalentVariableCount());
    EXPECT_FALSE(v2->hasEquivalentVariable(v1));
}

TEST(Connection, addEquivalenceNullptrSecondParameter)
{
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = nullptr;
    libcellml::Variable::addEquivalence(v1, v2);
    EXPECT_EQ(size_t(0), v1->equivalentVariableCount());
    EXPECT_FALSE(v1->hasEquivalentVariable(v2));
}

TEST(Connection, addEquivalenceNullptrBothParameters)
{
    libcellml::VariablePtr v1 = nullptr;
    libcellml::VariablePtr v2 = nullptr;
    libcellml::Variable::addEquivalence(v1, v2);
}

TEST(Connection, addAndGetEquivalentVariable)
{
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    libcellml::Variable::addEquivalence(v1, v2);
    EXPECT_EQ(v2, v1->equivalentVariable(0));
}

TEST(Connection, addAndGetEquivalentVariableReciprocal)
{
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    libcellml::Variable::addEquivalence(v1, v2);
    EXPECT_EQ(v1, v2->equivalentVariable(0));
}

TEST(Connection, addTwoEquivalentVariablesAndCount)
{
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    libcellml::VariablePtr v3 = libcellml::Variable::create();
    libcellml::Variable::addEquivalence(v1, v2);
    libcellml::Variable::addEquivalence(v1, v3);
    const size_t e = 2;
    size_t a = v1->equivalentVariableCount();
    EXPECT_EQ(e, a);
}

TEST(Connection, addDuplicateEquivalentVariablesAndCount)
{
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    libcellml::Variable::addEquivalence(v1, v2);
    libcellml::Variable::addEquivalence(v1, v2);
    libcellml::Variable::addEquivalence(v2, v1);
    libcellml::Variable::addEquivalence(v2, v1);
    const size_t e = 1;
    size_t a = v1->equivalentVariableCount();
    EXPECT_EQ(e, a);
}

TEST(Connection, hasNoEquivalentVariable)
{
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    EXPECT_FALSE(v1->hasEquivalentVariable(v2, true));

    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c = libcellml::Component::create();

    libcellml::Variable::addEquivalence(v1, v2);
    c->addVariable(v1);
    c->addVariable(v2);

    m->addComponent(c);
    EXPECT_TRUE(v1->hasEquivalentVariable(v2, true));

    c->removeVariable(v2);
    EXPECT_TRUE(v1->hasEquivalentVariable(v2, true));

    v2.reset();
    EXPECT_FALSE(v1->hasEquivalentVariable(v2, true));
}

TEST(Connection, hasIndirectEquivalentVariable)
{
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    libcellml::VariablePtr v3 = libcellml::Variable::create();
    libcellml::Variable::addEquivalence(v1, v2);
    libcellml::Variable::addEquivalence(v2, v3);
    EXPECT_TRUE(v1->hasEquivalentVariable(v3, true));
}

TEST(Connection, connectionId)
{
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    libcellml::Variable::addEquivalence(v1, v2);

    EXPECT_EQ("", libcellml::Variable::equivalenceConnectionId(v1, v2));

    libcellml::Variable::setEquivalenceConnectionId(v1, v2, "connection_id");
    EXPECT_EQ("connection_id", libcellml::Variable::equivalenceConnectionId(v1, v2));
}

TEST(Connection, componentlessVariableInvalidConnection)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"component1\">\n"
        "    <variable name=\"variable1\"/>\n"
        "  </component>\n"
        "  <connection component_1=\"component1\">\n"
        "    <map_variables variable_1=\"variable1\" variable_2=\"variable2\"/>\n"
        "  </connection>\n"
        "</model>\n";
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr comp1 = libcellml::Component::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    comp1->setName("component1");
    v1->setName("variable1");
    v2->setName("variable2");
    comp1->addVariable(v1);
    m->addComponent(comp1);
    libcellml::Variable::addEquivalence(v1, v2);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

TEST(Connection, componentlessVariableInvalidConnectionRemoveParentCheck)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"component2\">\n"
        "    <variable name=\"variable2\"/>\n"
        "  </component>\n"
        "  <connection component_1=\"component2\">\n"
        "    <map_variables variable_1=\"variable2\" variable_2=\"variable1\"/>\n"
        "  </connection>\n"
        "</model>\n";
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr comp2 = libcellml::Component::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    comp2->setName("component2");
    v1->setName("variable1");
    v2->setName("variable2");
    comp2->addVariable(v2);
    {
        // Place comp1 in a different scope.
        libcellml::ComponentPtr comp1 = libcellml::Component::create();
        comp1->setName("component1");
        comp1->addVariable(v1);
        m->addComponent(comp1);
    }
    m->addComponent(comp2);
    libcellml::Variable::addEquivalence(v1, v2);
    m->removeComponent("component1");
    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

TEST(Connection, validConnectionAndParse)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"component1\">\n"
        "    <variable name=\"variable1\"/>\n"
        "  </component>\n"
        "  <component name=\"component2\">\n"
        "    <variable name=\"variable2\"/>\n"
        "  </component>\n"
        "  <connection component_1=\"component1\" component_2=\"component2\">\n"
        "    <map_variables variable_1=\"variable1\" variable_2=\"variable2\"/>\n"
        "  </connection>\n"
        "</model>\n";
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr comp1 = libcellml::Component::create();
    libcellml::ComponentPtr comp2 = libcellml::Component::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    comp1->setName("component1");
    comp2->setName("component2");
    v1->setName("variable1");
    v2->setName("variable2");
    comp1->addVariable(v1);
    comp2->addVariable(v2);
    m->addComponent(comp1);
    m->addComponent(comp2);
    libcellml::Variable::addEquivalence(v1, v2);
    libcellml::PrinterPtr printer = libcellml::Printer::create();
    std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);

    // Parse
    libcellml::ParserPtr parser = libcellml::Parser::create();
    libcellml::ModelPtr model = parser->parseModel(e);
    a = printer->printModel(model);
    EXPECT_EQ(e, a);
}

TEST(Connection, parseValidAlternateFormConnection)
{
    const std::string in =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model name=\"\" xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <connection component_1=\"component1\" component_2=\"component2\">\n"
        "    <map_variables variable_1=\"variable1\" variable_2=\"variable2\"/>\n"
        "  </connection>\n"
        "  <component name=\"component1\">\n"
        "    <variable name=\"variable1\" units=\"dimensionless\"/>\n"
        "  </component>\n"
        "  <component name=\"component2\">\n"
        "    <variable name=\"variable2\" units=\"dimensionless\"/>\n"
        "  </component>\n"
        "</model>\n";

    // Parse
    libcellml::ParserPtr parser = libcellml::Parser::create();
    libcellml::ModelPtr model = parser->parseModel(in);

    EXPECT_EQ(size_t(0), parser->issueCount());
    EXPECT_EQ(size_t(2), model->componentCount());
    EXPECT_EQ(size_t(1), model->component("component1")->variable("variable1")->equivalentVariableCount());
}

TEST(Connection, twoMapVariablesConnection)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"component1\">\n"
        "    <variable name=\"variable11\"/>\n"
        "    <variable name=\"variable12\"/>\n"
        "  </component>\n"
        "  <component name=\"component2\">\n"
        "    <variable name=\"variable21\"/>\n"
        "    <variable name=\"variable22\"/>\n"
        "  </component>\n"
        "  <connection component_1=\"component1\" component_2=\"component2\">\n"
        "    <map_variables variable_1=\"variable11\" variable_2=\"variable21\"/>\n"
        "    <map_variables variable_1=\"variable12\" variable_2=\"variable22\"/>\n"
        "  </connection>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr comp1 = libcellml::Component::create();
    libcellml::ComponentPtr comp2 = libcellml::Component::create();
    libcellml::VariablePtr v11 = libcellml::Variable::create();
    libcellml::VariablePtr v12 = libcellml::Variable::create();
    libcellml::VariablePtr v21 = libcellml::Variable::create();
    libcellml::VariablePtr v22 = libcellml::Variable::create();

    comp1->setName("component1");
    comp2->setName("component2");
    v11->setName("variable11");
    v12->setName("variable12");
    v21->setName("variable21");
    v22->setName("variable22");

    comp1->addVariable(v11);
    comp1->addVariable(v12);
    comp2->addVariable(v21);
    comp2->addVariable(v22);
    m->addComponent(comp1);
    m->addComponent(comp2);
    libcellml::Variable::addEquivalence(v11, v21);
    libcellml::Variable::addEquivalence(v12, v22);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

TEST(Connection, threeMapVariablesConnectionOneDuplicate)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"component1\">\n"
        "    <variable name=\"variable11\"/>\n"
        "    <variable name=\"variable12\"/>\n"
        "    <variable name=\"variable13\"/>\n"
        "  </component>\n"
        "  <component name=\"component2\">\n"
        "    <variable name=\"variable21\"/>\n"
        "    <variable name=\"variable22\"/>\n"
        "    <variable name=\"variable23\"/>\n"
        "  </component>\n"
        "  <connection component_1=\"component1\" component_2=\"component2\">\n"
        "    <map_variables variable_1=\"variable11\" variable_2=\"variable21\"/>\n"
        "    <map_variables variable_1=\"variable12\" variable_2=\"variable22\"/>\n"
        "    <map_variables variable_1=\"variable13\" variable_2=\"variable23\"/>\n"
        "  </connection>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr comp1 = libcellml::Component::create();
    libcellml::ComponentPtr comp2 = libcellml::Component::create();
    libcellml::VariablePtr v11 = libcellml::Variable::create();
    libcellml::VariablePtr v12 = libcellml::Variable::create();
    libcellml::VariablePtr v13 = libcellml::Variable::create();
    libcellml::VariablePtr v21 = libcellml::Variable::create();
    libcellml::VariablePtr v22 = libcellml::Variable::create();
    libcellml::VariablePtr v23 = libcellml::Variable::create();

    comp1->setName("component1");
    comp2->setName("component2");
    v11->setName("variable11");
    v12->setName("variable12");
    v13->setName("variable13");
    v21->setName("variable21");
    v22->setName("variable22");
    v23->setName("variable23");

    comp1->addVariable(v11);
    comp1->addVariable(v12);
    comp1->addVariable(v13);
    comp2->addVariable(v21);
    comp2->addVariable(v22);
    comp2->addVariable(v23);
    m->addComponent(comp1);
    m->addComponent(comp2);

    libcellml::Variable::addEquivalence(v23, v13);
    libcellml::Variable::addEquivalence(v21, v11);
    libcellml::Variable::addEquivalence(v12, v22);
    libcellml::Variable::addEquivalence(v13, v23);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

TEST(Connection, nineVariablesTenConnectionsAndParse)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"component1\">\n"
        "    <variable name=\"variable11\"/>\n"
        "    <variable name=\"variable12\"/>\n"
        "    <variable name=\"variable13\"/>\n"
        "  </component>\n"
        "  <component name=\"component2\">\n"
        "    <variable name=\"variable21\"/>\n"
        "    <variable name=\"variable22\"/>\n"
        "    <variable name=\"variable23\"/>\n"
        "  </component>\n"
        "  <component name=\"component3\">\n"
        "    <variable name=\"variable31\"/>\n"
        "    <variable name=\"variable32\"/>\n"
        "    <variable name=\"variable33\"/>\n"
        "  </component>\n"
        "  <connection component_1=\"component1\" component_2=\"component2\">\n"
        "    <map_variables variable_1=\"variable11\" variable_2=\"variable21\"/>\n"
        "    <map_variables variable_1=\"variable12\" variable_2=\"variable22\"/>\n"
        "    <map_variables variable_1=\"variable13\" variable_2=\"variable23\"/>\n"
        "  </connection>\n"
        "  <connection component_1=\"component1\" component_2=\"component3\">\n"
        "    <map_variables variable_1=\"variable11\" variable_2=\"variable31\"/>\n"
        "    <map_variables variable_1=\"variable11\" variable_2=\"variable33\"/>\n"
        "    <map_variables variable_1=\"variable12\" variable_2=\"variable32\"/>\n"
        "    <map_variables variable_1=\"variable13\" variable_2=\"variable33\"/>\n"
        "  </connection>\n"
        "  <connection component_1=\"component2\" component_2=\"component3\">\n"
        "    <map_variables variable_1=\"variable21\" variable_2=\"variable33\"/>\n"
        "    <map_variables variable_1=\"variable23\" variable_2=\"variable31\"/>\n"
        "    <map_variables variable_1=\"variable23\" variable_2=\"variable33\"/>\n"
        "  </connection>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr comp1 = libcellml::Component::create();
    libcellml::ComponentPtr comp2 = libcellml::Component::create();
    libcellml::ComponentPtr comp3 = libcellml::Component::create();
    libcellml::VariablePtr v11 = libcellml::Variable::create();
    libcellml::VariablePtr v12 = libcellml::Variable::create();
    libcellml::VariablePtr v13 = libcellml::Variable::create();
    libcellml::VariablePtr v21 = libcellml::Variable::create();
    libcellml::VariablePtr v22 = libcellml::Variable::create();
    libcellml::VariablePtr v23 = libcellml::Variable::create();
    libcellml::VariablePtr v31 = libcellml::Variable::create();
    libcellml::VariablePtr v32 = libcellml::Variable::create();
    libcellml::VariablePtr v33 = libcellml::Variable::create();

    comp1->setName("component1");
    comp2->setName("component2");
    comp3->setName("component3");
    v11->setName("variable11");
    v12->setName("variable12");
    v13->setName("variable13");
    v21->setName("variable21");
    v22->setName("variable22");
    v23->setName("variable23");
    v31->setName("variable31");
    v32->setName("variable32");
    v33->setName("variable33");

    comp1->addVariable(v11);
    comp1->addVariable(v12);
    comp1->addVariable(v13);
    comp2->addVariable(v21);
    comp2->addVariable(v22);
    comp2->addVariable(v23);
    comp3->addVariable(v31);
    comp3->addVariable(v32);
    comp3->addVariable(v33);
    m->addComponent(comp1);
    m->addComponent(comp2);
    m->addComponent(comp3);

    libcellml::Variable::addEquivalence(v11, v21);
    libcellml::Variable::addEquivalence(v31, v11);
    libcellml::Variable::addEquivalence(v12, v22);
    libcellml::Variable::addEquivalence(v32, v12);
    libcellml::Variable::addEquivalence(v13, v23);
    libcellml::Variable::addEquivalence(v33, v13);
    libcellml::Variable::addEquivalence(v31, v23);
    libcellml::Variable::addEquivalence(v21, v33);
    libcellml::Variable::addEquivalence(v11, v33);
    libcellml::Variable::addEquivalence(v33, v23);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);

    // Parse
    libcellml::ParserPtr parser = libcellml::Parser::create();
    libcellml::ModelPtr model = parser->parseModel(e);
    a = printer->printModel(model);
    EXPECT_EQ(e, a);
}

TEST(Connection, twoValidConnections)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"component1\">\n"
        "    <variable name=\"variable1\"/>\n"
        "  </component>\n"
        "  <component name=\"component2\">\n"
        "    <variable name=\"variable2\"/>\n"
        "  </component>\n"
        "  <component name=\"component3\">\n"
        "    <variable name=\"variable3\"/>\n"
        "  </component>\n"
        "  <connection component_1=\"component1\" component_2=\"component2\">\n"
        "    <map_variables variable_1=\"variable1\" variable_2=\"variable2\"/>\n"
        "  </connection>\n"
        "  <connection component_1=\"component1\" component_2=\"component3\">\n"
        "    <map_variables variable_1=\"variable1\" variable_2=\"variable3\"/>\n"
        "  </connection>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr comp1 = libcellml::Component::create();
    libcellml::ComponentPtr comp2 = libcellml::Component::create();
    libcellml::ComponentPtr comp3 = libcellml::Component::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    libcellml::VariablePtr v3 = libcellml::Variable::create();
    comp1->setName("component1");
    comp2->setName("component2");
    comp3->setName("component3");
    v1->setName("variable1");
    v2->setName("variable2");
    v3->setName("variable3");
    comp1->addVariable(v1);
    comp2->addVariable(v2);
    comp3->addVariable(v3);
    m->addComponent(comp1);
    m->addComponent(comp2);
    m->addComponent(comp3);
    libcellml::Variable::addEquivalence(v1, v2);
    libcellml::Variable::addEquivalence(v1, v3);
    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

TEST(Connection, removeEquivalentVariableMethods)
{
    const std::string e1 =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"component1\">\n"
        "    <variable name=\"variable1\"/>\n"
        "  </component>\n"
        "  <component name=\"component2\">\n"
        "    <variable name=\"variable2\"/>\n"
        "  </component>\n"
        "  <component name=\"component3\">\n"
        "    <variable name=\"variable3\"/>\n"
        "  </component>\n"
        "  <connection component_1=\"component1\" component_2=\"component2\">\n"
        "    <map_variables variable_1=\"variable1\" variable_2=\"variable2\"/>\n"
        "  </connection>\n"
        "  <connection component_1=\"component1\" component_2=\"component3\" id=\"con2Id\">\n"
        "    <map_variables variable_1=\"variable1\" variable_2=\"variable3\"/>\n"
        "  </connection>\n"
        "  <connection component_1=\"component2\" component_2=\"component3\" id=\"con1Id\">\n"
        "    <map_variables variable_1=\"variable2\" variable_2=\"variable3\" id=\"mapId\"/>\n"
        "  </connection>\n"
        "</model>\n";
    const std::string e2 =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"component1\">\n"
        "    <variable name=\"variable1\"/>\n"
        "  </component>\n"
        "  <component name=\"component2\">\n"
        "    <variable name=\"variable2\"/>\n"
        "  </component>\n"
        "  <component name=\"component3\">\n"
        "    <variable name=\"variable3\"/>\n"
        "  </component>\n"
        "  <connection component_1=\"component1\" component_2=\"component2\">\n"
        "    <map_variables variable_1=\"variable1\" variable_2=\"variable2\"/>\n"
        "  </connection>\n"
        "  <connection component_1=\"component1\" component_2=\"component3\" id=\"con2Id\">\n"
        "    <map_variables variable_1=\"variable1\" variable_2=\"variable3\"/>\n"
        "  </connection>\n"
        "</model>\n";
    const std::string e3 =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"component1\">\n"
        "    <variable name=\"variable1\"/>\n"
        "  </component>\n"
        "  <component name=\"component2\">\n"
        "    <variable name=\"variable2\"/>\n"
        "  </component>\n"
        "  <component name=\"component3\">\n"
        "    <variable name=\"variable3\"/>\n"
        "  </component>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr comp1 = libcellml::Component::create();
    libcellml::ComponentPtr comp2 = libcellml::Component::create();
    libcellml::ComponentPtr comp3 = libcellml::Component::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    libcellml::VariablePtr v3 = libcellml::Variable::create();
    libcellml::VariablePtr v4 = libcellml::Variable::create();
    comp1->setName("component1");
    comp2->setName("component2");
    comp3->setName("component3");
    v1->setName("variable1");
    v2->setName("variable2");
    v3->setName("variable3");

    comp1->addVariable(v1);
    comp2->addVariable(v2);
    comp3->addVariable(v3);
    m->addComponent(comp1);
    m->addComponent(comp2);
    m->addComponent(comp3);
    libcellml::Variable::addEquivalence(v1, v2);
    libcellml::Variable::addEquivalence(v1, v3);
    libcellml::Variable::setEquivalenceConnectionId(v1, v3, "con2Id");
    libcellml::Variable::addEquivalence(v2, v3, "mapId", "con1Id");
    libcellml::PrinterPtr printer = libcellml::Printer::create();
    std::string a = printer->printModel(m);
    EXPECT_EQ(e1, a);

    EXPECT_TRUE(libcellml::Variable::removeEquivalence(v2, v3));
    a = printer->printModel(m);
    EXPECT_EQ(e2, a);
    EXPECT_FALSE(libcellml::Variable::removeEquivalence(v3, v4));

    v1->removeAllEquivalences();
    a = printer->printModel(m);
    EXPECT_EQ(e3, a);
}

TEST(Connection, setMappingIdForConnection)
{
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr comp1 = libcellml::Component::create();
    libcellml::ComponentPtr comp2 = libcellml::Component::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    comp1->setName("component1");
    comp2->setName("component2");
    v1->setName("variable1");
    v2->setName("variable2");

    comp1->addVariable(v1);
    comp2->addVariable(v2);
    m->addComponent(comp1);
    m->addComponent(comp2);

    libcellml::Variable::setEquivalenceMappingId(v1, v2, "mappingId");
    EXPECT_EQ("", libcellml::Variable::equivalenceMappingId(v1, v2));
    libcellml::Variable::setEquivalenceMappingId(v1, nullptr, "mappingId");
    EXPECT_EQ("", libcellml::Variable::equivalenceMappingId(v1, nullptr));
    libcellml::Variable::setEquivalenceMappingId(nullptr, v2, "mappingId");
    EXPECT_EQ("", libcellml::Variable::equivalenceMappingId(nullptr, v2));
    libcellml::Variable::setEquivalenceMappingId(nullptr, nullptr, "mappingId");
    EXPECT_EQ("", libcellml::Variable::equivalenceMappingId(nullptr, nullptr));

    libcellml::Variable::addEquivalence(v1, v2);

    libcellml::Variable::setEquivalenceMappingId(v1, v2, "mappingId");
    EXPECT_EQ("mappingId", libcellml::Variable::equivalenceMappingId(v1, v2));
}

TEST(Connection, setConnectionIdForConnection)
{
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr comp1 = libcellml::Component::create();
    libcellml::ComponentPtr comp2 = libcellml::Component::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    comp1->setName("component1");
    comp2->setName("component2");
    v1->setName("variable1");
    v2->setName("variable2");

    comp1->addVariable(v1);
    comp2->addVariable(v2);
    m->addComponent(comp1);
    m->addComponent(comp2);

    libcellml::Variable::setEquivalenceConnectionId(v1, v2, "mappingId");
    EXPECT_EQ("", libcellml::Variable::equivalenceConnectionId(v1, v2));
    libcellml::Variable::setEquivalenceConnectionId(v1, nullptr, "mappingId");
    EXPECT_EQ("", libcellml::Variable::equivalenceConnectionId(v1, nullptr));
    libcellml::Variable::setEquivalenceConnectionId(nullptr, v2, "mappingId");
    EXPECT_EQ("", libcellml::Variable::equivalenceConnectionId(nullptr, v2));
    libcellml::Variable::setEquivalenceConnectionId(nullptr, nullptr, "mappingId");
    EXPECT_EQ("", libcellml::Variable::equivalenceConnectionId(nullptr, nullptr));

    libcellml::Variable::addEquivalence(v1, v2);

    libcellml::Variable::setEquivalenceMappingId(v1, v2, "mappingId");
    EXPECT_EQ("mappingId", libcellml::Variable::equivalenceMappingId(v1, v2));
}

TEST(Connection, removeConnectionIdFromConnection)
{
    const std::string eWithConnectionId =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"component1\">\n"
        "    <variable name=\"variable1\"/>\n"
        "  </component>\n"
        "  <component name=\"component2\">\n"
        "    <variable name=\"variable2\"/>\n"
        "  </component>\n"
        "  <connection component_1=\"component1\" component_2=\"component2\" id=\"connectionId\">\n"
        "    <map_variables variable_1=\"variable1\" variable_2=\"variable2\"/>\n"
        "  </connection>\n"
        "</model>\n";
    const std::string eWithOutConnectionId =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"component1\">\n"
        "    <variable name=\"variable1\"/>\n"
        "  </component>\n"
        "  <component name=\"component2\">\n"
        "    <variable name=\"variable2\"/>\n"
        "  </component>\n"
        "  <connection component_1=\"component1\" component_2=\"component2\">\n"
        "    <map_variables variable_1=\"variable1\" variable_2=\"variable2\"/>\n"
        "  </connection>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr comp1 = libcellml::Component::create();
    libcellml::ComponentPtr comp2 = libcellml::Component::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    comp1->setName("component1");
    comp2->setName("component2");
    v1->setName("variable1");
    v2->setName("variable2");

    comp1->addVariable(v1);
    comp2->addVariable(v2);
    m->addComponent(comp1);
    m->addComponent(comp2);
    libcellml::Variable::addEquivalence(v1, v2);

    libcellml::Variable::setEquivalenceConnectionId(v1, v2, "connectionId");

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    std::string a = printer->printModel(m);
    EXPECT_EQ(eWithConnectionId, a);

    libcellml::Variable::removeEquivalenceConnectionId(v1, v2);

    a = printer->printModel(m);
    EXPECT_EQ(eWithOutConnectionId, a);

    libcellml::Variable::removeEquivalence(v1, v2);

    libcellml::Variable::removeEquivalenceConnectionId(v1, v2);
    EXPECT_EQ("", libcellml::Variable::equivalenceConnectionId(v1, v2));
    libcellml::Variable::removeEquivalenceConnectionId(v1, nullptr);
    EXPECT_EQ("", libcellml::Variable::equivalenceConnectionId(v1, nullptr));
    libcellml::Variable::removeEquivalenceConnectionId(nullptr, v2);
    EXPECT_EQ("", libcellml::Variable::equivalenceConnectionId(nullptr, v2));
    libcellml::Variable::removeEquivalenceConnectionId(nullptr, nullptr);
    EXPECT_EQ("", libcellml::Variable::equivalenceConnectionId(nullptr, nullptr));
}

TEST(Connection, removeMappingIdFromConnection)
{
    const std::string eWithMappingId =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"component1\">\n"
        "    <variable name=\"variable1\"/>\n"
        "  </component>\n"
        "  <component name=\"component2\">\n"
        "    <variable name=\"variable2\"/>\n"
        "  </component>\n"
        "  <connection component_1=\"component1\" component_2=\"component2\">\n"
        "    <map_variables variable_1=\"variable1\" variable_2=\"variable2\" id=\"mappingId\"/>\n"
        "  </connection>\n"
        "</model>\n";
    const std::string eWithOutMappingId =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"component1\">\n"
        "    <variable name=\"variable1\"/>\n"
        "  </component>\n"
        "  <component name=\"component2\">\n"
        "    <variable name=\"variable2\"/>\n"
        "  </component>\n"
        "  <connection component_1=\"component1\" component_2=\"component2\">\n"
        "    <map_variables variable_1=\"variable1\" variable_2=\"variable2\"/>\n"
        "  </connection>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr comp1 = libcellml::Component::create();
    libcellml::ComponentPtr comp2 = libcellml::Component::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    comp1->setName("component1");
    comp2->setName("component2");
    v1->setName("variable1");
    v2->setName("variable2");

    comp1->addVariable(v1);
    comp2->addVariable(v2);
    m->addComponent(comp1);
    m->addComponent(comp2);
    libcellml::Variable::addEquivalence(v1, v2);

    libcellml::Variable::setEquivalenceMappingId(v1, v2, "mappingId");

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    std::string a = printer->printModel(m);
    EXPECT_EQ(eWithMappingId, a);

    libcellml::Variable::removeEquivalenceMappingId(v1, v2);

    a = printer->printModel(m);
    EXPECT_EQ(eWithOutMappingId, a);

    libcellml::Variable::removeEquivalence(v1, v2);

    libcellml::Variable::removeEquivalenceMappingId(v1, v2);
    EXPECT_EQ("", libcellml::Variable::equivalenceMappingId(v1, v2));
    libcellml::Variable::removeEquivalenceMappingId(v1, nullptr);
    EXPECT_EQ("", libcellml::Variable::equivalenceMappingId(v1, nullptr));
    libcellml::Variable::removeEquivalenceMappingId(nullptr, v2);
    EXPECT_EQ("", libcellml::Variable::equivalenceMappingId(nullptr, v2));
    libcellml::Variable::removeEquivalenceMappingId(nullptr, nullptr);
    EXPECT_EQ("", libcellml::Variable::equivalenceMappingId(nullptr, nullptr));
}

TEST(Connection, removeVariablesFromConnections)
{
    // Initial model
    const std::string e1 =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"component1\">\n"
        "    <variable name=\"variable1_1\"/>\n"
        "    <variable name=\"variable1_2\"/>\n"
        "  </component>\n"
        "  <component name=\"component2\">\n"
        "    <variable name=\"variable2\"/>\n"
        "  </component>\n"
        "  <component name=\"component3\">\n"
        "    <variable name=\"variable3\"/>\n"
        "  </component>\n"
        "  <component name=\"component4\">\n"
        "    <variable name=\"variable4\"/>\n"
        "  </component>\n"
        "  <connection component_1=\"component1\" component_2=\"component2\" id=\"conId\">\n"
        "    <map_variables variable_1=\"variable1_1\" variable_2=\"variable2\"/>\n"
        "    <map_variables variable_1=\"variable1_2\" variable_2=\"variable2\"/>\n"
        "  </connection>\n"
        "  <connection component_1=\"component1\" component_2=\"component3\">\n"
        "    <map_variables variable_1=\"variable1_1\" variable_2=\"variable3\"/>\n"
        "  </connection>\n"
        "  <connection component_1=\"component1\" component_2=\"component4\">\n"
        "    <map_variables variable_1=\"variable1_1\" variable_2=\"variable4\" id=\"v11v4Id\"/>\n"
        "  </connection>\n"
        "  <connection component_1=\"component2\" component_2=\"component3\">\n"
        "    <map_variables variable_1=\"variable2\" variable_2=\"variable3\"/>\n"
        "  </connection>\n"
        "</model>\n";

    // Remove variable4
    const std::string e2 =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"component1\">\n"
        "    <variable name=\"variable1_1\"/>\n"
        "    <variable name=\"variable1_2\"/>\n"
        "  </component>\n"
        "  <component name=\"component2\">\n"
        "    <variable name=\"variable2\"/>\n"
        "  </component>\n"
        "  <component name=\"component3\">\n"
        "    <variable name=\"variable3\"/>\n"
        "  </component>\n"
        "  <component name=\"component4\"/>\n"
        "  <connection component_1=\"component1\" component_2=\"component2\" id=\"conId\">\n"
        "    <map_variables variable_1=\"variable1_1\" variable_2=\"variable2\"/>\n"
        "    <map_variables variable_1=\"variable1_2\" variable_2=\"variable2\"/>\n"
        "  </connection>\n"
        "  <connection component_1=\"component1\" component_2=\"component3\">\n"
        "    <map_variables variable_1=\"variable1_1\" variable_2=\"variable3\"/>\n"
        "  </connection>\n"
        "  <connection component_1=\"component1\">\n"
        "    <map_variables variable_1=\"variable1_1\" variable_2=\"variable4\" id=\"v11v4Id\"/>\n"
        "  </connection>\n"
        "  <connection component_1=\"component2\" component_2=\"component3\">\n"
        "    <map_variables variable_1=\"variable2\" variable_2=\"variable3\"/>\n"
        "  </connection>\n"
        "</model>\n";

    // Remove variable3
    const std::string e3 =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"component1\">\n"
        "    <variable name=\"variable1_1\"/>\n"
        "    <variable name=\"variable1_2\"/>\n"
        "  </component>\n"
        "  <component name=\"component2\">\n"
        "    <variable name=\"variable2\"/>\n"
        "  </component>\n"
        "  <component name=\"component3\"/>\n"
        "  <component name=\"component4\"/>\n"
        "  <connection component_1=\"component1\" component_2=\"component2\" id=\"conId\">\n"
        "    <map_variables variable_1=\"variable1_1\" variable_2=\"variable2\"/>\n"
        "    <map_variables variable_1=\"variable1_2\" variable_2=\"variable2\"/>\n"
        "  </connection>\n"
        "  <connection component_1=\"component1\">\n"
        "    <map_variables variable_1=\"variable1_1\" variable_2=\"variable3\"/>\n"
        "    <map_variables variable_1=\"variable1_1\" variable_2=\"variable4\" id=\"v11v4Id\"/>\n"
        "  </connection>\n"
        "  <connection component_1=\"component2\">\n"
        "    <map_variables variable_1=\"variable2\" variable_2=\"variable3\"/>\n"
        "  </connection>\n"
        "</model>\n";

    // Remove variable2
    const std::string e4 =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"component1\">\n"
        "    <variable name=\"variable1_1\"/>\n"
        "    <variable name=\"variable1_2\"/>\n"
        "  </component>\n"
        "  <component name=\"component2\"/>\n"
        "  <component name=\"component3\"/>\n"
        "  <component name=\"component4\"/>\n"
        "  <connection component_1=\"component1\" id=\"conId\">\n"
        "    <map_variables variable_1=\"variable1_1\" variable_2=\"variable2\"/>\n"
        "    <map_variables variable_1=\"variable1_1\" variable_2=\"variable3\"/>\n"
        "    <map_variables variable_1=\"variable1_1\" variable_2=\"variable4\" id=\"v11v4Id\"/>\n"
        "    <map_variables variable_1=\"variable1_2\" variable_2=\"variable2\"/>\n"
        "  </connection>\n"
        "</model>\n";

    // Remove all variables from component1 (variable1_1 and variable1_2)
    const std::string e5 =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"component1\"/>\n"
        "  <component name=\"component2\"/>\n"
        "  <component name=\"component3\"/>\n"
        "  <component name=\"component4\"/>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr comp1 = libcellml::Component::create();
    libcellml::ComponentPtr comp2 = libcellml::Component::create();
    libcellml::ComponentPtr comp3 = libcellml::Component::create();
    libcellml::ComponentPtr comp4 = libcellml::Component::create();
    libcellml::VariablePtr v1_1 = libcellml::Variable::create();
    libcellml::VariablePtr v1_2 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    libcellml::VariablePtr v3 = libcellml::Variable::create();
    libcellml::VariablePtr v4 = libcellml::Variable::create();
    comp1->setName("component1");
    comp2->setName("component2");
    comp3->setName("component3");
    comp4->setName("component4");
    v1_1->setName("variable1_1");
    v1_2->setName("variable1_2");
    v2->setName("variable2");
    v3->setName("variable3");
    v4->setName("variable4");

    comp1->addVariable(v1_1);
    comp1->addVariable(v1_2);
    comp2->addVariable(v2);
    comp3->addVariable(v3);
    comp4->addVariable(v4);
    m->addComponent(comp1);
    m->addComponent(comp2);
    m->addComponent(comp3);
    m->addComponent(comp4);
    libcellml::Variable::addEquivalence(v1_1, v2);
    libcellml::Variable::addEquivalence(v1_2, v2);
    libcellml::Variable::setEquivalenceConnectionId(v1_2, v2, "conId");
    libcellml::Variable::addEquivalence(v1_1, v3);
    libcellml::Variable::addEquivalence(v1_1, v4);
    libcellml::Variable::setEquivalenceMappingId(v1_1, v4, "v11v4Id");
    libcellml::Variable::addEquivalence(v2, v3);
    libcellml::PrinterPtr printer = libcellml::Printer::create();
    std::string a = printer->printModel(m);
    EXPECT_EQ(e1, a);

    comp4->removeVariable(v4);
    a = printer->printModel(m);
    EXPECT_EQ(e2, a);

    comp3->removeVariable("variable3");
    a = printer->printModel(m);
    EXPECT_EQ(e3, a);

    comp2->removeVariable(v2);
    a = printer->printModel(m);
    EXPECT_EQ(e4, a);

    comp1->removeAllVariables();
    a = printer->printModel(m);
    EXPECT_EQ(e5, a);
}

TEST(Connection, nonUniqueWayOfGettingEquivalenceConnectionId)
{
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr comp1 = libcellml::Component::create();
    libcellml::ComponentPtr comp2 = libcellml::Component::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    libcellml::VariablePtr v3 = libcellml::Variable::create();
    libcellml::VariablePtr v4 = libcellml::Variable::create();
    comp1->setName("component1");
    comp2->setName("component2");
    v1->setName("variable1");
    v2->setName("variable2");
    v3->setName("variable3");
    v4->setName("variable4");

    comp1->addVariable(v1);
    comp1->addVariable(v2);
    comp2->addVariable(v3);
    comp2->addVariable(v4);
    m->addComponent(comp1);
    m->addComponent(comp2);
    libcellml::Variable::addEquivalence(v1, v3);
    libcellml::Variable::addEquivalence(v2, v4);
    libcellml::Variable::setEquivalenceConnectionId(v1, v3, "connection_id");

    EXPECT_EQ("connection_id", libcellml::Variable::equivalenceConnectionId(v2, v4));
}

TEST(Connection, connectionIdFromReverseVariablesToThoseSet)
{
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr comp1 = libcellml::Component::create();
    libcellml::ComponentPtr comp2 = libcellml::Component::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    comp1->setName("component1");
    comp2->setName("component2");
    v1->setName("variable1");
    v2->setName("variable2");

    comp1->addVariable(v1);
    comp2->addVariable(v2);
    m->addComponent(comp1);
    m->addComponent(comp2);
    libcellml::Variable::addEquivalence(v1, v2);
    libcellml::Variable::setEquivalenceConnectionId(v1, v2, "connection_id");

    EXPECT_EQ("connection_id", libcellml::Variable::equivalenceConnectionId(v2, v1));
}

TEST(Connection, twoEncapsulatedChildComponentsWithConnectionsAndMixedInterfaces)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"parent\">\n"
        "    <variable name=\"variable1\" interface=\"private\"/>\n"
        "  </component>\n"
        "  <component name=\"child1\">\n"
        "    <variable name=\"variable2\" interface=\"public\"/>\n"
        "  </component>\n"
        "  <component name=\"child2\">\n"
        "    <variable name=\"variable3\" interface=\"public\"/>\n"
        "  </component>\n"
        "  <connection component_1=\"parent\" component_2=\"child1\">\n"
        "    <map_variables variable_1=\"variable1\" variable_2=\"variable2\"/>\n"
        "  </connection>\n"
        "  <connection component_1=\"parent\" component_2=\"child2\">\n"
        "    <map_variables variable_1=\"variable1\" variable_2=\"variable3\"/>\n"
        "  </connection>\n"
        "  <encapsulation>\n"
        "    <component_ref component=\"parent\">\n"
        "      <component_ref component=\"child1\"/>\n"
        "      <component_ref component=\"child2\"/>\n"
        "    </component_ref>\n"
        "  </encapsulation>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr parent = libcellml::Component::create();
    libcellml::ComponentPtr child1 = libcellml::Component::create();
    libcellml::ComponentPtr child2 = libcellml::Component::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    libcellml::VariablePtr v3 = libcellml::Variable::create();

    parent->setName("parent");
    child1->setName("child1");
    child2->setName("child2");
    v1->setName("variable1");
    v2->setName("variable2");
    v3->setName("variable3");

    m->addComponent(parent);
    parent->addComponent(child1);
    parent->addComponent(child2);
    parent->addVariable(v1);
    child1->addVariable(v2);
    child2->addVariable(v3);
    libcellml::Variable::addEquivalence(v1, v2);
    libcellml::Variable::addEquivalence(v1, v3);
    v1->setInterfaceType(libcellml::Variable::InterfaceType::PRIVATE);
    v2->setInterfaceType(libcellml::Variable::InterfaceType::PUBLIC);
    v3->setInterfaceType(libcellml::Variable::InterfaceType::PUBLIC);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

TEST(Connection, twoEncapsulatedChildComponentsWithConnectionsAndMixedInterfacesUsingIds)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"parent\">\n"
        "    <variable name=\"variable1\" interface=\"private\"/>\n"
        "  </component>\n"
        "  <component name=\"child1\">\n"
        "    <variable name=\"variable2\" interface=\"public\"/>\n"
        "  </component>\n"
        "  <component name=\"child2\">\n"
        "    <variable name=\"variable3\" interface=\"public\"/>\n"
        "  </component>\n"
        "  <connection component_1=\"parent\" component_2=\"child1\" id=\"con1\">\n"
        "    <map_variables variable_1=\"variable1\" variable_2=\"variable2\"/>\n"
        "  </connection>\n"
        "  <connection component_1=\"parent\" component_2=\"child2\" id=\"con2\">\n"
        "    <map_variables variable_1=\"variable1\" variable_2=\"variable3\" id=\"map1\"/>\n"
        "  </connection>\n"
        "  <encapsulation>\n"
        "    <component_ref component=\"parent\">\n"
        "      <component_ref component=\"child1\"/>\n"
        "      <component_ref component=\"child2\"/>\n"
        "    </component_ref>\n"
        "  </encapsulation>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr parent = libcellml::Component::create();
    libcellml::ComponentPtr child1 = libcellml::Component::create();
    libcellml::ComponentPtr child2 = libcellml::Component::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    libcellml::VariablePtr v3 = libcellml::Variable::create();

    parent->setName("parent");
    child1->setName("child1");
    child2->setName("child2");
    v1->setName("variable1");
    v2->setName("variable2");
    v3->setName("variable3");

    m->addComponent(parent);
    parent->addComponent(child1);
    parent->addComponent(child2);
    parent->addVariable(v1);
    child1->addVariable(v2);
    child2->addVariable(v3);
    libcellml::Variable::addEquivalence(v1, v2);
    libcellml::Variable::addEquivalence(v1, v3);
    libcellml::Variable::setEquivalenceConnectionId(v1, v2, "con1");
    libcellml::Variable::setEquivalenceConnectionId(v1, v3, "con2");
    libcellml::Variable::setEquivalenceMappingId(v1, v3, "map1");
    v1->setInterfaceType(libcellml::Variable::InterfaceType::PRIVATE);
    v2->setInterfaceType(libcellml::Variable::InterfaceType::PUBLIC);
    v3->setInterfaceType(libcellml::Variable::InterfaceType::PUBLIC);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

TEST(Connection, twoEncapsulatedChildComponentsWithConnectionsAndPublicInterfaces)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"parent\">\n"
        "    <variable name=\"variable1\" interface=\"public\"/>\n"
        "  </component>\n"
        "  <component name=\"child1\">\n"
        "    <variable name=\"variable2\" interface=\"public\"/>\n"
        "  </component>\n"
        "  <component name=\"child2\">\n"
        "    <variable name=\"variable3\" interface=\"public\"/>\n"
        "  </component>\n"
        "  <connection component_1=\"parent\" component_2=\"child1\">\n"
        "    <map_variables variable_1=\"variable1\" variable_2=\"variable2\"/>\n"
        "  </connection>\n"
        "  <connection component_1=\"parent\" component_2=\"child2\">\n"
        "    <map_variables variable_1=\"variable1\" variable_2=\"variable3\"/>\n"
        "  </connection>\n"
        "  <encapsulation>\n"
        "    <component_ref component=\"parent\">\n"
        "      <component_ref component=\"child1\"/>\n"
        "      <component_ref component=\"child2\"/>\n"
        "    </component_ref>\n"
        "  </encapsulation>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr parent = libcellml::Component::create();
    libcellml::ComponentPtr child1 = libcellml::Component::create();
    libcellml::ComponentPtr child2 = libcellml::Component::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    libcellml::VariablePtr v3 = libcellml::Variable::create();

    parent->setName("parent");
    child1->setName("child1");
    child2->setName("child2");
    v1->setName("variable1");
    v2->setName("variable2");
    v3->setName("variable3");

    m->addComponent(parent);
    parent->addComponent(child1);
    parent->addComponent(child2);
    parent->addVariable(v1);
    child1->addVariable(v2);
    child2->addVariable(v3);
    libcellml::Variable::addEquivalence(v1, v2);
    libcellml::Variable::addEquivalence(v1, v3);
    v1->setInterfaceType(libcellml::Variable::InterfaceType::PUBLIC);
    v2->setInterfaceType(libcellml::Variable::InterfaceType::PUBLIC);
    v3->setInterfaceType(libcellml::Variable::InterfaceType::PUBLIC);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

TEST(Connection, importedComponentConnectionAndParse)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <import xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"some-other-model.xml\">\n"
        "    <component component_ref=\"component_in_that_model\" name=\"component_in_this_model\"/>\n"
        "  </import>\n"
        "  <component name=\"component_bob\">\n"
        "    <variable name=\"variable_bob\" units=\"second\"/>\n"
        "  </component>\n"
        "  <connection component_1=\"component_in_this_model\" component_2=\"component_bob\">\n"
        "    <map_variables variable_1=\"variable_import\" variable_2=\"variable_bob\"/>\n"
        "  </connection>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ImportSourcePtr imp = libcellml::ImportSource::create();
    libcellml::ComponentPtr componentImported = libcellml::Component::create();
    libcellml::ComponentPtr componentBob = libcellml::Component::create();
    libcellml::VariablePtr variableImported = libcellml::Variable::create();
    libcellml::VariablePtr variableBob = libcellml::Variable::create();

    imp->setUrl("some-other-model.xml");
    componentImported->setName("component_in_this_model");
    componentImported->setSourceComponent(imp, "component_in_that_model");
    componentBob->setName("component_bob");
    variableImported->setName("variable_import");
    variableBob->setName("variable_bob");
    variableBob->setUnits("second");

    m->addComponent(componentImported);
    m->addComponent(componentBob);
    componentImported->addVariable(variableImported);
    componentBob->addVariable(variableBob);
    EXPECT_EQ(variableImported, componentImported->variable(0));

    libcellml::Variable::addEquivalence(variableImported, variableBob);
    libcellml::PrinterPtr printer = libcellml::Printer::create();
    std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);

    // Parse
    libcellml::ParserPtr parser = libcellml::Parser::create();
    libcellml::ModelPtr model = parser->parseModel(e);
    EXPECT_EQ(size_t(1), parser->issueCount());

    a = printer->printModel(model);
    EXPECT_EQ(e, a);
}

TEST(Connection, componentConnectionAndParseMissingVariable)
{
    const std::string s =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model name=\"\" xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"component_dave\">\n"
        "    <variable name=\"variable_dave\" units=\"dimensionless\"/>\n"
        "  </component>\n"
        "  <component name=\"component_bob\">\n"
        "    <variable name=\"variable_bob\" units=\"dimensionless\"/>\n"
        "  </component>\n"
        "  <connection component_1=\"component_dave\" component_2=\"component_bob\">\n"
        "    <map_variables variable_1=\"variable_angus\" variable_2=\"variable_bob\"/>\n"
        "  </connection>\n"
        "</model>\n";
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"component_dave\">\n"
        "    <variable name=\"variable_dave\" units=\"dimensionless\"/>\n"
        "  </component>\n"
        "  <component name=\"component_bob\">\n"
        "    <variable name=\"variable_bob\" units=\"dimensionless\"/>\n"
        "  </component>\n"
        "</model>\n";

    const std::string expectIssue = "Variable 'variable_angus' is specified as variable_1 in a connection but it does not exist in component_1 component 'component_dave' of model ''.";

    // Parse
    libcellml::ParserPtr parser = libcellml::Parser::create();
    libcellml::ModelPtr model = parser->parseModel(s);
    EXPECT_EQ(size_t(1), parser->issueCount());

    EXPECT_EQ(expectIssue, parser->issue(0)->description());

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(model);
    EXPECT_EQ(e, a);
}

TEST(Connection, mappingId)
{
    const std::string in =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\"  name=\"everything\" >\n"
        "  <component name=\"c1\" >\n"
        "    <variable name=\"v1\" units=\"units2\" interface=\"private\"/>\n"
        "  </component>\n"
        "  <component name=\"c2\">\n"
        "    <variable name=\"v2a\" units=\"units2\" interface=\"public\"/>\n"
        "    <variable name=\"v2b\" units=\"units2\" interface=\"public\"/>\n"
        "  </component>\n"
        "  <component name=\"c3\">\n"
        "    <variable name=\"v3\" units=\"units2\" interface=\"public\"/>\n"
        "  </component>\n"
        "  <connection component_1=\"c1\" component_2=\"c2\">\n"
        "    <map_variables variable_1=\"v1\" variable_2=\"v2a\" id=\"id12a\"/>\n"
        "    <map_variables variable_1=\"v1\" variable_2=\"v2b\" id=\"id12b\"/>\n"
        "  </connection>\n"
        "  <connection component_1=\"c1\" component_2=\"c3\">\n"
        "    <map_variables variable_1=\"v1\" variable_2=\"v3\" id=\"id13\"/>\n"
        "  </connection>\n"
        "</model>\n";

    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(in);
    EXPECT_EQ("id12a",
              libcellml::Variable::equivalenceMappingId(
                  model->component("c1")->variable("v1"),
                  model->component("c2")->variable("v2a")));
    EXPECT_EQ("id12b",
              libcellml::Variable::equivalenceMappingId(
                  model->component("c1")->variable("v1"),
                  model->component("c2")->variable("v2b")));
    EXPECT_EQ("id13",
              libcellml::Variable::equivalenceMappingId(
                  model->component("c1")->variable("v1"),
                  model->component("c3")->variable("v3")));
}

TEST(Connection, repeatedConnection)
{
    const std::string in =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"connection\">\n"
        "  <component name=\"component1\">\n"
        "    <variable name=\"variable1\" units=\"dimensionless\"/>\n"
        "  </component>\n"
        "  <component name=\"component2\">\n"
        "    <variable name=\"variable2\" units=\"dimensionless\"/>\n"
        "  </component>\n"
        "  <connection component_1=\"component1\" component_2=\"component2\">\n"
        "    <map_variables variable_1=\"variable1\" variable_2=\"variable2\"/>\n"
        "  </connection>\n"
        "  <connection component_1=\"component1\" component_2=\"component2\">\n"
        "    <map_variables variable_1=\"variable1\" variable_2=\"variable2\"/>\n"
        "  </connection>\n"
        "</model>\n";

    libcellml::ParserPtr p = libcellml::Parser::create();
    libcellml::ModelPtr m = p->parseModel(in);

    const std::string expectedIssue = "Connection in model 'connection' between 'component1' and 'component2' is not unique.";

    EXPECT_EQ(size_t(1), p->errorCount());
    EXPECT_EQ(expectedIssue, p->error(0)->description());
}

TEST(Connection, repeatedConnectionCross)
{
    const std::string in =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"connection\">\n"
        "  <component name=\"component1\">\n"
        "    <variable name=\"variable1\" units=\"dimensionless\"/>\n"
        "  </component>\n"
        "  <component name=\"component2\">\n"
        "    <variable name=\"variable2\" units=\"dimensionless\"/>\n"
        "  </component>\n"
        "  <connection component_1=\"component1\" component_2=\"component2\">\n"
        "    <map_variables variable_1=\"variable1\" variable_2=\"variable2\"/>\n"
        "  </connection>\n"
        "  <connection component_1=\"component2\" component_2=\"component1\">\n"
        "    <map_variables variable_1=\"variable2\" variable_2=\"variable1\"/>\n"
        "  </connection>\n"
        "</model>\n";

    libcellml::ParserPtr p = libcellml::Parser::create();
    libcellml::ModelPtr m = p->parseModel(in);

    const std::string expectedIssue = "Connection in model 'connection' between 'component1' and 'component2' is not unique.";

    EXPECT_EQ(size_t(1), p->errorCount());
    EXPECT_EQ(expectedIssue, p->error(0)->description());
}

TEST(Connection, repeatedMapVariables)
{
    const std::string in =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"connection\">\n"
        "  <component name=\"component1\">\n"
        "    <variable name=\"variable1\" units=\"dimensionless\"/>\n"
        "    <variable name=\"variable6\" units=\"dimensionless\"/>\n"
        "  </component>\n"
        "  <component name=\"component2\">\n"
        "    <variable name=\"variable2\" units=\"dimensionless\"/>\n"
        "    <variable name=\"variable3\" units=\"dimensionless\"/>\n"
        "    <variable name=\"variable5\" units=\"dimensionless\"/>\n"
        "  </component>\n"
        "  <connection component_1=\"component1\" component_2=\"component2\">\n"
        "    <map_variables variable_1=\"variable1\" variable_2=\"variable2\"/>\n"
        "    <map_variables variable_1=\"variable1\" variable_2=\"variable2\"/>\n"
        "    <map_variables variable_1=\"variable6\" variable_2=\"variable3\"/>\n"
        "    <map_variables variable_1=\"variable6\" variable_2=\"variable5\"/>\n"
        "    <map_variables variable_1=\"variable6\" variable_2=\"variable3\"/>\n"
        "  </connection>\n"
        "</model>\n";

    libcellml::ParserPtr p = libcellml::Parser::create();
    libcellml::ModelPtr m = p->parseModel(in);

    const std::vector<std::string> expectedIssues = {
        "Connection in model 'connection' between 'variable1' and 'variable2' is not unique.",
        "Connection in model 'connection' between 'variable3' and 'variable6' is not unique.",
    };

    EXPECT_EQ_ISSUES(expectedIssues, p);
}

TEST(Connection, addEquivalenceReturnsFalseProperly)
{
    auto m = libcellml::Model::create("m");
    auto c1 = libcellml::Component::create("c1");
    auto c2 = libcellml::Component::create("c2");
    auto v1 = libcellml::Variable::create("v1");
    auto v2 = libcellml::Variable::create("v2");

    EXPECT_TRUE(m->addComponent(c1));
    EXPECT_TRUE(m->addComponent(c2));
    EXPECT_TRUE(c1->addVariable(v1));
    EXPECT_TRUE(c2->addVariable(v2));

    // Create a connection with self variable, expect no connections have been created.
    EXPECT_FALSE(libcellml::Variable::addEquivalence(v1, v1));
    EXPECT_EQ(size_t(0), v1->equivalentVariableCount());

    // Create a connection with one nullptr, expect no connections have been created.
    EXPECT_FALSE(libcellml::Variable::addEquivalence(v2, nullptr));
    EXPECT_EQ(size_t(0), v2->equivalentVariableCount());
}

TEST(Connection, addEquivalenceConnectionIdPropagation)
{
    auto m = libcellml::Model::create("m");
    auto c1 = libcellml::Component::create("c1");
    auto c2 = libcellml::Component::create("c2");
    auto v1 = libcellml::Variable::create("v1");
    auto v2 = libcellml::Variable::create("v2");
    auto v3 = libcellml::Variable::create("v3");
    auto v4 = libcellml::Variable::create("v4");
    auto v5 = libcellml::Variable::create("v5");
    auto v6 = libcellml::Variable::create("v6");

    m->addComponent(c1);
    m->addComponent(c2);
    c1->addVariable(v1);
    c2->addVariable(v2);
    c1->addVariable(v3);
    c2->addVariable(v4);
    c1->addVariable(v5);
    c2->addVariable(v6);

    libcellml::Variable::addEquivalence(v1, v2);
    libcellml::Variable::addEquivalence(v3, v4);
    libcellml::Variable::setEquivalenceConnectionId(v1, v2, "connection_01");
    EXPECT_EQ("connection_01", libcellml::Variable::equivalenceConnectionId(v3, v4));

    libcellml::Variable::addEquivalence(v5, v6);
    EXPECT_EQ("connection_01", libcellml::Variable::equivalenceConnectionId(v5, v6));
    EXPECT_EQ("", libcellml::Variable::equivalenceConnectionId(v5, v6, false));

    libcellml::Variable::removeEquivalenceConnectionId(v1, v2);
    EXPECT_EQ("", libcellml::Variable::equivalenceConnectionId(v1, v2));
    EXPECT_EQ("", libcellml::Variable::equivalenceConnectionId(v3, v4));
    EXPECT_EQ("", libcellml::Variable::equivalenceConnectionId(v5, v6));
}

TEST(Connection, removeEquivalenceConnectionIdFromVariablesThatAreNotInComponents)
{
    auto v1 = libcellml::Variable::create("v1");
    auto v2 = libcellml::Variable::create("v2");

    libcellml::Variable::addEquivalence(v1, v2);
    libcellml::Variable::setEquivalenceConnectionId(v1, v2, "connection_01");
    EXPECT_EQ("connection_01", libcellml::Variable::equivalenceConnectionId(v1, v2));

    libcellml::Variable::removeEquivalenceConnectionId(v1, v2);
    EXPECT_EQ("", libcellml::Variable::equivalenceConnectionId(v1, v2));
}

TEST(Connection, addEquivalenceConnectionIdClearedAfterDisconnect)
{
    auto m = libcellml::Model::create("m");
    auto c1 = libcellml::Component::create("c1");
    auto c2 = libcellml::Component::create("c2");
    auto v1 = libcellml::Variable::create("v1");
    auto v2 = libcellml::Variable::create("v2");

    m->addComponent(c1);
    m->addComponent(c2);
    c1->addVariable(v1);
    c2->addVariable(v2);

    libcellml::Variable::addEquivalence(v1, v2);
    libcellml::Variable::setEquivalenceConnectionId(v1, v2, "connection_01");
    EXPECT_EQ("connection_01", libcellml::Variable::equivalenceConnectionId(v1, v2));

    libcellml::Variable::removeEquivalenceConnectionId(v1, v2);
    EXPECT_EQ("", libcellml::Variable::equivalenceConnectionId(v1, v2));

    libcellml::Variable::addEquivalence(v1, v2);
    EXPECT_EQ("", libcellml::Variable::equivalenceConnectionId(v1, v2));
}
