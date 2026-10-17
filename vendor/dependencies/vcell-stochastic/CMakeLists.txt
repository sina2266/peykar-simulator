cmake_minimum_required(VERSION 3.15)
project(VCellStochastic VERSION 1.0.0 LANGUAGES CXX C)

# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Default build type for single-config generators
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Options
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
option(OPTION_TARGET_MESSAGING "Enable messaging support" OFF)
option(OPTION_BUILD_PYTHON_BINDINGS "Build Python bindings using pybind11" OFF)

# scikit-build-core (pip wheel / cibuildwheel) only ever builds this project
# to produce the Python extension module.
if(SKBUILD)
    set(OPTION_BUILD_PYTHON_BINDINGS ON CACHE BOOL "Build Python bindings using pybind11" FORCE)
endif()

# Platform-specific compatibility
if(MSVC)
    add_compile_definitions(
            HAVE_UNISTD_H=0
            NOMINMAX
            _CRT_SECURE_NO_WARNINGS
    )
elseif(APPLE)
    # Architectures come from CMAKE_OSX_ARCHITECTURES (e.g. -DCMAKE_OSX_ARCHITECTURES=x86_64);
    # left unset, the build targets the host. The release builds each slice separately and
    # joins them with lipo (see ci/), because HDF5 is built per architecture.
elseif(UNIX)
    # -m64 is an x86-only option; GCC rejects it outright on other
    # architectures, such as the aarch64 manylinux images.
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64|AMD64|i[3-6]86)$")
        add_compile_options(-m64)
    endif()
    add_compile_options(-fpermissive)
endif()

# Packages
find_package(Threads REQUIRED)
# Only the HDF5 C API is used (MultiTrialStats). Release builds link a static HDF5
# built by ci/build-hdf5.sh (pass -DHDF5_ROOT=<prefix> -DHDF5_USE_STATIC_LIBRARIES=ON).
find_package(HDF5 REQUIRED COMPONENTS C)

# Optional messaging dependency
if(OPTION_TARGET_MESSAGING)
    find_package(CURL REQUIRED)
endif()

# Optional Python binding dependencies
if(OPTION_BUILD_PYTHON_BINDINGS)
    if(SKBUILD)
        # A wheel's extension module only needs Development.Module; the full
        # Development component additionally requires a libpython to embed,
        # which the manylinux/cibuildwheel images do not ship.
        find_package(Python3 REQUIRED COMPONENTS Interpreter Development.Module)
    else()
        find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
    endif()
    find_package(pybind11 CONFIG REQUIRED)
endif()

# Apache Arrow / Parquet setup
if(OPTION_USE_PARQUET)
    # First try to find Arrow/Parquet via CMAKE_PREFIX_PATH (e.g., from vcpkg)
    find_package(Arrow QUIET)
    find_package(Parquet QUIET)
    
    # If not found, check if ARROW_INSTALL_DIR is provided
    if(NOT Arrow_FOUND AND ARROW_INSTALL_DIR)
        message(STATUS "Using pre-built Apache Arrow from: ${ARROW_INSTALL_DIR}")
        set(CMAKE_PREFIX_PATH "${ARROW_INSTALL_DIR};${CMAKE_PREFIX_PATH}")
        find_package(Arrow REQUIRED)
        find_package(Parquet REQUIRED)
    elseif(NOT Arrow_FOUND)
        message(FATAL_ERROR "OPTION_USE_PARQUET requires either CMAKE_PREFIX_PATH to be set (e.g., vcpkg installed location) or ARROW_INSTALL_DIR to be set. Example: -DARROW_INSTALL_DIR='C:/arrow-install'")
    endif()
        
    message(STATUS "Multi-trial output format: Parquet")
else()
    message(STATUS "Multi-trial output format: HDF5")
endif()

# Version string printed by the executable ("Stochastic simulation version ...").
# Release builds pass -DGIT_DESCRIBE=vX.Y.Z; otherwise ask git.
if(NOT DEFINED GIT_DESCRIBE OR GIT_DESCRIBE STREQUAL "")
    find_package(Git QUIET)
    if(GIT_FOUND)
        execute_process(COMMAND ${GIT_EXECUTABLE} describe --tags --match "v*" --always --dirty
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                OUTPUT_VARIABLE GIT_DESCRIBE OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
    endif()
    if(NOT GIT_DESCRIBE)
        set(GIT_DESCRIBE "v${PROJECT_VERSION}")
    endif()
endif()
message(STATUS "VCellStoch version string: ${GIT_DESCRIBE}")

# Dependencies (vcell-expressionparser and vcell-messaging are git submodules)
add_subdirectory(vcell-expressionparser)
add_subdirectory(vcommons)
add_subdirectory(vcell-messaging)
add_subdirectory(VCellStoch)

# Testing
include(CTest)
if(BUILD_TESTING)
    add_subdirectory(Tests)
endif()

if(OPTION_BUILD_PYTHON_BINDINGS)
    add_subdirectory(python)
endif()

