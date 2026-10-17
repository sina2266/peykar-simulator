#
# Tests the Analyser class bindings
#
import unittest


class AnalyserTestCase(unittest.TestCase):
    VALUE = 'value'

    def test_create_destroy(self):
        from libcellml import Analyser

        x = Analyser()
        del x

    def test_inheritance(self):
        import libcellml
        from libcellml import Analyser

        x = Analyser()
        self.assertIsInstance(x, libcellml.logger.Logger)

        # Test access to inherited methods.
        self.assertIsNone(x.issue(0))
        self.assertIsNone(x.issue(-1))
        self.assertEqual(x.issueCount(), 0)

    def test_analyse_model(self):
        from libcellml import Analyser
        from libcellml import AnalyserModel
        from libcellml import Model
        from libcellml.analysermodel import AnalyserModel_typeAsString

        # Analyse an empty model and make sure that we get no errors and an
        # UNKNOWN type for the analyser model.

        m = Model('my_model')
        a = Analyser()

        a.analyseModel(m)

        self.assertEqual(0, a.errorCount())
        self.assertEqual(AnalyserModel.Type.UNKNOWN, a.analyserModel().type())
        self.assertEqual("unknown", AnalyserModel.typeAsString(a.analyserModel().type()))
        self.assertEqual("unknown", AnalyserModel_typeAsString(a.analyserModel().type()))

    def test_coverage(self):
        from libcellml import Analyser
        from libcellml import AnalyserEquation
        from libcellml import AnalyserEquationAst
        from libcellml import AnalyserExternalVariable
        from libcellml import AnalyserModel
        from libcellml import AnalyserVariable
        from libcellml import Model
        from libcellml import Parser
        from libcellml.analyserequation import AnalyserEquation_typeAsString
        from libcellml.analyserequationast import AnalyserEquationAst_typeAsString
        from libcellml.analyservariable import AnalyserVariable_typeAsString
        from test_resources import file_contents

        # Try to create an analyser equation/model/variable, something that is not allowed.

        self.assertRaises(AttributeError, AnalyserEquation)
        self.assertRaises(AttributeError, AnalyserModel)
        self.assertRaises(AttributeError, AnalyserVariable)

        # Analyse a model, so we can then do some coverage.

        p = Parser()
        m = p.parseModel(file_contents('generator/noble_model_1962/model.cellml'))

        a = Analyser()
        a.analyseModel(m)

        # Ensure coverage for Analyser.

        c = m.component(1)
        v0 = c.variable(0)

        aev = AnalyserExternalVariable(v0)

        self.assertTrue(a.addExternalVariable(aev))

        self.assertTrue(a.containsExternalVariable(aev))
        self.assertTrue(a.containsExternalVariable(v0))

        self.assertEqual(aev.variable().name(), a.externalVariable(0).variable().name())
        self.assertEqual(aev.variable().name(), a.externalVariable(v0).variable().name())

        v2 = c.variable(2)

        self.assertTrue(a.addExternalVariable(AnalyserExternalVariable(c.variable(1))))
        self.assertTrue(a.addExternalVariable(AnalyserExternalVariable(v2)))
        self.assertTrue(a.addExternalVariable(AnalyserExternalVariable(c.variable(3))))

        self.assertEqual(4, a.externalVariableCount())

        self.assertTrue(a.removeExternalVariable(1))
        self.assertTrue(a.removeExternalVariable(aev))
        self.assertTrue(a.removeExternalVariable(v2))

        a.removeAllExternalVariables()

        # Ensure coverage for AnalyserModel.

        am = a.analyserModel()

        self.assertTrue(am.isValid())

        self.assertFalse(am.hasExternalVariables())

        self.assertIsNotNone(am.voi())

        self.assertEqual(4, am.stateCount())
        self.assertIsNotNone(am.states())
        self.assertIsNotNone(am.state(3))

        self.assertEqual(5, am.constantCount())
        self.assertIsNotNone(am.constants())
        self.assertIsNotNone(am.constant(3))
        self.assertEqual(0, am.computedConstantCount())
        self.assertIsNotNone(am.computedConstants())
        self.assertIsNone(am.computedConstant(3))
        self.assertEqual(12, am.algebraicVariableCount())
        self.assertIsNotNone(am.algebraicVariables())
        self.assertIsNotNone(am.algebraicVariable(3))
        self.assertIsNone(am.analyserVariable(None))
        self.assertIsNotNone(am.analyserVariable(m.component("membrane").variable("V")))

        self.assertEqual(16, am.analyserEquationCount())
        self.assertIsNotNone(am.analyserEquations())
        self.assertIsNotNone(am.analyserEquation(3))

        self.assertFalse(am.needEqFunction())
        self.assertFalse(am.needNeqFunction())
        self.assertFalse(am.needLtFunction())
        self.assertFalse(am.needLeqFunction())
        self.assertFalse(am.needGtFunction())
        self.assertFalse(am.needGeqFunction())
        self.assertFalse(am.needAndFunction())
        self.assertFalse(am.needOrFunction())
        self.assertFalse(am.needXorFunction())
        self.assertFalse(am.needNotFunction())
        self.assertFalse(am.needMinFunction())
        self.assertFalse(am.needMaxFunction())
        self.assertFalse(am.needSecFunction())
        self.assertFalse(am.needCscFunction())
        self.assertFalse(am.needCotFunction())
        self.assertFalse(am.needSechFunction())
        self.assertFalse(am.needCschFunction())
        self.assertFalse(am.needCothFunction())
        self.assertFalse(am.needAsecFunction())
        self.assertFalse(am.needAcscFunction())
        self.assertFalse(am.needAcotFunction())
        self.assertFalse(am.needAsechFunction())
        self.assertFalse(am.needAcschFunction())
        self.assertFalse(am.needAcothFunction())

        self.assertTrue(am.areEquivalentVariables(am.voi().variable(), am.voi().variable()))

        # Ensure coverage for AnalyserVariable.

        av = am.algebraicVariable(3)

        self.assertEqual(AnalyserVariable.Type.ALGEBRAIC_VARIABLE, av.type())
        self.assertEqual("algebraic_variable", AnalyserVariable.typeAsString(av.type()))
        self.assertEqual("algebraic_variable", AnalyserVariable_typeAsString(av.type()))
        self.assertEqual(3, av.index())
        self.assertIsNone(av.initialisingVariable())
        self.assertIsNotNone(av.variable())
        self.assertIsNotNone(av.analyserModel())
        self.assertEqual(1, av.analyserEquationCount())
        self.assertIsNotNone(av.analyserEquations())
        self.assertIsNotNone(av.analyserEquation(0))

        # Ensure coverage for AnalyserEquation.

        ae = am.analyserEquation(3)

        self.assertEqual(AnalyserEquation.Type.ALGEBRAIC, ae.type())
        self.assertEqual("algebraic", AnalyserEquation.typeAsString(ae.type()))
        self.assertEqual("algebraic", AnalyserEquation_typeAsString(ae.type()))
        self.assertIsNotNone(ae.ast())
        self.assertEqual(2, ae.dependencyCount())
        self.assertIsNotNone(ae.dependencies())
        self.assertIsNotNone(ae.dependency(0))
        self.assertEqual(18446744073709551615, ae.nlaSystemIndex())
        self.assertEqual(0, ae.nlaSiblingCount())
        self.assertIsNotNone(ae.nlaSiblings())
        self.assertIsNone(ae.nlaSibling(0))
        self.assertTrue(ae.isStateRateBased())
        self.assertEqual(0, ae.computedConstantCount())
        self.assertIsNotNone(ae.computedConstants())
        self.assertIsNone(ae.computedConstant(0))
        self.assertEqual(1, ae.algebraicVariableCount())
        self.assertIsNotNone(ae.algebraicVariables())
        self.assertIsNotNone(ae.algebraicVariable(0))
        self.assertEqual(0, ae.externalVariableCount())
        self.assertIsNotNone(ae.externalVariables())
        self.assertIsNone(ae.externalVariable(0))

        # Check Analyser Equation type with invalid values.

        self.assertRaises(RuntimeError, AnalyserEquation.typeAsString, -1)
        self.assertRaises(RuntimeError, AnalyserEquation_typeAsString, -1)
        self.assertRaises(RuntimeError, AnalyserEquation.typeAsString, 999)
        self.assertRaises(RuntimeError, AnalyserEquation_typeAsString, 999)

        # Ensure coverage for AnalyserEquationAst.

        aea = ae.ast()

        self.assertEqual(AnalyserEquationAst.Type.EQUALITY, aea.type())
        self.assertEqual("equality", AnalyserEquationAst.typeAsString(aea.type()))
        self.assertEqual("equality", AnalyserEquationAst_typeAsString(aea.type()))
        self.assertEqual('', aea.value())
        self.assertIsNone(aea.variable())
        self.assertIsNone(aea.parent())
        self.assertIsNotNone(aea.leftChild())
        self.assertIsNotNone(aea.rightChild())

        aea.swapLeftAndRightChildren()
        aea.setType(AnalyserEquationAst.Type.EQ)
        aea.setValue(AnalyserTestCase.VALUE)
        aea.setVariable(av.variable())
        aea.setParent(aea)
        aea.setLeftChild(None)
        aea.setRightChild(None)

        self.assertEqual(AnalyserEquationAst.Type.EQ, aea.type())
        self.assertEqual("eq", AnalyserEquationAst.typeAsString(aea.type()))
        self.assertEqual("eq", AnalyserEquationAst_typeAsString(aea.type()))
        self.assertEqual(AnalyserTestCase.VALUE, aea.value())
        self.assertIsNotNone(aea.variable())
        self.assertIsNotNone(aea.parent())
        self.assertIsNone(aea.leftChild())
        self.assertIsNone(aea.rightChild())


if __name__ == '__main__':
    unittest.main()
