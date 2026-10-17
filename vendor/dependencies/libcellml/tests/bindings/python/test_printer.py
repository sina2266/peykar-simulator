#
# Tests the Printer class bindings
#
import unittest


class PrinterTestCase(unittest.TestCase):

    def test_create_destroy(self):
        from libcellml import Printer

        x = Printer()
        del x

    def test_print_model(self):
        from libcellml import Printer, Model

        # std::string printModel(ModelPtr model)
        p = Printer()
        self.assertIsInstance(p.printModel(Model()), str)


if __name__ == '__main__':
    unittest.main()
