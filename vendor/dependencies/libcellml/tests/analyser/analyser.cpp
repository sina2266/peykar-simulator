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

TEST(Analyser, unlinkedUnitsInModel)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    // When parsing a model all Units are automatically linked.

    EXPECT_FALSE(model->hasUnlinkedUnits());

    // When we set a Units units by name we create an unlinked Units. That is,
    // a Units that has a reference to a Units defined in the model but it is
    // not using the object Units defined in the model.

    model->component("membrane")->variable("V")->setUnits("millivolt");

    EXPECT_TRUE(model->hasUnlinkedUnits());

    // As a result of the aforementioned unlinked Units, the analyser should log
    // an error for it.

    const std::vector<std::string> expectedIssues = {
        "The model has units which are not linked together.",
    };

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ_ISSUES_CELLMLELEMENTTYPES_LEVELS_REFERENCERULES_URLS(expectedIssues,
                                                                   expectedCellmlElementTypes(expectedIssues.size(), libcellml::CellmlElementType::UNDEFINED),
                                                                   expectedLevels(expectedIssues.size(), libcellml::Issue::Level::ERROR),
                                                                   expectedReferenceRules(expectedIssues.size(), libcellml::Issue::ReferenceRule::ANALYSER_UNLINKED_UNITS),
                                                                   expectedUrls(expectedIssues.size(), "https://libcellml.org/documentation/guides/latest/runtime_codes/index?issue=ANALYSER_UNLINKED_UNITS"),
                                                                   analyser);
}

TEST(Analyser, notEqualityStatement)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("analyser/not_equality_statement.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    const std::vector<std::string> expectedIssues = {
        "Equation 'x+y+z' in component 'my_component' is not an equality statement (i.e. LHS = RHS).",
    };

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ_ISSUES_CELLMLELEMENTTYPES_LEVELS_REFERENCERULES_URLS(expectedIssues,
                                                                   expectedCellmlElementTypes(expectedIssues.size(), libcellml::CellmlElementType::COMPONENT),
                                                                   expectedLevels(expectedIssues.size(), libcellml::Issue::Level::ERROR),
                                                                   expectedReferenceRules(expectedIssues.size(), libcellml::Issue::ReferenceRule::ANALYSER_EQUATION_NOT_EQUALITY_STATEMENT),
                                                                   expectedUrls(expectedIssues.size(), "https://libcellml.org/documentation/guides/latest/runtime_codes/index?issue=ANALYSER_EQUATION_NOT_EQUALITY_STATEMENT"),
                                                                   analyser);

    EXPECT_EQ(libcellml::AnalyserModel::Type::INVALID, analyser->analyserModel()->type());
    EXPECT_EQ("invalid", libcellml::AnalyserModel::typeAsString(analyser->analyserModel()->type()));
}

TEST(Analyser, initialisedVariableOfIntegration)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("analyser/initialised_variable_of_integration.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    const std::vector<std::string> expectedIssues = {
        "Variable 'time' in component 'my_component' cannot be both a variable of integration and initialised.",
    };

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ_ISSUES_CELLMLELEMENTTYPES_LEVELS_REFERENCERULES_URLS(expectedIssues,
                                                                   expectedCellmlElementTypes(expectedIssues.size(), libcellml::CellmlElementType::VARIABLE),
                                                                   expectedLevels(expectedIssues.size(), libcellml::Issue::Level::ERROR),
                                                                   expectedReferenceRules(expectedIssues.size(), libcellml::Issue::ReferenceRule::ANALYSER_VOI_INITIALISED),
                                                                   expectedUrls(expectedIssues.size(), "https://libcellml.org/documentation/guides/latest/runtime_codes/index?issue=ANALYSER_VOI_INITIALISED"),
                                                                   analyser);

    EXPECT_EQ(libcellml::AnalyserModel::Type::INVALID, analyser->analyserModel()->type());
}

TEST(Analyser, initialisedVariableOfIntegrationInNonFirstComponent)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("analyser/initialised_variable_of_integration_in_non_first_component.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    const std::vector<std::string> expectedIssues = {
        "Variable 'time' in component 'environment' cannot be both a variable of integration and initialised.",
    };

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ_ISSUES_CELLMLELEMENTTYPES_LEVELS_REFERENCERULES_URLS(expectedIssues,
                                                                   expectedCellmlElementTypes(expectedIssues.size(), libcellml::CellmlElementType::VARIABLE),
                                                                   expectedLevels(expectedIssues.size(), libcellml::Issue::Level::ERROR),
                                                                   expectedReferenceRules(expectedIssues.size(), libcellml::Issue::ReferenceRule::ANALYSER_VOI_INITIALISED),
                                                                   expectedUrls(expectedIssues.size(), "https://libcellml.org/documentation/guides/latest/runtime_codes/index?issue=ANALYSER_VOI_INITIALISED"),
                                                                   analyser);

    EXPECT_EQ(libcellml::AnalyserModel::Type::INVALID, analyser->analyserModel()->type());
}

TEST(Analyser, twoVariablesOfIntegration)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("analyser/two_variables_of_integration.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    const std::vector<std::string> expectedIssues = {
        "Variable 'time' in component 'main' and variable 'other_time' in component 'sub_sub_sub' cannot both be the variable of integration.",
    };

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ_ISSUES_CELLMLELEMENTTYPES_LEVELS_REFERENCERULES_URLS(expectedIssues,
                                                                   expectedCellmlElementTypes(expectedIssues.size(), libcellml::CellmlElementType::VARIABLE),
                                                                   expectedLevels(expectedIssues.size(), libcellml::Issue::Level::ERROR),
                                                                   expectedReferenceRules(expectedIssues.size(), libcellml::Issue::ReferenceRule::ANALYSER_VOI_SEVERAL),
                                                                   expectedUrls(expectedIssues.size(), "https://libcellml.org/documentation/guides/latest/runtime_codes/index?issue=ANALYSER_VOI_SEVERAL"),
                                                                   analyser);

    EXPECT_EQ(libcellml::AnalyserModel::Type::INVALID, analyser->analyserModel()->type());
}

