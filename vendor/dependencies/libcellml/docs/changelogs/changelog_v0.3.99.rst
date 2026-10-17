libCellML v0.3.99 Changelog
===========================

Bug
---

* Fix validation equivalence calculation for base units by `@hsorby <https://github.com/hsorby>`_ [`#983 <https://github.com/cellml/libcellml/pull/983>`_].
* Create analyser test showing segfault due to unlinked Units by `@hsorby <https://github.com/hsorby>`_ [`#886 <https://github.com/cellml/libcellml/pull/886>`_].
* Move remove all issues by `@hsorby <https://github.com/hsorby>`_ [`#892 <https://github.com/cellml/libcellml/pull/892>`_].
* Move addIssue and removeError to private implementation of Logger class by `@hsorby <https://github.com/hsorby>`_ [`#891 <https://github.com/cellml/libcellml/pull/891>`_].
* Move ParentedEntities removeParent out of the public API by `@hsorby <https://github.com/hsorby>`_ [`#897 <https://github.com/cellml/libcellml/pull/897>`_].
* Add test where units are expected to be linked after parsing a model by `@hsorby <https://github.com/hsorby>`_ [`#888 <https://github.com/cellml/libcellml/pull/888>`_].
* Report errors from importing invalid models by `@hsorby <https://github.com/hsorby>`_ [`#875 <https://github.com/cellml/libcellml/pull/875>`_].
* Make resolve imports method more robust by `@hsorby <https://github.com/hsorby>`_ [`#873 <https://github.com/cellml/libcellml/pull/873>`_].
* Reference corrections by `@hsorby <https://github.com/hsorby>`_ [`#848 <https://github.com/cellml/libcellml/pull/848>`_].

Documentation
-------------

* Add changelogs for v0.1.0 and v0.2.0 by `@hsorby <https://github.com/hsorby>`_ [`#1009 <https://github.com/cellml/libcellml/pull/1009>`_].
* Permits doxygen table by `@hsorby <https://github.com/hsorby>`_ [`#994 <https://github.com/cellml/libcellml/pull/994>`_].
* Fix Doxygen warnings by `@hsorby <https://github.com/hsorby>`_ [`#890 <https://github.com/cellml/libcellml/pull/890>`_].
* Reform documentation for all assignId methods in the Annotator by `@hsorby <https://github.com/hsorby>`_ [`#889 <https://github.com/cellml/libcellml/pull/889>`_].

Generator
---------

* Generator: initialise external variables and fix our compute methods by `@agarny <https://github.com/agarny>`_ [`#961 <https://github.com/cellml/libcellml/pull/961>`_].
* Generator: use only one variable info structure by `@agarny <https://github.com/agarny>`_ [`#950 <https://github.com/cellml/libcellml/pull/950>`_].
* Generator profile: renamed some API methods by `@agarny <https://github.com/agarny>`_ [`#952 <https://github.com/cellml/libcellml/pull/952>`_].
* Generator: further refinements to the generated C code by `@agarny <https://github.com/agarny>`_ [`#941 <https://github.com/cellml/libcellml/pull/941>`_].

Infrastructure
--------------

