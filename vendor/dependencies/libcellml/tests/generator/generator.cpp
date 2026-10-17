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

static const std::string EMPTY_STRING;

TEST(Generator, emptyModel)
{
    libcellml::ModelPtr model = libcellml::Model::create("empty_model");
    libcellml::AnalyserPtr analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    EXPECT_EQ(EMPTY_STRING, generator->interfaceCode(analyserModel));
    EXPECT_EQ(EMPTY_STRING, generator->implementationCode(analyserModel));
}

TEST(Generator, algebraicEqnComputedVarOnRhs)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/algebraic_eqn_computed_var_on_rhs/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_eqn_computed_var_on_rhs/model.h", generator->interfaceCode(analyserModel));
    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_eqn_computed_var_on_rhs/model.c", generator->implementationCode(analyserModel));

    auto profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_eqn_computed_var_on_rhs/model.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, algebraicEqnComputedVarOnRhsWithComputedConstantAsExternalVariable)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/algebraic_eqn_computed_var_on_rhs/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->addExternalVariable(libcellml::AnalyserExternalVariable::create(model->component("my_algebraic_eqn")->variable("a")));

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    auto profile = libcellml::GeneratorProfile::create();

    profile->setInterfaceFileNameString("model.external.h");

    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_eqn_computed_var_on_rhs/model.external.h", generator->interfaceCode(analyserModel, profile));
    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_eqn_computed_var_on_rhs/model.external.c", generator->implementationCode(analyserModel, profile));

    profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_eqn_computed_var_on_rhs/model.external.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, algebraicEqnConstVarOnRhs)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/algebraic_eqn_const_var_on_rhs/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_eqn_const_var_on_rhs/model.h", generator->interfaceCode(analyserModel));
    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_eqn_const_var_on_rhs/model.c", generator->implementationCode(analyserModel));

    auto profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_eqn_const_var_on_rhs/model.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, algebraicEqnConstantOnRhs)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/algebraic_eqn_constant_on_rhs/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_eqn_constant_on_rhs/model.h", generator->interfaceCode(analyserModel));
    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_eqn_constant_on_rhs/model.c", generator->implementationCode(analyserModel));

    auto profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_eqn_constant_on_rhs/model.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, algebraicEqnDerivativeOnRhs)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/algebraic_eqn_derivative_on_rhs/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_eqn_derivative_on_rhs/model.h", generator->interfaceCode(analyserModel));
    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_eqn_derivative_on_rhs/model.c", generator->implementationCode(analyserModel));

    auto profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_eqn_derivative_on_rhs/model.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, algebraicEqnDerivativeOnRhsOneComponent)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/algebraic_eqn_derivative_on_rhs_one_component/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_eqn_derivative_on_rhs_one_component/model.h", generator->interfaceCode(analyserModel));
    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_eqn_derivative_on_rhs_one_component/model.c", generator->implementationCode(analyserModel));

    auto profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_eqn_derivative_on_rhs_one_component/model.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, algebraicEqnStateVarOnRhs)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/algebraic_eqn_state_var_on_rhs/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_eqn_state_var_on_rhs/model.h", generator->interfaceCode(analyserModel));
    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_eqn_state_var_on_rhs/model.c", generator->implementationCode(analyserModel));

    auto profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_eqn_state_var_on_rhs/model.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, algebraicEqnStateVarOnRhsOneComponent)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/algebraic_eqn_state_var_on_rhs_one_component/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_eqn_state_var_on_rhs_one_component/model.h", generator->interfaceCode(analyserModel));
    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_eqn_state_var_on_rhs_one_component/model.c", generator->implementationCode(analyserModel));

    auto profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_eqn_state_var_on_rhs_one_component/model.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, algebraicUnknownVarOnRhs)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/algebraic_unknown_var_on_rhs/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_unknown_var_on_rhs/model.h", generator->interfaceCode(analyserModel));
    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_unknown_var_on_rhs/model.c", generator->implementationCode(analyserModel));

    auto profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_unknown_var_on_rhs/model.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, algebraicEqnWithOneNonIsolatedUnknown)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/algebraic_eqn_with_one_non_isolated_unknown/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_eqn_with_one_non_isolated_unknown/model.h", generator->interfaceCode(analyserModel));
    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_eqn_with_one_non_isolated_unknown/model.c", generator->implementationCode(analyserModel));

    auto profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_eqn_with_one_non_isolated_unknown/model.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, algebraicEqnWithOneNonIsolatedUnknownWithExternalVariable)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/algebraic_eqn_with_one_non_isolated_unknown/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->addExternalVariable(libcellml::AnalyserExternalVariable::create(model->component("my_algebraic_eqn")->variable("c")));

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    auto profile = libcellml::GeneratorProfile::create();

    profile->setInterfaceFileNameString("model.external.h");

    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_eqn_with_one_non_isolated_unknown/model.external.h", generator->interfaceCode(analyserModel, profile));
    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_eqn_with_one_non_isolated_unknown/model.external.c", generator->implementationCode(analyserModel, profile));

    profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_eqn_with_one_non_isolated_unknown/model.external.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, algebraicSystemWithThreeLinkedUnknowns)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/algebraic_system_with_three_linked_unknowns/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_system_with_three_linked_unknowns/model.h", generator->interfaceCode(analyserModel));
    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_system_with_three_linked_unknowns/model.c", generator->implementationCode(analyserModel));

    auto profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_system_with_three_linked_unknowns/model.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, algebraicSystemWithThreeLinkedUnknownsWithThreeExternalVariables)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/algebraic_system_with_three_linked_unknowns/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->addExternalVariable(libcellml::AnalyserExternalVariable::create(model->component("my_algebraic_system")->variable("x")));
    analyser->addExternalVariable(libcellml::AnalyserExternalVariable::create(model->component("my_algebraic_system")->variable("y")));
    analyser->addExternalVariable(libcellml::AnalyserExternalVariable::create(model->component("my_algebraic_system")->variable("z")));

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    auto profile = libcellml::GeneratorProfile::create();

    profile->setInterfaceFileNameString("model.external.h");

    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_system_with_three_linked_unknowns/model.external.h", generator->interfaceCode(analyserModel, profile));
    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_system_with_three_linked_unknowns/model.external.c", generator->implementationCode(analyserModel, profile));

    profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_system_with_three_linked_unknowns/model.external.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, algebraicSystemWithVariousDependenciesOrdered)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/algebraic_system_with_various_dependencies/model.ordered.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    auto profile = libcellml::GeneratorProfile::create();

    profile->setInterfaceFileNameString("model.ordered.h");

    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_system_with_various_dependencies/model.ordered.h", generator->interfaceCode(analyserModel, profile));
    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_system_with_various_dependencies/model.ordered.c", generator->implementationCode(analyserModel, profile));

    profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_system_with_various_dependencies/model.ordered.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, algebraicSystemWithVariousDependenciesNotOrdered)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/algebraic_system_with_various_dependencies/model.not.ordered.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    auto profile = libcellml::GeneratorProfile::create();

    profile->setInterfaceFileNameString("model.not.ordered.h");

    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_system_with_various_dependencies/model.not.ordered.h", generator->interfaceCode(analyserModel, profile));
    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_system_with_various_dependencies/model.not.ordered.c", generator->implementationCode(analyserModel, profile));

    profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_system_with_various_dependencies/model.not.ordered.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, odeComputedVarOnRhs)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/ode_computed_var_on_rhs/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    EXPECT_EQ_FILE_CONTENTS("generator/ode_computed_var_on_rhs/model.h", generator->interfaceCode(analyserModel));
    EXPECT_EQ_FILE_CONTENTS("generator/ode_computed_var_on_rhs/model.c", generator->implementationCode(analyserModel));

    auto profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/ode_computed_var_on_rhs/model.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, odeComputedVarOnRhsOneComponent)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/ode_computed_var_on_rhs_one_component/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    EXPECT_EQ_FILE_CONTENTS("generator/ode_computed_var_on_rhs_one_component/model.h", generator->interfaceCode(analyserModel));
    EXPECT_EQ_FILE_CONTENTS("generator/ode_computed_var_on_rhs_one_component/model.c", generator->implementationCode(analyserModel));

    auto profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/ode_computed_var_on_rhs_one_component/model.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, odeConstVarOnRhs)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/ode_const_var_on_rhs/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    EXPECT_EQ_FILE_CONTENTS("generator/ode_const_var_on_rhs/model.h", generator->interfaceCode(analyserModel));
    EXPECT_EQ_FILE_CONTENTS("generator/ode_const_var_on_rhs/model.c", generator->implementationCode(analyserModel));

    auto profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/ode_const_var_on_rhs/model.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, odeConstVarOnRhsOneComponent)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/ode_const_var_on_rhs_one_component/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    EXPECT_EQ_FILE_CONTENTS("generator/ode_const_var_on_rhs_one_component/model.h", generator->interfaceCode(analyserModel));
    EXPECT_EQ_FILE_CONTENTS("generator/ode_const_var_on_rhs_one_component/model.c", generator->implementationCode(analyserModel));

    auto profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/ode_const_var_on_rhs_one_component/model.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, odeConstantOnRhs)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/ode_constant_on_rhs/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    EXPECT_EQ_FILE_CONTENTS("generator/ode_constant_on_rhs/model.h", generator->interfaceCode(analyserModel));
    EXPECT_EQ_FILE_CONTENTS("generator/ode_constant_on_rhs/model.c", generator->implementationCode(analyserModel));

    auto profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/ode_constant_on_rhs/model.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, odeConstantOnRhsOneComponent)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/ode_constant_on_rhs_one_component/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    EXPECT_EQ_FILE_CONTENTS("generator/ode_constant_on_rhs_one_component/model.h", generator->interfaceCode(analyserModel));
    EXPECT_EQ_FILE_CONTENTS("generator/ode_constant_on_rhs_one_component/model.c", generator->implementationCode(analyserModel));

    auto profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/ode_constant_on_rhs_one_component/model.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, odeMultipleDependentOdes)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/ode_multiple_dependent_odes/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    EXPECT_EQ_FILE_CONTENTS("generator/ode_multiple_dependent_odes/model.h", generator->interfaceCode(analyserModel));
    EXPECT_EQ_FILE_CONTENTS("generator/ode_multiple_dependent_odes/model.c", generator->implementationCode(analyserModel));

    auto profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/ode_multiple_dependent_odes/model.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, odeMultipleDependentOdesOneComponent)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/ode_multiple_dependent_odes_one_component/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    EXPECT_EQ_FILE_CONTENTS("generator/ode_multiple_dependent_odes_one_component/model.h", generator->interfaceCode(analyserModel));
    EXPECT_EQ_FILE_CONTENTS("generator/ode_multiple_dependent_odes_one_component/model.c", generator->implementationCode(analyserModel));

    auto profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/ode_multiple_dependent_odes_one_component/model.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, odeMultipleOdesWithSameName)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/ode_multiple_odes_with_same_name/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    EXPECT_EQ_FILE_CONTENTS("generator/ode_multiple_odes_with_same_name/model.h", generator->interfaceCode(analyserModel));
    EXPECT_EQ_FILE_CONTENTS("generator/ode_multiple_odes_with_same_name/model.c", generator->implementationCode(analyserModel));

    auto profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/ode_multiple_odes_with_same_name/model.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, odeUnknownVarOnRhs)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/ode_unknown_var_on_rhs/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    EXPECT_EQ_FILE_CONTENTS("generator/ode_unknown_var_on_rhs/model.h", generator->interfaceCode(analyserModel));
    EXPECT_EQ_FILE_CONTENTS("generator/ode_unknown_var_on_rhs/model.c", generator->implementationCode(analyserModel));

    auto profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/ode_unknown_var_on_rhs/model.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, cellmlMappingsAndEncapsulations)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/cellml_mappings_and_encapsulations/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    EXPECT_EQ_FILE_CONTENTS("generator/cellml_mappings_and_encapsulations/model.h", generator->interfaceCode(analyserModel));
    EXPECT_EQ_FILE_CONTENTS("generator/cellml_mappings_and_encapsulations/model.c", generator->implementationCode(analyserModel));

    auto profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/cellml_mappings_and_encapsulations/model.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, cellmlStateInitialisedUsingVariable)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/cellml_state_initialised_using_variable/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    EXPECT_EQ_FILE_CONTENTS("generator/cellml_state_initialised_using_variable/model.h", generator->interfaceCode(analyserModel));
    EXPECT_EQ_FILE_CONTENTS("generator/cellml_state_initialised_using_variable/model.c", generator->implementationCode(analyserModel));

    auto profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/cellml_state_initialised_using_variable/model.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, cellmlUnitScalingVoiIndirect)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/cellml_unit_scaling_voi_indirect/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    EXPECT_EQ_FILE_CONTENTS("generator/cellml_unit_scaling_voi_indirect/model.h", generator->interfaceCode(analyserModel));
    EXPECT_EQ_FILE_CONTENTS("generator/cellml_unit_scaling_voi_indirect/model.c", generator->implementationCode(analyserModel));

    auto profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/cellml_unit_scaling_voi_indirect/model.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, cellmlUnitScalingVoiDirect)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/cellml_unit_scaling_voi_direct/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    EXPECT_EQ_FILE_CONTENTS("generator/cellml_unit_scaling_voi_direct/model.h", generator->interfaceCode(analyserModel));
    EXPECT_EQ_FILE_CONTENTS("generator/cellml_unit_scaling_voi_direct/model.c", generator->implementationCode(analyserModel));

    auto profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/cellml_unit_scaling_voi_direct/model.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, cellmlUnitScalingConstant)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/cellml_unit_scaling_constant/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    const std::vector<std::string> expectedIssues = {
        "Variable 'c_M' in component 'constants' is initialised using variable 'c' which has different units.",
        "Variable 'oc_mM' in component 'other_constants' is initialised using variable 'c' which has different units.",
    };

    analyser->analyseModel(model);

    EXPECT_EQ_ISSUES(expectedIssues, analyser);

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    EXPECT_EQ_FILE_CONTENTS("generator/cellml_unit_scaling_constant/model.h", generator->interfaceCode(analyserModel));
    EXPECT_EQ_FILE_CONTENTS("generator/cellml_unit_scaling_constant/model.c", generator->implementationCode(analyserModel));

    auto profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/cellml_unit_scaling_constant/model.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, cellmlUnitScalingState)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/cellml_unit_scaling_state/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    EXPECT_EQ_FILE_CONTENTS("generator/cellml_unit_scaling_state/model.h", generator->interfaceCode(analyserModel));
    EXPECT_EQ_FILE_CONTENTS("generator/cellml_unit_scaling_state/model.c", generator->implementationCode(analyserModel));

    auto profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/cellml_unit_scaling_state/model.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, cellmlUnitScalingStateInitialisedUsingConstant)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/cellml_unit_scaling_state_initialised_using_constant/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    EXPECT_EQ_FILE_CONTENTS("generator/cellml_unit_scaling_state_initialised_using_constant/model.h", generator->interfaceCode(analyserModel));
    EXPECT_EQ_FILE_CONTENTS("generator/cellml_unit_scaling_state_initialised_using_constant/model.c", generator->implementationCode(analyserModel));

    auto profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/cellml_unit_scaling_state_initialised_using_constant/model.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, cellmlUnitScalingStateInitialisedUsingVariable)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/cellml_unit_scaling_state_initialised_using_variable/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    EXPECT_EQ_FILE_CONTENTS("generator/cellml_unit_scaling_state_initialised_using_variable/model.h", generator->interfaceCode(analyserModel));
    EXPECT_EQ_FILE_CONTENTS("generator/cellml_unit_scaling_state_initialised_using_variable/model.c", generator->implementationCode(analyserModel));

    auto profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/cellml_unit_scaling_state_initialised_using_variable/model.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, cellmlUnitScalingRate)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/cellml_unit_scaling_rate/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    EXPECT_EQ_FILE_CONTENTS("generator/cellml_unit_scaling_rate/model.h", generator->interfaceCode(analyserModel));
    EXPECT_EQ_FILE_CONTENTS("generator/cellml_unit_scaling_rate/model.c", generator->implementationCode(analyserModel));

    auto profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/cellml_unit_scaling_rate/model.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, dependentEqns)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/dependent_eqns/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    EXPECT_EQ_FILE_CONTENTS("generator/dependent_eqns/model.h", generator->interfaceCode(analyserModel));
    EXPECT_EQ_FILE_CONTENTS("generator/dependent_eqns/model.c", generator->implementationCode(analyserModel));

    auto profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/dependent_eqns/model.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, cellGeometryModel)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/cell_geometry_model/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    EXPECT_EQ_FILE_CONTENTS("generator/cell_geometry_model/model.h", generator->interfaceCode(analyserModel));
    EXPECT_EQ_FILE_CONTENTS("generator/cell_geometry_model/model.c", generator->implementationCode(analyserModel));

    auto profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/cell_geometry_model/model.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, cellGeometryModelWithSomeConstantsAsExternalVariables)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/cell_geometry_model/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->addExternalVariable(libcellml::AnalyserExternalVariable::create(model->component("cell_geometry")->variable("L")));
    analyser->addExternalVariable(libcellml::AnalyserExternalVariable::create(model->component("cell_geometry")->variable("rad")));

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    auto profile = libcellml::GeneratorProfile::create();

    profile->setInterfaceFileNameString("model.external.h");

    EXPECT_EQ_FILE_CONTENTS("generator/cell_geometry_model/model.external.h", generator->interfaceCode(analyserModel, profile));
    EXPECT_EQ_FILE_CONTENTS("generator/cell_geometry_model/model.external.c", generator->implementationCode(analyserModel, profile));

    profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/cell_geometry_model/model.external.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, fabbriFantiniWildersSeveriHumanSanModel2017)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/fabbri_fantini_wilders_severi_human_san_model_2017/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    EXPECT_EQ_FILE_CONTENTS("generator/fabbri_fantini_wilders_severi_human_san_model_2017/model.h", generator->interfaceCode(analyserModel));
    EXPECT_EQ_FILE_CONTENTS("generator/fabbri_fantini_wilders_severi_human_san_model_2017/model.c", generator->implementationCode(analyserModel));

    auto profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/fabbri_fantini_wilders_severi_human_san_model_2017/model.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, garnyKohlHunterBoyettNobleRabbitSanModel2003)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/garny_kohl_hunter_boyett_noble_rabbit_san_model_2003/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    EXPECT_EQ_FILE_CONTENTS("generator/garny_kohl_hunter_boyett_noble_rabbit_san_model_2003/model.h", generator->interfaceCode(analyserModel));
    EXPECT_EQ_FILE_CONTENTS("generator/garny_kohl_hunter_boyett_noble_rabbit_san_model_2003/model.c", generator->implementationCode(analyserModel));

    auto profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/garny_kohl_hunter_boyett_noble_rabbit_san_model_2003/model.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, hodgkinHuxleySquidAxonModel1952)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/model.h", generator->interfaceCode(analyserModel));
    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/model.c", generator->implementationCode(analyserModel));

    auto profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/model.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, hodgkinHuxleySquidAxonModel1952UnknownVarsOnRhs)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model_unknown_vars_on_rhs.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/model.h", generator->interfaceCode(analyserModel));
    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/model.c", generator->implementationCode(analyserModel));

    auto profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/model.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, hodgkinHuxleySquidAxonModel1952WithStateVariableAsExternalVariable)
{
    // Generate some code for the HH52 model with sodium_channel.m (i.e. not a
    // primary variable) as an external variable.

    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->addExternalVariable(libcellml::AnalyserExternalVariable::create(model->component("sodium_channel")->variable("m")));

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    auto profile = libcellml::GeneratorProfile::create();

    profile->setInterfaceFileNameString("model.state.h");

    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/model.state.h", generator->interfaceCode(analyserModel, profile));
    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/model.state.c", generator->implementationCode(analyserModel, profile));

    profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/model.state.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, hodgkinHuxleySquidAxonModel1952WithStateVariablesAsExternalVariablesIncludingOneDependingOnTheOther)
{
    // Generate some code for the HH52 model with sodium_channel.m (i.e. not a
    // primary variable) as an external variable which has a dependency on
    // sodium_channel.V (i.e. not a primary variable), another state marked as
    // an external variable.

    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();
    auto sodium_channel_V = model->component("sodium_channel")->variable("V");
    auto external_sodium_channel_m = libcellml::AnalyserExternalVariable::create(model->component("sodium_channel")->variable("m"));

    external_sodium_channel_m->addDependency(sodium_channel_V);

    analyser->addExternalVariable(libcellml::AnalyserExternalVariable::create(sodium_channel_V));
    analyser->addExternalVariable(external_sodium_channel_m);

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    auto profile = libcellml::GeneratorProfile::create();

    profile->setInterfaceFileNameString("model.dependent.state.h");

    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/model.dependent.state.h", generator->interfaceCode(analyserModel, profile));
    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/model.dependent.state.c", generator->implementationCode(analyserModel, profile));

    profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/model.dependent.state.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, hodgkinHuxleySquidAxonModel1952WithConstantAsExternalVariable)
{
    // Generate some code for the HH52 model with membrane.Cm as an external
    // variable.

    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->addExternalVariable(libcellml::AnalyserExternalVariable::create(model->component("membrane")->variable("Cm")));

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    auto profile = libcellml::GeneratorProfile::create();

    profile->setInterfaceFileNameString("model.constant.h");

    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/model.constant.h", generator->interfaceCode(analyserModel, profile));
    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/model.constant.c", generator->implementationCode(analyserModel, profile));

    profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/model.constant.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, hodgkinHuxleySquidAxonModel1952WithConstantsAsExternalVariablesIncludingOneDependingOnTheOther)
{
    // Generate some code for the HH52 model with membrane.Cm as an external
    // variable which has a dependency on sodium_channel.g_Na, another constant
    // marked as an external variable.

    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();
    auto sodium_channel_g_Na = model->component("sodium_channel")->variable("g_Na");
    auto external_sodium_channel_Cm = libcellml::AnalyserExternalVariable::create(model->component("membrane")->variable("Cm"));

    external_sodium_channel_Cm->addDependency(sodium_channel_g_Na);

    analyser->addExternalVariable(libcellml::AnalyserExternalVariable::create(sodium_channel_g_Na));
    analyser->addExternalVariable(external_sodium_channel_Cm);

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    auto profile = libcellml::GeneratorProfile::create();

    profile->setInterfaceFileNameString("model.dependent.constant.h");

    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/model.dependent.constant.h", generator->interfaceCode(analyserModel, profile));
    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/model.dependent.constant.c", generator->implementationCode(analyserModel, profile));

    profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/model.dependent.constant.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, hodgkinHuxleySquidAxonModel1952WithComputedConstantAsExternalVariable)
{
    // Generate some code for the HH52 model with leakage_current.E_L as an
    // external variable.

    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->addExternalVariable(libcellml::AnalyserExternalVariable::create(model->component("leakage_current")->variable("E_L")));

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    auto profile = libcellml::GeneratorProfile::create();

    profile->setInterfaceFileNameString("model.computed.constant.h");

    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/model.computed.constant.h", generator->interfaceCode(analyserModel, profile));
    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/model.computed.constant.c", generator->implementationCode(analyserModel, profile));

    profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/model.computed.constant.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, hodgkinHuxleySquidAxonModel1952WithComputedConstantsAsExternalVariablesIncludingOneDependingOnTheOther)
{
    // Generate some code for the HH52 model with leakage_current.E_R as an
    // external variable which has a dependency on potassium_channel.E_K,
    // another computed constant marked as an external variable.

    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();
    auto potassium_channel_E_K = model->component("potassium_channel")->variable("E_K");
    auto external_leakage_current_E_R = libcellml::AnalyserExternalVariable::create(model->component("membrane")->variable("E_R"));

    external_leakage_current_E_R->addDependency(potassium_channel_E_K);

    analyser->addExternalVariable(libcellml::AnalyserExternalVariable::create(potassium_channel_E_K));
    analyser->addExternalVariable(external_leakage_current_E_R);

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    auto profile = libcellml::GeneratorProfile::create();

    profile->setInterfaceFileNameString("model.dependent.computed.constant.h");

    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/model.dependent.computed.constant.h", generator->interfaceCode(analyserModel, profile));
    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/model.dependent.computed.constant.c", generator->implementationCode(analyserModel, profile));

    profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/model.dependent.computed.constant.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, hodgkinHuxleySquidAxonModel1952WithAlgebraicVariableAsExternalVariable)
{
    // Generate some code for the HH52 model with membrane.i_Stim as an external
    // variable.

    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->addExternalVariable(libcellml::AnalyserExternalVariable::create(model->component("membrane")->variable("i_Stim")));

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    auto profile = libcellml::GeneratorProfile::create();

    profile->setInterfaceFileNameString("model.algebraic.h");

    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/model.algebraic.h", generator->interfaceCode(analyserModel, profile));
    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/model.algebraic.c", generator->implementationCode(analyserModel, profile));

    profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/model.algebraic.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, hodgkinHuxleySquidAxonModel1952WithAlgebraicVariablesAsExternalVariablesIncludingOneDependingOnTheOther)
{
    // Generate some code for the HH52 model with membrane.i_Stim as an external
    // variable which has a dependency on potassium_channel_n_gate.beta_n,
    // another algebraic variable marked as an external variable.

    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();
    auto potassium_channel_n_gate_beta_n = model->component("potassium_channel_n_gate")->variable("beta_n");
    auto membrane_i_Stim = libcellml::AnalyserExternalVariable::create(model->component("membrane")->variable("i_Stim"));

    membrane_i_Stim->addDependency(potassium_channel_n_gate_beta_n);

    analyser->addExternalVariable(libcellml::AnalyserExternalVariable::create(potassium_channel_n_gate_beta_n));
    analyser->addExternalVariable(membrane_i_Stim);

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    auto profile = libcellml::GeneratorProfile::create();

    profile->setInterfaceFileNameString("model.dependent.algebraic.h");

    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/model.dependent.algebraic.h", generator->interfaceCode(analyserModel, profile));
    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/model.dependent.algebraic.c", generator->implementationCode(analyserModel, profile));

    profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/model.dependent.algebraic.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, hodgkinHuxleySquidAxonModel1952WithVariousExternalVariables)
{
    // Generate some code for the HH52 model with sodium_channel.i_Na as an
    // external variable which has a dependency on
    // potassium_channel_n_gate.alpha_n, another external variable (which is
    // going to be declared after i_Na, so that it ends up with a higher index,
    // meaning that it would normally be computed after i_Na if there was no
    // such a dependency), and on sodium_channel_h_gate.h, a state variable, so
    // that i_Na ends up being computed both in the computeRates() and
    // computeVariables() methods.

    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();
    auto potassium_channel_n_gate_alpha_n = model->component("potassium_channel_n_gate")->variable("alpha_n");
    auto external_sodium_channel_i_Na = libcellml::AnalyserExternalVariable::create(model->component("sodium_channel")->variable("i_Na"));

    external_sodium_channel_i_Na->addDependency(potassium_channel_n_gate_alpha_n);
    external_sodium_channel_i_Na->addDependency(model->component("sodium_channel_h_gate")->variable("h"));

    analyser->addExternalVariable(libcellml::AnalyserExternalVariable::create(model->component("membrane")->variable("V")));
    analyser->addExternalVariable(external_sodium_channel_i_Na);
    analyser->addExternalVariable(libcellml::AnalyserExternalVariable::create(potassium_channel_n_gate_alpha_n));

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    auto profile = libcellml::GeneratorProfile::create();

    profile->setInterfaceFileNameString("model.external.h");

    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/model.external.h", generator->interfaceCode(analyserModel, profile));
    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/model.external.c", generator->implementationCode(analyserModel, profile));

    profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/model.external.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, hodgkinHuxleySquidAxonModel195Dae)
{
    // Same as the hodgkinHuxleySquidAxonModel1952 test, except that all the
    // algebraic equations are to be computed using NLA systems of one equation.

    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.dae.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    auto profile = libcellml::GeneratorProfile::create();

    profile->setInterfaceFileNameString("model.dae.h");

    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/model.dae.h", generator->interfaceCode(analyserModel, profile));
    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/model.dae.c", generator->implementationCode(analyserModel, profile));

    profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/model.dae.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, hodgkinHuxleySquidAxonModel1952DaeWithVariousExternalVariables)
{
    // Same as hodgkinHuxleySquidAxonModel1952WithVariousExternalVariables but with the DAE version of the HH52 model.

    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.dae.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();
    auto potassium_channel_n_gate_alpha_n = model->component("potassium_channel_n_gate")->variable("alpha_n");
    auto external_sodium_channel_i_Na = libcellml::AnalyserExternalVariable::create(model->component("sodium_channel")->variable("i_Na"));

    external_sodium_channel_i_Na->addDependency(potassium_channel_n_gate_alpha_n);
    external_sodium_channel_i_Na->addDependency(model->component("sodium_channel_h_gate")->variable("h"));

    analyser->addExternalVariable(libcellml::AnalyserExternalVariable::create(model->component("membrane")->variable("V")));
    analyser->addExternalVariable(external_sodium_channel_i_Na);
    analyser->addExternalVariable(libcellml::AnalyserExternalVariable::create(potassium_channel_n_gate_alpha_n));

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    auto profile = libcellml::GeneratorProfile::create();

    profile->setInterfaceFileNameString("model.dae.external.h");

    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/model.dae.external.h", generator->interfaceCode(analyserModel, profile));
    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/model.dae.external.c", generator->implementationCode(analyserModel, profile));

    profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/model.dae.external.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, nobleModel1962)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/noble_model_1962/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    EXPECT_EQ_FILE_CONTENTS("generator/noble_model_1962/model.h", generator->interfaceCode(analyserModel));
    EXPECT_EQ_FILE_CONTENTS("generator/noble_model_1962/model.c", generator->implementationCode(analyserModel));

    auto profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/noble_model_1962/model.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, robertsonOdeModel1966)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/robertson_model_1966/model.ode.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    auto profile = libcellml::GeneratorProfile::create();

    profile->setInterfaceFileNameString("model.ode.h");

    EXPECT_EQ_FILE_CONTENTS("generator/robertson_model_1966/model.ode.h", generator->interfaceCode(analyserModel, profile));
    EXPECT_EQ_FILE_CONTENTS("generator/robertson_model_1966/model.ode.c", generator->implementationCode(analyserModel, profile));

    profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/robertson_model_1966/model.ode.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, robertsonDaeModel1966)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/robertson_model_1966/model.dae.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    auto profile = libcellml::GeneratorProfile::create();

    profile->setInterfaceFileNameString("model.dae.h");

    EXPECT_EQ_FILE_CONTENTS("generator/robertson_model_1966/model.dae.h", generator->interfaceCode(analyserModel, profile));
    EXPECT_EQ_FILE_CONTENTS("generator/robertson_model_1966/model.dae.c", generator->implementationCode(analyserModel, profile));

    profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/robertson_model_1966/model.dae.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, sineImports)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("sine_approximations_import.xml"));
    auto importer = libcellml::Importer::create();

    EXPECT_EQ(size_t(0), parser->issueCount());
    EXPECT_TRUE(model->hasUnresolvedImports());

    importer->resolveImports(model, resourcePath());

    EXPECT_FALSE(model->hasUnresolvedImports());

    model = importer->flattenModel(model);

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    EXPECT_EQ_FILE_CONTENTS("generator/sine_model_imports/model.h", generator->interfaceCode(analyserModel));
    EXPECT_EQ_FILE_CONTENTS("generator/sine_model_imports/model.c", generator->implementationCode(analyserModel));

    auto profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/sine_model_imports/model.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, analyserModelScopeTest)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    libcellml::AnalyserModelPtr analyserModel = nullptr;

    {
        auto analyser = libcellml::Analyser::create();

        analyser->analyseModel(model);

        EXPECT_EQ(size_t(0), analyser->errorCount());

        analyserModel = analyser->analyserModel();
    }

    auto generator = libcellml::Generator::create();

    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/model.h", generator->interfaceCode(analyserModel));
    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/model.c", generator->implementationCode(analyserModel));
}

