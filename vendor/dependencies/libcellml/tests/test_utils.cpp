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

#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "test_resources.h"

#define TEST_UTILS
#include "../src/commonutils.cpp"
#undef TEST_UTILS

std::string resourcePath(const std::string &resourceRelativePath)
{
    return TESTS_RESOURCE_LOCATION + "/" + resourceRelativePath;
}

std::string fileContents(const std::string &fileName)
{
    std::ifstream file(resourcePath(fileName));
    std::stringstream buffer;

    buffer << file.rdbuf();

    return buffer.str();
}

std::chrono::steady_clock::time_point timeNow()
{
    return std::chrono::steady_clock::now();
}

int elapsedTime(const std::chrono::steady_clock::time_point &startTime)
{
    return int(std::chrono::duration_cast<std::chrono::milliseconds>(timeNow() - startTime).count());
}

void printIssues(const libcellml::LoggerPtr &l, bool headings, bool cellmlElementTypes, bool rule)
{
    int width = int(floor(log10(l->errorCount())));
    for (size_t i = 0; i < l->issueCount(); ++i) {
        switch (l->issue(i)->level()) {
        case libcellml::Issue::Level::ERROR:
            std::cout << "Error " << std::setw(width) << i + 1 << ": ";
            break;
        case libcellml::Issue::Level::WARNING:
            std::cout << "Warning " << std::setw(width) << i + 1 << ": ";
            break;
        case libcellml::Issue::Level::MESSAGE:
            std::cout << "Message " << std::setw(width) << i + 1 << ": ";
            break;
        default:
            break;
        }

        if (headings) {
            std::cout << ", " << l->issue(i)->referenceHeading();
        }
        if (cellmlElementTypes) {
            std::cout << ", " << static_cast<int>(l->issue(i)->item()->type());
        }
        if (rule) {
            std::cout << ", " << static_cast<int>(l->issue(i)->referenceRule());
        }
        std::cout << std::endl
                  << l->issue(i)->description() << std::endl;
    }
}

static const std::string FIXED_INDENT = "    ";

void printComponent(const libcellml::ComponentPtr &component, size_t c, const std::string &indent, bool includeMaths)
{
    if (c == SIZE_MAX) {
        std::cout << "COMPONENT: '" << component->name() << "'";
    } else {
        std::cout << indent << "[" << c + 1 << "]: " << component->name();
    }

    if (component->id() != "") {
        std::cout << ", id: " << component->id();
    }

    std::cout << std::endl;
    std::cout << indent << FIXED_INDENT << "VARIABLES: " << component->variableCount() << " variables" << std::endl;

    // Printing the variables within the component.
    for (size_t v = 0; v < component->variableCount(); ++v) {
        std::cout << indent << FIXED_INDENT << FIXED_INDENT;
        std::cout << "[" << v + 1 << "]: " << component->variable(v)->name();
        if (component->variable(v)->units() != nullptr) {
            std::cout << " [" << component->variable(v)->units()->name() << "]";
        }
        if (component->variable(v)->initialValue() != "") {
            std::cout << ", initial = " << component->variable(v)->initialValue();
        }
        std::cout << std::endl;
        if (component->variable(v)->equivalentVariableCount() > 0) {
            std::cout << indent << FIXED_INDENT << FIXED_INDENT << FIXED_INDENT;
            std::string con = "  └──> ";
            for (size_t e = 0; e < component->variable(v)->equivalentVariableCount(); ++e) {
                auto ev = component->variable(v)->equivalentVariable(e);
                if (ev == nullptr) {
                    std::cout << "WHOOPS! Null equivalent variable!";
                    continue;
                }
                libcellml::ComponentPtr ev_parent = std::dynamic_pointer_cast<libcellml::Component>(ev->parent());
                if (ev_parent == nullptr) {
                    std::cout << "WHOOPS! Null parent component for equivalent variable!";
                    continue;
                }
                std::cout << con << ev_parent->name() << ":" << ev->name();
                if (ev->units() != nullptr) {
                    std::cout << " [" << ev->units()->name() << "]";
                }
                con = ", ";
            }
            std::cout << std::endl;
        }
    }

    // Print the maths within the component.
    if (includeMaths) {
        if (component->math() != "") {
            std::cout << indent << "  Maths in the component is:" << std::endl;
            std::cout << component->math() << std::endl;
        }
    }

    // Print the encapsulated components
    if (component->componentCount() > 0) {
        std::cout << indent << FIXED_INDENT << "CHILD COMPONENTS: " << component->componentCount()
                  << " child components" << std::endl;
        std::string newIndent = indent + FIXED_INDENT + FIXED_INDENT;

        for (size_t c2 = 0; c2 < component->componentCount(); ++c2) {
            auto child = component->component(c2);
            printComponent(child, c2, newIndent, includeMaths);
        }
    }
}

