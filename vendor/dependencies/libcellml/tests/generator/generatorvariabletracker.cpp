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

TEST(GeneratorVariableTracker, noModelOrVariable)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));
    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    auto analyserModel = analyser->analyserModel();
    auto generatorVariableTracker = libcellml::GeneratorVariableTracker::create();

    EXPECT_FALSE(generatorVariableTracker->isTrackedVariable(nullptr));
    EXPECT_FALSE(generatorVariableTracker->isUntrackedVariable(nullptr));

    const std::vector<std::string> nullVariableIssue = {"The variable is null."};
    const std::vector<libcellml::Issue::Level> errorLevel = {libcellml::Issue::Level::ERROR};
    const std::vector<libcellml::Issue::ReferenceRule> nullVariableReferenceRule = {libcellml::Issue::ReferenceRule::GENERATOR_NULL_VARIABLE};

    generatorVariableTracker->trackVariable(nullptr);
    EXPECT_EQ_ISSUES_LEVELS_REFERENCERULES(nullVariableIssue, errorLevel, nullVariableReferenceRule, generatorVariableTracker);

    generatorVariableTracker->untrackVariable(nullptr);
    EXPECT_EQ_ISSUES_LEVELS_REFERENCERULES(nullVariableIssue, errorLevel, nullVariableReferenceRule, generatorVariableTracker);

    const std::vector<std::string> nullModelIssue = {"The model is null."};
    const std::vector<libcellml::Issue::ReferenceRule> nullModelReferenceRule = {libcellml::Issue::ReferenceRule::GENERATOR_NULL_MODEL};

    generatorVariableTracker->trackAllConstants(nullptr);
    EXPECT_EQ_ISSUES_LEVELS_REFERENCERULES(nullModelIssue, errorLevel, nullModelReferenceRule, generatorVariableTracker);

    generatorVariableTracker->untrackAllConstants(nullptr);
    EXPECT_EQ_ISSUES_LEVELS_REFERENCERULES(nullModelIssue, errorLevel, nullModelReferenceRule, generatorVariableTracker);

    generatorVariableTracker->trackAllComputedConstants(nullptr);
    EXPECT_EQ_ISSUES_LEVELS_REFERENCERULES(nullModelIssue, errorLevel, nullModelReferenceRule, generatorVariableTracker);

    generatorVariableTracker->untrackAllComputedConstants(nullptr);
    EXPECT_EQ_ISSUES_LEVELS_REFERENCERULES(nullModelIssue, errorLevel, nullModelReferenceRule, generatorVariableTracker);

    generatorVariableTracker->trackAllAlgebraicVariables(nullptr);
    EXPECT_EQ_ISSUES_LEVELS_REFERENCERULES(nullModelIssue, errorLevel, nullModelReferenceRule, generatorVariableTracker);

    generatorVariableTracker->untrackAllAlgebraicVariables(nullptr);
    EXPECT_EQ_ISSUES_LEVELS_REFERENCERULES(nullModelIssue, errorLevel, nullModelReferenceRule, generatorVariableTracker);

    generatorVariableTracker->trackAllVariables(nullptr);
    EXPECT_EQ_ISSUES_LEVELS_REFERENCERULES(nullModelIssue, errorLevel, nullModelReferenceRule, generatorVariableTracker);

    generatorVariableTracker->untrackAllVariables(nullptr);
    EXPECT_EQ_ISSUES_LEVELS_REFERENCERULES(nullModelIssue, errorLevel, nullModelReferenceRule, generatorVariableTracker);

    EXPECT_EQ(size_t(0), generatorVariableTracker->trackedConstantCount(nullptr));
    EXPECT_EQ(size_t(0), generatorVariableTracker->untrackedConstantCount(nullptr));

    EXPECT_EQ(size_t(0), generatorVariableTracker->trackedComputedConstantCount(nullptr));
    EXPECT_EQ(size_t(0), generatorVariableTracker->untrackedComputedConstantCount(nullptr));

    EXPECT_EQ(size_t(0), generatorVariableTracker->trackedAlgebraicVariableCount(nullptr));
    EXPECT_EQ(size_t(0), generatorVariableTracker->untrackedAlgebraicVariableCount(nullptr));

    EXPECT_EQ(size_t(0), generatorVariableTracker->trackedVariableCount(nullptr));
    EXPECT_EQ(size_t(0), generatorVariableTracker->untrackedVariableCount(nullptr));
}