TEST(Generator, daeModel)
{
    auto parser = libcellml::Parser::create(false);
    auto model = parser->parseModel(fileContents("generator/dae_cellml_1_1_model/model.cellml"));

    EXPECT_EQ(size_t(0), parser->errorCount());

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    EXPECT_EQ_FILE_CONTENTS("generator/dae_cellml_1_1_model/model.h", generator->interfaceCode(analyserModel));
    EXPECT_EQ_FILE_CONTENTS("generator/dae_cellml_1_1_model/model.c", generator->implementationCode(analyserModel));

    auto profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/dae_cellml_1_1_model/model.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, variableInitialisedUsingAnotherVariable)
{
    // Note: this should be in sync with the corresponding Analyser test.

    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/variable_initialised_using_another_variable/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    EXPECT_EQ_FILE_CONTENTS("generator/variable_initialised_using_another_variable/model.h", generator->interfaceCode(analyserModel));
    EXPECT_EQ_FILE_CONTENTS("generator/variable_initialised_using_another_variable/model.c", generator->implementationCode(analyserModel));

    auto profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/variable_initialised_using_another_variable/model.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, modelOutOfScope)
{
    libcellml::AnalyserModelPtr analyserModel;

    {
        auto analyser = libcellml::Analyser::create();
        auto parser = libcellml::Parser::create();
        auto model = parser->parseModel(fileContents("generator/ode_multiple_dependent_odes/model.cellml"));

        EXPECT_EQ(size_t(0), parser->issueCount());

        analyser->analyseModel(model);

        EXPECT_EQ(size_t(0), analyser->errorCount());

        analyserModel = analyser->analyserModel();
    }

    auto generator = libcellml::Generator::create();

    auto profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/ode_multiple_dependent_odes/model.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, unknownVariableMarkedAsExternalVariable)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/unknown_variable_as_external_variable/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->addExternalVariable(libcellml::AnalyserExternalVariable::create(model->component("SLC_template3_ss")->variable("P_3")));

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    EXPECT_EQ_FILE_CONTENTS("generator/unknown_variable_as_external_variable/model.h", generator->interfaceCode(analyserModel));
    EXPECT_EQ_FILE_CONTENTS("generator/unknown_variable_as_external_variable/model.c", generator->implementationCode(analyserModel));

    auto profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/unknown_variable_as_external_variable/model.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, modelWithComplexUnitsOutOfScope)
{
    libcellml::AnalyserModelPtr analyserModel;

    {
        auto analyser = libcellml::Analyser::create();
        auto parser = libcellml::Parser::create();
        auto importer = libcellml::Importer::create();
        auto model = parser->parseModel(fileContents("generator/cellml_slc_example/slc_model.cellml"));

        EXPECT_EQ(size_t(0), parser->issueCount());

        importer->resolveImports(model, resourcePath("generator/cellml_slc_example"));
        EXPECT_FALSE(model->hasUnresolvedImports());

        model = importer->flattenModel(model);

        analyser->analyseModel(model);
        EXPECT_EQ(size_t(0), analyser->errorCount());

        analyserModel = analyser->analyserModel();
    }

    auto generator = libcellml::Generator::create();

    auto profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/cellml_slc_example/model.py", generator->implementationCode(analyserModel, profile));
}

