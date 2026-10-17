#
# Tests the AnalyserExternalVariable class bindings
#
import unittest


class AnalyserExternalVariableTestCase(unittest.TestCase):

    def test_create_destroy(self):
        from libcellml import AnalyserExternalVariable
        from libcellml import Component
        from libcellml import Model
        from libcellml import Variable

        m = Model('model')
        c = Component('component')
        v = Variable('test')

        self.assertTrue(m.addComponent(c))
        self.assertTrue(c.addVariable(v))

        aev = AnalyserExternalVariable(v)

        self.assertEqual(v.name(), aev.variable().name())

        d0 = Variable('d0')
        d1 = Variable('d1')
        d2 = Variable('d2')
        d3 = Variable('d3')

        c.addVariable(d0)
        c.addVariable(d1)
        c.addVariable(d2)
        c.addVariable(d3)

        self.assertTrue(aev.addDependency(d0))
        self.assertTrue(aev.addDependency(d1))
        self.assertTrue(aev.addDependency(d2))
        self.assertTrue(aev.addDependency(d3))

        self.assertEqual(4, aev.dependencyCount())

        self.assertIsNotNone(aev.dependency(0))
        self.assertIsNotNone(aev.dependency(m, c.name(), d1.name()))

        self.assertIsNotNone(aev.dependencies())

        self.assertTrue(aev.containsDependency(d0))
        self.assertTrue(aev.containsDependency(m, c.name(), d1.name()))

        self.assertTrue(aev.removeDependency(1))
        self.assertTrue(aev.removeDependency(d0))
        self.assertTrue(aev.removeDependency(m, c.name(), d2.name()))

        aev.removeAllDependencies()


if __name__ == '__main__':
    unittest.main()
