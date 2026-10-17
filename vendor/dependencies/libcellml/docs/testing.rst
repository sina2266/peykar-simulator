.. _testing:

=================
Testing libCellML
=================

Every pull request against the libCellML prime repository is tested as a matter of course, but there might be times when you'd like to test your own code without making a pull request.
There are two ways you can do this.

Using the Buildbot site
-----------------------
:buildbot:`Buildbot<>` has the capacity to build a libCellML compliant repository through the use of a :forcescheduler:`ForceScheduler<>` .
To make use of this facility, you will need to authenticate with the :buildbot:`Buildbot<>` system.
Access granted on request to David Nickerson d.nickerson@auckland.ac.nz.
The :buildbot:`Buildbot<>` system uses MD5 encrypted passwords as created by :htpasswd:`htpasswd<>`.
With your request to David include the output of this command::

   htpasswd -n <your-chosen-username>


The website http://www.htaccesstools.com/htpasswd-generator/ can be used if you do not have access to the htpasswd application.
When your request has been dealt with, you will be able to login to :libcellml_buildbot:`libCellML Buildbot<>` and submit code for testing.

Manually on your local computer
-------------------------------
When your libCellML has built successfully, and with tests included, executables for the tests will be located in the :code:`build/test` directory where your library has been built.

- **Unit tests** Run all of the unit tests using :code:`make test`.
   - Individual C++ unit tests may be run by locating the test executable within the :code:`tests/`, directory.
     Unit tests for a given class are named :code:`test_classname`, where :code:`classname` is the lowercase name of the class to test.
     These files are executable, so can be run from the command line :code:`tests/test_classname`.
   - To run individual Python tests, navigate first into the :code:`tests/bindings/python/` directory.
     The test files can then be run individually as needed using :code:`python test_classname.py`, where :code:`classname` is the lowercase name of the class to test.
- **Coverage tests** Coverage tests make sure that each line of code is covered by one or more unit tests.
  - A summary of coverage of the C++ code is accessible through :code:`make llvm_coverage`, which result in statistics of line coverage.
  - For detailed information regarding specific lines in each file that need test coverage, use the command **TODO**.
- **Formatting tests** Check that the code is correctly formatted using the :code:`make test_clang_format`, if you have CLang installed.  
  **TODO** Check what other OS use?
- **Memory tests** TODO
