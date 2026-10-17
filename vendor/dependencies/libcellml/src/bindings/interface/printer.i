%module(package="libcellml") printer

#define LIBCELLML_EXPORT

%include <std_string.i>

%import "createconstructor.i"
%import "logger.i"
%import "types.i"

%feature("docstring") libcellml::Printer
"Prints CellML models to an XML string.";

%feature("docstring") libcellml::Printer::printModel
"Serialises the given :class:`Model` to an XML string.";

%{
#include "libcellml/printer.h"
%}

%pythoncode %{
# libCellML generated wrapper code starts here.
%}

%create_constructor(Printer)

%include "libcellml/printer.h"