TEST(Analyser, nonFirstOrderOdes)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("analyser/non_first_order_odes.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    const std::vector<std::string> expectedIssues = {
        "The differential equation for variable 'x' in component 'main' must be of the first order.",
        "The differential equation for variable 'y' in component 'sub' must be of the first order.",
        "The differential equation for variable 'z' in component 'sub_sub' must be of the first order.",
    };

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ_ISSUES_CELLMLELEMENTTYPES_LEVELS_REFERENCERULES_URLS(expectedIssues,
                                                                   expectedCellmlElementTypes(expectedIssues.size(), libcellml::CellmlElementType::MATH),
                                                                   expectedLevels(expectedIssues.size(), libcellml::Issue::Level::ERROR),
                                                                   expectedReferenceRules(expectedIssues.size(), libcellml::Issue::ReferenceRule::ANALYSER_ODE_NOT_FIRST_ORDER),
                                                                   expectedUrls(expectedIssues.size(), "https://libcellml.org/documentation/guides/latest/runtime_codes/index?issue=ANALYSER_ODE_NOT_FIRST_ORDER"),
                                                                   analyser);

    EXPECT_EQ(libcellml::AnalyserModel::Type::INVALID, analyser->analyserModel()->type());
}

TEST(Analyser, undefinedVariables)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("analyser/undefined_variables.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    const std::vector<std::string> expectedIssues = {
        "MathML ci element has the child text 'a' which does not correspond with any variable names present in component 'my_component'.",
        "MathML ci element has the child text 'b' which does not correspond with any variable names present in component 'my_component'.",
    };

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ_ISSUES_CELLMLELEMENTTYPES_LEVELS_REFERENCERULES_URLS(expectedIssues,
                                                                   expectedCellmlElementTypes(expectedIssues.size(), libcellml::CellmlElementType::MATH),
                                                                   expectedLevels(expectedIssues.size(), libcellml::Issue::Level::ERROR),
                                                                   expectedReferenceRules(expectedIssues.size(), libcellml::Issue::ReferenceRule::MATH_CI_VARIABLE_REFERENCE),
                                                                   expectedUrls(expectedIssues.size(), "https://cellml-specification.readthedocs.io/en/latest/reference/formal_and_informative/specB12.html?issue=MATH_CI_VARIABLE_REFERENCE"),
                                                                   analyser);

    EXPECT_EQ(libcellml::AnalyserModel::Type::INVALID, analyser->analyserModel()->type());
}

TEST(Analyser, variableInitialisedTwice)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("analyser/variable_initialised_twice.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    const std::vector<std::string> expectedIssues = {
        "Variable 'x' in component 'sub' and variable 'x' in component 'main' are equivalent and cannot therefore both be initialised.",
    };

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ_ISSUES_CELLMLELEMENTTYPES_LEVELS_REFERENCERULES_URLS(expectedIssues,
                                                                   expectedCellmlElementTypes(expectedIssues.size(), libcellml::CellmlElementType::VARIABLE),
                                                                   expectedLevels(expectedIssues.size(), libcellml::Issue::Level::ERROR),
                                                                   expectedReferenceRules(expectedIssues.size(), libcellml::Issue::ReferenceRule::ANALYSER_VARIABLE_INITIALISED_MORE_THAN_ONCE),
                                                                   expectedUrls(expectedIssues.size(), "https://libcellml.org/documentation/guides/latest/runtime_codes/index?issue=ANALYSER_VARIABLE_INITIALISED_MORE_THAN_ONCE"),
                                                                   analyser);

    EXPECT_EQ(libcellml::AnalyserModel::Type::INVALID, analyser->analyserModel()->type());
}

TEST(Analyser, variableInitialisedUsingANonExistingVariable)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("analyser/variable_initialised_using_a_non_existing_variable.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    const std::vector<std::string> expectedIssues = {
        "Variable 'x' in component 'main' has an invalid initial value 'k'. Initial values must be a real number string or a variable reference.",
    };

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ_ISSUES_CELLMLELEMENTTYPES_LEVELS_REFERENCERULES_URLS(expectedIssues,
                                                                   expectedCellmlElementTypes(expectedIssues.size(), libcellml::CellmlElementType::VARIABLE),
                                                                   expectedLevels(expectedIssues.size(), libcellml::Issue::Level::ERROR),
                                                                   expectedReferenceRules(expectedIssues.size(), libcellml::Issue::ReferenceRule::VARIABLE_INITIAL_VALUE_VALUE),
                                                                   expectedUrls(expectedIssues.size(), "https://cellml-specification.readthedocs.io/en/latest/reference/formal_and_informative/specB08.html?issue=VARIABLE_INITIAL_VALUE_VALUE"),
                                                                   analyser);

    EXPECT_EQ(libcellml::AnalyserModel::Type::INVALID, analyser->analyserModel()->type());
}

TEST(Analyser, variableInitialisedUsingAnotherVariable)
{
    // Note: this should be in sync with the corresponding Generator test.

    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("analyser/variable_initialised_using_another_variable.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    const std::vector<std::string> expectedIssues = {
        "Variable 'kStateStateAlgebraic' in component 'main' is initialised using variable 'kStateAlgebraic', which is an algebraic variable. Only a reference to a constant, a computed constant, a state variable, or a computable non-linear algebraic variable is allowed.",
        "Variable 'kNlaStateAlgebraic' in component 'main' is initialised using variable 'kStateAlgebraic', which is an algebraic variable. Only a reference to a constant, a computed constant, a state variable, or a computable non-linear algebraic variable is allowed.",
        "Variable 'xStateNlaAlgebraic' in component 'main' is initialised using variable 'kStateNlaAlgebraic', which is an algebraic variable. Only a reference to a constant, a computed constant, a state variable, or a computable non-linear algebraic variable is allowed.",
        "Variable 'xNlaNlaAlgebraic' in component 'main' is initialised using variable 'kNlaNlaAlgebraic', which is an algebraic variable. Only a reference to a constant, a computed constant, a state variable, or a computable non-linear algebraic variable is allowed.",
        "Variable 'xStateAlgebraic' in component 'main' is initialised using variable 'kStateAlgebraic', which is an algebraic variable. Only a reference to a constant, a computed constant, a state variable, or a computable non-linear algebraic variable is allowed.",
        "Variable 'xNlaAlgebraic' in component 'main' is initialised using variable 'kNlaAlgebraic', which is an algebraic variable. Only a reference to a constant, a computed constant, a state variable, or a computable non-linear algebraic variable is allowed.",
    };

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ_ISSUES_CELLMLELEMENTTYPES_LEVELS_REFERENCERULES_URLS(expectedIssues,
                                                                   expectedCellmlElementTypes(expectedIssues.size(), libcellml::CellmlElementType::VARIABLE),
                                                                   expectedLevels(expectedIssues.size(), libcellml::Issue::Level::ERROR),
                                                                   expectedReferenceRules(expectedIssues.size(), libcellml::Issue::ReferenceRule::ANALYSER_VARIABLE_INITIALISED_USING_ALGEBRAIC_VARIABLE),
                                                                   expectedUrls(expectedIssues.size(), "https://libcellml.org/documentation/guides/latest/runtime_codes/index?issue=ANALYSER_VARIABLE_INITIALISED_USING_ALGEBRAIC_VARIABLE"),
                                                                   analyser);

    EXPECT_EQ(libcellml::AnalyserModel::Type::INVALID, analyser->analyserModel()->type());
}

