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

#include "libcellml/importsource.h"

#include "libcellml/importedentity.h"
#include "libcellml/types.h"

#include "entity_p.h"
#include "internaltypes.h"

namespace libcellml {

using ImportedEntityWeakPtr = std::weak_ptr<ImportedEntity>;

/**
 * @brief The ImportSource::ImportSourceImpl class.
 *
 * The private implementation for the ImportSource class.
 */
class ImportSource::ImportSourceImpl: public Entity::EntityImpl
{
public:
    std::string mUrl;
    ModelWeakPtr mModel;
};

ImportSource::ImportSourceImpl *ImportSource::pFunc()
{
    return reinterpret_cast<ImportSource::ImportSourceImpl *>(Entity::pFunc());
}

const ImportSource::ImportSourceImpl *ImportSource::pFunc() const
{
    return reinterpret_cast<ImportSource::ImportSourceImpl const *>(Entity::pFunc());
}

ImportSource::ImportSource()
    : Entity(new ImportSource::ImportSourceImpl())
{
}

ImportSource::~ImportSource()
{
    delete pFunc();
}
ImportSourcePtr ImportSource::create() noexcept
{
    return std::shared_ptr<ImportSource> {new ImportSource {}};
}

std::string ImportSource::url() const
{
    return pFunc()->mUrl;
}

void ImportSource::setUrl(const std::string &url)
{
    pFunc()->mUrl = url;
}

ModelPtr ImportSource::model() const
{
    if (pFunc()->mModel.expired()) {
        return nullptr;
    }
    return pFunc()->mModel.lock();
}

void ImportSource::setModel(const ModelPtr &model)
{
    if (model == nullptr) {
        pFunc()->mModel.reset();
    } else {
        pFunc()->mModel = model;
    }
}

void ImportSource::removeModel()
{
    pFunc()->mModel.reset();
}

bool ImportSource::hasModel() const
{
    return !pFunc()->mModel.expired();
}

ImportSourcePtr ImportSource::clone() const
{
    auto i = create();

    i->setId(id());
    i->setUrl(url());
    i->setModel(model());

    return i;
}

bool ImportSource::doEquals(const EntityPtr &other) const
{
    if (Entity::doEquals(other)) {
        auto importSource = std::dynamic_pointer_cast<ImportSource>(other);
        if (importSource != nullptr) {
            return pFunc()->mUrl == importSource->url();
        }
    }
    return false;
}

} // namespace libcellml
