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

#include <libcellml>

#include "test_utils.h"

TEST(Equality, invalidInput)
{
    libcellml::VariablePtr v1 = libcellml::Variable::create("variable");

    EXPECT_FALSE(v1->equals(nullptr));
}

TEST(Equality, variableEqualByNameOnly)
{
    libcellml::VariablePtr v1 = libcellml::Variable::create("variable");
    libcellml::VariablePtr v2 = libcellml::Variable::create("variable");

    EXPECT_NE(v1, v2);
    EXPECT_TRUE(v1->equals(v2));
    EXPECT_TRUE(v2->equals(v1));
}

TEST(Equality, variableNotEqualByNameOnly)
{
    libcellml::VariablePtr v1 = libcellml::Variable::create("variableA");
    libcellml::VariablePtr v2 = libcellml::Variable::create("variableB");

    EXPECT_NE(v1, v2);
    EXPECT_FALSE(v1->equals(v2));
    EXPECT_FALSE(v2->equals(v1));
}

TEST(Equality, variableNotEqualToUnits)
{
    libcellml::VariablePtr v1 = libcellml::Variable::create("variable");
    libcellml::UnitsPtr u1 = libcellml::Units::create("variable");

    EXPECT_FALSE(v1->equals(u1));
    EXPECT_FALSE(u1->equals(v1));
}

TEST(Equality, variableEqualById)
{
    libcellml::VariablePtr v1 = libcellml::Variable::create("variable");
    libcellml::VariablePtr v2 = libcellml::Variable::create("variable");

    v1->setId("lefisjeij");
    v2->setId("lefisjeij");

    EXPECT_TRUE(v1->equals(v2));
    EXPECT_TRUE(v2->equals(v1));
}

TEST(Equality, variableNotEqualById)
{
    libcellml::VariablePtr v1 = libcellml::Variable::create("variable");
    libcellml::VariablePtr v2 = libcellml::Variable::create("variable");

    v1->setId("eijei3009");
    v2->setId("lefisjeij");

    EXPECT_FALSE(v1->equals(v2));
    EXPECT_FALSE(v2->equals(v1));
}

TEST(Equality, variableEqualAllTestedConditionsVariant1)
{
    libcellml::VariablePtr v1 = libcellml::Variable::create();
    libcellml::VariablePtr v2 = libcellml::Variable::create();

    v1->setName("variable");
    v2->setName("variable");

    v1->setInitialValue(3.0);
    v2->setInitialValue("3");

    v1->setInterfaceType(libcellml::Variable::InterfaceType::PUBLIC);
    v2->setInterfaceType(libcellml::Variable::InterfaceType::PUBLIC);

    v1->setUnits("ampere");
    v2->setUnits("ampere");

    EXPECT_TRUE(v1->equals(v2));
    EXPECT_TRUE(v2->equals(v1));
}

TEST(Equality, variableEqualAllTestedConditionsVariant2)
{
    libcellml::VariablePtr v1 = libcellml::Variable::create("variable");
    libcellml::VariablePtr v2 = libcellml::Variable::create("variable");

    libcellml::VariablePtr v3 = libcellml::Variable::create("initialValue");
    libcellml::VariablePtr v4 = libcellml::Variable::create("initialValue");

    libcellml::UnitsPtr u1 = libcellml::Units::create("m_per_s");
    libcellml::UnitsPtr u2 = libcellml::Units::create("m_per_s");

    v1->setInitialValue(v3);
    v2->setInitialValue(v4);

    v1->setInterfaceType(libcellml::Variable::InterfaceType::PUBLIC);
    v2->setInterfaceType(libcellml::Variable::InterfaceType::PUBLIC);

    v1->setUnits(u1);
    v2->setUnits(u2);

    EXPECT_TRUE(v1->equals(v2));
    EXPECT_TRUE(v2->equals(v1));
}

TEST(Equality, variableNotEqualByUnits)
{
    libcellml::VariablePtr v1 = libcellml::Variable::create("variable");
    libcellml::VariablePtr v2 = libcellml::Variable::create("variable");

    libcellml::UnitsPtr u1 = libcellml::Units::create("m_per_s");

    v1->setUnits(u1);

    EXPECT_FALSE(v1->equals(v2));
    EXPECT_FALSE(v2->equals(v1));
}

TEST(Equality, variableNotEqualByInitialCondition)
{
    libcellml::VariablePtr v1 = libcellml::Variable::create("variable");
    libcellml::VariablePtr v2 = libcellml::Variable::create("variable");

    v1->setInitialValue(3.2);

    EXPECT_FALSE(v1->equals(v2));
    EXPECT_FALSE(v2->equals(v1));
}

