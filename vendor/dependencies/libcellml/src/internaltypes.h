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

#include <map>
#include <set>
#include <unordered_set>
#include <vector>

#include "libcellml/component.h"
#include "libcellml/model.h"
#include "libcellml/units.h"

namespace libcellml {

const std::string ORIGIN_MODEL_REF = ":this:";

using ComponentNameMap = std::map<std::string, ComponentPtr>; /**< Type definition for map of component name to component pointer. */

using IndexStack = std::vector<size_t>; /**< Type definition for tracking indices. */
using EquivalenceMap = std::map<IndexStack, std::vector<IndexStack>>; /**< Type definition for map of variable equivalences defined over model. */

using NamePair = std::pair<std::string, std::string>; /**< Type definition for pair of names. */
using NameList = std::vector<std::string>; /**< Type definition for list of names. */
using DescriptionList = std::vector<std::pair<VariablePtr, std::string>>; /**< Type definition for list of variables and associated description. */
using StringStringMap = std::map<std::string, std::string>; /**< Type definition for map of string to string. */
using UniqueNames = std::set<std::string>; /**< Type definition for a set of unique names. */
using NodeAttributeNamespaceInfo = std::vector<std::tuple<std::string, std::string, std::string, std::string, std::string>>; /**< Type definition for attribute namespace information. */

// VariableMap
using VariableMap = std::vector<VariablePairPtr>; /**< Type definition for vector of VariablePair.*/
using VariableMapIterator = VariableMap::const_iterator; /**< Type definition of const iterator for vector of VariablePair.*/

// ComponentMap
using ComponentPair = std::pair<ComponentPtr, ComponentPtr>; /**< Type definition for Component pointer pair using standard library.*/
using ComponentMap = std::vector<ComponentPair>; /**< Type definition for vector of ComponentPair.*/
using ComponentMapIterator = ComponentMap::const_iterator; /**< Type definition of const iterator for vector of ComponentPair.*/

using VariablePtrs = std::vector<VariablePtr>; /**< Type definition for list of variables. */

using IdMap = std::map<std::string, std::pair<int, std::vector<std::string>>>; /**< Type definition for map of IDs in Validator. **/
using ImportLibrary = std::map<std::string, ModelPtr>; /** Type definition for library map of imported models. */
using IdList = std::unordered_set<std::string>; /**< Type definition for list of identifiers. */

using ResetOrderMap = std::map<VariablePtr, std::vector<int>>; /** Type definition for map of variable to reset order. **/

using AnalyserEquationAstWeakPtr = std::weak_ptr<AnalyserEquationAst>; /**< Type definition for weak analyser equation AST pointer. */
using AnalyserEquationWeakPtr = std::weak_ptr<AnalyserEquation>; /**< Type definition for weak analyser equation pointer. */
using AnalyserModelWeakPtr = std::weak_ptr<AnalyserModel>; /**< Type definition for weak analyser model pointer. */
using AnalyserVariableWeakPtr = std::weak_ptr<AnalyserVariable>; /**< Type definition for weak analyser variable pointer. */
using ComponentWeakPtr = std::weak_ptr<Component>; /**< Type definition for weak component pointer. */
using ImportSourceWeakPtr = std::weak_ptr<ImportSource>; /**< Type definition for weak import source pointer. */
using ModelWeakPtr = std::weak_ptr<Model>; /**< Type definition for weak model pointer. */
using ResetWeakPtr = std::weak_ptr<Reset>; /**< Type definition for weak reset pointer. */
using UnitsWeakPtr = std::weak_ptr<Units>; /**< Type definition for weak units pointer. */
using VariableWeakPtr = std::weak_ptr<Variable>; /**< Type definition for weak variable pointer. */

using ComponentConstPtr = std::shared_ptr<const Component>; /**< Type definition for shared component const pointer. */
using ComponentEntityConstPtr = std::shared_ptr<const ComponentEntity>; /**< Type definition for shared component entity const pointer. */
using ImportedEntityConstPtr = std::shared_ptr<const ImportedEntity>; /**< Type definition for shared imported entity const pointer. */
using ModelConstPtr = std::shared_ptr<const Model>; /**< Type definition for shared model const pointer. */
using ParentedEntityConstPtr = std::shared_ptr<const ParentedEntity>; /**< Type definition for shared parented entity const pointer. */
using UnitsConstPtr = std::shared_ptr<const Units>; /**< Type definition for shared units const pointer. */

using ConnectionMap = std::map<VariablePtr, VariablePtr>; /**< Type definition for a connection map.*/
using NamePairList = std::vector<NamePair>; /**< Type definition for a list of a pair of names. */

using ComponentRawPtrPair = std::pair<const Component *, const Component *>; /**< Type definition for pair of raw component pointers. */
using ConnectionIdMap = std::map<ComponentRawPtrPair, std::string>; /**< Type definition for map of pair of raw component pointers to connection ID. */

/**
 * @brief Class for defining an epoch in the history of a @ref Component or @ref Units.
 *
 * This class holds information for defining an epoch in the history of resolving a @ref Component
 * or @ref Units.
 */
class HistoryEpoch
{
public:
    ModelPtr mDestinationModel;
    std::string mDestinationUrl;
    ModelPtr mSourceModel;
    std::string mName;
    std::string mReferenceName;
    std::string mSourceUrl;
    std::string mType;

    /**
     * @brief Create a HistoryEpoch.
     *
     * Create a history epoch for a @ref Units.
     *
     * @param units The @p units to create an epoch for.
     * @param sourceUrl The source URL of the @p units.
     * @param destinationUrl The destination URL of the @p units.
     */
    HistoryEpoch(const UnitsConstPtr &units, const std::string &sourceUrl, const std::string &destinationUrl);

    /**
     * @brief Create a HistoryEpoch.
     *
     * Create a history epoch for a @ref Component.
     *
     * @param component The @p component to create an epoch for.
     * @param sourceUrl The source URL of the @p component.
     * @param destinationUrl The destination URL of the @p component.
     */
    HistoryEpoch(const ComponentConstPtr &component, const std::string &sourceUrl, const std::string &destinationUrl);

    /**
     * @brief Set the reference name for the imported entity.
     *
     * If the entity is imported, set the reference name for it.
     *
     * @param importedEntity The imported entity to set the reference name for.
     */
    void setReferenceName(const ImportedEntityConstPtr &importedEntity);

    /**
     * @brief Set the destination model to the resolving model from the imported entity.
     *
     * If the entity is imported, set the destination model to the resolving model for it.
     *
     * @param importedEntity The imported entity to set the destination model for.
     */
    void setDestinationModel(const ImportedEntityConstPtr &importedEntity);
};

using HistoryEpochPtr = std::shared_ptr<HistoryEpoch>; /**< Type definition for shared history epoch pointer. */
using History = std::vector<HistoryEpochPtr>; /**< Type definition for history.*/

using Strings = std::vector<std::string>; /**< Type definition for strings.*/

enum class TestType
{
    RESOLVED,
    DEFINED,
};

} // namespace libcellml
