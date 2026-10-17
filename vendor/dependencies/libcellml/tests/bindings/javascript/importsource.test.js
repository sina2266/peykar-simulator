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

describe("Import Source tests", () => {
    beforeAll(async () => {
        libcellml = await loadLibCellML()
    })
    test('Checking ImportSource URL manipulation.', () => {
        const iS = new libcellml.ImportSource()

        expect(iS.url()).toBe("")

        iS.setUrl("url_string")

        expect(iS.url()).toBe("url_string")
    })
    test('Checking ImportSource model manipulation.', () => {
        const iS = new libcellml.ImportSource()
        const m = new libcellml.Model()

        expect(iS.model()).toBeNull()

        iS.setModel(m)

        expect(iS.hasModel()).toBe(true)

        iS.removeModel()

        expect(iS.hasModel()).toBe(false)
    })
    test('Checking ImportSource clone.', () => {
     const iS = new libcellml.ImportSource()

     iS.setUrl("long_url")

     const clonedIS = iS.clone()

     expect(clonedIS.url()).toBe("long_url")
    })
})
