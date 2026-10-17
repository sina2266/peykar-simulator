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

#include "libcellml/reset.h"

#include "parentedentity_p.h"

namespace libcellml {

/**
 * @brief The Reset::ResetImpl class.
 *
 * The private implementation for the Reset class.
 */
class Reset::ResetImpl: public ParentedEntityImpl
{
public:
    int mOrder = 0; /**< The relative order of the reset.*/
    bool mOrderSet = false; /**< Whether the relative order of the reset has been set.*/
    VariablePtr mVariable; /**< The associated variable for the reset.*/
    VariablePtr mTestVariable; /**< The associated test_variable for the reset.*/
    std::string mTestValue; /**< The MathML string for the test_value.*/
    std::string mTestValueId; /**< The identifier of the test_value block */
    std::string mResetValue; /**< The MathML string for the reset_value.*/
    std::string mResetValueId; /**< The identifier of the reset_value block */
};

} // namespace libcellml