TEST(Equality, variableNotEqualByInterfaceType)
{
    libcellml::VariablePtr v1 = libcellml::Variable::create("variable");
    libcellml::VariablePtr v2 = libcellml::Variable::create("variable");

    v1->setInterfaceType("public");

    EXPECT_FALSE(v1->equals(v2));
    EXPECT_FALSE(v2->equals(v1));
}

TEST(Equality, unitsEqualByNameOnly)
{
    libcellml::UnitsPtr u1 = libcellml::Units::create("units");
    libcellml::UnitsPtr u2 = libcellml::Units::create("units");

    EXPECT_NE(u1, u2);
    EXPECT_TRUE(u1->equals(u2));
    EXPECT_TRUE(u2->equals(u1));
    EXPECT_TRUE(libcellml::Units::equivalent(u1, u2));
}

TEST(Equality, unitsNotEqualByNameOnly)
{
    libcellml::UnitsPtr u1 = libcellml::Units::create("unitsA");
    libcellml::UnitsPtr u2 = libcellml::Units::create("unitsB");

    EXPECT_NE(u1, u2);
    EXPECT_FALSE(u1->equals(u2));
    EXPECT_FALSE(u2->equals(u1));
    EXPECT_FALSE(libcellml::Units::equivalent(u1, u2));
}

TEST(Equality, unitsNotEqualByUnitPrefixOnly)
{
    libcellml::UnitsPtr u1 = libcellml::Units::create("unitsA");
    libcellml::UnitsPtr u2 = libcellml::Units::create("unitsA");

    u1->addUnit("second", 3, 2.0, 5.0);
    u2->addUnit("second", 4, 2.0, 5.0);

    EXPECT_FALSE(u1->equals(u2));
    EXPECT_FALSE(u2->equals(u1));
    EXPECT_FALSE(libcellml::Units::equivalent(u1, u2));
}

TEST(Equality, unitsNotEqualByUnitIdOnly)
{
    libcellml::UnitsPtr u1 = libcellml::Units::create("unitsA");
    libcellml::UnitsPtr u2 = libcellml::Units::create("unitsA");

    u1->addUnit("second");
    u2->addUnit("second");

    u1->setUnitId(0, "UnitsASecond");

    EXPECT_NE(u1, u2);
    EXPECT_FALSE(u1->equals(u2));
    EXPECT_FALSE(u2->equals(u1));
    EXPECT_TRUE(libcellml::Units::equivalent(u1, u2));
}

TEST(Equality, unitsNotEqualByUnitMultiplierOnly)
{
    libcellml::UnitsPtr u1 = libcellml::Units::create("unitsA");
    libcellml::UnitsPtr u2 = libcellml::Units::create("unitsA");

    u1->addUnit("second", 3, 2.0, 5.0);
    u2->addUnit("second", 3, 2.0, 3.0);

    EXPECT_NE(u1, u2);
    EXPECT_FALSE(u1->equals(u2));
    EXPECT_FALSE(u2->equals(u1));
    EXPECT_FALSE(libcellml::Units::equivalent(u1, u2));
}

TEST(Equality, unitsEqualAllTestedConditionsVariant1)
{
    libcellml::UnitsPtr u1 = libcellml::Units::create("unitsA");
    libcellml::UnitsPtr u2 = libcellml::Units::create("unitsA");

    libcellml::ImportSourcePtr is = libcellml::ImportSource::create();

    u1->setImportReference("some_location");
    u2->setImportReference("some_location");

    u1->setSourceUnits(is, "ampere");
    u2->setSourceUnits(is, "ampere");

    u1->addUnit("banana");
    u2->addUnit("banana");

    EXPECT_TRUE(u1->equals(u2));
    EXPECT_TRUE(u2->equals(u1));
}

TEST(Equality, unitsEqualAllTestedConditionsVariant2)
{
    libcellml::UnitsPtr u1 = libcellml::Units::create("unitsA");
    libcellml::UnitsPtr u2 = libcellml::Units::create("unitsA");

    libcellml::ImportSourcePtr is = libcellml::ImportSource::create();

    u1->setImportReference("some_location");
    u2->setImportReference("some_location");

    u1->setSourceUnits(is, "ampere");
    u2->setSourceUnits(is, "ampere");

    u1->addUnit("second", 1.0, "unit_id");
    u2->addUnit("second", 1.0, "unit_id");

    EXPECT_TRUE(u1->equals(u2));
    EXPECT_TRUE(u2->equals(u1));
}

