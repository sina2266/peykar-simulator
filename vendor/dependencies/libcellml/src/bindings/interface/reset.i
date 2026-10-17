%module(package="libcellml") reset

#define LIBCELLML_EXPORT

%import "createconstructor.i"
%import "parentedentity.i"
%import "types.i"

%feature("docstring") libcellml::Reset
"The Reset class is for describing a CellML reset.";

%feature("docstring") libcellml::Reset::order
"Returns the order for this reset as an integer value.";

%feature("docstring") libcellml::Reset::setOrder
"Set the order for this reset.  The order value must be an integer.";

%feature("docstring") libcellml::Reset::removeOrder
"Sets the state of the order to unset.";

%feature("docstring") libcellml::Reset::isOrderSet
"Returns True if the order is set, False otherwise.";

%feature("docstring") libcellml::Reset::setVariable
"Sets the :class: variable for this reset.";

%feature("docstring") libcellml::Reset::variable
"Returns the :class: variable of this reset.";

%feature("docstring") libcellml::Reset::setTestVariable
"Sets the :class: test_variable for this reset.";

%feature("docstring") libcellml::Reset::testVariable
"Returns the :class: test_variable of this reset.";

%feature("docstring") libcellml::Reset::testValue
"Returns the :class: test_value MathML string of this reset.";

%feature("docstring") libcellml::Reset::resetValue
"Returns the :class: reset_value MathML string of this reset.";

%feature("docstring") libcellml::Reset::setTestValue
"Sets the :class: test_value MathML string of this reset.";

%feature("docstring") libcellml::Reset::removeTestValue
"Clears the :class: test_value MathML string of this reset.";

%feature("docstring") libcellml::Reset::setResetValue
"Sets the :class: reset_value MathML string of this reset.";

%feature("docstring") libcellml::Reset::removeResetValue
"Clears the :class: reset_value MathML string of this reset.";

%feature("docstring") libcellml::Reset::appendTestValue
"Appends to the :class: test_value MathML string of this reset.";

%feature("docstring") libcellml::Reset::appendResetValue
"Appends to the :class: reset_value MathML string of this reset.";

%feature("docstring") libcellml::Reset::setTestValueId
"Sets the :class: test_value identifier string of this reset.";

%feature("docstring") libcellml::Reset::removeTestValueId
"Clears the :class: test_value identifier string of this reset.";

%feature("docstring") libcellml::Reset::setResetValueId
"Sets the :class: reset_value identifier string of this reset.";

%feature("docstring") libcellml::Reset::removeResetValueId
"Clears the :class: reset_value identifier string of this reset.";

%feature("docstring") libcellml::Reset::testValueId
"Returns the :class: test_value identifier string of this reset.";

%feature("docstring") libcellml::Reset::resetValueId
"Returns the :class: reset_value identifier string of this reset.";

%feature("docstring") libcellml::Reset::clone
"Create a copy of this reset.";

#if defined(SWIGPYTHON)
    // Allow any type of input to be converted to bool
    %typemap(typecheck,precedence=SWIG_TYPECHECK_BOOL) bool { $1 = 1; }
    %typemap(in) bool { $1 = PyObject_IsTrue($input) == 1; }
#endif

%{
#include "libcellml/reset.h"
%}

%pythoncode %{
# libCellML generated wrapper code starts here.
%}

%create_constructor(Reset)

%include "libcellml/reset.h"
