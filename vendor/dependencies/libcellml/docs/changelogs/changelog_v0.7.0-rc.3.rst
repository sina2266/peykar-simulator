libCellML v0.7.0-rc.3 Changelog
===============================

Analyser
--------

* Analyser: account for an underconstrained NLA system by `@agarny <https://github.com/agarny>`_ [`#1338 <https://github.com/cellml/libcellml/pull/1338>`_].

Bug
---

* Fix bug with litre units by `@hsorby <https://github.com/hsorby>`_ [`#1314 <https://github.com/cellml/libcellml/pull/1314>`_].

Documentation
-------------

* Fix warnings when building the documentation with Doxygen by `@hsorby <https://github.com/hsorby>`_ [`#1349 <https://github.com/cellml/libcellml/pull/1349>`_].
* Updating citation information for libCellML by `@nickerso <https://github.com/nickerso>`_ [`#1361 <https://github.com/cellml/libcellml/pull/1361>`_].

Generator
---------

* Generator profile: fix `computeComputedConstants()`'s signature by `@agarny <https://github.com/agarny>`_ [`#1364 <https://github.com/cellml/libcellml/pull/1364>`_].
* Add additional overloads to `Generator` class implementationCode and interfaceCode methods by `@hsorby <https://github.com/hsorby>`_ [`#1345 <https://github.com/cellml/libcellml/pull/1345>`_].
* Add a `GeneratorVariableTracker` class to manage un/tracked variables by `@agarny <https://github.com/agarny>`_ [`#1340 <https://github.com/cellml/libcellml/pull/1340>`_].
* Allow a constant, a computed constant, or a state variable (but not an algebraic variable) to be used as an initial value by `@agarny <https://github.com/agarny>`_ [`#1325 <https://github.com/cellml/libcellml/pull/1325>`_].
* Code generator: allow for variables to be tracked/untracked by `@agarny <https://github.com/agarny>`_ [`#1256 <https://github.com/cellml/libcellml/pull/1256>`_].
* Code generator: use different arrays for constants, computed constants, and algebraic variables by `@agarny <https://github.com/agarny>`_ [`#1247 <https://github.com/cellml/libcellml/pull/1247>`_].
* Syntax error in generated Python code by `@agarny <https://github.com/agarny>`_ [`#1334 <https://github.com/cellml/libcellml/pull/1334>`_].
* API: simplify/improve the API for external variables by `@agarny <https://github.com/agarny>`_ [`#1252 <https://github.com/cellml/libcellml/pull/1252>`_].

Infrastructure
--------------

