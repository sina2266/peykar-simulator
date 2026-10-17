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

TEST(Generator, isolatedFirstOrderModel)
{
    // This test resulted from https://github.com/cellml/libcellml/issues/432

    //  1.a   Create the model instance.
    libcellml::ModelPtr model = libcellml::Model::create();
    model->setName("Tutorial4_FirstOrderModel");

    //  1.b   Create a component and add it into the model.
    libcellml::ComponentPtr component = libcellml::Component::create();
    component->setName("IonChannel");
    model->addComponent(component);

    //  2.a   Define the mathematics.
    const std::string mathHeader = "<math xmlns=\"http://www.w3.org/1998/Math/MathML\" xmlns:cellml=\"http://www.cellml.org/cellml/2.0#\">";

    // dy/dt = alpha_y*(1-y) - beta_y*y
    const std::string equation1 =
        "<apply>\n"
        "  <eq/>\n"
        "  <apply>\n"
        "    <diff/>\n"
        "    <bvar>\n"
        "      <ci>t</ci>\n"
        "    </bvar>\n"
        "    <ci>y</ci>\n"
        "  </apply>\n"
        "  <apply>\n"
        "    <minus/>\n"
        "    <apply>\n"
        "      <times/>\n"
        "      <ci>alpha_y</ci>\n"
        "      <apply>\n"
        "        <minus/>\n"
        "        <cn cellml:units=\"dimensionless\">1</cn>\n"
        "        <ci>y</ci>\n"
        "      </apply>\n"
        "    </apply>\n"
        "    <apply>\n"
        "      <times/>\n"
        "      <ci>beta_y</ci>\n"
        "      <ci>y</ci>\n"
        "    </apply>\n"
        "  </apply>\n"
        "</apply>\n";
    // i_y = g_y*power(y,gamma)*(V-E_y)
    const std::string equation2 =
        "<apply>\n"
        "  <eq/>\n"
        "  <ci>i_y</ci>\n"
        "  <apply>\n"
        "    <times/>\n"
        "    <ci>g_y</ci>\n"
        "    <apply>\n"
        "      <minus/>\n"
        "      <ci>V</ci>\n"
        "      <ci>E_y</ci>\n"
        "    </apply>\n"
        "    <apply>\n"
        "      <power/>\n"
        "      <ci>y</ci>\n"
        "      <ci>gamma</ci>\n"
        "    </apply>\n"
        "  </apply>\n"
        "</apply>";
    const std::string mathFooter = "</math>";

    //  2.b   Add the maths to the component. Note that there is only one maths
    //        string stored, so parts which are appended must create a viable
    //        MathML string when concatenated. To clear any string, which is
    //        already stored, simply call setMath("") with an empty string.
    component->setMath(mathHeader);
    component->appendMath(equation1);
    component->appendMath(equation2);
    component->appendMath(mathFooter);

    //  3.a,b Declaring the variables, their names, units, and initial conditions
    //        Note that the names given to variables must be the same as that used
    //        within the <ci> blocks in the MathML string we created in step 2.a.

    libcellml::VariablePtr t = libcellml::Variable::create();
    t->setName("t");
    t->setUnits("millisecond");
    // Note: time is our integration base variable so it is not initialised.

    libcellml::VariablePtr V = libcellml::Variable::create();
    V->setName("V");
    V->setUnits("millivolt");
    V->setInitialValue(0.0);

    libcellml::VariablePtr alpha_y = libcellml::Variable::create();
    alpha_y->setName("alpha_y");
    alpha_y->setUnits("per_millisecond");
    alpha_y->setInitialValue(1.0);

    libcellml::VariablePtr beta_y = libcellml::Variable::create();
    beta_y->setName("beta_y");
    beta_y->setUnits("per_millisecond");
    beta_y->setInitialValue(2.0);

    libcellml::VariablePtr y = libcellml::Variable::create();
    y->setName("y");
    y->setUnits("dimensionless");
    y->setInitialValue(1.0);

    libcellml::VariablePtr E_y = libcellml::Variable::create();
    E_y->setName("E_y");
    E_y->setUnits("millivolt");
    E_y->setInitialValue(-85.0);

    libcellml::VariablePtr i_y = libcellml::Variable::create();
    i_y->setName("i_y");
    i_y->setUnits("microA_per_cm2");
    // Note that no initial value is needed for this variable as its value
    // is defined by equation2.

    libcellml::VariablePtr g_y = libcellml::Variable::create();
    g_y->setName("g_y");
    g_y->setUnits("milliS_per_cm2");
    g_y->setInitialValue(36.0);

    libcellml::VariablePtr gamma = libcellml::Variable::create();
    gamma->setName("gamma");
    gamma->setUnits("dimensionless");
    gamma->setInitialValue(4.0);

    //  3.c   Adding the variables to the component. Note that Variables are
    //        added by their pointer (cf. their name).
    component->addVariable(t);
    component->addVariable(V);
    component->addVariable(E_y);
    component->addVariable(gamma);
    component->addVariable(i_y);
    component->addVariable(g_y);
    component->addVariable(alpha_y);
    component->addVariable(beta_y);
    component->addVariable(y);

    //  4.a   Defining the units of millisecond, millivolt, per_millisecond,
    //        microA_per_cm2, and milliS_per_cm2. Note that the dimensionless
    //        units are part of those built-in already, so they don't need to be
    //        defined here.
    libcellml::UnitsPtr ms = libcellml::Units::create();
    ms->setName("millisecond");
    ms->addUnit("second", "milli");

    libcellml::UnitsPtr mV = libcellml::Units::create();
    mV->setName("millivolt");
    mV->addUnit("volt", "milli");

    libcellml::UnitsPtr per_ms = libcellml::Units::create();
    per_ms->setName("per_millisecond");
    per_ms->addUnit("millisecond", -1.0);

    libcellml::UnitsPtr microA_per_cm2 = libcellml::Units::create();
    microA_per_cm2->setName("microA_per_cm2");
    microA_per_cm2->addUnit("ampere", "micro");
    microA_per_cm2->addUnit("metre", "centi", -2.0);

    libcellml::UnitsPtr mS_per_cm2 = libcellml::Units::create();
    mS_per_cm2->setName("milliS_per_cm2");
    mS_per_cm2->addUnit("siemens", "milli");
    mS_per_cm2->addUnit("metre", "centi", -2.0);

    //  4.b   Add these units into the model.
    model->addUnits(ms);
    model->addUnits(mV);
    model->addUnits(per_ms);
    model->addUnits(microA_per_cm2);
    model->addUnits(mS_per_cm2);

    // Link the units used by variables in the model to
    // units added to the model.
    model->linkUnits();

    //  4.c   Validate the final arrangement. No issues are expected at this stage.
    libcellml::ValidatorPtr validator = libcellml::Validator::create();
    validator->validateModel(model);
    EXPECT_EQ(size_t(0), validator->issueCount());

    //  5.a   Create an Analyser instance and analyse the model.

    libcellml::AnalyserPtr analyser = libcellml::Analyser::create();
    analyser->analyseModel(model);

    //  5.b   Check whether the analyser has encountered any issues.
    EXPECT_EQ(size_t(0), analyser->issueCount());
}
