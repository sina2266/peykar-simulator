#
# Tests for the ImportRequirement class.
#
import unittest

from test_resources import file_contents


class ImportRequirementTestCase(unittest.TestCase):

    def test_requirements(self):
        from libcellml import Parser, Importer
        keys = [
            'complicatedComponents.cellml',
            'complicatedUnits.cellml',
        ]

        parser = Parser()
        model = parser.parseModel(file_contents('import-requirements/complicatedExample.cellml'))
        self.assertEqual(0, parser.issueCount())

        requirements = model.importRequirements()

        i = 0
        for r in requirements:
            self.assertEqual(keys[i], r)
            i += 1


if __name__ == '__main__':
    unittest.main()
