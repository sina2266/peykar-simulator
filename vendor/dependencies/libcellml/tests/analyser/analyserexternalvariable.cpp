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

TEST(AnalyserExternalVariable, createExternalVariable)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto variable = model->component("membrane")->variable("V");
    auto externalVariable = libcellml::AnalyserExternalVariable::create(variable);

    EXPECT_EQ(variable, externalVariable->variable());
}

TEST(AnalyserExternalVariable, addSelfAsDependency)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto dependency = model->component("membrane")->variable("V");
    auto externalVariable = libcellml::AnalyserExternalVariable::create(dependency);

    externalVariable->addDependency(dependency);

    EXPECT_EQ(size_t(0), externalVariable->dependencyCount());

    externalVariable->addDependency(model->component("sodium_channel")->variable("V"));

    EXPECT_EQ(size_t(0), externalVariable->dependencyCount());
}

TEST(AnalyserExternalVariable, addSameDependency)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto externalVariable = libcellml::AnalyserExternalVariable::create(model->component("membrane")->variable("V"));
    auto dependency = model->component("membrane")->variable("Cm");

    externalVariable->addDependency(dependency);

    EXPECT_EQ(size_t(1), externalVariable->dependencyCount());

    externalVariable->addDependency(dependency);

    EXPECT_EQ(size_t(1), externalVariable->dependencyCount());
}

TEST(AnalyserExternalVariable, addDependencyFromDifferentModel)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto other_model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto externalVariable = libcellml::AnalyserExternalVariable::create(model->component("membrane")->variable("V"));

    externalVariable->addDependency(other_model->component("membrane")->variable("Cm"));

    EXPECT_EQ(size_t(0), externalVariable->dependencyCount());
}

TEST(AnalyserExternalVariable, removeDependencyByIndex)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto externalVariable = libcellml::AnalyserExternalVariable::create(model->component("membrane")->variable("V"));

    EXPECT_FALSE(externalVariable->removeDependency(0));

    externalVariable->addDependency(model->component("membrane")->variable("Cm"));

    EXPECT_TRUE(externalVariable->removeDependency(0));
    EXPECT_FALSE(externalVariable->removeDependency(1));
}

TEST(AnalyserExternalVariable, removeDependencyByName)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto externalVariable = libcellml::AnalyserExternalVariable::create(model->component("membrane")->variable("V"));

    EXPECT_FALSE(externalVariable->removeDependency(model, "membrane", "Cm"));

    externalVariable->addDependency(model->component("membrane")->variable("Cm"));

    EXPECT_TRUE(externalVariable->removeDependency(model, "membrane", "Cm"));
    EXPECT_FALSE(externalVariable->removeDependency(model, "membrane", "X"));
}

TEST(AnalyserExternalVariable, removeDependencyByPointer)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto externalVariable = libcellml::AnalyserExternalVariable::create(model->component("membrane")->variable("V"));
    auto dummy = model->component("membrane")->variable("Cm");

    EXPECT_FALSE(externalVariable->removeDependency(dummy));

    externalVariable->addDependency(dummy);

    EXPECT_TRUE(externalVariable->removeDependency(dummy));
    EXPECT_FALSE(externalVariable->removeDependency(nullptr));
}

TEST(AnalyserExternalVariable, removeAllDependencies)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto externalVariable = libcellml::AnalyserExternalVariable::create(model->component("membrane")->variable("V"));

    externalVariable->addDependency(model->component("membrane")->variable("i_Na"));
    externalVariable->addDependency(model->component("membrane")->variable("i_K"));
    externalVariable->addDependency(model->component("membrane")->variable("i_L"));

    EXPECT_EQ(size_t(3), externalVariable->dependencyCount());

    externalVariable->removeAllDependencies();

    EXPECT_EQ(size_t(0), externalVariable->dependencyCount());
}

TEST(AnalyserExternalVariable, containsDependencyByName)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto externalVariable = libcellml::AnalyserExternalVariable::create(model->component("membrane")->variable("V"));

    externalVariable->addDependency(model->component("membrane")->variable("Cm"));

    EXPECT_TRUE(externalVariable->containsDependency(model, "membrane", "Cm"));
    EXPECT_FALSE(externalVariable->containsDependency(model, "membrane", "X"));
}

TEST(AnalyserExternalVariable, containsDependencyByPointer)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto externalVariable = libcellml::AnalyserExternalVariable::create(model->component("membrane")->variable("V"));
    auto dependency = model->component("membrane")->variable("Cm");

    externalVariable->addDependency(dependency);

    EXPECT_TRUE(externalVariable->containsDependency(dependency));
    EXPECT_FALSE(externalVariable->containsDependency(nullptr));
}

TEST(AnalyserExternalVariable, dependencyByIndex)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto externalVariable = libcellml::AnalyserExternalVariable::create(model->component("membrane")->variable("V"));

    EXPECT_EQ(nullptr, externalVariable->dependency(0));

    auto dependency = model->component("membrane")->variable("Cm");

    externalVariable->addDependency(dependency);

    EXPECT_EQ(dependency, externalVariable->dependency(0));
    EXPECT_EQ(nullptr, externalVariable->dependency(1));
}

TEST(AnalyserExternalVariable, dependencyByName)
{
    auto parser = libcellml::Parser::create();
    auto model = parser->parseModel(fileContents("generator/hodgkin_huxley_squid_axon_model_1952/model.cellml"));

    EXPECT_EQ(size_t(0), parser->issueCount());

    auto externalVariable = libcellml::AnalyserExternalVariable::create(model->component("membrane")->variable("V"));

    EXPECT_EQ(nullptr, externalVariable->dependency(model, "membrane", "Cm"));

    auto dependency = model->component("membrane")->variable("Cm");

    externalVariable->addDependency(dependency);

    EXPECT_EQ(dependency, externalVariable->dependency(model, "membrane", "Cm"));
    EXPECT_EQ(nullptr, externalVariable->dependency(model, "membrane", "X"));
}
