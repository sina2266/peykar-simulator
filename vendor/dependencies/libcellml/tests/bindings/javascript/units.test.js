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

describe("Units tests", () => {
    beforeAll(async () => {
        libcellml = await libCellMLModule()
    })
    test('Checking Units is base unit.', () => {
        const u = new libcellml.Units()

        expect(u.isBaseUnit()).toBe(true)

        u.addUnitByStandardUnit(libcellml.Units.StandardUnit.FARAD)
        expect(u.isBaseUnit()).toBe(false)
    })
    test('Checking Units add unit by reference string prefix.', () => {
        const u = new libcellml.Units()

        expect(u.unitCount()).toBe(0)

        u.addUnitByReferenceStringPrefix("name", "milli", 1.0, 5, "")
        expect(u.unitCount()).toBe(1)
    })
    test('Checking Units add unit by reference enum prefix.', () => {
        const u = new libcellml.Units()

        expect(u.unitCount()).toBe(0)

        u.addUnitByReferenceEnumPrefix("name", libcellml.Units.Prefix.TERA, 7, 9, "")
        expect(u.unitCount()).toBe(1)
    })
    test('Checking Units add unit by reference int prefix.', () => {
        const u = new libcellml.Units()

        expect(u.unitCount()).toBe(0)

        u.addUnitByReferenceIntPrefix("name", -2, 1.0, 5, "")
        expect(u.unitCount()).toBe(1)
    })
    test('Checking Units add unit by reference exponent.', () => {
        const u = new libcellml.Units()

        expect(u.unitCount()).toBe(0)

        u.addUnitByReferenceExponent("name", -4.0, "")
        expect(u.unitCount()).toBe(1)
    })
    test('Checking Units add unit by reference.', () => {
        const u = new libcellml.Units()

        expect(u.unitCount()).toBe(0)

        u.addUnitByReference("name")
        expect(u.unitCount()).toBe(1)
    })
    test('Checking Units add unit by standard unit string prefix.', () => {
        const u = new libcellml.Units()

        expect(u.unitCount()).toBe(0)

        u.addUnitByStandardUnitStringPrefix(libcellml.Units.StandardUnit.TESLA, "milli", 1.0, 5, "")
        expect(u.unitCount()).toBe(1)
    })
    test('Checking Units add unit by standard unit enum prefix.', () => {
        const u = new libcellml.Units()

        expect(u.unitCount()).toBe(0)

        u.addUnitByStandardUnitEnumPrefix(libcellml.Units.StandardUnit.TESLA, libcellml.Units.Prefix.TERA, 7, 9, "")
        expect(u.unitCount()).toBe(1)
    })
    test('Checking Units add unit by standard unit int prefix.', () => {
        const u = new libcellml.Units()

        expect(u.unitCount()).toBe(0)

        u.addUnitByStandardUnitIntPrefix(libcellml.Units.StandardUnit.TESLA, -2, 1.0, 5, "")
        expect(u.unitCount()).toBe(1)
    })
    test('Checking Units add unit by standard unit exponent.', () => {
        const u = new libcellml.Units()

        expect(u.unitCount()).toBe(0)

        u.addUnitByStandardUnitExponent(libcellml.Units.StandardUnit.TESLA, -4.0, "")
        expect(u.unitCount()).toBe(1)
    })
    test('Checking Units add unit by standard unit.', () => {
        const u = new libcellml.Units()

        expect(u.unitCount()).toBe(0)

        u.addUnitByStandardUnit(libcellml.Units.StandardUnit.TESLA)
        expect(u.unitCount()).toBe(1)
    })
    test('Checking Units unit attribute reference.', () => {
        const u = new libcellml.Units()

        expect(u.unitCount()).toBe(0)

        u.addUnitByStandardUnit(libcellml.Units.StandardUnit.FARAD)
        expect(u.unitCount()).toBe(1)

        expect(u.unitAttributeReference(0)).toBe("farad")

        u.setUnitAttributeReference(0, "mole")
        expect(u.unitAttributeReference(0)).toBe("mole")
    })
    test('Checking Units unit attribute prefix.', () => {
        const u = new libcellml.Units()

        expect(u.unitCount()).toBe(0)

        u.addUnitByReferenceEnumPrefix("name", libcellml.Units.Prefix.TERA, 7, 9, "")
        expect(u.unitCount()).toBe(1)

        expect(u.unitAttributePrefix(0)).toBe("tera")
    })
    test('Checking Units unit attribute exponent.', () => {
        const u = new libcellml.Units()

        expect(u.unitCount()).toBe(0)

        u.addUnitByStandardUnitExponent(libcellml.Units.StandardUnit.TESLA, -4.0, "")
        expect(u.unitCount()).toBe(1)

        expect(u.unitAttributeExponent(0)).toBe(-4.0)
    })
    test('Checking Units unit attribute multiplier.', () => {
        const u = new libcellml.Units()

        expect(u.unitCount()).toBe(0)

        u.addUnitByStandardUnitStringPrefix(libcellml.Units.StandardUnit.TESLA, "milli", -2.0, 5, "")
        expect(u.unitCount()).toBe(1)

        expect(u.unitAttributeMultiplier(0)).toBe(5.0)
    })
    test('Checking Units remove unit by index.', () => {
        const u = new libcellml.Units()

        expect(u.unitCount()).toBe(0)

        u.addUnitByStandardUnit(libcellml.Units.StandardUnit.FARAD)
        u.addUnitByStandardUnit(libcellml.Units.StandardUnit.SIEMENS)
        u.addUnitByStandardUnit(libcellml.Units.StandardUnit.SIEVERT)
        expect(u.unitCount()).toBe(3)

        u.removeUnitByIndex(1)
        expect(u.unitCount()).toBe(2)
    })
    test('Checking Units remove unit by reference.', () => {
        const u = new libcellml.Units()

        expect(u.unitCount()).toBe(0)

        u.addUnitByStandardUnit(libcellml.Units.StandardUnit.FARAD)
        u.addUnitByStandardUnit(libcellml.Units.StandardUnit.SIEMENS)
        u.addUnitByStandardUnit(libcellml.Units.StandardUnit.SIEVERT)
        expect(u.unitCount()).toBe(3)

        u.removeUnitByReference("siemens")
        expect(u.unitCount()).toBe(2)
    })
    test('Checking Units remove unit by standard unit.', () => {
        const u = new libcellml.Units()

        expect(u.unitCount()).toBe(0)

        u.addUnitByStandardUnit(libcellml.Units.StandardUnit.FARAD)
        u.addUnitByStandardUnit(libcellml.Units.StandardUnit.SIEMENS)
        u.addUnitByStandardUnit(libcellml.Units.StandardUnit.SIEVERT)
        expect(u.unitCount()).toBe(3)

        u.removeUnitByStandardUnit(libcellml.Units.StandardUnit.SIEMENS)
        expect(u.unitCount()).toBe(2)
    })
    test('Checking Units remove all units.', () => {
        const u = new libcellml.Units()

        expect(u.unitCount()).toBe(0)

        u.addUnitByStandardUnit(libcellml.Units.StandardUnit.FARAD)
        u.addUnitByStandardUnit(libcellml.Units.StandardUnit.SIEMENS)
        u.addUnitByStandardUnit(libcellml.Units.StandardUnit.SIEVERT)
        expect(u.unitCount()).toBe(3)

        u.removeAllUnits()
        expect(u.unitCount()).toBe(0)
    })
    test('Checking Units set source units.', () => {
        const u = new libcellml.Units()
        const iS = new libcellml.ImportSource()

        expect(u.unitCount()).toBe(0)

        u.setSourceUnits(iS, "imported_units_name")
        expect(u.requiresImports()).toBe(true)
    })
    test('Checking Units clone.', () => {
        const u = new libcellml.Units("mine")

        u.addUnitByStandardUnitStringPrefix(libcellml.Units.StandardUnit.TESLA, "milli", 1.0, 5, "")

        const uClone = u.clone()
        expect(uClone.name()).toBe("mine")
        expect(uClone.unitAttributeReference(0)).toBe("tesla")
    })
    test('Checking Units unit id.', () => {
        const u = new libcellml.Units()

        u.addUnitByStandardUnitStringPrefix(libcellml.Units.StandardUnit.TESLA, "milli", 1.0, 5, "")
        expect(u.unitId(0)).toBe("")

        u.setUnitId(0, "bob")
        expect(u.unitId(0)).toBe("bob")
    })
    test('Checking Units import source.', () => {
        const u = new libcellml.Units()
        let iS = new libcellml.ImportSource()
        iS.setUrl("someplace")

        expect(u.isImport()).toBe(false)

        u.setImportSource(iS)
        expect(u.isImport()).toBe(true)
        expect(u.importSource().url()).toBe("someplace")
    })
    test('Checking Units import reference.', () => {
        const u = new libcellml.Units()

        expect(u.importReference()).toBe("")

        u.setImportReference("over_here")
        expect(u.importReference()).toBe("over_here")
    })
    test('Checking Units is resolved.', () => {
        const u = new libcellml.Units()

        expect(u.isResolved()).toBe(true)

        let iS = new libcellml.ImportSource()
        iS.setUrl("someplace")

        u.setImportSource(iS)
        u.setImportReference("this_thing")
        expect(u.isResolved()).toBe(false)
    })
    test('Checking Units is defined.', () => {
        const u = new libcellml.Units()

        expect(u.isDefined()).toBe(true)
    })
    test('Checking Units scaling factor.', () => {
        const u1 = new libcellml.Units("second")
        let u2 = new libcellml.Units("millisecond")
        u2.addUnitByReferenceStringPrefix("second", "milli", 1.0, 1.0, "")

        expect(libcellml.Units.scalingFactor(u1, u2, true)).toBe(0.001)
    })
    test('Checking Units compatible.', () => {
        const u1 = new libcellml.Units("second")
        let u2 = new libcellml.Units("millisecond")
        u2.addUnitByReferenceStringPrefix("second", "milli", 1.0, 1.0, "")

        expect(libcellml.Units.compatible(u1, u2)).toBe(true)
    })
    test('Checking Units equivalent.', () => {
        const u1 = new libcellml.Units("second")
        let u2 = new libcellml.Units("millisecond")
        u2.addUnitByReferenceStringPrefix("second", "milli", 1.0, 1.0, "")

        expect(libcellml.Units.equivalent(u1, u2)).toBe(false)
    })
})
