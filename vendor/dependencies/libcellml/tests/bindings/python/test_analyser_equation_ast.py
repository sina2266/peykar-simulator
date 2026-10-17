#
# Tests the AnalyserEquationAst class bindings
#
import unittest


class AnalyserEquationAstTestCase(unittest.TestCase):

    def test_create_destroy(self):
        from libcellml import AnalyserEquationAst

        x = AnalyserEquationAst()
        del x

    def test_analyser_equation_ast(self):
        from libcellml import AnalyserEquationAst

        # Create an equation AST and check its default settings.
        ast = AnalyserEquationAst()
        self.assertEqual(AnalyserEquationAst.Type.EQUALITY, ast.type())
        self.assertEqual('', ast.value())
        self.assertIsNone(ast.variable())
        self.assertIsNone(ast.parent())
        self.assertIsNone(ast.leftChild())
        self.assertIsNone(ast.rightChild())


if __name__ == '__main__':
    unittest.main()
