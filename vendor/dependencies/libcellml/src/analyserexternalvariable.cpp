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

#include "libcellml/analyserexternalvariable.h"

#include "analyserexternalvariable_p.h"
#include "commonutils.h"
#include "utilities.h"

namespace libcellml {

AnalyserExternalVariable::AnalyserExternalVariableImpl::AnalyserExternalVariableImpl(const VariablePtr &variable)
    : mVariable(variable)
{
}

std::vector<VariablePtr>::iterator AnalyserExternalVariable::AnalyserExternalVariableImpl::findDependency(const ModelPtr &model,
                                                                                                          const std::string &componentName,
                                                                                                          const std::string &variableName)
{
    return std::find_if(mDependencies.begin(), mDependencies.end(), [=](const auto &v) {
        return (owningModel(v) == model)
               && (owningComponent(v)->name() == componentName)
               && (v->name() == variableName);
    });
}

std::vector<VariablePtr>::iterator AnalyserExternalVariable::AnalyserExternalVariableImpl::findDependency(const VariablePtr &variable)
{
    return std::find_if(mDependencies.begin(), mDependencies.end(), [=](const auto &v) {
        return v == variable;
    });
}

AnalyserExternalVariable::AnalyserExternalVariable(const VariablePtr &variable)
    : mPimpl(new AnalyserExternalVariableImpl(variable))
{
}

AnalyserExternalVariable::~AnalyserExternalVariable()
{
    delete mPimpl;
}

AnalyserExternalVariablePtr AnalyserExternalVariable::create(const VariablePtr &variable) noexcept
{
    return std::shared_ptr<AnalyserExternalVariable> {new AnalyserExternalVariable {variable}};
}

VariablePtr AnalyserExternalVariable::variable() const
{
    return mPimpl->mVariable;
}

bool AnalyserExternalVariable::addDependency(const VariablePtr &variable)
{
    auto pimplVariable = AnalyserExternalVariable::variable();

    if ((pimplVariable != nullptr)
        && (owningModel(variable) == owningModel(pimplVariable))
        && (mPimpl->findDependency(variable) == mPimpl->mDependencies.end())
        && !areEquivalentVariables(variable, pimplVariable)) {
        mPimpl->mDependencies.push_back(variable);

        return true;
    }

    return false;
}

bool AnalyserExternalVariable::removeDependency(size_t index)
{
    if (index < mPimpl->mDependencies.size()) {
        mPimpl->mDependencies.erase(mPimpl->mDependencies.begin() + ptrdiff_t(index));

        return true;
    }

    return false;
}

bool AnalyserExternalVariable::removeDependency(const ModelPtr &model,
                                                const std::string &componentName,
                                                const std::string &variableName)
{
    auto result = mPimpl->findDependency(model, componentName, variableName);

    if (result != mPimpl->mDependencies.end()) {
        mPimpl->mDependencies.erase(result);

        return true;
    }

    return false;
}

bool AnalyserExternalVariable::removeDependency(const VariablePtr &variable)
{
    auto result = mPimpl->findDependency(variable);

    if (result != mPimpl->mDependencies.end()) {
        mPimpl->mDependencies.erase(result);

        return true;
    }

    return false;
}

void AnalyserExternalVariable::removeAllDependencies()
{
    mPimpl->mDependencies.clear();
}

bool AnalyserExternalVariable::containsDependency(const ModelPtr &model,
                                                  const std::string &componentName,
                                                  const std::string &variableName) const
{
    return mPimpl->findDependency(model, componentName, variableName) != mPimpl->mDependencies.end();
}

bool AnalyserExternalVariable::containsDependency(const VariablePtr &variable) const
{
    return mPimpl->findDependency(variable) != mPimpl->mDependencies.end();
}

VariablePtr AnalyserExternalVariable::dependency(size_t index) const
{
    return (index < mPimpl->mDependencies.size()) ?
               mPimpl->mDependencies.at(index) :
               nullptr;
}

VariablePtr AnalyserExternalVariable::dependency(const ModelPtr &model,
                                                 const std::string &componentName,
                                                 const std::string &variableName) const
{
    auto result = mPimpl->findDependency(model, componentName, variableName);

    return (result != mPimpl->mDependencies.end()) ?
               *result :
               nullptr;
}

std::vector<VariablePtr> AnalyserExternalVariable::dependencies() const
{
    return mPimpl->mDependencies;
}

size_t AnalyserExternalVariable::dependencyCount() const
{
    return mPimpl->mDependencies.size();
}

} // namespace libcellml
