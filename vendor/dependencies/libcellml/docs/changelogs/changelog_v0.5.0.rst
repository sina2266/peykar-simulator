libCellML v0.5.0 Changelog
==========================

Analyser
--------

* AnaylserModel: Fix loss of Units definition information by `@hsorby <https://github.com/hsorby>`_ [`#1196 <https://github.com/cellml/libcellml/pull/1196>`_].
* Analyser: add support for unknown variables that have been marked as external by `@agarny <https://github.com/agarny>`_ [`#1184 <https://github.com/cellml/libcellml/pull/1184>`_].
* AnalyserVariable: hold onto the owning component reference by `@agarny <https://github.com/agarny>`_ [`#1185 <https://github.com/cellml/libcellml/pull/1185>`_].
* Analyser: don't optimise the order of variables and equations by `@agarny <https://github.com/agarny>`_ [`#1090 <https://github.com/cellml/libcellml/pull/1090>`_].
* Analyser: reworked the analysis of a model with external variables by `@agarny <https://github.com/agarny>`_ [`#1077 <https://github.com/cellml/libcellml/pull/1077>`_].
* Analyser: allow for the unknown variable to be either on the LHS or RHS of an equation by `@agarny <https://github.com/agarny>`_ [`#1071 <https://github.com/cellml/libcellml/pull/1071>`_].

Bug
---

* Fix flattening segfault by `@hsorby <https://github.com/hsorby>`_ [`#1179 <https://github.com/cellml/libcellml/pull/1179>`_].
* Resolve imports by `@hsorby <https://github.com/hsorby>`_ [`#1177 <https://github.com/cellml/libcellml/pull/1177>`_].
* Flattening is changing imported models by `@hsorby <https://github.com/hsorby>`_ [`#1175 <https://github.com/cellml/libcellml/pull/1175>`_].
* Fix importing units from a common base model by `@hsorby <https://github.com/hsorby>`_ [`#1161 <https://github.com/cellml/libcellml/pull/1161>`_].
* Fix importing in permissive mode by `@hsorby <https://github.com/hsorby>`_ [`#1157 <https://github.com/cellml/libcellml/pull/1157>`_].
* Align assignAllIds with documentation by `@hsorby <https://github.com/hsorby>`_ [`#1151 <https://github.com/cellml/libcellml/pull/1151>`_].
* Analyser/Generator: don't use weak pointers by `@agarny <https://github.com/agarny>`_ [`#1133 <https://github.com/cellml/libcellml/pull/1133>`_].
* Fix an issue when initialising a state variable using a variable instead of a constant by `@agarny <https://github.com/agarny>`_ [`#1147 <https://github.com/cellml/libcellml/pull/1147>`_].
* Fix segfault when using Units::equivalent/compatible API by `@hsorby <https://github.com/hsorby>`_ [`#1145 <https://github.com/cellml/libcellml/pull/1145>`_].
* Fix parsing of CellML 1.X models with multiple math in a component by `@hsorby <https://github.com/hsorby>`_ [`#1146 <https://github.com/cellml/libcellml/pull/1146>`_].
* Enable printer to print models that have multiple math documents in a single math container object by `@hsorby <https://github.com/hsorby>`_ [`#1142 <https://github.com/cellml/libcellml/pull/1142>`_].
* Set minimum OSX deployment target that supports std::any by `@hsorby <https://github.com/hsorby>`_ [`#1111 <https://github.com/cellml/libcellml/pull/1111>`_].
* Try using uLong for buffer size in uncompress by `@hsorby <https://github.com/hsorby>`_ [`#1110 <https://github.com/cellml/libcellml/pull/1110>`_].

Coverage
--------

