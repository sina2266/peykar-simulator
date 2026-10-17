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

#include "libcellml/analyservariable.h"

#include "internaltypes.h"

namespace libcellml {

/**
 * @brief The AnalyserVariable::AnalyserVariableImpl struct.
 *
 * The private implementation for the AnalyserVariable class.
 */
struct AnalyserVariable::AnalyserVariableImpl
{
    AnalyserVariable::Type mType = Type::CONSTANT;
    size_t mIndex = 0;
    VariablePtr mInitialisingVariable;
    VariablePtr mVariable;
    AnalyserModelWeakPtr mAnalyserModel;
    std::vector<AnalyserEquationWeakPtr> mAnalyserEquations;

    static AnalyserVariablePtr create();

    void populate(AnalyserVariable::Type type, size_t index, const VariablePtr &initialisingVariable,
                  const VariablePtr &variable, const AnalyserModelPtr &analyserModel,
                  const std::vector<AnalyserEquationPtr> &analyserEquations);

    bool constantWithDummyEquation() const;
};

} // namespace libcellml