TEST(Equality, unitsNotEqualDifferingNumberOfUnits)
{
    libcellml::UnitsPtr u1 = libcellml::Units::create("unitsA");
    libcellml::UnitsPtr u2 = libcellml::Units::create("unitsA");

    u1->addUnit("second", 1.0, "second_id");
    u1->addUnit("metre", -1.0, "metre_id");
    u2->addUnit("second", 1.0, "second_id");

    EXPECT_FALSE(u1->equals(u2));
    EXPECT_FALSE(u2->equals(u1));
}

TEST(Equality, unitsNotEqualDifferingUnitsBasic)
{
    libcellml::UnitsPtr u1 = libcellml::Units::create("unitsA");
    libcellml::UnitsPtr u2 = libcellml::Units::create("unitsA");

    u1->addUnit("second", 1.0, "unit_id");
    u2->addUnit("second", 2.0, "unit_id");

    EXPECT_FALSE(u1->equals(u2));
    EXPECT_FALSE(u2->equals(u1));
}

TEST(Equality, unitsEqualDifferingOrderOfUnits)
{
    libcellml::UnitsPtr u1 = libcellml::Units::create("unitsA");
    libcellml::UnitsPtr u2 = libcellml::Units::create("unitsA");

    u1->addUnit("second", 1.0, "unit_id");
    u1->addUnit("metre", 3.0);
    u2->addUnit("metre", 3.0);
    u2->addUnit("second", 1.0, "unit_id");

    EXPECT_TRUE(u1->equals(u2));
    EXPECT_TRUE(u2->equals(u1));
}

TEST(Equality, unitsEqualDifferingOrderOfUnitsManyUnit)
{
    libcellml::UnitsPtr u1 = libcellml::Units::create("unitsA");
    libcellml::UnitsPtr u2 = libcellml::Units::create("unitsA");

    u1->addUnit("second", 1.0, "unit_id");
    u1->addUnit("metre", 3.0);
    u1->addUnit("apple", 1.0);
    u1->addUnit("jedi", "yota", -2.0);
    u1->addUnit("flip");
    u1->addUnit("lumen");

    u2->addUnit("jedi", "yota", -2.0);
    u2->addUnit("second", 1.0, "unit_id");
    u2->addUnit("lumen");
    u2->addUnit("metre", 3.0);
    u2->addUnit("apple", 1.0);
    u2->addUnit("flip");

    EXPECT_TRUE(u1->equals(u2));
    EXPECT_TRUE(u2->equals(u1));
}

TEST(Equality, unitsNotEqualDifferingOrderOfUnits)
{
    libcellml::UnitsPtr u1 = libcellml::Units::create("unitsA");
    libcellml::UnitsPtr u2 = libcellml::Units::create("unitsA");

    u1->addUnit("metre", 3.0);
    u1->addUnit("second", 1.0, "unit_id");
    u2->addUnit("second", 2.0, "unit_id");
    u2->addUnit("metre", 3.0);

    EXPECT_FALSE(u1->equals(u2));
    EXPECT_FALSE(u2->equals(u1));
}

TEST(Equality, unitsEqualPrecision1)
{
    libcellml::UnitsPtr u1 = libcellml::Units::create("unitsA");
    libcellml::UnitsPtr u2 = libcellml::Units::create("unitsA");

    u1->addUnit("second", 1.0);
    u2->addUnit("second", 1.0 + std::numeric_limits<double>::epsilon());

    EXPECT_TRUE(u1->equals(u2));
    EXPECT_TRUE(u2->equals(u1));
}

TEST(Equality, unitsEqualPrecision2)
{
    libcellml::UnitsPtr u1 = libcellml::Units::create("unitsA");
    libcellml::UnitsPtr u2 = libcellml::Units::create("unitsA");

    u1->addUnit("second", 1.0);
    u2->addUnit("second", 1.0 + 2 * std::numeric_limits<double>::epsilon());

    EXPECT_FALSE(u1->equals(u2));
    EXPECT_FALSE(u2->equals(u1));
}

TEST(Equality, unitsEqualPrecision3)
{
    libcellml::UnitsPtr u1 = libcellml::Units::create("unitsA");
    libcellml::UnitsPtr u2 = libcellml::Units::create("unitsA");

    u1->addUnit("second", 2.000000000000000444089209850062616169452667236328125);
    u2->addUnit("second", 2.0);

    EXPECT_TRUE(u1->equals(u2));
    EXPECT_TRUE(u2->equals(u1));
}

TEST(Equality, unitsEqualPrecision4)
{
    libcellml::UnitsPtr u1 = libcellml::Units::create("unitsA");
    libcellml::UnitsPtr u2 = libcellml::Units::create("unitsA");

    u1->addUnit("second", 238487722399.29384);
    u2->addUnit("second", 238487722399.29383);

    EXPECT_TRUE(u1->equals(u2));
    EXPECT_TRUE(u2->equals(u1));
}

