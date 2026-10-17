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

TEST(Units, construct)
{
    libcellml::UnitsPtr u = libcellml::Units::create();
    u->setName("valid_name");
    EXPECT_EQ("valid_name", u->name());

    const std::string validName = "another_valid_name";
    auto u2 = libcellml::Units::create(validName);
    EXPECT_EQ("another_valid_name", u2->name());
}

TEST(Units, validName)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <units name=\"valid_name\"/>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();

    libcellml::UnitsPtr u = libcellml::Units::create();
    u->setName("valid_name");

    m->addUnits(u);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
    EXPECT_EQ("valid_name", u->name());
}

TEST(Units, invalidName)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <units name=\"invalid name\"/>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();

    libcellml::UnitsPtr u = libcellml::Units::create();
    u->setName("invalid name");

    m->addUnits(u);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
    EXPECT_EQ("invalid name", u->name());
}

TEST(Units, compoundUnitsRaw)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <units name=\"compound_unit\">\n"
        "    <unit prefix=\"-6\" units=\"ampere\"/>\n"
        "    <unit units=\"kelvin\"/>\n"
        "    <unit exponent=\"-1\" prefix=\"-3\" units=\"siemens\"/>\n"
        "  </units>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();

    libcellml::UnitsPtr u = libcellml::Units::create();
    u->setName("compound_unit");

    u->addUnit("ampere", -6, 1.0);
    u->addUnit("kelvin");
    u->addUnit("siemens", -3, -1.0);

    m->addUnits(u);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

TEST(Units, addUnitsVariations)
{
    libcellml::UnitsPtr u = libcellml::Units::create();
    u->setName("compound_unit");

    u->addUnit(libcellml::Units::StandardUnit::AMPERE, libcellml::Units::Prefix::MICRO);
    u->addUnit(libcellml::Units::StandardUnit::KELVIN, -3, 2.0, 5.5);

    EXPECT_EQ(size_t(2), u->unitCount());
}

TEST(Units, compoundUnitsUsingDefines)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <units name=\"compound_unit\">\n"
        "    <unit prefix=\"micro\" units=\"ampere\"/>\n"
        "    <unit units=\"kelvin\"/>\n"
        "    <unit exponent=\"-1\" prefix=\"milli\" units=\"siemens\"/>\n"
        "  </units>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();

    libcellml::UnitsPtr u = libcellml::Units::create();
    u->setName("compound_unit");

    u->addUnit(libcellml::Units::StandardUnit::AMPERE, libcellml::Units::Prefix::MICRO);
    u->addUnit(libcellml::Units::StandardUnit::KELVIN);
    u->addUnit(libcellml::Units::StandardUnit::SIEMENS, libcellml::Units::Prefix::MILLI, -1.0);

    m->addUnits(u);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

TEST(Units, compoundUnitsUsingDefinesAndStringUnitsAndPrefix)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <units name=\"compound_unit\">\n"
        "    <unit prefix=\"micro\" units=\"ampere\"/>\n"
        "    <unit units=\"kelvin\"/>\n"
        "    <unit exponent=\"-1\" prefix=\"milli\" units=\"siemens\"/>\n"
        "    <unit prefix=\"1.7e310\" units=\"metre\"/>\n"
        "  </units>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();

    libcellml::UnitsPtr u = libcellml::Units::create();
    u->setName("compound_unit");

    u->addUnit(libcellml::Units::StandardUnit::AMPERE, "micro");
    u->addUnit("kelvin");
    u->addUnit("siemens", "milli", -1.0);
    u->addUnit("metre", "1.7e310");

    m->addUnits(u);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

TEST(Units, removeUnitsMethodsAndCount)
{
    const std::string e1 =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <units name=\"compound_unit\">\n"
        "    <unit prefix=\"micro\" units=\"ampere\"/>\n"
        "    <unit units=\"kelvin\"/>\n"
        "    <unit exponent=\"-1\" prefix=\"milli\" units=\"siemens\"/>\n"
        "    <unit prefix=\"1.7e10\" units=\"metre\"/>\n"
        "  </units>\n"
        "  <units name=\"simple_unit_2\"/>\n"
        "  <units name=\"simple_unit_3\"/>\n"
        "  <units name=\"simple_unit_4\"/>\n"
        "</model>\n";
    const std::string e2 =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <units name=\"compound_unit\"/>\n"
        "  <units name=\"simple_unit_2\"/>\n"
        "  <units name=\"simple_unit_3\"/>\n"
        "  <units name=\"simple_unit_4\"/>\n"
        "</model>\n";
    const std::string e3 =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <units name=\"compound_unit\"/>\n"
        "  <units name=\"simple_unit_4\"/>\n"
        "</model>\n";
    const std::string e4 =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\"/>\n";

    libcellml::ModelPtr m = libcellml::Model::create();

    libcellml::UnitsPtr u1 = libcellml::Units::create();
    libcellml::UnitsPtr u2 = libcellml::Units::create();
    libcellml::UnitsPtr u3 = libcellml::Units::create();
    libcellml::UnitsPtr u4 = libcellml::Units::create();
    libcellml::UnitsPtr u5 = libcellml::Units::create();
    u1->setName("compound_unit");
    u2->setName("simple_unit_2");
    u3->setName("simple_unit_3");
    u4->setName("simple_unit_4");

    u1->addUnit(libcellml::Units::StandardUnit::AMPERE, "micro");
    u1->addUnit("kelvin");
    u1->addUnit("siemens", "milli", -1.0);
    u1->addUnit("metre", "1.7e10");
    m->addUnits(u1);
    m->addUnits(u2);
    m->addUnits(u3);
    m->addUnits(u4);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    std::string a = printer->printModel(m);
    EXPECT_EQ(e1, a);

    u1->removeAllUnits();
    a = printer->printModel(m);
    EXPECT_EQ(e2, a);

    EXPECT_TRUE(m->removeUnits("simple_unit_2"));
    EXPECT_TRUE(m->removeUnits(u3));
    a = printer->printModel(m);
    EXPECT_EQ(e3, a);
    EXPECT_EQ(size_t(2), m->unitsCount());

    EXPECT_FALSE(m->removeUnits("gram"));
    EXPECT_FALSE(m->removeUnits(u5));
    EXPECT_FALSE(m->removeUnits(3));
    EXPECT_EQ(size_t(2), m->unitsCount());

    EXPECT_TRUE(m->removeUnits(1));
    EXPECT_EQ(size_t(1), m->unitsCount());

    m->removeAllUnits();
    a = printer->printModel(m);
    EXPECT_EQ(e4, a);
}

TEST(Units, hasUnitsName)
{
    libcellml::ModelPtr m = libcellml::Model::create();

    libcellml::UnitsPtr u = libcellml::Units::create();
    u->setName("a_unit");

    u->addUnit(libcellml::Units::StandardUnit::AMPERE, "micro");
    m->addUnits(u);
    EXPECT_TRUE(m->hasUnits("a_unit"));
}

TEST(Units, hasUnitsPtr)
{
    libcellml::ModelPtr m = libcellml::Model::create();

    libcellml::UnitsPtr u = libcellml::Units::create();
    u->setName("a_unit");

    u->addUnit(libcellml::Units::StandardUnit::AMPERE, "micro");
    m->addUnits(u);
    EXPECT_TRUE(m->hasUnits(u));
}

TEST(Units, takeUnits)
{
    libcellml::ModelPtr m = libcellml::Model::create();

    libcellml::UnitsPtr u1 = libcellml::Units::create();
    libcellml::UnitsPtr u2 = libcellml::Units::create();
    libcellml::UnitsPtr u3 = libcellml::Units::create();

    u1->setName("a_unit");
    u2->setName("b_unit");
    u3->setName("c_unit");

    u1->addUnit(libcellml::Units::StandardUnit::AMPERE, "micro");
    m->addUnits(u1);
    m->addUnits(u2);
    m->addUnits(u3);

    libcellml::UnitsPtr u4 = m->takeUnits("b_unit");
    EXPECT_EQ("b_unit", u4->name());
    EXPECT_EQ(size_t(2), m->unitsCount());

    libcellml::UnitsPtr u5 = m->takeUnits(1);
    EXPECT_EQ("c_unit", u5->name());
    EXPECT_EQ(size_t(1), m->unitsCount());

    EXPECT_EQ(nullptr, m->takeUnits(7));

    EXPECT_EQ(nullptr, m->takeUnits("d_unit"));
}

TEST(Units, replaceUnits)
{
    libcellml::ModelPtr m = libcellml::Model::create();

    libcellml::UnitsPtr u1 = libcellml::Units::create();
    libcellml::UnitsPtr u2 = libcellml::Units::create();
    libcellml::UnitsPtr u3 = libcellml::Units::create();

    u1->setName("a_unit");
    u2->setName("b_unit");
    u3->setName("c_unit");

    u1->addUnit(libcellml::Units::StandardUnit::AMPERE, "micro");
    m->addUnits(u1);
    m->addUnits(u2);

    EXPECT_TRUE(m->replaceUnits("b_unit", u3));
    EXPECT_EQ(size_t(2), m->unitsCount());

    libcellml::UnitsPtr u4 = m->takeUnits(1);
    EXPECT_EQ("c_unit", u4->name());
    EXPECT_EQ(size_t(1), m->unitsCount());

    EXPECT_TRUE(m->replaceUnits(0, u4));

    u1 = m->units(0);
    EXPECT_EQ("c_unit", u1->name());
    EXPECT_EQ(size_t(1), m->unitsCount());

    // Replace non-existent units.
    EXPECT_FALSE(m->replaceUnits("d_unit", u2));
    EXPECT_FALSE(m->replaceUnits(5, u1));

    // Replace with pointers
    EXPECT_FALSE(m->replaceUnits(u2, u1));
    EXPECT_TRUE(m->replaceUnits(u1, u2));
    EXPECT_FALSE(m->replaceUnits(u1, u2));
    EXPECT_TRUE(m->replaceUnits(u2, u1));
}

TEST(Units, hasUnlinkedUnitsWhenBaseUnitsAddedToVariableButNotModel)
{
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c1 = libcellml::Component::create();

    libcellml::UnitsPtr u1 = libcellml::Units::create();

    libcellml::VariablePtr v1 = libcellml::Variable::create();

    u1->setName("a_unit");

    v1->setUnits(u1);
    c1->addVariable(v1);
    m->addComponent(c1);

    EXPECT_TRUE(m->hasUnlinkedUnits());
}

TEST(Units, hasUnlinkedUnitsWhenVariableHasNoUnits)
{
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c1 = libcellml::Component::create();

    libcellml::VariablePtr v1 = libcellml::Variable::create();

    c1->addVariable(v1);
    m->addComponent(c1);

    EXPECT_FALSE(m->hasUnlinkedUnits());
}

TEST(Units, hasLinkedUnitsWhenUnitsObjectAddedToBothModelAndVariable)
{
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    libcellml::UnitsPtr u1 = libcellml::Units::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();

    u1->setName("a_unit");

    v1->setUnits(u1);

    c1->addVariable(v1);
    m->addComponent(c1);
    m->addUnits(u1);

    EXPECT_FALSE(m->hasUnlinkedUnits());
}

TEST(Units, hasUnlinkedUnitsWhenUnitsObjectsWithSameNameAddedToModelAndVariable)
{
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    libcellml::UnitsPtr u1 = libcellml::Units::create();
    libcellml::UnitsPtr u2 = libcellml::Units::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();

    u1->setName("a_unit");
    u2->setName("a_unit");

    v1->setUnits(u1);

    c1->addVariable(v1);
    m->addComponent(c1);
    m->addUnits(u2);

    EXPECT_TRUE(m->hasUnlinkedUnits());
}

TEST(Units, hasUnlinkedUnitsWhenBaseUnitsAddedToVariableByName)
{
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c1 = libcellml::Component::create();

    libcellml::UnitsPtr u1 = libcellml::Units::create();

    libcellml::VariablePtr v1 = libcellml::Variable::create();

    u1->setName("a_unit");

    c1->addVariable(v1);
    m->addComponent(c1);

    m->addUnits(u1);
    v1->setUnits("a_unit");

    EXPECT_TRUE(m->hasUnlinkedUnits());
}

TEST(Units, hasUnlinkedUnitsWhenBaseUnitsAddedToModelAfterNameUsedForVariableUnits)
{
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    libcellml::UnitsPtr u1 = libcellml::Units::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();

    u1->setName("a_unit");

    c1->addVariable(v1);
    m->addComponent(c1);

    v1->setUnits("a_unit");
    m->addUnits(u1);

    EXPECT_TRUE(m->hasUnlinkedUnits());
}

TEST(Units, hasUnlinkedUnitsLayeredComponentsWithVariables)
{
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    libcellml::ComponentPtr c2 = libcellml::Component::create();
    libcellml::ComponentPtr c3 = libcellml::Component::create();
    libcellml::UnitsPtr u1 = libcellml::Units::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    libcellml::VariablePtr v3 = libcellml::Variable::create();
    libcellml::VariablePtr v4 = libcellml::Variable::create();

    u1->setName("a_unit");
    u1->addUnit("second");

    v2->setUnits(u1);
    v3->setUnits(u1);

    c1->addVariable(v1);
    c2->addVariable(v2);
    c3->addVariable(v3);
    c3->addVariable(v4);

    c2->addComponent(c3);
    c1->addComponent(c2);
    m->addComponent(c1);

    v1->setUnits("a_unit");
    m->addUnits(u1);
    v4->setUnits(u1);

    EXPECT_TRUE(m->hasUnlinkedUnits());
}

TEST(Units, hasUnlinkedUnitsSiblingComponentsWithVariables)
{
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    libcellml::ComponentPtr c2 = libcellml::Component::create();
    libcellml::ComponentPtr c3 = libcellml::Component::create();
    libcellml::UnitsPtr u1 = libcellml::Units::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    libcellml::VariablePtr v3 = libcellml::Variable::create();
    libcellml::VariablePtr v4 = libcellml::Variable::create();

    u1->setName("a_unit");
    u1->addUnit("second");

    c1->addVariable(v1);
    c2->addVariable(v2);
    c3->addVariable(v3);
    c3->addVariable(v4);

    c1->addComponent(c2);
    c1->addComponent(c3);
    m->addComponent(c1);

    v4->setUnits("a_unit");

    m->addUnits(u1);
    v1->setUnits("a_unit");
    v2->setUnits(u1);
    v3->setUnits(u1);

    EXPECT_TRUE(m->hasUnlinkedUnits());
}

