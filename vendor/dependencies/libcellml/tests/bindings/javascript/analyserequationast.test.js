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

describe("Analyser Equation AST tests", () => {
    beforeAll(async () => {
        libcellml = await loadLibCellML()
    });
    test('Checking Analyser Equation AST type.', () => {
        const aea = new libcellml.AnalyserEquationAst()

        expect(aea.type().value).toBe(libcellml.AnalyserEquationAst.Type.EQUALITY.value)
        expect(libcellml.AnalyserEquationAst.typeAsString(aea.type())).toBe("equality")
        expect(aea.type()).toStrictEqual(libcellml.AnalyserEquationAst.Type.EQUALITY)
        expect(aea.type()).not.toStrictEqual(libcellml.AnalyserEquationAst.Type.OTHERWISE)

        aea.setType(libcellml.AnalyserEquationAst.Type.OTHERWISE)

        expect(aea.type().value).toBe(libcellml.AnalyserEquationAst.Type.OTHERWISE.value)
        expect(libcellml.AnalyserEquationAst.typeAsString(aea.type())).toBe("otherwise")
    });
    test('Checking Analyser Equation AST value.', () => {
        const aea = new libcellml.AnalyserEquationAst()

        expect(aea.value()).toBe("")

        aea.setValue("new value")

        expect(aea.value()).toBe("new value")
    });
    test('Checking Analyser Equation AST variable.', () => {
        const aea = new libcellml.AnalyserEquationAst()
        const v = new libcellml.Variable()
        v.setName("x")

        expect(aea.variable()).toBeNull()

        aea.setVariable(v)

        expect(aea.variable()).not.toBeNull()
        expect(aea.variable().name()).toBe("x")
    });
    test('Checking Analyser Equation AST tree manipulations.', () => {
        const aea = new libcellml.AnalyserEquationAst()

        aea.swapLeftAndRightChildren()

        const paea = new libcellml.AnalyserEquationAst()
        const laea = new libcellml.AnalyserEquationAst()
        const raea = new libcellml.AnalyserEquationAst()

        paea.setValue("parent value")
        laea.setValue("left child value")
        raea.setValue("right child value")

        expect(aea.parent()).toBeNull()
        expect(aea.leftChild()).toBeNull()
        expect(aea.rightChild()).toBeNull()

        aea.setParent(paea)

        expect(aea.parent()).not.toBeNull()
        expect(aea.parent().value()).toBe("parent value")

        aea.setLeftChild(laea)

        expect(aea.leftChild()).not.toBeNull()
        expect(aea.leftChild().value()).toBe("left child value")

        aea.setRightChild(raea)

        expect(aea.rightChild()).not.toBeNull()
        expect(aea.rightChild().value()).toBe("right child value")
    });
 })