TEST(GeneratorVariableTracker, tracAndkUntrackVariableOfIntegration)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));
    auto analyser = libcellml::Analyser::create();

    analyser->addExternalVariable(libcellml::AnalyserExternalVariable::create(model->component("membrane")->variable("V")));
    // Note: this external variable helps with our coverage since we check for the variable of integration last in
    //       Generator::GeneratorImpl::trackVariable().

    analyser->analyseModel(model);

    auto analyserModel = analyser->analyserModel();
    auto generatorVariableTracker = libcellml::GeneratorVariableTracker::create();

    auto variable = model->component("environment")->variable("time");

    EXPECT_TRUE(generatorVariableTracker->isTrackedVariable(analyserModel->analyserVariable(variable)));
    EXPECT_FALSE(generatorVariableTracker->isUntrackedVariable(analyserModel->analyserVariable(variable)));

    EXPECT_EQ(size_t(18), generatorVariableTracker->trackedVariableCount(analyserModel));
    EXPECT_EQ(size_t(0), generatorVariableTracker->untrackedVariableCount(analyserModel));

    generatorVariableTracker->untrackVariable(analyserModel->analyserVariable(variable));
    EXPECT_EQ_ISSUES({"Variable 'time' in component 'environment' is the variable of integration and cannot therefore be untracked."}, generatorVariableTracker);

    EXPECT_TRUE(generatorVariableTracker->isTrackedVariable(analyserModel->analyserVariable(variable)));
    EXPECT_FALSE(generatorVariableTracker->isUntrackedVariable(analyserModel->analyserVariable(variable)));

    EXPECT_EQ(size_t(18), generatorVariableTracker->trackedVariableCount(analyserModel));
    EXPECT_EQ(size_t(0), generatorVariableTracker->untrackedVariableCount(analyserModel));

    generatorVariableTracker->trackVariable(analyserModel->analyserVariable(variable));
    EXPECT_EQ_ISSUES({"Variable 'time' in component 'environment' is the variable of integration and is therefore always tracked."}, generatorVariableTracker);

    EXPECT_EQ(size_t(18), generatorVariableTracker->trackedVariableCount(analyserModel));
    EXPECT_EQ(size_t(0), generatorVariableTracker->untrackedVariableCount(analyserModel));
}

TEST(GeneratorVariableTracker, trackAndUntrackStateVariable)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));
    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    auto analyserModel = analyser->analyserModel();
    auto generatorVariableTracker = libcellml::GeneratorVariableTracker::create();

    auto variable = model->component("membrane")->variable("V");

    EXPECT_TRUE(generatorVariableTracker->isTrackedVariable(analyserModel->analyserVariable(variable)));
    EXPECT_FALSE(generatorVariableTracker->isUntrackedVariable(analyserModel->analyserVariable(variable)));

    EXPECT_EQ(size_t(18), generatorVariableTracker->trackedVariableCount(analyserModel));
    EXPECT_EQ(size_t(0), generatorVariableTracker->untrackedVariableCount(analyserModel));

    generatorVariableTracker->untrackVariable(analyserModel->analyserVariable(variable));
    EXPECT_EQ_ISSUES({"Variable 'V' in component 'membrane' is a state variable and cannot therefore be untracked."}, generatorVariableTracker);

    EXPECT_TRUE(generatorVariableTracker->isTrackedVariable(analyserModel->analyserVariable(variable)));
    EXPECT_FALSE(generatorVariableTracker->isUntrackedVariable(analyserModel->analyserVariable(variable)));

    EXPECT_EQ(size_t(18), generatorVariableTracker->trackedVariableCount(analyserModel));
    EXPECT_EQ(size_t(0), generatorVariableTracker->untrackedVariableCount(analyserModel));

    generatorVariableTracker->trackVariable(analyserModel->analyserVariable(variable));
    EXPECT_EQ_ISSUES({"Variable 'V' in component 'membrane' is a state variable and is therefore always tracked."}, generatorVariableTracker);

    EXPECT_EQ(size_t(18), generatorVariableTracker->trackedVariableCount(analyserModel));
    EXPECT_EQ(size_t(0), generatorVariableTracker->untrackedVariableCount(analyserModel));
}

TEST(GeneratorVariableTracker, trackAndUntrackConstant)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));
    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    auto analyserModel = analyser->analyserModel();
    auto generatorVariableTracker = libcellml::GeneratorVariableTracker::create();

    auto variable = model->component("membrane")->variable("Cm");

    EXPECT_TRUE(generatorVariableTracker->isTrackedVariable(analyserModel->analyserVariable(variable)));
    EXPECT_FALSE(generatorVariableTracker->isUntrackedVariable(analyserModel->analyserVariable(variable)));

    EXPECT_EQ(size_t(18), generatorVariableTracker->trackedVariableCount(analyserModel));
    EXPECT_EQ(size_t(0), generatorVariableTracker->untrackedVariableCount(analyserModel));

    generatorVariableTracker->untrackVariable(analyserModel->analyserVariable(variable));
    EXPECT_EQ_ISSUES_LEVELS_REFERENCERULES({}, {}, {}, generatorVariableTracker);

    EXPECT_FALSE(generatorVariableTracker->isTrackedVariable(analyserModel->analyserVariable(variable)));
    EXPECT_TRUE(generatorVariableTracker->isUntrackedVariable(analyserModel->analyserVariable(variable)));

    EXPECT_EQ(size_t(17), generatorVariableTracker->trackedVariableCount(analyserModel));
    EXPECT_EQ(size_t(1), generatorVariableTracker->untrackedVariableCount(analyserModel));

    generatorVariableTracker->trackVariable(analyserModel->analyserVariable(variable));
    EXPECT_EQ_ISSUES_LEVELS_REFERENCERULES({}, {}, {}, generatorVariableTracker);

    EXPECT_EQ(size_t(18), generatorVariableTracker->trackedVariableCount(analyserModel));
    EXPECT_EQ(size_t(0), generatorVariableTracker->untrackedVariableCount(analyserModel));
}

