project(vcellstochastic_python LANGUAGES CXX)

# This file assumes you will add a binding source file later, such as:
#   src/bindings.cpp
#
# For now, this directory is only active when OPTION_BUILD_PYTHON_BINDINGS=ON.

pybind11_add_module(vcellstochastic_py
        src/bindings.cpp
)

target_link_libraries(vcellstochastic_py
        PRIVATE
        VCellStochLib
)

target_compile_features(vcellstochastic_py PRIVATE cxx_std_14)

# Keep the link-time library directories (HDF5 in particular) on the installed
# module's RPATH so the wheel repair tools -- delocate on macOS, auditwheel on
# Linux -- can resolve those libraries and vendor them into the wheel.
set_target_properties(vcellstochastic_py PROPERTIES
        INSTALL_RPATH_USE_LINK_PATH TRUE
)

target_include_directories(vcellstochastic_py PRIVATE
        ${PROJECT_SOURCE_DIR}/VCellStoch/include
        ${PROJECT_SOURCE_DIR}/vcell-expressionparser/include
        ${PROJECT_SOURCE_DIR}/vcommons/include
        ${PROJECT_SOURCE_DIR}/vcell-messaging/include
)

if(BUILD_TESTING)
    add_test(
        NAME TestPythonBindings
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/test_binding.py
    )
    set_tests_properties(TestPythonBindings PROPERTIES
        ENVIRONMENT "PYTHONPATH=${CMAKE_BINARY_DIR}/bin:${CMAKE_SOURCE_DIR}/python/src"
    )
endif()

if(SKBUILD)
    # The "python" component is the only one scikit-build-core installs into
    # the wheel (see install.components in pyproject.toml). The extension and
    # its pure-Python wrapper both land at the top level of site-packages,
    # matching the layout the in-tree PYTHONPATH already provides.
    install(TARGETS vcellstochastic_py
            COMPONENT python
            DESTINATION .
    )
    install(FILES ${PROJECT_SOURCE_DIR}/src/vcellstochastic.py
            COMPONENT python
            DESTINATION .
    )
endif()