TEST(Generator, generateCodeUsingProfileEnum)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/algebraic_eqn_computed_var_on_rhs/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->errorCount());

    auto analyserModel = analyser->analyserModel();
    auto generator = libcellml::Generator::create();

    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_eqn_computed_var_on_rhs/model.h", generator->interfaceCode(analyserModel, libcellml::GeneratorProfile::Profile::C));
    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_eqn_computed_var_on_rhs/model.c", generator->implementationCode(analyserModel, libcellml::GeneratorProfile::Profile::C));

    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_eqn_computed_var_on_rhs/model.py", generator->implementationCode(analyserModel, libcellml::GeneratorProfile::Profile::PYTHON));
}

TEST(Generator, checkGeneratorReleasesAnalyserModel)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/algebraic_eqn_computed_var_on_rhs/model.cellml"));
    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    auto analyserModel = analyser->analyserModel();

    EXPECT_EQ(2, analyserModel.use_count());
    EXPECT_EQ("x = a", libcellml::Generator::equationCode(analyserModel->analyserEquation(0)->ast()));
    EXPECT_EQ(2, analyserModel.use_count());

    auto generator = libcellml::Generator::create();

    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_eqn_computed_var_on_rhs/model.c", generator->implementationCode(analyserModel));
    EXPECT_EQ(2, analyserModel.use_count());

    EXPECT_EQ_FILE_CONTENTS("generator/algebraic_eqn_computed_var_on_rhs/model.h", generator->interfaceCode(analyserModel));
    EXPECT_EQ(2, analyserModel.use_count());
}
