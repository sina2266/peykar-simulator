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
const { modelWithParseErrors } = require('./resources')

let libcellml = null

describe("Types tests", () => {
    let m

    beforeAll(async () => {
        libcellml = await loadLibCellML()
    });
    test("Checking UnitsItem.", () => {
        const u = new libcellml.Units()
        u.setName("fahr")
        const ui = new libcellml.UnitsItem(u, 3)

        expect(ui.units().name()).toBe("fahr")
        expect(ui.index()).toBe(3)
        expect(ui.isValid()).toBe(false)
    })
    test("Checking VariablePair.", () => {
         const v1 = new libcellml.Variable()
         v1.setName("v1")
         const v2 = new libcellml.Variable()
         v2.setName("v2")
         const vp = new libcellml.VariablePair(v1, v2)

         expect(vp.variable1().name()).toBe("v1")
         expect(vp.variable2().name()).toBe("v2")
         expect(vp.isValid()).toBe(true)
    });
    test("Checking AnyCellmlElement.", () => {
         const p = new libcellml.Parser(true)
         const m = p.parseModel(modelWithParseErrors)

         expect(p.issueCount()).toBe(5)

         const item = p.issue(2).item()

         expect(item.type().value).toBe(libcellml.CellmlElementType.IMPORT.value)
         expect(item.component()).toBeNull()
         expect(item.importSource()).not.toBeNull()
         expect(item.model()).toBeNull()
         expect(item.reset()).toBeNull()
         expect(item.units()).toBeNull()
         expect(item.unitsItem()).toBeNull()
         expect(item.variable()).toBeNull()
         expect(item.variablePair()).toBeNull()
    });
})