TEST(Analyser, nonInitialisedState)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("analyser/non_initialised_state.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    const std::vector<std::string> expectedIssues = {
        "Variable 'x' in component 'my_component' is used in an ODE, but it is not initialised.",
    };
    const std::string expectedVariableName = "x";

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ_ISSUES_CELLMLELEMENTTYPES_LEVELS_REFERENCERULES_URLS(expectedIssues,
                                                                   expectedCellmlElementTypes(expectedIssues.size(), libcellml::CellmlElementType::VARIABLE),
                                                                   expectedLevels(expectedIssues.size(), libcellml::Issue::Level::ERROR),
                                                                   expectedReferenceRules(expectedIssues.size(), libcellml::Issue::ReferenceRule::ANALYSER_STATE_NOT_INITIALISED),
                                                                   expectedUrls(expectedIssues.size(), "https://libcellml.org/documentation/guides/latest/runtime_codes/index?issue=ANALYSER_STATE_NOT_INITIALISED"),
                                                                   analyser);
    EXPECT_EQ(expectedVariableName, analyser->issue(0)->item()->variable()->name());

    EXPECT_EQ(libcellml::AnalyserModel::Type::UNDERCONSTRAINED, analyser->analyserModel()->type());
}

TEST(Analyser, underconstrained)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("analyser/underconstrained.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    const std::vector<std::string> expectedIssues = {
        "The type of variable 'x' in component 'my_component' is unknown.",
    };

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ_ISSUES_CELLMLELEMENTTYPES_LEVELS_REFERENCERULES_URLS(expectedIssues,
                                                                   expectedCellmlElementTypes(expectedIssues.size(), libcellml::CellmlElementType::VARIABLE),
                                                                   expectedLevels(expectedIssues.size(), libcellml::Issue::Level::ERROR),
                                                                   expectedReferenceRules(expectedIssues.size(), libcellml::Issue::ReferenceRule::ANALYSER_VARIABLE_UNUSED),
                                                                   expectedUrls(expectedIssues.size(), "https://libcellml.org/documentation/guides/latest/runtime_codes/index?issue=ANALYSER_VARIABLE_UNUSED"),
                                                                   analyser);

    EXPECT_EQ(libcellml::AnalyserModel::Type::UNDERCONSTRAINED, analyser->analyserModel()->type());
}

TEST(Analyser, overconstrained)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("analyser/overconstrained.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    const std::vector<std::string> expectedIssues = {
        "Variable 'x' in component 'my_component' is overconstrained.",
    };

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ_ISSUES_CELLMLELEMENTTYPES_LEVELS_REFERENCERULES_URLS(expectedIssues,
                                                                   expectedCellmlElementTypes(expectedIssues.size(), libcellml::CellmlElementType::VARIABLE),
                                                                   expectedLevels(expectedIssues.size(), libcellml::Issue::Level::ERROR),
                                                                   expectedReferenceRules(expectedIssues.size(), libcellml::Issue::ReferenceRule::ANALYSER_VARIABLE_OVERCONSTRAINED),
                                                                   expectedUrls(expectedIssues.size(), "https://libcellml.org/documentation/guides/latest/runtime_codes/index?issue=ANALYSER_VARIABLE_OVERCONSTRAINED"),
                                                                   analyser);

    EXPECT_EQ(libcellml::AnalyserModel::Type::OVERCONSTRAINED, analyser->analyserModel()->type());
}

TEST(Analyser, unsuitablyConstrained)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("analyser/unsuitably_constrained.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    const std::vector<std::string> expectedIssues = {
        "Variable 'y' in component 'my_component' is overconstrained.",
        "The type of variable 'x' in component 'my_component' is unknown.",
    };
    const std::vector<libcellml::Issue::ReferenceRule> expectedReferenceRules = {
        libcellml::Issue::ReferenceRule::ANALYSER_VARIABLE_OVERCONSTRAINED,
        libcellml::Issue::ReferenceRule::ANALYSER_VARIABLE_UNUSED,
    };
    const std::vector<std::string> expectedUrls = {
        "https://libcellml.org/documentation/guides/latest/runtime_codes/index?issue=ANALYSER_VARIABLE_OVERCONSTRAINED",
        "https://libcellml.org/documentation/guides/latest/runtime_codes/index?issue=ANALYSER_VARIABLE_UNUSED",
    };

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ_ISSUES_CELLMLELEMENTTYPES_LEVELS_REFERENCERULES_URLS(expectedIssues,
                                                                   expectedCellmlElementTypes(expectedIssues.size(), libcellml::CellmlElementType::VARIABLE),
                                                                   expectedLevels(expectedIssues.size(), libcellml::Issue::Level::ERROR),
                                                                   expectedReferenceRules,
                                                                   expectedUrls,
                                                                   analyser);

    EXPECT_EQ(libcellml::AnalyserModel::Type::UNSUITABLY_CONSTRAINED, analyser->analyserModel()->type());
}