TEST(Equality, unitsEqualPrecision4Switched)
{
    libcellml::UnitsPtr u1 = libcellml::Units::create("unitsA");
    libcellml::UnitsPtr u2 = libcellml::Units::create("unitsA");

    u1->addUnit("second", 238487722399.29383);
    u2->addUnit("second", 238487722399.29384);

    EXPECT_TRUE(u1->equals(u2));
    EXPECT_TRUE(u2->equals(u1));
}

TEST(Equality, unitsNotEqualExponent)
{
    libcellml::UnitsPtr u1 = libcellml::Units::create("unitsA");
    libcellml::UnitsPtr u2 = libcellml::Units::create("unitsA");

    u1->addUnit("second", -3.0);
    u2->addUnit("second", 2.0);

    EXPECT_FALSE(u1->equals(u2));
    EXPECT_FALSE(u2->equals(u1));
}

TEST(Equality, unitsNotEqualInfinity)
{
    libcellml::UnitsPtr u1 = libcellml::Units::create("unitsA");
    libcellml::UnitsPtr u2 = libcellml::Units::create("unitsA");

    u1->addUnit("second", std::numeric_limits<double>::infinity());
    u2->addUnit("second", 3.0);

    EXPECT_FALSE(u1->equals(u2));
    EXPECT_FALSE(u2->equals(u1));
}

TEST(Equality, unitsNotEqualNaN)
{
    libcellml::UnitsPtr u1 = libcellml::Units::create("unitsA");
    libcellml::UnitsPtr u2 = libcellml::Units::create("unitsA");

    u1->addUnit("second", std::numeric_limits<double>::quiet_NaN());
    u2->addUnit("second", 2.0);

    EXPECT_FALSE(u1->equals(u2));
    EXPECT_FALSE(u2->equals(u1));
}

libcellml::ResetPtr createInitialisedReset()
{
    libcellml::ResetPtr r = libcellml::Reset::create();
    libcellml::VariablePtr v1 = libcellml::Variable::create("vA");
    libcellml::VariablePtr tv1 = libcellml::Variable::create("vTest");

    r->setOrder(3);
    r->setResetValue("reset value math");
    r->setResetValueId("rv_id");
    r->setTestValue("test value math");
    r->setTestValueId("tv_id");
    r->setTestVariable(tv1);
    r->setVariable(v1);

    return r;
}

TEST(Equality, resetEqual)
{
    libcellml::ResetPtr r1 = createInitialisedReset();
    libcellml::ResetPtr r2 = createInitialisedReset();

    EXPECT_TRUE(r1->equals(r2));
    EXPECT_TRUE(r2->equals(r1));
}

TEST(Equality, resetNotEqualById)
{
    libcellml::ResetPtr r1 = createInitialisedReset();
    libcellml::ResetPtr r2 = createInitialisedReset();

    r2->setId("id");

    EXPECT_FALSE(r1->equals(r2));
    EXPECT_FALSE(r2->equals(r1));
}

TEST(Equality, resetNotEqualByOrder)
{
    libcellml::ResetPtr r1 = createInitialisedReset();
    libcellml::ResetPtr r2 = createInitialisedReset();

    r2->setOrder(2);

    EXPECT_FALSE(r1->equals(r2));
    EXPECT_FALSE(r2->equals(r1));
}

TEST(Equality, resetNotEqualByResetValue)
{
    libcellml::ResetPtr r1 = createInitialisedReset();
    libcellml::ResetPtr r2 = createInitialisedReset();

    r2->setResetValue("reset value different math");

    EXPECT_FALSE(r1->equals(r2));
    EXPECT_FALSE(r2->equals(r1));
}

TEST(Equality, resetNotEqualByResetValueId)
{
    libcellml::ResetPtr r1 = createInitialisedReset();
    libcellml::ResetPtr r2 = createInitialisedReset();

    r2->setResetValueId("rv_alt_id");

    EXPECT_FALSE(r1->equals(r2));
    EXPECT_FALSE(r2->equals(r1));
}

TEST(Equality, resetNotEqualByTestValue)
{
    libcellml::ResetPtr r1 = createInitialisedReset();
    libcellml::ResetPtr r2 = createInitialisedReset();

    r1->setTestValue("test value different math");

    EXPECT_FALSE(r1->equals(r2));
    EXPECT_FALSE(r2->equals(r1));
}

TEST(Equality, resetNotEqualByTestValueId)
{
    libcellml::ResetPtr r1 = createInitialisedReset();
    libcellml::ResetPtr r2 = createInitialisedReset();

    r1->setTestValueId("tv_alt_id");

    EXPECT_FALSE(r1->equals(r2));
    EXPECT_FALSE(r2->equals(r1));
}

