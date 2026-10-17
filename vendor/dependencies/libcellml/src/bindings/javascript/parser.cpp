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

// To work around multiple inheritance we have to create a combined Parser
// and Strict class that we can bind with Emscripten.
#define JAVASCRIPT_BINDINGS
#include "libcellml/parser.h"

using namespace emscripten;

EMSCRIPTEN_BINDINGS(libcellml_parser) {

    class_<libcellml::Parser, base<libcellml::Logger>>("Parser")
        .smart_ptr_constructor("Parser", &libcellml::Parser::create)
        .function("parseModel", &libcellml::Parser::parseModel)
        .function("isStrict", &libcellml::Parser::isStrict)
        .function("setStrict", &libcellml::Parser::setStrict)
    ;
}
