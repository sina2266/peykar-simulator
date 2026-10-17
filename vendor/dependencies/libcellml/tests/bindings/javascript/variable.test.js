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

describe("Variable tests", () => {
    beforeAll(async () => {
        libcellml = await loadLibCellML()
    })
    test('Checking Variable creation.', () => {
        const v1 = new libcellml.Variable()
        const v2 = new libcellml.Variable("ms")

        expect(v1.name()).toBe("")
        expect(v2.name()).toBe("ms")

        v1.delete()
        v2.delete()
    })
    test('Checking variable equivalence manipulation', () => {
        const v1 = new libcellml.Variable("milliseconds")
        const v2 = new libcellml.Variable("ms")
        const v3 = new libcellml.Variable("millis")

        expect(v1.hasEquivalentVariable(v2, false)).toBe(false)
        expect(v1.equivalentVariableCount()).toBe(0)
        expect(v2.equivalentVariableCount()).toBe(0)

        expect(libcellml.Variable.addEquivalence(v1, v2)).toBe(true)

        expect(v1.hasEquivalentVariable(v2, false)).toBe(true)
        expect(v1.equivalentVariableCount()).toBe(1)
        expect(v2.equivalentVariableCount()).toBe(1)

        expect(v1.equivalentVariable(0).name()).toBe("ms")

        expect(libcellml.Variable.removeEquivalence(v1, v2)).toBe(true)

        expect(v1.equivalentVariableCount()).toBe(0)
        expect(v2.equivalentVariableCount()).toBe(0)

        libcellml.Variable.addEquivalence(v1, v2)
        libcellml.Variable.addEquivalence(v1, v3)

        expect(v1.equivalentVariableCount()).toBe(2)

        v1.removeAllEquivalences()

        expect(v1.equivalentVariableCount()).toBe(0)

        v1.delete()
        v2.delete()
        v3.delete()
    })
    test('Checking variable units manipulation', () => {
        const v = new libcellml.Variable("V")
        const u = new libcellml.Units("mV")

        expect(v.units()).toBeNull()

        v.setUnitsByName("A")

        expect(v.units().name()).toBe("A")

        v.setUnitsByUnits(u)

        expect(v.units().name()).toBe("mV")

        v.removeUnits()

        expect(v.units()).toBeNull()

        v.delete()
        u.delete()
    })
    test('Checking variable initial value manipulation', () => {
        const v = new libcellml.Variable()
        const vInitial = new libcellml.Variable()

        vInitial.setInitialValueByDouble(2.0)

        expect(v.initialValue()).toBe("")

        v.setInitialValueByString("1.34")

        expect(v.initialValue()).toBe("1.34")

        v.setInitialValueByVariable(vInitial)

        expect(vInitial.initialValue()).toBe("2")

        v.removeInitialValue()

        expect(v.initialValue()).toBe("")

        v.delete()
        vInitial.delete()
    })
    test('Checking variable interface manipulation', () => {
        const v = new libcellml.Variable()

        expect(v.interfaceType()).toBe("")

        v.setInterfaceTypeByString("public")
        expect(v.interfaceType()).toBe("public")

        v.setInterfaceTypeByInterfaceType(libcellml.Variable.InterfaceType.PRIVATE)
        expect(v.interfaceType()).toBe("private")

        expect(v.hasInterfaceType(libcellml.Variable.InterfaceType.PUBLIC_AND_PRIVATE)).toBe(false)

        expect(v.permitsInterfaceType(libcellml.Variable.InterfaceType.PRIVATE)).toBe(true)

        v.removeInterfaceType()

        expect(v.interfaceType()).toBe("")

        v.delete()
    })
    test('Checking variable id manipulation', () => {
        const v1 = new libcellml.Variable("milliseconds")
        const v2 = new libcellml.Variable("ms")

        expect(libcellml.Variable.equivalenceConnectionId(v1, v2)).toBe("")
        expect(libcellml.Variable.equivalenceMappingId(v1, v2)).toBe("")

        libcellml.Variable.addEquivalence(v1, v2)
        libcellml.Variable.setEquivalenceConnectionId(v1, v2, "connection_id")
        libcellml.Variable.setEquivalenceMappingId(v1, v2, "mapping_id")

        expect(libcellml.Variable.equivalenceConnectionId(v1, v2)).toBe("connection_id")
        expect(libcellml.Variable.equivalenceMappingId(v1, v2)).toBe("mapping_id")

        libcellml.Variable.removeEquivalence(v1, v2)

        libcellml.Variable.addEquivalenceWithIds(v1, v2, "second_mapping_id", "second_connection_id")

        expect(libcellml.Variable.equivalenceConnectionId(v1, v2)).toBe("second_connection_id")
        expect(libcellml.Variable.equivalenceMappingId(v1, v2)).toBe("second_mapping_id")

        v1.delete()
        v2.delete()
    })
})
