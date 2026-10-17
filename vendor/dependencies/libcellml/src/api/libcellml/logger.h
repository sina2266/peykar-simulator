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

#include "libcellml/exportdefinitions.h"
#include "libcellml/types.h"

namespace libcellml {

/**
 * @brief The Logger class.
 *
 * Base class for all serialisable libCellML classes.
 */
class LIBCELLML_EXPORT Logger
{
public:
    virtual ~Logger() = 0; /**< Destructor, @private. */
    Logger(const Logger &rhs) = delete; /**< Copy constructor, @private. */
    Logger(Logger &&rhs) noexcept = delete; /**< Move constructor, @private. */

    /**
     * @brief Get the number of issues.
     *
     * Return the number of issues of any level currently stored in the logger.
     *
     * @return The number of issues.
     */
    size_t issueCount() const;

    /**
     * @brief Get issue of any level at the specified @p index.
     *
     * Returns an issue at the @p index.  If the @p index
     * is not valid a @c nullptr is returned, the valid range for the @p index
     * is [0, \#issues).
     *
     * @param index The index of the issue to return.
     *
     * @return A reference to the issue at the given index on success, @c nullptr otherwise.
     */
    IssuePtr issue(size_t index) const;

    /**
     * @brief Get the number of issues with level of ERROR.
     *
     * Return the number of errors currently stored in the logger.
     *
     * @return The number of errors.
     */
    size_t errorCount() const;

    /**
     * @brief Get issue of level ERROR at the specified @p index.
     *
     * Returns an error at the @p index.  If the @p index
     * is not valid a @c nullptr is returned, the valid range for the @p index
     * is [0, \#errors).
     *
     * @param index The index of the error to return.
     *
     * @return A reference to the error at the given index on success, @c nullptr otherwise.
     */
    IssuePtr error(size_t index) const;

    /**
     * @brief Get the number of issues with level of WARNING.
     *
     * Return the number of warnings currently stored in the logger.
     *
     * @return The number of warnings.
     */
    size_t warningCount() const;

    /**
     * @brief Get issue of level WARNING at the specified @p index.
     *
     * Returns a warning at the @p index.  If the @p index
     * is not valid a @c nullptr is returned, the valid range for the @p index
     * is [0, \#warnings).
     *
     * @param index The index of the warning to return.
     *
     * @return A reference to the warning at the given index on success, @c nullptr otherwise.
     */
    IssuePtr warning(size_t index) const;

    /**
     * @brief Get the number of issues with level of MESSAGE.
     *
     * Return the number of messages currently stored in the logger.
     *
     * @return The number of messages.
     */
    size_t messageCount() const;

    /**
     * @brief Get issue of level MESSAGE at the specified @p index.
     *
     * Returns a message at the @p index.  If the @p index is not valid a
     * @c nullptr is returned, the valid range for the @p index is
     * [0, \#messages).
     *
     * @param index The index of the message to return.
     *
     * @return A reference to the message at the given index on success,
     * @c nullptr otherwise.
     */
    IssuePtr message(size_t index) const;

protected:
    class LoggerImpl; /**< Forward declaration for pImpl idiom, @private. */

    explicit Logger(LoggerImpl *derivedPimpl); /**< Constructor, @private. */

    /**
     * @brief Getter for private implementation pointer.
     *
     * Getter for private implementation pointer, @private.
     *
     * @return A pointer to EntityImpl.
     */
    LoggerImpl *pFunc();

    /**
     * @brief Const getter for private implementation pointer.
     *
     * Const getter for private implementation pointer, @private.
     *
     * @return A pointer to const EntityImpl.
     */
    const LoggerImpl *pFunc() const;

private:
    LoggerImpl *mPimpl; /**< Private member to implementation pointer, @private. */
};

} // namespace libcellml
