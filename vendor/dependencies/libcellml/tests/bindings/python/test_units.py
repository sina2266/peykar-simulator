#
# Tests the Units class bindings
#
import unittest


class UnitsTestCase(unittest.TestCase):

    def test_create_destroy(self):
        from libcellml import Units

        x = Units()
        del x

        y = Units("mine")
        self.assertEqual("mine", y.name())
        del y

    def test_inheritance(self):
        import libcellml
        from libcellml import Units

        x = Units()
        self.assertIsInstance(x, libcellml.importedentity.ImportedEntity)
        self.assertIsInstance(x, libcellml.namedentity.NamedEntity)
        self.assertIsInstance(x, libcellml.entity.Entity)

        # Test access to inherited methods
        x = Units()
        idx = 'test'
        self.assertEqual(x.id(), '')
        x.setId(idx)
        self.assertEqual(x.id(), idx)

    def test_create_imported_entity(self):
        from libcellml.importedentity import ImportedEntity

        self.assertRaises(AttributeError, ImportedEntity)

    def test_standard_unit(self):
        from libcellml import Units

        u = Units()
        u.addUnit(Units.StandardUnit.AMPERE)
        u.addUnit(Units.StandardUnit.BECQUEREL)
        u.addUnit(Units.StandardUnit.CANDELA)
        u.addUnit(Units.StandardUnit.COULOMB)
        u.addUnit(Units.StandardUnit.DIMENSIONLESS)
        u.addUnit(Units.StandardUnit.FARAD)
        u.addUnit(Units.StandardUnit.GRAM)
        u.addUnit(Units.StandardUnit.GRAY)
        u.addUnit(Units.StandardUnit.HENRY)
        u.addUnit(Units.StandardUnit.HERTZ)
        u.addUnit(Units.StandardUnit.JOULE)
        u.addUnit(Units.StandardUnit.KATAL)
        u.addUnit(Units.StandardUnit.KELVIN)
        u.addUnit(Units.StandardUnit.KILOGRAM)
        u.addUnit(Units.StandardUnit.LITRE)
        u.addUnit(Units.StandardUnit.LUMEN)
        u.addUnit(Units.StandardUnit.LUX)
        u.addUnit(Units.StandardUnit.METRE)
        u.addUnit(Units.StandardUnit.MOLE)
        u.addUnit(Units.StandardUnit.NEWTON)
        u.addUnit(Units.StandardUnit.OHM)
        u.addUnit(Units.StandardUnit.PASCAL)
        u.addUnit(Units.StandardUnit.RADIAN)
        u.addUnit(Units.StandardUnit.SECOND)
        u.addUnit(Units.StandardUnit.SIEMENS)
        u.addUnit(Units.StandardUnit.SIEVERT)
        u.addUnit(Units.StandardUnit.STERADIAN)
        u.addUnit(Units.StandardUnit.TESLA)
        u.addUnit(Units.StandardUnit.VOLT)
        u.addUnit(Units.StandardUnit.WATT)
        u.addUnit(Units.StandardUnit.WEBER)
        self.assertRaises(
            RuntimeError, u.addUnit, Units.StandardUnit.AMPERE - 1)
        self.assertRaises(
            RuntimeError, u.addUnit, Units.StandardUnit.WEBER + 1)

    def test_is_base_unit(self):
        from libcellml import Units

        # bool isBaseUnit()
        u = Units()
        self.assertTrue(u.isBaseUnit())
        u.addUnit(Units.StandardUnit.NEWTON)
        self.assertFalse(u.isBaseUnit())

    def test_add_unit(self):
        from libcellml import Units

        # void addUnit(const std::string &reference, const std::string &prefix,
        #   double exponent=1.0, double multiplier=1.0)
        u = Units()
        u.addUnit('a', 'b')
        u.addUnit('a', 'b', 2.0)
        u.addUnit('a', 'b', -1)
        u.addUnit('a', 'b', 0)
        u.addUnit('a', 'b', 3, 3)
        u.addUnit('a', 'b', 0.1, -1.2)
        del u

        # void addUnit(const std::string &reference, int prefix,
        #   double exponent, double multiplier=1.0)
        u = Units()
        u.addUnit('a', 1, -1)
        u.addUnit('a', 1, 2.3)
        u.addUnit('a', 1, -1, 3)
        u.addUnit('a', 1, -1, 2.3)
        u.addUnit('a', 1, 1.2, 3.4)

        u.addUnit('a', -1, -1)
        u.addUnit('a', -1, 2.3)
        u.addUnit('a', -1, -1, 3)
        u.addUnit('a', -1, -1, 2.3)
        u.addUnit('a', -1, 1.2, 3.4)
        del u

        # void addUnit(const std::string &reference, double exponent)
        u = Units()
        u.addUnit('a', 1.0)
        # TODO Ints get converted to Prefix enum, not to double!
        # u.addUnit('a', -1)
        del u

        # void addUnit(const std::string &reference)
        u = Units()
        u.addUnit('')
        u.addUnit('a')
        del u

        # void addUnit(StandardUnit standardRef, const std::string &prefix,
        #   double exponent=1.0, double multiplier=1.0)
        u = Units()
        u.addUnit(Units.StandardUnit.KATAL, 'pico')
        u.addUnit(Units.StandardUnit.KATAL, 'pico', 1.0)
        u.addUnit(Units.StandardUnit.KATAL, 'pico', -1)
        u.addUnit(Units.StandardUnit.KATAL, 'pico', 1.0, 2.0)
        u.addUnit(Units.StandardUnit.KATAL, 'pico', 1, 2.0)
        u.addUnit(Units.StandardUnit.KATAL, 'pico', -1, 2)
        del u

        # void addUnit(StandardUnit standardRef, int prefix,
        #   double exponent, double multiplier=1.0)
        u = Units()
        u.addUnit(Units.StandardUnit.KATAL, 1, 1.0)
        u.addUnit(Units.StandardUnit.KATAL, -1, -1.0)
        u.addUnit(Units.StandardUnit.KATAL, 1, 1.0, 1.0)
        u.addUnit(Units.StandardUnit.KATAL, -1, -1.0, 1.0, 'id')
        del u

        # void addUnit(StandardUnit standardRef, double exponent)
        # Hidden to avoid confusion with addUnit(StandardUnit, Prefix, double,
        # double)

        # void addUnit(StandardUnit standardRef)
        u = Units()
        u.addUnit(Units.StandardUnit.KATAL)
        del u

    def test_unit_attributes(self):
        from libcellml import Units

        # void unitAttributes(size_t index, std::string &reference,
        #   std::string &prefix, double &exponent, double &multiplier, std::string &id)
        u = Units()
        x = u.unitAttributes(0)
        self.assertIsInstance(x, list)
        self.assertEqual(x, ['', '', 1.0, 1.0, ''])
        u.addUnit('blabla', 'hello', 1.2, 3.4, 'unitid')
        x = u.unitAttributes(0)
        self.assertIsInstance(x, list)
        self.assertEqual(x, ['blabla', 'hello', 1.2, 3.4, 'unitid'])
        x = u.unitAttributes(1)
        self.assertIsInstance(x, list)
        self.assertEqual(x, ['', '', 1.0, 1.0, ''])
        del [u, x]

        # void unitAttributes(const std::string &reference,
        #   std::string &prefix, double &exponent, double &multiplier) const;
        u = Units()
        x = u.unitAttributes('newton')
        self.assertIsInstance(x, list)
        self.assertEqual(x, ['newton', '', 1.0, 1.0, ''])
        u.addUnit('few', 'bars', 4.3, 2.1, 'job')
        x = u.unitAttributes('newton')
        self.assertIsInstance(x, list)
        self.assertEqual(x, ['newton', '', 1.0, 1.0, ''])
        x = u.unitAttributes('few')
        self.assertIsInstance(x, list)
        self.assertEqual(x, ['few', 'bars', 4.3, 2.1, 'job'])
        del [u, x]

        # This method conflicts with unitAttributes(size_t, ...)
        # void unitAttributes(StandardUnit standardRef, std::string &prefix,
        #   double &exponent, double &multiplier) const;

    def test_remove_unit(self):
        from libcellml import Units

        # bool removeUnit(size_t index)
        u = Units()
        self.assertFalse(u.removeUnit(0))
        self.assertFalse(u.removeUnit(1))
        self.assertFalse(u.removeUnit(-1))
        u.addUnit('hello')
        self.assertFalse(u.removeUnit(1))
        self.assertFalse(u.removeUnit(-1))
        self.assertTrue(u.removeUnit(0))
        self.assertFalse(u.removeUnit(0))
        del [u]

        # bool removeUnit(const std::string &reference)
        u = Units()
        self.assertFalse(u.removeUnit('hello'))
        u.addUnit('hello')
        self.assertFalse(u.removeUnit('hi'))
        self.assertTrue(u.removeUnit('hello'))
        self.assertFalse(u.removeUnit('hello'))
        del [u]

        # This method conflicts with removeUnit(size_t)
        # bool removeUnit(StandardUnit standardRef)

    def test_unit_count(self):
        from libcellml import Units

        # size_t unitCount()
        u = Units()
        self.assertEqual(u.unitCount(), 0)
        u.addUnit('')
        self.assertEqual(u.unitCount(), 1)
        u.addUnit('')
        self.assertEqual(u.unitCount(), 2)

    def test_remove_all_units(self):
        from libcellml import Units

        # void removeAllUnits()
        u = Units()
        self.assertEqual(u.unitCount(), 0)
        u.addUnit('')
        self.assertEqual(u.unitCount(), 1)
        u.addUnit('')
        self.assertEqual(u.unitCount(), 2)
        u.removeAllUnits()
        self.assertEqual(u.unitCount(), 0)

    def test_set_source_units(self):
        from libcellml import Units, ImportSource

        # void setSourceUnits(const ImportPtr &imp, const std::string &name)
        i = ImportSource()
        u = Units()
        u.setSourceUnits(i, 'hello')

        self.assertTrue(u.isImport())

    def test_import_units(self):
        from libcellml import Units, ImportSource

        i = ImportSource()
        u = Units()

        u.setImportSource(i)
        u.setImportReference("Volt")

        self.assertTrue(u.isImport())

    def test_requires_imports(self):
        from libcellml import Units

        u = Units("Volt")
        self.assertFalse(u.requiresImports())

    def test_scaling_factor(self):
        from libcellml import Units
        from libcellml.units import Units_scalingFactor

        u1 = Units("BigVolts")
        u1.addUnit("volt", 0, 1, 1000)

        u2 = Units("LittleVolts")
        u2.addUnit("volt")

        self.assertEqual(0.001, Units.scalingFactor(u1, u2))
        self.assertEqual(0.001, Units_scalingFactor(u1, u2))

    def test_compatible(self):
        from libcellml import Units
        from libcellml.units import Units_compatible

        u1 = Units("BigVolts")
        u1.addUnit("volt", 0, 1, 1000)

        u2 = Units("LittleVolts")
        u2.addUnit("volt")

        self.assertTrue(Units.compatible(u1, u2))
        self.assertTrue(Units_compatible(u1, u2))

    def test_equivalent(self):
        from libcellml import Units
        from libcellml.units import Units_equivalent

        u1 = Units("BigVolts")
        u1.addUnit("volt", 0, 1, 1000)

        u2 = Units("LittleVolts")
        u2.addUnit("volt")

        self.assertFalse(Units.equivalent(u1, u2))
        self.assertFalse(Units_equivalent(u1, u2))

    def test_clone(self):
        from libcellml import Units

        u = Units("BigVolts")
        u.addUnit("volt", 0, 1, 1000)

        uCloned = u.clone()
        self.assertEqual(1, uCloned.unitCount())

    def test_unit_id(self):
        from libcellml import Units

        u = Units("BigVolts")
        u.addUnit("volt", 0, 1, 1000)

        self.assertEqual("", u.unitId(0))

        u.setUnitId(0, "dangerous")
        self.assertEqual("dangerous", u.unitId(0))

    def test_unit_attributes_single(self):
        from libcellml import Units

        u = Units("Best")
        u.addUnit('coolio', 'hello', 1.2, 3.4)

        self.assertEqual('coolio', u.unitAttributeReference(0))
        self.assertEqual('hello', u.unitAttributePrefix(0))
        self.assertEqual(1.2, u.unitAttributeExponent(0))
        self.assertEqual(3.4, u.unitAttributeMultiplier(0))

        u.setUnitAttributeReference(0, 'arrdee')
        self.assertEqual('arrdee', u.unitAttributeReference(0))

    def test_is_defined(self):
        from libcellml import Units

        u = Units("BigVolts")
        u.addUnit("volt", 0, 1, 1000)

        self.assertTrue(u.isDefined())


if __name__ == '__main__':
    unittest.main()
