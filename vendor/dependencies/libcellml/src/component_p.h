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

#include "libcellml/component.h"

#include "componententity_p.h"
#include "internaltypes.h"

namespace libcellml {

/**
 * @brief The Component::ComponentImpl class.
 *
 * This class is the private implementation struct for the Component class.  Separating
 * the implementation from the definition allows for greater flexibility when
 * distributing the code.
 */
class Component::ComponentImpl: public ComponentEntityImpl
{
public:
    Component *mComponent = nullptr;
    std::string mMath;
    std::vector<ResetPtr> mResets;
    std::vector<VariablePtr> mVariables;

    std::vector<ResetPtr>::const_iterator findReset(const ResetPtr &reset) const;
    std::vector<VariablePtr>::const_iterator findVariable(const std::string &name) const;
    std::vector<VariablePtr>::const_iterator findVariable(const VariablePtr &variable) const;

    bool equalVariables(const ComponentPtr &other) const;
    bool equalResets(const ComponentPtr &other) const;

    bool performTestWithHistory(History &history, const ComponentConstPtr &component, TestType type) const;
};

} // namespace libcellml
