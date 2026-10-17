%module(package="libcellml") validator

#define LIBCELLML_EXPORT

%import "createconstructor.i"
%import "logger.i"
%import "types.i"

%feature("docstring") libcellml::Validator
"Validates CellML objects.";

%feature("docstring") libcellml::Validator::validateModel
"Validate the given `model` and its encapsulated entities using the CellML 2.0
Specification. Any errors will be logged in the `Validator`.";

%{
#include "libcellml/validator.h"
%}

%pythoncode %{
# libCellML generated wrapper code starts here.
%}

%create_constructor(Validator)

%include "libcellml/types.h"
%include "libcellml/validator.h"
