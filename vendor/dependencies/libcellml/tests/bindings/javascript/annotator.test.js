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
const { duplicatedIdsModel } = require('./resources')

let libcellml = null

describe("Annotator tests", () => {
    let m
    let a

    beforeAll(async () => {
        libcellml = await loadLibCellML()
    });
    beforeEach(() => {
        const p = new libcellml.Parser(true)
        m = p.parseModel(duplicatedIdsModel)
        a = new libcellml.Annotator()

        p.delete()
    });
    afterEach(() => {
        a.delete()
    });
    test('Checking Annotator model.', () => {
        expect(a.hasModel()).toBe(false)
        expect(a.model()).toBeNull()

        a.setModel(m)

        expect(a.hasModel()).toBe(true)
        expect(a.model()).not.toBeNull()
    });
    test('Checking Annotator assigning all.', () => {
        a.setModel(m)

        expect(a.itemCount("duplicateId2")).toBe(8)
        expect(a.isUnique("duplicateId2")).toBe(false)

        a.clearAllIds()

        expect(a.itemCount("duplicateId2")).toBe(0)

        expect(a.assignAllIds()).toBe(true)

        expect(a.itemCount("b4da55")).toBe(1)
        expect(a.assignAllIdsByModel(m)).toBe(false)

        a.clearAllIdsByModel(m)

        expect(a.itemCount("b4da55")).toBe(0)
    });
    test('Checking Annotator assigning by type.', () => {
        a.setModel(m)
        a.clearAllIds()

        const reset_type = libcellml.CellmlElementType.RESET
        expect(a.assignIds(reset_type)).toBe(true)

        expect(a.item("b4da55").type().value).toBe(libcellml.CellmlElementType.RESET.value)
        expect(a.itemByIndex("b4da55", 0).type().value).toBe(libcellml.CellmlElementType.RESET.value)
    });
    test('Checking Annotator assign id by class.', () => {
        a.setModel(m)
        a.clearAllIds()

        const v1 = m.componentByName("component2", true).variableByName("variable1")
        const v2 = m.componentByName("component3", true).variableByName("variable1")

        const v3 = m.componentByName("component2", true).variableByName("variable2")
        const v4 = m.componentByName("component3", true).variableByName("variable2")
        const vp = new libcellml.VariablePair(v1, v2)

        const ui = new libcellml.UnitsItem(m.unitsByIndex(0), 0)

        const importSource = m.componentByName("component1", true).importSource()

        expect(a.assignIdByModel(m, libcellml.CellmlElementType.MODEL)).toBe("b4da55")
        expect(a.assignIdByComponent(m.componentByIndex(0), libcellml.CellmlElementType.COMPONENT)).toBe("b4da56")
        expect(a.assignIdByImportSource(importSource)).toBe("b4da57")
        expect(a.assignIdByReset(m.componentByName("component2", true).reset(0), libcellml.CellmlElementType.TEST_VALUE)).toBe("b4da58")
        expect(a.assignIdByUnits(m.unitsByIndex(0))).toBe("b4da59")
        expect(a.assignIdByUnitsItem(ui)).toBe("b4da5a")
        expect(a.assignIdByVariable(v1)).toBe("b4da5b")
        expect(a.assignIdByVariablePair(vp, libcellml.CellmlElementType.CONNECTION)).toBe("b4da5c")
        expect(a.assignIdByVariableVariable(v3, v4, libcellml.CellmlElementType.MAP_VARIABLES)).toBe("b4da5d")
        expect(a.assignIdByUnitsIndex(m.unitsByIndex(0), 0)).toBe("b4da5e")

        vp.delete()
        ui.delete()
    });
    test('Checking Annotator ids.', () => {
        a.setModel(m)

        expect(a.ids().size()).toBe(4)
        expect(a.duplicateIds().size()).toBe(4)
        expect(a.items("duplicateId3").size()).toBe(7)
    });
    test('Checking Annotator get annotation by type.', () => {
        a.setModel(m)
        a.clearAllIds()

        a.assignAllIds()

        expect(a.component("b4da5e").name()).toBe("component4")
        expect(a.componentByIndex("b4da5e", 0).name()).toBe("component4")

        expect(a.componentEncapsulation("b4da65").name()).toBe("component2")
        expect(a.componentEncapsulationByIndex("b4da65", 0).name()).toBe("component2")

        expect(a.connection("b4da61").isValid()).toBe(true)
        expect(a.connectionByIndex("b4da61", 0).isValid()).toBe(true)

        expect(a.encapsulation("b4da72").name()).toBe("everything")
        expect(a.encapsulationByIndex("b4da72", 0).name()).toBe("everything")

        expect(a.importSource("b4da56").url()).toBe("some-other-model.xml")
        expect(a.importSourceByIndex("b4da56", 0).url()).toBe("some-other-model.xml")

        expect(a.mapVariables("b4da62").isValid()).toBe(true)
        expect(a.mapVariablesByIndex("b4da62", 0).isValid()).toBe(true)

        expect(a.modelById("b4da55").name()).toBe("everything")
        expect(a.modelByIdIndex("b4da55", 0).name()).toBe("everything")

        expect(a.reset("b4da6b").order()).toBe(1)
        expect(a.resetByIndex("b4da6b", 0).order()).toBe(1)

        expect(a.resetValue("b4da6c").variable().name()).toBe("variable1")
        expect(a.resetValueByIndex("b4da6c", 0).variable().name()).toBe("variable1")

        expect(a.testValue("b4da6d").testVariable().name()).toBe("variable2")
        expect(a.testValueByIndex("b4da6d", 0).testVariable().name()).toBe("variable2")

        expect(a.unitsItem("b4da5c").index()).toBe(0)
        expect(a.unitsItemByIndex("b4da5c", 0).index()).toBe(0)

        expect(a.units("b4da58").name()).toBe("units1")
        expect(a.unitsByIndex("b4da58", 0).name()).toBe("units1")

        expect(a.variable("b4da60").name()).toBe("variable2")
        expect(a.variableByIndex("b4da60", 0).name()).toBe("variable2")
    });
 })
