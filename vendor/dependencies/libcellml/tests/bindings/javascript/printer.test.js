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
let libcellml = null

describe("Printer tests", () => {
    beforeAll(async () => {
        libcellml = await loadLibCellML()
    })
    test('Checking Printer print model.', () => {
        const p = new libcellml.Printer()

        const model = new libcellml.Model()
        model.setName("my_model")

        const m = p.printModel(model, false)

        expect(m).toBe("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<model xmlns=\"http://www.cellml.org/cellml/2.0#\" name=\"my_model\"/>\n")
    })
})
