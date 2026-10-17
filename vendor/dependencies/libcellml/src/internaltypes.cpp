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

#include "internaltypes.h"

#include "commonutils.h"
#include "libcellml/importsource.h"

namespace libcellml {

HistoryEpoch::HistoryEpoch(const UnitsConstPtr &units, const std::string &sourceUrl, const std::string &destinationUrl)
    : mDestinationModel(nullptr)
    , mDestinationUrl(destinationUrl)
    , mSourceModel(owningModel(units))
    , mName(units->name())
    , mSourceUrl(sourceUrl)
    , mType("units")
{
    setReferenceName(units);
    setDestinationModel(units);
}

HistoryEpoch::HistoryEpoch(const ComponentConstPtr &component, const std::string &sourceUrl, const std::string &destinationUrl)
    : mDestinationModel(nullptr)
    , mDestinationUrl(destinationUrl)
    , mSourceModel(owningModel(component))
    , mName(component->name())
    , mSourceUrl(sourceUrl)
    , mType("component")
{
    setReferenceName(component);
    setDestinationModel(component);
}

void HistoryEpoch::setReferenceName(const ImportedEntityConstPtr &importedEntity)
{
    if (importedEntity->isImport()) {
        mReferenceName = importedEntity->importReference();
    }
}

void HistoryEpoch::setDestinationModel(const ImportedEntityConstPtr &importedEntity)
{
    if (importedEntity->isImport()) {
        mDestinationModel = importedEntity->importSource()->model();
    }
}

} // namespace libcellml
