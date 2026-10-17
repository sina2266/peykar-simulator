#
# Tests the Parser class bindings
#
import unittest


class ParserTestCase(unittest.TestCase):

    def test_create_destroy(self):
        from libcellml import Parser

        x = Parser()
        del x

    def test_create_strict(self):
        from libcellml.strict import Strict

        self.assertRaises(AttributeError, Strict)

    def test_parser_strict_interface(self):
        from libcellml import Parser

        x = Parser()
        self.assertTrue(x.isStrict())
        x.setStrict(False)
        self.assertFalse(x.isStrict())

    def test_inheritance(self):
        import libcellml
        from libcellml import Parser

        x = Parser()
        self.assertIsInstance(x, libcellml.logger.Logger)

        # Test access to inherited methods
        self.assertIsNone(x.issue(0))
        self.assertIsNone(x.issue(-1))
        self.assertEqual(x.issueCount(), 0)

    def test_parse_model(self):
        import libcellml
        from libcellml import Parser

        model_string = """<?xml version="1.0" encoding="iso-8859-1"?>
        <model name="sin" xmlns="http://www.cellml.org/cellml/2.0#">
        </model>
        """

        p = Parser()
        m = p.parseModel(model_string)
        self.assertIsInstance(m, libcellml.Model)
        self.assertEqual("sin", m.name())

    def test_parse_permissive_model(self):
        import libcellml
        from libcellml import Parser

        model_string = """<?xml version="1.0" encoding="iso-8859-1"?>
        <model name="sin" xmlns="http://www.cellml.org/cellml/1.0#">
        </model>
        """

        p = Parser(False)
        m = p.parseModel(model_string)
        self.assertIsInstance(m, libcellml.Model)
        self.assertEqual("sin", m.name())


if __name__ == '__main__':
    unittest.main()
