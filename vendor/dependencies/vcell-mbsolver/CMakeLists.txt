cmake_minimum_required(VERSION 3.13)
project(VCellMovingBoundary VERSION 1.0.5 LANGUAGES CXX C)

# C++ standard.  vcell-messaging (2.0) uses std::format and declares
# `target_compile_features(vcellmessaging PUBLIC cxx_std_20)`, so everything
# linking it is compiled as C++20 regardless; set it globally so the bundled
# libraries are built consistently.
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
option(VARIABLE_SPECIES_STORAGE "Dynamic species storage" OFF)
option(OPTION_TARGET_MESSAGING "Enable job-status messaging via CURL" OFF)
# The release archives (SOLVER-RELEASE in README.md) build only the executable,
# against a static HDF5 found through its CMake package config; they need
# neither the Python bindings nor FindHDF5's compiler-wrapper lookup.
option(MB_BUILD_PYTHON "Build the pybind11 Python bindings (pyvcell_mbsolver._core)" ON)
option(MB_HDF5_CONFIG "Find HDF5 through its CMake package config (e.g. a static HDF5 build)" OFF)

if(VARIABLE_SPECIES_STORAGE)
    add_compile_definitions(MB_VARY_MASS)
endif()

# Platform-specific settings
if(MSVC)
    add_compile_definitions(
        HAVE_UNISTD_H=0
        NOMINMAX
        _CRT_SECURE_NO_WARNINGS
        WIN32_LEAN_AND_MEAN
        # Drop <wingdi.h> so its `typedef ... FIXED` cannot clash with FronTier's
        # `FIXED` enum (front/fuserint.h) no matter which header pulls in
        # <windows.h> first.
        NOGDI
    )
elseif(APPLE)
    # Default to the host architecture; an explicit -DCMAKE_OSX_ARCHITECTURES
    # (the release build passes one per slice before lipo) is left alone.
    if(NOT CMAKE_OSX_ARCHITECTURES)
        execute_process(COMMAND uname -m OUTPUT_VARIABLE PROCESSOR_ARCHITECTURE OUTPUT_STRIP_TRAILING_WHITESPACE)
        if(PROCESSOR_ARCHITECTURE STREQUAL "arm64")
            add_compile_definitions(__arm64__)
            set(CMAKE_OSX_ARCHITECTURES "arm64")
        else()
            add_compile_definitions(__x86_64__)
            set(CMAKE_OSX_ARCHITECTURES "x86_64")
        endif()
    endif()
    add_compile_options(-fpermissive)
elseif(UNIX)
    add_compile_options(-fpermissive)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
        add_compile_options(-m64)
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64|ARM64")
        # plain `char` is unsigned on Linux/aarch64; the code (FronTier and the
        # tests alike) was written on x86, where it is signed.
        add_compile_options(-fsigned-char)
    endif()
endif()

# CURL (required if messaging is enabled)
if(OPTION_TARGET_MESSAGING)
    find_package(CURL REQUIRED)
endif()

# Required packages
find_package(Threads REQUIRED)

# HDF5: when vcpkg is active, try CONFIG mode first (vcpkg exposes HDF5 via
# CONFIG only; the MODULE/wrapper path fails there).  On Linux/macOS with
# system packages, MODULE mode correctly requests the CXX component and
# populates the legacy variables, so we skip CONFIG mode on those platforms.
if(VCPKG_TOOLCHAIN OR MB_HDF5_CONFIG)
    find_package(HDF5 CONFIG QUIET)
endif()
if(MB_HDF5_CONFIG AND NOT HDF5_FOUND)
    message(FATAL_ERROR "MB_HDF5_CONFIG=ON but no HDF5 package config was found (set HDF5_DIR or CMAKE_PREFIX_PATH)")
endif()
if(NOT HDF5_FOUND)
    find_package(HDF5 REQUIRED COMPONENTS C CXX)