* Fixes to get the CD working again by `@hsorby <https://github.com/hsorby>`_ [`#1008 <https://github.com/cellml/libcellml/pull/1008>`_].
* CMake: support CMAKE_BUILD_TYPE by `@agarny <https://github.com/agarny>`_ [`#978 <https://github.com/cellml/libcellml/pull/978>`_].
* CMake: allow zlib to find using ZLIB_ROOT by `@agarny <https://github.com/agarny>`_ [`#976 <https://github.com/cellml/libcellml/pull/976>`_].
* Update emscripten documentation to catch up with the current zlib build by `@hsorby <https://github.com/hsorby>`_ [`#967 <https://github.com/cellml/libcellml/pull/967>`_].
* Update build instructions for Windows by `@hsorby <https://github.com/hsorby>`_ [`#965 <https://github.com/cellml/libcellml/pull/965>`_].
* Add support for `buildcache` by `@agarny <https://github.com/agarny>`_ [`#960 <https://github.com/cellml/libcellml/pull/960>`_].
* CMake: make sure that ClangFormat is of the expected version by `@agarny <https://github.com/agarny>`_ [`#947 <https://github.com/cellml/libcellml/pull/947>`_].
* CMake: make sure that the version of ClangFormat is properly determined by `@agarny <https://github.com/agarny>`_ [`#944 <https://github.com/cellml/libcellml/pull/944>`_].
* CMake: mark `CLCACHE_EXE` as advanced by `@agarny <https://github.com/agarny>`_ [`#927 <https://github.com/cellml/libcellml/pull/927>`_].
* Disable Clang-Tidy by default by `@agarny <https://github.com/agarny>`_ [`#924 <https://github.com/cellml/libcellml/pull/924>`_].
* CMake: mark `CCACHE_EXE` as advanced by `@agarny <https://github.com/agarny>`_ [`#921 <https://github.com/cellml/libcellml/pull/921>`_].
* Python version update by `@hsorby <https://github.com/hsorby>`_ [`#869 <https://github.com/cellml/libcellml/pull/869>`_].
* Rework building of many linux python packages with GitHub actions by `@hsorby <https://github.com/hsorby>`_ [`#864 <https://github.com/cellml/libcellml/pull/864>`_].
* Adding a .npmrc file to upload libcellml.js to npmjs.com by `@hsorby <https://github.com/hsorby>`_ [`#844 <https://github.com/cellml/libcellml/pull/844>`_].
* Fixing the deploy script for cellml/libcellml by `@hsorby <https://github.com/hsorby>`_ [`#842 <https://github.com/cellml/libcellml/pull/842>`_].

JavaScript bindings
-------------------