TEST(Analyser, addSameExternalVariableAsVariable)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();
    auto variable = model->component("membrane")->variable("V");

    EXPECT_TRUE(analyser->addExternalVariable(variable));
    EXPECT_EQ(size_t(1), analyser->externalVariableCount());

    EXPECT_FALSE(analyser->addExternalVariable(variable));
    EXPECT_EQ(size_t(1), analyser->externalVariableCount());
}

TEST(Analyser, addSameExternalVariableAsExternalVariable)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();
    auto externalVariable = libcellml::AnalyserExternalVariable::create(model->component("membrane")->variable("V"));

    EXPECT_TRUE(analyser->addExternalVariable(externalVariable));
    EXPECT_EQ(size_t(1), analyser->externalVariableCount());

    EXPECT_FALSE(analyser->addExternalVariable(externalVariable));
    EXPECT_EQ(size_t(1), analyser->externalVariableCount());
}

TEST(Analyser, addExternalVariableFromDifferentModels)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto other_model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    const std::vector<std::string> expectedIssues = {
        "Variable 'V' in component 'membrane' is marked as an external variable, but it belongs to a different model and will therefore be ignored.",
    };

    auto analyser = libcellml::Analyser::create();

    EXPECT_TRUE(analyser->addExternalVariable(model->component("membrane")->variable("V")));
    EXPECT_TRUE(analyser->addExternalVariable(other_model->component("membrane")->variable("V")));
    EXPECT_EQ(size_t(2), analyser->externalVariableCount());

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(1), analyser->messageCount());
    EXPECT_EQ(expectedIssues[0], analyser->message(0)->description());

    EXPECT_EQ_ISSUES_CELLMLELEMENTTYPES_LEVELS_REFERENCERULES_URLS(expectedIssues,
                                                                   expectedCellmlElementTypes(expectedIssues.size(), libcellml::CellmlElementType::VARIABLE),
                                                                   expectedLevels(expectedIssues.size(), libcellml::Issue::Level::MESSAGE),
                                                                   expectedReferenceRules(expectedIssues.size(), libcellml::Issue::ReferenceRule::ANALYSER_EXTERNAL_VARIABLE_DIFFERENT_MODEL),
                                                                   expectedUrls(expectedIssues.size(), "https://libcellml.org/documentation/guides/latest/runtime_codes/index?issue=ANALYSER_EXTERNAL_VARIABLE_DIFFERENT_MODEL"),
                                                                   analyser);
}

TEST(Analyser, removeExternalVariableByIndex)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    EXPECT_FALSE(analyser->removeExternalVariable(0));
    EXPECT_TRUE(analyser->addExternalVariable(model->component("membrane")->variable("V")));
    EXPECT_TRUE(analyser->removeExternalVariable(0));
    EXPECT_FALSE(analyser->removeExternalVariable(1));
}

TEST(Analyser, removeExternalVariableByName)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();
    auto variable = model->component("membrane")->variable("V");

    EXPECT_FALSE(analyser->removeExternalVariable(variable));
    EXPECT_TRUE(analyser->addExternalVariable(variable));
    EXPECT_FALSE(analyser->removeExternalVariable(static_cast<libcellml::VariablePtr>(nullptr)));
    EXPECT_FALSE(analyser->removeExternalVariable(model->component("membrane")->variable("Cm")));
    EXPECT_TRUE(analyser->removeExternalVariable(variable));
    EXPECT_FALSE(analyser->removeExternalVariable(variable));
}

TEST(Analyser, removeExternalVariableByPointer)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();
    auto externalVariable = libcellml::AnalyserExternalVariable::create(model->component("membrane")->variable("V"));

    EXPECT_FALSE(analyser->removeExternalVariable(externalVariable));
    EXPECT_TRUE(analyser->addExternalVariable(externalVariable));
    EXPECT_FALSE(analyser->removeExternalVariable(static_cast<libcellml::AnalyserExternalVariablePtr>(nullptr)));
    EXPECT_TRUE(analyser->removeExternalVariable(externalVariable));
    EXPECT_FALSE(analyser->removeExternalVariable(externalVariable));
}

TEST(Analyser, removeAllExternalVariables)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    EXPECT_TRUE(analyser->addExternalVariable(model->component("membrane")->variable("V")));
    EXPECT_TRUE(analyser->addExternalVariable(model->component("sodium_channel")->variable("V")));
    EXPECT_TRUE(analyser->addExternalVariable(model->component("potassium_channel")->variable("V")));
    EXPECT_EQ(size_t(3), analyser->externalVariableCount());

    analyser->removeAllExternalVariables();

    EXPECT_EQ(size_t(0), analyser->externalVariableCount());
}

TEST(Analyser, containsExternalVariableByName)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();
    auto variable = model->component("membrane")->variable("V");

    EXPECT_TRUE(analyser->addExternalVariable(variable));
    EXPECT_TRUE(analyser->containsExternalVariable(variable));
    EXPECT_FALSE(analyser->containsExternalVariable(static_cast<libcellml::AnalyserExternalVariablePtr>(nullptr)));
    EXPECT_FALSE(analyser->containsExternalVariable(model->component("membrane")->variable("Cm")));
}

TEST(Analyser, containsExternalVariableByPointer)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();
    auto externalVariable = libcellml::AnalyserExternalVariable::create(model->component("membrane")->variable("V"));

    EXPECT_TRUE(analyser->addExternalVariable(externalVariable));
    EXPECT_TRUE(analyser->containsExternalVariable(externalVariable));
    EXPECT_FALSE(analyser->containsExternalVariable(static_cast<libcellml::AnalyserExternalVariablePtr>(nullptr)));
}

TEST(Analyser, externalVariableByIndex)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    EXPECT_EQ(nullptr, analyser->externalVariable(0));

    auto externalVariable = libcellml::AnalyserExternalVariable::create(model->component("membrane")->variable("V"));

    EXPECT_TRUE(analyser->addExternalVariable(externalVariable));
    EXPECT_EQ(externalVariable, analyser->externalVariable(0));
    EXPECT_EQ(nullptr, analyser->externalVariable(1));
}

