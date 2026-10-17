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
let libcellml = null

describe("Entity tests", () => {
    beforeAll(async () => {
        libcellml = await libCellMLModule()
    })
    test('Checking Entity id.', () => {
        const v = new libcellml.Variable()

        expect(v.id()).toBe('')

        v.setId('variable_1')
        expect(v.id()).toBe('variable_1')

        v.removeId()
        expect(v.id()).toBe('')
    })
    test('Checking Entity equals.', () => {
        let v1 = new libcellml.Variable()
        let v2 = new libcellml.Variable()

        v1.setName('angols')
        v2.setName('angols')

        v1.setInitialValueByDouble(3.0)
        v2.setInitialValueByDouble(3.0)

        expect(v1.equals(v2)).toBe(true)

        v1.setName('Bovost')
        expect(v1.equals(v2)).toBe(false)
    })
})