TEST(GeneratorVariableTracker, trackAndUntrackComputedConstant)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));
    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    auto analyserModel = analyser->analyserModel();
    auto generatorVariableTracker = libcellml::GeneratorVariableTracker::create();

    auto variable = model->component("leakage_current")->variable("E_L");

    EXPECT_TRUE(generatorVariableTracker->isTrackedVariable(analyserModel->analyserVariable(variable)));
    EXPECT_FALSE(generatorVariableTracker->isUntrackedVariable(analyserModel->analyserVariable(variable)));

    EXPECT_EQ(size_t(18), generatorVariableTracker->trackedVariableCount(analyserModel));
    EXPECT_EQ(size_t(0), generatorVariableTracker->untrackedVariableCount(analyserModel));

    generatorVariableTracker->untrackVariable(analyserModel->analyserVariable(variable));
    EXPECT_EQ_ISSUES_LEVELS_REFERENCERULES({}, {}, {}, generatorVariableTracker);

    EXPECT_FALSE(generatorVariableTracker->isTrackedVariable(analyserModel->analyserVariable(variable)));
    EXPECT_TRUE(generatorVariableTracker->isUntrackedVariable(analyserModel->analyserVariable(variable)));

    EXPECT_EQ(size_t(17), generatorVariableTracker->trackedVariableCount(analyserModel));
    EXPECT_EQ(size_t(1), generatorVariableTracker->untrackedVariableCount(analyserModel));

    generatorVariableTracker->trackVariable(analyserModel->analyserVariable(variable));
    EXPECT_EQ_ISSUES_LEVELS_REFERENCERULES({}, {}, {}, generatorVariableTracker);

    EXPECT_EQ(size_t(18), generatorVariableTracker->trackedVariableCount(analyserModel));
    EXPECT_EQ(size_t(0), generatorVariableTracker->untrackedVariableCount(analyserModel));
}

TEST(GeneratorVariableTracker, trackAndUntrackAlgebraicVariable)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));
    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    auto analyserModel = analyser->analyserModel();
    auto generatorVariableTracker = libcellml::GeneratorVariableTracker::create();

    auto variable = model->component("membrane")->variable("i_Stim");

    EXPECT_TRUE(generatorVariableTracker->isTrackedVariable(analyserModel->analyserVariable(variable)));
    EXPECT_FALSE(generatorVariableTracker->isUntrackedVariable(analyserModel->analyserVariable(variable)));

    EXPECT_EQ(size_t(18), generatorVariableTracker->trackedVariableCount(analyserModel));
    EXPECT_EQ(size_t(0), generatorVariableTracker->untrackedVariableCount(analyserModel));

    generatorVariableTracker->untrackVariable(analyserModel->analyserVariable(variable));
    EXPECT_EQ_ISSUES_LEVELS_REFERENCERULES({}, {}, {}, generatorVariableTracker);

    EXPECT_FALSE(generatorVariableTracker->isTrackedVariable(analyserModel->analyserVariable(variable)));
    EXPECT_TRUE(generatorVariableTracker->isUntrackedVariable(analyserModel->analyserVariable(variable)));

    EXPECT_EQ(size_t(17), generatorVariableTracker->trackedVariableCount(analyserModel));
    EXPECT_EQ(size_t(1), generatorVariableTracker->untrackedVariableCount(analyserModel));

    generatorVariableTracker->trackVariable(analyserModel->analyserVariable(variable));
    EXPECT_EQ_ISSUES_LEVELS_REFERENCERULES({}, {}, {}, generatorVariableTracker);

    EXPECT_EQ(size_t(18), generatorVariableTracker->trackedVariableCount(analyserModel));
    EXPECT_EQ(size_t(0), generatorVariableTracker->untrackedVariableCount(analyserModel));
}

TEST(GeneratorVariableTracker, trackAndUntrackExternalVariable)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));
    auto analyser = libcellml::Analyser::create();

    analyser->addExternalVariable(libcellml::AnalyserExternalVariable::create(model->component("membrane")->variable("V")));

    analyser->analyseModel(model);

    auto analyserModel = analyser->analyserModel();
    auto generatorVariableTracker = libcellml::GeneratorVariableTracker::create();

    auto variable = model->component("membrane")->variable("V");

    EXPECT_TRUE(generatorVariableTracker->isTrackedVariable(analyserModel->analyserVariable(variable)));
    EXPECT_FALSE(generatorVariableTracker->isUntrackedVariable(analyserModel->analyserVariable(variable)));

    EXPECT_EQ(size_t(18), generatorVariableTracker->trackedVariableCount(analyserModel));
    EXPECT_EQ(size_t(0), generatorVariableTracker->untrackedVariableCount(analyserModel));

    generatorVariableTracker->untrackVariable(analyserModel->analyserVariable(variable));
    EXPECT_EQ_ISSUES({"Variable 'V' in component 'membrane' is an external variable and cannot therefore be untracked."}, generatorVariableTracker);

    EXPECT_TRUE(generatorVariableTracker->isTrackedVariable(analyserModel->analyserVariable(variable)));
    EXPECT_FALSE(generatorVariableTracker->isUntrackedVariable(analyserModel->analyserVariable(variable)));

    EXPECT_EQ(size_t(18), generatorVariableTracker->trackedVariableCount(analyserModel));
    EXPECT_EQ(size_t(0), generatorVariableTracker->untrackedVariableCount(analyserModel));

    generatorVariableTracker->trackVariable(analyserModel->analyserVariable(variable));
    EXPECT_EQ_ISSUES({"Variable 'V' in component 'membrane' is an external variable and is therefore always tracked."}, generatorVariableTracker);

    EXPECT_EQ(size_t(18), generatorVariableTracker->trackedVariableCount(analyserModel));
    EXPECT_EQ(size_t(0), generatorVariableTracker->untrackedVariableCount(analyserModel));
}

