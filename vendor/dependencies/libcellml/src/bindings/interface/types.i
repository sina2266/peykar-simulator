/*
Provides support for shared pointers declared in types.h.
*/
%module(package="libcellml") types

#define LIBCELLML_EXPORT

%include <std_multimap.i>
%include <std_pair.i>
%include <std_shared_ptr.i>
%include <std_string.i>
%include <stdint.i>

%include "enums.i"

%shared_ptr(libcellml::Analyser)
%shared_ptr(libcellml::AnalyserEquation)
%shared_ptr(libcellml::AnalyserEquationAst)
%shared_ptr(libcellml::AnalyserExternalVariable)
%shared_ptr(libcellml::AnalyserModel)
%shared_ptr(libcellml::AnalyserVariable)
%shared_ptr(libcellml::Annotator)
%shared_ptr(libcellml::AnyCellmlElement)
%shared_ptr(libcellml::Component)
%shared_ptr(libcellml::ComponentEntity)
%shared_ptr(libcellml::Entity)
%shared_ptr(libcellml::Generator)
%shared_ptr(libcellml::GeneratorProfile)
%shared_ptr(libcellml::GeneratorVariableTracker)
%shared_ptr(libcellml::Importer)
%shared_ptr(libcellml::ImportSource)
%shared_ptr(libcellml::ImportedEntity)
%shared_ptr(libcellml::Issue)
%shared_ptr(libcellml::Logger)
%shared_ptr(libcellml::Model)
%shared_ptr(libcellml::NamedEntity)
%shared_ptr(libcellml::ParentedEntity)
%shared_ptr(libcellml::Parser)
%shared_ptr(libcellml::Printer)
%shared_ptr(libcellml::Reset)
%shared_ptr(libcellml::Strict)
%shared_ptr(libcellml::UnitsItem)
%shared_ptr(libcellml::Units)
%shared_ptr(libcellml::Validator)
%shared_ptr(libcellml::Variable)
%shared_ptr(libcellml::VariablePair)

%feature("docstring") libcellml::VariablePair
"A class for describing a variable pair.";

%feature("docstring") libcellml::VariablePair::variable1
"Return the first variable in the pair of variables.";

%feature("docstring") libcellml::VariablePair::variable2
"Return the second variable in the pair of variables.";

%feature("docstring") libcellml::VariablePair::isValid
"Test if the pair is valid.";

%feature("docstring") libcellml::UnitsItem
"A class for describing a units item.";

%feature("docstring") libcellml::UnitsItem::units
"Return the units for the units item reference.";

%feature("docstring") libcellml::UnitsItem::index
"Return the index for the units item reference.";

%feature("docstring") libcellml::UnitsItem::isValid
"Test if the units item reference is valid.";

%feature("docstring") libcellml::AnyCellmlElement
"A class for storing any kind of item.";

%feature("docstring") libcellml::AnyCellmlElement::type
"Get the type of the stored item.";

%feature("docstring") libcellml::AnyCellmlElement::component
"Return the item as a component.";

%feature("docstring") libcellml::AnyCellmlElement::importSource
"Return the item as an import source.";

%feature("docstring") libcellml::AnyCellmlElement::model
"Return the item as a model.";

%feature("docstring") libcellml::AnyCellmlElement::reset
"Return the item as a reset.";

%feature("docstring") libcellml::AnyCellmlElement::units
"Return the item as a units.";

%feature("docstring") libcellml::AnyCellmlElement::unitsItem
"Return the item as a units item.";

%feature("docstring") libcellml::AnyCellmlElement::variable
"Return the item as a variable.";

%feature("docstring") libcellml::AnyCellmlElement::variablePair
"Return the item as a variablePair.";

%{
#include "libcellml/types.h"
#include "libcellml/component.h"
#include "libcellml/importsource.h"
#include "libcellml/model.h"
#include "libcellml/reset.h"
#include "libcellml/units.h"
#include "libcellml/variable.h"
%}

%pythoncode %{
# libCellML generated wrapper code starts here.
%}

%template() std::multimap< std::string, libcellml::CellmlElementType>;

// Shared typemaps

