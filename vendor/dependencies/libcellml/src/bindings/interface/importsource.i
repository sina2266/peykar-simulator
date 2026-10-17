%module(package="libcellml") importsource

#define LIBCELLML_EXPORT

%import "createconstructor.i"
%import "entity.i"
%import "types.i"

%feature("docstring") libcellml::ImportSource
"The `ImportSource` class is for defining a location at which a CellML model is
expected to be obtainable (the 'source'). The `ImportSource` also acts as a
handle to indicate that imported entities originate from the same instance of
that imported source.";

%feature("docstring") libcellml::ImportSource::url
"Returns the URL of the source model (empty string if not set).";

%feature("docstring") libcellml::ImportSource::setUrl
"Sets the source model URL that this import source refers to (empty string to
unset).";

%feature("docstring") libcellml::ImportSource::model
"Returns the Model that has been assigned to resolve this ImportSource.";

%feature("docstring") libcellml::ImportSource::setModel
"Sets the Model to resolve this ImportSource.";

%feature("docstring") libcellml::ImportSource::hasModel
"Returns True if this ImportSource has been resolved, False otherwise.";

%feature("docstring") libcellml::ImportSource::removeModel
"Remove the model from this ImportSource.";

%feature("docstring") libcellml::ImportSource::clone
"Create a copy of this import source.";

%feature("docstring") libcellml::ImportSource::addComponent
"Add a Component to this ImportSource.";

%feature("docstring") libcellml::ImportSource::removeComponent
"Remove a Component from this ImportSource's dependency list.";

%feature("docstring") libcellml::ImportSource::componentCount
"Return the number of imported Components from this ImportSource.";

%feature("docstring") libcellml::ImportSource::removeAllComponents
"Remove all of the imported Components from this ImportSource.";

%feature("docstring") libcellml::ImportSource::component
"Retrieve the imported Component at the given index.";

%feature("docstring") libcellml::ImportSource::addUnits
"Add a Units item to this ImportSource.";

%feature("docstring") libcellml::ImportSource::removeUnits
"Remove a Units item from this ImportSource's dependency list.";

%feature("docstring") libcellml::ImportSource::removeAllUnits
"Remove all of the imported Units from this ImportSource.";

%feature("docstring") libcellml::ImportSource::unitsCount
"Return the number of Units items imported from this ImportSource.";

%feature("docstring") libcellml::ImportSource::units
"Retrieve the imported Units item at the given index.";

%{
#include "libcellml/importsource.h"
%}

%pythoncode %{
# libCellML generated wrapper code starts here.
%}

%create_constructor(ImportSource)

%include "libcellml/types.h"
%include "libcellml/importsource.h"