TEST(GeneratorVariableTracker, trackAndUntrackVariableFromOtherModel)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));
    auto otherModel = parser->parseModel(fileContents("generator/noble_model_1962/model.cellml"));
    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    auto analyserModel = analyser->analyserModel();
    auto generatorVariableTracker = libcellml::GeneratorVariableTracker::create();

    auto otherVariable = otherModel->component("membrane")->variable("Cm");

    generatorVariableTracker->trackVariable(analyserModel->analyserVariable(otherVariable));
    EXPECT_EQ_ISSUES({"The variable is null."}, generatorVariableTracker);

    generatorVariableTracker->untrackVariable(analyserModel->analyserVariable(otherVariable));
    EXPECT_EQ_ISSUES({"The variable is null."}, generatorVariableTracker);
}

TEST(GeneratorVariableTracker, trackAndUntrackAllConstants)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));
    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    auto analyserModel = analyser->analyserModel();
    auto generatorVariableTracker = libcellml::GeneratorVariableTracker::create();

    EXPECT_EQ(size_t(5), generatorVariableTracker->trackedConstantCount(analyserModel));
    EXPECT_EQ(size_t(0), generatorVariableTracker->untrackedConstantCount(analyserModel));
    EXPECT_EQ(size_t(18), generatorVariableTracker->trackedVariableCount(analyserModel));
    EXPECT_EQ(size_t(0), generatorVariableTracker->untrackedVariableCount(analyserModel));

    generatorVariableTracker->untrackAllConstants(analyserModel);
    EXPECT_EQ_ISSUES_LEVELS_REFERENCERULES({}, {}, {}, generatorVariableTracker);

    EXPECT_EQ(size_t(0), generatorVariableTracker->trackedConstantCount(analyserModel));
    EXPECT_EQ(size_t(5), generatorVariableTracker->untrackedConstantCount(analyserModel));
    EXPECT_EQ(size_t(13), generatorVariableTracker->trackedVariableCount(analyserModel));
    EXPECT_EQ(size_t(5), generatorVariableTracker->untrackedVariableCount(analyserModel));

    generatorVariableTracker->trackAllConstants(analyserModel);
    EXPECT_EQ_ISSUES_LEVELS_REFERENCERULES({}, {}, {}, generatorVariableTracker);

    EXPECT_EQ(size_t(5), generatorVariableTracker->trackedConstantCount(analyserModel));
    EXPECT_EQ(size_t(0), generatorVariableTracker->untrackedConstantCount(analyserModel));
    EXPECT_EQ(size_t(18), generatorVariableTracker->trackedVariableCount(analyserModel));
    EXPECT_EQ(size_t(0), generatorVariableTracker->untrackedVariableCount(analyserModel));
}

TEST(GeneratorVariableTracker, trackAndUntrackAllComputedConstants)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));
    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    auto analyserModel = analyser->analyserModel();
    auto generatorVariableTracker = libcellml::GeneratorVariableTracker::create();

    EXPECT_EQ(size_t(3), generatorVariableTracker->trackedComputedConstantCount(analyserModel));
    EXPECT_EQ(size_t(0), generatorVariableTracker->untrackedComputedConstantCount(analyserModel));
    EXPECT_EQ(size_t(18), generatorVariableTracker->trackedVariableCount(analyserModel));
    EXPECT_EQ(size_t(0), generatorVariableTracker->untrackedVariableCount(analyserModel));

    generatorVariableTracker->untrackAllComputedConstants(analyserModel);
    EXPECT_EQ_ISSUES_LEVELS_REFERENCERULES({}, {}, {}, generatorVariableTracker);

    EXPECT_EQ(size_t(0), generatorVariableTracker->trackedComputedConstantCount(analyserModel));
    EXPECT_EQ(size_t(3), generatorVariableTracker->untrackedComputedConstantCount(analyserModel));
    EXPECT_EQ(size_t(15), generatorVariableTracker->trackedVariableCount(analyserModel));
    EXPECT_EQ(size_t(3), generatorVariableTracker->untrackedVariableCount(analyserModel));

    generatorVariableTracker->trackAllComputedConstants(analyserModel);
    EXPECT_EQ_ISSUES_LEVELS_REFERENCERULES({}, {}, {}, generatorVariableTracker);

    EXPECT_EQ(size_t(3), generatorVariableTracker->trackedComputedConstantCount(analyserModel));
    EXPECT_EQ(size_t(0), generatorVariableTracker->untrackedComputedConstantCount(analyserModel));
    EXPECT_EQ(size_t(18), generatorVariableTracker->trackedVariableCount(analyserModel));
    EXPECT_EQ(size_t(0), generatorVariableTracker->untrackedVariableCount(analyserModel));
}

TEST(GeneratorVariableTracker, trackAndUntrackAllAlgebraicVariables)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));
    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    auto analyserModel = analyser->analyserModel();
    auto generatorVariableTracker = libcellml::GeneratorVariableTracker::create();

    EXPECT_EQ(size_t(10), generatorVariableTracker->trackedAlgebraicVariableCount(analyserModel));
    EXPECT_EQ(size_t(0), generatorVariableTracker->untrackedAlgebraicVariableCount(analyserModel));
    EXPECT_EQ(size_t(18), generatorVariableTracker->trackedVariableCount(analyserModel));
    EXPECT_EQ(size_t(0), generatorVariableTracker->untrackedVariableCount(analyserModel));

    generatorVariableTracker->untrackAllAlgebraicVariables(analyserModel);
    EXPECT_EQ_ISSUES_LEVELS_REFERENCERULES({}, {}, {}, generatorVariableTracker);

    EXPECT_EQ(size_t(0), generatorVariableTracker->trackedAlgebraicVariableCount(analyserModel));
    EXPECT_EQ(size_t(10), generatorVariableTracker->untrackedAlgebraicVariableCount(analyserModel));
    EXPECT_EQ(size_t(8), generatorVariableTracker->trackedVariableCount(analyserModel));
    EXPECT_EQ(size_t(10), generatorVariableTracker->untrackedVariableCount(analyserModel));

    generatorVariableTracker->trackAllAlgebraicVariables(analyserModel);
    EXPECT_EQ_ISSUES_LEVELS_REFERENCERULES({}, {}, {}, generatorVariableTracker);

    EXPECT_EQ(size_t(10), generatorVariableTracker->trackedAlgebraicVariableCount(analyserModel));
    EXPECT_EQ(size_t(0), generatorVariableTracker->untrackedAlgebraicVariableCount(analyserModel));
    EXPECT_EQ(size_t(18), generatorVariableTracker->trackedVariableCount(analyserModel));
    EXPECT_EQ(size_t(0), generatorVariableTracker->untrackedVariableCount(analyserModel));
}

