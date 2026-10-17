%module(package="libcellml") namedentity

#define LIBCELLML_EXPORT

%import "parentedentity.i"
%import "types.i"

%feature("docstring") libcellml::NamedEntity
"Abstract base class for all nameable libCellML classes.";

%feature("docstring") libcellml::NamedEntity::name
"Returns this entity's name (empty string if not set).";

%feature("docstring") libcellml::NamedEntity::setName
"Sets this entity's name (empty string to unset).";

%feature("docstring") libcellml::NamedEntity::removeName
"Remove this entity's name.";

%{
#include "libcellml/namedentity.h"
%}

%pythoncode %{
# libCellML generated wrapper code starts here.
%}

%ignore libcellml::NamedEntity::NamedEntity();

%include "libcellml/namedentity.h"
