#
# Tests the version function bindings
#
import unittest


class VersionTestCase(unittest.TestCase):

    def test_version(self):
        import libcellml

        # unsigned int version()
        self.assertIsInstance(libcellml.version(), int)
        self.assertGreater(libcellml.version(), 0)

        # const std::string versionString()
        self.assertIsInstance(libcellml.versionString(), str)
        self.assertNotEqual(libcellml.versionString(), '')


if __name__ == '__main__':
    unittest.main()