TEST(GeneratorVariableTracker, trackAndUntrackAllVariables)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));
    auto analyser = libcellml::Analyser::create();

    analyser->analyseModel(model);

    auto analyserModel = analyser->analyserModel();
    auto generatorVariableTracker = libcellml::GeneratorVariableTracker::create();

    EXPECT_EQ(size_t(18), generatorVariableTracker->trackedVariableCount(analyserModel));
    EXPECT_EQ(size_t(0), generatorVariableTracker->untrackedVariableCount(analyserModel));

    generatorVariableTracker->untrackAllVariables(analyserModel);
    EXPECT_EQ_ISSUES_LEVELS_REFERENCERULES({}, {}, {}, generatorVariableTracker);

    EXPECT_EQ(size_t(0), generatorVariableTracker->trackedVariableCount(analyserModel));
    EXPECT_EQ(size_t(18), generatorVariableTracker->untrackedVariableCount(analyserModel));

    generatorVariableTracker->trackAllVariables(analyserModel);
    EXPECT_EQ_ISSUES_LEVELS_REFERENCERULES({}, {}, {}, generatorVariableTracker);

    EXPECT_EQ(size_t(18), generatorVariableTracker->trackedVariableCount(analyserModel));
    EXPECT_EQ(size_t(0), generatorVariableTracker->untrackedVariableCount(analyserModel));
}

enum class TrackingType
{
    CONTROL,
    VARIABLES,
    CONSTANTS,
    COMPUTED_CONSTANTS,
    ALGEBRAIC
};

void untrack(const libcellml::AnalyserModelPtr &analyserModel, const libcellml::GeneratorVariableTrackerPtr &generatorVariableTracker, TrackingType trackingType)
{
    switch (trackingType) {
    case TrackingType::VARIABLES:
        generatorVariableTracker->trackAllVariables(analyserModel); // For coverage.
        generatorVariableTracker->untrackAllVariables(analyserModel);

        break;
    case TrackingType::CONSTANTS:
        generatorVariableTracker->untrackAllConstants(analyserModel);

        break;
    case TrackingType::COMPUTED_CONSTANTS:
        generatorVariableTracker->untrackAllComputedConstants(analyserModel);

        break;
    case TrackingType::ALGEBRAIC:
        generatorVariableTracker->untrackAllAlgebraicVariables(analyserModel);

        break;
    default: // CONTROL.
        break;
    }
}

void hodgkinHuxleySquidAxonModel1952CodeGeneration(bool ode, TrackingType trackingType,
                                                   const std::vector<std::string> &issues,
                                                   const std::vector<libcellml::Issue::Level> &levels,
                                                   const std::vector<libcellml::Issue::ReferenceRule> &referenceRules,
                                                   const std::vector<std::string> &externalIssues,
                                                   const std::vector<libcellml::Issue::Level> &externalLevels,
                                                   const std::vector<libcellml::Issue::ReferenceRule> &externalReferenceRules)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents(std::string("generator/hodgkin_huxley_squid_axon_model_1952/model") + (ode ? "" : ".dae.for.tracking") + ".cellml"));
    auto analyser = libcellml::Analyser::create();
    auto generator = libcellml::Generator::create();
    auto generatorVariableTracker = libcellml::GeneratorVariableTracker::create();
    std::string modelType = ode ? "model" : "model.dae.for.tracking";
    std::string variableType = (trackingType == TrackingType::VARIABLES) ?
                                   "untracked.variables" :
                                   ((trackingType == TrackingType::CONSTANTS) ?
                                        "untracked.constants" :
                                        ((trackingType == TrackingType::COMPUTED_CONSTANTS) ?
                                             "untracked.computed.constants" :
                                             ((trackingType == TrackingType::ALGEBRAIC) ?
                                                  "untracked.algebraic.variables" :
                                                  "control")));

    analyser->analyseModel(model);

    auto analyserModel = analyser->analyserModel();

    untrack(analyserModel, generatorVariableTracker, trackingType);

    EXPECT_EQ_ISSUES_LEVELS_REFERENCERULES(issues, levels, referenceRules, generatorVariableTracker);

    auto profile = libcellml::GeneratorProfile::create();

    profile->setInterfaceFileNameString(modelType + "." + variableType + ".h");

    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/" + modelType + "." + variableType + ".h", generator->interfaceCode(analyserModel, profile, generatorVariableTracker));
    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/" + modelType + "." + variableType + ".c", generator->implementationCode(analyserModel, profile, generatorVariableTracker));

    profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/" + modelType + "." + variableType + ".py", generator->implementationCode(analyserModel, profile, generatorVariableTracker));

    // With an external variable with a dependency on a constant, computed constant, and algebraic variable.

    auto membrane_Cm = model->component("membrane")->variable("Cm"); // Constant.
    auto potassium_channel_EK = model->component("potassium_channel")->variable("E_K"); // Computed constant.
    auto sodium_channel_m_gate_alapha_m = model->component("sodium_channel_m_gate")->variable("alpha_m"); // Algebraic variable.
    auto external_sodium_channel_i_Na = libcellml::AnalyserExternalVariable::create(model->component("sodium_channel")->variable("i_Na"));

    external_sodium_channel_i_Na->addDependency(membrane_Cm);
    external_sodium_channel_i_Na->addDependency(potassium_channel_EK);
    external_sodium_channel_i_Na->addDependency(sodium_channel_m_gate_alapha_m);

    analyser->addExternalVariable(external_sodium_channel_i_Na);

    analyser->analyseModel(model);

    analyserModel = analyser->analyserModel();

    untrack(analyserModel, generatorVariableTracker, trackingType);

    EXPECT_EQ_ISSUES_LEVELS_REFERENCERULES(externalIssues, externalLevels, externalReferenceRules, generatorVariableTracker);

    profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::C);

    profile->setInterfaceFileNameString(modelType + "." + variableType + ".with.externals.h");

    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/" + modelType + "." + variableType + ".with.externals.h", generator->interfaceCode(analyserModel, profile, generatorVariableTracker));
    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/" + modelType + "." + variableType + ".with.externals.c", generator->implementationCode(analyserModel, profile, generatorVariableTracker));

    profile = libcellml::GeneratorProfile::create(libcellml::GeneratorProfile::Profile::PYTHON);

    EXPECT_EQ_FILE_CONTENTS("generator/hodgkin_huxley_squid_axon_model_1952/" + modelType + "." + variableType + ".with.externals.py", generator->implementationCode(analyserModel, profile, generatorVariableTracker));
}

