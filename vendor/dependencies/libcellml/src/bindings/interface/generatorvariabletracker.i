%module(package="libcellml") generatorvariabletracker

#define LIBCELLML_EXPORT

%import "createconstructor.i"
%import "logger.i"

%feature("docstring") libcellml::GeneratorVariableTracker
"Creates a :class:`GeneratorVariableTracker` object.";

%feature("docstring") libcellml::GeneratorVariableTracker::isTrackedVariable
"Tests if the given variable is tracked.";

%feature("docstring") libcellml::GeneratorVariableTracker::isUntrackedVariable
"Tests if the given variable is untracked.";

%feature("docstring") libcellml::GeneratorVariableTracker::trackVariable
"Tracks the given variable.";

%feature("docstring") libcellml::GeneratorVariableTracker::untrackVariable
"Untracks the given variable.";

%feature("docstring") libcellml::GeneratorVariableTracker::trackAllConstants
"Tracks all the constants in the given model.";

%feature("docstring") libcellml::GeneratorVariableTracker::untrackAllConstants
"Untracks all the constants in the given model.";

%feature("docstring") libcellml::GeneratorVariableTracker::trackAllComputedConstants
"Tracks all the computed constants in the given model.";

%feature("docstring") libcellml::GeneratorVariableTracker::untrackAllComputedConstants
"Untracks all the computed constants in the given model.";

%feature("docstring") libcellml::GeneratorVariableTracker::trackAllAlgebraicVariables
"Tracks all the algebraic variables in the given model.";

%feature("docstring") libcellml::GeneratorVariableTracker::untrackAllAlgebraicVariables
"Untracks all the algebraic variables in the given model.";

%feature("docstring") libcellml::GeneratorVariableTracker::trackAllVariables
"Tracks all the variables in the given model.";

%feature("docstring") libcellml::GeneratorVariableTracker::untrackAllVariables
"Untracks all the variables in the given model.";

%feature("docstring") libcellml::GeneratorVariableTracker::trackedConstantCount
"Returns the number of tracked constants in the given model.";

%feature("docstring") libcellml::GeneratorVariableTracker::untrackedConstantCount
"Returns the number of untracked constants in the given model.";

%feature("docstring") libcellml::GeneratorVariableTracker::trackedComputedConstantCount
"Returns the number of tracked computed constants in the given model.";

%feature("docstring") libcellml::GeneratorVariableTracker::untrackedComputedConstantCount
"Returns the number of untracked computed constants in the given model.";

%feature("docstring") libcellml::GeneratorVariableTracker::trackedAlgebraicVariableCount
"Returns the number of tracked algebraic variables in the given model.";

%feature("docstring") libcellml::GeneratorVariableTracker::untrackedAlgebraicVariableCount
"Returns the number of untracked algebraic variables in the given model.";

%feature("docstring") libcellml::GeneratorVariableTracker::trackedVariableCount
"Returns the number of tracked variables in the given model.";

%feature("docstring") libcellml::GeneratorVariableTracker::untrackedVariableCount
"Returns the number of untracked variables in the given model.";

%{
#include "libcellml/generatorvariabletracker.h"
%}

%pythoncode %{
# libCellML generated wrapper code starts here.
%}

%create_constructor(GeneratorVariableTracker)

%include "libcellml/generatorvariabletracker.h"
