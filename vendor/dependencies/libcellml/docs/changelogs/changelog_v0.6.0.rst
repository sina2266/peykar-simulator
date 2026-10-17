libCellML v0.6.0 Changelog
==========================

Analyser
--------

* Analyser: improve the reporting of unit-related issues with the pow() function by `@agarny <https://github.com/agarny>`_ [`#1212 <https://github.com/cellml/libcellml/pull/1212>`_].

Coverage
--------

* Python docstrings tests: don't test for CellmlElementType by `@agarny <https://github.com/agarny>`_ [`#1246 <https://github.com/cellml/libcellml/pull/1246>`_].

Documentation
-------------

* Code of conduct by `@hsorby <https://github.com/hsorby>`_ [`#1232 <https://github.com/cellml/libcellml/pull/1232>`_].
* Fix some of the links in the documentation by `@hsorby <https://github.com/hsorby>`_ [`#1192 <https://github.com/cellml/libcellml/pull/1192>`_].

Generator
---------

* Code generation: unit scaling for rate computation is the other way round by `@agarny <https://github.com/agarny>`_ [`#1241 <https://github.com/cellml/libcellml/pull/1241>`_].
* Generator: `computeComputedConstants()` may not generate some dependencies in the right place by `@agarny <https://github.com/agarny>`_ [`#1217 <https://github.com/cellml/libcellml/pull/1217>`_].
* Generator: use `size_t` rather than `int` in `nlaSolve()` by `@agarny <https://github.com/agarny>`_ [`#1202 <https://github.com/cellml/libcellml/pull/1202>`_].

Imports
-------

* Fix the segfault when resolving imports by `@hsorby <https://github.com/hsorby>`_ [`#1204 <https://github.com/cellml/libcellml/pull/1204>`_].

Infrastructure
--------------

* CI build fixes by `@hsorby <https://github.com/hsorby>`_ [`#1268 <https://github.com/cellml/libcellml/pull/1268>`_].
* Update `cibuildwheel` by `@hsorby <https://github.com/hsorby>`_ [`#1264 <https://github.com/cellml/libcellml/pull/1264>`_].
* Add Codespell GitHub action by `@hsorby <https://github.com/hsorby>`_ [`#1234 <https://github.com/cellml/libcellml/pull/1234>`_].
* Remove FindPython* files by `@hsorby <https://github.com/hsorby>`_ [`#1236 <https://github.com/cellml/libcellml/pull/1236>`_].
* Update GitHub actions action to newer versions by `@hsorby <https://github.com/hsorby>`_ [`#1224 <https://github.com/cellml/libcellml/pull/1224>`_].
* Switch to using Here document by `@hsorby <https://github.com/hsorby>`_ [`#1200 <https://github.com/cellml/libcellml/pull/1200>`_].

Python bindings
---------------

* Protect against empty classifiers in Python bindings by `@hsorby <https://github.com/hsorby>`_ [`#1215 <https://github.com/cellml/libcellml/pull/1215>`_].
* Produce python wheels for Python 3.12 by `@hsorby <https://github.com/hsorby>`_ [`#1209 <https://github.com/cellml/libcellml/pull/1209>`_].

Validation
----------

* Update the validation  by `@hsorby <https://github.com/hsorby>`_ [`#1225 <https://github.com/cellml/libcellml/pull/1225>`_].

Contributors
------------

.. image:: https://avatars.githubusercontent.com/u/778048?v=4
   :target: https://github.com/hsorby
   :height: 32
   :width: 32
.. image:: https://avatars.githubusercontent.com/u/602265?v=4
   :target: https://github.com/agarny
   :height: 32
   :width: 32