TEST(GeneratorVariableTracker, hodgkinHuxleySquidAxonModel1952Control)
{
    hodgkinHuxleySquidAxonModel1952CodeGeneration(true, TrackingType::CONSTANTS,
                                                  {}, {}, {},
                                                  {"Variable 'Cm' in component 'membrane' is needed to compute an external variable and cannot therefore be untracked."},
                                                  {libcellml::Issue::Level::ERROR},
                                                  {libcellml::Issue::ReferenceRule::GENERATOR_EXTERNALLY_NEEDED_VARIABLE_NOT_UNTRACKABLE});
}

TEST(GeneratorVariableTracker, hodgkinHuxleySquidAxonModel1952UntrackedConstants)
{
    hodgkinHuxleySquidAxonModel1952CodeGeneration(true, TrackingType::CONSTANTS,
                                                  {}, {}, {},
                                                  {"Variable 'Cm' in component 'membrane' is needed to compute an external variable and cannot therefore be untracked."},
                                                  {libcellml::Issue::Level::ERROR},
                                                  {libcellml::Issue::ReferenceRule::GENERATOR_EXTERNALLY_NEEDED_VARIABLE_NOT_UNTRACKABLE});
}

TEST(GeneratorVariableTracker, hodgkinHuxleySquidAxonModel1952UntrackedComputedConstants)
{
    hodgkinHuxleySquidAxonModel1952CodeGeneration(true, TrackingType::COMPUTED_CONSTANTS,
                                                  {}, {}, {},
                                                  {"Variable 'E_K' in component 'potassium_channel' is needed to compute an external variable and cannot therefore be untracked."},
                                                  {libcellml::Issue::Level::ERROR},
                                                  {libcellml::Issue::ReferenceRule::GENERATOR_EXTERNALLY_NEEDED_VARIABLE_NOT_UNTRACKABLE});
}

const std::vector<std::string> algebraicRelatedExternalIssues = {
    "Variable 'alpha_m' in component 'sodium_channel_m_gate' is needed to compute an external variable and cannot therefore be untracked.",
};

TEST(GeneratorVariableTracker, hodgkinHuxleySquidAxonModel1952UntrackedAlgebraicVariables)
{
    hodgkinHuxleySquidAxonModel1952CodeGeneration(true, TrackingType::ALGEBRAIC,
                                                  {}, {}, {},
                                                  {"Variable 'alpha_m' in component 'sodium_channel_m_gate' is needed to compute an external variable and cannot therefore be untracked."},
                                                  {libcellml::Issue::Level::ERROR},
                                                  {libcellml::Issue::ReferenceRule::GENERATOR_EXTERNALLY_NEEDED_VARIABLE_NOT_UNTRACKABLE});
}

TEST(GeneratorVariableTracker, hodgkinHuxleySquidAxonModel1952UntrackedVariables)
{
    hodgkinHuxleySquidAxonModel1952CodeGeneration(true, TrackingType::VARIABLES,
                                                  {}, {}, {},
                                                  {
                                                      "Variable 'Cm' in component 'membrane' is needed to compute an external variable and cannot therefore be untracked.",
                                                      "Variable 'E_K' in component 'potassium_channel' is needed to compute an external variable and cannot therefore be untracked.",
                                                      "Variable 'alpha_m' in component 'sodium_channel_m_gate' is needed to compute an external variable and cannot therefore be untracked.",
                                                  },
                                                  expectedLevels(3, libcellml::Issue::Level::ERROR),
                                                  expectedReferenceRules(3, libcellml::Issue::ReferenceRule::GENERATOR_EXTERNALLY_NEEDED_VARIABLE_NOT_UNTRACKABLE));
}

