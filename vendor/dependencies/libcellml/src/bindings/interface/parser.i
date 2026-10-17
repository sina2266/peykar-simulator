%module(package="libcellml") parser

#define LIBCELLML_EXPORT

%include <std_string.i>

%import "createconstructor.i"
%import "logger.i"
%import "strict.i"
%import "types.i"

%feature("docstring") libcellml::Parser
"Can parse Models encoded as XML strings.";

%feature("docstring") libcellml::Parser::parseModel
"Parses a string and returns a :class:`Model`.";

%{
#include "libcellml/parser.h"
%}

%pythoncode %{
# libCellML generated wrapper code starts here.
%}

%create_constructor(Parser)
%extend libcellml::Parser {
    Parser(bool strict) {
        auto ptr = new std::shared_ptr<  libcellml::Parser >(libcellml::Parser::create(strict));
        return reinterpret_cast<libcellml::Parser *>(ptr);
    }
}

%include "libcellml/parser.h"
