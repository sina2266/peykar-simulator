#
# Tests the entity class bindings
#
import unittest


class EntityTestCase(unittest.TestCase):

    def test_create(self):
        from libcellml.entity import Entity

        self.assertRaises(AttributeError, Entity)

    def test_id(self):
        from libcellml import Model

        # std::string id()
        x = Model()
        self.assertEqual(x.id(), '')

        # void setId(const std::string &id)
        idx = 'test'
        x.setId(idx)
        self.assertEqual(x.id(), idx)

        x.removeId()
        self.assertEqual(x.id(), '')

    def test_parent(self):
        from libcellml import Model, Component

        x = Model("model")
        self.assertIsNone(x.parent())

        c = Component()
        self.assertIsNone(c.parent())
        x.addComponent(c)
        self.assertEqual("model", c.parent().name())
        self.assertEqual(1, x.componentCount())

    def test_has_parent(self):
        from libcellml import Component

        # bool hasParent(Component* c)
        x = Component()
        c1 = Component()
        self.assertFalse(x.hasParent())
        c1.addComponent(x)
        self.assertTrue(x.hasParent())

        c2 = Component()
        c1.addComponent(c2)
        c2.addComponent(x)
        self.assertTrue(x.hasParent())

    def test_has_ancestor(self):
        from libcellml import Component

        # bool hasParent(Component* c)
        x = Component()
        c1 = Component()
        c2 = Component()
        self.assertFalse(c2.hasAncestor(c1))
        self.assertFalse(c2.hasAncestor(x))

        x.addComponent(c1)
        c1.addComponent(c2)
        self.assertTrue(c2.hasAncestor(c1))
        self.assertTrue(c2.hasAncestor(x))

    def test_equals(self):
        from libcellml import Units

        u1 = Units("second")
        u1.setId('units_second')

        u2 = Units("second")
        u2.setId('units_second')

        self.assertTrue(u1.equals(u2))


class ParentedEntityTestCase(unittest.TestCase):

    def test_create(self):
        from libcellml.parentedentity import ParentedEntity

        self.assertRaises(AttributeError, ParentedEntity)


class NamedEntityTestCase(unittest.TestCase):

    def test_create(self):
        from libcellml.namedentity import NamedEntity

        self.assertRaises(AttributeError, NamedEntity)


class LoggerTestCase(unittest.TestCase):

    def test_create(self):
        from libcellml.logger import Logger

        self.assertRaises(AttributeError, Logger)


if __name__ == '__main__':
    unittest.main()