TEST(GeneratorVariableTracker, hodgkinHuxleySquidAxonModel1952DaeControl)
{
    hodgkinHuxleySquidAxonModel1952CodeGeneration(false, TrackingType::CONTROL,
                                                  {}, {}, {},
                                                  {}, {}, {});
}

TEST(GeneratorVariableTracker, hodgkinHuxleySquidAxonModel1952DaeUntrackedConstants)
{
    hodgkinHuxleySquidAxonModel1952CodeGeneration(false, TrackingType::CONSTANTS,
                                                  {}, {}, {},
                                                  {"Variable 'Cm' in component 'membrane' is needed to compute an external variable and cannot therefore be untracked."},
                                                  {libcellml::Issue::Level::ERROR},
                                                  {libcellml::Issue::ReferenceRule::GENERATOR_EXTERNALLY_NEEDED_VARIABLE_NOT_UNTRACKABLE});
}

TEST(GeneratorVariableTracker, hodgkinHuxleySquidAxonModel1952DaeUntrackedComputedConstants)
{
    hodgkinHuxleySquidAxonModel1952CodeGeneration(false, TrackingType::COMPUTED_CONSTANTS,
                                                  {}, {}, {},
                                                  {"Variable 'E_K' in component 'potassium_channel' is needed to compute an external variable and cannot therefore be untracked."},
                                                  {libcellml::Issue::Level::ERROR},
                                                  {libcellml::Issue::ReferenceRule::GENERATOR_EXTERNALLY_NEEDED_VARIABLE_NOT_UNTRACKABLE});
}

const std::vector<std::string> daeIssues = {
    "Variable 'i_Stim' in component 'membrane' is computed using an NLA system and cannot therefore be untracked.",
    "Variable 'i_L' in component 'leakage_current' is computed using an NLA system and cannot therefore be untracked.",
    "Variable 'i_K' in component 'potassium_channel' is computed using an NLA system and cannot therefore be untracked.",
    "Variable 'i_Na' in component 'sodium_channel' is computed using an NLA system and cannot therefore be untracked.",
    "Variable 'E_L' in component 'leakage_current' is computed using an NLA system and cannot therefore be untracked.",
    "Variable 'E_Na' in component 'sodium_channel' is computed using an NLA system and cannot therefore be untracked.",
    "Variable 'beta_m' in component 'sodium_channel_m_gate' is computed using an NLA system and cannot therefore be untracked.",
    "Variable 'alpha_h' in component 'sodium_channel_h_gate' is computed using an NLA system and cannot therefore be untracked.",
    "Variable 'beta_h' in component 'sodium_channel_h_gate' is computed using an NLA system and cannot therefore be untracked.",
    "Variable 'alpha_n' in component 'potassium_channel_n_gate' is computed using an NLA system and cannot therefore be untracked.",
    "Variable 'beta_n' in component 'potassium_channel_n_gate' is computed using an NLA system and cannot therefore be untracked.",
};
const std::vector<libcellml::Issue::Level> daeLevels = expectedLevels(daeIssues.size(), libcellml::Issue::Level::ERROR);
const std::vector<libcellml::Issue::ReferenceRule> daeReferenceRules = expectedReferenceRules(daeIssues.size(), libcellml::Issue::ReferenceRule::GENERATOR_NLA_BASED_VARIABLE_NOT_UNTRACKABLE);

TEST(GeneratorVariableTracker, hodgkinHuxleySquidAxonModel1952DaeUntrackedAlgebraicVariables)
{
    const std::vector<std::string> daeExternalIssues = {
        "Variable 'i_Stim' in component 'membrane' is computed using an NLA system and cannot therefore be untracked.",
        "Variable 'i_L' in component 'leakage_current' is computed using an NLA system and cannot therefore be untracked.",
        "Variable 'i_K' in component 'potassium_channel' is computed using an NLA system and cannot therefore be untracked.",
        "Variable 'E_L' in component 'leakage_current' is computed using an NLA system and cannot therefore be untracked.",
        "Variable 'E_Na' in component 'sodium_channel' is computed using an NLA system and cannot therefore be untracked.",
        "Variable 'alpha_m' in component 'sodium_channel_m_gate' is needed to compute an external variable and cannot therefore be untracked.",
        "Variable 'beta_m' in component 'sodium_channel_m_gate' is computed using an NLA system and cannot therefore be untracked.",
        "Variable 'alpha_h' in component 'sodium_channel_h_gate' is computed using an NLA system and cannot therefore be untracked.",
        "Variable 'beta_h' in component 'sodium_channel_h_gate' is computed using an NLA system and cannot therefore be untracked.",
        "Variable 'alpha_n' in component 'potassium_channel_n_gate' is computed using an NLA system and cannot therefore be untracked.",
        "Variable 'beta_n' in component 'potassium_channel_n_gate' is computed using an NLA system and cannot therefore be untracked.",
    };
    const std::vector<libcellml::Issue::ReferenceRule> daeExternalReferenceRules = {
        libcellml::Issue::ReferenceRule::GENERATOR_NLA_BASED_VARIABLE_NOT_UNTRACKABLE,
        libcellml::Issue::ReferenceRule::GENERATOR_NLA_BASED_VARIABLE_NOT_UNTRACKABLE,
        libcellml::Issue::ReferenceRule::GENERATOR_NLA_BASED_VARIABLE_NOT_UNTRACKABLE,
        libcellml::Issue::ReferenceRule::GENERATOR_NLA_BASED_VARIABLE_NOT_UNTRACKABLE,
        libcellml::Issue::ReferenceRule::GENERATOR_NLA_BASED_VARIABLE_NOT_UNTRACKABLE,
        libcellml::Issue::ReferenceRule::GENERATOR_EXTERNALLY_NEEDED_VARIABLE_NOT_UNTRACKABLE,
        libcellml::Issue::ReferenceRule::GENERATOR_NLA_BASED_VARIABLE_NOT_UNTRACKABLE,
        libcellml::Issue::ReferenceRule::GENERATOR_NLA_BASED_VARIABLE_NOT_UNTRACKABLE,
        libcellml::Issue::ReferenceRule::GENERATOR_NLA_BASED_VARIABLE_NOT_UNTRACKABLE,
        libcellml::Issue::ReferenceRule::GENERATOR_NLA_BASED_VARIABLE_NOT_UNTRACKABLE,
        libcellml::Issue::ReferenceRule::GENERATOR_NLA_BASED_VARIABLE_NOT_UNTRACKABLE,
    };

    hodgkinHuxleySquidAxonModel1952CodeGeneration(false, TrackingType::ALGEBRAIC,
                                                  daeIssues, daeLevels, daeReferenceRules,
                                                  daeExternalIssues, expectedLevels(daeExternalIssues.size(), libcellml::Issue::Level::ERROR), daeExternalReferenceRules);
}

