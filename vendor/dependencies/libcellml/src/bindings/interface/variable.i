%module(package="libcellml") variable

#define LIBCELLML_EXPORT

%import "createconstructor.i"
%import "namedentity.i"
%import "types.i"

%feature("docstring") libcellml::Variable
"Represents a CellML Variable entity";

%feature("docstring") libcellml::Variable::addEquivalence
"Adds an equivalence relation between this variable and the given one (both
objects are updated)";

%feature("docstring") libcellml::Variable::equivalentVariable
"Returns the equivalent variable at the given index.";

%feature("docstring") libcellml::Variable::hasEquivalentVariable
"Tests if the given variable is in this variable's set of equivalent
variables.";

%feature("docstring") libcellml::Variable::removeEquivalence
"Removes an equivalence between this variable and the given one (both objects
are updated).

Returns `True` on success.";

%feature("docstring") libcellml::Variable::removeAllEquivalences
"Removes all equivalent variables for this variable (all relevant objects are
updated).";

%feature("docstring") libcellml::Variable::equivalentVariableCount
"Returns the number of equivalent variables for this variable.";

%feature("docstring") libcellml::Variable::units
"Returns the name of the units set for this variable (empty string if none)";

%feature("docstring") libcellml::Variable::setUnits
"Sets the units for this variable to the given string (name) or Units object.";

%feature("docstring") libcellml::Variable::removeUnits
"Clears the units for this variable.";

%feature("docstring") libcellml::Variable::initialValue
"Returns the string corresponding to the initial value for this variable.";

%feature("docstring") libcellml::Variable::setInitialValue
"Sets this variable's initial value, given as a string, number, or variable
reference.";

%feature("docstring") libcellml::Variable::interfaceType
"Returns this variable's interface type as a string.";

%feature("docstring") libcellml::Variable::hasInterfaceType
"Tests if this variable has the given interface type.";

%feature("docstring") libcellml::Variable::permitsInterfaceType
"Tests if this variable will accept access through the given interface type.";

%feature("docstring") libcellml::Variable::removeInterfaceType
"Clears the interface type for this variable.";

%feature("docstring") libcellml::Variable::removeInitialValue
"Clears the initial value for this variable.";

%feature("docstring") libcellml::Variable::setInterfaceType
"Sets this variable's interfacetype to the given type specified as string or
InterfaceType.";

%feature("docstring") libcellml::Variable::setEquivalenceMappingId
"Record the given identifier as the mapping identifier for the equivalence defined with the given
variables.  The variables are commutative. This identifier appears in the 'map_variables'
element of the model when serialised.

To clear an equivalence mapping identifier set it to the empty string. If the two variables are
not equivalent the mapping identifier is not set.";

%feature("docstring") libcellml::Variable::setEquivalenceConnectionId
"Record the given identifier as the connection identifier for the equivalence defined with the given
variables.  The variables are commutative. This identifier appears in the 'connection'
element of the model when serialised.

To clear an equivalence connection identifier set it to the empty string. If the two variables are
not equivalent the connection identifier is not set.";

%feature("docstring") libcellml::Variable::equivalenceMappingId
"Get the mapping identifier set for the equivalence defined with the given variables.";

%feature("docstring") libcellml::Variable::equivalenceConnectionId
"Get the connection identifier set for the equivalence defined with the given variables.";

%feature("docstring") libcellml::Variable::removeEquivalenceConnectionId
"Remove the connection identifier for the equivalence defined with the given variables.";

%feature("docstring") libcellml::Variable::removeEquivalenceMappingId
"Remove the mapping identifier for the equivalence defined with the given variables.";

%feature("docstring") libcellml::Variable::clone
"Create a copy of this variable.";

#if defined(SWIGPYTHON)
    // Treat negative size_t as invalid index (instead of unknown method)
    %extend libcellml::Variable {
        VariablePtr equivalentVariable(long index) {
            if (index < 0) return nullptr;
            return $self->equivalentVariable(size_t(index));
        }
    }
#endif

%{
#include "libcellml/variable.h"
%}

%pythoncode %{
# libCellML generated wrapper code starts here.
%}

%create_constructor(Variable)
%create_name_constructor(Variable)

%include "libcellml/types.h"
%include "libcellml/variable.h"
