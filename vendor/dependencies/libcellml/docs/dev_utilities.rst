=========
Utilities
=========

This document covers the utilities that are available to developers of libCellML.
The utilities are not built or made available by default.
The utilities can be enabled through the configuration of the library.
In order to enable the utilities, set the configuration variable :code:`LIBCELLML_ENABLE_UTILITIES` to :code:`TRUE`.

The available utilities are:

#. Create MathML DTD files
#. Update generator profile SHA-1 values

Create MathML DTD files
=======================

The create MathML DTD files utility generates a compressed MathML DTD.
The MathML DTD is used to determine the validity of MathML strings in a CellML document.
This utility updates the header file :code:`mathmldtd.h` and source file :code:`mathmldtd.cpp` in the source tree of the current build.
For a make-based environment, the utility may be run with the following command::

  make create_mathml_dtd_files

Update generator profile SHA-1 values
=====================================

The update generator profile SHA-1 values utility calculates the SHA-1 values for the current C and Python generator profiles.
This utility updates the header file :code:`generatorprofilesha1values.h` in the source tree of the current build.
For a make-based environment, the utility may be run with the following command::

  make update_generator_profile_sha1_values
