#
# Tests the Variable class bindings
#
import unittest


class VariableTestCase(unittest.TestCase):

    def test_create_destroy(self):
        from libcellml import Variable

        x = Variable()
        del x

        y = Variable("nice")
        self.assertEqual("nice", y.name())
        del y

    def test_inheritance(self):
        import libcellml
        from libcellml import Variable

        x = Variable()
        self.assertIsInstance(x, libcellml.namedentity.NamedEntity)
        self.assertIsInstance(x, libcellml.entity.Entity)

        # Test access to inherited methods
        x = Variable()
        idx = 'test'
        self.assertEqual(x.id(), '')
        x.setId(idx)
        self.assertEqual(x.id(), idx)

    def test_id(self):
        from libcellml import Variable
        from libcellml.variable import Variable_setEquivalenceMappingId, Variable_setEquivalenceConnectionId, \
            Variable_equivalenceMappingId, Variable_equivalenceConnectionId
        from libcellml.variable import Variable_removeEquivalenceMappingId, Variable_removeEquivalenceConnectionId

        v1 = Variable("v1")
        v2 = Variable("v2")
        Variable.addEquivalence(v1, v2)

        self.assertEqual("", Variable.equivalenceMappingId(v1, v2))
        Variable.setEquivalenceMappingId(v1, v2, "mapping_id")
        self.assertEqual("mapping_id", Variable.equivalenceMappingId(v1, v2))
        Variable.removeEquivalenceMappingId(v1, v2)
        self.assertEqual("", Variable.equivalenceMappingId(v1, v2))

        Variable_setEquivalenceMappingId(v1, v2, "other_mapping_id")
        self.assertEqual("other_mapping_id", Variable_equivalenceMappingId(v1, v2))
        Variable_removeEquivalenceMappingId(v1, v2)
        self.assertEqual("", Variable_equivalenceMappingId(v1, v2))

        self.assertEqual("", Variable.equivalenceConnectionId(v1, v2))
        Variable.setEquivalenceConnectionId(v1, v2, "connection_id")
        self.assertEqual("connection_id", Variable.equivalenceConnectionId(v1, v2))
        Variable.removeEquivalenceConnectionId(v1, v2)
        self.assertEqual("", Variable.equivalenceConnectionId(v1, v2))

        Variable_setEquivalenceConnectionId(v1, v2, "other_connection_id")
        self.assertEqual("other_connection_id", Variable_equivalenceConnectionId(v1, v2))
        Variable_removeEquivalenceConnectionId(v1, v2)
        self.assertEqual("", Variable_equivalenceConnectionId(v1, v2))

    def test_equivalence(self):
        from libcellml import Variable
        from libcellml.variable import Variable_addEquivalence, Variable_removeEquivalence

        v1 = Variable()
        v2 = Variable()

        self.assertFalse(v1.hasEquivalentVariable(v2))

        Variable.addEquivalence(v1, v2)
        self.assertTrue(v1.hasEquivalentVariable(v2))

        Variable.removeEquivalence(v1, v2)
        self.assertFalse(v1.hasEquivalentVariable(v2))

        Variable_addEquivalence(v1, v2)
        self.assertTrue(v1.hasEquivalentVariable(v2))

        Variable_removeEquivalence(v1, v2)
        self.assertFalse(v1.hasEquivalentVariable(v2))

    def test_remove_equivalence(self):
        from libcellml import Variable

        # static bool removeEquivalence(const VariablePtr &variable1,
        #   const VariablePtr &variable2)
        v1 = Variable()
        v2 = Variable()
        v3 = Variable()
        self.assertFalse(Variable.removeEquivalence(v1, v2))
        Variable.addEquivalence(v1, v2)
        self.assertFalse(Variable.removeEquivalence(v1, v3))
        self.assertFalse(Variable.removeEquivalence(v3, v2))
        self.assertTrue(Variable.removeEquivalence(v1, v2))
        self.assertFalse(Variable.removeEquivalence(v1, v2))
        Variable.addEquivalence(v1, v2)
        self.assertTrue(Variable.removeEquivalence(v2, v1))

    def test_remove_all_equivalences(self):
        from libcellml import Variable

        # void removeAllEquivalences()
        v1 = Variable()
        v2 = Variable()
        v3 = Variable()
        Variable.addEquivalence(v1, v2)
        Variable.addEquivalence(v1, v3)
        v2.removeAllEquivalences()
        self.assertFalse(Variable.removeEquivalence(v1, v2))
        self.assertTrue(Variable.removeEquivalence(v1, v3))

    def test_equivalent_variable(self):
        from libcellml import Variable

        # VariablePtr equivalentVariable(size_t index)
        v1 = Variable()
        v2 = Variable()
        v3 = Variable()
        self.assertIsNone(v1.equivalentVariable(0))
        self.assertIsNone(v1.equivalentVariable(1))
        self.assertIsNone(v1.equivalentVariable(-1))
        Variable.addEquivalence(v1, v2)
        Variable.addEquivalence(v1, v3)
        self.assertIsNone(v1.equivalentVariable(2))
        self.assertIsNone(v1.equivalentVariable(-1))
        self.assertIsNotNone(v1.equivalentVariable(0))
        self.assertIsNotNone(v1.equivalentVariable(1))

    def test_equivalent_variable_count(self):
        from libcellml import Variable

        # size_t equivalentVariableCount()
        v1 = Variable()
        v2 = Variable()
        v3 = Variable()
        self.assertEqual(v1.equivalentVariableCount(), 0)
        self.assertEqual(v2.equivalentVariableCount(), 0)
        self.assertEqual(v3.equivalentVariableCount(), 0)
        Variable.addEquivalence(v1, v2)
        Variable.addEquivalence(v2, v3)
        self.assertEqual(v1.equivalentVariableCount(), 1)
        self.assertEqual(v2.equivalentVariableCount(), 2)
        self.assertEqual(v3.equivalentVariableCount(), 1)

    def test_has_direct_equivalent_variable(self):
        from libcellml import Variable

        # bool hasEquivalentVariable(const VariablePtr &equivalentVariable)
        v1 = Variable()
        v2 = Variable()
        v3 = Variable()

        self.assertFalse(v1.hasEquivalentVariable(v1))
        self.assertFalse(v1.hasEquivalentVariable(v2))
        self.assertFalse(v1.hasEquivalentVariable(v3))

        self.assertFalse(v2.hasEquivalentVariable(v1))
        self.assertFalse(v2.hasEquivalentVariable(v2))
        self.assertFalse(v2.hasEquivalentVariable(v3))

        self.assertFalse(v3.hasEquivalentVariable(v1))
        self.assertFalse(v3.hasEquivalentVariable(v2))
        self.assertFalse(v3.hasEquivalentVariable(v3))

        Variable.addEquivalence(v1, v2)
        Variable.addEquivalence(v2, v3)

        self.assertFalse(v1.hasEquivalentVariable(v1))
        self.assertTrue(v1.hasEquivalentVariable(v2))
        self.assertFalse(v1.hasEquivalentVariable(v3))

        self.assertTrue(v2.hasEquivalentVariable(v1))
        self.assertFalse(v2.hasEquivalentVariable(v2))
        self.assertTrue(v2.hasEquivalentVariable(v3))

        self.assertFalse(v3.hasEquivalentVariable(v1))
        self.assertTrue(v3.hasEquivalentVariable(v2))
        self.assertFalse(v3.hasEquivalentVariable(v3))

    def test_has_equivalent_variable(self):
        from libcellml import Variable

        # bool hasEquivalentVariable(const VariablePtr &equivalentVariable)
        v1 = Variable()
        v2 = Variable()
        v3 = Variable()

        self.assertFalse(v1.hasEquivalentVariable(v1, True))
        self.assertFalse(v1.hasEquivalentVariable(v2, True))
        self.assertFalse(v1.hasEquivalentVariable(v3, True))

        self.assertFalse(v2.hasEquivalentVariable(v1, True))
        self.assertFalse(v2.hasEquivalentVariable(v2, True))
        self.assertFalse(v2.hasEquivalentVariable(v3, True))

        self.assertFalse(v3.hasEquivalentVariable(v1, True))
        self.assertFalse(v3.hasEquivalentVariable(v2, True))
        self.assertFalse(v3.hasEquivalentVariable(v3, True))

        Variable.addEquivalence(v1, v2)
        Variable.addEquivalence(v2, v3)

        self.assertFalse(v1.hasEquivalentVariable(v1, True))
        self.assertTrue(v1.hasEquivalentVariable(v2, True))
        self.assertTrue(v1.hasEquivalentVariable(v3, True))

        self.assertTrue(v2.hasEquivalentVariable(v1, True))
        self.assertFalse(v2.hasEquivalentVariable(v2, True))
        self.assertTrue(v2.hasEquivalentVariable(v3, True))

        self.assertTrue(v3.hasEquivalentVariable(v1, True))
        self.assertTrue(v3.hasEquivalentVariable(v2, True))
        self.assertFalse(v3.hasEquivalentVariable(v3, True))

    def test_set_units(self):
        from libcellml import Variable, Units

        # void setUnits(const std::string &name)
        v = Variable()
        v.setUnits('')
        v.setUnits('Hello')
        v.setUnits('')
        del v

        # void setUnits(const UnitsPtr &units)
        name = 'tiger'
        u = Units()
        u.setName(name)
        v = Variable()
        v.setUnits(u)
        self.assertEqual(name, v.units().name())

        v.removeUnits()
        self.assertIsNone(v.units())

    def test_units(self):
        from libcellml import Variable

        # std::string units()
        name = 'testo'
        v = Variable()
        self.assertEqual(v.units(), None)
        v.setUnits(name)
        self.assertEqual(v.units().name(), name)
        v.setUnits('')
        self.assertEqual(v.units().name(), '')

    def test_set_initial_value(self):
        from libcellml import Variable

        # void setInitialValue(const std::string &initialValue)
        v = Variable()
        v.setInitialValue('test1')
        self.assertEqual("test1", v.initialValue())

        # void setInitialValue(double initialValue)
        v = Variable()
        v.setInitialValue(3.0)
        self.assertEqual("3", v.initialValue())

        v.removeInitialValue()
        self.assertEqual("", v.initialValue())

        # void setInitialValue(const VariablePtr &variable)
        v1 = Variable()
        v2 = Variable()
        v1.setInitialValue(v2)
        self.assertEqual("", v1.initialValue())

    def test_initial_value(self):
        from libcellml import Variable

        # std::string initialValue()
        value = '5 + x'
        v = Variable()
        self.assertEqual(v.initialValue(), '')
        v.setInitialValue(value)
        self.assertEqual(v.initialValue(), value)

    def test_interface_type(self):
        from libcellml import Variable

        # InterfaceType
        self.assertNotEqual(
            Variable.InterfaceType.NONE,
            Variable.InterfaceType.PRIVATE,
        )
        self.assertNotEqual(
            Variable.InterfaceType.PUBLIC,
            Variable.InterfaceType.PUBLIC_AND_PRIVATE,
        )

    def test_set_interface_type(self):
        from libcellml import Variable

        # void setInterfaceType(const std::string &interfaceType)
        v = Variable()
        v.setInterfaceType('special type')

        # void setInterfaceType(InterfaceType interfaceType)
        v = Variable()
        v.setInterfaceType(Variable.InterfaceType.NONE)
        v.setInterfaceType(Variable.InterfaceType.PRIVATE)
        self.assertEqual("private", v.interfaceType())
        v.removeInterfaceType()
        self.assertEqual("", v.interfaceType())
        v.setInterfaceType(Variable.InterfaceType.PUBLIC)
        v.setInterfaceType(Variable.InterfaceType.PUBLIC_AND_PRIVATE)
        self.assertTrue(v.hasInterfaceType(Variable.InterfaceType.PUBLIC_AND_PRIVATE))
        self.assertRaises(
            RuntimeError, v.setInterfaceType, Variable.InterfaceType.NONE - 1)
        self.assertRaises(
            RuntimeError, v.setInterfaceType,
            Variable.InterfaceType.PUBLIC_AND_PRIVATE + 1)
        del v

        # void setInterfaceType(const std::string &interfaceType)
        v = Variable()
        v.setInterfaceType('not an interface type')
        del v

    def test_interface_type(self):
        from libcellml import Variable

        # std::string interfaceType()
        v = Variable()
        self.assertEqual(v.interfaceType(), '')
        v.setInterfaceType(Variable.InterfaceType.NONE)
        self.assertEqual(v.interfaceType(), 'none')
        v.setInterfaceType(Variable.InterfaceType.PRIVATE)
        self.assertEqual(v.interfaceType(), 'private')
        v.setInterfaceType(Variable.InterfaceType.PUBLIC)
        self.assertEqual(v.interfaceType(), 'public')
        v.setInterfaceType(Variable.InterfaceType.PUBLIC_AND_PRIVATE)
        self.assertEqual(v.interfaceType(), 'public_and_private')

    def test_minimum_interface_type(self):
        from libcellml import Variable

        v_public = Variable()
        v_public.setName("v_public")
        v_public.setInterfaceType("public")

        v_private = Variable()
        v_public.setName("v_private")
        v_private.setInterfaceType("private")

        v_public_and_private = Variable()
        v_public_and_private.setName("v_public_and_private")
        v_public_and_private.setInterfaceType("public_and_private")

        v_none = Variable()
        v_none.setName("v_none")
        v_none.setInterfaceType("none")

        v_empty = Variable()
        v_empty.setName("v_empty")

        # Stored public_and_private meets all requirements.
        self.assertTrue(v_public_and_private.permitsInterfaceType(Variable.InterfaceType.NONE))
        self.assertTrue(v_public_and_private.permitsInterfaceType(Variable.InterfaceType.PRIVATE))
        self.assertTrue(v_public_and_private.permitsInterfaceType(Variable.InterfaceType.PUBLIC))
        self.assertTrue(v_public_and_private.permitsInterfaceType(Variable.InterfaceType.PUBLIC_AND_PRIVATE))

        # Stored private meets private and none requirements.
        self.assertTrue(v_private.permitsInterfaceType(Variable.InterfaceType.NONE))
        self.assertTrue(v_private.permitsInterfaceType(Variable.InterfaceType.PRIVATE))
        self.assertFalse(v_private.permitsInterfaceType(Variable.InterfaceType.PUBLIC))
        self.assertFalse(v_private.permitsInterfaceType(Variable.InterfaceType.PUBLIC_AND_PRIVATE))

        # Stored public meets public and none requirements.
        self.assertTrue(v_public.permitsInterfaceType(Variable.InterfaceType.NONE))
        self.assertFalse(v_public.permitsInterfaceType(Variable.InterfaceType.PRIVATE))
        self.assertTrue(v_public.permitsInterfaceType(Variable.InterfaceType.PUBLIC))
        self.assertFalse(v_public.permitsInterfaceType(Variable.InterfaceType.PUBLIC_AND_PRIVATE))

        # Stored none meets none requirements.
        self.assertTrue(v_none.permitsInterfaceType(Variable.InterfaceType.NONE))
        self.assertFalse(v_none.permitsInterfaceType(Variable.InterfaceType.PRIVATE))
        self.assertFalse(v_none.permitsInterfaceType(Variable.InterfaceType.PUBLIC))
        self.assertFalse(v_none.permitsInterfaceType(Variable.InterfaceType.PUBLIC_AND_PRIVATE))

        # Stored empty meets none requirements.
        self.assertTrue(v_empty.permitsInterfaceType(Variable.InterfaceType.NONE))
        self.assertFalse(v_empty.permitsInterfaceType(Variable.InterfaceType.PRIVATE))
        self.assertFalse(v_empty.permitsInterfaceType(Variable.InterfaceType.PUBLIC))
        self.assertFalse(v_empty.permitsInterfaceType(Variable.InterfaceType.PUBLIC_AND_PRIVATE))

    def test_clone(self):
        from libcellml import Units, Variable

        v = Variable("sodium")
        u = Units("kg_per_ml")

        v.setUnits(u)

        vCloned = v.clone()
        self.assertEqual("sodium", vCloned.name())
        self.assertEqual("kg_per_ml", vCloned.units().name())


if __name__ == '__main__':
    unittest.main()
