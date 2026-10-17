%module(package="libcellml") generator

#define LIBCELLML_EXPORT

%include <std_string.i>

%import "analysermodel.i"
%import "createconstructor.i"
%import "generatorprofile.i"
%import "generatorvariabletracker.i"
%import "logger.i"

%feature("docstring") libcellml::Generator
"Creates a :class:`Generator` object.";

%feature("docstring") libcellml::Generator::interfaceCode(const AnalyserModelPtr &analyserModel, const GeneratorProfilePtr &generatorProfile, const GeneratorVariableTrackerPtr &generatorVariableTracker)
"Returns the interface code for the analyser model using the generator profile and generator variable tracker.";

%feature("docstring") libcellml::Generator::interfaceCode(const AnalyserModelPtr &analyserModel, const GeneratorProfilePtr &generatorProfile)
"Returns the interface code for the analyser model using the generator profile.";

%feature("docstring") libcellml::Generator::interfaceCode(const AnalyserModelPtr &analyserModel, GeneratorProfile::Profile profile)
"Returns the interface code for the analyser model using the generator profile enumeration type.";

%feature("docstring") libcellml::Generator::interfaceCode(const AnalyserModelPtr &analyserModel, const GeneratorVariableTrackerPtr &generatorVariableTracker)
"Returns the interface code for the analyser model using the generator variable tracker.";

%feature("docstring") libcellml::Generator::interfaceCode(const AnalyserModelPtr &analyserModel)
"Returns the interface code for the analyser model.";

%feature("docstring") libcellml::Generator::implementationCode(const AnalyserModelPtr &analyserModel, const GeneratorProfilePtr &generatorProfile, const GeneratorVariableTrackerPtr &generatorVariableTracker)
"Returns the implementation code for the analyser model using the generator profile and generator variable tracker.";

%feature("docstring") libcellml::Generator::implementationCode(const AnalyserModelPtr &analyserModel, const GeneratorProfilePtr &generatorProfile)
"Returns the implementation code for the analyser model using the generator profile.";

%feature("docstring") libcellml::Generator::implementationCode(const AnalyserModelPtr &analyserModel, GeneratorProfile::Profile profile)
"Returns the implementation code for the analyser model using the generator profile enumeration type.";

%feature("docstring") libcellml::Generator::implementationCode(const AnalyserModelPtr &analyserModel, const GeneratorVariableTrackerPtr &generatorVariableTracker)
"Returns the implementation code for the analyser model using the generator variable tracker.";

%feature("docstring") libcellml::Generator::implementationCode(const AnalyserModelPtr &analyserModel)
"Returns the implementation code for the analyser model.";

%feature("docstring") libcellml::Generator::equationCode
"Returns the equation code for the given equation AST using the generator profile.";

%feature("docstring") libcellml::Generator::equationCode(const AnalyserEquationAstPtr &ast)
"Returns the equation code for the given equation AST using a default generator profile.";

%{
#include "libcellml/generator.h"
#include "libcellml/generatorvariabletracker.h"
%}

%pythoncode %{
# libCellML generated wrapper code starts here.
%}

%create_constructor(Generator)

%include "libcellml/generator.h"