void printComponent(const libcellml::ComponentPtr &component, bool includeMaths)
{
    printComponent(component, SIZE_MAX, {}, includeMaths);
}

void printModel(const libcellml::ModelPtr &model, bool includeMaths)
{
    std::cout << "MODEL: '" << model->name() << "'";
    if (model->id() != "") {
        std::cout << ", id: '" << model->id() << "'";
    }
    std::cout << std::endl;

    std::cout << FIXED_INDENT << "UNITS: " << model->unitsCount() << " custom units" << std::endl;
    for (size_t u = 0; u < model->unitsCount(); ++u) {
        std::cout << FIXED_INDENT << FIXED_INDENT << "[" << u + 1 << "]: " << model->units(u)->name() << std::endl;
    }

    std::cout << FIXED_INDENT << "COMPONENTS: " << model->componentCount() << " components" << std::endl;
    for (size_t c = 0; c < model->componentCount(); ++c) {
        auto component = model->component(c);
        printComponent(component, c, FIXED_INDENT + FIXED_INDENT, includeMaths);
    }
}

template<typename T>
std::vector<T> expectedValues(size_t size, T value)
{
    std::vector<T> res;

    res.assign(size, value);

    return res;
}

std::vector<libcellml::CellmlElementType> expectedCellmlElementTypes(size_t size, libcellml::CellmlElementType type)
{
    return expectedValues<libcellml::CellmlElementType>(size, type);
}

std::vector<libcellml::Issue::Level> expectedLevels(size_t size, libcellml::Issue::Level level)
{
    return expectedValues<libcellml::Issue::Level>(size, level);
}

std::vector<libcellml::Issue::ReferenceRule> expectedReferenceRules(size_t size, libcellml::Issue::ReferenceRule rule)
{
    return expectedValues<libcellml::Issue::ReferenceRule>(size, rule);
}

std::vector<std::string> expectedUrls(size_t size, std::string url)
{
    return expectedValues<std::string>(size, url);
}

void expectEqualIssues(const std::vector<std::string> &issues, const libcellml::LoggerPtr &logger)
{
    EXPECT_EQ(issues.size(), logger->issueCount());
    for (size_t i = 0; i < logger->issueCount() && i < issues.size(); ++i) {
        EXPECT_EQ(issues.at(i), logger->issue(i)->description());
    }
}

void expectEqualIssuesSpecificationHeadingsUrls(const std::vector<std::string> &issues,
                                                const std::vector<std::string> &specificationHeadings,
                                                const std::vector<std::string> &urls,
                                                const libcellml::LoggerPtr &logger)
{
    EXPECT_EQ(issues.size(), logger->issueCount());
    EXPECT_EQ(specificationHeadings.size(), logger->issueCount());
    EXPECT_EQ(urls.size(), logger->issueCount());
    for (size_t i = 0; i < logger->issueCount() && i < issues.size(); ++i) {
        EXPECT_EQ(issues.at(i), logger->issue(i)->description());
        EXPECT_EQ(specificationHeadings.at(i), logger->issue(i)->referenceHeading());
        EXPECT_EQ(urls.at(i), logger->issue(i)->url());
    }
}