TEST(Units, hasUnlinkedUnitsInDeepComponentHierarchy)
{
    libcellml::ModelPtr model = libcellml::Model::create();
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    libcellml::ComponentPtr c2 = libcellml::Component::create();
    libcellml::ComponentPtr c3 = libcellml::Component::create();

    libcellml::UnitsPtr u = libcellml::Units::create();

    model->setName("model");
    u->setName("my_units");
    model->addUnits(u);

    c1->setName("c1");
    c2->setName("c2");
    c3->setName("c3");

    model->addComponent(c1);
    c1->addComponent(c2);
    c2->addComponent(c3);

    c3->addVariable(createVariableWithUnits("v", "my_units"));

    EXPECT_TRUE(model->hasUnlinkedUnits());

    model->linkUnits();
    EXPECT_FALSE(model->hasUnlinkedUnits());
}

TEST(Units, linkUnitsViaName)
{
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    libcellml::UnitsPtr u1 = libcellml::Units::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();

    u1->setName("a_unit");

    c1->addVariable(v1);
    m->addComponent(c1);

    v1->setUnits("a_unit");
    m->addUnits(u1);

    m->linkUnits();
    EXPECT_FALSE(m->hasUnlinkedUnits());
}

TEST(Units, linkUnitsViaUnitsObject)
{
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    libcellml::UnitsPtr u1 = libcellml::Units::create();
    libcellml::UnitsPtr u2 = libcellml::Units::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();

    u1->setName("a_unit");
    u2->setName("a_unit");

    v1->setUnits(u1);

    c1->addVariable(v1);
    m->addComponent(c1);
    m->addUnits(u2);

    m->linkUnits();
    EXPECT_FALSE(m->hasUnlinkedUnits());
}

TEST(Units, cannotLinkUnitsNotAddedToModel)
{
    libcellml::ModelPtr m = libcellml::Model::create();
    libcellml::ComponentPtr c1 = libcellml::Component::create();
    libcellml::UnitsPtr u1 = libcellml::Units::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create();

    u1->setName("a_unit");

    c1->addVariable(v1);
    m->addComponent(c1);

    v1->setUnits(u1);

    m->linkUnits();
    EXPECT_TRUE(m->hasUnlinkedUnits());
}

TEST(Units, linkingToUnitsInAnotherModelReturnsUnlinked)
{
    auto m1 = libcellml::Model::create("m1");
    auto m2 = libcellml::Model::create("m2");
    auto c1 = libcellml::Component::create("c1");
    auto u1 = libcellml::Units::create("u1");
    auto v1 = libcellml::Variable::create("v1");
    auto v2 = libcellml::Variable::create("v2");

    c1->addVariable(v1);
    c1->addVariable(v2);
    m1->addComponent(c1);
    v1->setUnits(u1);
    v2->setUnits("second");

    m2->addUnits(u1); // Units assigned to v1 exist in a different model.

    EXPECT_FALSE(m1->linkUnits());
    EXPECT_TRUE(m1->hasUnlinkedUnits());
}

TEST(Units, multiply)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <units name=\"compound_unit\">\n"
        "    <unit prefix=\"micro\" units=\"ampere\"/>\n"
        "    <unit units=\"kelvin\"/>\n"
        "    <unit exponent=\"-1\" prefix=\"milli\" units=\"siemens\"/>\n"
        "  </units>\n"
        "  <units name=\"valid_name\"/>\n"
        "  <units name=\"multiplied\">\n"
        "    <unit units=\"compound_unit\"/>\n"
        "    <unit units=\"valid_name\"/>\n"
        "  </units>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();

    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("compound_unit");

    u1->addUnit(libcellml::Units::StandardUnit::AMPERE, libcellml::Units::Prefix::MICRO);
    u1->addUnit(libcellml::Units::StandardUnit::KELVIN);
    u1->addUnit(libcellml::Units::StandardUnit::SIEMENS, libcellml::Units::Prefix::MILLI, -1.0);

    m->addUnits(u1);

    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("valid_name");

    m->addUnits(u2);

    libcellml::UnitsPtr u3 = libcellml::Units::create();
    u3->setName("multiplied");
    u3->addUnit("compound_unit");
    u3->addUnit("valid_name");

    m->addUnits(u3);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
}

TEST(Units, newBaseUnit)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <units name=\"pH\"/>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();

    libcellml::UnitsPtr u = libcellml::Units::create();
    u->setName("pH");

    m->addUnits(u);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
    EXPECT_EQ("pH", u->name());
}

TEST(Units, isBaseUnit)
{
    libcellml::UnitsPtr u = libcellml::Units::create();
    u->setName("pH");

    EXPECT_TRUE(u->isBaseUnit());

    u->addUnit("Candela");

    EXPECT_FALSE(u->isBaseUnit());
}

TEST(Units, farhenheit)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <units name=\"fahrenheitish\">\n"
        "    <unit multiplier=\"1.8\" units=\"kelvin\"/>\n"
        "  </units>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();

    libcellml::UnitsPtr u = libcellml::Units::create();
    u->setName("fahrenheitish");

    /* Give prefix and exponent their default values. */
    u->addUnit("kelvin", 0, 1.0, 1.8);
    m->addUnits(u);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    const std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);
    EXPECT_EQ("fahrenheitish", u->name());
}

TEST(Units, unitAttributes)
{
    libcellml::ModelPtr m = libcellml::Model::create();

    libcellml::UnitsPtr u = libcellml::Units::create();
    u->setName("fahrenheitish");

    /* Give prefix and exponent their default values. */
    u->addUnit("kelvin", 0, 1.0, 1.8);
    m->addUnits(u);

    std::string reference;
    std::string prefix;
    std::string id;
    double exponent;
    double multiplier;
    u->unitAttributes(0, reference, prefix, exponent, multiplier, id);
    EXPECT_EQ("kelvin", reference);
    EXPECT_EQ("", prefix);
    EXPECT_DOUBLE_EQ(1.0, exponent);
    EXPECT_DOUBLE_EQ(1.8, multiplier);

    u->addUnit("NewUnit", 4, 1.05, 17.0);
    u->unitAttributes(1, reference, prefix, exponent, multiplier, id);
    EXPECT_EQ("NewUnit", reference);
    EXPECT_EQ("4", prefix);
    EXPECT_DOUBLE_EQ(1.05, exponent);
    EXPECT_DOUBLE_EQ(17, multiplier);

    // Get non-existent unit.
    u->unitAttributes(2, reference, prefix, exponent, multiplier, id);
    EXPECT_EQ("", reference);
    EXPECT_EQ("", prefix);
    EXPECT_DOUBLE_EQ(1, exponent);
    EXPECT_DOUBLE_EQ(1, multiplier);

    u->addUnit("daves", "house");
    u->unitAttributes(2, reference, prefix, exponent, multiplier, id);
    EXPECT_EQ("daves", reference);
    EXPECT_EQ("house", prefix);

    u->unitAttributes("daves", prefix, exponent, multiplier, id);
    EXPECT_EQ("daves", reference);
    EXPECT_EQ("house", prefix);

    u->unitAttributes("kelvin", prefix, exponent, multiplier, id);
    EXPECT_EQ("", prefix);
    EXPECT_DOUBLE_EQ(1.0, exponent);
    EXPECT_DOUBLE_EQ(1.8, multiplier);
}

TEST(Units, unitAttributeReference)
{
    libcellml::UnitsPtr u = libcellml::Units::create();

    EXPECT_EQ("", u->unitAttributeReference(0));
    u->addUnit("NewUnit", 4, 1.05, 17.0);

    EXPECT_EQ("NewUnit", u->unitAttributeReference(0));
    EXPECT_EQ("", u->unitAttributeReference(4));

    u->setUnitAttributeReference(0, "NewerUnit");
    EXPECT_EQ("NewerUnit", u->unitAttributeReference(0));

    u->setUnitAttributeReference(3, "FreshUnit");
    EXPECT_EQ("", u->unitAttributeReference(3));
    EXPECT_EQ("NewerUnit", u->unitAttributeReference(0));
}

TEST(Units, unitAttributePrefix)
{
    libcellml::UnitsPtr u = libcellml::Units::create();

    EXPECT_EQ("", u->unitAttributePrefix(0));
    u->addUnit("NewUnit", 4, 1.05, 17.0);

    EXPECT_EQ("4", u->unitAttributePrefix(0));
    EXPECT_EQ("", u->unitAttributePrefix(3));
}

TEST(Units, unitAttributeExponent)
{
    libcellml::UnitsPtr u = libcellml::Units::create();

    EXPECT_EQ(1.0, u->unitAttributeExponent(0));
    u->addUnit("NewUnit", 4, 1.05, 17.0);

    EXPECT_EQ(1.05, u->unitAttributeExponent(0));
    EXPECT_EQ(1.0, u->unitAttributeExponent(2));
}

TEST(Units, unitAttributeMultiplier)
{
    libcellml::UnitsPtr u = libcellml::Units::create();

    EXPECT_EQ(1.0, u->unitAttributeMultiplier(0));
    u->addUnit("NewUnit", 4, 1.05, 17.0);

    EXPECT_EQ(17.0, u->unitAttributeMultiplier(0));
    EXPECT_EQ(1.0, u->unitAttributeMultiplier(5));
}

TEST(Units, multipleUnitUsingStandardRef)
{
    auto u = libcellml::Units::create();

    u->addUnit(libcellml::Units::StandardUnit::AMPERE, "micro");
    u->addUnit(libcellml::Units::StandardUnit::AMPERE, "milli");
    u->addUnit(libcellml::Units::StandardUnit::AMPERE, libcellml::Units::Prefix::CENTI);
    u->addUnit(libcellml::Units::StandardUnit::AMPERE, libcellml::Units::Prefix::MICRO);

    EXPECT_EQ(size_t(4), u->unitCount());

    u->removeUnit(libcellml::Units::StandardUnit::AMPERE);

    EXPECT_EQ(size_t(3), u->unitCount());

    std::string prefix;
    std::string reference;
    std::string id;
    double exponent;
    double multiplier;
    u->unitAttributes(libcellml::Units::StandardUnit::AMPERE, prefix, exponent, multiplier, id);
    EXPECT_EQ("milli", prefix);
    u->unitAttributes(0, reference, prefix, exponent, multiplier, id);
    EXPECT_EQ("milli", prefix);
    u->unitAttributes(1, reference, prefix, exponent, multiplier, id);
    EXPECT_EQ("centi", prefix);
    u->unitAttributes(2, reference, prefix, exponent, multiplier, id);
    EXPECT_EQ("micro", prefix);
}

TEST(Units, removeUnit)
{
    libcellml::UnitsPtr u = libcellml::Units::create();

    u->addUnit(libcellml::Units::StandardUnit::AMPERE, "micro");
    u->addUnit("kelvin");
    u->addUnit("siemens", "milli", -1.0);
    u->addUnit("metre", "1.7e10");

    EXPECT_EQ(size_t(4), u->unitCount());
    EXPECT_TRUE(u->removeUnit("siemens"));
    EXPECT_TRUE(u->removeUnit(libcellml::Units::StandardUnit::KELVIN));
    EXPECT_TRUE(u->removeUnit(1));
    EXPECT_EQ(size_t(1), u->unitCount());

    // Remove non-existent unit
    EXPECT_FALSE(u->removeUnit("gram"));
    EXPECT_FALSE(u->removeUnit(libcellml::Units::StandardUnit::BECQUEREL));
    EXPECT_FALSE(u->removeUnit(3));
    EXPECT_EQ(size_t(1), u->unitCount());

    u->removeAllUnits();
    EXPECT_EQ(size_t(0), u->unitCount());
}

TEST(Units, multipleAndParse)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <units name=\"fahrenheitish\">\n"
        "    <unit multiplier=\"1.8\" units=\"kelvin\"/>\n"
        "  </units>\n"
        "  <units name=\"metres_per_second\">\n"
        "    <unit units=\"metre\"/>\n"
        "    <unit exponent=\"-1\" units=\"second\"/>\n"
        "  </units>\n"
        "</model>\n";

    libcellml::ModelPtr m = libcellml::Model::create();

    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("fahrenheitish");

    /* Give prefix and exponent their default values. */
    u1->addUnit("kelvin", 0, 1.0, 1.8);

    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("metres_per_second");
    u2->addUnit(libcellml::Units::StandardUnit::METRE);
    u2->addUnit(libcellml::Units::StandardUnit::SECOND, -1.0);

    m->addUnits(u1);
    m->addUnits(u2);

    libcellml::PrinterPtr printer = libcellml::Printer::create();
    std::string a = printer->printModel(m);
    EXPECT_EQ(e, a);

    // Parse
    libcellml::ParserPtr parser = libcellml::Parser::create();
    libcellml::ModelPtr model = parser->parseModel(e);
    a = printer->printModel(model);
    EXPECT_EQ(e, a);
}

TEST(Units, unitsWithPrefixOutOfRange)
{
    // int limit is 2,147,483,647

    const std::string e = "Prefix '2147483648' of a unit referencing 'second' in units 'myUnits' is out of the integer range.";

    libcellml::ValidatorPtr validator = libcellml::Validator::create();
    libcellml::ModelPtr m = libcellml::Model::create();
    m->setName("myModel");
    libcellml::ComponentPtr c = libcellml::Component::create();
    c->setName("myComponent");
    libcellml::VariablePtr v = libcellml::Variable::create();
    v->setName("myVariable");
    libcellml::UnitsPtr u = libcellml::Units::create();

    u->setName("myUnits");
    u->addUnit("second", "2147483648");
    v->setUnits(u);
    c->addVariable(v);
    m->addComponent(c);
    m->addUnits(u);

    EXPECT_FALSE(m->hasUnlinkedUnits());

    validator->validateModel(m);

    EXPECT_EQ(size_t(1), validator->issueCount());
    EXPECT_EQ(e, validator->issue(0)->description());
}

TEST(Units, parentOfUnits)
{
    libcellml::ModelPtr model = libcellml::Model::create();
    model->setName("I_am_a_model");

    libcellml::UnitsPtr u = libcellml::Units::create();
    u->setName("u");

    model->addUnits(u);

    libcellml::ModelPtr parent = std::dynamic_pointer_cast<libcellml::Model>(u->parent());
    EXPECT_FALSE(parent == nullptr);
    EXPECT_EQ(model, parent);
}

TEST(Units, parentlessUsingRemoveUnits)
{
    libcellml::ModelPtr model = libcellml::Model::create();
    model->setName("I_am_a_model");

    libcellml::UnitsPtr u = libcellml::Units::create();
    u->setName("u");

    model->addUnits(u);
    EXPECT_TRUE(u->hasParent());

    model->removeUnits(0);
    EXPECT_FALSE(u->hasParent());

    model->addUnits(u);
    EXPECT_TRUE(u->hasParent());

    model->removeUnits("u");
    EXPECT_FALSE(u->hasParent());

    model->addUnits(u);
    EXPECT_TRUE(u->hasParent());

    model->removeUnits(u);
    EXPECT_FALSE(u->hasParent());
}