endif()
# CONFIG mode (e.g. vcpkg) provides imported targets but may not set the legacy
# HDF5_INCLUDE_DIRS / HDF5_LIBRARIES / HDF5_HL_LIBRARIES variables that the
# target_include_directories / target_link_libraries calls below expect.
# Populate them from the imported target properties when absent.
if(NOT HDF5_INCLUDE_DIRS)
    foreach(_hdf5_tgt
            hdf5::hdf5-shared hdf5::hdf5_cpp-shared
            hdf5::hdf5-static hdf5::hdf5_cpp-static)
        if(TARGET ${_hdf5_tgt})
            get_target_property(_hdf5_inc ${_hdf5_tgt} INTERFACE_INCLUDE_DIRECTORIES)
            if(_hdf5_inc)
                set(HDF5_INCLUDE_DIRS ${_hdf5_inc})
                break()
            endif()
        endif()
    endforeach()
endif()
if(NOT HDF5_LIBRARIES)
    foreach(_hdf5_tgt hdf5::hdf5-shared hdf5::hdf5-static)
        if(TARGET ${_hdf5_tgt})
            list(APPEND HDF5_LIBRARIES ${_hdf5_tgt})
            break()
        endif()
    endforeach()
    foreach(_hdf5_tgt hdf5::hdf5_cpp-shared hdf5::hdf5_cpp-static)
        if(TARGET ${_hdf5_tgt})
            list(APPEND HDF5_LIBRARIES ${_hdf5_tgt})
            break()
        endif()
    endforeach()
endif()
if(NOT HDF5_HL_LIBRARIES)
    foreach(_hdf5_tgt hdf5::hdf5_hl-shared hdf5::hdf5_hl-static)
        if(TARGET ${_hdf5_tgt})
            list(APPEND HDF5_HL_LIBRARIES ${_hdf5_tgt})
            break()
        endif()
    endforeach()
endif()

# Boost: header-only libraries needed for multi_array, iterator, etc.
# Try CONFIG mode first (vcpkg exports Boost via a config package); fall back
# to MODULE mode (system installs on Linux/macOS).
if(VCPKG_TOOLCHAIN)
    find_package(Boost CONFIG QUIET)
endif()
if(NOT Boost_FOUND)
    find_package(Boost REQUIRED)
endif()
if(NOT Boost_FOUND)
    message(FATAL_ERROR "Boost not found. Please ensure libboost-dev (Linux), boost (macOS), or boost-headers (vcpkg) is installed.")
endif()
# When vcpkg config-mode Boost is found, Boost_INCLUDE_DIRS may be empty.
# Try to populate it from the imported target's INTERFACE_INCLUDE_DIRECTORIES.
# Skip generator expressions (e.g. $<BUILD_INTERFACE:...>) — they are not
# usable as plain filesystem paths at configure time.
set(VCELL_BOOST_TARGET)
foreach(_boost_tgt Boost::headers Boost::boost)
    if(TARGET ${_boost_tgt})
        set(VCELL_BOOST_TARGET ${_boost_tgt})
        if(NOT Boost_INCLUDE_DIRS)
            get_target_property(_boost_inc ${_boost_tgt} INTERFACE_INCLUDE_DIRECTORIES)
            if(_boost_inc AND NOT _boost_inc MATCHES "-NOTFOUND$"
                          AND NOT _boost_inc MATCHES "\\$<")
                set(Boost_INCLUDE_DIRS ${_boost_inc})
            endif()
        endif()
        break()
    endif()
endforeach()
# Normalize Boost include dirs: strip generator expressions and keep only entries
# that actually contain boost/version.hpp (including versioned include roots).
set(_boost_valid_includes)
foreach(_boost_inc_dir IN LISTS Boost_INCLUDE_DIRS)
    if(NOT _boost_inc_dir OR _boost_inc_dir MATCHES "\\$<")
        continue()
    endif()
    if(EXISTS "${_boost_inc_dir}/boost/version.hpp")
        list(APPEND _boost_valid_includes "${_boost_inc_dir}")
        continue()
    endif()
    file(GLOB _boost_versioned_dirs "${_boost_inc_dir}/boost-*")
    foreach(_boost_versioned_dir IN LISTS _boost_versioned_dirs)
        if(EXISTS "${_boost_versioned_dir}/boost/version.hpp")
            list(APPEND _boost_valid_includes "${_boost_versioned_dir}")
        endif()
    endforeach()
