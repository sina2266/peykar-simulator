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

describe("Analyser Variable tests", () => {
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

        expect(am.constantCount()).toBe(5)
        expect(am.computedConstantCount()).toBe(3)
        expect(am.algebraicVariableCount()).toBe(10)
        expect(am.externalVariableCount()).toBe(0)
    });
    test('Checking Analyser Variable type.', () => {
        const av = am.algebraicVariable(0)
        expect(av.type().value).toBe(libcellml.AnalyserVariable.Type.ALGEBRAIC_VARIABLE.value)
        expect(libcellml.AnalyserVariable.typeAsString(av.type())).toBe("algebraic_variable")
    });
    test('Checking Analyser Variable index.', () => {
        const av = am.algebraicVariable(7)
        expect(av.index()).toBe(7)
    });
    test('Checking Analyser Variable initialising variable.', () => {
        const av = am.constant(3)
        expect(av.initialisingVariable().name()).toBe("g_Na")
    });
    test('Checking Analyser Variable variable.', () => {
        const av = am.algebraicVariable(3)
        expect(av.variable().name()).toBe("i_Na")
    });
    test('Checking Analyser Variable model.', () => {
        const av = am.algebraicVariable(3)
        expect(av.analyserModel()).toStrictEqual(am)
    });
    test('Checking Analyser Equation analyserEquationCount.', () => {
        const av = am.computedConstant(1)
        expect(av.analyserEquationCount()).toBe(1)
    });
    test('Checking Analyser Variable analyserEquations.', () => {
        const av = am.computedConstant(1)
        expect(av.analyserEquations().size()).toBe(1)
    });
    test('Checking Analyser Variable analyserEquation.', () => {
        const av = am.computedConstant(1)
        expect(av.analyserEquation(0).type().value).toBe(libcellml.AnalyserEquation.Type.COMPUTED_CONSTANT.value)
    });
 })
