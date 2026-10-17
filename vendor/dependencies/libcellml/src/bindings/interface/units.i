%module(package="libcellml") units

#define LIBCELLML_EXPORT

%import "createconstructor.i"
%import "importedentity.i"
%import "namedentity.i"
%import "types.i"

// Add typemaps to fix string ref inputs (fixes memory leak issues)
%include "typemaps.i"
%apply std::string &OUTPUT { std::string &reference };
%apply std::string &OUTPUT { std::string &prefix };
%apply double &OUTPUT { double &exponent };
%apply double &OUTPUT { double &multiplier };
%apply std::string &OUTPUT { std::string &id };
// Add typemaps for reference arguments used to return attributes
%apply std::string &INPUT { const std::string &reference };
%apply std::string &INPUT { const std::string &prefix };
%apply std::string &INPUT { const std::string &id };

// Remove methods which cause conflicts for languages with bad enum support
// (e.g. Python)
%ignore libcellml::Units::unitAttributes(StandardUnit standardRef,
 std::string &prefix, double &exponent, double &multiplier, std::string &id) const;
%ignore libcellml::Units::removeUnit(StandardUnit standardRef);
// This one causes confusion: addUnit(1, 1.0) --> (StandardUnit, double exp)
// but: addUnit(1, 1) --> (StandardUnit, Prefix, default=1, default=1)
%ignore libcellml::Units::addUnit(StandardUnit standardRef, double exponent);

// Removing the overload of enumeration and integers for the Prefix argument
%ignore libcellml::Units::addUnit(StandardUnit standardRef, Prefix prefix, double exponent, double multiplier, const std::string &id);
%ignore libcellml::Units::addUnit(StandardUnit standardRef, Prefix prefix, double exponent=1.0, double multiplier=1.0, const std::string &id="");
%ignore libcellml::Units::addUnit(const std::string &reference, Prefix prefix, double exponent, double multiplier, const std::string &id);
%ignore libcellml::Units::addUnit(const std::string &reference, Prefix prefix, double exponent=1.0, double multiplier=1.0, const std::string &id="");

%feature("docstring") libcellml::Units
"Represents a CellML Units definition.";

%feature("docstring") libcellml::Units::addUnit
"Lets users add a Unit to this definition.

Possible signatures:

 - addUnit(reference, prefix, exponent=1, multiplier=1)
 - addUnit(reference, exponent)
 - addUnit(reference)

where `reference` can be a string or a StandardUnits. And `prefix` can be a
string or an integer.";

%feature("docstring") libcellml::Units::removeUnit
"Removes the unit specified by index, reference, or StandardUnit.

Only the first matching unit is removed.

`True` is returned on success.";

%feature("docstring") libcellml::Units::removeAllUnits
"Remove all units stored in this Units object. ";

%feature("docstring") libcellml::Units::isBaseUnit
"Tests if this is a base unit.";

%feature("docstring") libcellml::Units::unitAttributes
"Returns the attributes of the unit specified by index, reference, or
StandardUnit.";

%feature("docstring") libcellml::Units::requiresImports
"Returns `True` when this Units object depends on an imported child
Units object, or `False` otherwise.";

%feature("docstring") libcellml::Units::unitCount
"Returns the number of units contained by this units object.";

%feature("docstring") libcellml::Units::setSourceUnits
"Makes this Units an imported units by defining an `ImportSource` from which to
extract the units with the given `name`.";

%feature("docstring") libcellml::Units::scalingFactor
"Returns the scaling factor between two Units objects.";

%feature("docstring") libcellml::Units::compatible
"Tests if two Units have the same dimensional basis.";

%feature("docstring") libcellml::Units::equivalent
"Tests if two Units are equivalent in base units as well as scaling factor.";

%feature("docstring") libcellml::Units::isDefined
"Tests if this units is defined.";

%feature("docstring") libcellml::Units::clone
"Create a copy of this units.";

%feature("docstring") libcellml::Units::unitId
"Returns the identifier string of the Unit item at the given index.";

%feature("docstring") libcellml::Units::setUnitId
"Sets the identifier string of the Unit item at the given index.";

%feature("docstring") libcellml::Units::unitAttributeReference
"Get the reference from the unit attribute.";

%feature("docstring") libcellml::Units::setUnitAttributeReference
"Set the reference unit attribute.";

%feature("docstring") libcellml::Units::unitAttributePrefix
"Get the prefix from the unit attribute.";

%feature("docstring") libcellml::Units::unitAttributeExponent
"Get the exponent from the unit attribute.";

%feature("docstring") libcellml::Units::unitAttributeMultiplier
"Get the multiplier from the unit attribute.";

#if defined(SWIGPYTHON)
    // Treat negative size_t as invalid index (instead of unknown method)
    %extend libcellml::Units {
        bool removeUnit(long index) {
            if (index < 0) return false;
            return $self->removeUnit((size_t)index);
        }
    }
#endif

%{
#include "libcellml/units.h"
%}

%pythoncode %{
# libCellML generated wrapper code starts here.
%}

// The name 'reference' has been typemapped above, which causes an issue for
// the removeUnit(reference) method. This is solved below by creating a new
// method removeUnit(ref) and telling swig not to use removeUnit(reference).
%extend libcellml::Units {
    bool removeUnit(const std::string &ref) {
        return $self->removeUnit(ref);
    }
}
%ignore libcellml::Units::removeUnit(const std::string &reference);

%create_constructor(Units)
%create_name_constructor(Units)

%include "libcellml/units.h"