TEST(Units, compareEqualMultiplierSimple)
{
    // u1 = 1000*u2
    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("u1");
    u1->addUnit("metre", 0, 1.0, 113.0);
    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("u2");
    u2->addUnit("metre", 0, 1.0, 113.0);

    EXPECT_EQ(1.0, libcellml::Units::scalingFactor(u1, u2));
}

TEST(Units, compareMultiplierSimple)
{
    // u1 = 1000*u2
    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("u1");
    u1->addUnit("metre", 0, 1.0, 1000.0);
    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("u2");
    u2->addUnit("metre", 0, 1.0, 1.0);

    EXPECT_EQ(0.001, libcellml::Units::scalingFactor(u1, u2)); // 10^-3.0 factor difference
    EXPECT_EQ(1000.0, libcellml::Units::scalingFactor(u2, u1)); // 10^3.0 factor difference
}

TEST(Units, compareMultiplierStandardUnit)
{
    // u1 = 1000*u2
    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("u1");
    u1->addUnit(libcellml::Units::StandardUnit::BECQUEREL, 0, 1.0, 500.0);
    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("u2");
    u2->addUnit(libcellml::Units::StandardUnit::BECQUEREL, 0, 1.0, 1.0);

    EXPECT_NEAR(0.002, libcellml::Units::scalingFactor(u1, u2), 1e-12);
    EXPECT_NEAR(500.0, libcellml::Units::scalingFactor(u2, u1), 1e-12);
}

TEST(Units, compareSameBaseUnits)
{
    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("frufru");
    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("frufru");

    EXPECT_EQ(1.0, libcellml::Units::scalingFactor(u1, u2));
}

TEST(Units, compareBaseUnitsAgainstItself)
{
    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("frufru");

    EXPECT_EQ(1.0, libcellml::Units::scalingFactor(u1, u1));
}

TEST(Units, compareBaseUnitsWithCapitalisation)
{
    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("frufru");
    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("Frufru");

    EXPECT_EQ(0.0, libcellml::Units::scalingFactor(u1, u2));
}

TEST(Units, compareScalingFactorWithNullptrAsFirstParameter)
{
    libcellml::UnitsPtr u = libcellml::Units::create();
    u->setName("u");
    u->addUnit(libcellml::Units::StandardUnit::LUX, 0, 1.0, 1.0);

    EXPECT_EQ(0.0, libcellml::Units::scalingFactor(nullptr, u));
    EXPECT_EQ(0.0, libcellml::Units::scalingFactor(nullptr, u, false));
}

TEST(Units, compareScalingFactorWithNullptrAsSecondParameter)
{
    libcellml::UnitsPtr u = libcellml::Units::create();
    u->setName("u");
    u->addUnit(libcellml::Units::StandardUnit::LUX, 0, 1.0, 1.0);

    EXPECT_EQ(0.0, libcellml::Units::scalingFactor(u, nullptr));
    EXPECT_EQ(0.0, libcellml::Units::scalingFactor(u, nullptr, false));
}

TEST(Units, compareScalingFactorWithNullptrAsBothParameters)
{
    EXPECT_EQ(0.0, libcellml::Units::scalingFactor(nullptr, nullptr));
    EXPECT_EQ(0.0, libcellml::Units::scalingFactor(nullptr, nullptr, false));
}

TEST(Units, compareScalingFactorWithUnitBasedOnUnknownUnit)
{
    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("u1");
    u1->addUnit("apple", 0, 1.0, 1.0);

    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("u2");
    u2->addUnit(libcellml::Units::StandardUnit::LUX, 0, 1.0, 1.0);

    EXPECT_EQ(0.0, libcellml::Units::scalingFactor(u1, u2));
    EXPECT_EQ(0.0, libcellml::Units::scalingFactor(u2, u1));
}

TEST(Units, compareScalingFactorWithTwoUnitsBasedOnUnknownUnits)
{
    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("u1");
    u1->addUnit("apple", 0, 1.0, 1.0);

    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("u2");
    u2->addUnit("banana", 0, 1.0, 1.0);

    EXPECT_EQ(0.0, libcellml::Units::scalingFactor(u1, u2));
}

TEST(Units, scalingFactorWithOneEmptyUnit)
{
    // Add unit with no members
    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("u1");

    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("u2");
    u2->addUnit(libcellml::Units::StandardUnit::LITRE, 0, 1.0, 1.0);

    EXPECT_EQ(0.0, libcellml::Units::scalingFactor(u1, u2));
    EXPECT_EQ(0.0, libcellml::Units::scalingFactor(u2, u1));
}

TEST(Units, scalingFactorWithTwoEmptyUnits)
{
    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("u1");

    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("u2");

    EXPECT_EQ(0.0, libcellml::Units::scalingFactor(u2, u1));
}

TEST(Units, scalingFactorBetweenDissimilarUnits)
{
    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("u1");
    u1->addUnit("metre", 0, 1.0, 1000.0);
    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("u2");
    u2->addUnit("metre", 0, 1.0, 1.0);
    u2->addUnit("metre", 0, 1.0, 1.0);
    u2->addUnit("metre", 0, 1.0, 1.0);

    EXPECT_EQ(0.0, libcellml::Units::scalingFactor(u2, u1));
}

TEST(Units, compareIncompatibleMultiplierSimple)
{
    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("u1");
    u1->addUnit("metre", 0, 1.0, 1.0);
    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("u2");
    u2->addUnit("second", 0, 1.0, 1.0);

    // Incompatible units return a scaling factor of 0.0.
    EXPECT_EQ(0.0, libcellml::Units::scalingFactor(u1, u2));
    EXPECT_EQ(0.0, libcellml::Units::scalingFactor(u2, u1));
}

TEST(Units, dimensionlessScalingFactor)
{
    auto m = libcellml::Model::create("model");
    auto u = libcellml::Units::create("u");

    auto u1 = libcellml::Units::create("u1");
    u1->addUnit("u", "milli", 2.0, 1000.0);
    u1->addUnit("dimensionless");
    auto u2 = libcellml::Units::create("u2");
    u2->addUnit("u", "kilo", 2.0, 0.001);
    u2->addUnit("dimensionless");

    auto u3 = libcellml::Units::create("u3");
    u3->addUnit("u", "milli", 2.0, 1000.0);
    auto u4 = libcellml::Units::create("u4");
    u4->addUnit("u", "kilo", 2.0, 0.001);

    auto u5 = libcellml::Units::create("u5");
    u5->addUnit("u", "milli", 2.0, 1000.0);
    u5->addUnit("dimensionless", 2.0);
    auto u6 = libcellml::Units::create("u6");
    u6->addUnit("u", "kilo", 2.0, 0.001);
    u6->addUnit("dimensionless", 2.0);

    m->addUnits(u);
    m->addUnits(u1);
    m->addUnits(u2);
    m->addUnits(u3);
    m->addUnits(u4);
    m->addUnits(u5);
    m->addUnits(u6);

    EXPECT_EQ(1.0, libcellml::Units::scalingFactor(u1, u2));
    EXPECT_EQ(1.0, libcellml::Units::scalingFactor(u2, u1));
    EXPECT_EQ(1.0, libcellml::Units::scalingFactor(u3, u4));
    EXPECT_EQ(1.0, libcellml::Units::scalingFactor(u4, u3));
    EXPECT_EQ(1.0, libcellml::Units::scalingFactor(u1, u3));
    EXPECT_EQ(1.0, libcellml::Units::scalingFactor(u2, u4));
    EXPECT_EQ(1.0, libcellml::Units::scalingFactor(u1, u4));
    EXPECT_EQ(1.0, libcellml::Units::scalingFactor(u2, u3));
    EXPECT_EQ(1.0, libcellml::Units::scalingFactor(u5, u6));
    EXPECT_EQ(1.0, libcellml::Units::scalingFactor(u6, u5));
    EXPECT_EQ(1.0, libcellml::Units::scalingFactor(u1, u5));
}

TEST(Units, customUnitsScalingFactorSimple)
{
    auto m = libcellml::Model::create("model");
    auto u1 = libcellml::Units::create("u1");
    m->addUnits(u1);
    auto u2 = libcellml::Units::create("u2");
    u2->addUnit("u1");
    m->addUnits(u2);

    m->linkUnits();

    EXPECT_TRUE(libcellml::Units::compatible(u1, u2));
    EXPECT_EQ(1.0, libcellml::Units::scalingFactor(u1, u2));
    EXPECT_TRUE(libcellml::Units::equivalent(u1, u2));
}

TEST(Units, customUnitsScalingFactorIncludingDimensionless)
{
    auto m = libcellml::Model::create("model");
    auto u1 = libcellml::Units::create("u1");

    m->addUnits(u1);

    auto u2 = libcellml::Units::create("u2");
    u2->addUnit("u1");
    u2->addUnit("dimensionless");
    u2->addUnit("radian");
    u2->addUnit("steradian");
    u2->addUnit("metre", 1.0);
    u2->addUnit("metre", -1.0);

    m->addUnits(u2);

    m->linkUnits();

    EXPECT_TRUE(libcellml::Units::compatible(u1, u2));
    EXPECT_EQ(1.0, libcellml::Units::scalingFactor(u1, u2));
    EXPECT_TRUE(libcellml::Units::equivalent(u1, u2));
}

TEST(Units, complicatedMultiplicationFactorUnits)
{
    libcellml::ModelPtr model = libcellml::Model::create();

    libcellml::UnitsPtr u = libcellml::Units::create();
    u->setName("u");

    libcellml::UnitsPtr u1 = libcellml::Units::create("u1");
    u1->addUnit("u", "milli", 2.0, 1000.0); // u1 = u^2
    u1->addUnit("dimensionless");

    libcellml::UnitsPtr u2 = libcellml::Units::create("u2");
    u2->addUnit("u", "kilo", 2.0, 0.001); // u2 = u^2
    u2->addUnit("dimensionless");

    libcellml::UnitsPtr u3 = libcellml::Units::create("u3");
    u3->addUnit("u", "kilo", 4.0, 0.001); // u3 = u^4

    libcellml::UnitsPtr u4 = libcellml::Units::create("u4");
    u4->addUnit("u2", 2.0); // u4 = u^4

    libcellml::UnitsPtr apple = libcellml::Units::create("apple");
    libcellml::UnitsPtr banana = libcellml::Units::create("banana");

    libcellml::UnitsPtr bushell_of_apples = libcellml::Units::create("bushell_of_apples");
    bushell_of_apples->addUnit("apple", "mega", 1.0, 1000.0); // 1000*mega*apple^1

    libcellml::UnitsPtr square_apple = libcellml::Units::create("square_apple");
    square_apple->addUnit("apple", 2.0);

    libcellml::UnitsPtr incredible_pile_of_square_apples = libcellml::Units::create("incredible_pile_of_square_apples");
    incredible_pile_of_square_apples->addUnit("square_apple", "mega", 1.0, 100.0);

    libcellml::UnitsPtr bunch_of_bananas = libcellml::Units::create("bunch_of_bananas");
    bunch_of_bananas->addUnit("banana", 1, 1.0, 1.0); // 10 bananas

    model->setName("model");
    model->addUnits(u);
    model->addUnits(u1);
    model->addUnits(u2);
    model->addUnits(u3);
    model->addUnits(u4);
    model->addUnits(bushell_of_apples);
    model->addUnits(apple);
    model->addUnits(banana);
    model->addUnits(square_apple);
    model->addUnits(incredible_pile_of_square_apples);
    model->addUnits(bunch_of_bananas);

    EXPECT_EQ(1.0, libcellml::Units::scalingFactor(u1, u2));
    EXPECT_EQ(1.0, libcellml::Units::scalingFactor(u3, u4));
    EXPECT_EQ(1e-08, libcellml::Units::scalingFactor(incredible_pile_of_square_apples, square_apple));
    // Incompatible units so we return a scaling factor of 0.0.
    EXPECT_EQ(0.0, libcellml::Units::scalingFactor(incredible_pile_of_square_apples, bunch_of_bananas));
}

TEST(Units, checkingOwningModelBothUnits)
{
    libcellml::ModelPtr model = libcellml::Model::create();

    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("u1");
    u1->addUnit("u", "milli", 2.0, 1000.0); // m^2
    u1->addUnit("dimensionless");

    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("u2");
    u2->addUnit("u", "kilo", 2.0, 0.001); // standard, exponent.
    u2->addUnit("dimensionless");

    model->addUnits(u1);
    model->addUnits(u2);

    EXPECT_EQ(0.0, libcellml::Units::scalingFactor(u1, u2));
    EXPECT_EQ(0.0, libcellml::Units::scalingFactor(u2, u1));
}

TEST(Units, checkingOwningModelWithUnitBaseOnlyInModel)
{
    libcellml::ModelPtr model = libcellml::Model::create();

    libcellml::UnitsPtr u = libcellml::Units::create();
    u->setName("apple");

    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("u1");
    u1->addUnit("apple", "kilo", 2.0, 0.001); // standard, exponent.
    u1->addUnit("dimensionless");

    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("u2");
    u2->addUnit("apple", "kilo", 4.0, 0.001); // standard, exponent.

    model->addUnits(u);
    model->addUnits(u1);

    EXPECT_EQ(0.0, libcellml::Units::scalingFactor(u1, u2));
    EXPECT_EQ(0.0, libcellml::Units::scalingFactor(u2, u1));
}

TEST(Units, checkingOwningModelOneUnit)
{
    libcellml::ModelPtr model = libcellml::Model::create();

    libcellml::UnitsPtr u = libcellml::Units::create();
    u->setName("u");

    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("u1");
    u1->addUnit("u", "milli", 1.0, 1.0); // u1 is a parent of u

    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("u2");
    u2->addUnit("apples", "kilo", 2.0, 1000.0); // apples not in model

    model->addUnits(u);
    model->addUnits(u1);
    model->addUnits(u2);

    EXPECT_EQ(0.0, libcellml::Units::scalingFactor(u1, u2));
    EXPECT_EQ(0.0, libcellml::Units::scalingFactor(u2, u1));
}

TEST(Units, checkScalingFactorImportedModel)
{
    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("u1");
    u1->addUnit(libcellml::Units::StandardUnit::RADIAN, 0, 1.0, 1.0);

    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("u2");
    u2->addUnit(libcellml::Units::StandardUnit::RADIAN, 0, 1.0, 1.0);

    libcellml::ModelPtr model = libcellml::Model::create();
    model->setName("model");
    model->addUnits(u1);

    libcellml::ImportSourcePtr import = libcellml::ImportSource::create();
    import->setUrl("I_am_a_url");
    import->setModel(model);

    model->addUnits(u1);
    model->addUnits(u2);

    EXPECT_EQ(1.0, libcellml::Units::scalingFactor(u1, u2));
    EXPECT_EQ(1.0, libcellml::Units::scalingFactor(u2, u1));
}

