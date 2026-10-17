libCellML v0.1.0 Changelog
==========================

No category
-----------

* Add support for Clang-Format by `@agarny <https://github.com/agarny>`_ [`#310 <https://github.com/cellml/libcellml/pull/310>`_].
* CMake: address policy CMP0086 by `@agarny <https://github.com/agarny>`_ [`#308 <https://github.com/cellml/libcellml/pull/308>`_].
* Add list of source code contributors as recorded by GitHub by `@hsorby <https://github.com/hsorby>`_ [`#281 <https://github.com/cellml/libcellml/pull/281>`_].
* Add the Component.takeVariableMethods() #214 by `@TedAhmadi <https://github.com/TedAhmadi>`_ [`#278 <https://github.com/cellml/libcellml/pull/278>`_].
* Update the XmlNode and XmlAttribute type methods by `@nickerso <https://github.com/nickerso>`_ [`#275 <https://github.com/cellml/libcellml/pull/275>`_].
* Parsing should be done on fully qualified element names (#263) by `@agarny <https://github.com/agarny>`_ [`#264 <https://github.com/cellml/libcellml/pull/264>`_].
* Various basic clean-ups by `@agarny <https://github.com/agarny>`_ [`#267 <https://github.com/cellml/libcellml/pull/267>`_].
* Improve the configuration by `@hsorby <https://github.com/hsorby>`_ [`#255 <https://github.com/cellml/libcellml/pull/255>`_].
* Roadmap: replaced "CellML 1.2" with "CellML 2.0" by `@agarny <https://github.com/agarny>`_ [`#262 <https://github.com/cellml/libcellml/pull/262>`_].
* Tests for resolving imports by `@nickerso <https://github.com/nickerso>`_ [`#167 <https://github.com/cellml/libcellml/pull/167>`_].
* Imports revisited by `@hsorby <https://github.com/hsorby>`_ [`#248 <https://github.com/cellml/libcellml/pull/248>`_].
* Upgraded gtest to version 1.8.0 by `@agarny <https://github.com/agarny>`_ [`#251 <https://github.com/cellml/libcellml/pull/251>`_].
* Compilation error "catching polymorphic type ... by value" (#246) by `@agarny <https://github.com/agarny>`_ [`#247 <https://github.com/cellml/libcellml/pull/247>`_].
* Add Model::hasUnits(UnitsPtr) method by `@nickerso <https://github.com/nickerso>`_ [`#235 <https://github.com/cellml/libcellml/pull/235>`_].
* Windows: renamed the `cellml.lib`/`cellml.dll` files to `libcellml.lib`/`libcellml.dll` by `@agarny <https://github.com/agarny>`_ [`#243 <https://github.com/cellml/libcellml/pull/243>`_].
* Make sure that `PASCAL` is not already defined by `@agarny <https://github.com/agarny>`_ [`#245 <https://github.com/cellml/libcellml/pull/245>`_].
* Tests: treat warnings as errors (#239) by `@agarny <https://github.com/agarny>`_ [`#240 <https://github.com/cellml/libcellml/pull/240>`_].
* Test CellML file: make sure we declare the CellML 2.0 namespace by `@agarny <https://github.com/agarny>`_ [`#241 <https://github.com/cellml/libcellml/pull/241>`_].
* Configuration improvements by `@hsorby <https://github.com/hsorby>`_ [`#230 <https://github.com/cellml/libcellml/pull/230>`_].
* Validation by `@hsorby <https://github.com/hsorby>`_ [`#227 <https://github.com/cellml/libcellml/pull/227>`_].
* Update internal_utils.cmake by `@hsorby <https://github.com/hsorby>`_ [`#229 <https://github.com/cellml/libcellml/pull/229>`_].
* Add resets by `@hsorby <https://github.com/hsorby>`_ [`#177 <https://github.com/cellml/libcellml/pull/177>`_].
* BUGFIX: CMake 3.11 changed the directory lookup by `@rblake-llnl <https://github.com/rblake-llnl>`_ [`#231 <https://github.com/cellml/libcellml/pull/231>`_].
* [bindings] Split Python tests into multiple functions. Closes #225 by `@MichaelClerx <https://github.com/MichaelClerx>`_ [`#226 <https://github.com/cellml/libcellml/pull/226>`_].
* SWIG Interfaces, Python bindings and tests for the Python bindings by `@MichaelClerx <https://github.com/MichaelClerx>`_ [`#205 <https://github.com/cellml/libcellml/pull/205>`_].
* Improve windows development setup documentation by `@hsorby <https://github.com/hsorby>`_ [`#223 <https://github.com/cellml/libcellml/pull/223>`_].
* Parser test: allow two different versions of an error message (#218) by `@agarny <https://github.com/agarny>`_ [`#219 <https://github.com/cellml/libcellml/pull/219>`_].
* Removed Format enum. Closes #209 by `@MichaelClerx <https://github.com/MichaelClerx>`_ [`#210 <https://github.com/cellml/libcellml/pull/210>`_].
* Added Model.replaceUnits(UnitsPtr, UnitsPtr) by `@MichaelClerx <https://github.com/MichaelClerx>`_ [`#201 <https://github.com/cellml/libcellml/pull/201>`_].
* Add ComponentEntity.replaceComponent(c1, c2) method by `@MichaelClerx <https://github.com/MichaelClerx>`_ [`#199 <https://github.com/cellml/libcellml/pull/199>`_].
* Added Component.removeVariable(size_t) method by `@MichaelClerx <https://github.com/MichaelClerx>`_ [`#200 <https://github.com/cellml/libcellml/pull/200>`_].
* Rename the `Import` class to `ImportSource` by `@agarny <https://github.com/agarny>`_ [`#191 <https://github.com/cellml/libcellml/pull/191>`_].
* adding test to illustrate #183 by `@nickerso <https://github.com/nickerso>`_ [`#184 <https://github.com/cellml/libcellml/pull/184>`_].
* Added an uninstall target (#181) by `@agarny <https://github.com/agarny>`_ [`#185 <https://github.com/cellml/libcellml/pull/185>`_].
* Clear our error handler by `@nickerso <https://github.com/nickerso>`_ [`#180 <https://github.com/cellml/libcellml/pull/180>`_].
* Move test resource definitions into a common location by `@hsorby <https://github.com/hsorby>`_ [`#173 <https://github.com/cellml/libcellml/pull/173>`_].
* New test case with complex encapsulation hierarchy by `@nickerso <https://github.com/nickerso>`_ [`#171 <https://github.com/cellml/libcellml/pull/171>`_].
* Update documentation rough object images by `@hsorby <https://github.com/hsorby>`_ [`#169 <https://github.com/cellml/libcellml/pull/169>`_].
* Remove map_components element by `@hsorby <https://github.com/hsorby>`_ [`#160 <https://github.com/cellml/libcellml/pull/160>`_].
* Adding some tests on component counts with imports by `@nickerso <https://github.com/nickerso>`_ [`#163 <https://github.com/cellml/libcellml/pull/163>`_].
* Remove ability for Component class to hold Units by `@hsorby <https://github.com/hsorby>`_ [`#159 <https://github.com/cellml/libcellml/pull/159>`_].
* Remove offset from Units by `@hsorby <https://github.com/hsorby>`_ [`#158 <https://github.com/cellml/libcellml/pull/158>`_].
* Remove base_unit attribute from expected outputs by `@hsorby <https://github.com/hsorby>`_ [`#152 <https://github.com/cellml/libcellml/pull/152>`_].
* Add configuration files and implement export configuration by `@hsorby <https://github.com/hsorby>`_ [`#146 <https://github.com/cellml/libcellml/pull/146>`_].
* Printer by `@hsorby <https://github.com/hsorby>`_ [`#144 <https://github.com/cellml/libcellml/pull/144>`_].
* No exceptions by `@hsorby <https://github.com/hsorby>`_ [`#143 <https://github.com/cellml/libcellml/pull/143>`_].
* Remove methods that return const references to smart pointers by `@hsorby <https://github.com/hsorby>`_ [`#141 <https://github.com/cellml/libcellml/pull/141>`_].
* Tidy up header file includes to the correct order by `@hsorby <https://github.com/hsorby>`_ [`#142 <https://github.com/cellml/libcellml/pull/142>`_].
* Hide private by `@hsorby <https://github.com/hsorby>`_ [`#137 <https://github.com/cellml/libcellml/pull/137>`_].
* Resolve warning from Visual Studio about dll interface for std::string by `@hsorby <https://github.com/hsorby>`_ [`#138 <https://github.com/cellml/libcellml/pull/138>`_].
* Xml indentation by `@hsorby <https://github.com/hsorby>`_ [`#135 <https://github.com/cellml/libcellml/pull/135>`_].
* Clean up inefficient multiline methods by `@hsorby <https://github.com/hsorby>`_ [`#136 <https://github.com/cellml/libcellml/pull/136>`_].
* Change CellML version from 1.2 to 2.0 by `@hsorby <https://github.com/hsorby>`_ [`#133 <https://github.com/cellml/libcellml/pull/133>`_].
* Update copyright statement by `@hsorby <https://github.com/hsorby>`_ [`#134 <https://github.com/cellml/libcellml/pull/134>`_].
* Tidy up the codebase to meet our software standards by `@hsorby <https://github.com/hsorby>`_ [`#132 <https://github.com/cellml/libcellml/pull/132>`_].
* Add tests and implementation for 'Validate' (use-case 4) by `@dladd <https://github.com/dladd>`_ [`#127 <https://github.com/cellml/libcellml/pull/127>`_].
* Change to using #pragma once for header guards by `@hsorby <https://github.com/hsorby>`_ [`#131 <https://github.com/cellml/libcellml/pull/131>`_].
* Rename generated header files.   by `@hsorby <https://github.com/hsorby>`_ [`#128 <https://github.com/cellml/libcellml/pull/128>`_].
* Add tests and implementation for 'Load` (use-cases step 3) by `@dladd <https://github.com/dladd>`_ [`#114 <https://github.com/cellml/libcellml/pull/114>`_].
* Update object models for validate step by `@dladd <https://github.com/dladd>`_ [`#121 <https://github.com/cellml/libcellml/pull/121>`_].
* Updated the link to the Google C++ style guide by `@agarny <https://github.com/agarny>`_ [`#119 <https://github.com/cellml/libcellml/pull/119>`_].
* Update use-cases diagram with proposed load XML methods by `@dladd <https://github.com/dladd>`_ [`#113 <https://github.com/cellml/libcellml/pull/113>`_].
* Implement clean-up items before 'Load' step by `@dladd <https://github.com/dladd>`_ [`#112 <https://github.com/cellml/libcellml/pull/112>`_].
* Add tests and implementation for 'Modify' (use-case 2) by `@dladd <https://github.com/dladd>`_ [`#109 <https://github.com/cellml/libcellml/pull/109>`_].
* Update use-cases object model to Modify step (2) by `@dladd <https://github.com/dladd>`_ [`#108 <https://github.com/cellml/libcellml/pull/108>`_].
* Implement clean-up items before 'Modify' step by `@dladd <https://github.com/dladd>`_ [`#107 <https://github.com/cellml/libcellml/pull/107>`_].
* Add documentation for the use of labels by `@dladd <https://github.com/dladd>`_ [`#106 <https://github.com/cellml/libcellml/pull/106>`_].
* Changes to unit prefix handling by `@dladd <https://github.com/dladd>`_ [`#103 <https://github.com/cellml/libcellml/pull/103>`_].
* Clean up the documentation (#92) by `@agarny <https://github.com/agarny>`_ [`#93 <https://github.com/cellml/libcellml/pull/93>`_].
* Cleanup minor issues from math implementation pull by `@dladd <https://github.com/dladd>`_ [`#102 <https://github.com/cellml/libcellml/pull/102>`_].
* Add tests and implementation for maths (use-case 1.xiii-xiv) by `@dladd <https://github.com/dladd>`_ [`#98 <https://github.com/cellml/libcellml/pull/98>`_].
* Swap method/attribute names from 'mathString' to 'math' in object model by `@dladd <https://github.com/dladd>`_ [`#101 <https://github.com/cellml/libcellml/pull/101>`_].
* Bugfix: remove extraneous text from Apache license by `@dladd <https://github.com/dladd>`_ [`#100 <https://github.com/cellml/libcellml/pull/100>`_].
* Add maths use-cases to doc by `@dladd <https://github.com/dladd>`_ [`#96 <https://github.com/cellml/libcellml/pull/96>`_].
* Add tests and implementation for connections (use-case 1.xii) by `@dladd <https://github.com/dladd>`_ [`#89 <https://github.com/cellml/libcellml/pull/89>`_].
* Add connections to use-cases object model by `@dladd <https://github.com/dladd>`_ [`#85 <https://github.com/cellml/libcellml/pull/85>`_].
* CMake files: some minor cleaning up (#83) by `@agarny <https://github.com/agarny>`_ [`#84 <https://github.com/cellml/libcellml/pull/84>`_].
* Add funding acknowledgement by `@dladd <https://github.com/dladd>`_ [`#91 <https://github.com/cellml/libcellml/pull/91>`_].
* Tests and implementation for variable interfaces by `@dladd <https://github.com/dladd>`_ [`#80 <https://github.com/cellml/libcellml/pull/80>`_].
* Nicer target presentation for Visual Studio by `@hsorby <https://github.com/hsorby>`_ [`#70 <https://github.com/cellml/libcellml/pull/70>`_].
* Some further cleaning up by `@agarny <https://github.com/agarny>`_ [`#81 <https://github.com/cellml/libcellml/pull/81>`_].
* The treatment of MathML in libCellML by `@nickerso <https://github.com/nickerso>`_ [`#66 <https://github.com/cellml/libcellml/pull/66>`_].
* Add interface use-cases to variable use-case item by `@dladd <https://github.com/dladd>`_ [`#78 <https://github.com/cellml/libcellml/pull/78>`_].
* Code tidy-up by `@dladd <https://github.com/dladd>`_ [`#76 <https://github.com/cellml/libcellml/pull/76>`_].
* Update use-cases object model with variable interfaces by `@dladd <https://github.com/dladd>`_ [`#79 <https://github.com/cellml/libcellml/pull/79>`_].
* CMake: replaced obsolete use of add_compiler_export_flags() function (#72) by `@agarny <https://github.com/agarny>`_ [`#73 <https://github.com/cellml/libcellml/pull/73>`_].
* Supporting variable initial values (part of use-cases 1.xi) by `@dladd <https://github.com/dladd>`_ [`#68 <https://github.com/cellml/libcellml/pull/68>`_].
* Bugfix: enable variable serialisation in unnamed component by `@dladd <https://github.com/dladd>`_ [`#71 <https://github.com/cellml/libcellml/pull/71>`_].
* Supporting use-case 1.xi (a-c): a model with variables by `@dladd <https://github.com/dladd>`_ [`#64 <https://github.com/cellml/libcellml/pull/64>`_].
* Update use cases: add variable initial values and modify connections by `@dladd <https://github.com/dladd>`_ [`#67 <https://github.com/cellml/libcellml/pull/67>`_].
* Proposed object model for use-cases 1.i-1.xi by `@dladd <https://github.com/dladd>`_ [`#62 <https://github.com/cellml/libcellml/pull/62>`_].
* Re-order use-cases doc to modify models before loading by `@dladd <https://github.com/dladd>`_ [`#60 <https://github.com/cellml/libcellml/pull/60>`_].
* Usecase 1.x by `@hsorby <https://github.com/hsorby>`_ [`#58 <https://github.com/cellml/libcellml/pull/58>`_].
* Add initial tests for Units, including skeleton Units class by `@hsorby <https://github.com/hsorby>`_ [`#52 <https://github.com/cellml/libcellml/pull/52>`_].
* Add corrections to object model as noted by @agarny by `@hsorby <https://github.com/hsorby>`_ [`#55 <https://github.com/cellml/libcellml/pull/55>`_].
* Add a short document on current thinking by `@hsorby <https://github.com/hsorby>`_ [`#56 <https://github.com/cellml/libcellml/pull/56>`_].
* Adjust use cases to match current thinking by `@hsorby <https://github.com/hsorby>`_ [`#51 <https://github.com/cellml/libcellml/pull/51>`_].
* Update Object model images by `@hsorby <https://github.com/hsorby>`_ [`#50 <https://github.com/cellml/libcellml/pull/50>`_].
* Add test for non-existent url by `@hsorby <https://github.com/hsorby>`_ [`#47 <https://github.com/cellml/libcellml/pull/47>`_].
* Change the usecases of imports to reflect the latest developments in … by `@hsorby <https://github.com/hsorby>`_ [`#46 <https://github.com/cellml/libcellml/pull/46>`_].
* starting to play around with options for component imports by `@nickerso <https://github.com/nickerso>`_ [`#45 <https://github.com/cellml/libcellml/pull/45>`_].
* Implementation for usecases upto 1vii smart by `@hsorby <https://github.com/hsorby>`_ [`#42 <https://github.com/cellml/libcellml/pull/42>`_].
* Implementation for usecases upto 1vii by `@hsorby <https://github.com/hsorby>`_ [`#36 <https://github.com/cellml/libcellml/pull/36>`_].
* Docs by `@hsorby <https://github.com/hsorby>`_ [`#40 <https://github.com/cellml/libcellml/pull/40>`_].
* New use cases for modification of model and component hierarchy by `@nickerso <https://github.com/nickerso>`_ [`#34 <https://github.com/cellml/libcellml/pull/34>`_].
* Prefer gcovr over gcovcoverage by `@hsorby <https://github.com/hsorby>`_ [`#37 <https://github.com/cellml/libcellml/pull/37>`_].
* Implementation for use cases upto 1.vi by `@hsorby <https://github.com/hsorby>`_ [`#30 <https://github.com/cellml/libcellml/pull/30>`_].
* Usecases 1ii 1iii by `@hsorby <https://github.com/hsorby>`_ [`#26 <https://github.com/cellml/libcellml/pull/26>`_].
* Proposed rough object model for use cases 1.i - 1.vi by `@hsorby <https://github.com/hsorby>`_ [`#28 <https://github.com/cellml/libcellml/pull/28>`_].
* Usecase 1i by `@hsorby <https://github.com/hsorby>`_ [`#22 <https://github.com/cellml/libcellml/pull/22>`_].
* Add rules around variable naming by `@hsorby <https://github.com/hsorby>`_ [`#24 <https://github.com/cellml/libcellml/pull/24>`_].
* Fix use case png file name to suit Sphinx pdf generation by `@hsorby <https://github.com/hsorby>`_ [`#21 <https://github.com/cellml/libcellml/pull/21>`_].
* Add initial documentation by `@hsorby <https://github.com/hsorby>`_ [`#14 <https://github.com/cellml/libcellml/pull/14>`_].
* Scope by `@hsorby <https://github.com/hsorby>`_ [`#18 <https://github.com/cellml/libcellml/pull/18>`_].
* Requirements check by `@hsorby <https://github.com/hsorby>`_ [`#17 <https://github.com/cellml/libcellml/pull/17>`_].
* Set default options for develop branch by `@hsorby <https://github.com/hsorby>`_ [`#16 <https://github.com/cellml/libcellml/pull/16>`_].
* Add basic object model documentation by `@hsorby <https://github.com/hsorby>`_ [`#10 <https://github.com/cellml/libcellml/pull/10>`_].
* Minor fixes by `@hsorby <https://github.com/hsorby>`_ [`#8 <https://github.com/cellml/libcellml/pull/8>`_].
* Adding initial code stub with tests by `@hsorby <https://github.com/hsorby>`_ [`#6 <https://github.com/cellml/libcellml/pull/6>`_].
* Adding a fused gtest-1.7.0.  Also added a CMakeLists.txt file for use by.. by `@hsorby <https://github.com/hsorby>`_ [`#5 <https://github.com/cellml/libcellml/pull/5>`_].
* Adding README.rst and .gitignore by `@hsorby <https://github.com/hsorby>`_ [`#4 <https://github.com/cellml/libcellml/pull/4>`_].

Contributors
------------

.. image:: https://avatars.githubusercontent.com/u/811244?v=4
   :target: https://github.com/nickerso
   :height: 32
   :width: 32
.. image:: https://avatars.githubusercontent.com/u/811296?v=4
   :target: https://github.com/dladd
   :height: 32
   :width: 32
.. image:: https://avatars.githubusercontent.com/u/602265?v=4
   :target: https://github.com/agarny
   :height: 32
   :width: 32
.. image:: https://avatars.githubusercontent.com/u/778048?v=4
   :target: https://github.com/hsorby
   :height: 32
   :width: 32
.. image:: https://avatars.githubusercontent.com/u/24722037?v=4
   :target: https://github.com/rblake-llnl
   :height: 32
   :width: 32
.. image:: https://avatars.githubusercontent.com/u/3896149?v=4
   :target: https://github.com/TedAhmadi
   :height: 32
   :width: 32
.. image:: https://avatars.githubusercontent.com/u/517644?v=4
   :target: https://github.com/MichaelClerx
   :height: 32
   :width: 32
