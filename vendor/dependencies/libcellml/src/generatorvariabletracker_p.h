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

#include "libcellml/generatorvariabletracker.h"

#include "issue_p.h"
#include "logger_p.h"

namespace libcellml {

/**
 * @brief The GeneratorVariableTracker::GeneratorVariableTrackerImpl struct.
 *
 * The private implementation for the GeneratorVariableTracker class.
 */
struct GeneratorVariableTracker::GeneratorVariableTrackerImpl: public Logger::LoggerImpl
{
    std::map<AnalyserModelWeakPtr, std::map<AnalyserVariableWeakPtr, bool, std::owner_less<AnalyserVariableWeakPtr>>, std::owner_less<AnalyserModelWeakPtr>> mTrackedVariables;

    void cleanupExpiredEntries();

    bool isTrackedVariable(const AnalyserModelPtr &analyserModel, const AnalyserVariablePtr &analyserVariable, bool tracked);
    bool isTrackedVariable(const AnalyserVariablePtr &analyserVariable, bool tracked);

    void addTrackingIssue(const AnalyserVariablePtr &analyserVariable, bool tracked, const std::string &variableInfo,
                          const std::string &trackedInfo, const std::string &untrackedInfo,
                          Issue::ReferenceRule trackedReferenceRule, Issue::ReferenceRule untrackedReferenceRule);

    void addNeededToComputeExternalVariableIssue(const AnalyserVariablePtr &analyserVariable, bool tracked);

    bool trackableVariable(const AnalyserVariablePtr &analyserVariable, bool tracked, bool canAddIssue = true);
    bool specialVariable(const AnalyserVariablePtr &analyserVariable, const AnalyserVariablePtr &specialAnalyserVariable, bool tracked,
                         Issue::ReferenceRule trackedReferenceRule, Issue::ReferenceRule untrackedReferenceRule);

    void trackVariable(const AnalyserVariablePtr &analyserVariable, bool tracked, bool needRemoveAllIssues = true);

    void trackVariables(const std::vector<AnalyserVariablePtr> &analyserVariables, bool tracked);

    bool validAnalyserModel(const AnalyserModelPtr &analyserModel);

    void trackAllConstants(const AnalyserModelPtr &analyserModel, bool tracked);
    void trackAllComputedConstants(const AnalyserModelPtr &analyserModel, bool tracked);
    void trackAllAlgebraicVariables(const AnalyserModelPtr &analyserModel, bool tracked);

    std::vector<AnalyserVariablePtr> trackableVariables(const AnalyserModelPtr &analyserModel) const;

    void trackAllVariables(const AnalyserModelPtr &analyserModel, bool tracked);

    size_t trackedVariableCount(const AnalyserModelPtr &analyserModel,
                                const std::vector<AnalyserVariablePtr> &analyserVariables, bool tracked);

    size_t trackedConstantCount(const AnalyserModelPtr &analyserModel, bool tracked);
    size_t trackedComputedConstantCount(const AnalyserModelPtr &analyserModel, bool tracked);
    size_t trackedAlgebraicVariableCount(const AnalyserModelPtr &analyserModel, bool tracked);
    size_t trackedVariableCount(const AnalyserModelPtr &analyserModel, bool tracked);
};

} // namespace libcellml