%typemap(in) libcellml::AnalyserEquation::Type (int val, int ecode) {
  ecode = SWIG_AsVal(int)($input, &val);
  if (!SWIG_IsOK(ecode)) {
    %argument_fail(ecode, "$type", $symname, $argnum);
  } else {
    if (val < %static_cast($type::CONSTANT, int) || %static_cast($type::EXTERNAL, int) < val) {
      %argument_fail(ecode, "$type is not a valid value for the enumeration.", $symname, $argnum);
    }
    $1 = %static_cast(val, $basetype);
  }
}

%typemap(in) libcellml::AnalyserEquationAst::Type (int val, int ecode) {
  ecode = SWIG_AsVal(int)($input, &val);
  if (!SWIG_IsOK(ecode)) {
    %argument_fail(ecode, "$type", $symname, $argnum);
  } else {
    if (val < %static_cast($type::EQUALITY, int) || %static_cast($type::NAN, int) < val) {
      %argument_fail(ecode, "$type is not a valid value for the enumeration.", $symname, $argnum);
    }
    $1 = %static_cast(val, $basetype);
  }
}

%typemap(in) libcellml::AnalyserModel::Type (int val, int ecode) {
  ecode = SWIG_AsVal(int)($input, &val);
  if (!SWIG_IsOK(ecode)) {
    %argument_fail(ecode, "$type", $symname, $argnum);
  } else {
    if (val < %static_cast($type::UNKNOWN, int) || %static_cast($type::UNSUITABLY_CONSTRAINED, int) < val) {
      %argument_fail(ecode, "$type is not a valid value for the enumeration.", $symname, $argnum);
    }
    $1 = %static_cast(val, $basetype);
  }
}

%typemap(in) libcellml::AnalyserVariable::Type (int val, int ecode) {
  ecode = SWIG_AsVal(int)($input, &val);
  if (!SWIG_IsOK(ecode)) {
    %argument_fail(ecode, "$type", $symname, $argnum);
  } else {
    if (val < %static_cast($type::VARIABLE_OF_INTEGRATION, int) || %static_cast($type::EXTERNAL_VARIABLE, int) < val) {
      %argument_fail(ecode, "$type is not a valid value for the enumeration.", $symname, $argnum);
    }
    $1 = %static_cast(val, $basetype);
  }
}

%typemap(in) libcellml::GeneratorProfile::Profile (int val, int ecode) {
  ecode = SWIG_AsVal(int)($input, &val);
  if (!SWIG_IsOK(ecode)) {
    %argument_fail(ecode, "$type", $symname, $argnum);
  } else {
    if (val < %static_cast($type::C, int) || %static_cast($type::PYTHON, int) < val) {
      %argument_fail(ecode, "$type is not a valid value for the enumeration.", $symname, $argnum);
    }
    $1 = %static_cast(val, $basetype);
  }
}

%typemap(in) libcellml::Issue::Level (int val, int ecode) {
  ecode = SWIG_AsVal(int)($input, &val);
  if (!SWIG_IsOK(ecode)) {
    %argument_fail(ecode, "$type", $symname, $argnum);
  } else {
    if (val < %static_cast($type::ERROR, int) || %static_cast($type::MESSAGE, int) < val) {
      %argument_fail(ecode, "$type is not a valid value for the enumeration.", $symname, $argnum);
    }
    $1 = %static_cast(val, $basetype);
  }
}

%typemap(in) libcellml::Issue::ReferenceRule (int val, int ecode) {
  ecode = SWIG_AsVal(int)($input, &val);
  if (!SWIG_IsOK(ecode)) {
    %argument_fail(ecode, "$type", $symname, $argnum);
  } else {
    if (val < %static_cast($type::UNDEFINED, int) || %static_cast($type::MAP_VARIABLES_IDENTICAL_UNIT_REDUCTION, int) < val) {
      %argument_fail(ecode, "$type is not a valid value for the enumeration.", $symname, $argnum);
    }
    $1 = %static_cast(val, $basetype);
  }
}

