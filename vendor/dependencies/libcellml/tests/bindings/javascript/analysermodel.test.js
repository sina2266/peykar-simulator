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

describe("Analyser Model tests", () => {
    let m
    let a
    let am

    beforeAll(async () => {
        libcellml = await loadLibCellML()
    });
    beforeEach(() => {
        const p = new libcellml.Parser(true)
        m = p.parseModel(hhSquidAxon1952)
        a = new libcellml.Analyser()

        a.analyseModel(m)
        expect(a.issueCount()).toBe(0)

        am = a.analyserModel()
    });
    test('Checking Analyser Model type.', () => {
        expect(am.type().value).toBe(libcellml.AnalyserModel.Type.ODE.value)
        expect(libcellml.AnalyserModel.typeAsString(am.type())).toBe("ode")
    });
    test('Checking Analyser Model isValid.', () => {
        expect(am.isValid()).toBe(true)
    });
    test('Checking Analyser Model has external variables.', () => {
        expect(am.hasExternalVariables()).toBe(false)
    });
    test('Checking Analyser Model VOI.', () => {
        expect(am.voi().variable().name()).toBe("time")
    });
    test('Checking Analyser Model equations related API.', () => {
        expect(am.analyserEquationCount()).toBe(17)
        expect(am.analyserEquations().size()).toBe(17)
        expect(am.analyserEquations().get(1).type().value).toBe(libcellml.AnalyserEquation.Type.ODE.value)
        expect(am.analyserEquation(1).type().value).toBe(libcellml.AnalyserEquation.Type.ODE.value)
    });
    test('Checking Analyser Model states related API.', () => {
        expect(am.stateCount()).toBe(4)
        expect(am.states().size()).toBe(4)
        expect(am.state(2).variable().name()).toBe("m")
    });
    test('Checking Analyser Model constants related API.', () => {
        expect(am.constantCount()).toBe(5)
        expect(am.constants().size()).toBe(5)
        expect(am.constant(2).variable().name()).toBe("g_L")
    });
    test('Checking Analyser Model computed constants related API.', () => {
        expect(am.computedConstantCount()).toBe(3)
        expect(am.computedConstants().size()).toBe(3)
        expect(am.computedConstant(2).variable().name()).toBe("E_K")
    });
    test('Checking Analyser Model algebraic variables related API.', () => {
        expect(am.algebraicVariableCount()).toBe(10)
        expect(am.algebraicVariables().size()).toBe(10)
        expect(am.algebraicVariable(2).variable().name()).toBe("i_K")
    });
    test('Checking Analyser Model external variables related API.', () => {
        expect(am.externalVariableCount()).toBe(0)
        expect(am.externalVariables().size()).toBe(0)
        expect(am.externalVariable(0)).toBeNull()
    });
    test('Checking Analyser Model variable API.', () => {
        expect(am.analyserVariable(null)).toBeNull()
        expect(am.analyserVariable(m.componentByName("membrane").variableByName("V"))).not.toBeNull()
    });
    test('Checking Analyser Model need* API.', () => {
        expect(am.needEqFunction()).toBe(false)
        expect(am.needNeqFunction()).toBe(false)
        expect(am.needLtFunction()).toBe(false)
        expect(am.needLeqFunction()).toBe(true)
        expect(am.needGtFunction()).toBe(false)
        expect(am.needGeqFunction()).toBe(true)
        expect(am.needAndFunction()).toBe(true)
        expect(am.needOrFunction()).toBe(false)
        expect(am.needXorFunction()).toBe(false)
        expect(am.needNotFunction()).toBe(false)
        expect(am.needMinFunction()).toBe(false)
        expect(am.needMaxFunction()).toBe(false)
        expect(am.needSecFunction()).toBe(false)
        expect(am.needCscFunction()).toBe(false)
        expect(am.needCotFunction()).toBe(false)
        expect(am.needSechFunction()).toBe(false)
        expect(am.needCschFunction()).toBe(false)
        expect(am.needCothFunction()).toBe(false)
        expect(am.needAsecFunction()).toBe(false)
        expect(am.needAcscFunction()).toBe(false)
        expect(am.needAcotFunction()).toBe(false)
        expect(am.needAsechFunction()).toBe(false)
        expect(am.needAcschFunction()).toBe(false)
        expect(am.needAcothFunction()).toBe(false)
    });
    test('Checking Analyser Model are equivalent variables.', () => {
        expect(am.areEquivalentVariables(am.algebraicVariable(2).variable(), am.algebraicVariable(7).variable())).toBe(false)
    });
 })
