#
# Tests the Reset class bindings
#
import unittest


class ResetTestCase(unittest.TestCase):

    def test_create_destroy(self):
        from libcellml import Reset

        x = Reset()
        del x

    def test_set_get_variable(self):
        from libcellml import Reset
        from libcellml import Variable

        r = Reset()
        v = Variable()
        v.setName("glucose")

        self.assertEqual(None, r.variable())

        r.setVariable(v)

        self.assertEqual("glucose", r.variable().name())

    def test_set_get_test_variable(self):
        from libcellml import Reset
        from libcellml import Variable

        r = Reset()
        v = Variable()
        v.setName("glucose")

        self.assertEqual(None, r.testVariable())

        r.setTestVariable(v)

        self.assertEqual("glucose", r.testVariable().name())

    def test_set_order(self):
        from libcellml import Reset

        x = Reset()
        x.setOrder(3)
        self.assertEqual(3, x.order())

    def test_is_order_set(self):
        from libcellml import Reset

        # bool isOrderSet()
        x = Reset()
        self.assertFalse(x.isOrderSet())
        x.setOrder(2)
        self.assertTrue(x.isOrderSet())

    def test_unset_order(self):
        from libcellml import Reset

        # removeOrder()
        x = Reset()
        x.setOrder(-4)
        self.assertTrue(x.isOrderSet())
        x.removeOrder()

        self.assertFalse(x.isOrderSet())
        self.assertEqual(0, x.order())

    def test_test_value(self):
        from libcellml import Reset

        x = Reset()
        self.assertEqual("", x.testValue())

        x.setTestValue("A MathML string.")
        self.assertEqual("A MathML string.", x.testValue())

        x.appendTestValue(" More MathML.")
        self.assertEqual("A MathML string. More MathML.", x.testValue())

        self.assertEqual("", x.testValueId())
        x.setTestValueId("my_test_value")
        self.assertEqual("my_test_value", x.testValueId())

        x.removeTestValue()
        self.assertEqual("", x.testValue())

        x.removeTestValueId()
        self.assertEqual("", x.testValueId())

    def test_reset_value(self):
        from libcellml import Reset

        x = Reset()
        self.assertEqual("", x.resetValue())

        x.setResetValue("A reset MathML string.")

        self.assertEqual("A reset MathML string.", x.resetValue())

        x.appendResetValue(" More MathML.")
        self.assertEqual("A reset MathML string. More MathML.", x.resetValue())

        self.assertEqual("", x.resetValueId())
        x.setResetValueId("my_reset_value")
        self.assertEqual("my_reset_value", x.resetValueId())

        x.removeResetValue()
        self.assertEqual("", x.resetValue())

        x.removeResetValueId()
        self.assertEqual("", x.resetValueId())

    def test_clone(self):
        from libcellml import Reset

        x = Reset()
        self.assertEqual("", x.resetValue())

        x.setResetValue("A reset MathML string.")
        x.setTestValue("A test MathML string.")

        rCloned = x.clone()

        self.assertEqual("A reset MathML string.", rCloned.resetValue())
        self.assertEqual("A test MathML string.", rCloned.testValue())


if __name__ == '__main__':
    unittest.main()
