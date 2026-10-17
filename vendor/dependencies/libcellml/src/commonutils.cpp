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

#include "commonutils.h"

#include "libcellml/component.h"
#include "libcellml/model.h"

#ifndef TEST_UTILS
namespace libcellml {
#endif

libcellml::ModelPtr owningModel(const libcellml::ParentedEntityConstPtr &entity)
{
    auto model = std::dynamic_pointer_cast<libcellml::Model>(entity->parent());
    auto component = owningComponent(entity);
    while ((model == nullptr) && (component != nullptr)) {
        model = std::dynamic_pointer_cast<libcellml::Model>(component->parent());
        component = owningComponent(component);
    }

    return model;
}

libcellml::ComponentPtr owningComponent(const libcellml::ParentedEntityConstPtr &entity)
{
    return std::dynamic_pointer_cast<libcellml::Component>(entity->parent());
}

#ifndef TEST_UTILS
} // namespace libcellml
#endif