TEST(Units, checkScalingFactorOneUnitImported)
{
    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("u1");
    u1->addUnit(libcellml::Units::StandardUnit::RADIAN, 0, 1.0, 1.0);

    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("u2");
    u2->addUnit(libcellml::Units::StandardUnit::RADIAN, 0, 1.0, 1.0);

    libcellml::ImportSourcePtr import = libcellml::ImportSource::create();
    import->setUrl("I_am_a_url");

    u1->setImportSource(import);

    // Scaling factor returns 0.0 because of imported units.
    EXPECT_EQ(0.0, libcellml::Units::scalingFactor(u1, u2));
    EXPECT_EQ(0.0, libcellml::Units::scalingFactor(u2, u1));
}

TEST(Units, checkScalingFactorOneNonBaseUnitImported)
{
    libcellml::ModelPtr model = libcellml::Model::create();
    model->setName("model");

    libcellml::UnitsPtr u = libcellml::Units::create();
    u->setName("u");
    u->addUnit("apples", 0, 1.0, 1.0);

    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("u1");
    u1->addUnit(libcellml::Units::StandardUnit::RADIAN, 0, 1.0, 1.0);

    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("u2");
    u2->addUnit(libcellml::Units::StandardUnit::RADIAN, 0, 1.0, 1.0);
    u2->addUnit("u", 0, 1.0, 1.0);

    model->addUnits(u);
    model->addUnits(u1);
    model->addUnits(u2);

    libcellml::ImportSourcePtr import = libcellml::ImportSource::create();
    import->setUrl("I_am_a_url");

    u->setImportSource(import);

    EXPECT_TRUE(u->isImport());
    EXPECT_EQ(0.0, libcellml::Units::scalingFactor(u1, u2));
    EXPECT_EQ(0.0, libcellml::Units::scalingFactor(u2, u1));
}

TEST(Units, checkScalingFactorBothNonBaseUnitsImported)
{
    libcellml::ModelPtr model = libcellml::Model::create();
    model->setName("model");

    libcellml::UnitsPtr u = libcellml::Units::create();
    u->setName("u");
    u->addUnit("apples", 0, 1.0, 1.0);

    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("u1");
    u1->addUnit(libcellml::Units::StandardUnit::AMPERE, 0, 1.0, 1.0);
    u1->addUnit("u", 0, 1.0, 1.0);

    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("u2");
    u2->addUnit(libcellml::Units::StandardUnit::RADIAN, 0, 1.0, 1.0);
    u2->addUnit("u", 0, 1.0, 1.0);

    model->addUnits(u);
    model->addUnits(u1);
    model->addUnits(u2);

    libcellml::ImportSourcePtr import = libcellml::ImportSource::create();
    import->setUrl("I_am_a_url");

    u->setImportSource(import);

    EXPECT_TRUE(u->isImport());
    EXPECT_EQ(0.0, libcellml::Units::scalingFactor(u1, u2));
    EXPECT_EQ(0.0, libcellml::Units::scalingFactor(u2, u1));
}

TEST(Units, compareEquivalentUnits)
{
    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("u1");
    u1->addUnit(libcellml::Units::StandardUnit::LUX, 0, 1.0, 1.0);

    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("u2");
    u2->addUnit(libcellml::Units::StandardUnit::LUX, 0, 1.0, 1.0);

    EXPECT_TRUE(libcellml::Units::equivalent(u1, u2));
}

TEST(Units, compareDimensionallyNonEquivalentUnitsWhichHaveSameBase)
{
    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("u1");
    u1->addUnit(libcellml::Units::StandardUnit::LUX, "milli");

    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("u2");
    u2->addUnit(libcellml::Units::StandardUnit::LUX, 0, 1.0, 1.0);

    EXPECT_FALSE(libcellml::Units::equivalent(u1, u2));
    EXPECT_FALSE(libcellml::Units::equivalent(u2, u1));
}

TEST(Units, compareEquivalentUnitsWhichHaveNonStandardBase)
{
    libcellml::ModelPtr model = libcellml::Model::create();
    model->setName("model");

    libcellml::UnitsPtr u = libcellml::Units::create();
    u->setName("u");

    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("u1");
    u1->addUnit("u");

    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("u2");
    u2->addUnit("u");

    model->addUnits(u);
    model->addUnits(u1);
    model->addUnits(u2);

    EXPECT_TRUE(libcellml::Units::equivalent(u1, u2));
    EXPECT_TRUE(libcellml::Units::equivalent(u2, u1));
}

TEST(Units, compareNonEquivalentUnitsWhichHaveDifferentBase)
{
    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("u1");
    u1->addUnit(libcellml::Units::StandardUnit::BECQUEREL, 0, 1.0, 1.0);

    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("u2");
    u2->addUnit(libcellml::Units::StandardUnit::LUX, 0, 1.0, 1.0);

    EXPECT_FALSE(libcellml::Units::equivalent(u1, u2));
}

TEST(Units, compareEquivalentUnitsComplex)
{
    libcellml::UnitsPtr u = libcellml::Units::create();
    u->setName("u");
    u->addUnit(libcellml::Units::StandardUnit::CANDELA, "micro", 2.0, 3.0);

    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("u1");
    u1->addUnit("metre", "milli", 2.0, 1.0);
    u1->addUnit(libcellml::Units::StandardUnit::AMPERE, 0, 1.0, 1.0);
    u1->addUnit("u");

    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("u2");
    u2->addUnit(libcellml::Units::StandardUnit::AMPERE, 0, 1.0, 1.0);
    u2->addUnit("u");
    u2->addUnit("metre", "milli", 2.0, 1.0); // Ordering shouldn't matter

    libcellml::ModelPtr model = libcellml::Model::create();
    model->setName("model");
    model->addUnits(u);
    model->addUnits(u1);
    model->addUnits(u2);

    EXPECT_TRUE(libcellml::Units::equivalent(u1, u2));
    EXPECT_TRUE(libcellml::Units::equivalent(u2, u1));
}

TEST(Units, compareIncomparableEquivalentUnitsComplex)
{
    libcellml::UnitsPtr u = libcellml::Units::create();
    u->setName("u");
    u->addUnit(libcellml::Units::StandardUnit::CANDELA, "micro", 2.0, 3.0);

    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("u1");
    u1->addUnit("metre", "milli", 2.0, 1.0);
    u1->addUnit(libcellml::Units::StandardUnit::AMPERE, 0, 1.0, 1.0);
    u1->addUnit("u");

    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("u2");
    u2->addUnit(libcellml::Units::StandardUnit::AMPERE, 0, 1.0, 1.0);
    u2->addUnit("metre", "milli", 2.0, 1.0);

    libcellml::ModelPtr model = libcellml::Model::create();
    model->setName("model");
    model->addUnits(u);
    model->addUnits(u1);
    model->addUnits(u2);

    EXPECT_FALSE(libcellml::Units::equivalent(u1, u2));
    EXPECT_FALSE(libcellml::Units::equivalent(u2, u1));
}

TEST(Units, compareBaseIncomparableUnitsWithInputAsNullptr)
{
    libcellml::UnitsPtr u = libcellml::Units::create();
    u->setName("u");
    u->addUnit(libcellml::Units::StandardUnit::LUX, "milli");

    EXPECT_FALSE(libcellml::Units::compatible(u, nullptr));
    EXPECT_FALSE(libcellml::Units::compatible(nullptr, u));
}

TEST(Units, compareBaseIncomparableUnitsWithBothInputsAsNullptr)
{
    EXPECT_FALSE(libcellml::Units::compatible(nullptr, nullptr));
}

TEST(Units, compareCompatibleUnits)
{
    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("u1");
    u1->addUnit(libcellml::Units::StandardUnit::LUX, "milli");

    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("u2");
    u2->addUnit(libcellml::Units::StandardUnit::LUX, 0, 1.0, 1.0);

    EXPECT_TRUE(libcellml::Units::compatible(u1, u2));
}

TEST(Units, compareEquivalentNonStandardWhichCannotBeResolvedUnits)
{
    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("a");
    u1->addUnit("millisecond", -1);

    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("b");
    u2->addUnit("millisecond", -1);

    EXPECT_FALSE(libcellml::Units::equivalent(u1, u2));
}

TEST(Units, compareEquivalentNonStandardUnitsOneUnitsWithParentModel)
{
    libcellml::ModelPtr m = libcellml::Model::create();

    libcellml::UnitsPtr u = libcellml::Units::create();
    u->setName("millisecond");
    u->addUnit("second", "milli");

    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("per_millisecond");
    u1->addUnit("millisecond", -1);

    m->addUnits(u);
    m->addUnits(u1);

    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("per_millisecond");
    u2->addUnit("millisecond", -1);

    // Cannot resolve reference to millisecond in u2 so
    // the two units are not considered equivalent.
    EXPECT_FALSE(libcellml::Units::equivalent(u1, u2));
}

TEST(Units, compareNonCompatibleUnits)
{
    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("u1");
    u1->addUnit(libcellml::Units::StandardUnit::WEBER, "milli");

    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("u2");
    u2->addUnit(libcellml::Units::StandardUnit::LUX, 0, 1.0, 1.0);

    EXPECT_FALSE(libcellml::Units::compatible(u1, u2));
}

TEST(Units, compareOneUnitWithNullptr)
{
    libcellml::UnitsPtr u = libcellml::Units::create();
    u->setName("u");
    u->addUnit(libcellml::Units::StandardUnit::LUX, "milli");

    EXPECT_FALSE(libcellml::Units::compatible(u, nullptr));
    EXPECT_FALSE(libcellml::Units::compatible(nullptr, u));
}

TEST(Units, compareBothInputsNullptr)
{
    EXPECT_FALSE(libcellml::Units::compatible(nullptr, nullptr));
}

TEST(Units, compareUnitsCompatibleComplex)
{
    libcellml::UnitsPtr u = libcellml::Units::create();
    u->setName("u");
    u->addUnit(libcellml::Units::StandardUnit::CANDELA, "micro", 2.0, 3.0);

    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("u1");
    u1->addUnit("metre", "milli", 2.0, 1.0);
    u1->addUnit(libcellml::Units::StandardUnit::AMPERE, "milli", 1.0, 1.0);
    u1->addUnit("u");

    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("u2");
    u2->addUnit(libcellml::Units::StandardUnit::AMPERE, 0, 1.0, 3.0);
    u2->addUnit("u");
    u2->addUnit("metre", "milli", 2.0, 1.0); // Only unit attributes which matter now are the exponent and the base unit. Not interested in multipliers.

    libcellml::ModelPtr model = libcellml::Model::create();
    model->setName("model");
    model->addUnits(u);
    model->addUnits(u1);
    model->addUnits(u2);

    EXPECT_TRUE(libcellml::Units::compatible(u1, u2));
    EXPECT_TRUE(libcellml::Units::compatible(u2, u1));
}

TEST(Units, compareUnitsNotCompatibleComplex)
{
    libcellml::UnitsPtr u = libcellml::Units::create();
    u->setName("u");
    u->addUnit(libcellml::Units::StandardUnit::CANDELA, "micro", 2.0, 3.0);

    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("u1");
    u1->addUnit("metre", "milli", 2.0, 1.0);
    u1->addUnit(libcellml::Units::StandardUnit::AMPERE, 0, 1.0, 1.0);
    u1->addUnit("u");

    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("u2");
    u2->addUnit(libcellml::Units::StandardUnit::WEBER, 0, 1.0, 1.0);
    u2->addUnit("metre", "milli", 2.0, 1.0);

    libcellml::ModelPtr model = libcellml::Model::create();
    model->setName("model");
    model->addUnits(u);
    model->addUnits(u1);
    model->addUnits(u2);

    EXPECT_FALSE(libcellml::Units::compatible(u1, u2));
    EXPECT_FALSE(libcellml::Units::compatible(u2, u1));
}

TEST(Units, compareUnitsEquivalentSameBaseTwice)
{
    libcellml::UnitsPtr u = libcellml::Units::create();
    u->setName("u");
    u->addUnit(libcellml::Units::StandardUnit::CANDELA, "micro", 2.0, 3.0);

    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("u1");
    u1->addUnit("u");
    u1->addUnit("u");

    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("u2");
    u2->addUnit("u");
    u2->addUnit("u");

    libcellml::ModelPtr model = libcellml::Model::create();
    model->setName("model");
    model->addUnits(u);
    model->addUnits(u1);
    model->addUnits(u2);

    EXPECT_TRUE(libcellml::Units::compatible(u1, u2));
    EXPECT_TRUE(libcellml::Units::compatible(u2, u1));
}

TEST(Units, compareStandardUnitsAsCompatible)
{
    libcellml::UnitsPtr u = libcellml::Units::create();
    u->setName("u");

    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("u1");
    u1->addUnit("u");
    u1->addUnit("u");

    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("u2");
    u2->addUnit("u");
    u2->addUnit("u");

    libcellml::ModelPtr model = libcellml::Model::create();
    model->addUnits(u);
    model->addUnits(u1);
    model->addUnits(u2);

    EXPECT_TRUE(libcellml::Units::compatible(u1, u2));
    EXPECT_TRUE(libcellml::Units::compatible(u2, u1));
}

TEST(Units, isCompatibleOneUnitImported)
{
    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("u1");
    u1->addUnit(libcellml::Units::StandardUnit::RADIAN, 0, 1.0, 1.0);

    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("u2");
    u2->addUnit(libcellml::Units::StandardUnit::RADIAN, 0, 1.0, 1.0);

    libcellml::ImportSourcePtr import = libcellml::ImportSource::create();
    import->setUrl("I_am_a_url");

    u1->setImportSource(import);

    EXPECT_TRUE(u1->isImport());
    EXPECT_FALSE(libcellml::Units::compatible(u1, u2));
    EXPECT_FALSE(libcellml::Units::compatible(u2, u1));
}

TEST(Units, isCompatibleBothUnitsImported)
{
    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("u1");
    u1->addUnit(libcellml::Units::StandardUnit::RADIAN, 0, 1.0, 1.0);

    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("u2");
    u2->addUnit(libcellml::Units::StandardUnit::RADIAN, 0, 1.0, 1.0);

    libcellml::ImportSourcePtr import = libcellml::ImportSource::create();
    import->setUrl("I_am_a_url");

    u1->setImportSource(import);
    u2->setImportSource(import);

    EXPECT_TRUE(u1->isImport());
    EXPECT_FALSE(libcellml::Units::compatible(u1, u2));
    EXPECT_FALSE(libcellml::Units::compatible(u2, u1));
}

