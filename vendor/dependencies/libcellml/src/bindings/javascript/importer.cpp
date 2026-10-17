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

// To work around multiple inheritance we have to create a combined Importer
// and Strict class that we can bind with Emscripten.
#define JAVASCRIPT_BINDINGS
#include "libcellml/importer.h"

using namespace emscripten;

EMSCRIPTEN_BINDINGS(libcellml_importer)
{
    class_<libcellml::Importer, base<libcellml::Logger>>("Importer")
        .smart_ptr_constructor("Importer", &libcellml::Importer::create)
        .function("flattenModel", &libcellml::Importer::flattenModel)
        .function("resolveImports", &libcellml::Importer::resolveImports)
        .function("libraryCount", &libcellml::Importer::libraryCount)
        .function("libraryByKey", select_overload<libcellml::ModelPtr(const std::string &)>(&libcellml::Importer::library))
        .function("libraryByIndex", select_overload<libcellml::ModelPtr(const size_t &)>(&libcellml::Importer::library))
        .function("key", &libcellml::Importer::key)
        .function("addModel", &libcellml::Importer::addModel)
        .function("replaceModel", &libcellml::Importer::replaceModel)
        .function("clearImports", &libcellml::Importer::clearImports)
        .function("removeAllModels", &libcellml::Importer::removeAllModels)
        .function("addImportSource", &libcellml::Importer::addImportSource)
        .function("importSourceCount", &libcellml::Importer::importSourceCount)
        .function("importSource", &libcellml::Importer::importSource)
        .function("removeImportSourceByIndex", select_overload<bool(size_t)>(&libcellml::Importer::removeImportSource))
        .function("removeImportSourceByImportSource", select_overload<bool(const libcellml::ImportSourcePtr &)>(&libcellml::Importer::removeImportSource))
        .function("removeAllImportSources", &libcellml::Importer::removeAllImportSources)
        .function("hasImportSource", &libcellml::Importer::hasImportSource)
        .function("isStrict", &libcellml::Importer::isStrict)
        .function("setStrict", &libcellml::Importer::setStrict)
    ;
}