%typemap(in) libcellml::Units::Prefix (int val, int ecode) {
  ecode = SWIG_AsVal(int)($input, &val);
  if (!SWIG_IsOK(ecode)) {
    %argument_fail(ecode, "$type", $symname, $argnum);
  } else {
    if (val < %static_cast($type::YOTTA, int) || %static_cast($type::YOCTO, int) < val) {
      %argument_fail(ecode, "$type is not a valid value for the enumeration.", $symname, $argnum);
    }
    $1 = %static_cast(val, $basetype);
  }
}

%typemap(in) libcellml::Units::StandardUnit (int val, int ecode) {
  ecode = SWIG_AsVal(int)($input, &val);
  if (!SWIG_IsOK(ecode)) {
    %argument_fail(ecode, "$type", $symname, $argnum);
  } else {
    if (val < %static_cast($type::AMPERE, int) || %static_cast($type::WEBER, int) < val) {
      %argument_fail(ecode, "$type is not a valid value for the enumeration.", $symname, $argnum);
    }
    $1 = %static_cast(val, $basetype);
  }
}

%typemap(in) libcellml::Variable::InterfaceType (int val, int ecode) {
  ecode = SWIG_AsVal(int)($input, &val);
  if (!SWIG_IsOK(ecode)) {
    %argument_fail(ecode, "$type", $symname, $argnum);
  } else {
    if (val < %static_cast($type::NONE, int) || %static_cast($type::PUBLIC_AND_PRIVATE, int) < val) {
      %argument_fail(ecode, "$type is not a valid value for the enumeration.", $symname, $argnum);
    }
    $1 = %static_cast(val, $basetype);
  }
}

%typemap(out) libcellml::UnitsItem *UnitsItem() {
  /*
  Here we take the returned value from the Constructor for this object and cast it
  to the pointer that it actually is.  Once that is done we can set the required resultobj.
  */
  std::shared_ptr<  libcellml::UnitsItem > *smartresult = reinterpret_cast<std::shared_ptr<  libcellml::UnitsItem > *>(result);
  resultobj = SWIG_NewPointerObj(SWIG_as_voidptr(smartresult), SWIGTYPE_p_std__shared_ptrT_libcellml__UnitsItem_t, SWIG_POINTER_NEW | SWIG_POINTER_OWN);
}

%typemap(out) libcellml::VariablePair *VariablePair() {
  /*
  Here we take the returned value from the Constructor for this object and cast it
  to the pointer that it actually is.  Once that is done we can set the required resultobj.
  */
  std::shared_ptr<  libcellml::VariablePair > *smartresult = reinterpret_cast<std::shared_ptr<  libcellml::VariablePair > *>(result);
  resultobj = SWIG_NewPointerObj(SWIG_as_voidptr(smartresult), SWIGTYPE_p_std__shared_ptrT_libcellml__VariablePair_t, SWIG_POINTER_NEW | SWIG_POINTER_OWN);
}

%typemap(out) libcellml::AnyCellmlElement *AnyCellmlElement() {
  /*
  Here we take the returned value from the Constructor for this object and cast it
  to the pointer that it actually is.  Once that is done we can set the required resultobj.
  */
  std::shared_ptr<  libcellml::AnyCellmlElement > *smartresult = reinterpret_cast<std::shared_ptr<  libcellml::AnyCellmlElement > *>(result);
  resultobj = SWIG_NewPointerObj(SWIG_as_voidptr(smartresult), SWIGTYPE_p_std__shared_ptrT_libcellml__AnyCellmlElement_t, SWIG_POINTER_NEW | SWIG_POINTER_OWN);
}

%extend libcellml::UnitsItem {
    UnitsItem(const UnitsPtr &units, size_t index) {
        auto ptr = new std::shared_ptr<libcellml::UnitsItem>(libcellml::UnitsItem::create(units, index));
        return reinterpret_cast<libcellml::UnitsItem *>(ptr);
    }
}

%extend libcellml::VariablePair {
    VariablePair(const VariablePtr &variable1, const VariablePtr &variable2) {
        auto ptr = new std::shared_ptr<libcellml::VariablePair>(libcellml::VariablePair::create(variable1, variable2));
        return reinterpret_cast<libcellml::VariablePair *>(ptr);
    }
}

%ignore libcellml::UnitsItem::create;
%ignore libcellml::VariablePair::create;

%include "libcellml/types.h"