TEST(Units, isCompatibleBaseUnitImported)
{
    libcellml::ModelPtr model = libcellml::Model::create();
    model->setName("model");

    libcellml::UnitsPtr u = libcellml::Units::create();
    u->setName("u");
    u->addUnit("apples", 0, 1.0, 1.0);

    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("u1");
    u1->addUnit(libcellml::Units::StandardUnit::RADIAN, 0, 1.0, 1.0);

    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("u2");
    u2->addUnit(libcellml::Units::StandardUnit::RADIAN, 0, 1.0, 1.0);
    u2->addUnit("u", 0, 1.0, 1.0);

    model->addUnits(u);
    model->addUnits(u1);
    model->addUnits(u2);

    libcellml::ImportSourcePtr import = libcellml::ImportSource::create();
    import->setUrl("I_am_a_url");

    u->setImportSource(import);

    EXPECT_TRUE(u->isImport());
    EXPECT_FALSE(libcellml::Units::compatible(u1, u2));
    EXPECT_FALSE(libcellml::Units::compatible(u2, u1));
}

TEST(Units, isCompatibleBaseGrandchildUnitImported)
{
    libcellml::ModelPtr model = libcellml::Model::create();
    model->setName("model");

    libcellml::UnitsPtr u = libcellml::Units::create("u");

    libcellml::UnitsPtr u3 = libcellml::Units::create();
    u3->setName("u3");
    u3->addUnit("u", 0, 1.0, 1.0);

    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("u1");
    u1->addUnit(libcellml::Units::StandardUnit::RADIAN, 0, 1.0, 1.0);

    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("u2");
    u2->addUnit(libcellml::Units::StandardUnit::RADIAN, 0, 1.0, 1.0);
    u2->addUnit("u3", 0, 1.0, 1.0);

    model->addUnits(u);
    model->addUnits(u1);
    model->addUnits(u2);
    model->addUnits(u3);

    libcellml::ImportSourcePtr import = libcellml::ImportSource::create();
    import->setUrl("I_am_a_url");

    u->setImportSource(import);

    EXPECT_TRUE(u->isImport());
    EXPECT_FALSE(libcellml::Units::compatible(u1, u2));
    EXPECT_FALSE(libcellml::Units::compatible(u2, u1));
}

TEST(Units, isEquivalentBaseUnitNotInModel)
{
    libcellml::ModelPtr model = libcellml::Model::create();
    model->setName("model");

    libcellml::UnitsPtr u = libcellml::Units::create();
    u->setName("u");
    u->addUnit("apples", 0, 1.0, 1.0);

    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("u1");
    u1->addUnit(libcellml::Units::StandardUnit::RADIAN, 0, 1.0, 1.0);

    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("u2");
    u2->addUnit(libcellml::Units::StandardUnit::RADIAN, 0, 1.0, 1.0);
    u2->addUnit("u", 0, 1.0, 1.0);
    // u1 compatible to u2 test should fail because base unit "u" is not present in u1, regardless of whether "apples" exists or not

    model->addUnits(u1);
    model->addUnits(u2);

    // Units "u" not in model.  Units "apples" doesn't exist.  Units are not linked.
    EXPECT_FALSE(libcellml::Units::compatible(u1, u2));
    EXPECT_FALSE(libcellml::Units::compatible(u2, u1));
}

TEST(Units, compareCompatibleSameSizeButDifferentUnits)
{
    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("u1");

    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("u2");

    EXPECT_FALSE(libcellml::Units::compatible(u1, u2));
    EXPECT_FALSE(libcellml::Units::compatible(u2, u1));
}

TEST(Units, compareCompatibleSameSizeButDifferentExponent)
{
    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("u1");
    u1->addUnit(libcellml::Units::StandardUnit::AMPERE, 0, 1.0, 1.0);

    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("u2");
    u2->addUnit(libcellml::Units::StandardUnit::AMPERE, 0, 2.0, 1.0);

    EXPECT_FALSE(libcellml::Units::compatible(u1, u2));
    EXPECT_FALSE(libcellml::Units::compatible(u2, u1));
}

TEST(Units, compareCompatibleUnitsWhichAreDimensionless)
{
    libcellml::ModelPtr model = libcellml::Model::create();

    // u1 = u2 = u3: testing that cancelled units become dimensionless and equivalent to radians, steradians, etc.
    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("dimensionless");
    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("testunit2");
    u2->addUnit("metre", -2.0);
    u2->addUnit("metre", 2.0);
    libcellml::UnitsPtr u3 = libcellml::Units::create();
    u3->setName("testunit3");
    u3->addUnit("steradian");
    libcellml::UnitsPtr u4 = libcellml::Units::create();
    u4->setName("testunit4");
    u4->addUnit("metre", 1.0);
    u4->addUnit("metre", -1.0);
    libcellml::UnitsPtr u5 = libcellml::Units::create();
    u5->setName("testunit5");
    u5->addUnit("radian");

    model->addUnits(u1);
    model->addUnits(u2);
    model->addUnits(u3);

    EXPECT_TRUE(libcellml::Units::compatible(u1, u2));
    EXPECT_TRUE(libcellml::Units::compatible(u1, u3));
    EXPECT_TRUE(libcellml::Units::compatible(u1, u4));
    EXPECT_TRUE(libcellml::Units::compatible(u1, u5));
}

TEST(Units, compareCompatibleUnitsWhichAreNested)
{
    libcellml::ModelPtr model = libcellml::Model::create();

    // millimetres
    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("u1");
    u1->addUnit("metre", "milli"); // standard, prefix.

    // mm^3
    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("u2");
    u2->addUnit("u1", 3.0); // standard, exponent.

    // mm^6
    libcellml::UnitsPtr u3 = libcellml::Units::create();
    u3->setName("u3");
    u3->addUnit("u2", 2.0); // standard, exponent.

    // m^6
    libcellml::UnitsPtr u4 = libcellml::Units::create();
    u4->setName("u4");
    u4->addUnit("u3", 15, 1.0); // standard, prefix, exponent.

    libcellml::UnitsPtr u5 = libcellml::Units::create();
    u5->setName("u5");
    u5->addUnit("metre", 6.0); // standard, exponent.

    model->setName("model");
    model->addUnits(u1);
    model->addUnits(u2);
    model->addUnits(u3);
    model->addUnits(u4);
    model->addUnits(u5);

    EXPECT_TRUE(libcellml::Units::compatible(u4, u5));
}

TEST(Units, isBaseUnitsImported)
{
    libcellml::ModelPtr model = libcellml::Model::create();

    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("u");

    model->addUnits(u1);

    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("some_u");

    libcellml::ImportSourcePtr importSource = libcellml::ImportSource::create();
    importSource->setUrl("I_am_a_url");
    importSource->setModel(model);

    u2->setImportSource(importSource);
    u2->setImportReference("u");

    EXPECT_TRUE(u2->isImport());
    EXPECT_TRUE(u2->isBaseUnit());
}

TEST(Units, isNotBaseUnitsImported)
{
    libcellml::ModelPtr model = libcellml::Model::create();
    model->setName("model");

    libcellml::UnitsPtr u1 = libcellml::Units::create();
    libcellml::UnitsPtr u2 = libcellml::Units::create();

    u1->setName("u1");
    u1->addUnit(libcellml::Units::StandardUnit::AMPERE, 0, 1.0, 1.0);
    u2->setName("some_u");

    model->addUnits(u1);

    libcellml::ImportSourcePtr import = libcellml::ImportSource::create();
    import->setUrl("I_am_a_url");
    import->setModel(model);

    u2->setImportSource(import);
    u2->setImportReference("u1");

    EXPECT_TRUE(u2->isImport());
    EXPECT_FALSE(u2->isBaseUnit());
}

TEST(Units, isBaseUnitsNoImport)
{
    libcellml::UnitsPtr u = libcellml::Units::create();

    u->setName("some_u"); // Base unit

    EXPECT_FALSE(u->isImport());
    EXPECT_TRUE(u->isBaseUnit());
}

TEST(Units, isBaseUnitSecond)
{
    libcellml::UnitsPtr u = libcellml::Units::create();

    u->setName("second");

    EXPECT_TRUE(u->isBaseUnit());
}

TEST(Units, isBaseUnitDimensionless)
{
    libcellml::UnitsPtr u = libcellml::Units::create();

    u->setName("dimensionless");

    EXPECT_TRUE(u->isBaseUnit());
}

TEST(Units, isBaseUnitSecondRedefined)
{
    libcellml::UnitsPtr u = libcellml::Units::create();

    u->setName("second");
    u->addUnit("volt");

    EXPECT_FALSE(u->isBaseUnit());
}

TEST(Units, isBaseUnitSecondRedefinedPrinted)
{
    const std::string e =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"model\">\n"
        "  <units name=\"second\">\n"
        "    <unit units=\"volt\"/>\n"
        "  </units>\n"
        "</model>\n";

    libcellml::PrinterPtr p = libcellml::Printer::create();
    libcellml::ModelPtr m = libcellml::Model::create("model");
    libcellml::UnitsPtr u = libcellml::Units::create();

    u->setName("second");
    u->addUnit("volt");

    m->addUnits(u);

    EXPECT_FALSE(u->isBaseUnit());

    EXPECT_EQ(e, p->printModel(m));
}

TEST(Units, isBaseUnitVolt)
{
    libcellml::UnitsPtr u = libcellml::Units::create();

    u->setName("volt");

    EXPECT_FALSE(u->isBaseUnit());
}

TEST(Units, isBaseUnitAllStandardUnits)
{
    const std::vector<libcellml::Units::StandardUnit> standardUnits = {
        libcellml::Units::StandardUnit::AMPERE,
        libcellml::Units::StandardUnit::BECQUEREL,
        libcellml::Units::StandardUnit::CANDELA,
        libcellml::Units::StandardUnit::COULOMB,
        libcellml::Units::StandardUnit::DIMENSIONLESS,
        libcellml::Units::StandardUnit::FARAD,
        libcellml::Units::StandardUnit::GRAM,
        libcellml::Units::StandardUnit::GRAY,
        libcellml::Units::StandardUnit::HENRY,
        libcellml::Units::StandardUnit::HERTZ,
        libcellml::Units::StandardUnit::JOULE,
        libcellml::Units::StandardUnit::KATAL,
        libcellml::Units::StandardUnit::KELVIN,
        libcellml::Units::StandardUnit::KILOGRAM,
        libcellml::Units::StandardUnit::LITRE,
        libcellml::Units::StandardUnit::LUMEN,
        libcellml::Units::StandardUnit::LUX,
        libcellml::Units::StandardUnit::METRE,
        libcellml::Units::StandardUnit::MOLE,
        libcellml::Units::StandardUnit::NEWTON,
        libcellml::Units::StandardUnit::OHM,
        libcellml::Units::StandardUnit::PASCAL,
        libcellml::Units::StandardUnit::RADIAN,
        libcellml::Units::StandardUnit::SECOND,
        libcellml::Units::StandardUnit::SIEMENS,
        libcellml::Units::StandardUnit::SIEVERT,
        libcellml::Units::StandardUnit::STERADIAN,
        libcellml::Units::StandardUnit::TESLA,
        libcellml::Units::StandardUnit::VOLT,
        libcellml::Units::StandardUnit::WATT,
        libcellml::Units::StandardUnit::WEBER,
    };

    const std::vector<size_t> baseUnitIndices = {0, 2, 4, 12, 13, 17, 18, 23};

    EXPECT_EQ(size_t(31), standardUnits.size());
    EXPECT_EQ(size_t(8), baseUnitIndices.size());

    libcellml::UnitsPtr u = libcellml::Units::create();
    std::string ref;
    std::string pre;
    std::string id;
    double expMult;
    double uExp;

    size_t index = 0;
    for (auto standardUnit : standardUnits) {
        u->addUnit(standardUnit);
        u->unitAttributes(0, ref, pre, uExp, expMult, id);

        libcellml::UnitsPtr testUnit = libcellml::Units::create(ref);
        auto found = std::find(baseUnitIndices.begin(), baseUnitIndices.end(), index);
        if (found != baseUnitIndices.end()) {
            EXPECT_TRUE(testUnit->isBaseUnit());
        } else {
            EXPECT_FALSE(testUnit->isBaseUnit());
        }

        u->removeUnit(standardUnit);
        index++;
    }
}

TEST(Units, isBaseUnitsImportModelUnresolved)
{
    libcellml::UnitsPtr u1 = libcellml::Units::create();
    libcellml::UnitsPtr u2 = libcellml::Units::create();

    u1->setName("u1");
    u2->setName("some_u");

    libcellml::ImportSourcePtr import = libcellml::ImportSource::create();
    import->setUrl("I_am_a_url");

    u2->setImportSource(import);
    u2->setImportReference("u1");

    EXPECT_TRUE(u2->isImport());
    EXPECT_EQ(u2->importSource()->model(), nullptr);
    EXPECT_FALSE(u2->isBaseUnit());
}

TEST(Units, isBaseUnitImportReferenceUnresolved)
{
    libcellml::ModelPtr model = libcellml::Model::create();
    model->setName("model");

    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("u");
    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("some_other_u");

    libcellml::ImportSourcePtr import = libcellml::ImportSource::create();
    import->setUrl("I_am_a_url");

    model->addUnits(u1);

    u2->setImportSource(import);
    import->setModel(model);

    EXPECT_TRUE(u2->isImport());
    EXPECT_TRUE(u2->importReference().empty());
    EXPECT_FALSE(u2->isBaseUnit());
}

TEST(Units, isBaseUnitImportedTwice)
{
    libcellml::ModelPtr m_i1 = libcellml::Model::create();
    m_i1->setName("m_i1");
    libcellml::ModelPtr m_i2 = libcellml::Model::create();
    m_i2->setName("m_i2");

    libcellml::UnitsPtr u_i1 = libcellml::Units::create();
    libcellml::UnitsPtr u_i2 = libcellml::Units::create();
    libcellml::UnitsPtr u_i0 = libcellml::Units::create();

    u_i0->setName("some_other_u");
    u_i1->setName("u_i1");
    u_i2->setName("u_i2");

    m_i1->addUnits(u_i1);
    m_i2->addUnits(u_i2);

    libcellml::ImportSourcePtr i2 = libcellml::ImportSource::create();
    i2->setUrl("I_am_a_url");
    i2->setModel(m_i2);
    u_i1->setImportSource(i2);
    u_i1->setImportReference("u_i2");

    libcellml::ImportSourcePtr i1 = libcellml::ImportSource::create();
    i1->setUrl("I_am_another_url");
    i1->setModel(m_i1);
    u_i0->setImportSource(i1);
    u_i0->setImportReference("u_i1");

    EXPECT_TRUE(u_i0->isImport());
    EXPECT_TRUE(u_i1->isImport());
    EXPECT_FALSE(u_i2->isImport());
    EXPECT_TRUE(u_i0->isBaseUnit());
}

