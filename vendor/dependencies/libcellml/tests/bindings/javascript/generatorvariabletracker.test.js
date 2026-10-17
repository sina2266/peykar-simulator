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

const loadLibCellML = require('libcellml.js/libcellml.common')
const { hhSquidAxon1952 } = require('./resources')

let libcellml = null

describe("GeneratorVariableTracker tests", () => {
    beforeAll(async () => {
        libcellml = await loadLibCellML()
    })
    test('Checking Generator tracking/untracking of variables.', () => {
        const p = new libcellml.Parser(true)
        const m = p.parseModel(hhSquidAxon1952)
        const v = m.componentByName("membrane", true).variableByName("Cm")
        const a = new libcellml.Analyser()

        a.analyseModel(m)

        const am = a.analyserModel()
        const av = am.analyserVariable(v)
        const gvt = new libcellml.GeneratorVariableTracker()

        gvt.untrackVariable(av)

        expect(gvt.isTrackedVariable(av)).toBe(false)
        expect(gvt.isUntrackedVariable(av)).toBe(true)

        gvt.trackVariable(av)

        expect(gvt.isTrackedVariable(av)).toBe(true)
        expect(gvt.isUntrackedVariable(av)).toBe(false)

        gvt.untrackAllConstants(am)

        expect(gvt.trackedConstantCount(am)).toBe(0)
        expect(gvt.untrackedConstantCount(am)).toBe(5)

        gvt.trackAllConstants(am)

        expect(gvt.trackedConstantCount(am)).toBe(5)
        expect(gvt.untrackedConstantCount(am)).toBe(0)

        gvt.untrackAllComputedConstants(am)

        expect(gvt.trackedComputedConstantCount(am)).toBe(0)
        expect(gvt.untrackedComputedConstantCount(am)).toBe(3)

        gvt.trackAllComputedConstants(am)

        expect(gvt.trackedComputedConstantCount(am)).toBe(3)
        expect(gvt.untrackedComputedConstantCount(am)).toBe(0)

        gvt.untrackAllAlgebraicVariables(am)

        expect(gvt.trackedAlgebraicVariableCount(am)).toBe(0)
        expect(gvt.untrackedAlgebraicVariableCount(am)).toBe(10)

        gvt.trackAllAlgebraicVariables(am)

        expect(gvt.trackedAlgebraicVariableCount(am)).toBe(10)
        expect(gvt.untrackedAlgebraicVariableCount(am)).toBe(0)

        gvt.untrackAllVariables(am)

        expect(gvt.trackedVariableCount(am)).toBe(0)
        expect(gvt.untrackedVariableCount(am)).toBe(18)

        gvt.trackAllVariables(am)

        expect(gvt.trackedVariableCount(am)).toBe(18)
        expect(gvt.untrackedVariableCount(am)).toBe(0)
    })
})