void expectEqualIssuesCellmlElementTypesLevelsReferenceRulesUrls(const std::vector<std::string> &issues,
                                                                 const std::vector<libcellml::CellmlElementType> &cellmlElementTypes,
                                                                 const std::vector<libcellml::Issue::Level> &levels,
                                                                 const std::vector<libcellml::Issue::ReferenceRule> &referenceRules,
                                                                 const std::vector<std::string> &urls,
                                                                 const libcellml::LoggerPtr &logger)
{
    EXPECT_EQ(issues.size(), logger->issueCount());
    EXPECT_EQ(cellmlElementTypes.size(), logger->issueCount());
    EXPECT_EQ(levels.size(), logger->issueCount());
    EXPECT_EQ(referenceRules.size(), logger->issueCount());
    EXPECT_EQ(urls.size(), logger->issueCount());
    for (size_t i = 0; i < logger->issueCount() && i < issues.size(); ++i) {
        EXPECT_EQ(issues.at(i), logger->issue(i)->description());
        EXPECT_EQ(cellmlElementTypes.at(i), logger->issue(i)->item()->type());
        EXPECT_EQ(levels.at(i), logger->issue(i)->level());
        EXPECT_EQ(referenceRules.at(i), logger->issue(i)->referenceRule());
        EXPECT_EQ(urls.at(i), logger->issue(i)->url());
    }
}

void expectEqualIssuesLevelsReferenceRules(const std::vector<std::string> &issues,
                                           const std::vector<libcellml::Issue::Level> &levels,
                                           const std::vector<libcellml::Issue::ReferenceRule> &referenceRules,
                                           const libcellml::LoggerPtr &logger)
{
    EXPECT_EQ(issues.size(), logger->issueCount());
    EXPECT_EQ(levels.size(), logger->issueCount());
    EXPECT_EQ(referenceRules.size(), logger->issueCount());
    for (size_t i = 0; i < logger->issueCount() && i < issues.size(); ++i) {
        EXPECT_EQ(issues.at(i), logger->issue(i)->description());
        EXPECT_EQ(levels.at(i), logger->issue(i)->level());
        EXPECT_EQ(referenceRules.at(i), logger->issue(i)->referenceRule());
    }
}

libcellml::ModelPtr createModel(const std::string &name)
{
    libcellml::ModelPtr model = libcellml::Model::create();
    model->setName(name);
    return model;
}

libcellml::ComponentPtr createComponentInModel(const libcellml::ModelPtr &model, const std::string &componentName)
{
    libcellml::ComponentPtr component = libcellml::Component::create();
    component->setName(componentName);
    model->addComponent(component);
    return component;
}

libcellml::ModelPtr createModelWithComponent(const std::string &modelName, const std::string &componentName)
{
    libcellml::ModelPtr model = libcellml::Model::create();
    model->setName(modelName);
    createComponentInModel(model, componentName);
    return model;
}

libcellml::VariablePtr createVariableWithUnits(const std::string &name, const std::string &units)
{
    libcellml::VariablePtr v = libcellml::Variable::create();
    v->setName(name);
    v->setUnits(units);

    return v;
}

libcellml::ModelPtr createModelTwoComponentsWithOneVariableEach(const std::string &modelName, const std::string &c1Name, const std::string &c2Name, const std::string &v1Name, const std::string &v2Name)
{
    libcellml::ModelPtr model = libcellml::Model::create();
    model->setName(modelName);
    auto c1 = createComponentInModel(model, c1Name);
    auto c2 = createComponentInModel(model, c2Name);

    libcellml::VariablePtr v1 = libcellml::Variable::create();
    v1->setName(v1Name);
    v1->setInterfaceType("public");
    c1->addVariable(v1);
    libcellml::VariablePtr v2 = libcellml::Variable::create();
    v2->setName(v2Name);
    v2->setInterfaceType("public");
    c2->addVariable(v2);

    return model;
}

void compareUnit(const libcellml::UnitsPtr &u1, const libcellml::UnitsPtr &u2)
{
    EXPECT_EQ(u1->unitCount(), u2->unitCount());

    std::string reference1;
    std::string prefix1;
    std::string id1;
    std::string reference2;
    std::string prefix2;
    std::string id2;
    double exponent1;
    double multiplier1;
    double exponent2;
    double multiplier2;
    for (size_t index = 0; index < u1->unitCount(); ++index) {
        u1->unitAttributes(index, reference1, prefix1, exponent1, multiplier1, id1);
        u2->unitAttributes(index, reference2, prefix2, exponent2, multiplier2, id2);

        EXPECT_EQ(reference1, reference2);
        EXPECT_EQ(prefix1, prefix2);
        EXPECT_EQ(exponent1, exponent2);
        EXPECT_EQ(multiplier1, multiplier2);
        EXPECT_EQ(id1, id2);
    }
}