TEST(Units, isNotBaseUnitImportedTwice)
{
    libcellml::ModelPtr m_i1 = libcellml::Model::create();
    m_i1->setName("m_i1");
    libcellml::ModelPtr m_i2 = libcellml::Model::create();
    m_i2->setName("m_i2");

    libcellml::UnitsPtr u_i1 = libcellml::Units::create();
    libcellml::UnitsPtr u_i2 = libcellml::Units::create();
    libcellml::UnitsPtr u_i0 = libcellml::Units::create();

    u_i2->setName("u_i2");
    u_i2->addUnit(libcellml::Units::StandardUnit::AMPERE, 0, 1.0, 1.0);
    u_i1->setName("u_i1");
    u_i0->setName("some_other_u");

    m_i1->addUnits(u_i1);
    m_i2->addUnits(u_i2);

    libcellml::ImportSourcePtr i2 = libcellml::ImportSource::create();
    i2->setUrl("I_am_a_url");
    i2->setModel(m_i2);
    u_i1->setImportSource(i2);
    u_i1->setImportReference("u_i2");

    libcellml::ImportSourcePtr i1 = libcellml::ImportSource::create();
    i1->setUrl("I_am_another_url");
    i1->setModel(m_i1);
    u_i0->setImportSource(i1);
    u_i0->setImportReference("u_i1");

    EXPECT_TRUE(u_i0->isImport());
    EXPECT_TRUE(u_i1->isImport());
    EXPECT_FALSE(u_i2->isImport());
    EXPECT_FALSE(u_i0->isBaseUnit());
}

TEST(Units, scalingFactorAcrossBaseUnits)
{
    libcellml::UnitsPtr u1 = libcellml::Units::create("second");
    libcellml::UnitsPtr u2 = libcellml::Units::create("millisecond");
    u2->addUnit(libcellml::Units::StandardUnit::SECOND, libcellml::Units::Prefix::MILLI);

    EXPECT_EQ(0.001, libcellml::Units::scalingFactor(u1, u2));
    EXPECT_EQ(1000.0, libcellml::Units::scalingFactor(u2, u1));
}

TEST(Units, scalingFactorAcrossStandardUnits)
{
    libcellml::UnitsPtr u1 = libcellml::Units::create("volt");
    libcellml::UnitsPtr u2 = libcellml::Units::create("millivolt");
    u2->addUnit(libcellml::Units::StandardUnit::VOLT, libcellml::Units::Prefix::MILLI);

    EXPECT_EQ(0.001, libcellml::Units::scalingFactor(u1, u2));
    EXPECT_EQ(1000.0, libcellml::Units::scalingFactor(u2, u1));
}

TEST(Units, scalingFactorChildUnitsNotFound)
{
    auto model = libcellml::Model::create("nurseryrhymes");
    auto u1 = libcellml::Units::create("bo_peep");
    u1->addUnit("sheep");

    auto u2 = libcellml::Units::create("flock");

    model->addUnits(u1);
    model->addUnits(u2);

    EXPECT_EQ(0.0, libcellml::Units::scalingFactor(u1, u2));
}

TEST(Units, scalingFactorGrandchildUnitsNotFound)
{
    auto model = libcellml::Model::create("nurseryrhymes");

    auto u0 = libcellml::Units::create("sheep");
    u0->addUnit("wool");

    auto u1 = libcellml::Units::create("bo_peep");
    u1->addUnit("sheep");

    auto u2 = libcellml::Units::create("flock");
    u2->addUnit("sheep");

    model->addUnits(u0);
    model->addUnits(u1);
    model->addUnits(u2);

    // Child units of "wool" are not found within "sheep" units.  Comparison should return 0.0.
    EXPECT_EQ(0.0, libcellml::Units::scalingFactor(u1, u2));
}

TEST(Units, compatibleUnitsInDifferentModelsChildNotFound)
{
    auto m1 = libcellml::Model::create("nurseryrhymes");
    auto m2 = libcellml::Model::create("farmwork");
    auto u1 = libcellml::Units::create("bo_peep");
    u1->addUnit("sheep");

    auto u2 = libcellml::Units::create("flock");
    auto u3 = libcellml::Units::create("sheep");
    u2->addUnit("sheep");

    m1->addUnits(u1);
    m2->addUnits(u2);
    m2->addUnits(u3);

    // Child unit of "sheep" exists only in m2, return 0.0.
    EXPECT_EQ(0.0, libcellml::Units::scalingFactor(u1, u2));
}

TEST(Units, importedGrandchildUnitsNotCompatible)
{
    auto model = libcellml::Model::create("nurseryrhymes");

    auto imported_units = libcellml::Units::create("imported_units");
    auto imp = libcellml::ImportSource::create();
    imp->setUrl("i_am_a_url");
    imported_units->setImportSource(imp);

    auto u0 = libcellml::Units::create("u0");
    u0->addUnit("imported_units");

    auto u1 = libcellml::Units::create("u1");
    u1->addUnit("u0");

    auto u2 = libcellml::Units::create("u2");
    u2->addUnit("u1");

    auto u3 = libcellml::Units::create("u3");
    u3->addUnit("imported_units");

    model->addUnits(u0);
    model->addUnits(u1);
    model->addUnits(u2);
    model->addUnits(u3);
    model->addUnits(imported_units);

    // Expect false only because the first units "imported_units" is imported... otherwise it would be true.
    // Imports are not resolved during the compatibility/equivalence checking, so any comparisons which
    // have unresolved imports must return `false` as there's not enough information to confirm that they're
    // the same.
    EXPECT_FALSE(libcellml::Units::compatible(u2, u3));
}

TEST(Units, scalingFactorBetweenExponentialUnits)
{
    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("u1");
    u1->addUnit("metre", 0, 3.0, 1000.0);
    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("u2");
    u2->addUnit("metre", 0, 1.0, 1.0);
    u2->addUnit("metre", 0, 1.0, 1.0);
    u2->addUnit("metre", 0, 1.0, 1.0);

    EXPECT_EQ(1000.0, libcellml::Units::scalingFactor(u2, u1));
    EXPECT_EQ(0.001, libcellml::Units::scalingFactor(u1, u2));
}

TEST(Units, scalingFactorBetweenMultipliedUnits)
{
    libcellml::UnitsPtr u1 = libcellml::Units::create();
    u1->setName("u1");
    u1->addUnit("metre", 0, 3.0, 1000.0);
    libcellml::UnitsPtr u2 = libcellml::Units::create();
    u2->setName("u2");
    u2->addUnit("metre", 0, 3.0, 1.0);

    EXPECT_EQ(1000.0, libcellml::Units::scalingFactor(u2, u1));
    EXPECT_EQ(0.001, libcellml::Units::scalingFactor(u1, u2));
}

TEST(Units, parsedUnitsRequiringImports)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("requires_imports.cellml"));

    // When parsing a model it's hard to know whether the units it uses are imported
    // or not if the import is in a child units item.  Using the Units::requiresImports()
    // function should tell us.

    EXPECT_EQ(size_t(3), model->unitsCount());
    EXPECT_FALSE(model->units("grandpa")->isImport());
    EXPECT_TRUE(model->units("grandpa")->requiresImports());
}

TEST(Units, scalingFactorIncompatibleUnitsNoChecking)
{
    // This test shows that a scaling factor can be returned for incompatible units
    // when compatibility checking is turned off.

    auto model = libcellml::Model::create("st_clements");
    auto lemons = libcellml::Units::create("lemons");
    lemons->addUnit("metre", "kilo");
    auto oranges = libcellml::Units::create("oranges");
    oranges->addUnit("second", "milli");
    model->addUnits(oranges);
    model->addUnits(lemons);

    EXPECT_EQ(1.0E+6, libcellml::Units::scalingFactor(oranges, lemons, false));
}

TEST(Units, scalingFactorSelfReferencingUnits1)
{
    auto model = libcellml::Model::create();
    auto X = libcellml::Units::create("X");
    X->addUnit("mole");
    auto mX = libcellml::Units::create("mX");
    mX->addUnit("X", "milli");
    model->addUnits(X);
    model->addUnits(mX);

    EXPECT_EQ(1000.0, libcellml::Units::scalingFactor(mX, X));

    X->addUnit("litre", -1.0);

    EXPECT_EQ(1000.0, libcellml::Units::scalingFactor(mX, X));
}

TEST(Units, scalingFactorSelfReferencingUnits2)
{
    auto model = libcellml::Model::create();
    auto X = libcellml::Units::create("X");
    X->addUnit("mole");
    auto mX = libcellml::Units::create("mX");
    mX->addUnit("X", "milli");
    model->addUnits(X);
    model->addUnits(mX);

    EXPECT_EQ(1000.0, libcellml::Units::scalingFactor(mX, X));

    X->addUnit("litre", -3.0);

    EXPECT_EQ(1000.0, libcellml::Units::scalingFactor(mX, X));
}

TEST(Units, addUnitsMultipleTimes)
{
    auto model = libcellml::Model::create("model");
    auto units = libcellml::Units::create("units");

    // Add the units once.
    EXPECT_TRUE(model->addUnits(units));
    EXPECT_EQ(size_t(1), model->unitsCount());

    // Try to add the same units a second time. Rejected.
    EXPECT_TRUE(model->addUnits(units));

    // We can't add the same units more than once, hence we still have one
    // units.
    EXPECT_EQ(size_t(2), model->unitsCount());
}

TEST(Units, setGetUnitId)
{
    auto units = libcellml::Units::create();
    units->addUnit("second");
    units->addUnit("metre");

    const std::string id1 = "id1";
    const std::string id2 = "id2";
    const std::string oor = "out_of_range";

    EXPECT_TRUE(units->setUnitId(0, id1));
    EXPECT_TRUE(units->setUnitId(1, id2));
    EXPECT_FALSE(units->setUnitId(99, oor));

    EXPECT_EQ(id1, units->unitId(0));
    EXPECT_EQ(id2, units->unitId(1));
    EXPECT_EQ("", units->unitId(99));
}

TEST(Units, childUnitsWithIllegalPrefix)
{
    auto model = libcellml::Model::create("model");

    auto u1 = libcellml::Units::create("u1");
    auto u2 = libcellml::Units::create("u2");
    auto u3 = libcellml::Units::create("u3");
    auto u4 = libcellml::Units::create("u4");
    auto u5 = libcellml::Units::create("u5");

    model->addUnits(u1);
    model->addUnits(u2);
    model->addUnits(u3);
    model->addUnits(u4);
    model->addUnits(u5);

    u5->addUnit("u4");
    u4->addUnit("u3");
    u3->addUnit("u2", "notAPrefix");
    u2->addUnit("second");
    u1->addUnit("second", "milli");

    auto scaling = libcellml::Units::scalingFactor(u1, u5);
    EXPECT_EQ(0.0, scaling);
}

TEST(Units, scalingFactorBetweenSameUnits)
{
    auto model = libcellml::Model::create("model");
    auto u1 = libcellml::Units::create("u1");
    auto u2 = libcellml::Units::create("u2");
    u1->addUnit("u2");
    model->addUnits(u1);
    model->addUnits(u2);
    auto scaling = libcellml::Units::scalingFactor(u1, u1);
    EXPECT_EQ(1.0, scaling);
}

TEST(Units, scalingFactorBetweenUnitsSameNameDifferentModels)
{
    auto model1 = libcellml::Model::create("model1");
    auto u1 = libcellml::Units::create("units");
    u1->addUnit("second", "milli");
    model1->addUnits(u1);

    auto model2 = model1->clone();
    auto u2 = model2->units(0);

    auto scaling = libcellml::Units::scalingFactor(u1, u2);
    EXPECT_EQ(1.0, scaling);
}

TEST(Units, scalingFactorBetweenUnitsSameNameLostChildren)
{
    auto model1 = libcellml::Model::create("model1");
    auto u1 = libcellml::Units::create("units");
    u1->addUnit("oranges");
    model1->addUnits(u1);

    auto model2 = model1->clone();
    auto u2 = model2->units(0);

    auto scaling = libcellml::Units::scalingFactor(u1, u2);
    EXPECT_EQ(0.0, scaling);
}

TEST(Units, scalingFactorBetweenUnitsSameNameDifferentDefinitions)
{
    auto model1 = libcellml::Model::create("model1");
    auto u1 = libcellml::Units::create("units");
    u1->addUnit("second", "milli");
    model1->addUnits(u1);

    auto u2 = libcellml::Units::create("units");
    u2->addUnit("volt");

    auto scaling = libcellml::Units::scalingFactor(u1, u2);
    EXPECT_EQ(0.0, scaling);
}

TEST(Units, scalingFactorBetweenBaseUnitsSameName)
{
    auto model = libcellml::Model::create("model");
    auto u1 = libcellml::Units::create("units");
    model->addUnits(u1);

    auto u2 = libcellml::Units::create("units");
    model->addUnits(u2);

    auto scaling = libcellml::Units::scalingFactor(u1, u2);
    EXPECT_EQ(1.0, scaling);
}

TEST(Units, scalingFactorBetweenUnitsSameNameDifferentModelsDifferentScale)
{
    auto model1 = libcellml::Model::create("model1");
    auto u1 = libcellml::Units::create("units");
    u1->addUnit("second", "milli");
    model1->addUnits(u1);

    auto model2 = libcellml::Model::create("model2");
    auto u2 = libcellml::Units::create("units");
    u2->addUnit("second");
    model2->addUnits(u2);

    auto scaling = libcellml::Units::scalingFactor(u1, u2);
    EXPECT_EQ(1000.0, scaling);
}

TEST(Units, scalingFactorBetweenUnitsDifferentScale)
{
    auto u1 = libcellml::Units::create("units");
    u1->addUnit("second", "kilo");

    auto u2 = libcellml::Units::create("units");
    u2->addUnit("second", 5, 1);

    auto scaling = libcellml::Units::scalingFactor(u1, u2);
    EXPECT_EQ(100.0, scaling);
}

TEST(Units, unknownUnitsScalingFactorCompatible)
{
    auto model = libcellml::Model::create("model1");
    auto u1 = libcellml::Units::create("units");
    u1->addUnit("banana");

    model->addUnits(u1);

    auto scaling = libcellml::Units::scalingFactor(u1, u1, true);
    EXPECT_EQ(0.0, scaling);
}

TEST(Units, unknownUnitsScalingFactorIncompatible)
{
    auto model = libcellml::Model::create("model1");
    auto u1 = libcellml::Units::create("units");
    u1->addUnit("banana");

    model->addUnits(u1);

    auto scaling = libcellml::Units::scalingFactor(u1, u1, false);
    EXPECT_EQ(0.0, scaling);
}

TEST(ExampleUnits, unitsScalingWithUnresolvedImports)
{
    const std::string modelString =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"multiple_clash\">\n"
        "  <import xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"my_model.cellml\">\n"
        "    <units units_ref=\"my_units\" name=\"my_units\"/>\n"
        "  </import>\n"
        "  <units name=\"strange_units\">\n"
        "    <unit units=\"my_units\"/>\n"
        "    <unit exponent=\"-1\" units=\"\"/>\n"
        "  </units>\n"
        "</model>";

    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(modelString);
    auto u2 = libcellml::Units::create("units");
    u2->addUnit("banana");

    model->addUnits(u2);
    auto u1 = model->units("strange_units");

    auto scaling = libcellml::Units::scalingFactor(u1, u2, false);
    EXPECT_EQ(0.0, scaling);
}

