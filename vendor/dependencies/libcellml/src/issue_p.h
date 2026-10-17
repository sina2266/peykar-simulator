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

#include "libcellml/issue.h"

#include "anycellmlelement_p.h"

namespace libcellml {

/**
 * @brief The Issue::IssueImpl struct.
 *
 * The private implementation for the Issue class.
 */
struct Issue::IssueImpl
{
    std::string mDescription; /**< The string description for why this issue was raised. */
    Issue::Level mLevel = Issue::Level::ERROR; /**< The Issue::Level enum value for this issue. */
    Issue::ReferenceRule mReferenceRule = Issue::ReferenceRule::UNDEFINED; /**< The Issue::ReferenceRule enum value for this issue. */
    AnyCellmlElementPtr mItem = AnyCellmlElement::AnyCellmlElementImpl::create(); /**< The item for this issue. */

    /**
     * @brief Create an issue.
     *
     * Create an issue.
     *
     * @return The issue.
     */
    static IssuePtr create();

    void setDescription(const std::string &description);
    void setLevel(Level level);
    void setReferenceRule(ReferenceRule referenceRule);
};

} // namespace libcellml
