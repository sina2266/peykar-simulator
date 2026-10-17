#
# Tests if all CellML bindings have docstrings set.
#
import types
import unittest


class DocstringTestCase(unittest.TestCase):

    def test_docstrings(self):
        import libcellml

        # Scan for missing or empty docstrings
        def scan(root, missing, prefix=''):
            prefix += root.__name__
            if not root.__doc__ and root.__name__ != 'CellmlElementType':
                missing.append(prefix)
            prefix += '.'
            # Scan children, using dict instead of dir to avoid inherited
            # methods.
            for x in root.__dict__:
                if x[:1] != '_' and x is not root:
                    child = getattr(root, x)
                    if isinstance(child, (type, types.FunctionType)):
                        scan(child, missing, prefix)

        missing = []
        scan(libcellml, missing)
        if missing:
            raise Exception(
                'Missing (' + str(len(missing)) + ') docstrings, for: '
                + ', '.join(missing))


if __name__ == '__main__':
    unittest.main()