TEST(Units, circularImportDeeperLevelBaseUnits)
{
    auto model1 = libcellml::Model::create("model1");
    auto model2 = libcellml::Model::create("model2");
    auto model3 = libcellml::Model::create("model3");

    auto units1 = libcellml::Units::create("units1");
    auto units2 = libcellml::Units::create("units2");
    auto units3 = libcellml::Units::create("units3");

    auto imp1 = libcellml::ImportSource::create();
    auto imp2 = libcellml::ImportSource::create();
    auto imp3 = libcellml::ImportSource::create();

    imp1->setModel(model2);
    imp2->setModel(model3);
    imp3->setModel(model2);

    model1->addUnits(units1);
    model2->addUnits(units2);
    model3->addUnits(units3);

    units1->setImportSource(imp1);
    units1->setImportReference("units2");
    units2->setImportSource(imp2);
    units2->setImportReference("units3");
    units3->setImportSource(imp3);
    units3->setImportReference("units2");

    EXPECT_FALSE(units1->isBaseUnit());
}

TEST(Units, validateConnectionsWithBaseUnits)
{
    const std::string m =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"base_units_test\">\n"
        "  <units name=\"cells\"/>\n"
        "  <component name=\"c1\">\n"
        "    <variable name=\"c1_v\" units=\"cells\" initial_value=\"100\" interface=\"public\"/>\n"
        "  </component>\n"
        "  <component name=\"c2\">\n"
        "    <variable name=\"c2_v\" units=\"cells\" interface=\"public\"/>\n"
        "  </component>\n"
        "  <connection component_1=\"c1\" component_2=\"c2\">\n"
        "    <map_variables variable_1=\"c1_v\" variable_2=\"c2_v\"/>\n"
        "  </connection>\n"
        "</model>\n";

    libcellml::ParserPtr parser = libcellml::Parser::create();
    libcellml::ModelPtr model = parser->parseModel(m);

    libcellml::ValidatorPtr validator = libcellml::Validator::create();
    validator->validateModel(model);

    EXPECT_EQ(size_t(0), validator->issueCount());
}

TEST(Units, equivalentUnitsMatchingBuiltinUnits)
{
    libcellml::UnitsPtr units = libcellml::Units::create("kelvin");
    units->addUnit(libcellml::Units::StandardUnit::KELVIN);

    libcellml::UnitsPtr unitsClone = units->clone();

    EXPECT_TRUE(libcellml::Units::compatible(units, unitsClone));
}

TEST(Units, equivalentUnitsMatchingBuiltinUnitsFromVariable)
{
    libcellml::ModelPtr model = libcellml::Model::create("unit_compatible_test");

    libcellml::VariablePtr variable = libcellml::Variable::create("T");
    libcellml::UnitsPtr units = libcellml::Units::create("kelvin");
    units->addUnit(libcellml::Units::StandardUnit::KELVIN);

    model->addUnits(units);
    variable->setUnits(units);

    libcellml::VariablePtr variableParam = variable->clone();

    EXPECT_TRUE(libcellml::Units::compatible(variable->units(), variableParam->units()));
}

TEST(Units, equivalentUnitsWithImportUsingNonExistentUnits)
{
    const std::string importModelString =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"strange_units\">\n"
        "  <units name=\"strange_units\">\n"
        "    <unit units=\"my_units\"/>\n"
        "    <unit exponent=\"-1\" units=\"second\"/>\n"
        "  </units>\n"
        "</model>";
    const std::string modelString =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"missing_unit_definition\">\n"
        "  <import xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"strange_units.cellml\">\n"
        "    <units units_ref=\"strange_units\" name=\"strange_units\"/>\n"
        "  </import>\n"
        "</model>";

    libcellml::ImporterPtr importer = libcellml::Importer::create();
    libcellml::ParserPtr parser = libcellml::Parser::create();

    libcellml::ModelPtr model = parser->parseModel(modelString);
    libcellml::ModelPtr importModel = parser->parseModel(importModelString);

    libcellml::UnitsPtr units = libcellml::Units::create("kelvin");
    units->addUnit(libcellml::Units::StandardUnit::KELVIN);

    model->addUnits(units);

    importer->addModel(importModel, "strange_units.cellml");
    importer->resolveImports(model, ".");

    EXPECT_FALSE(libcellml::Units::compatible(model->units(0), units));
}

libcellml::ModelPtr prepareImportUnitsWithReferenceToNonStandardUnits(const std::string &importUnitsName)
{
    libcellml::ModelPtr unitsModel = libcellml::Model::create("Units_Database");
    libcellml::ModelPtr model = libcellml::Model::create("main_model");

    libcellml::UnitsPtr u1 = libcellml::Units::create("fmol");
    u1->addUnit("mole", "femto");
    libcellml::UnitsPtr u2 = libcellml::Units::create("per_fmol");
    u2->addUnit("fmol", -1.0);
    libcellml::UnitsPtr u3 = libcellml::Units::create("per_sec");
    u3->addUnit("second", -1.0);
    libcellml::UnitsPtr u4 = libcellml::Units::create("per_sec_fmol");
    u4->addUnit("per_sec");
    u4->addUnit("per_fmol");
    libcellml::UnitsPtr u5 = libcellml::Units::create(importUnitsName);

    unitsModel->addUnits(u1);
    unitsModel->addUnits(u2);
    unitsModel->addUnits(u3);
    unitsModel->addUnits(u4);

    libcellml::ImportSourcePtr import = libcellml::ImportSource::create();
    import->setUrl("I_am_a_url");
    import->setModel(unitsModel);

    libcellml::ComponentPtr c = libcellml::Component::create("env");
    libcellml::VariablePtr v = libcellml::Variable::create("v");

    u5->setImportSource(import);
    u5->setImportReference("per_sec_fmol");

    v->setUnits(u5);
    c->addVariable(v);
    model->addUnits(u5);
    model->addComponent(c);

    EXPECT_FALSE(model->hasUnresolvedImports());

    libcellml::ImporterPtr i = libcellml::Importer::create();

    return i->flattenModel(model);
}

TEST(Units, importUnitsWithReferenceToNonStandardUnits)
{
    libcellml::ModelPtr m = prepareImportUnitsWithReferenceToNonStandardUnits("pre_sec_fmol");
    EXPECT_TRUE(m->hasUnits("fmol"));
}

TEST(Units, importUnitsWithReferenceToNonStandardUnitsDuplicateName)
{
    libcellml::ModelPtr m = prepareImportUnitsWithReferenceToNonStandardUnits("fmol");
    EXPECT_TRUE(m->hasUnits("fmol_1"));
}

TEST(Units, importMultipleUnitsFromSameDocumentWithSameUnitsDependency)
{
    libcellml::ModelPtr unitsModel = libcellml::Model::create("Units_Database");
    libcellml::ModelPtr model = libcellml::Model::create("main_model");

    libcellml::UnitsPtr u1 = libcellml::Units::create("fmol");
    u1->addUnit("mole", "femto");
    libcellml::UnitsPtr u2 = libcellml::Units::create("per_fmol");
    u2->addUnit("fmol", -1.0);

    libcellml::UnitsPtr iu1 = libcellml::Units::create("fmol");
    libcellml::UnitsPtr iu2 = libcellml::Units::create("per_fmol");

    unitsModel->addUnits(u1);
    unitsModel->addUnits(u2);

    libcellml::ImportSourcePtr import = libcellml::ImportSource::create();
    import->setUrl("I_am_a_url");
    import->setModel(unitsModel);

    iu1->setImportSource(import);
    iu1->setImportReference("fmol");
    iu2->setImportSource(import);
    iu2->setImportReference("per_fmol");

    model->addUnits(iu1);
    model->addUnits(iu2);

    EXPECT_FALSE(model->hasUnresolvedImports());

    libcellml::ImporterPtr i = libcellml::Importer::create();

    auto flatModel = i->flattenModel(model);

    EXPECT_EQ(size_t(2), flatModel->unitsCount());
    EXPECT_EQ("fmol", flatModel->units(0)->name());
    EXPECT_EQ("per_fmol", flatModel->units(1)->name());
}

TEST(Units, importUnitsMultipleWaysSameDocument)
{
    libcellml::ParserPtr parser = libcellml::Parser::create();
    libcellml::ImporterPtr importer = libcellml::Importer::create();

    auto model = parser->parseModel(fileContents("importer/units/main.cellml"));

    EXPECT_TRUE(model->hasUnresolvedImports());

    importer->resolveImports(model, resourcePath("importer/units"));

    EXPECT_FALSE(model->hasUnresolvedImports());

    auto flatModel = importer->flattenModel(model);

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(flatModel);
    EXPECT_EQ(size_t(2), analyser->issueCount());
}

libcellml::ImportSourcePtr createBaseModelImport(libcellml::ModelPtr &importModel, const std::string &resourcePath)
{
    libcellml::ParserPtr parser = libcellml::Parser::create();

    importModel = parser->parseModel(fileContents(resourcePath));

    libcellml::ImportSourcePtr import = libcellml::ImportSource::create();
    import->setUrl("I_am_a_url");
    import->setModel(importModel);

    return import;
}

TEST(Units, flattenImportedUnitsWithNonStandardReferenceDirectly)
{
    libcellml::ModelPtr importModel;
    libcellml::ImporterPtr importer = libcellml::Importer::create();
    libcellml::ImportSourcePtr import = createBaseModelImport(importModel, "importer/units/base_model.cellml");

    libcellml::ModelPtr model = libcellml::Model::create("main_model");

    libcellml::UnitsPtr iu = libcellml::Units::create("per_fmol");
    iu->setImportSource(import);
    iu->setImportReference("per_fmol");

    model->addUnits(iu);

    auto flatModel = importer->flattenModel(model);

    EXPECT_EQ(size_t(2), flatModel->unitsCount());
    EXPECT_EQ("per_fmol", flatModel->units(0)->name());
    EXPECT_EQ("fmol", flatModel->units(1)->name());
}

TEST(Units, flattenImportedUnitsWithNonStandardReferenceThroughComponent)
{
    libcellml::ModelPtr importModel;
    libcellml::ImporterPtr importer = libcellml::Importer::create();
    libcellml::ImportSourcePtr import = createBaseModelImport(importModel, "importer/units/base_model.cellml");

    libcellml::ModelPtr model = libcellml::Model::create("main_model");

    libcellml::ComponentPtr ic = libcellml::Component::create("my_component");
    ic->setImportSource(import);
    ic->setImportReference("my_component");

    model->addComponent(ic);

    auto flatModel = importer->flattenModel(model);

    EXPECT_EQ(size_t(0), importer->issueCount());
}

TEST(Units, flattenImportedUnitsWithNonStandardReferenceDirectlyWithExistingEquivalentUnits)
{
    libcellml::ModelPtr importModel;
    libcellml::ImporterPtr importer = libcellml::Importer::create();
    libcellml::ImportSourcePtr import = createBaseModelImport(importModel, "importer/units/base_model.cellml");

    libcellml::ModelPtr model = libcellml::Model::create("main_model");

    libcellml::UnitsPtr iu = libcellml::Units::create("per_fmol");
    iu->setImportSource(import);
    iu->setImportReference("per_fmol");

    libcellml::UnitsPtr u = libcellml::Units::create("fmol");
    u->addUnit("mole", "femto");
    model->addUnits(iu);
    model->addUnits(u);

    auto flatModel = importer->flattenModel(model);

    EXPECT_EQ(size_t(2), flatModel->unitsCount());
    EXPECT_EQ("per_fmol", flatModel->units(0)->name());
    EXPECT_EQ("fmol", flatModel->units(1)->name());
}

TEST(Units, flattenImportedUnitsWithNonStandardReferenceDirectlyWithExistingEquivalentUnitsWithDifferentName)
{
    libcellml::ModelPtr importModel;
    libcellml::ImporterPtr importer = libcellml::Importer::create();
    libcellml::ImportSourcePtr import = createBaseModelImport(importModel, "importer/units/base_model.cellml");

    libcellml::ModelPtr model = libcellml::Model::create("main_model");

    libcellml::UnitsPtr iu = libcellml::Units::create("per_fmol");
    iu->setImportSource(import);
    iu->setImportReference("per_fmol");

    libcellml::UnitsPtr u = libcellml::Units::create("femto_mole");
    u->addUnit("mole", "femto");
    model->addUnits(iu);
    model->addUnits(u);

    auto flatModel = importer->flattenModel(model);

    EXPECT_EQ(size_t(2), flatModel->unitsCount());
    EXPECT_EQ("per_fmol", flatModel->units(0)->name());
    EXPECT_EQ("femto_mole", flatModel->units(1)->name());
}

libcellml::ModelPtr setupFlattenImportedUnitsThroughComponent(const std::string &baseModelName)
{
    libcellml::ModelPtr importModel;
    libcellml::ImporterPtr importer = libcellml::Importer::create();
    libcellml::ImportSourcePtr import = createBaseModelImport(importModel, baseModelName);

    libcellml::ModelPtr model = libcellml::Model::create("main_model");

    libcellml::ComponentPtr ic = libcellml::Component::create("my_component");
    ic->setImportSource(import);
    ic->setImportReference("my_component");

    libcellml::UnitsPtr u = libcellml::Units::create("fmol");
    u->addUnit("mole", "femto");

    model->addComponent(ic);
    model->addUnits(u);

    return importer->flattenModel(model);
}

TEST(Units, flattenImportedUnitsWithNonStandardReferenceDirectlyWithExistingEquivalentUnitsThroughComponent)
{
    auto flatModel = setupFlattenImportedUnitsThroughComponent("importer/units/base_model.cellml");

    EXPECT_EQ(size_t(2), flatModel->unitsCount());
    EXPECT_EQ("fmol", flatModel->units(0)->name());
    EXPECT_EQ("per_fmol", flatModel->units(1)->name());
    EXPECT_EQ("per_fmol", flatModel->component(0)->variable(0)->units()->name());
}

TEST(Units, flattenImportedUnitsWithNonStandardReferenceDirectlyWithExistingEquivalentUnitsThroughComponentContainingIllDefinedUnits)
{
    auto flatModel = setupFlattenImportedUnitsThroughComponent("importer/units/base_model_illdefined.cellml");

    EXPECT_EQ(nullptr, flatModel);
}

