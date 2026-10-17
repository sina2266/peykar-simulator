%module(package="libcellml") analyserequationast

#define LIBCELLML_EXPORT

%include <std_string.i>

%import "createconstructor.i"
%import "types.i"

%feature("docstring") libcellml::AnalyserEquationAst
"Creates an :class:`AnalyserEquationAst` object.";

%feature("docstring") libcellml::AnalyserEquationAst::type
"Returns the :enum:`AnalyserEquationAst::Type` for this :class:`AnalyserEquationAst` object.";

%feature("docstring") libcellml::AnalyserEquationAst::typeAsString
"Returns the :enum:`AnalyserEquationAst::Type` as a string for this :class:`AnalyserEquationAst` object.";

%feature("docstring") libcellml::AnalyserEquationAst::setType
"Sets the :enum:`AnalyserEquationAst::Type` for this :class:`AnalyserEquationAst` object.";

%feature("docstring") libcellml::AnalyserEquationAst::value
"Returns the value string for this :class:`AnalyserEquationAst` object.";

%feature("docstring") libcellml::AnalyserEquationAst::setValue
"Sets the value string for this :class:`AnalyserEquationAst` object.";

%feature("docstring") libcellml::AnalyserEquationAst::variable
"Returns the :class:`Variable` for this :class:`AnalyserEquationAst` object.";

%feature("docstring") libcellml::AnalyserEquationAst::setVariable
"Sets the :class:`Variable` for this :class:`AnalyserEquationAst` object.";

%feature("docstring") libcellml::AnalyserEquationAst::parent
"Returns the :class:`AnalyserEquationAst` parent for this :class:`AnalyserEquationAst` object.";

%feature("docstring") libcellml::AnalyserEquationAst::setParent
"Sets the :class:`AnalyserEquationAst` parent for this :class:`AnalyserEquationAst` object.";

%feature("docstring") libcellml::AnalyserEquationAst::leftChild
"Returns the :class:`AnalyserEquationAst` left child for this :class:`AnalyserEquationAst` object.";

%feature("docstring") libcellml::AnalyserEquationAst::setLeftChild
"Sets the :class:`AnalyserEquationAst` left child for this :class:`AnalyserEquationAst` object.";

%feature("docstring") libcellml::AnalyserEquationAst::rightChild
"Returns the :class:`AnalyserEquationAst` right child for this :class:`AnalyserEquationAst` object.";

%feature("docstring") libcellml::AnalyserEquationAst::setRightChild
"Sets the :class:`AnalyserEquationAst` right child for this :class:`AnalyserEquationAst` object.";

%feature("docstring") libcellml::AnalyserEquationAst::swapLeftAndRightChildren
"Swaps the left and right children of this :class:`AnalyserEquationAst` object.";

%{
#include <float.h>
#include <math.h>

#include "libcellml/analyserequationast.h"
%}

%pythoncode %{
# libCellML generated wrapper code starts here.
%}

%create_constructor(AnalyserEquationAst)

%include "libcellml/types.h"
%include "libcellml/analyserequationast.h"
