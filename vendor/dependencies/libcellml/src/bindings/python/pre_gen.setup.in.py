""" libCellML Library: A library for the parsing, printing, and manipulation
of CellML 2.0 compliant models.

"""

classifiers = """\
Development Status :: 5 - Production/Stable
Intended Audience :: Developers
Intended Audience :: Education
Intended Audience :: Science/Research
License :: OSI Approved :: Apache Software License
Programming Language :: Python
Operating System :: Microsoft :: Windows
Operating System :: Unix
Operating System :: MacOS :: MacOS X
Topic :: Software Development :: Libraries :: Python Modules
"""

import os
import subprocess

from setuptools import setup
from setuptools.dist import Distribution


doclines = __doc__.split("\n")


class BinaryDistribution(Distribution):
    def is_pure(self):
        return False

    def has_ext_modules(self):
        return True


# This creates a list which is empty but returns a length of 1.
# Should make the wheel a binary distribution and platlib compliant.
class EmptyListWithLength(list):
    def __len__(self):
        return 1


setup(
    name='@PYPI_PACKAGE_NAME@',
    version='@PYPI_PACKAGE_VERSION@@PYPI_PACKAGE_DEVELOPER_VERSION@',
    author='libCellML developers',
    author_email='libcellml@googlegroups.com',
    packages=['libcellml'],
    url='@PYPI_PACKAGE_URL@',
    license='Apache Software License',
    description=doclines[0],
    classifiers=[cl for cl in classifiers.split("\n") if cl],
    long_description=open('README.rst').read(),
    long_description_content_type='text/x-rst',
    distclass=BinaryDistribution,
    ext_modules=EmptyListWithLength(),
    package_data={'libcellml': [@SETUP_PY_PACKAGE_FILES_STR@]},
    include_package_data=True,
    zip_safe=False,
)