TEST(Analyser, externalVariableByName)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();
    auto variable = model->component("membrane")->variable("V");

    EXPECT_EQ(nullptr, analyser->externalVariable(variable));

    auto externalVariable = libcellml::AnalyserExternalVariable::create(model->component("membrane")->variable("V"));

    EXPECT_TRUE(analyser->addExternalVariable(externalVariable));
    EXPECT_EQ(externalVariable, analyser->externalVariable(variable));
    EXPECT_EQ(nullptr, analyser->externalVariable(static_cast<libcellml::VariablePtr>(nullptr)));
    EXPECT_EQ(nullptr, analyser->externalVariable(model->component("membrane")->variable("Cm")));
}

TEST(Analyser, onePrimaryVoiExternalVariable)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    const std::vector<std::string> expectedIssues = {
        "Variable 'time' in component 'environment' is marked as an external variable, but it is the variable of integration which cannot be used as an external variable.",
    };

    auto analyser = libcellml::Analyser::create();

    EXPECT_TRUE(analyser->addExternalVariable(model->component("environment")->variable("time")));

    analyser->analyseModel(model);

    EXPECT_EQ_ISSUES(expectedIssues, analyser);
}

TEST(Analyser, oneNonPrimaryVoiExternalVariable)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    const std::vector<std::string> expectedIssues = {
        "Variable 'time' in component 'membrane' is marked as an external variable, but it is equivalent to variable 'time' in component 'environment', the primary variable of integration which cannot be used as an external variable.",
    };

    auto analyser = libcellml::Analyser::create();

    EXPECT_TRUE(analyser->addExternalVariable(model->component("membrane")->variable("time")));

    analyser->analyseModel(model);

    EXPECT_EQ_ISSUES(expectedIssues, analyser);
}

TEST(Analyser, twoEquivalentVoiExternalVariablesIncludingPrimaryVariable)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    const std::vector<std::string> expectedIssues = {
        "Both variable 'time' in component 'environment' and variable 'time' in component 'membrane' are marked as external variables, but they are equivalent to variable 'time' in component 'environment', the primary variable of integration which cannot be used as an external variable.",
    };

    auto analyser = libcellml::Analyser::create();

    EXPECT_TRUE(analyser->addExternalVariable(model->component("environment")->variable("time")));
    EXPECT_TRUE(analyser->addExternalVariable(model->component("membrane")->variable("time")));

    analyser->analyseModel(model);

    EXPECT_EQ_ISSUES_CELLMLELEMENTTYPES_LEVELS_REFERENCERULES_URLS(expectedIssues,
                                                                   expectedCellmlElementTypes(expectedIssues.size(), libcellml::CellmlElementType::VARIABLE),
                                                                   expectedLevels(expectedIssues.size(), libcellml::Issue::Level::MESSAGE),
                                                                   expectedReferenceRules(expectedIssues.size(), libcellml::Issue::ReferenceRule::ANALYSER_EXTERNAL_VARIABLE_VOI),
                                                                   expectedUrls(expectedIssues.size(), "https://libcellml.org/documentation/guides/latest/runtime_codes/index?issue=ANALYSER_EXTERNAL_VARIABLE_VOI"),
                                                                   analyser);
}

TEST(Analyser, twoEquivalentVoiExternalVariablesNotIncludingPrimaryVariable)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    const std::vector<std::string> expectedIssues = {
        "Both variable 'time' in component 'membrane' and variable 'time' in component 'sodium_channel' are marked as external variables, but they are equivalent to variable 'time' in component 'environment', the primary variable of integration which cannot be used as an external variable.",
    };

    auto analyser = libcellml::Analyser::create();

    EXPECT_TRUE(analyser->addExternalVariable(model->component("membrane")->variable("time")));
    EXPECT_TRUE(analyser->addExternalVariable(model->component("sodium_channel")->variable("time")));

    analyser->analyseModel(model);

    EXPECT_EQ_ISSUES_CELLMLELEMENTTYPES_LEVELS_REFERENCERULES_URLS(expectedIssues,
                                                                   expectedCellmlElementTypes(expectedIssues.size(), libcellml::CellmlElementType::VARIABLE),
                                                                   expectedLevels(expectedIssues.size(), libcellml::Issue::Level::MESSAGE),
                                                                   expectedReferenceRules(expectedIssues.size(), libcellml::Issue::ReferenceRule::ANALYSER_EXTERNAL_VARIABLE_VOI),
                                                                   expectedUrls(expectedIssues.size(), "https://libcellml.org/documentation/guides/latest/runtime_codes/index?issue=ANALYSER_EXTERNAL_VARIABLE_VOI"),
                                                                   analyser);
}

TEST(Analyser, threeEquivalentVoiExternalVariablesIncludingPrimaryVariable)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    const std::vector<std::string> expectedIssues = {
        "Variable 'time' in component 'environment', variable 'time' in component 'membrane' and variable 'time' in component 'sodium_channel' are marked as external variables, but they are equivalent to variable 'time' in component 'environment', the primary variable of integration which cannot be used as an external variable.",
    };

    auto analyser = libcellml::Analyser::create();

    EXPECT_TRUE(analyser->addExternalVariable(model->component("environment")->variable("time")));
    EXPECT_TRUE(analyser->addExternalVariable(model->component("membrane")->variable("time")));
    EXPECT_TRUE(analyser->addExternalVariable(model->component("sodium_channel")->variable("time")));

    analyser->analyseModel(model);

    EXPECT_EQ_ISSUES_CELLMLELEMENTTYPES_LEVELS_REFERENCERULES_URLS(expectedIssues,
                                                                   expectedCellmlElementTypes(expectedIssues.size(), libcellml::CellmlElementType::VARIABLE),
                                                                   expectedLevels(expectedIssues.size(), libcellml::Issue::Level::MESSAGE),
                                                                   expectedReferenceRules(expectedIssues.size(), libcellml::Issue::ReferenceRule::ANALYSER_EXTERNAL_VARIABLE_VOI),
                                                                   expectedUrls(expectedIssues.size(), "https://libcellml.org/documentation/guides/latest/runtime_codes/index?issue=ANALYSER_EXTERNAL_VARIABLE_VOI"),
                                                                   analyser);
}

