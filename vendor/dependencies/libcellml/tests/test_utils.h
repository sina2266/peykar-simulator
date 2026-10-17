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

#pragma once

#include <chrono>
#include <iostream>
#include <libcellml>
#include <sstream>

#include "test_exportdefinitions.h"

#define TEST_UTILS
#include "../src/commonutils.h"
#undef TEST_UTILS

const std::string EMPTY_MATH = "<math xmlns=\"http://www.w3.org/1998/Math/MathML\"/>\n";

const std::string NON_EMPTY_MATH =
    "<math xmlns:cellml=\"http://www.cellml.org/cellml/2.0#\" xmlns=\"http://www.w3.org/1998/Math/MathML\">\n"
    "  <apply>\n"
    "    <eq/>\n"
    "    <ci>var</ci>\n"
    "    <apply>\n"
    "      <plus/>\n"
    "      <cn cellml:units=\"dimensionless\" type=\"e-notation\">3.44<sep/>2</cn>\n"
    "      <cn cellml:units=\"dimensionless\">-9.612</cn>\n"
    "    </apply>\n"
    "  </apply>\n"
    "</math>\n";

struct Debug
{
    explicit Debug(bool newLine = true)
        : mNewLine(newLine)
    {
    }

    ~Debug()
    {
        std::cout << mSS.str();
        if (mNewLine) {
            std::cout << std::endl;
        }
    }

    Debug &operator<<(const void *p)
    {
        std::ostringstream ss;
        ss << static_cast<const void *>(p);
        mSS << ss.str();
        return *this;
    }

    // Accept just about anything.
    template<class T>
    Debug &operator<<(const T &x)
    {
        mSS << x;
        return *this;
    }

private:
    std::ostringstream mSS;
    bool mNewLine;
};

std::chrono::steady_clock::time_point TEST_EXPORT timeNow();
int TEST_EXPORT elapsedTime(const std::chrono::steady_clock::time_point &startTime);

std::string TEST_EXPORT resourcePath(const std::string &resourceRelativePath = "");
std::string TEST_EXPORT fileContents(const std::string &fileName);
void TEST_EXPORT printIssues(const libcellml::LoggerPtr &l, bool headings = false, bool cellmlElementTypes = false, bool rule = false);

void TEST_EXPORT printModel(const libcellml::ModelPtr &model, bool includeMaths = true);
void TEST_EXPORT printComponent(const libcellml::ComponentPtr &component, bool includeMaths = true);

std::vector<libcellml::CellmlElementType> TEST_EXPORT expectedCellmlElementTypes(size_t size, libcellml::CellmlElementType type);
std::vector<libcellml::Issue::Level> TEST_EXPORT expectedLevels(size_t size, libcellml::Issue::Level level);
std::vector<libcellml::Issue::ReferenceRule> TEST_EXPORT expectedReferenceRules(size_t size, libcellml::Issue::ReferenceRule rule);
std::vector<std::string> TEST_EXPORT expectedUrls(size_t size, std::string url);

void TEST_EXPORT expectEqualIssues(const std::vector<std::string> &issues, const libcellml::LoggerPtr &logger);
void TEST_EXPORT expectEqualIssuesSpecificationHeadingsUrls(const std::vector<std::string> &issues,
                                                            const std::vector<std::string> &specificationHeadings,
                                                            const std::vector<std::string> &urls,
                                                            const libcellml::LoggerPtr &logger);
void TEST_EXPORT expectEqualIssuesCellmlElementTypesLevelsReferenceRulesUrls(const std::vector<std::string> &issues,
                                                                             const std::vector<libcellml::CellmlElementType> &cellmlElementTypes,
                                                                             const std::vector<libcellml::Issue::Level> &levels,
                                                                             const std::vector<libcellml::Issue::ReferenceRule> &referenceRules,
                                                                             const std::vector<std::string> &urls,
                                                                             const libcellml::LoggerPtr &logger);
void TEST_EXPORT expectEqualIssuesLevelsReferenceRules(const std::vector<std::string> &issues,
                                                       const std::vector<libcellml::Issue::Level> &levels,
                                                       const std::vector<libcellml::Issue::ReferenceRule> &referenceRules,
                                                       const libcellml::LoggerPtr &logger);
void TEST_EXPORT expectEqualFileContents(const std::string &fileName, const std::string &fileContents);

libcellml::ModelPtr TEST_EXPORT createModel(const std::string &name = "");
libcellml::ModelPtr TEST_EXPORT createModelWithComponent(const std::string &modelName = "",
                                                         const std::string &componentName = "");
libcellml::VariablePtr TEST_EXPORT createVariableWithUnits(const std::string &name, const std::string &units);
libcellml::ModelPtr TEST_EXPORT createModelTwoComponentsWithOneVariableEach(const std::string &modelName = "", const std::string &c1Name = "", const std::string &c2Name = "", const std::string &v1Name = "", const std::string &v2Name = "");

void TEST_EXPORT compareUnit(const libcellml::UnitsPtr &u1, const libcellml::UnitsPtr &u2);
void TEST_EXPORT compareUnits(const libcellml::UnitsPtr &u1, const libcellml::UnitsPtr &u2, const libcellml::EntityPtr &expectedParent = nullptr);
void TEST_EXPORT compareComponent(const libcellml::ComponentPtr &c1, const libcellml::ComponentPtr &c2, const libcellml::EntityPtr &expectedParent = nullptr);
void TEST_EXPORT compareReset(const libcellml::ResetPtr &r1, const libcellml::ResetPtr &r2);
void TEST_EXPORT compareModel(const libcellml::ModelPtr &m1, const libcellml::ModelPtr &m2);

#define EXPECT_EQ_ISSUES(issues, logger) \
    SCOPED_TRACE("Issue occurred here."); \
    expectEqualIssues(issues, logger)

#define EXPECT_EQ_ISSUES_SPECIFICATION_HEADINGS_URLS(issues, specificationHeadings, urls, logger) \
    SCOPED_TRACE("Issue occurred here."); \
    expectEqualIssuesSpecificationHeadingsUrls(issues, specificationHeadings, urls, logger)

#define EXPECT_EQ_ISSUES_CELLMLELEMENTTYPES_LEVELS_REFERENCERULES_URLS(issues, cellmlElementTypes, levels, referenceRules, urls, logger) \
    SCOPED_TRACE("Issue occurred here."); \
    expectEqualIssuesCellmlElementTypesLevelsReferenceRulesUrls(issues, cellmlElementTypes, levels, referenceRules, urls, logger)

#define EXPECT_EQ_ISSUES_LEVELS_REFERENCERULES(issues, levels, referenceRules, logger) \
    SCOPED_TRACE("Issue occurred here."); \
    expectEqualIssuesLevelsReferenceRules(issues, levels, referenceRules, logger)

#define EXPECT_EQ_FILE_CONTENTS(fileName, fileContents) \
    SCOPED_TRACE("Issue occurred here."); \
    expectEqualFileContents(fileName, fileContents)
