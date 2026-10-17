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

#include "libcellml/analysermodel.h"

#include "analysermodel_p.h"
#include "analyservariable_p.h"
#include "utilities.h"

namespace libcellml {

AnalyserModelPtr AnalyserModel::AnalyserModelImpl::create(const ModelPtr &model)
{
    auto res = std::shared_ptr<AnalyserModel> {new AnalyserModel(model)};

    if (model) {
        res->mPimpl->buildEquivalentVariablesCache();
    }

    return res;
}

AnalyserModel::AnalyserModelImpl::AnalyserModelImpl(const ModelPtr &model)
    : mModel(model)
{
}

AnalyserModel::AnalyserModel(const ModelPtr &model)
    : mPimpl(new AnalyserModelImpl(model))
{
}

AnalyserModel::~AnalyserModel()
{
    delete mPimpl;
}

void exploreEquivalentVariables(const VariablePtr &variable, std::unordered_set<uintptr_t> &equivalentGroup, std::unordered_set<uintptr_t> &visited)
{
    auto rawPtr = reinterpret_cast<uintptr_t>(variable.get());

    if (visited.insert(rawPtr).second) {
        equivalentGroup.insert(rawPtr);

        for (size_t i = 0; i < variable->equivalentVariableCount(); ++i) {
            exploreEquivalentVariables(variable->equivalentVariable(i), equivalentGroup, visited);
        }
    }
}

void AnalyserModel::AnalyserModelImpl::buildEquivalentVariablesCache()
{
    std::unordered_set<uintptr_t> visited;
    size_t groupCount = 0;
    mEquivalentVariableCache.clear();

    for (size_t i = 0; i < mModel->componentCount(); ++i) {
        buildEquivalentVariablesCache(mModel->component(i), visited, groupCount);
    }
}

void AnalyserModel::AnalyserModelImpl::buildEquivalentVariablesCache(const ComponentPtr &component, std::unordered_set<uintptr_t> &visited, size_t &groupCount)
{
    for (size_t i = 0; i < component->variableCount(); ++i) {
        auto variable = component->variable(i);
        auto rawPtr = reinterpret_cast<uintptr_t>(variable.get());

        if (visited.count(rawPtr) == 0) {
            std::unordered_set<uintptr_t> equivalentGroup;
            exploreEquivalentVariables(variable, equivalentGroup, visited);

            for (uintptr_t v : equivalentGroup) {
                mEquivalentVariableCache[v] = groupCount;
            }

            ++groupCount;
        }
    }

    for (size_t i = 0; i < component->componentCount(); ++i) {
        buildEquivalentVariablesCache(component->component(i), visited, groupCount);
    }
}

bool AnalyserModel::isValid() const
{
    switch (mPimpl->mType) {
    case AnalyserModel::Type::ODE:
    case AnalyserModel::Type::DAE:
    case AnalyserModel::Type::NLA:
    case AnalyserModel::Type::ALGEBRAIC:
        return true;
    default:
        return false;
    }
}

AnalyserModel::Type AnalyserModel::type() const
{
    return mPimpl->mType;
}

std::string AnalyserModel::typeAsString(Type type)
{
    static constexpr const char *names[] = {
        "unknown",
        "algebraic",
        "dae",
        "invalid",
        "nla",
        "ode",
        "overconstrained",
        "underconstrained",
        "unsuitably_constrained",
    };

    return names[static_cast<size_t>(type)];
}

bool AnalyserModel::hasExternalVariables() const
{
    if (!isValid()) {
        return false;
    }

    return mPimpl->mHasExternalVariables;
}

AnalyserVariablePtr AnalyserModel::voi() const
{
    if (!isValid()) {
        return {};
    }

    return mPimpl->mVoi;
}

size_t AnalyserModel::stateCount() const
{
    if (!isValid()) {
        return 0;
    }

    return mPimpl->mStates.size();
}

std::vector<AnalyserVariablePtr> AnalyserModel::states() const
{
    if (!isValid()) {
        return {};
    }

    return mPimpl->mStates;
}

AnalyserVariablePtr AnalyserModel::state(size_t index) const
{
    if (!isValid()
        || (index >= mPimpl->mStates.size())) {
        return {};
    }

    return mPimpl->mStates[index];
}

size_t AnalyserModel::constantCount() const
{
    if (!isValid()) {
        return 0;
    }

    return mPimpl->mConstants.size();
}

std::vector<AnalyserVariablePtr> AnalyserModel::constants() const
{
    if (!isValid()) {
        return {};
    }

    return mPimpl->mConstants;
}

AnalyserVariablePtr AnalyserModel::constant(size_t index) const
{
    if (!isValid() || (index >= mPimpl->mConstants.size())) {
        return {};
    }

    return mPimpl->mConstants[index];
}

size_t AnalyserModel::computedConstantCount() const
{
    if (!isValid()) {
        return 0;
    }

    return mPimpl->mComputedConstants.size();
}

std::vector<AnalyserVariablePtr> AnalyserModel::computedConstants() const
{
    if (!isValid()) {
        return {};
    }

    return mPimpl->mComputedConstants;
}

AnalyserVariablePtr AnalyserModel::computedConstant(size_t index) const
{
    if (!isValid() || (index >= mPimpl->mComputedConstants.size())) {
        return {};
    }

    return mPimpl->mComputedConstants[index];
}

size_t AnalyserModel::algebraicVariableCount() const
{
    if (!isValid()) {
        return 0;
    }

    return mPimpl->mAlgebraicVariables.size();
}

std::vector<AnalyserVariablePtr> AnalyserModel::algebraicVariables() const
{
    if (!isValid()) {
        return {};
    }

    return mPimpl->mAlgebraicVariables;
}

AnalyserVariablePtr AnalyserModel::algebraicVariable(size_t index) const
{
    if (!isValid() || (index >= mPimpl->mAlgebraicVariables.size())) {
        return {};
    }

    return mPimpl->mAlgebraicVariables[index];
}

size_t AnalyserModel::externalVariableCount() const
{
    if (!isValid()) {
        return 0;
    }

    return mPimpl->mExternalVariables.size();
}

std::vector<AnalyserVariablePtr> AnalyserModel::externalVariables() const
{
    if (!isValid()) {
        return {};
    }

    return mPimpl->mExternalVariables;
}

AnalyserVariablePtr AnalyserModel::externalVariable(size_t index) const
{
    if (!isValid() || (index >= mPimpl->mExternalVariables.size())) {
        return {};
    }

    return mPimpl->mExternalVariables[index];
}

AnalyserVariablePtr AnalyserModel::analyserVariable(const VariablePtr &variable)
{
    if (!isValid() || (variable == nullptr)) {
        return {};
    }

    if (mPimpl->mVoi && areEquivalentVariables(variable, mPimpl->mVoi->variable())) {
        return mPimpl->mVoi;
    }

    for (const auto &state : mPimpl->mStates) {
        if (areEquivalentVariables(variable, state->variable())) {
            return state;
        }
    }

    for (const auto &constant : mPimpl->mConstants) {
        if (areEquivalentVariables(variable, constant->variable())) {
            return constant;
        }
    }

    for (const auto &computedConstant : mPimpl->mComputedConstants) {
        if (areEquivalentVariables(variable, computedConstant->variable())) {
            return computedConstant;
        }
    }

    for (const auto &algebraicVariable : mPimpl->mAlgebraicVariables) {
        if (areEquivalentVariables(variable, algebraicVariable->variable())) {
            return algebraicVariable;
        }
    }

    for (const auto &externalVariable : mPimpl->mExternalVariables) {
        if (areEquivalentVariables(variable, externalVariable->variable())) {
            return externalVariable;
        }
    }

    return {};
}

size_t AnalyserModel::analyserEquationCount() const
{
    if (!isValid()) {
        return 0;
    }

    return mPimpl->mAnalyserEquations.size();
}

std::vector<AnalyserEquationPtr> AnalyserModel::analyserEquations() const
{
    if (!isValid()) {
        return {};
    }

    return mPimpl->mAnalyserEquations;
}

AnalyserEquationPtr AnalyserModel::analyserEquation(size_t index) const
{
    if (!isValid() || (index >= mPimpl->mAnalyserEquations.size())) {
        return {};
    }

    return mPimpl->mAnalyserEquations[index];
}

bool AnalyserModel::needEqFunction() const
{
    if (!isValid()) {
        return false;
    }

    return mPimpl->mNeedEqFunction;
}

bool AnalyserModel::needNeqFunction() const
{
    if (!isValid()) {
        return false;
    }

    return mPimpl->mNeedNeqFunction;
}

bool AnalyserModel::needLtFunction() const
{
    if (!isValid()) {
        return false;
    }

    return mPimpl->mNeedLtFunction;
}

bool AnalyserModel::needLeqFunction() const
{
    if (!isValid()) {
        return false;
    }

    return mPimpl->mNeedLeqFunction;
}

bool AnalyserModel::needGtFunction() const
{
    if (!isValid()) {
        return false;
    }

    return mPimpl->mNeedGtFunction;
}

bool AnalyserModel::needGeqFunction() const
{
    if (!isValid()) {
        return false;
    }

    return mPimpl->mNeedGeqFunction;
}

bool AnalyserModel::needAndFunction() const
{
    if (!isValid()) {
        return false;
    }

    return mPimpl->mNeedAndFunction;
}

bool AnalyserModel::needOrFunction() const
{
    if (!isValid()) {
        return false;
    }

    return mPimpl->mNeedOrFunction;
}

bool AnalyserModel::needXorFunction() const
{
    if (!isValid()) {
        return false;
    }

    return mPimpl->mNeedXorFunction;
}

bool AnalyserModel::needNotFunction() const
{
    if (!isValid()) {
        return false;
    }

    return mPimpl->mNeedNotFunction;
}

bool AnalyserModel::needMinFunction() const
{
    if (!isValid()) {
        return false;
    }

    return mPimpl->mNeedMinFunction;
}

bool AnalyserModel::needMaxFunction() const
{
    if (!isValid()) {
        return false;
    }

    return mPimpl->mNeedMaxFunction;
}

bool AnalyserModel::needSecFunction() const
{
    if (!isValid()) {
        return false;
    }

    return mPimpl->mNeedSecFunction;
}

bool AnalyserModel::needCscFunction() const
{
    if (!isValid()) {
        return false;
    }

    return mPimpl->mNeedCscFunction;
}

bool AnalyserModel::needCotFunction() const
{
    if (!isValid()) {
        return false;
    }

    return mPimpl->mNeedCotFunction;
}

bool AnalyserModel::needSechFunction() const
{
    if (!isValid()) {
        return false;
    }

    return mPimpl->mNeedSechFunction;
}

bool AnalyserModel::needCschFunction() const
{
    if (!isValid()) {
        return false;
    }

    return mPimpl->mNeedCschFunction;
}

bool AnalyserModel::needCothFunction() const
{
    if (!isValid()) {
        return false;
    }

    return mPimpl->mNeedCothFunction;
}

bool AnalyserModel::needAsecFunction() const
{
    if (!isValid()) {
        return false;
    }

    return mPimpl->mNeedAsecFunction;
}

bool AnalyserModel::needAcscFunction() const
{
    if (!isValid()) {
        return false;
    }

    return mPimpl->mNeedAcscFunction;
}

bool AnalyserModel::needAcotFunction() const
{
    if (!isValid()) {
        return false;
    }

    return mPimpl->mNeedAcotFunction;
}

bool AnalyserModel::needAsechFunction() const
{
    if (!isValid()) {
        return false;
    }

    return mPimpl->mNeedAsechFunction;
}

bool AnalyserModel::needAcschFunction() const
{
    if (!isValid()) {
        return false;
    }

    return mPimpl->mNeedAcschFunction;
}

bool AnalyserModel::needAcothFunction() const
{
    if (!isValid()) {
        return false;
    }

    return mPimpl->mNeedAcothFunction;
}

bool AnalyserModel::areEquivalentVariables(const VariablePtr &variable1,
                                           const VariablePtr &variable2)
{
    // This is a cached version of the areEquivalentVariables() utility. Indeed,
    // an AnalyserModel object refers to a static version of a model, which
    // means that we can safely cache the result of a call to that utility. In
    // turn, this means that we can speed up any feature (e.g., code generation)
    // that also relies on that utility.

    if ((variable1 == nullptr) || (variable2 == nullptr)) {
        return false;
    }

    if (variable1 == variable2) {
        return true;
    }

    const auto v1 = reinterpret_cast<uintptr_t>(variable1.get());
    const auto v2 = reinterpret_cast<uintptr_t>(variable2.get());

    const auto it1 = mPimpl->mEquivalentVariableCache.find(v1);

    if (it1 == mPimpl->mEquivalentVariableCache.end()) {
        return false;
    }

    const auto it2 = mPimpl->mEquivalentVariableCache.find(v2);

    if (it2 == mPimpl->mEquivalentVariableCache.end()) {
        return false;
    }

    return it1->second == it2->second;
}

} // namespace libcellml
