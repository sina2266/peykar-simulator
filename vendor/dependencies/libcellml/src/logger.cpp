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

#include "libcellml/logger.h"

#include <vector>

#include "libcellml/issue.h"
#include "libcellml/types.h"

#include "logger_p.h"

namespace libcellml {

Logger::LoggerImpl *Logger::pFunc()
{
    return mPimpl;
}

const Logger::LoggerImpl *Logger::pFunc() const
{
    return mPimpl;
}

Logger::Logger(LoggerImpl *derivedPimpl)
    : mPimpl(derivedPimpl)
{
}

Logger::~Logger() = default;

size_t Logger::errorCount() const
{
    return pFunc()->mErrors.size();
}

IssuePtr Logger::error(size_t index) const
{
    IssuePtr issue = nullptr;
    if (index < pFunc()->mErrors.size()) {
        issue = pFunc()->mIssues.at(pFunc()->mErrors.at(index));
    }
    return issue;
}

size_t Logger::warningCount() const
{
    return pFunc()->mWarnings.size();
}

IssuePtr Logger::warning(size_t index) const
{
    IssuePtr issue = nullptr;
    if (index < pFunc()->mWarnings.size()) {
        issue = pFunc()->mIssues.at(pFunc()->mWarnings.at(index));
    }
    return issue;
}

size_t Logger::messageCount() const
{
    return pFunc()->mMessages.size();
}

IssuePtr Logger::message(size_t index) const
{
    IssuePtr issue = nullptr;
    if (index < pFunc()->mMessages.size()) {
        issue = pFunc()->mIssues.at(pFunc()->mMessages.at(index));
    }
    return issue;
}

void Logger::LoggerImpl::removeAllIssues()
{
    mIssues.clear();
    mErrors.clear();
    mWarnings.clear();
    mMessages.clear();
}

void Logger::LoggerImpl::removeError(size_t index)
{
    mIssues.erase(mIssues.begin() + ptrdiff_t(mErrors.at(index)));
    mErrors.erase(mErrors.begin() + ptrdiff_t(index));
}

void Logger::LoggerImpl::addIssue(const IssuePtr &issue)
{
    // When an issue is added, update the appropriate array based on its level.
    size_t index = mIssues.size();
    mIssues.push_back(issue);
    libcellml::Issue::Level level = issue->level();
    switch (level) {
    case libcellml::Issue::Level::ERROR:
        mErrors.push_back(index);
        break;
    case libcellml::Issue::Level::WARNING:
        mWarnings.push_back(index);
        break;
    default:
        // Only remaining level is MESSAGE.
        mMessages.push_back(index);
        break;
    }
}

size_t Logger::issueCount() const
{
    return pFunc()->mIssues.size();
}

IssuePtr Logger::issue(size_t index) const
{
    IssuePtr issue = nullptr;
    if (index < pFunc()->mIssues.size()) {
        issue = pFunc()->mIssues.at(index);
    }
    return issue;
}

} // namespace libcellml
