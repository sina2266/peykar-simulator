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

#include "libcellml/analyserexternalvariable.h"

namespace libcellml {

/**
 * @brief The AnalyserExternalVariable::AnalyserExternalVariableImpl struct.
 *
 * The private implementation for the AnalyserExternalVariable class.
 */
struct AnalyserExternalVariable::AnalyserExternalVariableImpl
{
    VariablePtr mVariable;
    std::vector<VariablePtr> mDependencies;

    explicit AnalyserExternalVariableImpl(const VariablePtr &variable);

    std::vector<VariablePtr>::iterator findDependency(const ModelPtr &model,
                                                      const std::string &componentName,
                                                      const std::string &variableName);
    std::vector<VariablePtr>::iterator findDependency(const VariablePtr &variable);
};

} // namespace libcellml
