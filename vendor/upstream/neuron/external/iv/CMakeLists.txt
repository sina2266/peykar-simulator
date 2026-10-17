cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(INTERVIEWS C CXX)

# =============================================================================
# CMake common project settings
# =============================================================================
set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(PROJECT_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR})


# =============================================================================
# To be able to use project as submodule, avoid using PROJECT_SOURCE_DIR
# =============================================================================
set(IV_PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(IV_PROJECT_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
# For future target customization
set(IV_AS_SUBPROJECT OFF)
if (NOT CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(IV_AS_SUBPROJECT ON)
endif()

# =============================================================================
# Project build options
# =============================================================================
option(IV_ENABLE_SHARED "Build libraries shared or static" OFF)
# Useful for MAC if XQuartz not installed on user machine.
option(IV_ENABLE_X11_DYNAMIC "dlopen X11 after launch" OFF)
# Use only if X11 API changes (Uses very brittle script).
option(IV_ENABLE_X11_DYNAMIC_MAKE_HEADERS "Remake the X11 dynamic .h files." OFF)

# =============================================================================
# CMake build settings
# =============================================================================
set(allowableBuildTypes Custom Debug Release RelWithDebInfo Fast FastDebug)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE
      RelWithDebInfo
      CACHE STRING "Empty or one of ${allowableBuildTypes}" FORCE)
elseif(NOT CMAKE_BUILD_TYPE IN_LIST allowableBuildTypes)
  message(FATAL_ERROR "Invalid build type: ${CMAKE_BUILD_TYPE} : Must be one of ${allowableBuildTypes}")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

if(NOT IV_ENABLE_SHARED)
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
  set(IV_LIB_TYPE "STATIC")
else()
  set(IV_LIB_TYPE "SHARED")
endif()

# CYGWIN macros is used in the code
if(${CMAKE_SYSTEM_NAME} MATCHES "CYGWIN")
  set(CYGWIN 1)
endif()

# set up installation dirs
if(NOT DEFINED IV_LIB_INSTALL_DIR)
    set(IV_LIB_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib)
endif()

if(NOT DEFINED IV_HEADERS_INSTALL_DIR)
    set(IV_HEADERS_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/include)
endif()

if(NOT DEFINED IV_BIN_INSTALL_DIR)
    set(IV_BIN_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/bin)
endif()

# =============================================================================
# Include cmake modules
# =============================================================================
list(APPEND CMAKE_MODULE_PATH ${IV_PROJECT_SOURCE_DIR}/cmake)
include(HelperFunctions)
# do not conflict with nrn PlatformHelper.cmake when submodule.
include(${IV_PROJECT_SOURCE_DIR}/cmake/PlatformHelper.cmake)
include(RpathHelper)
include(CheckIncludeFiles)
include(CheckFunctionExists)

# =============================================================================
# Find dependencies
# =============================================================================
find_package(X11)

if(NOT X11_FOUND AND NOT IV_WINDOWS_BUILD)
  if(APPLE)
    message(FATAL_ERROR "Install XQuartz from https://www.xquartz.org/ to build iv")
  else()
    message(FATAL_ERROR "Install X11 to build iv (e.g. 'apt install libx11-dev libxcomposite-dev' on Ubuntu")
  endif()
endif()

if(NOT X11_Xcomposite_FOUND AND NOT IV_WINDOWS_BUILD)
  message(FATAL_ERROR "libXcomposite is required for X11 support (e.g. 'apt install libxcomposite-dev' on Ubuntu)")
endif()

if(IV_ENABLE_X11_DYNAMIC AND NOT X11_FOUND)
    message(FATAL_ERROR "Cannot IV_ENABLE_X11_DYNAMIC without X11 installed")
endif()

if (IV_ENABLE_X11_DYNAMIC_MAKE_HEADERS)
  # need any Python to run mkdynam.py to generate some .h files
  find_package(PythonInterp REQUIRED)
  if (NOT X11_FOUND)
    message(FATAL_ERROR "Cannot IV_ENABLE_X11_DYNAMIC_MAKE_HEADERS without X11 installed")
  endif()
endif()

# =============================================================================
# Check existance of various headers, functions and directories
# =============================================================================
set(CMAKE_REQUIRED_QUIET TRUE)
message(STATUS "Checking for include files")
check_include_files(osfcn.h HAVE_OSFCN_H)
check_include_files(stdlib.h HAVE_STDLIB_H)
check_include_files(string.h HAVE_STRING_H)
check_include_files(strings.h HAVE_STRINGS_H)
check_include_files(stropts.h HAVE_STROPTS_H)
check_include_files(sys/conf.h HAVE_SYS_CONF_H)
check_include_files(sys/mman.h HAVE_SYS_MMAN_H)
check_include_files(sys/stat.h HAVE_SYS_STAT_H)
check_include_files(unistd.h HAVE_UNISTD_H)

message(STATUS "Checking for functions")
check_function_exists(sigprocmask HAVE_POSIX_SIGNALS)

message(STATUS "Checking for include directories")
iv_check_dir_exists(dirent.h HAVE_DIRENT_H)
iv_check_dir_exists(ndir.h HAVE_NDIR_H)
iv_check_dir_exists(sys/dir.h HAVE_SYS_DIR_H)
iv_check_dir_exists(sys/ndir.h HAVE_SYS_NDIR_H)

message(STATUS "Checking for types")
iv_check_type_exists(sys/types.h gid_t int gid_t)
iv_check_type_exists(sys/types.h off_t "long int" off_t)
iv_check_type_exists(sys/types.h pid_t int pid_t)
iv_check_type_exists(sys/types.h size_t "unsigned int" size_t)
iv_check_type_exists(sys/types.h uid_t int uid_t)

# =============================================================================
# Check if signals support
# =============================================================================
# NOTE: check_function_exists(sigsetmask HAVE_BSD_SIGNALS) made obsolete by sigprocmask(2).
# sigsetmask exists but the usage generates a variety of errors. See the old autoconf acinclude.m4
# AC_DEFUN([BASH_SIGNAL_CHECK] For now, use only if don't have posix signals.
if(NOT ${HAVE_POSIX_SIGNALS})
  set(HAVE_BSD_SIGNALS 1)
else()
  set(HAVE_BSD_SIGNALS 0)
endif()

# =============================================================================
# Generate config.h after all checks
# =============================================================================
add_definitions(-DHAVE_CONFIG_H)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/iv-config.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/cmake/iv-config.cmake @ONLY)

# =============================================================================
# Include source directories
# =============================================================================
include_directories(${IV_PROJECT_SOURCE_DIR}/src/include ${IV_PROJECT_BINARY_DIR}/src/lib)
add_subdirectory(src/lib)
add_subdirectory(src/bin)

# =============================================================================
# Install binaries and headers
# =============================================================================
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/include/ DESTINATION "${IV_HEADERS_INSTALL_DIR}")
if(NOT IV_AS_SUBPROJECT)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cmake/iv-config.cmake DESTINATION ${IV_LIB_INSTALL_DIR}/cmake/iv)
    install(EXPORT iv DESTINATION ${IV_LIB_INSTALL_DIR}/cmake/iv)
