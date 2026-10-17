libCellML v0.6.3 Changelog
==========================

Analyser
--------

* Analyser: make sure that we are dealing with a power function when units maps are not dimensionless by `@agarny <https://github.com/agarny>`_ [`#1291 <https://github.com/cellml/libcellml/pull/1291>`_].

Infrastructure
--------------

* Still require this undefine for older macOS by `@hsorby <https://github.com/hsorby>`_ [`#1299 <https://github.com/cellml/libcellml/pull/1299>`_].
* Fix static build and test on Linux by `@hsorby <https://github.com/hsorby>`_ [`#1292 <https://github.com/cellml/libcellml/pull/1292>`_].
* SWIG: make it work with Clang 19 by `@agarny <https://github.com/agarny>`_ [`#1250 <https://github.com/cellml/libcellml/pull/1250>`_].
* Replace use of mikehardy/buildcache-action@v2 with cscouto/buildcache-action@v1 by `@hsorby <https://github.com/hsorby>`_ [`#1296 <https://github.com/cellml/libcellml/pull/1296>`_].
* Deploy on release updates by `@hsorby <https://github.com/hsorby>`_ [`#1287 <https://github.com/cellml/libcellml/pull/1287>`_].
* CI: add some "basic" GitHub Actions by `@agarny <https://github.com/agarny>`_ [`#1254 <https://github.com/cellml/libcellml/pull/1254>`_].

Modified feature
----------------

*  Tidy analyseNode by `@hsorby <https://github.com/hsorby>`_ [`#1295 <https://github.com/cellml/libcellml/pull/1295>`_].

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
