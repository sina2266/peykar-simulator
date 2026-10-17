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

#include "libcellml/generatorvariabletracker.h"

#include "libcellml/analyserequation.h"
#include "libcellml/analysermodel.h"
#include "libcellml/analyservariable.h"

#include "analyserequation_p.h"
#include "commonutils.h"
#include "generatorvariabletracker_p.h"
#include "utilities.h"

namespace libcellml {

void GeneratorVariableTracker::GeneratorVariableTrackerImpl::cleanupExpiredEntries()
{
    // Remove all expired weak pointers from the map (to prevent memory bloat and ensure that we don't try to access
    // expired entries).
    // Note: an analyser variable cannot expire while its analyser model is still alive, so we only need to check the
    //       analyser model weak pointers here.

    auto analyserModelIt = mTrackedVariables.begin();

    while (analyserModelIt != mTrackedVariables.end()) {
        if (analyserModelIt->first.expired()) {
            analyserModelIt = mTrackedVariables.erase(analyserModelIt);
        } else {
            ++analyserModelIt;
        }
    }
}

bool GeneratorVariableTracker::GeneratorVariableTrackerImpl::isTrackedVariable(const AnalyserModelPtr &analyserModel, const AnalyserVariablePtr &analyserVariable,
                                                                               bool tracked)
{
    // By default an analyser variable is always tracked.

    cleanupExpiredEntries();

    if (mTrackedVariables[analyserModel].find(analyserVariable) == mTrackedVariables[analyserModel].end()) {
        mTrackedVariables[analyserModel][analyserVariable] = true;
    }

    return mTrackedVariables[analyserModel][analyserVariable] == tracked;
}

bool GeneratorVariableTracker::GeneratorVariableTrackerImpl::isTrackedVariable(const AnalyserVariablePtr &analyserVariable, bool tracked)
{
    if (analyserVariable == nullptr) {
        return false;
    }

    return isTrackedVariable(analyserVariable->analyserModel(), analyserVariable, tracked);
}

void GeneratorVariableTracker::GeneratorVariableTrackerImpl::addTrackingIssue(const AnalyserVariablePtr &analyserVariable, bool tracked,
                                                                              const std::string &variableInfo, const std::string &trackedInfo,
                                                                              const std::string &untrackedInfo,
                                                                              Issue::ReferenceRule trackedReferenceRule,
                                                                              Issue::ReferenceRule untrackedReferenceRule)
{
    auto issue = Issue::IssueImpl::create();

    issue->mPimpl->setDescription("Variable '" + analyserVariable->variable()->name()
                                  + "' in component '" + owningComponent(analyserVariable->variable())->name()
                                  + "' is " + variableInfo + " and "
                                  + (tracked ? trackedInfo : untrackedInfo) + ".");
    issue->mPimpl->setReferenceRule(tracked ? trackedReferenceRule : untrackedReferenceRule);

    if (tracked) {
        issue->mPimpl->setLevel(Issue::Level::MESSAGE);
    }

    addIssue(issue);
}

void GeneratorVariableTracker::GeneratorVariableTrackerImpl::addNeededToComputeExternalVariableIssue(const AnalyserVariablePtr &analyserVariable, bool tracked)
{
    addTrackingIssue(analyserVariable, tracked, "needed to compute an external variable",
                     "is therefore always tracked", "cannot therefore be untracked",
                     Issue::ReferenceRule::GENERATOR_EXTERNALLY_NEEDED_VARIABLE_ALWAYS_TRACKED,
                     Issue::ReferenceRule::GENERATOR_EXTERNALLY_NEEDED_VARIABLE_NOT_UNTRACKABLE);
}

bool GeneratorVariableTracker::GeneratorVariableTrackerImpl::trackableVariable(const AnalyserVariablePtr &analyserVariable, bool tracked, bool canAddIssue)
{
    // A trackable analyser variable is an analyser variable that is not a variable of integration, a state, or an
    // external variable, which is already the case when we reach this point. However, a trackable analyser variable is
    // also an analyser variable that is not computed using an NLA system.

    for (const auto &analyserEquation : analyserVariable->analyserEquations()) {
        if (analyserEquation->type() == AnalyserEquation::Type::NLA) {
            if (canAddIssue) {
                addTrackingIssue(analyserVariable, tracked, "computed using an NLA system",
                                 "is therefore always tracked", "cannot therefore be untracked",
                                 Issue::ReferenceRule::GENERATOR_NLA_BASED_VARIABLE_ALWAYS_TRACKED,
                                 Issue::ReferenceRule::GENERATOR_NLA_BASED_VARIABLE_NOT_UNTRACKABLE);
            }

            return false;
        }
    }

    // A trackable analyser variable is also not an analyser variable that is needed to compute an external variable.

    for (const auto &externalVariable : analyserVariable->analyserModel()->externalVariables()) {
        auto externalEquationPimpl = externalVariable->analyserEquations().front()->mPimpl;

        // Check whether the analyser variable is a constant dependency.

        for (const auto &constantDependency : externalEquationPimpl->mConstantDependencies) {
            if (analyserVariable == constantDependency) {
                if (canAddIssue) {
                    addNeededToComputeExternalVariableIssue(analyserVariable, tracked);
                }

                return false;
            }
        }

        // Check whether the analyser variable is a computed constant dependency or an algebraic dependency.

        for (const auto &dependency : externalEquationPimpl->mDependencies) {
            auto dependencyPimpl = dependency.lock()->mPimpl;

            for (const auto &computedConstant : dependencyPimpl->mComputedConstants) {
                if (analyserVariable == computedConstant) {
                    if (canAddIssue) {
                        addNeededToComputeExternalVariableIssue(analyserVariable, tracked);
                    }

                    return false;
                }
            }

            for (const auto &algebraicVariable : dependencyPimpl->mAlgebraicVariables) {
                if (analyserVariable == algebraicVariable) {
                    if (canAddIssue) {
                        addNeededToComputeExternalVariableIssue(analyserVariable, tracked);
                    }

                    return false;
                }
            }
        }
    }

    return true;
}

bool GeneratorVariableTracker::GeneratorVariableTrackerImpl::specialVariable(const AnalyserVariablePtr &analyserVariable,
                                                                             const AnalyserVariablePtr &specialAnalyserVariable, bool tracked,
                                                                             Issue::ReferenceRule trackedReferenceRule,
                                                                             Issue::ReferenceRule untrackedReferenceRule)
{
    if (analyserVariable == specialAnalyserVariable) {
        auto variableInfo = (specialAnalyserVariable->type() == AnalyserVariable::Type::VARIABLE_OF_INTEGRATION) ?
                                "the variable of integration" :
                                ((specialAnalyserVariable->type() == AnalyserVariable::Type::STATE) ?
                                     "a state variable" :
                                     "an external variable");

        addTrackingIssue(analyserVariable, tracked, variableInfo,
                         "is therefore always tracked", "cannot therefore be untracked",
                         trackedReferenceRule, untrackedReferenceRule);

        return true;
    }

    return false;
}

void GeneratorVariableTracker::GeneratorVariableTrackerImpl::trackVariable(const AnalyserVariablePtr &analyserVariable, bool tracked,
                                                                           bool needRemoveAllIssues)
{
    // Make sure that we have a variable.

    if (needRemoveAllIssues) {
        removeAllIssues();
    }

    if (analyserVariable == nullptr) {
        auto issue = Issue::IssueImpl::create();

        issue->mPimpl->setDescription("The variable is null.");
        issue->mPimpl->setReferenceRule(Issue::ReferenceRule::GENERATOR_NULL_VARIABLE);

        addIssue(issue);

        return;
    }

    // Check whether we want to track/untrack a constant, a computed constant, or an algebraic variable though we can
    // only track/untrack a variable if it is actually trackable.

    auto analyserModel = analyserVariable->analyserModel();

    for (const auto &analyserModelVariable : analyserVariables(analyserVariable)) {
        if (analyserVariable == analyserModelVariable) {
            if (trackableVariable(analyserVariable, tracked)) {
                cleanupExpiredEntries();

                mTrackedVariables[analyserModelVariable->analyserModel()][analyserModelVariable] = tracked;
            }

            return;
        }
    }

    // Check whether we tried to track/untrack a state variable.

    for (const auto &state : analyserModel->states()) {
        if (specialVariable(analyserVariable, state, tracked,
                            Issue::ReferenceRule::GENERATOR_STATE_VARIABLE_ALWAYS_TRACKED,
                            Issue::ReferenceRule::GENERATOR_STATE_VARIABLE_NOT_UNTRACKABLE)) {
            return;
        }
    }

    // Check whether we tried to track/untrack an external variable.

    for (const auto &externalVariable : analyserModel->externalVariables()) {
        if (specialVariable(analyserVariable, externalVariable, tracked,
                            Issue::ReferenceRule::GENERATOR_EXTERNAL_VARIABLE_ALWAYS_TRACKED,
                            Issue::ReferenceRule::GENERATOR_EXTERNAL_VARIABLE_NOT_UNTRACKABLE)) {
            return;
        }
    }

    // In the end, we tried to track/untrack the variable of integration.

    specialVariable(analyserVariable, analyserModel->voi(), tracked,
                    Issue::ReferenceRule::GENERATOR_VOI_VARIABLE_ALWAYS_TRACKED,
                    Issue::ReferenceRule::GENERATOR_VOI_VARIABLE_NOT_UNTRACKABLE);
}

void GeneratorVariableTracker::GeneratorVariableTrackerImpl::trackVariables(const std::vector<AnalyserVariablePtr> &analyserVariables, bool tracked)
{
    removeAllIssues();

    for (const auto &analyserVariable : analyserVariables) {
        trackVariable(analyserVariable, tracked, false);
    }
}

bool GeneratorVariableTracker::GeneratorVariableTrackerImpl::validAnalyserModel(const AnalyserModelPtr &analyserModel)
{
    removeAllIssues();

    if (analyserModel == nullptr) {
        auto issue = Issue::IssueImpl::create();

        issue->mPimpl->setDescription("The model is null.");
        issue->mPimpl->setReferenceRule(Issue::ReferenceRule::GENERATOR_NULL_MODEL);

        addIssue(issue);

        return false;
    }

    return true;
}

void GeneratorVariableTracker::GeneratorVariableTrackerImpl::trackAllConstants(const AnalyserModelPtr &analyserModel, bool tracked)
{
    if (validAnalyserModel(analyserModel)) {
        trackVariables(analyserModel->constants(), tracked);
    }
}

void GeneratorVariableTracker::GeneratorVariableTrackerImpl::trackAllComputedConstants(const AnalyserModelPtr &analyserModel, bool tracked)
{
    if (validAnalyserModel(analyserModel)) {
        trackVariables(analyserModel->computedConstants(), tracked);
    }
}

void GeneratorVariableTracker::GeneratorVariableTrackerImpl::trackAllAlgebraicVariables(const AnalyserModelPtr &analyserModel, bool tracked)
{
    if (validAnalyserModel(analyserModel)) {
        trackVariables(analyserModel->algebraicVariables(), tracked);
    }
}

std::vector<AnalyserVariablePtr> GeneratorVariableTracker::GeneratorVariableTrackerImpl::trackableVariables(const AnalyserModelPtr &analyserModel) const
{
    auto res = analyserModel->constants();
    auto computedConstants = analyserModel->computedConstants();
    auto algebraic = analyserModel->algebraicVariables();

    res.insert(res.end(), computedConstants.begin(), computedConstants.end());
    res.insert(res.end(), algebraic.begin(), algebraic.end());

    return res;
}

void GeneratorVariableTracker::GeneratorVariableTrackerImpl::trackAllVariables(const AnalyserModelPtr &analyserModel, bool tracked)
{
    if (validAnalyserModel(analyserModel)) {
        trackVariables(trackableVariables(analyserModel), tracked);
    }
}

size_t GeneratorVariableTracker::GeneratorVariableTrackerImpl::trackedVariableCount(const AnalyserModelPtr &analyserModel,
                                                                                    const std::vector<AnalyserVariablePtr> &analyserVariables, bool tracked)
{
    size_t res = 0;

    for (const auto &analyserVariable : analyserVariables) {
        if (trackableVariable(analyserVariable, tracked, false)) {
            if (isTrackedVariable(analyserModel, analyserVariable, tracked)) {
                ++res;
            }
        } else {
            // The analyser variable is not trackable per se, which means that it is always tracked.

            ++res;
        }
    }

    return res;
}

size_t GeneratorVariableTracker::GeneratorVariableTrackerImpl::trackedConstantCount(const AnalyserModelPtr &analyserModel, bool tracked)
{
    if (analyserModel == nullptr) {
        return 0;
    }

    return trackedVariableCount(analyserModel, analyserModel->constants(), tracked);
}

size_t GeneratorVariableTracker::GeneratorVariableTrackerImpl::trackedComputedConstantCount(const AnalyserModelPtr &analyserModel, bool tracked)
{
    if (analyserModel == nullptr) {
        return 0;
    }

    return trackedVariableCount(analyserModel, analyserModel->computedConstants(), tracked);
}

size_t GeneratorVariableTracker::GeneratorVariableTrackerImpl::trackedAlgebraicVariableCount(const AnalyserModelPtr &analyserModel, bool tracked)
{
    if (analyserModel == nullptr) {
        return 0;
    }

    return trackedVariableCount(analyserModel, analyserModel->algebraicVariables(), tracked);
}

size_t GeneratorVariableTracker::GeneratorVariableTrackerImpl::trackedVariableCount(const AnalyserModelPtr &analyserModel, bool tracked)
{
    if (analyserModel == nullptr) {
        return 0;
    }

    return trackedVariableCount(analyserModel, trackableVariables(analyserModel), tracked);
}

GeneratorVariableTracker::GeneratorVariableTrackerImpl *GeneratorVariableTracker::pFunc()
{
    return reinterpret_cast<GeneratorVariableTracker::GeneratorVariableTrackerImpl *>(Logger::pFunc());
}

/*TODO
const GeneratorVariableTracker::GeneratorVariableTrackerImpl *GeneratorVariableTracker::pFunc() const
{
    return reinterpret_cast<GeneratorVariableTracker::GeneratorVariableTrackerImpl const *>(Logger::pFunc());
}
*/

GeneratorVariableTracker::GeneratorVariableTracker()
    : Logger(new GeneratorVariableTrackerImpl())
{
}

GeneratorVariableTracker::~GeneratorVariableTracker()
{
    delete pFunc();
}

GeneratorVariableTrackerPtr GeneratorVariableTracker::create() noexcept
{
    return std::shared_ptr<GeneratorVariableTracker> {new GeneratorVariableTracker {}};
}

bool GeneratorVariableTracker::isTrackedVariable(const AnalyserVariablePtr &analyserVariable)
{
    return pFunc()->isTrackedVariable(analyserVariable, true);
}

bool GeneratorVariableTracker::isUntrackedVariable(const AnalyserVariablePtr &analyserVariable)
{
    return pFunc()->isTrackedVariable(analyserVariable, false);
}

void GeneratorVariableTracker::trackVariable(const AnalyserVariablePtr &analyserVariable)
{
    pFunc()->trackVariable(analyserVariable, true);
}

void GeneratorVariableTracker::untrackVariable(const AnalyserVariablePtr &analyserVariable)
{
    pFunc()->trackVariable(analyserVariable, false);
}

void GeneratorVariableTracker::trackAllConstants(const AnalyserModelPtr &analyserModel)
{
    pFunc()->trackAllConstants(analyserModel, true);
}

void GeneratorVariableTracker::untrackAllConstants(const AnalyserModelPtr &analyserModel)
{
    pFunc()->trackAllConstants(analyserModel, false);
}

void GeneratorVariableTracker::trackAllComputedConstants(const AnalyserModelPtr &analyserModel)
{
    pFunc()->trackAllComputedConstants(analyserModel, true);
}

void GeneratorVariableTracker::untrackAllComputedConstants(const AnalyserModelPtr &analyserModel)
{
    pFunc()->trackAllComputedConstants(analyserModel, false);
}

void GeneratorVariableTracker::trackAllAlgebraicVariables(const AnalyserModelPtr &analyserModel)
{
    pFunc()->trackAllAlgebraicVariables(analyserModel, true);
}

void GeneratorVariableTracker::untrackAllAlgebraicVariables(const AnalyserModelPtr &analyserModel)
{
    pFunc()->trackAllAlgebraicVariables(analyserModel, false);
}

void GeneratorVariableTracker::trackAllVariables(const AnalyserModelPtr &analyserModel)
{
    pFunc()->trackAllVariables(analyserModel, true);
}

void GeneratorVariableTracker::untrackAllVariables(const AnalyserModelPtr &analyserModel)
{
    pFunc()->trackAllVariables(analyserModel, false);
}

size_t GeneratorVariableTracker::trackedConstantCount(const AnalyserModelPtr &analyserModel)
{
    return pFunc()->trackedConstantCount(analyserModel, true);
}

size_t GeneratorVariableTracker::untrackedConstantCount(const AnalyserModelPtr &analyserModel)
{
    return pFunc()->trackedConstantCount(analyserModel, false);
}

size_t GeneratorVariableTracker::trackedComputedConstantCount(const AnalyserModelPtr &analyserModel)
{
    return pFunc()->trackedComputedConstantCount(analyserModel, true);
}

size_t GeneratorVariableTracker::untrackedComputedConstantCount(const AnalyserModelPtr &analyserModel)
{
    return pFunc()->trackedComputedConstantCount(analyserModel, false);
}

size_t GeneratorVariableTracker::trackedAlgebraicVariableCount(const AnalyserModelPtr &analyserModel)
{
    return pFunc()->trackedAlgebraicVariableCount(analyserModel, true);
}

size_t GeneratorVariableTracker::untrackedAlgebraicVariableCount(const AnalyserModelPtr &analyserModel)
{
    return pFunc()->trackedAlgebraicVariableCount(analyserModel, false);
}

size_t GeneratorVariableTracker::trackedVariableCount(const AnalyserModelPtr &analyserModel)
{
    return pFunc()->trackedVariableCount(analyserModel, true);
}

size_t GeneratorVariableTracker::untrackedVariableCount(const AnalyserModelPtr &analyserModel)
{
    return pFunc()->trackedVariableCount(analyserModel, false);
}

} // namespace libcellml