endif()

# =============================================================================
# Print build status
# =============================================================================
message(STATUS "")
message(STATUS "Configured INTERVIEWS ${PROJECT_VERSION}")
message(STATUS "")
string(TOLOWER "${CMAKE_GENERATOR}" cmake_generator_tolower)
if(cmake_generator_tolower MATCHES "makefile")
  message(STATUS "Some things you can do now:")
  message(STATUS "--------------+--------------------------------------------------------------")
  message(STATUS "Command       |   Description")
  message(STATUS "--------------+--------------------------------------------------------------")
  message(STATUS "make install  | Will install INTERVIEWS to: ${CMAKE_INSTALL_PREFIX}")
  message(STATUS "              | Change the install location of NEURON using:")
  message(STATUS "              |     cmake <src_path> -DCMAKE_INSTALL_PREFIX=<install_path>")
  message(STATUS "--------------+--------------------------------------------------------------")
  message(STATUS "Build option  | Status")
  message(STATUS "--------------+--------------------------------------------------------------")
  message(STATUS "BUILD_TYPE    | ${CMAKE_BUILD_TYPE} (allowed: ${allowableBuildTypes})")
  message(STATUS "SHARED        | ${IV_ENABLE_SHARED}")
  if (X11_FOUND)
    message(STATUS "X11_DYNAMIC   | ${IV_ENABLE_X11_DYNAMIC}")
    if (IV_ENABLE_X11_DYNAMIC_MAKE_HEADERS)
    message(STATUS "              |   ...MAKE_HEADERS=${IV_ENABLE_X11_DYNAMIC_MAKE_HEADERS}")
    endif()
  endif()
  message(STATUS "--------------+--------------------------------------------------------------")
  message(STATUS " See more : https://github.com/neuronsimulator/iv")
  message(STATUS "--------------+--------------------------------------------------------------")
endif()
message(STATUS "")
