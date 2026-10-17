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

describe("ComponentEntity tests", () => {
    beforeAll(async () => {
        libcellml = await libCellMLModule()
    })
    test('Checking ComponentEntity add component.', () => {
        let m = new libcellml.Model()
        let c = new libcellml.Component()

        expect(m.componentCount()).toBe(0)

        m.addComponent(c)
        expect(m.componentCount()).toBe(1)
    })
    test('Checking ComponentEntity remove component.', () => {
        let m = new libcellml.Model()
        let c1 = new libcellml.Component('c1')
        let c2 = new libcellml.Component('c2')
        let c3 = new libcellml.Component('c3')

        expect(m.componentCount()).toBe(0)

        m.addComponent(c1)
        m.addComponent(c2)
        m.addComponent(c3)
        expect(m.componentCount()).toBe(3)

        m.removeComponentByIndex(1)
        expect(m.componentCount()).toBe(2)

        m.removeComponentByName('c3', false)
        expect(m.componentCount()).toBe(1)

        m.addComponent(c2)
        expect(m.componentCount()).toBe(2)

        m.removeComponentByComponent(c2, false)
        expect(m.componentCount()).toBe(1)

        m.addComponent(c2)
        m.addComponent(c3)
        expect(m.componentCount()).toBe(3)

        m.removeAllComponents()
        expect(m.componentCount()).toBe(0)
    })
    test('Checking ComponentEntity contains component.', () => {
        let m = new libcellml.Model()
        let c1 = new libcellml.Component('c1')
        let c2 = new libcellml.Component('c2')
        let c3 = new libcellml.Component('c3')

        expect(m.componentCount()).toBe(0)
        expect(m.containsComponentByName('c2', false)).toBe(false)
        expect(m.containsComponentByComponent(c3, false)).toBe(false)

        m.addComponent(c1)
        m.addComponent(c2)
        m.addComponent(c3)
        expect(m.componentCount()).toBe(3)

        expect(m.containsComponentByName('c2', false)).toBe(true)
        expect(m.containsComponentByComponent(c3, false)).toBe(true)

        expect(m.containsComponentByName('c4', false)).toBe(false)
    })
    test('Checking ComponentEntity component.', () => {
        let m = new libcellml.Model()
        let c1 = new libcellml.Component('c1')
        let c2 = new libcellml.Component('c2')
        let c3 = new libcellml.Component('c3')

        expect(m.componentCount()).toBe(0)

        m.addComponent(c1)
        m.addComponent(c2)
        m.addComponent(c3)
        expect(m.componentCount()).toBe(3)

        expect(m.componentByIndex(1).equals(c2)).toBe(true)

        expect(m.componentByName('c3', false)).toStrictEqual(c3)
    })
    test('Checking ComponentEntity take component.', () => {
        let m = new libcellml.Model()
        let c1 = new libcellml.Component('c1')
        let c2 = new libcellml.Component('c2')
        let c3 = new libcellml.Component('c3')

        expect(m.componentCount()).toBe(0)

        m.addComponent(c1)
        m.addComponent(c2)
        m.addComponent(c3)
        expect(m.componentCount()).toBe(3)

        let cTaken = m.takeComponentByIndex(1)
        expect(cTaken).toStrictEqual(c2)

        cTaken = m.takeComponentByName('c3', false)
        expect(cTaken).toStrictEqual(c3)
    })
    test('Checking ComponentEntity replace component.', () => {
        let m = new libcellml.Model()
        let c1 = new libcellml.Component('c1')
        let c2 = new libcellml.Component('c2')
        let c3 = new libcellml.Component('c3')

        expect(m.componentCount()).toBe(0)

        m.addComponent(c1)
        m.addComponent(c2)
        expect(m.componentCount()).toBe(2)

        m.replaceComponentByIndex(1, c3)
        expect(m.componentCount()).toBe(2)

        m.replaceComponentByName('c3', c2, false)
        expect(m.componentCount()).toBe(2)

        m.replaceComponentByOldComponent(c2, c3, false)
        expect(m.componentCount()).toBe(2)
    })
    test('Checking ComponentEntity encapsulation id.', () => {
        const c = new libcellml.Component()

        expect(c.encapsulationId()).toBe("")

        c.setEncapsulationId("bob")
        expect(c.encapsulationId()).toBe("bob")

        c.removeEncapsulationId()
        expect(c.encapsulationId()).toBe("")
    })
})