TEST(Equality, resetNotEqualByTestVariable)
{
    libcellml::ResetPtr r1 = createInitialisedReset();
    libcellml::ResetPtr r2 = createInitialisedReset();

    libcellml::VariablePtr tv1 = libcellml::Variable::create("vTestA");
    r1->setTestVariable(tv1);

    EXPECT_FALSE(r1->equals(r2));
    EXPECT_FALSE(r2->equals(r1));
}

TEST(Equality, resetNotEqualByVariable)
{
    libcellml::ResetPtr r1 = createInitialisedReset();
    libcellml::ResetPtr r2 = createInitialisedReset();

    libcellml::VariablePtr v2 = libcellml::Variable::create("vB");
    r2->setVariable(v2);

    EXPECT_FALSE(r1->equals(r2));
    EXPECT_FALSE(r2->equals(r1));
}

TEST(Equality, resetNotEqualByNullTestVariable)
{
    libcellml::ResetPtr r1 = createInitialisedReset();
    libcellml::ResetPtr r2 = createInitialisedReset();

    r1->setTestVariable(nullptr);

    EXPECT_FALSE(r1->equals(r2));
    EXPECT_FALSE(r2->equals(r1));
}

TEST(Equality, resetNotEqualByNullVariable)
{
    libcellml::ResetPtr r1 = createInitialisedReset();
    libcellml::ResetPtr r2 = createInitialisedReset();

    r2->setVariable(nullptr);

    EXPECT_FALSE(r1->equals(r2));
    EXPECT_FALSE(r2->equals(r1));
}

TEST(Equality, componentEqual)
{
    libcellml::ComponentPtr c1 = libcellml::Component::create("c");
    libcellml::ComponentPtr c2 = libcellml::Component::create("c");

    libcellml::ComponentPtr cChild1 = libcellml::Component::create("child");
    libcellml::ComponentPtr cChild2 = libcellml::Component::create("child");

    libcellml::VariablePtr v1 = libcellml::Variable::create("vA");
    libcellml::VariablePtr v2 = libcellml::Variable::create("vA");

    libcellml::ResetPtr r1 = libcellml::Reset::create();
    libcellml::ResetPtr r2 = libcellml::Reset::create();

    r1->setOrder(5);
    r2->setOrder(5);

    c1->addComponent(cChild1);
    c2->addComponent(cChild2);

    c1->addVariable(v1);
    c2->addVariable(v2);

    c1->addReset(r1);
    c2->addReset(r2);

    EXPECT_TRUE(c1->equals(c2));
    EXPECT_TRUE(c2->equals(c1));
}

TEST(Equality, componentEqualDifferentOrderResets)
{
    libcellml::ComponentPtr c1 = libcellml::Component::create("c");
    libcellml::ComponentPtr c2 = libcellml::Component::create("c");

    libcellml::ResetPtr r1 = libcellml::Reset::create();
    libcellml::ResetPtr r2 = libcellml::Reset::create();
    libcellml::ResetPtr r3 = libcellml::Reset::create();
    libcellml::ResetPtr r4 = libcellml::Reset::create();
    libcellml::ResetPtr r5 = libcellml::Reset::create();
    libcellml::ResetPtr r6 = libcellml::Reset::create();

    r1->setOrder(5);
    r2->setOrder(3);
    r3->setOrder(7);
    r4->setOrder(5);
    r5->setOrder(3);
    r6->setOrder(7);

    c1->addReset(r1);
    c1->addReset(r2);
    c1->addReset(r3);

    c2->addReset(r4);
    c2->addReset(r6);
    c2->addReset(r5);

    EXPECT_TRUE(c1->equals(c2));
    EXPECT_TRUE(c2->equals(c1));
}

TEST(Equality, componentEqualDifferentOrderVariables)
{
    libcellml::ComponentPtr c1 = libcellml::Component::create("c");
    libcellml::ComponentPtr c2 = libcellml::Component::create("c");

    libcellml::VariablePtr v1 = libcellml::Variable::create("vA");
    libcellml::VariablePtr v2 = libcellml::Variable::create("vB");
    libcellml::VariablePtr v3 = libcellml::Variable::create("vC");
    libcellml::VariablePtr v4 = libcellml::Variable::create("vA");
    libcellml::VariablePtr v5 = libcellml::Variable::create("vB");
    libcellml::VariablePtr v6 = libcellml::Variable::create("vC");

    c1->addVariable(v1);
    c1->addVariable(v2);
    c1->addVariable(v3);

    c2->addVariable(v6);
    c2->addVariable(v4);
    c2->addVariable(v5);

    EXPECT_TRUE(c1->equals(c2));
    EXPECT_TRUE(c2->equals(c1));
}

