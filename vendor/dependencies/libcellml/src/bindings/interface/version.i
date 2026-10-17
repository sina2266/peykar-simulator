%module(package="libcellml") version

#define LIBCELLML_EXPORT

%include <std_string.i>

%feature("docstring") libcellml::version
"Returns the version of the library as a number.";

%feature("docstring") libcellml::versionString
"Returns the libcellml version string.

The version string is in the format x.y.z, where the '.'s are literal, and x, y
and z represent counting numbers, in which case x is the major version, y the
minor version, and z the patch level.";

%{
#include "libcellml/version.h"
%}

%pythoncode %{
# libCellML generated wrapper code starts here.
%}

%include "libcellml/version.h"
