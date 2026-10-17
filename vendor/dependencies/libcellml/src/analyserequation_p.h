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

#include "libcellml/analyserequation.h"

#include "internaltypes.h"

namespace libcellml {

/**
 * @brief The AnalyserEquation::AnalyserEquationImpl struct.
 *
 * The private implementation for the AnalyserEquation class.
 */
struct AnalyserEquation::AnalyserEquationImpl
{
    AnalyserEquation::Type mType = AnalyserEquation::Type::ALGEBRAIC;
    AnalyserEquationAstPtr mAst;
    size_t mNlaSystemIndex;
    bool mIsStateRateBased = false;

    AnalyserVariablePtr mConstant;

    std::vector<AnalyserVariablePtr> mStates;
    std::vector<AnalyserVariablePtr> mComputedConstants;
    std::vector<AnalyserVariablePtr> mAlgebraicVariables;
    std::vector<AnalyserVariablePtr> mExternalVariables;

    std::vector<AnalyserVariablePtr> mConstantDependencies;
    std::vector<AnalyserEquationWeakPtr> mDependencies;
    std::vector<AnalyserEquationWeakPtr> mNlaSiblings;

    static AnalyserEquationPtr create();

    static bool isStagingDependency(const AnalyserEquationWeakPtr &dependency);

    void removeStagingDependencies();
};

} // namespace libcellml
