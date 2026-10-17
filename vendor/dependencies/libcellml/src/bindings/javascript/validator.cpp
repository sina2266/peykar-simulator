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

#include "libcellml/validator.h"

using namespace emscripten;

EMSCRIPTEN_BINDINGS(libcellml_validator) {

    class_<libcellml::Validator, base<libcellml::Logger>>("Validator")
        .smart_ptr_constructor("Validator", &libcellml::Validator::create)
        .function("validateModel", &libcellml::Validator::validateModel)
    ;
}
