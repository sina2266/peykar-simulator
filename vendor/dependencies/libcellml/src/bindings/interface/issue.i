%module(package="libcellml") issue

#define LIBCELLML_EXPORT

%include <std_string.i>

%import "createconstructor.i"
%import "enums.i"
%import "types.i"

%feature("docstring") libcellml::Issue
"Base class for issues used with logger derived classes."

%feature("docstring") libcellml::Issue::description
"Get a string description for why this issue was raised.";

%feature("docstring") libcellml::Issue::level
"Get the ``level`` of this issue. If no level has been set for this issue,
Level::ERROR will be returned.";

%feature("docstring") libcellml::Issue::referenceRule
"Get the :class:`ReferenceRule` of this issue.";

%feature("docstring") libcellml::Issue::item
"Get the item for this issue.";

%feature("docstring") libcellml::Issue::url
"Get the URL for more information about this issue.";

%feature("docstring") libcellml::Issue::referenceHeading
"Returns the CellML 2.0 Specification heading associated with the
:class:`ReferenceRule` for this issue (empty string if not set).";

%{
#include "libcellml/issue.h"
%}

%pythoncode %{
# libCellML generated wrapper code starts here.
%}

%include "libcellml/issue.h"