void compareUnits(const libcellml::UnitsPtr &u1, const libcellml::UnitsPtr &u2, const libcellml::EntityPtr &expectedParent)
{
    EXPECT_EQ(u1->id(), u2->id());
    EXPECT_EQ(u1->isBaseUnit(), u2->isBaseUnit());
    EXPECT_EQ(u1->isImport(), u2->isImport());
    EXPECT_EQ(u1->importReference(), u2->importReference());
    EXPECT_EQ(u1->name(), u2->name());
    EXPECT_EQ(expectedParent, u2->parent());

    compareUnit(u1, u2);
}

void compareComponent(const libcellml::ComponentPtr &c1, const libcellml::ComponentPtr &c2, const libcellml::EntityPtr &expectedParent)
{
    EXPECT_EQ(c1->name(), c2->name());
    EXPECT_EQ(c1->id(), c2->id());
    EXPECT_EQ(c1->isImport(), c2->isImport());
    if (c1->isImport() && c2->isImport()) {
        EXPECT_EQ(c1->importSource()->url(), c2->importSource()->url());
        EXPECT_EQ(c1->importSource()->id(), c2->importSource()->id());
    }
    EXPECT_EQ(c1->importReference(), c2->importReference());
    EXPECT_EQ(c1->componentCount(), c2->componentCount());
    EXPECT_EQ(c1->resetCount(), c2->resetCount());
    EXPECT_EQ(c1->variableCount(), c2->variableCount());

    EXPECT_EQ(expectedParent, c2->parent());

    for (size_t index = 0; index < c1->componentCount(); ++index) {
        auto c1i = c1->component(index);
        auto c2i = c2->component(index);
        compareComponent(c1i, c2i, c2);
    }
    for (size_t index = 0; index < c2->resetCount(); ++index) {
        auto r = c2->reset(index);
        if (r->variable() != nullptr) {
            EXPECT_TRUE(c2->hasVariable(r->variable()));
        }
        if (r->testVariable() != nullptr) {
            EXPECT_TRUE(c2->hasVariable(r->testVariable()));
        }
    }
}

void compareModel(const libcellml::ModelPtr &m1, const libcellml::ModelPtr &m2)
{
    EXPECT_EQ(m1->id(), m2->id());
    EXPECT_EQ(m1->name(), m2->name());

    EXPECT_EQ(m1->unitsCount(), m2->unitsCount());
    EXPECT_EQ(m1->componentCount(), m2->componentCount());

    for (size_t index = 0; index < m1->unitsCount(); ++index) {
        auto u1 = m1->units(index);
        auto u2 = m2->units(index);
        compareUnits(u1, u2, m2);
    }

    for (size_t index = 0; index < m1->componentCount(); ++index) {
        auto c1 = m1->component(index);
        auto c2 = m2->component(index);
        compareComponent(c1, c2, m2);
    }
}

void compareReset(const libcellml::ResetPtr &r1, const libcellml::ResetPtr &r2)
{
    EXPECT_EQ(r1->id(), r2->id());
    EXPECT_EQ(r1->order(), r2->order());
    if (r1->variable() != nullptr) {
        EXPECT_NE(r1->variable(), r2->variable());
        EXPECT_EQ(r1->variable()->name(), r2->variable()->name());
    }
    if (r1->testVariable() != nullptr) {
        EXPECT_NE(r1->testVariable(), r2->testVariable());
        EXPECT_EQ(r1->testVariable()->name(), r2->testVariable()->name());
    }
    EXPECT_EQ(r1->testValueId(), r2->testValueId());
    EXPECT_EQ(r1->resetValueId(), r2->resetValueId());
}

void expectEqualFileContents(const std::string &fileName, const std::string &fileContents)
{
    // Uncomment the below when you want to generate the expected file contents.
    // #define NEW_GENERATOR

#ifdef NEW_GENERATOR
    std::ofstream file(resourcePath(fileName));

    file << fileContents;

    file.close();
#endif

    EXPECT_EQ(::fileContents(fileName), fileContents);
}
