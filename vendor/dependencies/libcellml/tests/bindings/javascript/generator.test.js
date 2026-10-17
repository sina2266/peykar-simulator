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
const { basicModel } = require('./resources')

let libcellml = null

describe("Generator tests", () => {
    beforeAll(async () => {
        libcellml = await loadLibCellML()
    })
    test('Checking Generator profile manipulation.', () => {
        const gp = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

        expect(gp.commentString()).toBe("/* [CODE] */\n")

        gp.setProfile(libcellml.GeneratorProfile.Profile.PYTHON)

        expect(gp.commentString()).toBe("# [CODE]\n")
    })
    test('Checking Generator code generation.', () => {
        const g = new libcellml.Generator()
        const p = new libcellml.Parser(true)

        m = p.parseModel(basicModel)
        a = new libcellml.Analyser()

        a.analyseModel(m)

        am = a.analyserModel()

        const interface_lines = g.interfaceCode(am).split('\n')
        expect(interface_lines.length).toBe(38)

        const gp = new libcellml.GeneratorProfile(libcellml.GeneratorProfile.Profile.C)

        const interface_lines_profile = g.interfaceCodeByProfile(am, gp).split('\n')
        expect(interface_lines_profile.length).toBe(38)

        const interface_lines_profile_enumeration = g.interfaceCodeByProfileEnumeration(am, libcellml.GeneratorProfile.Profile.C).split('\n')
        expect(interface_lines_profile_enumeration.length).toBe(38)

        const gvt = new libcellml.GeneratorVariableTracker()

        const interface_lines_variable_tracker = g.interfaceCodeByVariableTracker(am, gvt).split('\n')
        expect(interface_lines_variable_tracker.length).toBe(38)

        const interface_lines_profile_variable_tracker = g.interfaceCodeByProfileAndVariableTracker(am, gp, gvt).split('\n')
        expect(interface_lines_profile_variable_tracker.length).toBe(38)

        const implementation_lines = g.implementationCode(am).split('\n')
        expect(implementation_lines.length).toBe(97)

        const implementation_lines_profile = g.implementationCodeByProfile(am, gp).split('\n')
        expect(implementation_lines_profile.length).toBe(97)

        const implementation_lines_profile_enumeration = g.implementationCodeByProfileEnumeration(am, libcellml.GeneratorProfile.Profile.C).split('\n')
        expect(implementation_lines_profile_enumeration.length).toBe(97)

        const implementation_lines_variable_tracker = g.implementationCodeByVariableTracker(am, gvt).split('\n')
        expect(implementation_lines_variable_tracker.length).toBe(97)

        const implementation_lines_profile_variable_tracker = g.implementationCodeByProfileAndVariableTracker(am, gp, gvt).split('\n')
        expect(implementation_lines_profile_variable_tracker.length).toBe(97)

        const equation_line_1 = libcellml.Generator.equationCode(a.analyserModel().analyserEquation(0).ast())
        expect(equation_line_1.length).toBe(14)

        const equation_line_2 = libcellml.Generator.equationCodeByProfile(a.analyserModel().analyserEquation(0).ast(), gp)
        expect(equation_line_2.length).toBe(14)
    })
})
