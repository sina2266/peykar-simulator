/*
Copyright libCellML Contributors

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <emscripten/bind.h>

#include "libcellml/generatorvariabletracker.h"

using namespace emscripten;

EMSCRIPTEN_BINDINGS(libcellml_generatorvariabletracker)
{
    class_<libcellml::GeneratorVariableTracker, base<libcellml::Logger>>("GeneratorVariableTracker")
        .smart_ptr_constructor("GeneratorVariableTracker", &libcellml::GeneratorVariableTracker::create)
        .function("isTrackedVariable", &libcellml::GeneratorVariableTracker::isTrackedVariable)
        .function("isUntrackedVariable", &libcellml::GeneratorVariableTracker::isUntrackedVariable)
        .function("trackVariable", &libcellml::GeneratorVariableTracker::trackVariable)
        .function("untrackVariable", &libcellml::GeneratorVariableTracker::untrackVariable)
        .function("trackAllConstants", &libcellml::GeneratorVariableTracker::trackAllConstants)
        .function("untrackAllConstants", &libcellml::GeneratorVariableTracker::untrackAllConstants)
        .function("trackAllComputedConstants", &libcellml::GeneratorVariableTracker::trackAllComputedConstants)
        .function("untrackAllComputedConstants", &libcellml::GeneratorVariableTracker::untrackAllComputedConstants)
        .function("trackAllAlgebraicVariables", &libcellml::GeneratorVariableTracker::trackAllAlgebraicVariables)
        .function("untrackAllAlgebraicVariables", &libcellml::GeneratorVariableTracker::untrackAllAlgebraicVariables)
        .function("trackAllVariables", &libcellml::GeneratorVariableTracker::trackAllVariables)
        .function("untrackAllVariables", &libcellml::GeneratorVariableTracker::untrackAllVariables)
        .function("trackedConstantCount", &libcellml::GeneratorVariableTracker::trackedConstantCount)
        .function("untrackedConstantCount", &libcellml::GeneratorVariableTracker::untrackedConstantCount)
        .function("trackedComputedConstantCount", &libcellml::GeneratorVariableTracker::trackedComputedConstantCount)
        .function("untrackedComputedConstantCount", &libcellml::GeneratorVariableTracker::untrackedComputedConstantCount)
        .function("trackedAlgebraicVariableCount", &libcellml::GeneratorVariableTracker::trackedAlgebraicVariableCount)
        .function("untrackedAlgebraicVariableCount", &libcellml::GeneratorVariableTracker::untrackedAlgebraicVariableCount)
        .function("trackedVariableCount", &libcellml::GeneratorVariableTracker::trackedVariableCount)
        .function("untrackedVariableCount", &libcellml::GeneratorVariableTracker::untrackedVariableCount)
    ;
}