TEST(Equality, componentEqualDifferentOrderComponents)
{
    libcellml::ComponentPtr c1 = libcellml::Component::create("c");
    libcellml::ComponentPtr c2 = libcellml::Component::create("c");

    libcellml::ComponentPtr cChild1 = libcellml::Component::create("child1");
    libcellml::ComponentPtr cChild2 = libcellml::Component::create("child2");
    libcellml::ComponentPtr cChild3 = libcellml::Component::create("child3");
    libcellml::ComponentPtr cChild4 = libcellml::Component::create("child1");
    libcellml::ComponentPtr cChild5 = libcellml::Component::create("child2");
    libcellml::ComponentPtr cChild6 = libcellml::Component::create("child3");

    c1->addComponent(cChild1);
    c1->addComponent(cChild2);
    c1->addComponent(cChild3);

    c2->addComponent(cChild5);
    c2->addComponent(cChild6);
    c2->addComponent(cChild4);

    EXPECT_TRUE(c1->equals(c2));
    EXPECT_TRUE(c2->equals(c1));
}

TEST(Equality, componentEqualWithComponentHierarchy)
{
    libcellml::ComponentPtr c1 = libcellml::Component::create("c");
    libcellml::ComponentPtr c2 = libcellml::Component::create("c");

    libcellml::ComponentPtr cChild1 = libcellml::Component::create("child1");
    libcellml::ComponentPtr cChild2 = libcellml::Component::create("child1");

    libcellml::ComponentPtr cChild11 = libcellml::Component::create("child11");
    libcellml::ComponentPtr cChild21 = libcellml::Component::create("child11");

    c1->addComponent(cChild1);
    c2->addComponent(cChild2);

    cChild1->addComponent(cChild11);
    cChild2->addComponent(cChild21);

    EXPECT_TRUE(c1->equals(c2));
    EXPECT_TRUE(c2->equals(c1));
}

TEST(Equality, componentNotEqualByReset)
{
    libcellml::ComponentPtr c1 = libcellml::Component::create("c");
    libcellml::ComponentPtr c2 = libcellml::Component::create("c");

    libcellml::ResetPtr r1 = libcellml::Reset::create();
    libcellml::ResetPtr r2 = libcellml::Reset::create();

    r1->setOrder(5);
    r2->setOrder(7);

    c1->addReset(r1);
    c2->addReset(r2);

    EXPECT_FALSE(c1->equals(c2));
    EXPECT_FALSE(c2->equals(c1));
}

TEST(Equality, componentNotEqualByComponent)
{
    libcellml::ComponentPtr c1 = libcellml::Component::create("c");
    libcellml::ComponentPtr c2 = libcellml::Component::create("c");

    libcellml::ComponentPtr cChild1 = libcellml::Component::create("child1");
    libcellml::ComponentPtr cChild2 = libcellml::Component::create("child2");

    c1->addComponent(cChild1);
    c2->addComponent(cChild2);

    EXPECT_FALSE(c1->equals(c2));
    EXPECT_FALSE(c2->equals(c1));
}

TEST(Equality, componentNotEqualByNumberOfComponents)
{
    libcellml::ComponentPtr c1 = libcellml::Component::create("c");
    libcellml::ComponentPtr c2 = libcellml::Component::create("c");

    libcellml::ComponentPtr cChild1 = libcellml::Component::create("child1");

    c1->addComponent(cChild1);

    EXPECT_FALSE(c1->equals(c2));
    EXPECT_FALSE(c2->equals(c1));
}

TEST(Equality, componentNotEqualByVariable)
{
    libcellml::ComponentPtr c1 = libcellml::Component::create("c");
    libcellml::ComponentPtr c2 = libcellml::Component::create("c");

    libcellml::VariablePtr v1 = libcellml::Variable::create("vA");
    libcellml::VariablePtr v2 = libcellml::Variable::create("vB");

    c1->addVariable(v1);
    c2->addVariable(v2);

    EXPECT_FALSE(c1->equals(c2));
    EXPECT_FALSE(c2->equals(c1));
}

TEST(Equality, componentNotEqualByMath)
{
    libcellml::ComponentPtr c1 = libcellml::Component::create("child1");
    libcellml::ComponentPtr c1Alt = libcellml::Component::create("child1");

    c1Alt->setMath("math");

    EXPECT_FALSE(c1->equals(c1Alt));
    EXPECT_FALSE(c1Alt->equals(c1));
}

