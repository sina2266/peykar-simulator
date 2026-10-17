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

#include "libcellml/logger.h"

namespace libcellml {

/**
 * @brief The Logger::LoggerImpl class.
 *
 * This class is the private implementation class for the LoggerImpl class.  Separating
 * the implementation from the definition allows for greater flexibility when
 * distributing the code.
 */
class Logger::LoggerImpl
{
public:
    std::vector<size_t> mErrors;
    std::vector<size_t> mWarnings;
    std::vector<size_t> mMessages;
    std::vector<IssuePtr> mIssues;

    /**
     * @brief Add an issue to the logger.
     *
     * Adds the argument @p issue to this logger.
     *
     * @param issue The @c IssuePtr to add.
     */
    void addIssue(const IssuePtr &issue);

    /**
     * @brief Remove issue of level ERROR at the specified @p index.
     *
     * Removes an error at @p index.
     *
     * @param index The index of the error to remove.
     */
    void removeError(size_t index);

    /**
     * @brief Clear the issues from the logger.
     *
     * Clear the issues from the logger.
     */
    void removeAllIssues();
};

} // namespace libcellml
