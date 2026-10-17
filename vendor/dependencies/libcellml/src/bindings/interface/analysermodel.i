%module(package="libcellml") analysermodel

#define LIBCELLML_EXPORT

%include <std_shared_ptr.i>
%include <std_vector.i>

%import "types.i"

%feature("docstring") libcellml::AnalyserModel
"Creates an :class:`AnalyserModel` object.";

%feature("docstring") libcellml::AnalyserModel::isValid
"Tests if this :class:`AnalyserModel` object is valid.";

%feature("docstring") libcellml::AnalyserModel::type
"Returns the :enum:`AnalyserModel::Type`.";

%feature("docstring") libcellml::AnalyserModel::typeAsString
"Returns the :enum:`AnalyserModel::Type` as a string.";

%feature("docstring") libcellml::AnalyserModel::hasExternalVariables
"Tests if this :class:`AnalyserModel` object has external variables.";

%feature("docstring") libcellml::AnalyserModel::voi
"Returns the :class:`AnalyserVariable` for the variable of integration.";

%feature("docstring") libcellml::AnalyserModel::stateCount
"Returns the number of states contained by this :class:`AnalyserModel` object.";

%feature("docstring") libcellml::AnalyserModel::states
"Returns the states contained by this :class:`AnalyserModel` object.";

%feature("docstring") libcellml::AnalyserModel::state
"Returns the state, specified by index, contained by this :class:`AnalyserModel` object.";

%feature("docstring") libcellml::AnalyserModel::constantCount
"Returns the number of constants contained by this :class:`AnalyserModel` object.";

%feature("docstring") libcellml::AnalyserModel::constants
"Returns the constants contained by this :class:`AnalyserModel` object.";

%feature("docstring") libcellml::AnalyserModel::constant
"Returns the constant, specified by index, contained by this :class:`AnalyserModel` object.";

%feature("docstring") libcellml::AnalyserModel::computedConstantCount
"Returns the number of computed constants contained by this :class:`AnalyserModel` object.";

%feature("docstring") libcellml::AnalyserModel::computedConstants
"Returns the computed constants contained by this :class:`AnalyserModel` object.";

%feature("docstring") libcellml::AnalyserModel::computedConstant
"Returns the computed constant, specified by index, contained by this :class:`AnalyserModel` object.";

%feature("docstring") libcellml::AnalyserModel::algebraicVariableCount
"Returns the number of algebraic variables contained by this :class:`AnalyserModel` object.";

%feature("docstring") libcellml::AnalyserModel::algebraicVariables
"Returns the algebraic variables contained by this :class:`AnalyserModel` object.";

%feature("docstring") libcellml::AnalyserModel::algebraicVariable
"Returns the algebraic variable, specified by index, contained by this :class:`AnalyserModel` object.";

%feature("docstring") libcellml::AnalyserModel::externalVariableCount
"Returns the number of external variables contained by this :class:`AnalyserModel` object.";

%feature("docstring") libcellml::AnalyserModel::externalVariables
"Returns the external variables contained by this :class:`AnalyserModel` object.";

%feature("docstring") libcellml::AnalyserModel::externalVariable
"Returns the external variable, specified by index, contained by this :class:`AnalyserModel` object.";

%feature("docstring") libcellml::AnalyserModel::analyserVariable
"Returns the analyser variable for the given variable.";

%feature("docstring") libcellml::AnalyserModel::analyserEquationCount
"Returns the number of analyser equations contained by this :class:`AnalyserModel` object.";

%feature("docstring") libcellml::AnalyserModel::analyserEquations
"Returns the analyser equations contained by this :class:`AnalyserModel` object.";

%feature("docstring") libcellml::AnalyserModel::analyserEquation
"Returns the analyser equation, specified by index, contained by this :class:`AnalyserModel` object.";

%feature("docstring") libcellml::AnalyserModel::needEqFunction
"Tests if this :class:`AnalyserModel` object needs an \"equal to\" function.";

%feature("docstring") libcellml::AnalyserModel::needNeqFunction
"Tests if this :class:`AnalyserModel` object needs a \"not equal to\" function.";