TEST(Analyser, threeEquivalentVoiExternalVariablesNotIncludingPrimaryVariable)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    const std::vector<std::string> expectedIssues = {
        "Variable 'time' in component 'membrane', variable 'time' in component 'sodium_channel' and variable 'time' in component 'potassium_channel' are marked as external variables, but they are equivalent to variable 'time' in component 'environment', the primary variable of integration which cannot be used as an external variable.",
    };

    auto analyser = libcellml::Analyser::create();

    EXPECT_TRUE(analyser->addExternalVariable(model->component("membrane")->variable("time")));
    EXPECT_TRUE(analyser->addExternalVariable(model->component("sodium_channel")->variable("time")));
    EXPECT_TRUE(analyser->addExternalVariable(model->component("potassium_channel")->variable("time")));

    analyser->analyseModel(model);

    EXPECT_EQ_ISSUES_CELLMLELEMENTTYPES_LEVELS_REFERENCERULES_URLS(expectedIssues,
                                                                   expectedCellmlElementTypes(expectedIssues.size(), libcellml::CellmlElementType::VARIABLE),
                                                                   expectedLevels(expectedIssues.size(), libcellml::Issue::Level::MESSAGE),
                                                                   expectedReferenceRules(expectedIssues.size(), libcellml::Issue::ReferenceRule::ANALYSER_EXTERNAL_VARIABLE_VOI),
                                                                   expectedUrls(expectedIssues.size(), "https://libcellml.org/documentation/guides/latest/runtime_codes/index?issue=ANALYSER_EXTERNAL_VARIABLE_VOI"),
                                                                   analyser);
}

TEST(Analyser, onePrimaryExternalVariable)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto analyser = libcellml::Analyser::create();

    EXPECT_TRUE(analyser->addExternalVariable(model->component("membrane")->variable("V")));

    analyser->analyseModel(model);

    EXPECT_EQ(size_t(0), analyser->issueCount());
}

TEST(Analyser, oneNonPrimaryExternalVariable)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    const std::vector<std::string> expectedIssues = {
        "Variable 'V' in component 'sodium_channel' is marked as an external variable, but it is not a primary variable. Variable 'V' in component 'membrane' is its corresponding primary variable and will therefore be the one used as an external variable.",
    };

    auto analyser = libcellml::Analyser::create();

    EXPECT_TRUE(analyser->addExternalVariable(model->component("sodium_channel")->variable("V")));

    analyser->analyseModel(model);

    EXPECT_EQ_ISSUES_CELLMLELEMENTTYPES_LEVELS_REFERENCERULES_URLS(expectedIssues,
                                                                   expectedCellmlElementTypes(expectedIssues.size(), libcellml::CellmlElementType::VARIABLE),
                                                                   expectedLevels(expectedIssues.size(), libcellml::Issue::Level::MESSAGE),
                                                                   expectedReferenceRules(expectedIssues.size(), libcellml::Issue::ReferenceRule::ANALYSER_EXTERNAL_VARIABLE_USE_PRIMARY_VARIABLE),
                                                                   expectedUrls(expectedIssues.size(), "https://libcellml.org/documentation/guides/latest/runtime_codes/index?issue=ANALYSER_EXTERNAL_VARIABLE_USE_PRIMARY_VARIABLE"),
                                                                   analyser);
}

TEST(Analyser, twoEquivalentExternalVariablesIncludingPrimaryVariable)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    const std::vector<std::string> expectedIssues = {
        "Both variable 'V' in component 'membrane' and variable 'V' in component 'sodium_channel' are marked as external variables, but they are equivalent. Variable 'V' in component 'membrane' is the primary variable and will therefore be the one used as an external variable.",
    };

    auto analyser = libcellml::Analyser::create();

    EXPECT_TRUE(analyser->addExternalVariable(model->component("membrane")->variable("V")));
    EXPECT_TRUE(analyser->addExternalVariable(model->component("sodium_channel")->variable("V")));

    analyser->analyseModel(model);

    EXPECT_EQ_ISSUES_CELLMLELEMENTTYPES_LEVELS_REFERENCERULES_URLS(expectedIssues,
                                                                   expectedCellmlElementTypes(expectedIssues.size(), libcellml::CellmlElementType::VARIABLE),
                                                                   expectedLevels(expectedIssues.size(), libcellml::Issue::Level::MESSAGE),
                                                                   expectedReferenceRules(expectedIssues.size(), libcellml::Issue::ReferenceRule::ANALYSER_EXTERNAL_VARIABLE_USE_PRIMARY_VARIABLE),
                                                                   expectedUrls(expectedIssues.size(), "https://libcellml.org/documentation/guides/latest/runtime_codes/index?issue=ANALYSER_EXTERNAL_VARIABLE_USE_PRIMARY_VARIABLE"),
                                                                   analyser);
}

TEST(Analyser, twoEquivalentExternalVariablesNotIncludingPrimaryVariable)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    const std::vector<std::string> expectedIssues = {
        "Both variable 'V' in component 'sodium_channel' and variable 'V' in component 'potassium_channel' are marked as external variables, but they are equivalent. Variable 'V' in component 'membrane' is their corresponding primary variable and will therefore be the one used as an external variable.",
    };

    auto analyser = libcellml::Analyser::create();

    EXPECT_TRUE(analyser->addExternalVariable(model->component("sodium_channel")->variable("V")));
    EXPECT_TRUE(analyser->addExternalVariable(model->component("potassium_channel")->variable("V")));

    analyser->analyseModel(model);

    EXPECT_EQ_ISSUES_CELLMLELEMENTTYPES_LEVELS_REFERENCERULES_URLS(expectedIssues,
                                                                   expectedCellmlElementTypes(expectedIssues.size(), libcellml::CellmlElementType::VARIABLE),
                                                                   expectedLevels(expectedIssues.size(), libcellml::Issue::Level::MESSAGE),
                                                                   expectedReferenceRules(expectedIssues.size(), libcellml::Issue::ReferenceRule::ANALYSER_EXTERNAL_VARIABLE_USE_PRIMARY_VARIABLE),
                                                                   expectedUrls(expectedIssues.size(), "https://libcellml.org/documentation/guides/latest/runtime_codes/index?issue=ANALYSER_EXTERNAL_VARIABLE_USE_PRIMARY_VARIABLE"),
                                                                   analyser);
}

