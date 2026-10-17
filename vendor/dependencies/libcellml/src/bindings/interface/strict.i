%module(package="libcellml") strict

#define LIBCELLML_EXPORT

%import "types.i"

%feature("docstring") libcellml::Strict
"Abstract base class for entities that can be strict.";

%feature("docstring") libcellml::Strict::isStrict
"Returns True if the entity is strict, else False.";

%feature("docstring") libcellml::Strict::setStrict
"Set the entity as strict or not strict.";

%{
#include "libcellml/types.h"
#include "libcellml/strict.h"
%}

%pythoncode %{
# libCellML generated wrapper code starts here.
%}

%ignore libcellml::Strict::Strict();

%include "libcellml/strict.h"
