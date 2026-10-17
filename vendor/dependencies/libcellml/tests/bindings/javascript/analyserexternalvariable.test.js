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

describe("Analyser External Variable tests", () => {
    let m

    beforeAll(async () => {
        libcellml = await loadLibCellML()
    });
    beforeEach(() => {
        const p = new libcellml.Parser(true)
        m = p.parseModel(hhSquidAxon1952)
    });
    test("Checking Analyser.External Variable variable.", () => {
        aev = new libcellml.AnalyserExternalVariable(m.componentByName("membrane", true).variableByName("V"))

        expect(aev.variable().name()).toBe("V")
    })
    test("Checking Analyser.External Variable dependency.", () => {
        aev = new libcellml.AnalyserExternalVariable(m.componentByName("membrane", true).variableByName("V"))

        expect(aev.dependencyCount()).toBe(0)

        const dependentVariable1 = m.componentByName("membrane", true).variableByName("i_Na")
        const dependentVariable2 = m.componentByName("membrane", true).variableByName("i_K")
        const dependentVariable3 = m.componentByName("membrane", true).variableByName("i_L")

        expect(aev.addDependency(dependentVariable1)).toEqual(true)
        expect(aev.containsDependencyByModel(m, "membrane", "i_Na")).toEqual(true)
        expect(aev.containsDependencyByVariable(dependentVariable1)).toEqual(true)

        expect(aev.dependencyCount()).toBe(1)

        aev.addDependency(dependentVariable2)
        aev.addDependency(dependentVariable3)

        expect(aev.dependencyByIndex(1).name()).toBe("i_K")
        expect(aev.dependencyByModel(m, "membrane", "i_L").name()).toBe("i_L")

        expect(aev.dependencies().size()).toBe(3)

        expect(aev.removeDependencyByIndex(2)).toEqual(true)
        expect(aev.removeDependencyByModel(m, "membrane", "i_K")).toEqual(true)
        expect(aev.removeDependencyByVariable(dependentVariable1)).toEqual(true)

        expect(aev.dependencyCount()).toBe(0)

        aev.addDependency(dependentVariable1)
        aev.addDependency(dependentVariable2)
        aev.addDependency(dependentVariable3)

        expect(aev.dependencyCount()).toBe(3)

        aev.removeAllDependencies()

        expect(aev.dependencyCount()).toBe(0)
    });
})