endforeach()
if(_boost_valid_includes)
    list(REMOVE_DUPLICATES _boost_valid_includes)
    set(Boost_INCLUDE_DIRS "${_boost_valid_includes}")
endif()

set(_boost_headers_resolved FALSE)
foreach(_boost_inc_dir IN LISTS Boost_INCLUDE_DIRS)
    if(EXISTS "${_boost_inc_dir}/boost/version.hpp")
        set(_boost_headers_resolved TRUE)
        break()
    endif()
endforeach()

# Last-resort fallback: if Boost include dirs are still unresolved (common with
# vcpkg config mode), search BOOST_ROOT, BOOST_INCLUDEDIR, or CMAKE_PREFIX_PATH
# entries directly for boost/version.hpp.
if(NOT _boost_headers_resolved)
    set(_boost_found_inc "")
    set(_boost_include_hints)
    if(DEFINED BOOST_INCLUDEDIR)
        list(APPEND _boost_include_hints "${BOOST_INCLUDEDIR}")
    endif()
    if(DEFINED BOOST_ROOT)
        list(APPEND _boost_include_hints "${BOOST_ROOT}/include")
    endif()
    foreach(_pfx IN LISTS CMAKE_PREFIX_PATH)
        list(APPEND _boost_include_hints "${_pfx}/include")
    endforeach()

    foreach(_boost_hint IN LISTS _boost_include_hints)
        if(EXISTS "${_boost_hint}/boost/version.hpp")
            set(_boost_found_inc "${_boost_hint}")
            break()
        endif()
        file(GLOB _boost_versioned_dirs "${_boost_hint}/boost-*")
        foreach(_boost_versioned_dir IN LISTS _boost_versioned_dirs)
            if(EXISTS "${_boost_versioned_dir}/boost/version.hpp")
                set(_boost_found_inc "${_boost_versioned_dir}")
                break()
            endif()
        endforeach()
        if(_boost_found_inc)
            break()
        endif()
    endforeach()
    if(_boost_found_inc)
        set(Boost_INCLUDE_DIRS "${_boost_found_inc}")
        set(_boost_headers_resolved TRUE)
    endif()
endif()
if(NOT _boost_headers_resolved)
    # Use CMake's find_path to search broadly for boost/version.hpp.  This
    # handles cases where the Boost imported target only carries generator
    # expressions and none of the earlier hint-based lookups succeeded.
    find_path(_boost_find_path_result boost/version.hpp
        HINTS
            ${BOOST_INCLUDEDIR}
            ${BOOST_ROOT}/include
        PATH_SUFFIXES
            boost-1_87 boost-1_86 boost-1_85 boost-1_84 boost-1_83 boost-1_82
    )
    if(_boost_find_path_result)
        set(Boost_INCLUDE_DIRS "${_boost_find_path_result}")
        set(_boost_headers_resolved TRUE)
        message(STATUS "Boost include directory found via find_path: ${_boost_find_path_result}")
    endif()
endif()
if(NOT _boost_headers_resolved)
    if(VCELL_BOOST_TARGET)
        message(STATUS "Boost include directory not resolved at configure time; using ${VCELL_BOOST_TARGET} imported target includes.")
    else()
        message(FATAL_ERROR "Boost include directory not found. "
            "Set BOOST_ROOT or BOOST_INCLUDEDIR, or add the Boost prefix to CMAKE_PREFIX_PATH.")
    endif()
endif()

# Python bindings (on by default; MB_BUILD_PYTHON=OFF skips them).
# Use Development.Module (not the full Development component): manylinux images
# ship no shared libpython, so requesting Python3_LIBRARIES would fail there.
# Development.Module provides the headers/SOABI (and the python3_add_library
# command that pybind11's FindPython "new tools" mode needs) without linking
# libpython — the correct policy for wheels on all platforms.
if(MB_BUILD_PYTHON)
    find_package(Python3 REQUIRED COMPONENTS Interpreter Development.Module)
    find_package(pybind11 CONFIG REQUIRED)
endif()

# --- Bundled / external library dependencies ---

# FronTier (bundled in FronTierLib/)
add_subdirectory(FronTierLib)

