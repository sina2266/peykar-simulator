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

namespace libcellml {

// clang-format off
#define LIBCELLML_VERSION_MAJOR @libCellML_VERSION_MAJOR@
#define LIBCELLML_VERSION_MINOR @libCellML_VERSION_MINOR@
#define LIBCELLML_VERSION_PATCH @libCellML_VERSION_PATCH@

static unsigned int LIBCELLML_LIBRARY_VERSION = @LIBCELLML_LIBRARY_VERSION@;
// clang-format on
static const std::string LIBCELLML_LIBRARY_VERSION_STRING = "@LIBCELLML_LIBRARY_VERSION_STRING@";

} // namespace libcellml