* Resolve branch coverage in validator.cpp by `@hsorby <https://github.com/hsorby>`_ [`#1128 <https://github.com/cellml/libcellml/pull/1128>`_].
* Resolve branch coverage in annotator.cpp by `@hsorby <https://github.com/hsorby>`_ [`#1127 <https://github.com/cellml/libcellml/pull/1127>`_].
* Resolve branch coverage in utilities.cpp by `@hsorby <https://github.com/hsorby>`_ [`#1126 <https://github.com/cellml/libcellml/pull/1126>`_].
* Resolve branch coverage in units.cpp by `@hsorby <https://github.com/hsorby>`_ [`#1125 <https://github.com/cellml/libcellml/pull/1125>`_].
* Resolve branch coverage in importer.cpp by `@hsorby <https://github.com/hsorby>`_ [`#1124 <https://github.com/cellml/libcellml/pull/1124>`_].
* Resolve branch coverage in printer.cpp by `@hsorby <https://github.com/hsorby>`_ [`#1123 <https://github.com/cellml/libcellml/pull/1123>`_].
* Resolve branch coverage in variable.cpp by `@hsorby <https://github.com/hsorby>`_ [`#1122 <https://github.com/cellml/libcellml/pull/1122>`_].
* Resolve branch coverage in component.cpp by `@hsorby <https://github.com/hsorby>`_ [`#1116 <https://github.com/cellml/libcellml/pull/1116>`_].
* Resolve branch coverage in reset.cpp by `@hsorby <https://github.com/hsorby>`_ [`#1120 <https://github.com/cellml/libcellml/pull/1120>`_].
* Resolve branch coverage in model.cpp by `@hsorby <https://github.com/hsorby>`_ [`#1121 <https://github.com/cellml/libcellml/pull/1121>`_].
* Resolve branch coverage in types.cpp by `@hsorby <https://github.com/hsorby>`_ [`#1119 <https://github.com/cellml/libcellml/pull/1119>`_].
* Resolve branch coverage in logger.cpp by `@hsorby <https://github.com/hsorby>`_ [`#1118 <https://github.com/cellml/libcellml/pull/1118>`_].
* Resolve branch coverage in importsource.cpp by `@hsorby <https://github.com/hsorby>`_ [`#1117 <https://github.com/cellml/libcellml/pull/1117>`_].
* Resolve branch coverage in componententity.cpp by `@hsorby <https://github.com/hsorby>`_ [`#1115 <https://github.com/cellml/libcellml/pull/1115>`_].
* Resolve branch coverage in xmlattribute.cpp by `@hsorby <https://github.com/hsorby>`_ [`#1114 <https://github.com/cellml/libcellml/pull/1114>`_].
* Resolve branch coverage in xmlnode.cpp by `@hsorby <https://github.com/hsorby>`_ [`#1113 <https://github.com/cellml/libcellml/pull/1113>`_].
* Resolve branch coverage in xmldoc.cpp by `@hsorby <https://github.com/hsorby>`_ [`#1112 <https://github.com/cellml/libcellml/pull/1112>`_].

Generator
---------

* Generator: cast our malloc() calls by `@agarny <https://github.com/agarny>`_ [`#1076 <https://github.com/cellml/libcellml/pull/1076>`_].

Imports
-------

* Flattening multiple equivalences by `@hsorby <https://github.com/hsorby>`_ [`#1170 <https://github.com/cellml/libcellml/pull/1170>`_].
* Importing units with references to non-standard units by `@hsorby <https://github.com/hsorby>`_ [`#1167 <https://github.com/cellml/libcellml/pull/1167>`_].

Infrastructure
--------------

* Update setup-msbuild to the latest version by `@hsorby <https://github.com/hsorby>`_ [`#1190 <https://github.com/cellml/libcellml/pull/1190>`_].
* Update deploy script  by `@hsorby <https://github.com/hsorby>`_ [`#1188 <https://github.com/cellml/libcellml/pull/1188>`_].
* Set macOS 10.15 as the minimum deployment target for macOS by `@hsorby <https://github.com/hsorby>`_ [`#1181 <https://github.com/cellml/libcellml/pull/1181>`_].
* GitHub Actions update by `@hsorby <https://github.com/hsorby>`_ [`#1154 <https://github.com/cellml/libcellml/pull/1154>`_].
* CMake: fixed an issue with BINDINGS_PYTHON's value not being taken into account by `@agarny <https://github.com/agarny>`_ [`#1149 <https://github.com/cellml/libcellml/pull/1149>`_].
* CMake: fixed a warning when running the `UNDEFINED_SYMBOLS_ALLOWED` test by `@agarny <https://github.com/agarny>`_ [`#1138 <https://github.com/cellml/libcellml/pull/1138>`_].
* Analyser/Generator: cleaning up of code by `@agarny <https://github.com/agarny>`_ [`#1080 <https://github.com/cellml/libcellml/pull/1080>`_].
* Fix for CI issues by `@hsorby <https://github.com/hsorby>`_ [`#1088 <https://github.com/cellml/libcellml/pull/1088>`_].
* Update environmentchecks.cmake to handle newer LibXml2 CMake configuration by `@hsorby <https://github.com/hsorby>`_ [`#1085 <https://github.com/cellml/libcellml/pull/1085>`_].

Miscellaneous
-------------

* Analyser/Generator: added some `xxxAsString()` methods by `@agarny <https://github.com/agarny>`_ [`#1131 <https://github.com/cellml/libcellml/pull/1131>`_].
* Make gcc 11.3.0 happy on Ubuntu 22.04 LTS by `@agarny <https://github.com/agarny>`_ [`#1153 <https://github.com/cellml/libcellml/pull/1153>`_].
* Analyser/Generator: replace if...else statements with switch ones wherever possible by `@agarny <https://github.com/agarny>`_ [`#1135 <https://github.com/cellml/libcellml/pull/1135>`_].
* Tests: added support for libXml2 2.9.11+ by `@agarny <https://github.com/agarny>`_ [`#1069 <https://github.com/cellml/libcellml/pull/1069>`_].

Validation
----------

* Improve MathML validation by `@agarny <https://github.com/agarny>`_ [`#1083 <https://github.com/cellml/libcellml/pull/1083>`_].

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
