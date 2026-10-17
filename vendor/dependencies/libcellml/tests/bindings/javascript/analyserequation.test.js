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
const { basicModel } = require('./resources')

let libcellml = null

describe("Analyser Equation tests", () => {
    let m
    let a
    let am
    let eqn

    beforeAll(async () => {
        libcellml = await loadLibCellML()
    });
    beforeEach(() => {
        const p = new libcellml.Parser(true)
        m = p.parseModel(basicModel)
        a = new libcellml.Analyser()

        a.analyseModel(m)
        expect(a.issueCount()).toBe(0)

        am = a.analyserModel()

        expect(am.analyserEquationCount()).toBe(1)

        eqn = am.analyserEquation(0)
    });
    test('Checking Analyser Equation type.', () => {
        expect(eqn.type().value).toBe(libcellml.AnalyserEquation.Type.ODE.value)
        expect(libcellml.AnalyserEquation.typeAsString(eqn.type())).toBe("ode")
    });
    test('Checking Analyser Equation isStateRateBased.', () => {
        expect(eqn.isStateRateBased()).toBe(false)
    });
    test('Checking Analyser Equation dependencyCount.', () => {
        expect(eqn.dependencyCount()).toBe(0)
    });
    test('Checking Analyser Equation dependencies.', () => {
        expect(eqn.dependencies().size()).toBe(0)
    });
    test('Checking Analyser Equation dependency.', () => {
        expect(eqn.dependency(0)).toBeNull()
    });
    test('Checking Analyser Equation nlaSystemIndex.', () => {
        expect(eqn.nlaSystemIndex()).toBe(4294967295)
    });
    test('Checking Analyser Equation nlaSiblingCount.', () => {
        expect(eqn.nlaSiblingCount()).toBe(0)
    });
    test('Checking Analyser Equation nlaSiblings.', () => {
        expect(eqn.nlaSiblings().size()).toBe(0)
    });
    test('Checking Analyser Equation nlaSibling.', () => {
        expect(eqn.nlaSibling(0)).toBeNull()
    });
    test('Checking Analyser Equation computedConstantCount.', () => {
        expect(eqn.computedConstantCount()).toBe(0)
    });
    test('Checking Analyser Equation computedConstants.', () => {
        expect(eqn.computedConstants().size()).toBe(0)
    });
    test('Checking Analyser Equation computedConstant.', () => {
        expect(eqn.computedConstant(0)).toBeNull()
    });
    test('Checking Analyser Equation algebraicVariableCount.', () => {
        expect(eqn.algebraicVariableCount()).toBe(0)
    });
    test('Checking Analyser Equation algebraicVariables.', () => {
        expect(eqn.algebraicVariables().size()).toBe(0)
    });
    test('Checking Analyser Equation algebraicVariable.', () => {
        expect(eqn.algebraicVariable(0)).toBeNull()
    });
    test('Checking Analyser Equation externalVariableCount.', () => {
        expect(eqn.externalVariableCount()).toBe(0)
    });
    test('Checking Analyser Equation externalVariables.', () => {
        expect(eqn.externalVariables().size()).toBe(0)
    });
    test('Checking Analyser Equation externalVariable.', () => {
        expect(eqn.externalVariable(0)).toBeNull()
    });
    test('Checking Analyser Equation AST.', () => {
        expect(eqn.ast().value()).toBe("")
    });
 })
