%module(package="libcellml") enums

#define LIBCELLML_EXPORT

%include <std_string.i>

%feature("docstring") libcellml::cellmlElementTypeAsString
"Convert a CellmlElementType enumeration value into its string form.";

%{
#include "libcellml/enums.h"
%}

%pythoncode %{
# libCellML generated wrapper code starts here.
%}

%include "libcellml/enums.h"

%pythoncode %{
from enum import IntEnum
def redo(prefix):
    tmpD = {k:v for k,v in globals().items() if k.startswith(prefix + '_')}
    tmpD = {k[len(prefix)+1:]:v for k,v in tmpD.items()}
    globals()[prefix] = IntEnum(prefix,tmpD)
redo('CellmlElementType')
del redo  # cleaning up the namespace
del IntEnum
%}

%typemap(in) libcellml::CellmlElementType (int val, int ecode) {
    ecode = SWIG_AsVal(int)($input, &val);
    if (!SWIG_IsOK(ecode)) {
          %argument_fail(ecode, "$type", $symname, $argnum);
    } else {
        if (val < %static_cast($type::COMPONENT, int) || %static_cast($type::VARIABLE, int) < val) {
            %argument_fail(ecode, "$type is not a valid value for the enumeration.", $symname, $argnum);
        }
        $1 = %static_cast(val, $basetype);
    }
}