* Tidy up JS bindings tests by `@hsorby <https://github.com/hsorby>`_ [`#1005 <https://github.com/cellml/libcellml/pull/1005>`_].
* Add JS bindings Enums tests  by `@hsorby <https://github.com/hsorby>`_ [`#1004 <https://github.com/cellml/libcellml/pull/1004>`_].
* Add JS bindings Types tests  by `@hsorby <https://github.com/hsorby>`_ [`#1003 <https://github.com/cellml/libcellml/pull/1003>`_].
* Add JS bindings Annotator tests by `@hsorby <https://github.com/hsorby>`_ [`#1002 <https://github.com/cellml/libcellml/pull/1002>`_].
* Add JS bindings Analyser Model tests by `@hsorby <https://github.com/hsorby>`_ [`#1001 <https://github.com/cellml/libcellml/pull/1001>`_].
* Add JS bindings Analyser Variable tests by `@hsorby <https://github.com/hsorby>`_ [`#1000 <https://github.com/cellml/libcellml/pull/1000>`_].
* Add JS bindings Analyser External Variable tests by `@hsorby <https://github.com/hsorby>`_ [`#997 <https://github.com/cellml/libcellml/pull/997>`_].
* Add JS bindings Analyser Equation Ast tests by `@hsorby <https://github.com/hsorby>`_ [`#995 <https://github.com/cellml/libcellml/pull/995>`_].
* Add JS bindings AnalyserEquation test by `@hsorby <https://github.com/hsorby>`_ [`#992 <https://github.com/cellml/libcellml/pull/992>`_].
* Add JS bindings Generator test by `@hsorby <https://github.com/hsorby>`_ [`#989 <https://github.com/cellml/libcellml/pull/989>`_].
* Add JS bindings ImportSource test by `@hsorby <https://github.com/hsorby>`_ [`#988 <https://github.com/cellml/libcellml/pull/988>`_].
* Add JS bindings Printer test by `@hsorby <https://github.com/hsorby>`_ [`#987 <https://github.com/cellml/libcellml/pull/987>`_].
* Add JS binding tests for Variable by `@hsorby <https://github.com/hsorby>`_ [`#986 <https://github.com/cellml/libcellml/pull/986>`_].
* Test Emscripten NamedEntity by `@hsorby <https://github.com/hsorby>`_ [`#936 <https://github.com/cellml/libcellml/pull/936>`_].
* Test Emscripten ComponentEntity by `@hsorby <https://github.com/hsorby>`_ [`#937 <https://github.com/cellml/libcellml/pull/937>`_].
* Test Emscripten ParentedEntity by `@hsorby <https://github.com/hsorby>`_ [`#934 <https://github.com/cellml/libcellml/pull/934>`_].
* Test Emscripten Entity by `@hsorby <https://github.com/hsorby>`_ [`#933 <https://github.com/cellml/libcellml/pull/933>`_].
* Implement javascript tests for Parser class by `@hsorby <https://github.com/hsorby>`_ [`#919 <https://github.com/cellml/libcellml/pull/919>`_].
* Implement javascript tests for Component class by `@hsorby <https://github.com/hsorby>`_ [`#918 <https://github.com/cellml/libcellml/pull/918>`_].
* Implement javascript tests for Units class by `@hsorby <https://github.com/hsorby>`_ [`#903 <https://github.com/cellml/libcellml/pull/903>`_].
* Add javascript tests for Reset by `@hsorby <https://github.com/hsorby>`_ [`#896 <https://github.com/cellml/libcellml/pull/896>`_].
* Update Emscripten bindings by `@hsorby <https://github.com/hsorby>`_ [`#874 <https://github.com/cellml/libcellml/pull/874>`_].
* Update the javascript bindings to use UnitsItem and not Unit by `@hsorby <https://github.com/hsorby>`_ [`#861 <https://github.com/cellml/libcellml/pull/861>`_].
* Allow only out of range exceptions for stoi and stod by `@hsorby <https://github.com/hsorby>`_ [`#851 <https://github.com/cellml/libcellml/pull/851>`_].
* Units prefix improvements by `@hsorby <https://github.com/hsorby>`_ [`#846 <https://github.com/cellml/libcellml/pull/846>`_].

Modified feature
----------------

* XmlDoc: compress the XHTML+MathML+SVG DTD only once by `@agarny <https://github.com/agarny>`_ [`#979 <https://github.com/cellml/libcellml/pull/979>`_].
* Generator: refine the generated code for algebraic models by `@agarny <https://github.com/agarny>`_ [`#901 <https://github.com/cellml/libcellml/pull/901>`_].
* Rename unsetOrder to removeOrder by `@hsorby <https://github.com/hsorby>`_ [`#898 <https://github.com/cellml/libcellml/pull/898>`_].
* Privatise set parent by `@hsorby <https://github.com/hsorby>`_ [`#876 <https://github.com/cellml/libcellml/pull/876>`_].
* Add assignId API for AnyCellmlElement by `@hsorby <https://github.com/hsorby>`_ [`#880 <https://github.com/cellml/libcellml/pull/880>`_].
* Annotator: replace std::pair with AnyCellmlElement by `@agarny <https://github.com/agarny>`_ [`#836 <https://github.com/cellml/libcellml/pull/836>`_].
* Rename Unit class to UnitsItem class by `@hsorby <https://github.com/hsorby>`_ [`#853 <https://github.com/cellml/libcellml/pull/853>`_].

New feature
-----------

* Validator check for cyclical imports by `@hsorby <https://github.com/hsorby>`_ [`#837 <https://github.com/cellml/libcellml/pull/837>`_].

Python bindings
---------------

* Expose cellmlElementTypeAsString in the Python bindings by `@hsorby <https://github.com/hsorby>`_ [`#985 <https://github.com/cellml/libcellml/pull/985>`_].
* Remove InstallCommand from setup.in.py by `@hsorby <https://github.com/hsorby>`_ [`#885 <https://github.com/cellml/libcellml/pull/885>`_].

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
