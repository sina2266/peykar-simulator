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
    test('Checking ParentedEntity parent.', () => {
        let m = new libcellml.Model()
        let c = new libcellml.Component()
        let v = new libcellml.Variable()

        v.setName('v')
        c.setName('c')
        m.setName('m')

        expect(v.hasParent()).toBe(false)

        m.addComponent(c)
        c.addVariable(v)


        expect(v.hasParent()).toBe(true)
        expect(c.hasParent()).toBe(true)

        expect(v.hasAncestor(m)).toBe(true)

        const parent = v.parent()
        expect(parent instanceof libcellml.Component).toBe(true)
        expect(parent instanceof libcellml.Model).toBe(false)
    })
})