TEST(Analyser, threeEquivalentExternalVariablesIncludingPrimaryVariable)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    const std::vector<std::string> expectedIssues = {
        "Variable 'V' in component 'membrane', variable 'V' in component 'sodium_channel' and variable 'V' in component 'potassium_channel' are marked as external variables, but they are all equivalent. Variable 'V' in component 'membrane' is the primary variable and will therefore be the one used as an external variable.",
    };

    auto analyser = libcellml::Analyser::create();

    EXPECT_TRUE(analyser->addExternalVariable(model->component("membrane")->variable("V")));
    EXPECT_TRUE(analyser->addExternalVariable(model->component("sodium_channel")->variable("V")));
    EXPECT_TRUE(analyser->addExternalVariable(model->component("potassium_channel")->variable("V")));

    analyser->analyseModel(model);

    EXPECT_EQ_ISSUES_CELLMLELEMENTTYPES_LEVELS_REFERENCERULES_URLS(expectedIssues,
                                                                   expectedCellmlElementTypes(expectedIssues.size(), libcellml::CellmlElementType::VARIABLE),
                                                                   expectedLevels(expectedIssues.size(), libcellml::Issue::Level::MESSAGE),
                                                                   expectedReferenceRules(expectedIssues.size(), libcellml::Issue::ReferenceRule::ANALYSER_EXTERNAL_VARIABLE_USE_PRIMARY_VARIABLE),
                                                                   expectedUrls(expectedIssues.size(), "https://libcellml.org/documentation/guides/latest/runtime_codes/index?issue=ANALYSER_EXTERNAL_VARIABLE_USE_PRIMARY_VARIABLE"),
                                                                   analyser);
}

TEST(Analyser, threeEquivalentExternalVariablesNotIncludingPrimaryVariable)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    const std::vector<std::string> expectedIssues = {
        "Variable 'V' in component 'sodium_channel', variable 'V' in component 'potassium_channel' and variable 'V' in component 'leakage_current' are marked as external variables, but they are all equivalent. Variable 'V' in component 'membrane' is their corresponding primary variable and will therefore be the one used as an external variable.",
    };

    auto analyser = libcellml::Analyser::create();

    EXPECT_TRUE(analyser->addExternalVariable(model->component("sodium_channel")->variable("V")));
    EXPECT_TRUE(analyser->addExternalVariable(model->component("potassium_channel")->variable("V")));
    EXPECT_TRUE(analyser->addExternalVariable(model->component("leakage_current")->variable("V")));

    analyser->analyseModel(model);

    EXPECT_EQ_ISSUES_CELLMLELEMENTTYPES_LEVELS_REFERENCERULES_URLS(expectedIssues,
                                                                   expectedCellmlElementTypes(expectedIssues.size(), libcellml::CellmlElementType::VARIABLE),
                                                                   expectedLevels(expectedIssues.size(), libcellml::Issue::Level::MESSAGE),
                                                                   expectedReferenceRules(expectedIssues.size(), libcellml::Issue::ReferenceRule::ANALYSER_EXTERNAL_VARIABLE_USE_PRIMARY_VARIABLE),
                                                                   expectedUrls(expectedIssues.size(), "https://libcellml.org/documentation/guides/latest/runtime_codes/index?issue=ANALYSER_EXTERNAL_VARIABLE_USE_PRIMARY_VARIABLE"),
                                                                   analyser);
}

TEST(Analyser, algebraicSystemWithThreeLinkedUnknownsWithOneExternalVariable)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/algebraic_system_with_three_linked_unknowns/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    const std::vector<std::string> expectedIssues = {
        "Variable 'z' in component 'my_algebraic_system' is overconstrained.",
        "Variable 'y' in component 'my_algebraic_system' is overconstrained.",
    };

    auto analyser = libcellml::Analyser::create();

    EXPECT_TRUE(analyser->addExternalVariable(model->component("my_algebraic_system")->variable("x")));

    analyser->analyseModel(model);

    EXPECT_EQ_ISSUES_CELLMLELEMENTTYPES_LEVELS_REFERENCERULES_URLS(expectedIssues,
                                                                   expectedCellmlElementTypes(expectedIssues.size(), libcellml::CellmlElementType::VARIABLE),
                                                                   expectedLevels(expectedIssues.size(), libcellml::Issue::Level::ERROR),
                                                                   expectedReferenceRules(expectedIssues.size(), libcellml::Issue::ReferenceRule::ANALYSER_VARIABLE_OVERCONSTRAINED),
                                                                   expectedUrls(expectedIssues.size(), "https://libcellml.org/documentation/guides/latest/runtime_codes/index?issue=ANALYSER_VARIABLE_OVERCONSTRAINED"),
                                                                   analyser);

    EXPECT_EQ(libcellml::AnalyserModel::Type::OVERCONSTRAINED, analyser->analyserModel()->type());
}

TEST(Analyser, algebraicSystemWithThreeLinkedUnknownsWithTwoExternalVariables)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/algebraic_system_with_three_linked_unknowns/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    const std::vector<std::string> expectedIssues = {
        "Variable 'y' in component 'my_algebraic_system' is overconstrained.",
    };

    auto analyser = libcellml::Analyser::create();

    EXPECT_TRUE(analyser->addExternalVariable(model->component("my_algebraic_system")->variable("x")));
    EXPECT_TRUE(analyser->addExternalVariable(model->component("my_algebraic_system")->variable("z")));

    analyser->analyseModel(model);

    EXPECT_EQ_ISSUES_CELLMLELEMENTTYPES_LEVELS_REFERENCERULES_URLS(expectedIssues,
                                                                   expectedCellmlElementTypes(expectedIssues.size(), libcellml::CellmlElementType::VARIABLE),
                                                                   expectedLevels(expectedIssues.size(), libcellml::Issue::Level::ERROR),
                                                                   expectedReferenceRules(expectedIssues.size(), libcellml::Issue::ReferenceRule::ANALYSER_VARIABLE_OVERCONSTRAINED),
                                                                   expectedUrls(expectedIssues.size(), "https://libcellml.org/documentation/guides/latest/runtime_codes/index?issue=ANALYSER_VARIABLE_OVERCONSTRAINED"),
                                                                   analyser);

    EXPECT_EQ(libcellml::AnalyserModel::Type::OVERCONSTRAINED, analyser->analyserModel()->type());
}

