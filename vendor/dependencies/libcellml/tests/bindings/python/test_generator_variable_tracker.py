#
# Tests the GeneratorVariableTracker class bindings
#
import unittest


class GeneratorVariableTrackerTestCase(unittest.TestCase):

    def test_tracked_untracked_variables(self):
        from libcellml import Analyser
        from libcellml import GeneratorVariableTracker
        from libcellml import Parser
        from test_resources import file_contents

        p = Parser()
        m = p.parseModel(file_contents('generator/hodgkin_huxley_squid_axon_model_1952/model.cellml'))
        v = m.component("membrane").variable("Cm")
        a = Analyser()

        a.analyseModel(m)

        am = a.analyserModel()
        av = am.analyserVariable(v)
        gvt = GeneratorVariableTracker()

        gvt.untrackVariable(av)

        self.assertFalse(gvt.isTrackedVariable(av))
        self.assertTrue(gvt.isUntrackedVariable(av))

        gvt.trackVariable(av)

        self.assertTrue(gvt.isTrackedVariable(av))
        self.assertFalse(gvt.isUntrackedVariable(av))

        gvt.untrackAllConstants(am)

        self.assertEqual(0, gvt.trackedConstantCount(am))
        self.assertEqual(5, gvt.untrackedConstantCount(am))

        gvt.trackAllConstants(am)

        self.assertEqual(5, gvt.trackedConstantCount(am))
        self.assertEqual(0, gvt.untrackedConstantCount(am))

        gvt.untrackAllComputedConstants(am)

        self.assertEqual(0, gvt.trackedComputedConstantCount(am))
        self.assertEqual(3, gvt.untrackedComputedConstantCount(am))

        gvt.trackAllComputedConstants(am)

        self.assertEqual(3, gvt.trackedComputedConstantCount(am))
        self.assertEqual(0, gvt.untrackedComputedConstantCount(am))

        gvt.untrackAllAlgebraicVariables(am)

        self.assertEqual(0, gvt.trackedAlgebraicVariableCount(am))
        self.assertEqual(10, gvt.untrackedAlgebraicVariableCount(am))

        gvt.trackAllAlgebraicVariables(am)

        self.assertEqual(10, gvt.trackedAlgebraicVariableCount(am))
        self.assertEqual(0, gvt.untrackedAlgebraicVariableCount(am))

        gvt.untrackAllVariables(am)

        self.assertEqual(0, gvt.trackedVariableCount(am))
        self.assertEqual(18, gvt.untrackedVariableCount(am))

        gvt.trackAllVariables(am)

        self.assertEqual(18, gvt.trackedVariableCount(am))
        self.assertEqual(0, gvt.untrackedVariableCount(am))


if __name__ == '__main__':
    unittest.main()
