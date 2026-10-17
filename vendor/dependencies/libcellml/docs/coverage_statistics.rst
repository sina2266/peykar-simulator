.. _coverage_statistics:

===================
Coverage Statistics
===================

**TODO**
The output from the coverage testing using gcov is available TODO.

.. note::

   The coverage test sometimes (this behaviour has been observed on macOS using Clang) reports single lines containing only a closing curly brace as not covered.
   This is currently being treated as a false positive.
   This can be seen in the 'Missing' column of the test report where only single lines are reported.
   For the case discussed here, the reported line should only contain a single closing curly brace.
   In this situation, we will accept the missed coverage report.
