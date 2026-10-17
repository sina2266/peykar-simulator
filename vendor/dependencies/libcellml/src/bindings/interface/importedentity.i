%module(package="libcellml") importedentity

#define LIBCELLML_EXPORT

%import "types.i"

%feature("docstring") libcellml::ImportedEntity
"Abstract base class for entities that can be imported.";

%feature("docstring") libcellml::ImportedEntity::importSource
"Returns an ImportSource if this entity is imported, else `None`.";

%feature("docstring") libcellml::ImportedEntity::setImportSource
"Set the ImportSource for this entity (use `None` to unset).";

%feature("docstring") libcellml::ImportedEntity::importReference
"Returns the reference to the entity in the imported model, or an empty string
if not set.";

%feature("docstring") libcellml::ImportedEntity::setImportReference
"Set the import reference to an entity in the imported model (use an empty
string to unset).";

%feature("docstring") libcellml::ImportedEntity::isImport
"Test if this entity is imported.";

%{
#include "libcellml/importedentity.h"
%}

%pythoncode %{
# libCellML generated wrapper code starts here.
%}

%ignore libcellml::ImportedEntity::ImportedEntity();

%include "libcellml/importedentity.h"