* Don't push changes to main too early in finalise workflow by `@hsorby <https://github.com/hsorby>`_ [`#1433 <https://github.com/cellml/libcellml/pull/1433>`_].
* Reset working directory when ccloning passed documentation repositories by `@hsorby <https://github.com/hsorby>`_ [`#1432 <https://github.com/cellml/libcellml/pull/1432>`_].
* Revert to windows-2022 to avoid issues with windows-2025 by `@hsorby <https://github.com/hsorby>`_ [`#1431 <https://github.com/cellml/libcellml/pull/1431>`_].
* Try a different approach to setting permissions for make release workflow by `@hsorby <https://github.com/hsorby>`_ [`#1430 <https://github.com/cellml/libcellml/pull/1430>`_].
* Re-instate final fixes to release process workflow overwritten during testing by `@hsorby <https://github.com/hsorby>`_ [`#1428 <https://github.com/cellml/libcellml/pull/1428>`_].
* Perform final sanity check before pushing release changes to main by `@hsorby <https://github.com/hsorby>`_ [`#1427 <https://github.com/cellml/libcellml/pull/1427>`_].
* Correct conditional syntax in finalise release workflow by `@hsorby <https://github.com/hsorby>`_ [`#1426 <https://github.com/cellml/libcellml/pull/1426>`_].
* Skip user documentation update if the documentation tests failed by `@hsorby <https://github.com/hsorby>`_ [`#1425 <https://github.com/cellml/libcellml/pull/1425>`_].
* Fix more references to libcellml/website-src by `@hsorby <https://github.com/hsorby>`_ [`#1424 <https://github.com/cellml/libcellml/pull/1424>`_].
* Fix the way commits for release are pushed to main by `@hsorby <https://github.com/hsorby>`_ [`#1423 <https://github.com/cellml/libcellml/pull/1423>`_].
* Set default summary if documentation checks returns nothing by `@hsorby <https://github.com/hsorby>`_ [`#1422 <https://github.com/cellml/libcellml/pull/1422>`_].
* Fix missing installation of ResolveZLIB.cmake file by `@hsorby <https://github.com/hsorby>`_ [`#1421 <https://github.com/cellml/libcellml/pull/1421>`_].
* Set identity of Git user to github actions bot for tagging release source by `@hsorby <https://github.com/hsorby>`_ [`#1419 <https://github.com/cellml/libcellml/pull/1419>`_].
* Correctly tag source of release by `@hsorby <https://github.com/hsorby>`_ [`#1418 <https://github.com/cellml/libcellml/pull/1418>`_].
* Switch to release branch before committing changelog changes by `@hsorby <https://github.com/hsorby>`_ [`#1416 <https://github.com/cellml/libcellml/pull/1416>`_].
* Remove -v from git tag -a command in release-prepare.yml by `@hsorby <https://github.com/hsorby>`_ [`#1415 <https://github.com/cellml/libcellml/pull/1415>`_].
* Set a source tag and use that to determine which merge commits to consider by `@hsorby <https://github.com/hsorby>`_ [`#1414 <https://github.com/cellml/libcellml/pull/1414>`_].
* Tweaks to release process, investigating changelog creation issue by `@hsorby <https://github.com/hsorby>`_ [`#1412 <https://github.com/cellml/libcellml/pull/1412>`_].
* Adjust finalise release workflow by `@hsorby <https://github.com/hsorby>`_ [`#1411 <https://github.com/cellml/libcellml/pull/1411>`_].
* Release process fixes by `@hsorby <https://github.com/hsorby>`_ [`#1409 <https://github.com/cellml/libcellml/pull/1409>`_].
* Add finalise release workflow by `@hsorby <https://github.com/hsorby>`_ [`#1405 <https://github.com/cellml/libcellml/pull/1405>`_].
* Add make release workflow by `@hsorby <https://github.com/hsorby>`_ [`#1404 <https://github.com/cellml/libcellml/pull/1404>`_].
* Add check release workflow by `@hsorby <https://github.com/hsorby>`_ [`#1403 <https://github.com/cellml/libcellml/pull/1403>`_].
* Add GitHub action to prepare a release by `@hsorby <https://github.com/hsorby>`_ [`#1394 <https://github.com/cellml/libcellml/pull/1394>`_].
* CI by `@agarny <https://github.com/agarny>`_ [`#1315 <https://github.com/cellml/libcellml/pull/1315>`_].

JavaScript bindings
-------------------

* JavaScript: account for the fact that our JavaScript bindings use a 32-bit address space by `@agarny <https://github.com/agarny>`_ [`#1384 <https://github.com/cellml/libcellml/pull/1384>`_].

Miscellaneous
-------------

* Remove all unnecessary headers as guided by Qt Creator on macOS by `@hsorby <https://github.com/hsorby>`_ [`#1346 <https://github.com/cellml/libcellml/pull/1346>`_].

Contributors
------------

.. image:: https://avatars.githubusercontent.com/u/602265?v=4
   :target: https://github.com/agarny
   :height: 32
   :width: 32
.. image:: https://avatars.githubusercontent.com/u/778048?v=4
   :target: https://github.com/hsorby
   :height: 32
   :width: 32
.. image:: https://avatars.githubusercontent.com/u/811244?v=4
   :target: https://github.com/nickerso
   :height: 32
   :width: 32
