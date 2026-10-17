/*
Copyright libCellML Contributors

Licensed under the Apache License, Version 2.0 (the "License")
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

describe("Reset tests", () => {
    beforeAll(async () => {
        libcellml = await libCellMLModule()
    })
    test('Checking Reset order.', () => {
        const r = new libcellml.Reset()

        expect(r.order()).toBe(0)

        r.setOrder(-3)
        expect(r.order()).toBe(-3)
    })
    test('Checking Reset order set.', () => {
        const r = new libcellml.Reset()

        expect(r.isOrderSet()).toBe(false)

        r.setOrder(5)
        expect(r.isOrderSet()).toBe(true)
    })
    test('Checking Reset variable.', () => {
        const r = new libcellml.Reset()
        const v = new libcellml.Variable()

        expect(r.variable()).toBeNull()

        r.setVariable(v)
        expect(r.variable()).toStrictEqual(v)
    })
    test('Checking Reset test variable.', () => {
        const r = new libcellml.Reset()
        const v = new libcellml.Variable()

        expect(r.testVariable()).toBeNull()

        r.setTestVariable(v)
        expect(r.testVariable()).toStrictEqual(v)
    })
    test('Checking Reset test value.', () => {
        const r = new libcellml.Reset()

        expect(r.testValue()).toBe('')

        r.setTestValue('some value')
        expect(r.testValue()).toBe('some value')

        r.appendTestValue(' more')
        expect(r.testValue()).toBe('some value more')

        r.removeTestValue()
        expect(r.testValue()).toBe('')
    })
    test('Checking Reset test value id.', () => {
        const r = new libcellml.Reset()

        expect(r.testValueId()).toBe('')

        r.setTestValueId('id')
        expect(r.testValueId()).toBe('id')

        r.removeTestValueId()
        expect(r.testValueId()).toBe('')
    })
    test('Checking Reset reset value.', () => {
        const r = new libcellml.Reset()

        expect(r.resetValue()).toBe('')

        r.setResetValue('some value')
        expect(r.resetValue()).toBe('some value')

        r.appendResetValue(' more')
        expect(r.resetValue()).toBe('some value more')

        r.removeResetValue()
        expect(r.resetValue()).toBe('')
    })
    test('Checking Reset reset value id.', () => {
        const r = new libcellml.Reset()

        expect(r.resetValueId()).toBe('')

        r.setResetValueId('id')
        expect(r.resetValueId()).toBe('id')

        r.removeResetValueId()
        expect(r.resetValueId()).toBe('')
    })
    test('Checking Reset clone.', () => {
        const r = new libcellml.Reset()

        r.setOrder(7)
        r.setTestValue('some test value')

        const rClone = r.clone()
        expect(rClone).toStrictEqual(r)
    })
    test('Checking Reset parent.', () => {
        const r = new libcellml.Reset()
        const c = new libcellml.Component()

        expect(r.hasParent()).toBe(false)

        c.addReset(r)
        expect(r.hasParent()).toBe(true)
        expect(r.hasAncestor(c)).toBe(true)
    })
    test('Checking Reset id.', () => {
        const r = new libcellml.Reset()

        expect(r.id()).toBe('')

        r.setId('id')
        expect(r.id()).toBe('id')
    })
})
