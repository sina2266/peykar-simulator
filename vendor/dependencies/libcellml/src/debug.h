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

#include <iostream>
#include <sstream>

#include "internaltypes.h"

namespace libcellml {

/**
 * @brief Internal class for printing output to the terminal.
 *
 * Internal class intended to be used for debugging purposes where
 * information can be printed to the standard output.
 *
 * Use:
 *   Debug() << "debug information";
 *
 * This class automatically adds a new line to the end of a statement.
 * Note: This class can not be used over multiple lines, it is designed
 * for single line statements only.
 */
struct Debug
{
    explicit Debug(bool newLine = true)
        : mNewLine(newLine)
    {
    }

    /**
     * @brief Destructor writes stream to standard output when deleted.
     *
     * Writes out the contents of the buffer to standard output when deleted.
     */
    ~Debug()
    {
        std::cout << mSS.str();
        if (mNewLine) {
            std::cout << std::endl;
        }
    }

    /**
     * @brief Overload the operator <<.
     *
     * Overload the operator << to accept pointers.  Returns itself
     * so that the operator can be chained.
     *
     * @param p A castable to const void pointer parameter.
     * @return This object.
     */
    Debug &operator<<(const void *p)
    {
        std::ostringstream ss;
        ss << static_cast<const void *>(p);
        mSS << ss.str();
        return *this;
    }

    /**
     * @brief Overload the operator << to accept just about anything.
     *
     * Accept just about anything and add it to the buffer.  Returns itself
     * so that the operator can be chained.
     *
     * @return This object.
     */
    template<class T>
    Debug &operator<<(const T &x)
    {
        mSS << x;
        return *this;
    }

private:
    std::ostringstream mSS;
    bool mNewLine;
};

void listModelsUnits(const ModelPtr &model);
void printAnalyserModelEquations(const AnalyserModelPtr &analyserModel);
void printAnalyserModelVariables(const AnalyserModelPtr &analyserModel);
void printAstAsTree(const AnalyserEquationAstPtr &ast);
void printAstAsCode(const AnalyserEquationAstPtr &ast);
void printComponentMap(const ComponentMap &map);
void printConnectionMap(const ConnectionMap &map);
void printEquivalenceMap(const EquivalenceMap &map);
void printEquivalenceMapWithModelInfo(const EquivalenceMap &map, const ModelPtr &model);
void printEquivalences(const VariablePtr &variable);
void printHistory(const History &history);
void printHistoryEpoch(const HistoryEpochPtr &historyEpoch);
void printImportLibrary(const ImportLibrary &importlibrary);
void printNamedPath(const ParentedEntityPtr &parented);
void printStack(const IndexStack &stack);
void printStackWithModelInfo(const IndexStack &stack, const ModelPtr &model);
void printStringStringMap(const StringStringMap &map);
void printUnits(const UnitsPtr &units);
void printVariableMap(const VariableMap &map);

} // namespace libcellml
