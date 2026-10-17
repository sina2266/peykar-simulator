%module(package="libcellml") component

#define LIBCELLML_EXPORT

%include <std_shared_ptr.i>

%import "componententity.i"
%import "createconstructor.i"
%import "types.i"

%feature("docstring") libcellml::Component
"Represents a CellML component.";

%feature("docstring") libcellml::Component::setSourceComponent
"Marks this component as an imported component by defining an
`importSource` specifying the source of the import and the `name` of the
component in the `importSource`.";

%feature("docstring") libcellml::Component::appendMath
"Appends `math` to the existing math string for this component.";

%feature("docstring") libcellml::Component::math
"Returns a math string if one has been created for this component (empty string
if not).";

%feature("docstring") libcellml::Component::setMath
"Sets the math string for this component.
If `math` is an empty string, math will be removed from the component.";

%feature("docstring") libcellml::Component::removeMath
"Clears the math from this component.";

%feature("docstring") libcellml::Component::addVariable
"Adds variable `variable` to this component.";

%feature("docstring") libcellml::Component::variable
"Returns a Variable from this component, specified by name or index.

Only the first matching variable is returned.";

%feature("docstring") libcellml::Component::hasVariable
"Tests if this component contains a given variable, specified by name or as
`Variable` object.";

%feature("docstring") libcellml::Component::removeVariable
"Removes a variable from this component, specified by name, index, or
`Variable` object.

Only the first matching variable is removed.

If the variable to be removed is in a connection (is equivalent to another
variable), this component will not be serialised in the connection.

Returns `True` on success.";

%feature("docstring") libcellml::Component::takeVariable
"Removes a variable and returns it from this component, specified by name, or
index.

Returns the `Variable` on success.";

%feature("docstring") libcellml::Component::removeAllVariables
"Clears all variables that have been added to this component.

If any of the variables to be removed are in connections (are equivalent to
other variables), this component will not be serialised in the connection.";

%feature("docstring") libcellml::Component::variableCount
"Returns the number of variables the component directly contains.";

%feature("docstring") libcellml::Component::addReset
"Add a reset `reset` to this component.";

%feature("docstring") libcellml::Component::takeReset
"Removes a reset and returns it from this component, specified by index.

Returns the `Reset` on success.";

%feature("docstring") libcellml::Component::removeReset
"Remove the reset at the given index from this component.
If the index is not valid @c false is returned, the valid
range for the index is [0, #resets).";

%feature("docstring") libcellml::Component::removeAllResets
"Clears all resets that have been added to this component.";

%feature("docstring") libcellml::Component::reset
"Returns a reference to a reset at the index @p index for this
component. If the index is not valid a @c nullptr is returned, the valid
range for the index is [0, #resets).";

%feature("docstring") libcellml::Component::resetCount
"Returns the number of resets the component contains.";

%feature("docstring") libcellml::Component::hasReset
"Tests whether the argument :param: reset exists in the set of this component's
resets. Returns True if the :param: reset is in this component's
resets and False otherwise.";

%feature("docstring") libcellml::Component::isDefined
"Tests if this component is defined.";

%feature("docstring") libcellml::Component::clone
"Create a copy of this component.";

%feature("docstring") libcellml::Component::requiresImports
"Determines whether this component relies on any imports.  If this component
or any of its encapsulated components are imported, returns @c true,
otherwise @c false.";

#if defined(SWIGPYTHON)
    // Treat negative size_t as invalid index (instead of unknown method)
    %extend libcellml::Component {
        VariablePtr variable(long index) const {
            if (index < 0) return nullptr;
            return $self->variable(size_t(index));
        }
        bool removeVariable(long index) {
            if (index < 0) return false;
            return $self->removeVariable(size_t(index));
        }
    }
#endif

%{
#include "libcellml/component.h"
%}

%pythoncode %{
# libCellML generated wrapper code starts here.
%}

%create_constructor(Component)
%create_name_constructor(Component)

%include "libcellml/component.h"