TEST(Units, flattenImportedUnitsWithNonStandardReferenceDirectlyWithExistingEquivalentUnitsWithDifferentNameThroughComponent)
{
    libcellml::ModelPtr importModel;
    libcellml::ImporterPtr importer = libcellml::Importer::create();
    libcellml::ImportSourcePtr import = createBaseModelImport(importModel, "importer/units/base_model.cellml");

    libcellml::ModelPtr model = libcellml::Model::create("main_model");

    libcellml::ComponentPtr ic = libcellml::Component::create("my_component");
    ic->setImportSource(import);
    ic->setImportReference("my_component");

    libcellml::UnitsPtr u = libcellml::Units::create("femtomole");
    u->addUnit("mole", "femto");

    model->addComponent(ic);
    model->addUnits(u);

    auto flatModel = importer->flattenModel(model);

    EXPECT_EQ(size_t(2), flatModel->unitsCount());
    EXPECT_EQ("femtomole", flatModel->units(0)->name());
    EXPECT_EQ("per_fmol", flatModel->units(1)->name());
    EXPECT_EQ("femtomole", flatModel->units(1)->unitAttributeReference(0));
    EXPECT_EQ("per_fmol", flatModel->component(0)->variable(0)->units()->name());
}

TEST(Units, flattenImportedUnitsWithNonStandardReferenceDirectlyWithExistingNonEquivalentUnits)
{
    libcellml::ModelPtr importModel;
    libcellml::ImporterPtr importer = libcellml::Importer::create();
    libcellml::ImportSourcePtr import = createBaseModelImport(importModel, "importer/units/base_model.cellml");

    libcellml::ModelPtr model = libcellml::Model::create("main_model");

    libcellml::UnitsPtr iu = libcellml::Units::create("per_fmol");
    iu->setImportSource(import);
    iu->setImportReference("per_fmol");

    libcellml::UnitsPtr u = libcellml::Units::create("fmol");
    u->addUnit("farad");
    u->addUnit("mole");
    model->addUnits(iu);
    model->addUnits(u);

    auto flatModel = importer->flattenModel(model);

    EXPECT_EQ(size_t(3), flatModel->unitsCount());
    EXPECT_EQ("per_fmol", flatModel->units(0)->name());
    EXPECT_EQ("fmol", flatModel->units(1)->name());
    EXPECT_EQ("fmol_1", flatModel->units(2)->name());
}

TEST(Units, flattenImportedUnitsWithNonStandardReferenceDirectlyWithExistingNonEquivalentUnitsThroughComponent)
{
    libcellml::ModelPtr importModel;
    libcellml::ImporterPtr importer = libcellml::Importer::create();
    libcellml::ImportSourcePtr import = createBaseModelImport(importModel, "importer/units/base_model.cellml");

    libcellml::ModelPtr model = libcellml::Model::create("main_model");

    libcellml::ComponentPtr ic = libcellml::Component::create("my_component");
    ic->setImportSource(import);
    ic->setImportReference("my_component");

    libcellml::UnitsPtr u = libcellml::Units::create("fmol");
    u->addUnit("farad");
    u->addUnit("mole");

    model->addComponent(ic);
    model->addUnits(u);

    auto flatModel = importer->flattenModel(model);

    EXPECT_EQ(size_t(3), flatModel->unitsCount());
    EXPECT_EQ("fmol", flatModel->units(0)->name());
    EXPECT_EQ("fmol_1", flatModel->units(1)->name());
    EXPECT_EQ("per_fmol", flatModel->units(2)->name());
    EXPECT_EQ("fmol_1", flatModel->units(2)->unitAttributeReference(0));
    EXPECT_EQ("per_fmol", flatModel->component(0)->variable(0)->units()->name());
}

TEST(Units, flattenImportedUnitsWithNonStandardReferenceDirectlyWithExistingNonEquivalentUnitsThroughComponentWithVariableNeedingRenamedUnits)
{
    libcellml::ModelPtr importModel;
    libcellml::ImporterPtr importer = libcellml::Importer::create();
    libcellml::ImportSourcePtr import = createBaseModelImport(importModel, "importer/units/base_model.cellml");

    libcellml::ModelPtr model = libcellml::Model::create("main_model");

    libcellml::ComponentPtr ic = libcellml::Component::create("my_component");
    ic->setImportSource(import);
    ic->setImportReference("my_component");

    libcellml::VariablePtr v = libcellml::Variable::create("R");
    v->setUnits("fmol");
    importModel->component(0)->addVariable(v);

    libcellml::UnitsPtr u = libcellml::Units::create("fmol");
    u->addUnit("farad");
    u->addUnit("mole");

    model->addComponent(ic);
    model->addUnits(u);

    auto flatModel = importer->flattenModel(model);

    EXPECT_EQ(size_t(3), flatModel->unitsCount());
    EXPECT_EQ("fmol", flatModel->units(0)->name());
    EXPECT_EQ("fmol_1", flatModel->units(1)->name());
    EXPECT_EQ("per_fmol", flatModel->units(2)->name());
    EXPECT_EQ("fmol_1", flatModel->units(2)->unitAttributeReference(0));
    EXPECT_EQ("per_fmol", flatModel->component(0)->variable(0)->units()->name());
    EXPECT_EQ("fmol_1", flatModel->component(0)->variable(1)->units()->name());
}

TEST(Units, flattenImportedUnitsWithNonStandardReferenceDirectlyWithExistingNonEquivalentUnitsThroughComponentWithCnNeedingRenamedUnits)
{
    const std::string math =
        "<math xmlns=\"http://www.w3.org/1998/Math/MathML\" xmlns:cellml=\"http://www.cellml.org/cellml/2.0#\">\n"
        "  <apply>\n"
        "    <eq/>\n"
        "    <ci>P</ci>\n"
        "    <cn cellml:units=\"per_fmol\">3.0</cn>\n"
        "  </apply>\n"
        "  <apply>\n"
        "    <eq/>\n"
        "    <ci>R</ci>\n"
        "    <cn cellml:units=\"fmol\">4.2</cn>\n"
        "  </apply>\n"
        "</math>";

    libcellml::ModelPtr importModel;
    libcellml::ImporterPtr importer = libcellml::Importer::create();
    libcellml::ImportSourcePtr import = createBaseModelImport(importModel, "importer/units/base_model.cellml");

    libcellml::ModelPtr model = libcellml::Model::create("main_model");

    libcellml::ComponentPtr ic = libcellml::Component::create("my_component");
    ic->setImportSource(import);
    ic->setImportReference("my_component");

    importModel->component(0)->setMath(math);

    libcellml::UnitsPtr u = libcellml::Units::create("fmol");
    u->addUnit("farad");
    u->addUnit("mole");

    model->addComponent(ic);
    model->addUnits(u);

    auto flatModel = importer->flattenModel(model);

    EXPECT_EQ(size_t(3), flatModel->unitsCount());
    EXPECT_EQ("fmol", flatModel->units(0)->name());
    EXPECT_EQ("fmol_1", flatModel->units(1)->name());
    EXPECT_EQ("per_fmol", flatModel->units(2)->name());
    EXPECT_EQ("fmol_1", flatModel->units(2)->unitAttributeReference(0));
    EXPECT_EQ("per_fmol", flatModel->component(0)->variable(0)->units()->name());
    EXPECT_TRUE(flatModel->component(0)->math().find("fmol_1") != std::string::npos);
}

void setupFlattenImportedUnits(libcellml::ImporterPtr &importer, libcellml::ModelPtr &model, const std::string &variation)
{
    libcellml::ModelPtr unitsDefinitionsModel;
    libcellml::ImportSourcePtr import = createBaseModelImport(unitsDefinitionsModel, "importer/units/units_definitions.cellml");

    libcellml::ModelPtr intermediaryImportModel1 = libcellml::Model::create("import_model1");
    libcellml::ModelPtr intermediaryImportModel2 = libcellml::Model::create("import_model2");

    importer = libcellml::Importer::create();

    importer->addModel(unitsDefinitionsModel, "units_definitions");
    importer->addModel(intermediaryImportModel1, "int1");
    importer->addModel(intermediaryImportModel2, "int2");

    model = libcellml::Model::create("main_model");

    libcellml::UnitsPtr iu1 = libcellml::Units::create("per_fmol");
    iu1->setImportSource(import);
    iu1->setImportReference("per_fmol");

    libcellml::UnitsPtr iu2 = libcellml::Units::create("fmol");
    iu2->setImportSource(import);
    iu2->setImportReference("fmol");

    intermediaryImportModel1->addUnits(iu1);
    intermediaryImportModel2->addUnits(iu2);

    libcellml::ImportSourcePtr importIntermediary1 = libcellml::ImportSource::create();
    importIntermediary1->setUrl("I_am_a_url");
    importIntermediary1->setModel(intermediaryImportModel1);

    libcellml::ImportSourcePtr importIntermediary2 = libcellml::ImportSource::create();
    importIntermediary2->setUrl("I_am_a_url");
    importIntermediary2->setModel(intermediaryImportModel2);

    libcellml::UnitsPtr iu3 = libcellml::Units::create("per_fmol");
    iu3->setImportSource(importIntermediary1);
    iu3->setImportReference("per_fmol");

    libcellml::UnitsPtr iu4 = libcellml::Units::create("fmol");
    iu4->setImportSource(importIntermediary2);
    iu4->setImportReference("fmol");

    if (!variation.empty()) {
        libcellml::UnitsPtr u = libcellml::Units::create("fmol");
        u->addUnit("mole", "femto");
        if (variation == "int1") {
            intermediaryImportModel1->addUnits(u);
        } else if (variation == "main") {
            model->addUnits(u);
        }
    }

    model->addUnits(iu3);
    model->addUnits(iu4);
}

TEST(Units, flattenImportedUnitsWithNonStandardReferenceFromTwoLevelsDeep)
{
    libcellml::ImporterPtr importer;
    libcellml::ModelPtr model;

    setupFlattenImportedUnits(importer, model, "");

    auto flatModel = importer->flattenModel(model);

    EXPECT_EQ(size_t(2), flatModel->unitsCount());
    EXPECT_EQ("per_fmol", flatModel->units(0)->name());
    EXPECT_EQ("fmol", flatModel->units(1)->name());
}

TEST(Units, flattenImportedUnitsWithNonStandardReferenceFromTwoLevelsDeepEquivalentUnitsInIntermediary)
{
    libcellml::ImporterPtr importer;
    libcellml::ModelPtr model;

    setupFlattenImportedUnits(importer, model, "int1");

    auto flatModel = importer->flattenModel(model);

    EXPECT_EQ(size_t(2), flatModel->unitsCount());
    EXPECT_EQ("per_fmol", flatModel->units(0)->name());
    EXPECT_EQ("fmol", flatModel->units(1)->name());
}

TEST(Units, flattenImportedUnitsWithNonStandardReferenceFromTwoLevelsDeepEquivalentUnitsInMain)
{
    libcellml::ImporterPtr importer;
    libcellml::ModelPtr model;

    setupFlattenImportedUnits(importer, model, "main");

    auto flatModel = importer->flattenModel(model);

    EXPECT_EQ(size_t(3), flatModel->unitsCount());
    EXPECT_EQ("fmol", flatModel->units(0)->name());
    EXPECT_EQ("per_fmol", flatModel->units(1)->name());
    EXPECT_EQ("fmol", flatModel->units(2)->name());
}

TEST(Units, aliasingUnits)
{
    libcellml::UnitsPtr u1 = libcellml::Units::create("per_hour");
    u1->addUnit("mm");
    libcellml::UnitsPtr u2 = libcellml::Units::create("mm");
    u2->addUnit("second", "milli");

    libcellml::ModelPtr model = libcellml::Model::create("containing_model");

    model->addUnits(u1);
    model->addUnits(u2);

    EXPECT_TRUE(libcellml::Units::equivalent(u1, u2));
}

TEST(Units, definedUnitsBaseUnit)
{
    libcellml::UnitsPtr u = libcellml::Units::create("base_unit");

    EXPECT_TRUE(u->isDefined());
}

TEST(Units, definedUnitsStandardUnit)
{
    libcellml::UnitsPtr u = libcellml::Units::create("only_standard_units");
    u->addUnit("second");
    u->addUnit("farad");

    EXPECT_TRUE(u->isDefined());
}

TEST(Units, undefinedUnitsNonStandardUnit)
{
    libcellml::UnitsPtr u = libcellml::Units::create("only_standard_units");
    u->addUnit("battle");

    EXPECT_FALSE(u->isDefined());
    EXPECT_TRUE(u->isResolved());
}

TEST(Units, definedUnitsImport)
{
    libcellml::UnitsPtr u = libcellml::Units::create("only_standard_units");

    libcellml::ImportSourcePtr importSource = libcellml::ImportSource::create();
    importSource->setUrl("I_am_a_url");

    u->setImportSource(importSource);
    u->setImportReference("ref");

    EXPECT_FALSE(u->isDefined());
}

TEST(Units, oneLitreEqualToOneThousandCentiMetreCubed)
{
    auto model = libcellml::Model::create();
    auto oneLitre = libcellml::Units::create("oneLitre");
    oneLitre->addUnit("litre");

    auto oneThousandCentiMetreCubed = libcellml::Units::create("oneThousandCentiMetreCubed");
    oneThousandCentiMetreCubed->addUnit("metre", "centi", 3, 1000);

    model->addUnits(oneLitre);
    model->addUnits(oneThousandCentiMetreCubed);

    EXPECT_TRUE(libcellml::Units::equivalent(oneLitre, oneThousandCentiMetreCubed));
}

TEST(Units, oneKilogramEqualToOneThousandGrams)
{
    auto model = libcellml::Model::create();
    auto oneThousandGrams = libcellml::Units::create("oneThousandGrams");
    oneThousandGrams->addUnit("gram", 0, 1.0, 1000);

    auto kilogram = libcellml::Units::create("kilogram");

    model->addUnits(oneThousandGrams);
    model->addUnits(kilogram);

    EXPECT_TRUE(libcellml::Units::equivalent(kilogram, oneThousandGrams));
}

TEST(Units, oneLitreEqualToOneThousandCentiMetreCubedAllCubed)
{
    auto model = libcellml::Model::create();
    auto oneLitre = libcellml::Units::create("oneLitre");
    oneLitre->addUnit("litre");
    auto oneLitreCubed = libcellml::Units::create("oneLitreCubed");
    oneLitreCubed->addUnit("oneLitre", 3);

    auto oneThousandCentiMetreCubed = libcellml::Units::create("oneThousandCentiMetreCubed");
    oneThousandCentiMetreCubed->addUnit("metre", "centi", 3, 1000);
    auto oneThousandCentiMetreCubedCubed = libcellml::Units::create("oneThousandCentiMetreCubedCubed");
    oneThousandCentiMetreCubedCubed->addUnit("oneThousandCentiMetreCubed", 3);

    model->addUnits(oneLitre);
    model->addUnits(oneLitreCubed);
    model->addUnits(oneThousandCentiMetreCubed);
    model->addUnits(oneThousandCentiMetreCubedCubed);

    EXPECT_TRUE(libcellml::Units::equivalent(oneLitreCubed, oneThousandCentiMetreCubedCubed));
}
