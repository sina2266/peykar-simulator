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

#include "internaltypes.h"

#ifndef TEST_UTILS
#    define TEST_EXPORT

namespace libcellml {
#endif

/**
 * @brief Get the @c Model that the entity is owned by.
 *
 * Travel up the entities hierarchy to find the owning model. If
 * the entity doesn't have an owning model return the @c nullptr.
 *
 * @param entity The entity to get the owning model for.
 * @return The owning @c Model or the @c nullptr if no model owns this entity.
 */
libcellml::ModelPtr TEST_EXPORT owningModel(const libcellml::ParentedEntityConstPtr &entity);

/**
 * @brief Get the @c Component that the variable is owned by.
 *
 * Return the owning component for this variable.
 *
 * @param entity The entity to get the owning component for.
 * @return The owning @c Component of this entity.
 */
libcellml::ComponentPtr TEST_EXPORT owningComponent(const libcellml::ParentedEntityConstPtr &entity);

#ifndef TEST_UTILS
} // namespace libcellml

#    undef TEST_EXPORT
#endif
