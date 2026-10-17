%module(package="libcellml") analyserequation

#define LIBCELLML_EXPORT

%include <std_shared_ptr.i>
%include <std_vector.i>

%import "types.i"

%feature("docstring") libcellml::AnalyserEquation
"Creates an :class:`AnalyserEquation` object.";

%feature("docstring") libcellml::AnalyserEquation::type
"Returns the :enum:`AnalyserEquation::Type` for this :class:`AnalyserEquation` object.";

%feature("docstring") libcellml::AnalyserEquation::typeAsString
"Returns the :enum:`AnalyserEquation::Type` as a string for this :class:`AnalyserEquation` object.";

%feature("docstring") libcellml::AnalyserEquation::ast
"Returns the :class:`AnalyserEquationAst` object for this :class:`AnalyserEquation` object.";

%feature("docstring") libcellml::AnalyserEquation::dependencyCount
"Returns the number of dependencies for this :class:`AnalyserEquation` object.";

%feature("docstring") libcellml::AnalyserEquation::dependencies
"Returns the dependencies for this :class:`AnalyserEquation` object.";

%feature("docstring") libcellml::AnalyserEquation::dependency
"Returns the dependency, at the given index, for this :class:`AnalyserEquation` object.";

%feature("docstring") libcellml::AnalyserEquation::nlaSystemIndex
"Returns the index of the NLA system for this :class:`AnalyserEquation` object.";

%feature("docstring") libcellml::AnalyserEquation::nlaSiblingCount
"Returns the number of NLA siblings for this :class:`AnalyserEquation` object.";

%feature("docstring") libcellml::AnalyserEquation::nlaSiblings
"Returns the NLA siblings for this :class:`AnalyserEquation` object.";

%feature("docstring") libcellml::AnalyserEquation::nlaSibling
"Returns the NLA sibling, at the given index, for this :class:`AnalyserEquation` object.";

%feature("docstring") libcellml::AnalyserEquation::isStateRateBased
"Tests if this :class:`AnalyserEquation` object relies on states and/or rates.";

%feature("docstring") libcellml::AnalyserEquation::stateCount
"Returns the number of states computed by this :class:`AnalyserEquation` object.";

%feature("docstring") libcellml::AnalyserEquation::states
"Returns the states computed by this :class:`AnalyserEquation` object.";

%feature("docstring") libcellml::AnalyserEquation::state
"Returns the state, at the given index, computed by this :class:`AnalyserEquation` object.";

%feature("docstring") libcellml::AnalyserEquation::computedConstantCount
"Returns the number of computed constants computed by this :class:`AnalyserEquation` object.";

%feature("docstring") libcellml::AnalyserEquation::computedConstants
"Returns the computed constants computed by this :class:`AnalyserEquation` object.";

%feature("docstring") libcellml::AnalyserEquation::computedConstant
"Returns the computed constant, at the given index, computed by this :class:`AnalyserEquation` object.";

%feature("docstring") libcellml::AnalyserEquation::algebraicVariableCount
"Returns the number of algebraic variables computed by this :class:`AnalyserEquation` object.";

%feature("docstring") libcellml::AnalyserEquation::algebraicVariables
"Returns the algebraic variables computed by this :class:`AnalyserEquation` object.";

%feature("docstring") libcellml::AnalyserEquation::algebraicVariable
"Returns the algebraic variable, at the given index, computed by this :class:`AnalyserEquation` object.";

%feature("docstring") libcellml::AnalyserEquation::externalVariableCount
"Returns the number of external variables computed by this :class:`AnalyserEquation` object.";

%feature("docstring") libcellml::AnalyserEquation::externalVariables
"Returns the external variables computed by this :class:`AnalyserEquation` object.";

%feature("docstring") libcellml::AnalyserEquation::externalVariable
"Returns the external variable, at the given index, computed by this :class:`AnalyserEquation` object.";

%{
#include "libcellml/analyserequation.h"
%}

%template(AnalyserEquationVector) std::vector<libcellml::AnalyserEquationPtr>;

%pythoncode %{
# libCellML generated wrapper code starts here.
%}

%shared_ptr(libcellml::AnalyserEquation);

%include "libcellml/types.h"
%include "libcellml/analyserequation.h"
