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

describe("Component tests", () => {
    beforeAll(async () => {
        libcellml = await libCellMLModule()
    })
    test('Checking Component constructors.', () => {
        const c1 = new libcellml.Component()
        expect(c1.componentCount()).toBe(0)
        expect(c1.name()).toBe("")

        const c2 = new libcellml.Component("dave")
        expect(c2.componentCount()).toBe(0)
        expect(c2.name()).toBe("dave")
    })
    test('Checking Component set source component.', () => {
        const c = new libcellml.Component()
        const iS = new libcellml.ImportSource()

        expect(c.requiresImports()).toBe(false)

        c.setSourceComponent(iS, "imported_component_name")
        expect(c.requiresImports()).toBe(true)
    })
    test('Checking Component component count.', () => {
        const c = new libcellml.Component()
        const c1 = new libcellml.Component()

        expect(c.componentCount()).toBe(0)

        c.addComponent(c1)
        expect(c.componentCount()).toBe(1)
    })
    test('Checking Component math.', () => {
        const c = new libcellml.Component()

        expect(c.math()).toBe("")

        c.setMath("some_math")
        expect(c.math()).toBe("some_math")

        c.appendMath("_and_more_math")
        expect(c.math()).toBe("some_math_and_more_math")

        c.removeMath()
        expect(c.math()).toBe("")
    })
    test('Checking Component add and get variable.', () => {
        const c = new libcellml.Component()
        const v = new libcellml.Variable()

        c.addVariable(v)
        expect(c.variableCount()).toBe(1)
    })
    test('Checking Component remove variable.', () => {
        const c = new libcellml.Component()
        const v1 = new libcellml.Variable()
        const v2 = new libcellml.Variable()
        const v3 = new libcellml.Variable()

        v1.setName("var1");
        v2.setName("var2");
        v3.setName("var3");

        expect(c.variableCount()).toBe(0)

        c.addVariable(v1);
        c.addVariable(v2);
        c.addVariable(v3);

        expect(c.variableCount()).toBe(3)

        c.removeVariableByIndex(1);

        expect(c.variableCount()).toBe(2)

        c.removeVariableByName("var1");

        expect(c.variableCount()).toBe(1)

        c.removeVariableByVariable(v3);

        expect(c.variableCount()).toBe(0)
    })
    test('Checking Component remove all variables.', () => {
        const c = new libcellml.Component()
        const v1 = new libcellml.Variable()
        const v2 = new libcellml.Variable()
        const v3 = new libcellml.Variable()

        v1.setName("var1");
        v2.setName("var2");
        v3.setName("var3");

        expect(c.variableCount()).toBe(0)

        c.addVariable(v1);
        c.addVariable(v2);
        c.addVariable(v3);

        expect(c.variableCount()).toBe(3)

        c.removeAllVariables()

        expect(c.variableCount()).toBe(0)
    })
    test('Checking Component take variable.', () => {
        const c = new libcellml.Component()
        const v1 = new libcellml.Variable()
        const v2 = new libcellml.Variable()
        const v3 = new libcellml.Variable()

        v1.setName("var1");
        v2.setName("var2");
        v3.setName("var3");

        expect(c.variableCount()).toBe(0)

        c.addVariable(v1);
        c.addVariable(v2);
        c.addVariable(v3);

        const vTaken1 = c.takeVariableByIndex(1)
        expect(vTaken1.name()).toBe("var2")

        const vTaken2 = c.takeVariableByName("var3");
        expect(vTaken2.name()).toBe("var3");
    })
    test('Checking Component has variable.', () => {
        const c = new libcellml.Component()
        const v1 = new libcellml.Variable()
        const v2 = new libcellml.Variable()
        const v3 = new libcellml.Variable()

        v1.setName("var1");
        v2.setName("var2");
        v3.setName("var3");

        expect(c.variableCount()).toBe(0)

        c.addVariable(v1);
        c.addVariable(v2);
        c.addVariable(v3);

        expect(c.hasVariableByName("notvar2")).toBe(false)
        expect(c.hasVariableByName("var2")).toBe(true)
        expect(c.hasVariableByVariable(v3)).toBe(true)

    })
    test('Checking Component add reset.', () => {
        const c = new libcellml.Component()
        const r = new libcellml.Reset()

        expect(c.resetCount()).toBe(0)

        c.addReset(r)
        expect(c.resetCount()).toBe(1)
    })
    test('Checking Component take reset.', () => {
        const c = new libcellml.Component()
        const r = new libcellml.Reset()
        r.setOrder(3)

        expect(c.resetCount()).toBe(0)

        c.addReset(r)
        expect(c.resetCount()).toBe(1)

        const rTaken = c.takeReset(0)
        expect(rTaken.order()).toBe(3)
    })
    test('Checking Component remove reset.', () => {
        const c = new libcellml.Component()
        const r1 = new libcellml.Reset()
        const r2 = new libcellml.Reset()

        expect(c.resetCount()).toBe(0)

        c.addReset(r1)
        c.addReset(r2)

        expect(c.resetCount()).toBe(2)

        c.removeResetByIndex(1)
        expect(c.resetCount()).toBe(1)

        c.removeResetByReset(r1)
        expect(c.resetCount()).toBe(0)

    })
    test('Checking Component remove all resets.', () => {
        const c = new libcellml.Component()
        const r1 = new libcellml.Reset()
        const r2 = new libcellml.Reset()

        expect(c.resetCount()).toBe(0)

        c.addReset(r1)
        c.addReset(r2)
        expect(c.resetCount()).toBe(2)

        c.removeAllResets()
        expect(c.resetCount()).toBe(0)
    })
    test('Checking Component has reset.', () => {
        const c = new libcellml.Component()
        const r1 = new libcellml.Reset()
        const r2 = new libcellml.Reset()

        r1.setOrder(2)
        r2.setOrder(5)

        expect(c.resetCount()).toBe(0)

        c.addReset(r1)
        expect(c.hasReset(r1)).toBe(true)
        expect(c.hasReset(r2)).toBe(false)
    })
    test('Checking Component clone.', () => {
        const c = new libcellml.Component("mine")
        let iS = new libcellml.ImportSource()
        iS.setUrl("someplace")

        c.setMath("some_math")
        c.setImportSource(iS)

        const cClone = c.clone()
        expect(cClone.name()).toBe("mine")
        expect(cClone.math()).toBe("some_math")
        expect(cClone.importSource().url()).toBe("someplace")
    })
    test('Checking Component import source.', () => {
        const c = new libcellml.Component()
        let iS = new libcellml.ImportSource()
        iS.setUrl("someplace")

        expect(c.isImport()).toBe(false)

        c.setImportSource(iS)
        expect(c.isImport()).toBe(true)
        expect(c.importSource().url()).toBe("someplace")
    })
    test('Checking Component import reference.', () => {
        const c = new libcellml.Component()

        expect(c.importReference()).toBe("")

        c.setImportReference("over_here")
        expect(c.importReference()).toBe("over_here")
    })
    test('Checking Component is resolved.', () => {
        const c = new libcellml.Component()

        expect(c.isResolved()).toBe(true)

        let iS = new libcellml.ImportSource()
        iS.setUrl("someplace")

        c.setImportSource(iS)
        c.setImportReference("this_thing")
        expect(c.isResolved()).toBe(false)
    })
    test('Checking Component is defined.', () => {
        const c = new libcellml.Component()

        expect(c.isDefined()).toBe(true)
    })
})