TEST(Equality, componentNotEqualByEncapsulationId)
{
    libcellml::ComponentPtr c1 = libcellml::Component::create("child1");
    libcellml::ComponentPtr c1Alt = libcellml::Component::create("child1");

    c1Alt->setEncapsulationId("id1");

    EXPECT_FALSE(c1->equals(c1Alt));
    EXPECT_FALSE(c1Alt->equals(c1));
}

TEST(Equality, importSourceEqual)
{
    libcellml::ImportSourcePtr is1 = libcellml::ImportSource::create();
    libcellml::ImportSourcePtr is2 = libcellml::ImportSource::create();

    is1->setUrl("great_url");
    is2->setUrl("great_url");

    EXPECT_TRUE(is1->equals(is2));
    EXPECT_TRUE(is2->equals(is1));
}

TEST(Equality, importSourceNotEqual)
{
    libcellml::ImportSourcePtr is1 = libcellml::ImportSource::create();
    libcellml::ImportSourcePtr is2 = libcellml::ImportSource::create();

    is1->setUrl("another_great_url");
    is2->setUrl("great_url");

    EXPECT_FALSE(is1->equals(is2));
    EXPECT_FALSE(is2->equals(is1));
}

TEST(Equality, importSourceNotEqualInvalidInput)
{
    libcellml::ImportSourcePtr is1 = libcellml::ImportSource::create();

    is1->setUrl("a_great_url");

    EXPECT_FALSE(is1->equals(nullptr));
}

TEST(Equality, importSourceNotEqualImportSource)
{
    libcellml::ComponentPtr c1 = libcellml::Component::create("bob");
    libcellml::ComponentPtr c2 = libcellml::Component::create("bob");

    libcellml::ImportSourcePtr is1 = libcellml::ImportSource::create();
    is1->setUrl("a_great_url");

    c1->setImportSource(is1);

    EXPECT_FALSE(c1->equals(c2));
}

TEST(Equality, importSourceNotEqualImportSourceReference)
{
    libcellml::ComponentPtr c1 = libcellml::Component::create("bob");
    libcellml::ComponentPtr c2 = libcellml::Component::create("bob");

    libcellml::ImportSourcePtr is1 = libcellml::ImportSource::create();
    is1->setUrl("a_great_url");

    libcellml::ImportSourcePtr is2 = libcellml::ImportSource::create();
    is2->setUrl("a_better_url");

    c1->setImportSource(is1);
    c1->setImportReference("dave");

    c2->setImportSource(is2);

    EXPECT_FALSE(c1->equals(c2));
}

TEST(Equality, modelEqual)
{
    libcellml::ModelPtr m1 = libcellml::Model::create();
    libcellml::ModelPtr m2 = libcellml::Model::create();

    libcellml::ComponentPtr c1 = libcellml::Component::create("c");
    libcellml::ComponentPtr c2 = libcellml::Component::create("c");

    libcellml::UnitsPtr u1 = libcellml::Units::create("unitsA");
    libcellml::UnitsPtr u2 = libcellml::Units::create("unitsA");

    m1->addComponent(c1);
    m2->addComponent(c2);

    m1->addUnits(u1);
    m2->addUnits(u2);

    EXPECT_TRUE(m1->equals(m2));
    EXPECT_TRUE(m2->equals(m1));
}

TEST(Equality, modelNotEqualInvalidInput)
{
    libcellml::ModelPtr m1 = libcellml::Model::create();

    EXPECT_FALSE(m1->equals(nullptr));
}

TEST(Equality, modelNotEqualByComponents)
{
    libcellml::ModelPtr m1 = libcellml::Model::create();
    libcellml::ModelPtr m2 = libcellml::Model::create();

    libcellml::ComponentPtr c1 = libcellml::Component::create("c1");
    libcellml::ComponentPtr c2 = libcellml::Component::create("c2");

    m1->addComponent(c1);
    m2->addComponent(c2);

    EXPECT_FALSE(m1->equals(m2));
    EXPECT_FALSE(m2->equals(m1));
}

TEST(Equality, modelNotEqualByUnits)
{
    libcellml::ModelPtr m1 = libcellml::Model::create();
    libcellml::ModelPtr m2 = libcellml::Model::create();

    libcellml::UnitsPtr u1 = libcellml::Units::create("unitsA");
    libcellml::UnitsPtr u2 = libcellml::Units::create("unitsB");

    m1->addUnits(u1);
    m2->addUnits(u2);

    EXPECT_FALSE(m1->equals(m2));
    EXPECT_FALSE(m2->equals(m1));
}