%feature("docstring") libcellml::AnalyserModel::needLtFunction
"Tests if this :class:`AnalyserModel` object needs a \"less than\" function.";

%feature("docstring") libcellml::AnalyserModel::needLeqFunction
"Tests if this :class:`AnalyserModel` object needs a \"less than or equal to\" function.";

%feature("docstring") libcellml::AnalyserModel::needGtFunction
"Tests if this :class:`AnalyserModel` object needs a \"greater than\" function.";

%feature("docstring") libcellml::AnalyserModel::needGeqFunction
"Tests if this :class:`AnalyserModel` object needs a \"greater than or equal to\" function.";

%feature("docstring") libcellml::AnalyserModel::needAndFunction
"Tests if this :class:`AnalyserModel` object needs an \"and\" function.";

%feature("docstring") libcellml::AnalyserModel::needOrFunction
"Tests if this :class:`AnalyserModel` object needs an \"or\" function.";

%feature("docstring") libcellml::AnalyserModel::needXorFunction
"Tests if this :class:`AnalyserModel` object needs a \"exclusive or\" function.";

%feature("docstring") libcellml::AnalyserModel::needNotFunction
"Tests if this :class:`AnalyserModel` object needs a \"not\" function.";

%feature("docstring") libcellml::AnalyserModel::needMinFunction
"Tests if this :class:`AnalyserModel` object needs a \"minimum\" function.";

%feature("docstring") libcellml::AnalyserModel::needMaxFunction
"Tests if this :class:`AnalyserModel` object needs a \"maximum\" function.";

%feature("docstring") libcellml::AnalyserModel::needSecFunction
"Tests if this :class:`AnalyserModel` object needs a \"secant\" function.";

%feature("docstring") libcellml::AnalyserModel::needCscFunction
"Tests if this :class:`AnalyserModel` object needs a \"cosecant\" function.";

%feature("docstring") libcellml::AnalyserModel::needCotFunction
"Tests if this :class:`AnalyserModel` object needs a \"cotangent\" function.";

%feature("docstring") libcellml::AnalyserModel::needSechFunction
"Tests if this :class:`AnalyserModel` object needs a \"hyperbolic secant\" function.";

%feature("docstring") libcellml::AnalyserModel::needCschFunction
"Tests if this :class:`AnalyserModel` object needs a \"hyperbolic cosecant\" function.";

%feature("docstring") libcellml::AnalyserModel::needCothFunction
"Tests if this :class:`AnalyserModel` object needs a \"hyperbolic cotangent\" function.";

%feature("docstring") libcellml::AnalyserModel::needAsecFunction
"Tests if this :class:`AnalyserModel` object needs an \"arc secant\" function.";

%feature("docstring") libcellml::AnalyserModel::needAcscFunction
"Tests if this :class:`AnalyserModel` object needs an \"arc cosecant\" function.";

%feature("docstring") libcellml::AnalyserModel::needAcotFunction
"Tests if this :class:`AnalyserModel` object needs an \"arc cotangent\" function.";

%feature("docstring") libcellml::AnalyserModel::needAsechFunction
"Tests if this :class:`AnalyserModel` object needs an \"arc hyperbolic secant\" function.";

%feature("docstring") libcellml::AnalyserModel::needAcschFunction
"Tests if this :class:`AnalyserModel` object needs an \"arc hyperbolic cosecant\" function.";

%feature("docstring") libcellml::AnalyserModel::needAcothFunction
"Tests if this :class:`AnalyserModel` object needs an \"arc hyperbolic cotangent\" function.";

%feature("docstring") libcellml::AnalyserModel::areEquivalentVariables
"Tests if the two variables are equivalents.";

%{
#include "libcellml/analysermodel.h"
%}

%template(AnalyserEquationVector) std::vector<libcellml::AnalyserEquationPtr>;
%template(AnalyserVariableVector) std::vector<libcellml::AnalyserVariablePtr>;

%pythoncode %{
# libCellML generated wrapper code starts here.
%}

%shared_ptr(libcellml::AnalyserModel);

%include "libcellml/types.h"
%include "libcellml/analysermodel.h"
