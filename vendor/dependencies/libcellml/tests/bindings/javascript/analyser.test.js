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

const libCellMLModule = require('libcellml.js/libcellml.common')
const { basicModel, modelWithError } = require('./resources')

let libcellml = null

describe("Analyser tests", () => {
  let m
  let a
  let aev
  let v

  beforeAll(async () => {
    libcellml = await libCellMLModule();
  });
  beforeEach(() => {
    const p = new libcellml.Parser(true)
    m = p.parseModel(basicModel)
    a = new libcellml.Analyser()

    a.analyseModel(m)
    expect(a.issueCount()).toBe(0)

    v = m.componentByIndex(0).variableByIndex(0)
    aev = new libcellml.AnalyserExternalVariable(v)
  });
  test("Checking Analyser.analyseModel.", () => {
    const p = new libcellml.Parser(true)

    m = p.parseModel(modelWithError)
    expect(p.issueCount()).toBe(0)

    a.analyseModel(m)

    expect(a.issueCount()).toBe(1)
    expect(a.issue(0).description()).toBe("Variable 'time' in component 'my_component' cannot be both a variable of integration and initialised.")
  });
  test("Checking Analyser.addExternalVariableByVariable.", () => {
     expect(a.addExternalVariableByVariable(v)).toBe(true)
  });
  test("Checking Analyser.addExternalVariableByExternalVariable.", () => {
     expect(a.addExternalVariableByExternalVariable(aev)).toBe(true)
  });
  test("Checking Analyser.removeExternalVariableByIndex.", () => {
    a.addExternalVariableByVariable(v)
    expect(a.externalVariableCount()).toBe(1)
    a.removeExternalVariableByIndex(0)
    expect(a.externalVariableCount()).toBe(0)
  });
  test("Checking Analyser.removeExternalVariableByVariable.", () => {
    a.addExternalVariableByVariable(v)
    expect(a.externalVariableCount()).toBe(1)
    a.removeExternalVariableByVariable(v)
    expect(a.externalVariableCount()).toBe(0)
  });
  test("Checking Analyser.removeExternalVariableByExternalVariable.", () => {
    a.addExternalVariableByExternalVariable(aev)
    expect(a.externalVariableCount()).toBe(1)
    a.removeExternalVariableByExternalVariable(aev)
    expect(a.externalVariableCount()).toBe(0)
  });
  test("Checking Analyser.removeAllExternalVariables.", () => {
    a.addExternalVariableByVariable(v)
    expect(a.externalVariableCount()).toBe(1)
    a.removeAllExternalVariables()
    expect(a.externalVariableCount()).toBe(0)
  });
  test("Checking Analyser.containsExternalVariableByVariable.", () => {
    expect(a.containsExternalVariableByVariable(v)).toBe(false)
    a.addExternalVariableByVariable(v)
    expect(a.containsExternalVariableByVariable(v)).toBe(true)
  });
  test("Checking Analyser.containsExternalVariableByExternalVariable.", () => {
    expect(a.containsExternalVariableByExternalVariable(aev)).toBe(false)
    a.addExternalVariableByExternalVariable(aev)
    expect(a.containsExternalVariableByExternalVariable(aev)).toBe(true)
  });
  test("Checking Analyser.externalVariableByIndex.", () => {
    a.addExternalVariableByExternalVariable(aev)
    expect(a.externalVariableByIndex(0)).toStrictEqual(aev)
  });
  test("Checking Analyser.externalVariableByVariable.", () => {
    a.addExternalVariableByExternalVariable(aev)
    expect(a.externalVariableByVariable(v)).toStrictEqual(aev)
  });
  test("Checking Analyser.externalVariableCount.", () => {
    expect(a.externalVariableCount()).toBe(0)
    a.addExternalVariableByVariable(v)
    expect(a.externalVariableCount()).toBe(1)
  });
  test("Checking Analyser.model.", () => {
    expect(a.analyserModel()).toBeDefined()
  });
})
