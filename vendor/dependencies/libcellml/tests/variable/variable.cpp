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

TEST(Component, construct)
{
    const std::string validName = "valid_name";

    libcellml::VariablePtr v1 = libcellml::Variable::create();
    v1->setName(validName);

    EXPECT_EQ("valid_name", v1->name());

    auto v2 = libcellml::Variable::create(validName);
    EXPECT_EQ("valid_name", v2->name());
}

TEST(Variable, setValidVariableName)
{
    const std::string in = "valid_name";
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component>\n"
        "    <variable name=\"valid_name\"/>\n"
        "  </component>\n"
        "</model>\n";
    libcellml::ModelPtr m = createModelWithComponent();
    libcellml::ComponentPtr c = m->component(0);
    libcellml::VariablePtr v = libcellml::Variable::create();
    v->setName(in);

    c->addVariable(v);
    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

TEST(Variable, setInvalidVariableName)
{
    const std::string in = "invalid name";
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component>\n"
        "    <variable name=\"invalid name\"/>\n"
        "  </component>\n"
        "</model>\n";
    libcellml::ModelPtr m = createModelWithComponent();
    libcellml::ComponentPtr c = m->component(0);
    libcellml::VariablePtr v = libcellml::Variable::create();
    v->setName(in);
    c->addVariable(v);
    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

TEST(Variable, validVariableName)
{
    const std::string in = "valid_name";
    const std::string e = in;
    libcellml::VariablePtr v = libcellml::Variable::create();
    v->setName(in);
    const std::string a = v->name();
    EXPECT_EQ(e, a);
}

TEST(Variable, invalidVariableName)
{
    const std::string in = "invalid name";
    const std::string e = in;
    libcellml::VariablePtr v = libcellml::Variable::create();
    v->setName(in);
    const std::string a = v->name();
    EXPECT_EQ(e, a);
}

TEST(Variable, setUnits)
{
    const std::string in = "dimensionless";
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component>\n"
        "    <variable units=\"dimensionless\"/>\n"
        "  </component>\n"
        "</model>\n";
    libcellml::ModelPtr m = createModelWithComponent();
    libcellml::ComponentPtr c = m->component(0);
    libcellml::VariablePtr v = libcellml::Variable::create();

    libcellml::UnitsPtr u = libcellml::Units::create();
    u->setName(in);
    v->setUnits(u);
    c->addVariable(v);
    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

TEST(Variable, setUnitsAndName)
{
    const std::string in = "valid_name";
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component>\n"
        "    <variable name=\"valid_name\" units=\"dimensionless\"/>\n"
        "  </component>\n"
        "</model>\n";
    libcellml::ModelPtr m = createModelWithComponent();
    libcellml::ComponentPtr c = m->component(0);
    libcellml::VariablePtr v = libcellml::Variable::create();
    v->setName(in);

    libcellml::UnitsPtr u = libcellml::Units::create();
    u->setName("dimensionless");
    v->setUnits(u);
    c->addVariable(v);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

TEST(Variable, removeUnits)
{
    const std::string in = "dimensionless";
    const std::string eVariableWithUnits =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component>\n"
        "    <variable units=\"dimensionless\"/>\n"
        "  </component>\n"
        "</model>\n";
    const std::string eVariableWithOutUnits =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component>\n"
        "    <variable/>\n"
        "  </component>\n"
        "</model>\n";
    libcellml::ModelPtr m = createModelWithComponent();
    libcellml::ComponentPtr c = m->component(0);
    libcellml::VariablePtr v = libcellml::Variable::create();

    libcellml::UnitsPtr u = libcellml::Units::create();
    u->setName(in);
    v->setUnits(u);
    c->addVariable(v);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    std::string a = printer->printModel(m);
    EXPECT_EQ(eVariableWithUnits, a);

    v->removeUnits();
    a = printer->printModel(m);
    EXPECT_EQ(eVariableWithOutUnits, a);
}

TEST(Variable, setInitialValueByString)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component>\n"
        "    <variable initial_value=\"0.0\"/>\n"
        "  </component>\n"
        "</model>\n";
    libcellml::ModelPtr m = createModelWithComponent();
    libcellml::ComponentPtr c = m->component(0);
    libcellml::VariablePtr v = libcellml::Variable::create();
    v->setInitialValue("0.0");
    c->addVariable(v);
    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

TEST(Variable, setInitialValueByDouble)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component>\n"
        "    <variable initial_value=\"0\"/>\n"
        "  </component>\n"
        "</model>\n";
    libcellml::ModelPtr m = createModelWithComponent();
    libcellml::ComponentPtr c = m->component(0);
    libcellml::VariablePtr v = libcellml::Variable::create();
    double value = 0.0;
    v->setInitialValue(value);
    c->addVariable(v);
    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

TEST(Variable, setInitialValueByReference)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component>\n"
        "    <variable initial_value=\"referencedVariable\"/>\n"
        "  </component>\n"
        "</model>\n";
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    v1->setName("referencedVariable");
    libcellml::ModelPtr m = createModelWithComponent();
    libcellml::ComponentPtr c = m->component(0);
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    v2->setInitialValue(v1);
    c->addVariable(v2);
    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

TEST(Variable, unsetInitialValue)
{
    libcellml::VariablePtr v = libcellml::Variable::create();
    v->setInitialValue("3.0");

    EXPECT_EQ(v->initialValue(), "3.0");

    v->removeInitialValue();
    EXPECT_EQ(v->initialValue(), "");
}

TEST(Variable, setInitialValue)
{
    libcellml::VariablePtr v = libcellml::Variable::create();
    const std::string e = "0.0";
    v->setInitialValue(e);
    const std::string a = v->initialValue();
    EXPECT_EQ(e, a);
}

TEST(Variable, setInterfaceTypeByInvalidString)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component>\n"
        "    <variable interface=\"invalid\"/>\n"
        "  </component>\n"
        "</model>\n";
    libcellml::ModelPtr m = createModelWithComponent();
    libcellml::ComponentPtr c = m->component(0);
    libcellml::VariablePtr v = libcellml::Variable::create();
    v->setInterfaceType("invalid");
    c->addVariable(v);
    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

TEST(Variable, setInterfaceTypeNoneByValidString)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component>\n"
        "    <variable interface=\"none\"/>\n"
        "  </component>\n"
        "</model>\n";
    libcellml::ModelPtr m = createModelWithComponent();
    libcellml::ComponentPtr c = m->component(0);
    libcellml::VariablePtr v = libcellml::Variable::create();
    v->setInterfaceType("none");
    c->addVariable(v);
    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

TEST(Variable, setInterfaceTypeNoneByEnum)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component>\n"
        "    <variable interface=\"none\"/>\n"
        "  </component>\n"
        "</model>\n";
    libcellml::ModelPtr m = createModelWithComponent();
    libcellml::ComponentPtr c = m->component(0);
    libcellml::VariablePtr v = libcellml::Variable::create();
    v->setInterfaceType(libcellml::Variable::InterfaceType::NONE);
    c->addVariable(v);
    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

TEST(Variable, setInterfaceTypePrivate)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component>\n"
        "    <variable interface=\"private\"/>\n"
        "  </component>\n"
        "</model>\n";
    libcellml::ModelPtr m = createModelWithComponent();
    libcellml::ComponentPtr c = m->component(0);
    libcellml::VariablePtr v = libcellml::Variable::create();
    v->setInterfaceType(libcellml::Variable::InterfaceType::PRIVATE);
    c->addVariable(v);
    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

TEST(Variable, setInterfaceTypePublic)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component>\n"
        "    <variable interface=\"public\"/>\n"
        "  </component>\n"
        "</model>\n";
    libcellml::ModelPtr m = createModelWithComponent();
    libcellml::ComponentPtr c = m->component(0);
    libcellml::VariablePtr v = libcellml::Variable::create();
    v->setInterfaceType(libcellml::Variable::InterfaceType::PUBLIC);
    c->addVariable(v);
    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

TEST(Variable, setInterfaceTypePublicAndPrivate)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component>\n"
        "    <variable interface=\"public_and_private\"/>\n"
        "  </component>\n"
        "</model>\n";
    libcellml::ModelPtr m = createModelWithComponent();
    libcellml::ComponentPtr c = m->component(0);
    libcellml::VariablePtr v = libcellml::Variable::create();
    v->setInterfaceType(libcellml::Variable::InterfaceType::PUBLIC_AND_PRIVATE);
    c->addVariable(v);
    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

TEST(Variable, removeInterfaceTypePublicAndPrivate)
{
    const std::string eWithInterfaceType =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component>\n"
        "    <variable interface=\"public_and_private\"/>\n"
        "  </component>\n"
        "</model>\n";
    const std::string eWithOutInterfaceType =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component>\n"
        "    <variable/>\n"
        "  </component>\n"
        "</model>\n";

    libcellml::ModelPtr m = createModelWithComponent();
    libcellml::ComponentPtr c = m->component(0);
    libcellml::VariablePtr v = libcellml::Variable::create();
    v->setInterfaceType(libcellml::Variable::InterfaceType::PUBLIC_AND_PRIVATE);
    c->addVariable(v);
    libcellml::PrinterPtr printer = libcellml::Printer::create();
    std::string a = printer->printModel(m);
    EXPECT_EQ(eWithInterfaceType, a);

    v->removeInterfaceType();

    a = printer->printModel(m);
    EXPECT_EQ(eWithOutInterfaceType, a);
}

TEST(Variable, setGetInterfaceType)
{
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    libcellml::VariablePtr v3 = libcellml::Variable::create();
    libcellml::VariablePtr v4 = libcellml::Variable::create();
    libcellml::Variable::InterfaceType interfaceType1 = libcellml::Variable::InterfaceType::NONE;
    libcellml::Variable::InterfaceType interfaceType2 = libcellml::Variable::InterfaceType::PRIVATE;
    libcellml::Variable::InterfaceType interfaceType3 = libcellml::Variable::InterfaceType::PUBLIC;
    libcellml::Variable::InterfaceType interfaceType4 = libcellml::Variable::InterfaceType::PUBLIC_AND_PRIVATE;
    v1->setInterfaceType(interfaceType1);
    v2->setInterfaceType(interfaceType2);
    v3->setInterfaceType(interfaceType3);
    v4->setInterfaceType(interfaceType4);

    const std::string interfaceTypeString1 = "none";
    const std::string interfaceTypeString2 = "private";
    const std::string interfaceTypeString3 = "public";
    const std::string interfaceTypeString4 = "public_and_private";

    EXPECT_EQ(interfaceTypeString1, v1->interfaceType());
    EXPECT_EQ(interfaceTypeString2, v2->interfaceType());
    EXPECT_EQ(interfaceTypeString3, v3->interfaceType());
    EXPECT_EQ(interfaceTypeString4, v4->interfaceType());
}

TEST(Variable, hasInterfaceType)
{
    libcellml::VariablePtr v1 = libcellml::Variable::create();

    v1->setInterfaceType(libcellml::Variable::InterfaceType::PRIVATE);
    EXPECT_TRUE(v1->hasInterfaceType(libcellml::Variable::InterfaceType::PRIVATE));

    v1->setInterfaceType(libcellml::Variable::InterfaceType::PUBLIC);
    EXPECT_FALSE(v1->hasInterfaceType(libcellml::Variable::InterfaceType::PRIVATE));
    EXPECT_TRUE(v1->hasInterfaceType(libcellml::Variable::InterfaceType::PUBLIC));

    v1->setInterfaceType(libcellml::Variable::InterfaceType::NONE);
    EXPECT_FALSE(v1->hasInterfaceType(libcellml::Variable::InterfaceType::PUBLIC));
    EXPECT_TRUE(v1->hasInterfaceType(libcellml::Variable::InterfaceType::NONE));

    v1->setInterfaceType(libcellml::Variable::InterfaceType::PUBLIC_AND_PRIVATE);
    EXPECT_FALSE(v1->hasInterfaceType(libcellml::Variable::InterfaceType::NONE));
    EXPECT_TRUE(v1->hasInterfaceType(libcellml::Variable::InterfaceType::PUBLIC_AND_PRIVATE));

    v1->removeInterfaceType();
    EXPECT_FALSE(v1->hasInterfaceType(libcellml::Variable::InterfaceType::PUBLIC_AND_PRIVATE));
    EXPECT_TRUE(v1->hasInterfaceType(libcellml::Variable::InterfaceType::NONE));
}

TEST(Variable, addVariable)
{
    const std::string in = "valid_name";
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"valid_name\">\n"
        "    <variable name=\"valid_name\" units=\"dimensionless\"/>\n"
        "  </component>\n"
        "</model>\n";

    libcellml::ModelPtr m = createModelWithComponent();
    libcellml::ComponentPtr c = m->component(0);
    c->setName(in);

    libcellml::VariablePtr v = libcellml::Variable::create();
    v->setName(in);
    c->addVariable(v);

    libcellml::UnitsPtr u = libcellml::Units::create();
    u->setName("dimensionless");
    v->setUnits(u);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

TEST(Variable, parentOfVariable)
{
    libcellml::ComponentPtr c = libcellml::Component::create();
    libcellml::VariablePtr v = libcellml::Variable::create();
    c->addVariable(v);
    EXPECT_EQ(c, v->parent());
}

TEST(Variable, nullParentOfVariable)
{
    libcellml::VariablePtr v = libcellml::Variable::create();
    EXPECT_EQ(nullptr, v->parent());
}

TEST(Variable, hasEquivalentVariable)
{
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    libcellml::VariablePtr v3 = libcellml::Variable::create();

    EXPECT_FALSE(v1->hasEquivalentVariable(v1));
    EXPECT_FALSE(v1->hasEquivalentVariable(v2));
    EXPECT_FALSE(v1->hasEquivalentVariable(v3));

    EXPECT_FALSE(v2->hasEquivalentVariable(v1));
    EXPECT_FALSE(v2->hasEquivalentVariable(v2));
    EXPECT_FALSE(v2->hasEquivalentVariable(v3));

    EXPECT_FALSE(v3->hasEquivalentVariable(v1));
    EXPECT_FALSE(v3->hasEquivalentVariable(v2));
    EXPECT_FALSE(v3->hasEquivalentVariable(v3));

    libcellml::Variable::addEquivalence(v1, v2);
    libcellml::Variable::addEquivalence(v2, v3);

    EXPECT_FALSE(v1->hasEquivalentVariable(v1));
    EXPECT_TRUE(v1->hasEquivalentVariable(v2));
    EXPECT_FALSE(v1->hasEquivalentVariable(v3));

    EXPECT_TRUE(v2->hasEquivalentVariable(v1));
    EXPECT_FALSE(v2->hasEquivalentVariable(v2));
    EXPECT_TRUE(v2->hasEquivalentVariable(v3));

    EXPECT_FALSE(v3->hasEquivalentVariable(v1));
    EXPECT_TRUE(v3->hasEquivalentVariable(v2));
    EXPECT_FALSE(v3->hasEquivalentVariable(v3));
}

TEST(Variable, hasIndirectEquivalentVariable)
{
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    libcellml::VariablePtr v3 = libcellml::Variable::create();

    EXPECT_FALSE(v1->hasEquivalentVariable(v1, true));
    EXPECT_FALSE(v1->hasEquivalentVariable(v2, true));
    EXPECT_FALSE(v1->hasEquivalentVariable(v3, true));

    EXPECT_FALSE(v2->hasEquivalentVariable(v1, true));
    EXPECT_FALSE(v2->hasEquivalentVariable(v2, true));
    EXPECT_FALSE(v2->hasEquivalentVariable(v3, true));

    EXPECT_FALSE(v3->hasEquivalentVariable(v1, true));
    EXPECT_FALSE(v3->hasEquivalentVariable(v2, true));
    EXPECT_FALSE(v3->hasEquivalentVariable(v3, true));

    libcellml::Variable::addEquivalence(v1, v2);
    libcellml::Variable::addEquivalence(v2, v3);

    EXPECT_FALSE(v1->hasEquivalentVariable(v1, true));
    EXPECT_TRUE(v1->hasEquivalentVariable(v2, true));
    EXPECT_TRUE(v1->hasEquivalentVariable(v3, true));

    EXPECT_TRUE(v2->hasEquivalentVariable(v1, true));
    EXPECT_FALSE(v2->hasEquivalentVariable(v2, true));
    EXPECT_TRUE(v2->hasEquivalentVariable(v3, true));

    EXPECT_TRUE(v3->hasEquivalentVariable(v1, true));
    EXPECT_TRUE(v3->hasEquivalentVariable(v2, true));
    EXPECT_FALSE(v3->hasEquivalentVariable(v3, true));
}

TEST(Variable, addVariableToUnnamedComponent)
{
    const std::string in = "valid_name";
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component>\n"
        "    <variable name=\"valid_name\"/>\n"
        "  </component>\n"
        "</model>\n";

    libcellml::ModelPtr m = createModelWithComponent();
    libcellml::ComponentPtr c = m->component(0);
    libcellml::VariablePtr v = libcellml::Variable::create();

    v->setName(in);
    c->addVariable(v);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

TEST(Variable, addTwoVariables)
{
    const std::string in = "valid_name";
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"valid_name\">\n"
        "    <variable name=\"variable1\"/>\n"
        "    <variable name=\"variable2\"/>\n"
        "  </component>\n"
        "</model>\n";

    libcellml::ModelPtr m = createModelWithComponent();
    libcellml::ComponentPtr c = m->component(0);
    c->setName(in);

    libcellml::VariablePtr v1 = libcellml::Variable::create();
    v1->setName("variable1");
    c->addVariable(v1);

    libcellml::VariablePtr v2 = libcellml::Variable::create();
    v2->setName("variable2");
    c->addVariable(v2);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

TEST(Variable, addVariablesWithAndWithoutNameAndUnits)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component>\n"
        "    <variable name=\"var1\" units=\"dimensionless\"/>\n"
        "    <variable name=\"var2\"/>\n"
        "    <variable units=\"dimensionless\"/>\n"
        "    <variable/>\n"
        "  </component>\n"
        "</model>\n";

    libcellml::ModelPtr m = createModelWithComponent();
    libcellml::ComponentPtr c = m->component(0);
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    v1->setName("var1");
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    v2->setName("var2");
    libcellml::VariablePtr v3 = libcellml::Variable::create();
    libcellml::VariablePtr v4 = libcellml::Variable::create();

    c->addVariable(v1);
    c->addVariable(v2);
    c->addVariable(v3);
    c->addVariable(v4);

    libcellml::UnitsPtr u = libcellml::Units::create();
    u->setName("dimensionless");
    v1->setUnits(u);
    v3->setUnits(u);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

TEST(Variable, componentWithTwoVariablesWithInitialValues)
{
    const std::string in = "valid_name";
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"valid_name\">\n"
        "    <variable initial_value=\"1\"/>\n"
        "    <variable initial_value=\"-1\"/>\n"
        "  </component>\n"
        "</model>\n";

    libcellml::ModelPtr m = createModelWithComponent();
    libcellml::ComponentPtr c = m->component(0);
    c->setName(in);

    libcellml::VariablePtr v1 = libcellml::Variable::create();
    v1->setInitialValue(1.0);
    c->addVariable(v1);

    libcellml::VariablePtr v2 = libcellml::Variable::create();
    v2->setInitialValue(-1.0);
    c->addVariable(v2);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

TEST(Variable, removeVariableMethods)
{
    const std::string in = "valid_name";
    const std::string e1 =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"valid_name\">\n"
        "    <variable name=\"variable2\"/>\n"
        "  </component>\n"
        "</model>\n";
    const std::string e2 =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"valid_name\"/>\n"
        "</model>\n";

    libcellml::ModelPtr m = createModelWithComponent();
    libcellml::ComponentPtr c = m->component(0);
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    libcellml::VariablePtr v3 = libcellml::Variable::create();
    libcellml::VariablePtr v4 = libcellml::Variable::create();
    libcellml::VariablePtr v5 = libcellml::Variable::create();

    c->setName(in);
    v1->setName("variable1");
    v2->setName("variable2");
    v3->setName("variable3");
    v4->setName("variable4");

    c->addVariable(v1);
    c->addVariable(v2);
    c->addVariable(v3);

    EXPECT_TRUE(c->removeVariable("variable1"));
    EXPECT_TRUE(c->removeVariable(v3));
    libcellml::PrinterPtr printer = libcellml::Printer::create();
    std::string a = printer->printModel(m);
    EXPECT_EQ(e1, a);
    EXPECT_FALSE(c->removeVariable("BAD_NAME"));

    c->addVariable(v4);
    c->removeAllVariables();
    a = printer->printModel(m);
    EXPECT_EQ(e2, a);
    EXPECT_FALSE(c->removeVariable(v5));

    c->addVariable(v1);
    c->addVariable(v2);
    c->addVariable(v3);

    EXPECT_TRUE(c->removeVariable(0)); // v1
    EXPECT_TRUE(c->removeVariable(1)); // new index of v3
    a = printer->printModel(m);
    EXPECT_EQ(e1, a);
    EXPECT_FALSE(c->removeVariable(1));
}

TEST(Variable, variableMethods)
{
    const std::string in = "valid_name";
    libcellml::ComponentPtr c = libcellml::Component::create();
    c->setName(in);

    libcellml::VariablePtr v1 = libcellml::Variable::create();
    v1->setName("variable1");
    c->addVariable(v1);
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    v2->setName("variable2");
    c->addVariable(v2);
    libcellml::VariablePtr v3 = libcellml::Variable::create();
    v3->setName("variable3");
    c->addVariable(v3);
    libcellml::VariablePtr v4 = libcellml::Variable::create();
    v4->setName("variable4");
    c->addVariable(v4);

    // Get by string
    libcellml::VariablePtr vMethod1 = c->variable("variable1");
    const std::string a1 = vMethod1->name();
    EXPECT_EQ("variable1", a1);

    // Get by index
    libcellml::VariablePtr vMethod2 = c->variable(1);
    const std::string a2 = vMethod2->name();
    EXPECT_EQ("variable2", a2);

    // Get const by string
    const libcellml::VariablePtr vMethod3 = c->variable("variable3");
    const std::string a3 = vMethod3->name();
    EXPECT_EQ("variable3", a3);

    // Get const by index
    const libcellml::VariablePtr vMethod4 = c->variable(3);
    const std::string a4 = vMethod4->name();
    EXPECT_EQ("variable4", a4);

    // Get invalid index
    EXPECT_EQ(nullptr, c->variable(7));

    // Get non-existent variable by string
    EXPECT_EQ(nullptr, c->variable("doesntexist"));
}

TEST(Variable, takeVariableMethods)
{
    const std::string in = "valid_name";
    libcellml::ComponentPtr c = libcellml::Component::create();
    c->setName(in);

    libcellml::VariablePtr v1 = libcellml::Variable::create();
    v1->setName("variable1");
    c->addVariable(v1);
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    v2->setName("variable2");
    c->addVariable(v2);
    libcellml::VariablePtr v3 = libcellml::Variable::create();
    v3->setName("variable3");
    c->addVariable(v3);
    libcellml::VariablePtr v4 = libcellml::Variable::create();
    v4->setName("variable4");
    c->addVariable(v4);

    // Take by index
    libcellml::VariablePtr tv = c->takeVariable(0);
    std::string tvn = tv->name();
    EXPECT_EQ("variable1", tvn);
    libcellml::VariablePtr gv = c->variable(0);
    std::string gvn = gv->name();
    EXPECT_EQ("variable2", gvn);
    tv = c->takeVariable(0);
    tvn = tv->name();
    EXPECT_EQ("variable2", tvn);
    gv = c->variable(0);
    gvn = gv->name();
    EXPECT_EQ("variable3", gvn);

    // Take by string
    libcellml::VariablePtr tv3 = c->takeVariable("variable3");
    const std::string tvn3 = tv3->name();
    EXPECT_EQ("variable3", tvn3);

    // Get invalid index
    EXPECT_EQ(nullptr, c->takeVariable(737));

    // Get non-existent variable by string
    EXPECT_EQ(nullptr, c->takeVariable("notreal"));
    EXPECT_EQ(nullptr, c->takeVariable("doesntexist"));
}

TEST(Variable, modelWithComponentWithVariableWithValidName)
{
    const std::string in = "valid_name";
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"valid_name\">\n"
        "    <variable name=\"valid_name\" units=\"dimensionless\"/>\n"
        "  </component>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();

    libcellml::ComponentPtr c = libcellml::Component::create();
    c->setName(in);
    m->addComponent(c);

    libcellml::VariablePtr v = libcellml::Variable::create();
    v->setName(in);
    c->addVariable(v);

    libcellml::UnitsPtr u = libcellml::Units::create();
    u->setName("dimensionless");
    v->setUnits(u);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
    EXPECT_EQ("valid_name", v->name());
}

TEST(Variable, modelWithComponentWithVariableWithInvalidName)
{
    const std::string in = "invalid name";
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"invalid name\">\n"
        "    <variable name=\"invalid name\" units=\"dimensionless\"/>\n"
        "  </component>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();

    libcellml::ComponentPtr c = libcellml::Component::create();
    c->setName(in);
    m->addComponent(c);

    libcellml::VariablePtr v = libcellml::Variable::create();
    v->setName(in);
    c->addVariable(v);

    libcellml::UnitsPtr u = libcellml::Units::create();
    u->setName("dimensionless");
    v->setUnits(u);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
    EXPECT_EQ("invalid name", v->name());
}

TEST(Variable, modelWithComponentWithVariableWithInvalidUnitsNameAndParse)
{
    const std::string in = "valid_name";
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"valid_name\">\n"
        "    <variable name=\"valid_name\" units=\"invalid name\"/>\n"
        "  </component>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();

    libcellml::ComponentPtr c = libcellml::Component::create();
    c->setName(in);
    m->addComponent(c);

    libcellml::VariablePtr v = libcellml::Variable::create();
    v->setName(in);
    c->addVariable(v);

    libcellml::UnitsPtr u = libcellml::Units::create();
    u->setName("invalid name");
    v->setUnits(u);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
    EXPECT_EQ("invalid name", u->name());

    // Parse
    libcellml::ParserPtr parser = libcellml::Parser::create();
    libcellml::ModelPtr model = parser->parseModel(e);
    a = printer->printModel(model);
    EXPECT_EQ(e, a);
}

TEST(Variable, modelWithComponentWithTwoNamedVariablesWithInitialValues)
{
    const std::string in = "valid_name";
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"valid_name\">\n"
        "    <variable name=\"variable1\" initial_value=\"1.0\"/>\n"
        "    <variable name=\"variable2\" initial_value=\"-1.0\"/>\n"
        "  </component>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();

    libcellml::ComponentPtr c = libcellml::Component::create();
    c->setName(in);
    m->addComponent(c);

    libcellml::VariablePtr v1 = libcellml::Variable::create();
    v1->setName("variable1");
    v1->setInitialValue("1.0");
    c->addVariable(v1);

    libcellml::VariablePtr v2 = libcellml::Variable::create();
    v2->setName("variable2");
    v2->setInitialValue("-1.0");
    c->addVariable(v2);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

TEST(Variable, modelWithComponentWithTwoNamedVariablesWithInitialValuesOneReferenced)
{
    const std::string in = "valid_name";
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"valid_name\">\n"
        "    <variable name=\"variable1\" initial_value=\"1\"/>\n"
        "    <variable name=\"variable2\" initial_value=\"variable1\"/>\n"
        "  </component>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();

    libcellml::ComponentPtr c = libcellml::Component::create();
    c->setName(in);
    m->addComponent(c);

    libcellml::VariablePtr v1 = libcellml::Variable::create();
    v1->setName("variable1");
    v1->setInitialValue(1.0);
    c->addVariable(v1);

    libcellml::VariablePtr v2 = libcellml::Variable::create();
    v2->setName("variable2");
    v2->setInitialValue(v1);
    c->addVariable(v2);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

TEST(Variable, modelWithComponentWithTwoNamedVariablesWithInitialValuesAndParse)
{
    const std::string in = "valid_name";
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"valid_name\">\n"
        "    <variable name=\"variable1\" initial_value=\"1.0\"/>\n"
        "    <variable name=\"variable2\" initial_value=\"-1.0\"/>\n"
        "  </component>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();

    libcellml::ComponentPtr c = libcellml::Component::create();
    c->setName(in);
    m->addComponent(c);

    libcellml::VariablePtr v1 = libcellml::Variable::create();
    v1->setName("variable1");
    v1->setInitialValue("1.0");
    c->addVariable(v1);

    libcellml::VariablePtr v2 = libcellml::Variable::create();
    v2->setName("variable2");
    v2->setInitialValue("-1.0");
    c->addVariable(v2);

    libcellml::ParserPtr parser = libcellml::Parser::create();
    libcellml::ModelPtr model = parser->parseModel(e);
    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(model);
    EXPECT_EQ(e, a);
}

TEST(Variable, modelWithComponentWithFourNamedVariablesWithInterfaces)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"valid_name\">\n"
        "    <variable name=\"variable1\" interface=\"none\"/>\n"
        "    <variable name=\"variable2\" interface=\"public\"/>\n"
        "    <variable name=\"variable3\" interface=\"private\"/>\n"
        "    <variable name=\"variable4\" interface=\"public_and_private\"/>\n"
        "  </component>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();

    libcellml::ComponentPtr c = libcellml::Component::create();
    c->setName("valid_name");
    m->addComponent(c);

    libcellml::VariablePtr v1 = libcellml::Variable::create();
    v1->setName("variable1");
    v1->setInterfaceType(libcellml::Variable::InterfaceType::NONE);
    c->addVariable(v1);

    libcellml::VariablePtr v2 = libcellml::Variable::create();
    v2->setName("variable2");
    v2->setInterfaceType(libcellml::Variable::InterfaceType::PUBLIC);
    c->addVariable(v2);

    libcellml::VariablePtr v3 = libcellml::Variable::create();
    v3->setName("variable3");
    v3->setInterfaceType(libcellml::Variable::InterfaceType::PRIVATE);
    c->addVariable(v3);

    libcellml::VariablePtr v4 = libcellml::Variable::create();
    v4->setName("variable4");
    v4->setInterfaceType(libcellml::Variable::InterfaceType::PUBLIC_AND_PRIVATE);
    c->addVariable(v4);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

TEST(Variable, modelWithComponentWithFourNamedVariablesWithInterfacesAndParse)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"valid_name\">\n"
        "    <variable name=\"variable1\" interface=\"none\"/>\n"
        "    <variable name=\"variable2\" interface=\"public\"/>\n"
        "    <variable name=\"variable3\" interface=\"private\"/>\n"
        "    <variable name=\"variable4\" interface=\"public_and_private\"/>\n"
        "  </component>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();

    libcellml::ComponentPtr c = libcellml::Component::create();
    c->setName("valid_name");
    m->addComponent(c);

    libcellml::VariablePtr v1 = libcellml::Variable::create();
    v1->setName("variable1");
    v1->setInterfaceType(libcellml::Variable::InterfaceType::NONE);
    c->addVariable(v1);

    libcellml::VariablePtr v2 = libcellml::Variable::create();
    v2->setName("variable2");
    v2->setInterfaceType("public");
    c->addVariable(v2);

    libcellml::VariablePtr v3 = libcellml::Variable::create();
    v3->setName("variable3");
    v3->setInterfaceType(libcellml::Variable::InterfaceType::PRIVATE);
    c->addVariable(v3);

    libcellml::VariablePtr v4 = libcellml::Variable::create();
    v4->setName("variable4");
    v4->setInterfaceType(libcellml::Variable::InterfaceType::PUBLIC_AND_PRIVATE);
    c->addVariable(v4);

    libcellml::ParserPtr parser = libcellml::Parser::create();
    libcellml::ModelPtr model = parser->parseModel(e);
    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(model);
    EXPECT_EQ(e, a);
}

TEST(Variable, modelWithComponentWithFiveNamedVariablesWithInterfacesAndParse)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"valid_name\">\n"
        "    <variable name=\"variable1\" interface=\"none\"/>\n"
        "    <variable name=\"variable2\" interface=\"public\"/>\n"
        "    <variable name=\"variable3\" interface=\"private\"/>\n"
        "    <variable name=\"variable4\" interface=\"public_and_private\"/>\n"
        "    <variable name=\"variable5\"/>\n"
        "  </component>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();

    libcellml::ComponentPtr c = libcellml::Component::create();
    c->setName("valid_name");
    m->addComponent(c);

    libcellml::VariablePtr v1 = libcellml::Variable::create();
    v1->setName("variable1");
    v1->setInterfaceType(libcellml::Variable::InterfaceType::NONE);
    c->addVariable(v1);

    libcellml::VariablePtr v2 = libcellml::Variable::create();
    v2->setName("variable2");
    v2->setInterfaceType("public");
    c->addVariable(v2);

    libcellml::VariablePtr v3 = libcellml::Variable::create();
    v3->setName("variable3");
    v3->setInterfaceType(libcellml::Variable::InterfaceType::PRIVATE);
    c->addVariable(v3);

    libcellml::VariablePtr v4 = libcellml::Variable::create();
    v4->setName("variable4");
    v4->setInterfaceType(libcellml::Variable::InterfaceType::PUBLIC_AND_PRIVATE);
    c->addVariable(v4);

    libcellml::VariablePtr v5 = libcellml::Variable::create();
    v5->setName("variable4");
    v5->setInterfaceType("other");
    c->addVariable(v5);

    libcellml::ParserPtr parser = libcellml::Parser::create();
    libcellml::ModelPtr model = parser->parseModel(e);
    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(model);
    EXPECT_EQ(e, a);
}

TEST(Variable, modelUnitsAttributeBeforeNameAttribute)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model name=\"model\" xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <component name=\"valid_name\">\n"
        "    <variable units=\"dimensionless\" name=\"variable1\" interface=\"none\"/>\n"
        "    <variable id=\"sin\" units=\"dimensionless\" name=\"sin1\" interface=\"public_and_private\"/>\n"
        "    <variable id=\"deriv_approx_initial_value\" units=\"dimensionless\" initial_value=\"0\" name=\"deriv_approx_initial_value\" interface=\"public_and_private\"/>\n"
        "  </component>\n"
        "</model>\n";

    libcellml::ParserPtr parser = libcellml::Parser::create();
    parser->parseModel(e);
    EXPECT_EQ(size_t(0), parser->issueCount());
}

TEST(Variable, parentlessUsingRemoveVariable)
{
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr comp1 = libcellml::Component::create();

    libcellml::VariablePtr v1 = libcellml::Variable::create();

    m->setName("modelName");
    comp1->setName("component1");

    v1->setName("variable1");

    v1->setUnits("dimensionless");

    comp1->addVariable(v1);
    m->addComponent(comp1);

    EXPECT_TRUE(v1->hasParent());
    // Make a variable without a parent component.
    comp1->removeVariable(v1);

    EXPECT_EQ(size_t(0), comp1->variableCount());
    EXPECT_FALSE(v1->hasParent());
    EXPECT_EQ(nullptr, v1->parent());
}

TEST(Variable, parentlessUsingRemoveAllVariables)
{
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr comp1 = libcellml::Component::create();

    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();

    m->setName("modelName");
    comp1->setName("component1");

    v1->setName("variable1");
    v2->setName("variable2");

    v1->setUnits("dimensionless");
    v2->setUnits("dimensionless");

    comp1->addVariable(v1);
    comp1->addVariable(v2);
    m->addComponent(comp1);

    EXPECT_TRUE(v1->hasParent());
    EXPECT_TRUE(v2->hasParent());

    // Make a variable without a parent component.
    comp1->removeAllVariables();

    EXPECT_EQ(size_t(0), comp1->variableCount());
    EXPECT_FALSE(v1->hasParent());
    EXPECT_FALSE(v2->hasParent());
    EXPECT_EQ(nullptr, v1->parent());
    EXPECT_EQ(nullptr, v2->parent());
}

TEST(Variable, removeIndirectEquivalence)
{
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr comp1 = libcellml::Component::create();

    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    libcellml::VariablePtr v3 = libcellml::Variable::create();

    m->setName("modelName");
    comp1->setName("component1");

    v1->setName("variable1");
    v2->setName("variable2");
    v3->setName("variable3");

    v1->setUnits("dimensionless");
    v2->setUnits("dimensionless");
    v3->setUnits("dimensionless");

    comp1->addVariable(v1);
    comp1->addVariable(v2);
    comp1->addVariable(v3);
    m->addComponent(comp1);

    EXPECT_TRUE(v1->hasParent());
    EXPECT_TRUE(v2->hasParent());
    EXPECT_TRUE(v3->hasParent());

    // Make v1=v2=v3
    libcellml::Variable::addEquivalence(v2, v3);
    libcellml::Variable::addEquivalence(v1, v3);

    // All variables connected ...
    EXPECT_TRUE(v1->hasEquivalentVariable(v2, true));
    EXPECT_TRUE(v2->hasEquivalentVariable(v3, true));
    EXPECT_TRUE(v1->hasEquivalentVariable(v3, true));

    // ... but not directly through v1-v2
    EXPECT_FALSE(v1->hasEquivalentVariable(v2));
    EXPECT_TRUE(v1->hasEquivalentVariable(v3));
    EXPECT_TRUE(v2->hasEquivalentVariable(v3));

    // Attempt to remove v1=v2
    EXPECT_FALSE(libcellml::Variable::removeEquivalence(v1, v2));

    // But cannot remove indirect equivalence
    EXPECT_TRUE(v1->hasEquivalentVariable(v2, true));
    EXPECT_FALSE(v1->hasEquivalentVariable(v2)); // It never was anyway, no change here

    // Nothing else should have changed here
    EXPECT_TRUE(v1->hasEquivalentVariable(v3, true));
    EXPECT_TRUE(v2->hasEquivalentVariable(v3, true));
    EXPECT_TRUE(v1->hasEquivalentVariable(v3));
    EXPECT_TRUE(v2->hasEquivalentVariable(v3));
}

TEST(Variable, leaveExpiredVariable)
{
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    libcellml::VariablePtr v3 = libcellml::Variable::create();
    libcellml::Variable::addEquivalence(v1, v2);
    v2.reset();
    libcellml::Variable::addEquivalence(v1, v3);
    EXPECT_FALSE(v1->hasEquivalentVariable(v2));
    EXPECT_TRUE(v1->hasEquivalentVariable(v3));
}

TEST(Variable, addEquivalenceFirstParameterNullptr)
{
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr comp1 = libcellml::Component::create();

    libcellml::VariablePtr v1 = libcellml::Variable::create();

    m->setName("modelName");
    comp1->setName("component1");

    v1->setName("variable1");

    v1->setUnits("dimensionless");

    comp1->addVariable(v1);

    m->addComponent(comp1);

    EXPECT_FALSE(libcellml::Variable::addEquivalence(nullptr, v1));
}

TEST(Variable, addEquivalenceSecondParameterNullptr)
{
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr comp1 = libcellml::Component::create();

    libcellml::VariablePtr v1 = libcellml::Variable::create();

    m->setName("modelName");
    comp1->setName("component1");

    v1->setName("variable1");

    v1->setUnits("dimensionless");

    comp1->addVariable(v1);

    m->addComponent(comp1);

    EXPECT_FALSE(libcellml::Variable::addEquivalence(v1, nullptr));
}

TEST(Variable, addEquivalenceBothParametersNullptr)
{
    EXPECT_FALSE(libcellml::Variable::addEquivalence(nullptr, nullptr));
}

TEST(Variable, removeEquivalenceFirstParametersNullptr)
{
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr comp1 = libcellml::Component::create();

    libcellml::VariablePtr v1 = libcellml::Variable::create();

    m->setName("modelName");
    comp1->setName("component1");

    v1->setName("variable1");

    v1->setUnits("dimensionless");

    comp1->addVariable(v1);

    m->addComponent(comp1);

    EXPECT_FALSE(libcellml::Variable::removeEquivalence(nullptr, v1));
}

TEST(Variable, removeEquivalenceSecondParametersNullptr)
{
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr comp1 = libcellml::Component::create();

    libcellml::VariablePtr v1 = libcellml::Variable::create();

    m->setName("modelName");
    comp1->setName("component1");

    v1->setName("variable1");

    v1->setUnits("dimensionless");

    comp1->addVariable(v1);

    m->addComponent(comp1);

    EXPECT_FALSE(libcellml::Variable::removeEquivalence(v1, nullptr));
}

TEST(Variable, removeEquivalenceBothParametersNullptr)
{
    EXPECT_FALSE(libcellml::Variable::removeEquivalence(nullptr, nullptr));
}

TEST(Variable, removeEquivalenceFromNonEquivalentVariables)
{
    libcellml::VariablePtr v1 = libcellml::Variable::create("var1");
    libcellml::VariablePtr v2 = libcellml::Variable::create("var2");

    EXPECT_FALSE(libcellml::Variable::removeEquivalence(v1, v2));
}

TEST(Variable, removeAllEquivalences)
{
    libcellml::VariablePtr v1 = libcellml::Variable::create("var1");
    libcellml::VariablePtr v2 = libcellml::Variable::create("var2");
    libcellml::VariablePtr v3 = libcellml::Variable::create("var2");

    EXPECT_TRUE(libcellml::Variable::addEquivalence(v1, v2));
    EXPECT_TRUE(libcellml::Variable::addEquivalence(v1, v3));

    EXPECT_EQ(size_t(2), v1->equivalentVariableCount());

    v1->removeAllEquivalences();

    EXPECT_EQ(size_t(0), v1->equivalentVariableCount());
}

TEST(Variable, removeAllEquivalencesWithDeletedVariable)
{
    libcellml::VariablePtr v1 = libcellml::Variable::create("var1");
    libcellml::VariablePtr v2 = libcellml::Variable::create("var2");
    libcellml::VariablePtr v3 = libcellml::Variable::create("var2");

    EXPECT_TRUE(libcellml::Variable::addEquivalence(v1, v2));
    EXPECT_TRUE(libcellml::Variable::addEquivalence(v1, v3));

    EXPECT_EQ(size_t(2), v1->equivalentVariableCount());

    v3 = nullptr;

    v1->removeAllEquivalences();

    EXPECT_EQ(size_t(0), v1->equivalentVariableCount());
}

TEST(Variable, hasEquivalentVariableWithNullptr)
{
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();

    m->setName("modelName");
    c1->setName("component1");
    v1->setName("variable1");
    v1->setUnits("dimensionless");

    c1->addVariable(v1);
    m->addComponent(c1);

    EXPECT_FALSE(v1->hasEquivalentVariable(nullptr));
}

TEST(Variable, variableEquivalenceInterface)
{
    libcellml::ModelPtr model = libcellml::Model::create();
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    libcellml::ComponentPtr c2 = libcellml::Component::create();
    libcellml::ComponentPtr c3 = libcellml::Component::create();

    model->setName("model");
    c1->setName("c1");
    c2->setName("c2");
    c3->setName("c3");

    model->addComponent(c1);
    model->addComponent(c2);
    c2->addComponent(c3);

    libcellml::VariablePtr v1 = libcellml::Variable::create();
    v1->setName("v1");
    v1->setUnits("dimensionless");

    libcellml::VariablePtr v2 = libcellml::Variable::create();
    v2->setName("v2");
    v2->setUnits("dimensionless");

    libcellml::VariablePtr v3 = libcellml::Variable::create();
    v3->setName("v3");
    v3->setUnits("dimensionless");

    c1->addVariable(v1);
    c2->addVariable(v2);
    c3->addVariable(v3);

    libcellml::Variable::addEquivalence(v1, v2);
    libcellml::Variable::addEquivalence(v2, v3);

    EXPECT_TRUE(model->fixVariableInterfaces());

    EXPECT_EQ("public", v1->interfaceType());
    EXPECT_EQ("public_and_private", v2->interfaceType());
    EXPECT_EQ("public", v3->interfaceType());
}

TEST(Variable, variableEquivalencePublicInterface)
{
    libcellml::ModelPtr model = libcellml::Model::create();
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    libcellml::ComponentPtr c2 = libcellml::Component::create();

    model->setName("model");
    c1->setName("c1");
    c2->setName("c2");

    model->addComponent(c1);
    model->addComponent(c2);

    libcellml::VariablePtr v1 = libcellml::Variable::create();
    v1->setName("v1");
    v1->setUnits("dimensionless");

    libcellml::VariablePtr v2 = libcellml::Variable::create();
    v2->setName("v2");
    v2->setUnits("dimensionless");

    c1->addVariable(v1);
    c2->addVariable(v2);

    libcellml::Variable::addEquivalence(v1, v2);

    EXPECT_TRUE(model->fixVariableInterfaces());

    EXPECT_EQ("public", v1->interfaceType());
    EXPECT_EQ("public", v2->interfaceType());
}

TEST(Variable, variableEquivalencePrivateInterface)
{
    libcellml::ModelPtr model = libcellml::Model::create();
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    libcellml::ComponentPtr c2 = libcellml::Component::create();

    model->setName("model");
    c1->setName("c1");
    c2->setName("c2");

    model->addComponent(c1);
    c1->addComponent(c2);

    libcellml::VariablePtr v1 = libcellml::Variable::create();
    v1->setName("v1");
    v1->setUnits("dimensionless");

    libcellml::VariablePtr v2 = libcellml::Variable::create();
    v2->setName("v2");
    v2->setUnits("dimensionless");

    c1->addVariable(v1);
    c2->addVariable(v2);

    libcellml::Variable::addEquivalence(v1, v2);

    EXPECT_TRUE(model->fixVariableInterfaces());

    EXPECT_EQ("private", v1->interfaceType());
    EXPECT_EQ("public", v2->interfaceType());
}

TEST(Variable, variableEquivalencePublicAndPrivateInterface)
{
    libcellml::ModelPtr model = libcellml::Model::create();
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    libcellml::ComponentPtr c2 = libcellml::Component::create();
    libcellml::ComponentPtr c3 = libcellml::Component::create();

    model->setName("model");
    c1->setName("c1");
    c2->setName("c2");
    c3->setName("c3");

    model->addComponent(c1);
    c1->addComponent(c2);
    c2->addComponent(c3);

    libcellml::VariablePtr v1 = libcellml::Variable::create();
    v1->setName("v1");
    v1->setUnits("dimensionless");

    libcellml::VariablePtr v2 = libcellml::Variable::create();
    v2->setName("v2");
    v2->setUnits("dimensionless");

    libcellml::VariablePtr v3 = libcellml::Variable::create();
    v3->setName("v3");
    v3->setUnits("dimensionless");

    c1->addVariable(v1);
    c2->addVariable(v2);
    c3->addVariable(v3);

    libcellml::Variable::addEquivalence(v1, v2);
    libcellml::Variable::addEquivalence(v3, v2);

    EXPECT_TRUE(model->fixVariableInterfaces());

    EXPECT_EQ("private", v1->interfaceType());
    EXPECT_EQ("public_and_private", v2->interfaceType());
    EXPECT_EQ("public", v3->interfaceType());
}

TEST(Variable, distantVariableEquivalence)
{
    libcellml::ModelPtr model = libcellml::Model::create();
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    libcellml::ComponentPtr c2 = libcellml::Component::create();
    libcellml::ComponentPtr c3 = libcellml::Component::create();

    model->setName("model");
    c1->setName("c1");
    c2->setName("c2");
    c3->setName("c3");

    model->addComponent(c1);
    c1->addComponent(c2);
    c2->addComponent(c3);

    libcellml::VariablePtr v1 = libcellml::Variable::create();
    v1->setName("v1");
    v1->setUnits("dimensionless");

    libcellml::VariablePtr v2 = libcellml::Variable::create();
    v2->setName("v2");
    v2->setUnits("dimensionless");

    libcellml::VariablePtr v3 = libcellml::Variable::create();
    v3->setName("v3");
    v3->setUnits("dimensionless");

    c1->addVariable(v1);
    c2->addVariable(v2);
    c3->addVariable(v3);

    libcellml::Variable::addEquivalence(v1, v3);

    EXPECT_FALSE(model->fixVariableInterfaces());

    // Couldn't determine interface types for variables that are
    // too distant from each other in the component hierarchy.
    EXPECT_EQ("", v1->interfaceType());
    EXPECT_EQ("", v3->interfaceType());
}

TEST(Variable, variableEquivalencePromoteFromPrivate)
{
    libcellml::ModelPtr model = libcellml::Model::create();
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    libcellml::ComponentPtr c2 = libcellml::Component::create();

    model->setName("model");
    c1->setName("c1");
    c2->setName("c2");

    model->addComponent(c1);
    c1->addComponent(c2);

    libcellml::VariablePtr v1 = libcellml::Variable::create();
    v1->setName("v1");
    v1->setUnits("dimensionless");

    libcellml::VariablePtr v2 = libcellml::Variable::create();
    v2->setName("v2");
    v2->setUnits("dimensionless");

    c1->addVariable(v1);
    c2->addVariable(v2);

    v2->setInterfaceType("private");

    libcellml::Variable::addEquivalence(v1, v2);

    EXPECT_TRUE(model->fixVariableInterfaces());

    EXPECT_EQ("private", v1->interfaceType());
    EXPECT_EQ("public", v2->interfaceType());
}

TEST(Variable, variableInterfaceDontDowngrade)
{
    libcellml::ModelPtr model = libcellml::Model::create();
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    libcellml::ComponentPtr c2 = libcellml::Component::create();
    libcellml::ComponentPtr c3 = libcellml::Component::create();
    libcellml::ComponentPtr c4 = libcellml::Component::create();

    model->setName("model");
    c1->setName("c1");
    c2->setName("c2");
    c3->setName("c3");
    c4->setName("c4");

    model->addComponent(c1);
    model->addComponent(c4);
    c1->addComponent(c2);
    c2->addComponent(c3);

    libcellml::VariablePtr v1 = libcellml::Variable::create();
    v1->setName("v1");
    v1->setUnits("dimensionless");
    v1->setInterfaceType("public");

    libcellml::VariablePtr v3 = libcellml::Variable::create();
    v3->setName("v3");
    v3->setUnits("dimensionless");

    libcellml::VariablePtr v4 = libcellml::Variable::create();
    v4->setName("v4");
    v4->setUnits("dimensionless");

    c1->addVariable(v1);
    c3->addVariable(v3);
    c4->addVariable(v4);

    libcellml::Variable::addEquivalence(v1, v4);
    libcellml::Variable::addEquivalence(v1, v3);

    EXPECT_FALSE(model->fixVariableInterfaces());

    EXPECT_EQ("public", v1->interfaceType());
    EXPECT_EQ("", v3->interfaceType());
    EXPECT_EQ("public", v4->interfaceType());
}

TEST(Variable, variableInterfaceDontDowngradeFromPublicAndPrivate)
{
    libcellml::ModelPtr model = libcellml::Model::create();
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    libcellml::ComponentPtr c2 = libcellml::Component::create();
    libcellml::ComponentPtr c3 = libcellml::Component::create();

    model->setName("model");
    c1->setName("c1");
    c2->setName("c2");
    c3->setName("c3");

    model->addComponent(c1);
    c1->addComponent(c2);
    c2->addComponent(c3);

    libcellml::VariablePtr v1 = libcellml::Variable::create();
    v1->setName("v1");
    v1->setUnits("dimensionless");
    v1->setInterfaceType("public_and_private");

    libcellml::VariablePtr v2 = libcellml::Variable::create();
    v2->setName("v2");
    v2->setUnits("dimensionless");

    libcellml::VariablePtr v3 = libcellml::Variable::create();
    v3->setName("v3");
    v3->setUnits("dimensionless");

    c1->addVariable(v1);
    c2->addVariable(v2);
    c3->addVariable(v3);

    libcellml::Variable::addEquivalence(v1, v2);
    libcellml::Variable::addEquivalence(v2, v3);

    EXPECT_TRUE(model->fixVariableInterfaces());

    EXPECT_EQ("public_and_private", v1->interfaceType());
    EXPECT_EQ("public_and_private", v2->interfaceType());
    EXPECT_EQ("public", v3->interfaceType());
}

TEST(Variable, minimumInterfaceType)
{
    auto vPublic = libcellml::Variable::create("vPublic");
    vPublic->setInterfaceType("public");

    auto vPrivate = libcellml::Variable::create("vPrivate");
    vPrivate->setInterfaceType("private");

    auto vPublicAndPrivate = libcellml::Variable::create("vPublicAndPrivate");
    vPublicAndPrivate->setInterfaceType("public_and_private");

    auto vNone = libcellml::Variable::create("vNone");
    vNone->setInterfaceType("none");

    auto vEmpty = libcellml::Variable::create("vEmpty");

    // Stored public_and_private meets all requirements.
    EXPECT_TRUE(vPublicAndPrivate->permitsInterfaceType(libcellml::Variable::InterfaceType::NONE));
    EXPECT_TRUE(vPublicAndPrivate->permitsInterfaceType(libcellml::Variable::InterfaceType::PRIVATE));
    EXPECT_TRUE(vPublicAndPrivate->permitsInterfaceType(libcellml::Variable::InterfaceType::PUBLIC));
    EXPECT_TRUE(vPublicAndPrivate->permitsInterfaceType(libcellml::Variable::InterfaceType::PUBLIC_AND_PRIVATE));

    // Stored private meets private and none requirements.
    EXPECT_TRUE(vPrivate->permitsInterfaceType(libcellml::Variable::InterfaceType::NONE));
    EXPECT_TRUE(vPrivate->permitsInterfaceType(libcellml::Variable::InterfaceType::PRIVATE));
    EXPECT_FALSE(vPrivate->permitsInterfaceType(libcellml::Variable::InterfaceType::PUBLIC));
    EXPECT_FALSE(vPrivate->permitsInterfaceType(libcellml::Variable::InterfaceType::PUBLIC_AND_PRIVATE));

    // Stored public meets public and none requirements.
    EXPECT_TRUE(vPublic->permitsInterfaceType(libcellml::Variable::InterfaceType::NONE));
    EXPECT_FALSE(vPublic->permitsInterfaceType(libcellml::Variable::InterfaceType::PRIVATE));
    EXPECT_TRUE(vPublic->permitsInterfaceType(libcellml::Variable::InterfaceType::PUBLIC));
    EXPECT_FALSE(vPublic->permitsInterfaceType(libcellml::Variable::InterfaceType::PUBLIC_AND_PRIVATE));

    // Stored none meets none requirements.
    EXPECT_TRUE(vNone->permitsInterfaceType(libcellml::Variable::InterfaceType::NONE));
    EXPECT_FALSE(vNone->permitsInterfaceType(libcellml::Variable::InterfaceType::PRIVATE));
    EXPECT_FALSE(vNone->permitsInterfaceType(libcellml::Variable::InterfaceType::PUBLIC));
    EXPECT_FALSE(vNone->permitsInterfaceType(libcellml::Variable::InterfaceType::PUBLIC_AND_PRIVATE));

    // Stored empty meets none requirements.
    EXPECT_TRUE(vEmpty->permitsInterfaceType(libcellml::Variable::InterfaceType::NONE));
    EXPECT_FALSE(vEmpty->permitsInterfaceType(libcellml::Variable::InterfaceType::PRIVATE));
    EXPECT_FALSE(vEmpty->permitsInterfaceType(libcellml::Variable::InterfaceType::PUBLIC));
    EXPECT_FALSE(vEmpty->permitsInterfaceType(libcellml::Variable::InterfaceType::PUBLIC_AND_PRIVATE));
}

TEST(Variable, connectionsPersistAfterImporting)
{
    auto model = libcellml::Model::create("model");
    auto importer = libcellml::Importer::create();

    auto importedComponent = libcellml::Component::create("importedComponent");
    model->addComponent(importedComponent);

    auto importSource = libcellml::ImportSource::create();
    importSource->setUrl("importedModelWithMaps.cellml");
    importedComponent->setImportSource(importSource);
    importedComponent->setImportReference("importMe");

    EXPECT_TRUE(model->hasUnresolvedImports());
    importer->resolveImports(model, resourcePath());
    EXPECT_FALSE(model->hasUnresolvedImports());

    model = importer->flattenModel(model);

    EXPECT_NE(nullptr, model->component("importedComponent"));
    EXPECT_NE(nullptr, model->component("importedComponent")->component("child1"));
    EXPECT_NE(nullptr, model->component("importedComponent")->component("child2"));
    EXPECT_NE(nullptr, model->component("importedComponent")->component("child1")->variable("x"));
    EXPECT_NE(nullptr, model->component("importedComponent")->component("child2")->variable("x"));

    auto x1 = model->component("importedComponent")->component("child1")->variable("x");
    auto x2 = model->component("importedComponent")->component("child2")->variable("x");
    EXPECT_EQ(size_t(1), x1->equivalentVariableCount());
    EXPECT_EQ(size_t(1), x2->equivalentVariableCount());
    EXPECT_EQ(x1, x2->equivalentVariable(0));
    EXPECT_EQ(x2, x1->equivalentVariable(0));
}

TEST(Variable, addVariableDuplicates)
{
    auto model = libcellml::Model::create("model");
    auto tomato = libcellml::Component::create("tomato");
    auto apple = libcellml::Component::create("apple");
    auto pip = libcellml::Variable::create("pip");

    EXPECT_TRUE(model->addComponent(tomato));
    EXPECT_TRUE(model->addComponent(apple));

    // Adding a pip to the tomato.
    EXPECT_TRUE(tomato->addVariable(pip));
    EXPECT_EQ(size_t(1), tomato->variableCount());

    // Add the same pip again.
    EXPECT_TRUE(tomato->addVariable(pip));
    EXPECT_EQ(size_t(2), tomato->variableCount());

    // Add the same pip to the apple this time, which will effectively move it
    // from the tomato to the apple. Leaving one pip in the tomato.
    EXPECT_TRUE(apple->addVariable(pip));

    EXPECT_EQ(size_t(1), apple->variableCount());
    EXPECT_EQ(size_t(1), tomato->variableCount());
}
