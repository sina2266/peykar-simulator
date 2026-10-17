%module(package="libcellml") importer

#define LIBCELLML_EXPORT

%include <std_pair.i>
%include <std_string.i>

%import "createconstructor.i"
%import "logger.i"
%import "strict.i"
%import "types.i"

%feature("docstring") libcellml::Importer
"Manages all importing operations between CellML models.";

%feature("docstring") libcellml::Importer::resolveImports
"Resolves all imports in this model.

Resolves all component and units imports by loading the
models from local disk through relative URLs. The ``baseFile`` is used to
determine the full path to the source model relative to this one.";

%feature("docstring") libcellml::Importer::flattenModel
"Instantiate all imported components and units and return a self-contained model.";

%feature("docstring") libcellml::Importer::libraryCount
"Returns the number of models loaded into the importer's library."

%feature("docstring") libcellml::Importer::library
"Returns the model instance stored at the given URL in the library.
The URL string must be the absolute path to the document, including both the filename
and the baseFile path."

%feature("docstring") libcellml::Importer::addModel
"Manually add a local model instance into the library under the URL key provided.
Returns True if the model was added, and False if the URL key already exists."

%feature("docstring") libcellml::Importer::replaceModel
"Manually replace an existing library model instance with the one provided.
Returns True if the URL key is found in the library and the model is added,
and False if the URL key does not exist."

%feature("docstring") libcellml::Importer::clearImports
"Clears the links with other models from all import sources."

%feature("docstring") libcellml::Importer::key
"Returns a string corresponding to the key at which a model is stored in the
library by index, or an empty string if the index is out of range."

%feature("docstring") libcellml::Importer::removeAllModels
"Removes all models from the library.";

%feature("docstring") libcellml::Importer::addImportSource
"Manually add an ImportSource to the Importer.";

%feature("docstring") libcellml::Importer::importSourceCount
"Return the number of ImportSource items in the Importer.";

%feature("docstring") libcellml::Importer::importSource
"Retrieve an ImportSource by index.";

%feature("docstring") libcellml::Importer::removeImportSource
"Remove an ImportSource from the Importer by index or reference.";

%feature("docstring") libcellml::Importer::removeAllImportSources
"Remove all ImportSource items from the Importer.";

%feature("docstring") libcellml::Importer::hasImportSource
"Test whether this Importer already contains the given ImportSource.";

%{
#include "libcellml/importer.h"
%}

%template(StringPair) std::pair<std::string, std::string>;

%pythoncode %{
# libCellML generated wrapper code starts here.
%}

%create_constructor(Importer)
%extend libcellml::Importer {
    Importer(bool strict) {
        auto ptr = new std::shared_ptr<  libcellml::Importer >(libcellml::Importer::create(strict));
        return reinterpret_cast<libcellml::Importer *>(ptr);
    }
}

%include "libcellml/importer.h"