# ExpressionParser (bundled in ExpressionParser/)
add_subdirectory(vcell-expressionparser)

# vcommons (bundled in vcommons/)
add_subdirectory(vcommons)

# vcell-messaging (bundled in VCell-Messaging) /)
# GIT_DESCRIBE feeds g_GIT_DESCRIBE (vcell-messaging's GitDescribe.cpp.in): the
# version the solver prints at startup and stores as the output's
# runTime/gitVersion attribute. An empty one makes that HDF5 attribute
# zero-length, which HDF5 rejects at the end of every run. Fall back to the
# project version when git (or the .git directory) is unavailable.
if(NOT GIT_DESCRIBE)
    find_package(Git QUIET)
    if(GIT_FOUND)
        execute_process(COMMAND ${GIT_EXECUTABLE} describe --tags --always --dirty
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            OUTPUT_VARIABLE GIT_DESCRIBE OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET RESULT_VARIABLE _git_describe_rc)
        if(NOT _git_describe_rc EQUAL 0)
            set(GIT_DESCRIBE "")
        endif()
    endif()
    if(NOT GIT_DESCRIBE)
        set(GIT_DESCRIBE "v${PROJECT_VERSION}")
    endif()
endif()
message(STATUS "GIT_DESCRIBE:      ${GIT_DESCRIBE}")
add_subdirectory(vcell-messaging)

# SQLite: prefer system, fall back to bundled copy in sqlite/
find_file(SYSTEM_SQLITE3_H sqlite3.h)
if(SYSTEM_SQLITE3_H)
    get_filename_component(SQLITE3_INCLUDE_DIR "${SYSTEM_SQLITE3_H}" DIRECTORY)
    message(STATUS "sqlite3:           ${SYSTEM_SQLITE3_H} (system)")
else()
    set(SQLITE3_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/sqlite)
    message(STATUS "sqlite3:           bundled (${SQLITE3_INCLUDE_DIR})")
endif()

message(STATUS "Platform:          ${CMAKE_SYSTEM_NAME}")
message(STATUS "Build type:        ${CMAKE_BUILD_TYPE}")
message(STATUS "HDF5 version:      ${HDF5_VERSION}, libs: ${HDF5_LIBRARIES}")
message(STATUS "Boost version:     ${Boost_VERSION}, include: ${Boost_INCLUDE_DIRS}")

# --- Source files ---
file(GLOB MB_SOLVER_MAIN  Solver/src/driver.cpp)
file(GLOB MB_ALLOC_MAIN   Solver/src/alloctestmain.cpp)
file(GLOB MB_SRC_FILES    Solver/src/*.cpp)
list(REMOVE_ITEM MB_SRC_FILES ${MB_SOLVER_MAIN} ${MB_ALLOC_MAIN})

# --- Library target ---
add_library(MovingBoundaryLib ${MB_SRC_FILES})

target_include_directories(MovingBoundaryLib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Solver/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Solver/include/MBridge>
        $<BUILD_INTERFACE:${SQLITE3_INCLUDE_DIR}>
        ${HDF5_INCLUDE_DIRS}
        ${Boost_INCLUDE_DIRS}
        $<INSTALL_INTERFACE:include/vcell-mbsolver>
)

target_link_libraries(MovingBoundaryLib
    PUBLIC
        ${VCELL_BOOST_TARGET}
        ${HDF5_LIBRARIES}
        ${HDF5_HL_LIBRARIES}
        frontier
        vcellexpressionparser
        vcommons
        vcellmessaging
        Threads::Threads
)

# --- Executable target ---
add_executable(MovingBoundarySolver ${MB_SOLVER_MAIN})
target_link_libraries(MovingBoundarySolver PRIVATE MovingBoundaryLib)

# --- Testing ---
include(CTest)
if(BUILD_TESTING)
    add_subdirectory(Tests)
    add_subdirectory(ExpressionParserTest)
endif()

# --- Python bindings ---
if(MB_BUILD_PYTHON)
    add_subdirectory(python)
endif()

# --- Install ---
install(TARGETS MovingBoundaryLib MovingBoundarySolver
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY Solver/include/
    DESTINATION include/vcell-mbsolver
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)