TEST(GeneratorVariableTracker, hodgkinHuxleySquidAxonModel1952DaeUntrackedVariables)
{
    const std::vector<std::string> daeExternalIssues = {
        "Variable 'Cm' in component 'membrane' is needed to compute an external variable and cannot therefore be untracked.",
        "Variable 'E_K' in component 'potassium_channel' is needed to compute an external variable and cannot therefore be untracked.",
        "Variable 'i_Stim' in component 'membrane' is computed using an NLA system and cannot therefore be untracked.",
        "Variable 'i_L' in component 'leakage_current' is computed using an NLA system and cannot therefore be untracked.",
        "Variable 'i_K' in component 'potassium_channel' is computed using an NLA system and cannot therefore be untracked.",
        "Variable 'E_L' in component 'leakage_current' is computed using an NLA system and cannot therefore be untracked.",
        "Variable 'E_Na' in component 'sodium_channel' is computed using an NLA system and cannot therefore be untracked.",
        "Variable 'alpha_m' in component 'sodium_channel_m_gate' is needed to compute an external variable and cannot therefore be untracked.",
        "Variable 'beta_m' in component 'sodium_channel_m_gate' is computed using an NLA system and cannot therefore be untracked.",
        "Variable 'alpha_h' in component 'sodium_channel_h_gate' is computed using an NLA system and cannot therefore be untracked.",
        "Variable 'beta_h' in component 'sodium_channel_h_gate' is computed using an NLA system and cannot therefore be untracked.",
        "Variable 'alpha_n' in component 'potassium_channel_n_gate' is computed using an NLA system and cannot therefore be untracked.",
        "Variable 'beta_n' in component 'potassium_channel_n_gate' is computed using an NLA system and cannot therefore be untracked.",
    };
    const std::vector<libcellml::Issue::ReferenceRule> daeExternalReferenceRules = {
        libcellml::Issue::ReferenceRule::GENERATOR_EXTERNALLY_NEEDED_VARIABLE_NOT_UNTRACKABLE,
        libcellml::Issue::ReferenceRule::GENERATOR_EXTERNALLY_NEEDED_VARIABLE_NOT_UNTRACKABLE,
        libcellml::Issue::ReferenceRule::GENERATOR_NLA_BASED_VARIABLE_NOT_UNTRACKABLE,
        libcellml::Issue::ReferenceRule::GENERATOR_NLA_BASED_VARIABLE_NOT_UNTRACKABLE,
        libcellml::Issue::ReferenceRule::GENERATOR_NLA_BASED_VARIABLE_NOT_UNTRACKABLE,
        libcellml::Issue::ReferenceRule::GENERATOR_NLA_BASED_VARIABLE_NOT_UNTRACKABLE,
        libcellml::Issue::ReferenceRule::GENERATOR_NLA_BASED_VARIABLE_NOT_UNTRACKABLE,
        libcellml::Issue::ReferenceRule::GENERATOR_EXTERNALLY_NEEDED_VARIABLE_NOT_UNTRACKABLE,
        libcellml::Issue::ReferenceRule::GENERATOR_NLA_BASED_VARIABLE_NOT_UNTRACKABLE,
        libcellml::Issue::ReferenceRule::GENERATOR_NLA_BASED_VARIABLE_NOT_UNTRACKABLE,
        libcellml::Issue::ReferenceRule::GENERATOR_NLA_BASED_VARIABLE_NOT_UNTRACKABLE,
        libcellml::Issue::ReferenceRule::GENERATOR_NLA_BASED_VARIABLE_NOT_UNTRACKABLE,
        libcellml::Issue::ReferenceRule::GENERATOR_NLA_BASED_VARIABLE_NOT_UNTRACKABLE,
    };

    hodgkinHuxleySquidAxonModel1952CodeGeneration(false, TrackingType::VARIABLES,
                                                  daeIssues, daeLevels, daeReferenceRules,
                                                  daeExternalIssues, expectedLevels(daeExternalIssues.size(), libcellml::Issue::Level::ERROR), daeExternalReferenceRules);
}
