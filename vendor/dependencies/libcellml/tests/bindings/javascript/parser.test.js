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
const { sineModel, componentImportModel } = require('./resources')

let libcellml = null

describe("Parser tests", () => {
    beforeAll(async () => {
        libcellml = await libCellMLModule()
    })
    test('Checking Parser parse model.', () => {
        const p = new libcellml.Parser(true)

        const m = p.parseModel(sineModel)

        expect(m.componentCount()).toBe(1)
    })
    test('Checking Parser parse model import component.', () => {
        const p = new libcellml.Parser(true)

        const m = p.parseModel(componentImportModel)

        expect(m.componentCount()).toBe(1)
        expect(m.componentByIndex(0).isImport()).toBe(true)
    })
    test('Checking Parser parse isStrict/setStrict.', () => {
        const p = new libcellml.Parser(true)

        expect(p.isStrict()).toBe(true)
        p.setStrict(false)
        expect(p.isStrict()).toBe(false)
    })
})
