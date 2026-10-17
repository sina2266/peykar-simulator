Options
-------

Some options are platform or tool dependent.
For example, :code:`MEMCHECK` is only available if the valgrind tool is available.

======================== ============ ===========================================
Config                    Default      Description
======================== ============ ===========================================
:code:`BUILD_SHARED`      ON           Build shared libraries (so, dylib, DLLs).
------------------------ ------------ -------------------------------------------
:code:`BUILD_TYPE`        Release      The type of build Release, Debug, etc.
------------------------ ------------ -------------------------------------------
:code:`COMPILER_CACHE`    ON           Enable compiler cache (if available).
------------------------ ------------ -------------------------------------------
:code:`COVERAGE`          ON           Enable coverage testing (if available).
------------------------ ------------ -------------------------------------------
:code:`ENABLE_UTILITIES`  OFF          Enable utility targets (if available).
------------------------ ------------ -------------------------------------------
:code:`INSTALL_PREFIX`    /usr/lib     Install path prefix (platform specific).
------------------------ ------------ -------------------------------------------
:code:`MEMCHECK`          ON           Enable memcheck testing (if available).
------------------------ ------------ -------------------------------------------
:code:`TWAE`              ON           Treat warnings as errors.
------------------------ ------------ -------------------------------------------
:code:`UNIT_TESTS`        ON           Enable tests.
======================== ============ ===========================================


.. note In CMake GUI Configuration applications the config variable is prefixed with 'LIBCELLML\_'

.. ================================== ================ =========================================
.. GUI Config                         CLI Config       Description
.. ================================== ================ =========================================
.. LIBCELLML_BUILD_SHARED             BUILD_SHARED     Build shared libraries (so, dylib, DLLs).
.. ---------------------------------- ---------------- -----------------------------------------
.. LIBCELLML_BUILD_TYPE               BUILD_TYPE       The type of build Release, Debug, etc.
.. ---------------------------------- ---------------- -----------------------------------------
.. LIBCELML_COMPILER_CACHE            COMPILER_CACHE   Enable compiler cache (if available).
.. ---------------------------------- ---------------- -----------------------------------------
.. LIBCELML_COVERAGE                  COVERAGE         Enable coverage testing (if available).
.. ---------------------------------- ---------------- -----------------------------------------
.. LIBCELML_ENABLE_UTILITIES          ENABLE_UTILITIES Enable utility targets (if available).
.. ---------------------------------- ---------------- -----------------------------------------
.. LIBCELLML_INSTALL_PREFIX           INSTALL_PREFIX   Install path prefix.
.. ---------------------------------- ---------------- -----------------------------------------
.. LIBCELLML_MEMCHECK                 MEMCHECK         Enable memcheck testing (if available).
.. ---------------------------------- ---------------- -----------------------------------------
.. LIBCELLML_TREAT_WARNINGS_AS_ERRORS TWAE             Treat warnings as errors.
.. ---------------------------------- ---------------- -----------------------------------------
.. LIBCELLML_UNIT_TESTS               UNIT_TESTS       Enable tests.
.. ================================== ================ =========================================