TEST(Analyser, underconstrainedNlaSystem)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("analyser/underconstrained_nla_system.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    const std::vector<std::string> expectedIssues = {
        "Variable 'z' in component 'my_algebraic_system' is underconstrained.",
        "Variable 'y' in component 'my_algebraic_system' is underconstrained.",
        "Variable 'x' in component 'my_algebraic_system' is underconstrained.",
    };

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ_ISSUES_CELLMLELEMENTTYPES_LEVELS_REFERENCERULES_URLS(expectedIssues,
                                                                   expectedCellmlElementTypes(expectedIssues.size(), libcellml::CellmlElementType::VARIABLE),
                                                                   expectedLevels(expectedIssues.size(), libcellml::Issue::Level::ERROR),
                                                                   expectedReferenceRules(expectedIssues.size(), libcellml::Issue::ReferenceRule::ANALYSER_VARIABLE_UNDERCONSTRAINED),
                                                                   expectedUrls(expectedIssues.size(), "https://libcellml.org/documentation/guides/latest/runtime_codes/index?issue=ANALYSER_VARIABLE_UNDERCONSTRAINED"),
                                                                   analyser);

    EXPECT_EQ(libcellml::AnalyserModel::Type::UNDERCONSTRAINED, analyser->analyserModel()->type());
}

TEST(Analyser, overconstrainedNlaSystem)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("analyser/overconstrained_nla_system.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    const std::vector<std::string> expectedIssues = {
        "Variable 'z' in component 'my_algebraic_system' is overconstrained.",
        "Variable 'y' in component 'my_algebraic_system' is overconstrained.",
        "Variable 'x' in component 'my_algebraic_system' is overconstrained.",
    };

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    EXPECT_EQ_ISSUES_CELLMLELEMENTTYPES_LEVELS_REFERENCERULES_URLS(expectedIssues,
                                                                   expectedCellmlElementTypes(expectedIssues.size(), libcellml::CellmlElementType::VARIABLE),
                                                                   expectedLevels(expectedIssues.size(), libcellml::Issue::Level::ERROR),
                                                                   expectedReferenceRules(expectedIssues.size(), libcellml::Issue::ReferenceRule::ANALYSER_VARIABLE_OVERCONSTRAINED),
                                                                   expectedUrls(expectedIssues.size(), "https://libcellml.org/documentation/guides/latest/runtime_codes/index?issue=ANALYSER_VARIABLE_OVERCONSTRAINED"),
                                                                   analyser);

    EXPECT_EQ(libcellml::AnalyserModel::Type::OVERCONSTRAINED, analyser->analyserModel()->type());
}

TEST(Analyser, unsuitablyConstrainedNlaSystem)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("analyser/unsuitably_constrained_nla_system.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    const std::vector<std::string> expectedIssues = {
        "Variable 'z1' in component 'my_algebraic_system' is underconstrained.",
        "Variable 'y1' in component 'my_algebraic_system' is underconstrained.",
        "Variable 'x1' in component 'my_algebraic_system' is underconstrained.",
        "Variable 'z2' in component 'my_algebraic_system' is overconstrained.",
        "Variable 'y2' in component 'my_algebraic_system' is overconstrained.",
        "Variable 'x2' in component 'my_algebraic_system' is overconstrained.",
    };
    const std::vector<libcellml::Issue::ReferenceRule> expectedReferenceRules = {
        libcellml::Issue::ReferenceRule::ANALYSER_VARIABLE_UNDERCONSTRAINED,
        libcellml::Issue::ReferenceRule::ANALYSER_VARIABLE_UNDERCONSTRAINED,
        libcellml::Issue::ReferenceRule::ANALYSER_VARIABLE_UNDERCONSTRAINED,
        libcellml::Issue::ReferenceRule::ANALYSER_VARIABLE_OVERCONSTRAINED,
        libcellml::Issue::ReferenceRule::ANALYSER_VARIABLE_OVERCONSTRAINED,
        libcellml::Issue::ReferenceRule::ANALYSER_VARIABLE_OVERCONSTRAINED,
    };
    const std::vector<std::string> expectedUrls = {
        "https://libcellml.org/documentation/guides/latest/runtime_codes/index?issue=ANALYSER_VARIABLE_UNDERCONSTRAINED",
        "https://libcellml.org/documentation/guides/latest/runtime_codes/index?issue=ANALYSER_VARIABLE_UNDERCONSTRAINED",
        "https://libcellml.org/documentation/guides/latest/runtime_codes/index?issue=ANALYSER_VARIABLE_UNDERCONSTRAINED",
        "https://libcellml.org/documentation/guides/latest/runtime_codes/index?issue=ANALYSER_VARIABLE_OVERCONSTRAINED",
        "https://libcellml.org/documentation/guides/latest/runtime_codes/index?issue=ANALYSER_VARIABLE_OVERCONSTRAINED",
        "https://libcellml.org/documentation/guides/latest/runtime_codes/index?issue=ANALYSER_VARIABLE_OVERCONSTRAINED",
    };

    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);
    printIssues(analyser);

    EXPECT_EQ_ISSUES_CELLMLELEMENTTYPES_LEVELS_REFERENCERULES_URLS(expectedIssues,
                                                                   expectedCellmlElementTypes(expectedIssues.size(), libcellml::CellmlElementType::VARIABLE),
                                                                   expectedLevels(expectedIssues.size(), libcellml::Issue::Level::ERROR),
                                                                   expectedReferenceRules,
                                                                   expectedUrls,
                                                                   analyser);

    EXPECT_EQ(libcellml::AnalyserModel::Type::UNSUITABLY_CONSTRAINED, analyser->analyserModel()->type());
}
