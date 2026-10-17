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

#include <string>

#include <libcellml/types.h>

namespace libcellml {

/**
 * @brief Compute the SHA-1 value of the @p string @c std::string.
 *
 * Compute and return the SHA-1 value of the @p string @c std::string.
 *
 * @param string The @c std::string value for which we want the SHA-1 value.
 *
 * @return The @c std::string SHA-1 value.
 */
std::string sha1(const std::string &string);

/**
 * @brief Form the @c std::string representation of the given @p profile.
 *
 * Form the @c std::string representation of the given @p profile.
 *
 * @param generatorProfile The profile to create the @c std::string representation of.
 *
 * @return The @c std::string representation of the given @p profile.
 */
std::string generatorProfileAsString(const GeneratorProfilePtr &generatorProfile);

} // namespace libcellml
