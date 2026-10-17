%module(package="libcellml") entity

#define LIBCELLML_EXPORT

%include <std_string.i>

%import "types.i"

%feature("docstring") libcellml::Entity
"Abstract base class for all libCellML core classes."

%feature("docstring") libcellml::Entity::id
"Returns the `id` document identifier for this entity, or an empty string if
not set.";

%feature("docstring") libcellml::Entity::setId
"Sets the `id` document identifier for this entity.";

%feature("docstring") libcellml::Entity::removeId
"Removes the `id` document identifier set for this entity.";

%{
#include "libcellml/entity.h"
%}

%pythoncode %{
# libCellML generated wrapper code starts here.
%}

%ignore libcellml::Entity::Entity();

%include "libcellml/entity.h"