TEST(Equality, modelHH)
{
    libcellml::ParserPtr parser = libcellml::Parser::create();
    libcellml::ModelPtr model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto clonedModel = model->clone();

    EXPECT_TRUE(model->equals(clonedModel));
    EXPECT_TRUE(clonedModel->equals(model));
}

TEST(Equality, modelHHWithDifference)
{
    libcellml::ParserPtr parser = libcellml::Parser::create();
    libcellml::ModelPtr model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto clonedModel = model->clone();

    auto v = clonedModel->component(3)->component(0)->variable(3);
    v->setUnits("millisecond");

    EXPECT_FALSE(model->equals(clonedModel));
    EXPECT_FALSE(clonedModel->equals(model));
}

TEST(Equality, parseMath)
{
    libcellml::ParserPtr parser = libcellml::Parser::create();
    libcellml::ModelPtr model = parser->parseModel(fileContents("deriv_approx_sin.xml"));

    libcellml::ComponentPtr component = libcellml::Component::create("sin");

    libcellml::VariablePtr x = libcellml::Variable::create("x");
    x->setUnits("dimensionless");
    x->setInterfaceType(libcellml::Variable::InterfaceType::PUBLIC_AND_PRIVATE);

    libcellml::VariablePtr sin_initial_value = libcellml::Variable::create("sin_initial_value");
    sin_initial_value->setUnits("dimensionless");
    sin_initial_value->setInterfaceType(libcellml::Variable::InterfaceType::PUBLIC_AND_PRIVATE);

    libcellml::VariablePtr sin = libcellml::Variable::create("sin");
    sin->setUnits("dimensionless");
    sin->setInterfaceType(libcellml::Variable::InterfaceType::PUBLIC_AND_PRIVATE);
    sin->setInitialValue(sin_initial_value);

    component->setMath(
        "<math xmlns=\"http://www.w3.org/1998/Math/MathML\">\n"
        "      <apply><eq/>\n"
        "        <apply><diff/>\n"
        "          <bvar><ci>x</ci></bvar>\n"
        "          <ci>sin</ci>\n"
        "        </apply>\n"
        "        <apply><cos/>\n"
        "          <ci>x</ci>\n"
        "        </apply>\n"
        "      </apply>\n"
        "    </math>\n");

    EXPECT_TRUE(x->equals(model->component(0)->variable("x")));
    EXPECT_TRUE(sin_initial_value->equals(model->component(0)->variable("sin_initial_value")));
    EXPECT_TRUE(sin->equals(model->component(0)->variable("sin")));

    EXPECT_TRUE(component->equals(model->component(0)));
}

TEST(Equality, clone)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("modelflattening/outsidecomponentequivalentvariables.xml"));
    auto clonedModel = model->clone();
    auto importer = libcellml::Importer::create();
    importer->resolveImports(model, resourcePath("modelflattening/"));
    model = importer->flattenModel(model);

    importer->resolveImports(clonedModel, resourcePath("modelflattening/"));
    clonedModel = importer->flattenModel(clonedModel);

    EXPECT_TRUE(model->equals(clonedModel));
    EXPECT_TRUE(clonedModel->equals(model));
}

TEST(Equality, namedEntityNotEqualNonNamedEntity)
{
    libcellml::VariablePtr v1 = libcellml::Variable::create("variableA");
    libcellml::ImportSourcePtr is1 = libcellml::ImportSource::create();
    libcellml::ResetPtr r1 = libcellml::Reset::create();

    EXPECT_FALSE(v1->equals(is1));
    EXPECT_FALSE(is1->equals(v1));

    EXPECT_FALSE(v1->equals(r1));
    EXPECT_FALSE(r1->equals(v1));
}

TEST(Equality, modelNotEqualComponent)
{
    libcellml::ModelPtr m = libcellml::Model::create("name");
    libcellml::ComponentPtr c = libcellml::Component::create("name");

    EXPECT_FALSE(m->equals(c));
    EXPECT_FALSE(c->equals(m));
}

TEST(Equality, componentNotEqualVariable)
{
    libcellml::ComponentPtr c = libcellml::Component::create("name");
    libcellml::VariablePtr v = libcellml::Variable::create("name");

    EXPECT_FALSE(c->equals(v));
    EXPECT_FALSE(v->equals(c));
}

TEST(Equality, unitsNotEqualImportedUnits)
{
    libcellml::UnitsPtr u1 = libcellml::Units::create("name");
    libcellml::UnitsPtr u2 = libcellml::Units::create("name");

    libcellml::ImportSourcePtr iS = libcellml::ImportSource::create();

    u1->setImportSource(iS);

    EXPECT_FALSE(u1->equals(u2));
    EXPECT_FALSE(u2->equals(u1));
}
