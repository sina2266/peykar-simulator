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

#include <emscripten/bind.h>

#include "libcellml/componententity.h"

using namespace emscripten;

EMSCRIPTEN_BINDINGS(libcellml_componententity) {

    class_<libcellml::ComponentEntity, base<libcellml::NamedEntity>>("ComponentEntity")
        .function("addComponent", &libcellml::ComponentEntity::addComponent)
        .function("removeComponentByIndex", select_overload<bool(size_t)>(&libcellml::ComponentEntity::removeComponent))
        .function("removeComponentByName", select_overload<bool(const std::string &, bool)>(&libcellml::ComponentEntity::removeComponent))
        .function("removeComponentByComponent", select_overload<bool(const libcellml::ComponentPtr &, bool)>(&libcellml::ComponentEntity::removeComponent))
        .function("removeAllComponents", &libcellml::ComponentEntity::removeAllComponents)
        .function("containsComponentByName", select_overload<bool(const std::string &, bool) const>(&libcellml::ComponentEntity::containsComponent))
        .function("containsComponentByComponent", select_overload<bool(const libcellml::ComponentPtr &, bool) const>(&libcellml::ComponentEntity::containsComponent))
        .function("componentByIndex", select_overload<libcellml::ComponentPtr(size_t) const>(&libcellml::ComponentEntity::component))
        .function("componentByName", select_overload<libcellml::ComponentPtr(const std::string &, bool) const>(&libcellml::ComponentEntity::component))
        .function("takeComponentByIndex", select_overload<libcellml::ComponentPtr(size_t)>(&libcellml::ComponentEntity::takeComponent))
        .function("takeComponentByName", select_overload<libcellml::ComponentPtr(const std::string &, bool)>(&libcellml::ComponentEntity::takeComponent))
        .function("replaceComponentByIndex", select_overload<bool(size_t, const libcellml::ComponentPtr &)>(&libcellml::ComponentEntity::replaceComponent))
        .function("replaceComponentByName", select_overload<bool(const std::string &, const libcellml::ComponentPtr &, bool)>(&libcellml::ComponentEntity::replaceComponent))
        .function("replaceComponentByOldComponent", select_overload<bool(const libcellml::ComponentPtr &, const libcellml::ComponentPtr &, bool)>(&libcellml::ComponentEntity::replaceComponent))
        .function("componentCount", &libcellml::ComponentEntity::componentCount)
        .function("setEncapsulationId", &libcellml::ComponentEntity::setEncapsulationId)
        .function("encapsulationId", &libcellml::ComponentEntity::encapsulationId)
        .function("removeEncapsulationId", &libcellml::ComponentEntity::removeEncapsulationId)
    ;
}
