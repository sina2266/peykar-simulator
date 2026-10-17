cmake_minimum_required(VERSION 3.13)

# Python bindings for VCellMovingBoundary via pybind11.
# pybind11 and Python3 are found in the parent CMakeLists.txt.
#
# The bindings are shipped as the `pyvcell_mbsolver` package: the compiled
# extension is the private submodule `pyvcell_mbsolver._core`, and the
# pure-Python wrapper is the package's __init__.py.

set(MB_PY_PACKAGE pyvcell_mbsolver)

pybind11_add_module(_core
    pyvcellmbsolver.cpp
)

target_link_libraries(_core
    PRIVATE
        MovingBoundaryLib
        vcommons        # for tinyxml2
)

# Assemble the package in the build tree (bin/pyvcell_mbsolver/) so it is
# importable as `pyvcell_mbsolver` with PYTHONPATH=bin, without installing.
set(MB_PY_BUILD_DIR ${CMAKE_BINARY_DIR}/bin/${MB_PY_PACKAGE})
set_target_properties(_core PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${MB_PY_BUILD_DIR}
)
# Multi-config generators (Visual Studio) append a per-config subdirectory
# (e.g. Release/) to LIBRARY_OUTPUT_DIRECTORY, which would place _core.pyd in
# bin/pyvcell_mbsolver/Release/ while __init__.py is copied to
# bin/pyvcell_mbsolver/ — breaking `import pyvcell_mbsolver._core`. Pin the
# per-config output dirs so the extension always sits next to __init__.py.
foreach(_cfg ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${_cfg} _cfg_upper)
    set_target_properties(_core PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY_${_cfg_upper} ${MB_PY_BUILD_DIR}
        RUNTIME_OUTPUT_DIRECTORY_${_cfg_upper} ${MB_PY_BUILD_DIR}
    )
endforeach()
add_custom_command(TARGET _core POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/${MB_PY_PACKAGE}/__init__.py
            ${MB_PY_BUILD_DIR}/__init__.py
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/${MB_PY_PACKAGE}/py.typed
            ${MB_PY_BUILD_DIR}/py.typed
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/${MB_PY_PACKAGE}/_core.pyi
            ${MB_PY_BUILD_DIR}/_core.pyi
    COMMENT "Copying pyvcell_mbsolver/{__init__.py,py.typed,_core.pyi} into the build tree"
)

# When building a wheel via scikit-build-core, SKBUILD is defined and the
# install prefix already points inside the wheel's platlib, so destinations
# must be relative. For a plain `cmake --install`, install into site-packages.
if(SKBUILD)
    set(MB_PY_INSTALL_DEST "${MB_PY_PACKAGE}")
else()
    set(MB_PY_INSTALL_DEST "${Python3_SITEARCH}/${MB_PY_PACKAGE}")
endif()

install(TARGETS _core
    LIBRARY DESTINATION ${MB_PY_INSTALL_DEST}
    COMPONENT python
)

# Install the pure-Python wrapper (the package __init__.py), the PEP 561
# py.typed marker, and the _core type stub so type checkers use the package's
# inline annotations and the compiled extension's signatures.
install(FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/${MB_PY_PACKAGE}/__init__.py
        ${CMAKE_CURRENT_SOURCE_DIR}/${MB_PY_PACKAGE}/py.typed
        ${CMAKE_CURRENT_SOURCE_DIR}/${MB_PY_PACKAGE}/_core.pyi
    DESTINATION ${MB_PY_INSTALL_DEST}
    COMPONENT python
)

# ---------------------------------------------------------------------------
# Python tests (pytest) — run inside a dedicated virtual environment.
#
# The venv is created (or refreshed) at configure time in the build tree so
# the test environment is reproducible and isolated from the system Python.
# pytest is installed into the venv; the extension .so is made importable
# via PYTHONPATH rather than by installing it into the venv.
# ---------------------------------------------------------------------------
if(BUILD_TESTING)
    set(VENV_DIR ${CMAKE_BINARY_DIR}/python_venv)

    # Platform-specific paths inside the venv
    if(WIN32)
        set(VENV_PYTHON ${VENV_DIR}/Scripts/python.exe)
        set(VENV_PIP    ${VENV_DIR}/Scripts/pip.exe)
    else()
        set(VENV_PYTHON ${VENV_DIR}/bin/python)
        set(VENV_PIP    ${VENV_DIR}/bin/pip)
    endif()

    # Create the venv (idempotent: safe to re-run on reconfigure)
    execute_process(
        COMMAND ${Python3_EXECUTABLE} -m venv ${VENV_DIR}
        RESULT_VARIABLE _venv_rc
    )
    if(NOT _venv_rc EQUAL 0)
        message(WARNING "Python venv creation failed (exit ${_venv_rc}). "
                        "Python tests may not run correctly.")
    endif()

    # Install pytest into the venv
    execute_process(
        COMMAND ${VENV_PIP} install --quiet pytest
        RESULT_VARIABLE _pip_rc
    )
    if(NOT _pip_rc EQUAL 0)
        message(WARNING "pip install pytest failed (exit ${_pip_rc}). "
                        "Python tests may not run correctly.")
    endif()

    # --import-mode=importlib keeps pytest from prepending the source tree to
    # sys.path, which would otherwise shadow the built pyvcell_mbsolver package
    # (the source dir has the wrapper but not the compiled _core extension).
    add_test(
        NAME PyVcellMbSolver
        COMMAND ${VENV_PYTHON} -m pytest
                ${CMAKE_CURRENT_SOURCE_DIR}/tests
                -v --tb=short --import-mode=importlib
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    # PYTHONPATH points at bin/, where the assembled `pyvcell_mbsolver` package
    # (_core extension + __init__.py) lives, so it is importable without
    # installing into the venv.
    set_tests_properties(PyVcellMbSolver PROPERTIES
        ENVIRONMENT
            "PYTHONPATH=${CMAKE_BINARY_DIR}/bin"
    )
endif()
