#
# Tests the Generator class bindings
#
import unittest


class GeneratorTestCase(unittest.TestCase):
    def test_create_destroy(self):
        from libcellml import Generator

        x = Generator()
        del x

    def test_algebraic_eqn_computed_var_on_rhs(self):
        from libcellml import Analyser
        from libcellml import AnalyserModel
        from libcellml import Generator
        from libcellml.generator import Generator_equationCode
        from libcellml import GeneratorProfile
        from libcellml import GeneratorVariableTracker
        from libcellml import Parser
        from test_resources import file_contents

        p = Parser()
        m = p.parseModel(file_contents('generator/algebraic_eqn_computed_var_on_rhs/model.cellml'))

        a = Analyser()
        a.analyseModel(m)

        am = a.analyserModel()

        self.assertEqual(AnalyserModel.Type.ALGEBRAIC, am.type())

        g = Generator()

        self.assertEqual(file_contents("generator/algebraic_eqn_computed_var_on_rhs/model.h"), g.interfaceCode(am))
        self.assertEqual(file_contents("generator/algebraic_eqn_computed_var_on_rhs/model.c"), g.implementationCode(am))

        profile = GeneratorProfile(GeneratorProfile.Profile.PYTHON)

        self.assertEqual("", g.interfaceCode(am, profile))
        self.assertEqual(file_contents("generator/algebraic_eqn_computed_var_on_rhs/model.py"), g.implementationCode(am, profile))

        self.assertEqual("", g.interfaceCode(am, GeneratorProfile.Profile.PYTHON))
        self.assertEqual(file_contents("generator/algebraic_eqn_computed_var_on_rhs/model.py"), g.implementationCode(am, GeneratorProfile.Profile.PYTHON))

        variable_tracker = GeneratorVariableTracker()

        self.assertEqual("", g.interfaceCode(am, profile, variable_tracker))
        self.assertEqual(file_contents("generator/algebraic_eqn_computed_var_on_rhs/model.py"), g.implementationCode(am, profile, variable_tracker))

        self.assertEqual("x = a", Generator.equationCode(am.analyserEquation(0).ast()))
        self.assertEqual("x = a", Generator_equationCode(am.analyserEquation(0).ast()))


if __name__ == '__main__':
    unittest.main()
