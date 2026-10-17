# Copyright libCellML Contributors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

function(replace_compiler_flag _OLD _NEW)
  set(_OLD "(^| )${_OLD}($| )")

  if(NOT "${_NEW}" STREQUAL "")
    set(_NEW " ${_NEW} ")
  endif()

  foreach(_VAR CMAKE_CXX_FLAGS
               CMAKE_CXX_FLAGS_DEBUG
               CMAKE_CXX_FLAGS_RELEASE
               CMAKE_CXX_FLAGS_MINSIZEREL
               CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    if("${${_VAR}}" MATCHES "${_OLD}")
      string(REGEX REPLACE "${_OLD}" "${_NEW}" ${_VAR} "${${_VAR}}")
    endif()

    set(${_VAR} "${${_VAR}}" PARENT_SCOPE)
  endforeach()
endfunction()

function(target_warnings_as_errors _TARGET)
  set(_COMPILER_WAE)

  set(_GNU_FLAGS_COMPILER_COMPAT "GNU" "AppleClang" "Clang")
  list(FIND _GNU_FLAGS_COMPILER_COMPAT "${CMAKE_CXX_COMPILER_ID}" _INDEX)
  if(${_INDEX} GREATER -1)
    set(_COMPILER_WAE -Wall -W -Werror)
  elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(_COMPILER_WAE /W4 /WX)
  endif()

  if(_COMPILER_WAE)
    target_compile_options(${_TARGET} PRIVATE ${_COMPILER_WAE})
  endif()
  unset(_COMPILER_WAE)
endfunction()

function(internalise_cmake_variables)
  # internalise some CMake variables
  file(TO_CMAKE_PATH ${LIBCELLML_INSTALL_PREFIX} _CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX ${_CMAKE_INSTALL_PREFIX} CACHE INTERNAL "Internalise CMAKE_INSTALL_PREFIX, manipulate via LIBCELLML_INSTALL_PREFIX" FORCE)
  set(CMAKE_BUILD_TYPE ${LIBCELLML_BUILD_TYPE} CACHE INTERNAL "Internalise CMAKE_BUILD_TYPE, manipulate via LIBCELLML_BUILD_TYPE" FORCE)
  set(BUILD_SHARED_LIBS ${LIBCELLML_BUILD_SHARED} CACHE INTERNAL "Internalise BUILD_SHARED_LIBS, manipulate via LIBCELLML_BUILD_SHARED" FORCE)
endfunction()

function(hide_distracting_variables)
  # Mark cache variables that aren't libCellML configuration variables as advanced
  # to hide them from the user in a CMake GUI.
  mark_as_advanced(CMAKE_CONFIGURATION_TYPES)
  mark_as_advanced(CMAKE_CODEBLOCKS_EXECUTABLE)
  mark_as_advanced(QT_QMAKE_EXECUTABLE)
  mark_as_advanced(ZLIB_DIR)
  mark_as_advanced(LibXml2_DIR)
  mark_as_advanced(LIBXML2_INCLUDE_DIR)
  mark_as_advanced(LIBXML2_LIBRARY)
  mark_as_advanced(LIBXML2_XMLLINT_EXECUTABLE)
  if(APPLE)
    mark_as_advanced(CMAKE_OSX_ARCHITECTURES)
    mark_as_advanced(CMAKE_OSX_DEPLOYMENT_TARGET)
    mark_as_advanced(CMAKE_OSX_SYSROOT)
  endif()
  if(NOT WIN32)
    mark_as_advanced(pkgcfg_lib_PC_LIBXML_xml2)
  endif()
endfunction()

function(group_source_to_dir_structure)
  if(MSVC)
    foreach(_FILE ${ARGN})
      # Basic test for determining if current file is a source file or header file.
      if(${_FILE} MATCHES ".+\.cpp$")
        set(_FILE_PREFIX "Source Files\\")
      else()
        set(_FILE_PREFIX "Header Files\\")
      endif()
      # Get relative path from current source directory or current binary directory.
      if(${_FILE} MATCHES "^${CMAKE_CURRENT_SOURCE_DIR}.+")
        file(RELATIVE_PATH _FILE_RELATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}" ${_FILE})
      else()
        file(RELATIVE_PATH _FILE_RELATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}" ${_FILE})
        # Assume all files in the current binary directory are generated header files!
        set(_FILE_PREFIX "Generated Header Files\\")
      endif()
      get_filename_component(_FILE_GROUP ${_FILE_RELATIVE_PATH} DIRECTORY)
      if(_FILE_GROUP)
        string(REPLACE "/" "\\" _FILE_GROUP ${_FILE_GROUP})
      endif()
      source_group("${_FILE_PREFIX}${_FILE_GROUP}" FILES "${_FILE}")
    endforeach()
  endif()
endfunction()

function(configure_clang_and_clang_tidy_settings _TARGET)
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"
     OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    # The full list of diagnostic flags in Clang can be found at
    # https://clang.llvm.org/docs/DiagnosticsReference.html.
    set(_COMPILE_OPTIONS
      -Weverything
      -Wno-c++98-compat
      -Wno-c++98-compat-pedantic
      -Wno-documentation
      -Wno-documentation-unknown-command
      -Wno-exit-time-destructors
      -Wno-global-constructors
      -Wno-missing-prototypes
      -Wno-padded
      -Wno-undefined-func-template
      -Wno-weak-vtables # Applying this because NamedEntity.h doesn't define any virtual methods other than the destructor.
    )

    if (${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER_EQUAL 7.0.0)
      list(APPEND _COMPILE_OPTIONS
        -Wno-reserved-id-macro
      )
    endif()

    if(NOT "${_TARGET}" STREQUAL "cellml")
      list(APPEND _COMPILE_OPTIONS
        -Wno-old-style-cast
        -Wno-used-but-marked-unused
        --system-header-prefix=gtest/
      )
    endif()

    set_target_properties(${_TARGET} PROPERTIES
      COMPILE_OPTIONS "${_COMPILE_OPTIONS}"
    )
  endif()

  if(CLANG_TIDY_AVAILABLE)
    if(NOT "${_TARGET}" STREQUAL "cellml")
        set(_DISABLED_BUGPRONE_CHECKS
          -bugprone-exception-escape
          -bugprone-suspicious-include
        )
        set(_DISABLED_CPPCOREGUIDELINES_CHECKS
          -cppcoreguidelines-avoid-non-const-global-variables
          -cppcoreguidelines-pro-bounds-pointer-arithmetic
          -cppcoreguidelines-pro-type-vararg
        )
        set(_DISABLED_HICPP_CHECKS
          -hicpp-vararg
        )
        set(_DISABLED_READABILITY_CHECKS
          -readability-named-parameter
        )
    endif()

    # The full list of Clang-Tidy checks can be found at
    # https://clang.llvm.org/extra/clang-tidy/checks/list.html.
    set(_CLANG_TIDY_CHECKS
      -*
      bugprone-*
      -bugprone-branch-clone
      ${_DISABLED_BUGPRONE_CHECKS}
      cert-*
      -cert-err58-cpp
      cppcoreguidelines-*
      -cppcoreguidelines-avoid-magic-numbers
      -cppcoreguidelines-init-variables
      -cppcoreguidelines-owning-memory
      -cppcoreguidelines-pro-type-reinterpret-cast
      -cppcoreguidelines-slicing
      -cppcoreguidelines-special-member-functions
      ${_DISABLED_CPPCOREGUIDELINES_CHECKS}
      fuchsia-*
      -fuchsia-default-arguments
      -fuchsia-default-arguments-calls
      -fuchsia-default-arguments-declarations
      -fuchsia-multiple-inheritance
      -fuchsia-overloaded-operator
      -fuchsia-statically-constructed-objects
      google-*
      -google-readability-todo
      -google-runtime-references
      hicpp-*
      -hicpp-special-member-functions
      ${_DISABLED_HICPP_CHECKS}
      llvm-*
      -llvm-qualified-auto
      -llvm-header-guard
      misc-*
      -misc-non-private-member-variables-in-classes
      -misc-no-recursion
      modernize-*
      -modernize-make-shared
      -modernize-pass-by-value
      -modernize-raw-string-literal
      -modernize-use-nodiscard
      -modernize-use-trailing-return-type
      performance-*
      -performance-inefficient-string-concatenation
      readability-*
      -readability-convert-member-functions-to-static
      -readability-function-cognitive-complexity
      -readability-magic-numbers
      -readability-qualified-auto
      ${_DISABLED_READABILITY_CHECKS}
    )
    string(REPLACE ";" ","
           _CLANG_TIDY_CHECKS "${_CLANG_TIDY_CHECKS}")
    if(LIBCELLML_TREAT_WARNINGS_AS_ERRORS)
      set(_CLANG_TIDY_WARNINGS_AS_ERRORS ";-warnings-as-errors=${_CLANG_TIDY_CHECKS}")
    endif()

    if("${CMAKE_GENERATOR}" STREQUAL "Ninja")
      set(_HEADER_FILTER_DIR ..)
    else()
      set(_HEADER_FILTER_DIR ${CMAKE_SOURCE_DIR})
    endif()

    set(_HEADER_FILTER_DIR "${_HEADER_FILTER_DIR}/src/")

    string(REPLACE "." "\\\."
           _HEADER_FILTER_DIR "${_HEADER_FILTER_DIR}")
    string(REPLACE "/" "\\\/"
           _HEADER_FILTER_DIR "${_HEADER_FILTER_DIR}")

    if(MSVC)
      # Extra argument for Clang-Tidy when used with cl.
      # See https://gitlab.kitware.com/cmake/cmake/-/issues/20512#note_722771.
      set(_EXTRA_ARG ";--extra-arg=/EHsc")
    endif()

    set_target_properties(${_TARGET} PROPERTIES
      CXX_CLANG_TIDY "${CLANG_TIDY_EXE}${_EXTRA_ARG};-checks=${_CLANG_TIDY_CHECKS};-header-filter=${_HEADER_FILTER_DIR}.*${_CLANG_TIDY_WARNINGS_AS_ERRORS}"
    )
  endif()
endfunction()

function(get_system_name RETURN_SYSTEM_NAME)
  if(WIN32)
    set(SYSTEM_NAME "Windows" )
  elseif(APPLE)
    set(SYSTEM_NAME "macOS" )
  elseif(UNIX)
    find_program(LSB lsb_release DOC "Distribution information tool")
    mark_as_advanced(LSB)
    if(LSB)
      execute_process(COMMAND ${LSB} -i
        RESULT_VARIABLE RETFLAG
        OUTPUT_VARIABLE DISTINFO
        ERROR_VARIABLE ERRDISTINFO
        OUTPUT_STRIP_TRAILING_WHITESPACE
      )
      if(NOT RETFLAG)
        string(SUBSTRING ${DISTINFO} 16 -1 SYSTEM_NAME)
      endif()
    endif()
    if(NOT SYSTEM_NAME)
      execute_process(COMMAND cat /etc/issue RESULT_VARIABLE COMMAND_RESULT OUTPUT_VARIABLE COMMAND_OUTPUT ERROR_VARIABLE ERROR_OUTPUT)
      if(COMMAND_RESULT EQUAL 0)
        if(NOT COMMAND_OUTPUT MATCHES "^\\\\")
          string(REGEX MATCH "^[^\\]*" COMMAND_OUTPUT ${COMMAND_OUTPUT} )
          string(STRIP ${COMMAND_OUTPUT} COMMAND_OUTPUT )
          string(REPLACE " " "-" SYSTEM_NAME ${COMMAND_OUTPUT} )
        endif()
      endif()
    endif()
    if(NOT SYSTEM_NAME)
      execute_process(COMMAND cat /etc/os-release RESULT_VARIABLE COMMAND_RESULT OUTPUT_VARIABLE COMMAND_OUTPUT ERROR_VARIABLE ERROR_OUTPUT)
      if(COMMAND_RESULT EQUAL 0)
        string(REGEX MATCH "^NAME=\\\"([^\\\"]*)" NAME_OUTPUT ${COMMAND_OUTPUT})
        string(REPLACE " " "-" SYSTEM_NAME ${CMAKE_MATCH_1})
      endif()
    endif()
    if(NOT SYSTEM_NAME)
      set(SYSTEM_NAME Unix-Based)
    endif()
  else()
    message(FATAL_ERROR "Don't yet know this system.")
  endif()

  string(REPLACE "\n" "_" SYSTEM_NAME ${SYSTEM_NAME})

  set(${RETURN_SYSTEM_NAME} ${SYSTEM_NAME} PARENT_SCOPE)
endfunction()

function(debian_based _RESULT)
  set(_DEBIAN_BASED FALSE)
  string(FIND ${LIBCELLML_SYSTEM} "Ubuntu" INDEX)
  if(INDEX EQUAL 0)
    set(_DEBIAN_BASED TRUE)
  elseif(EXISTS "/etc/debian_version")
    set(_DEBIAN_BASED TRUE)
  endif()
  set(${_RESULT} ${_DEBIAN_BASED} PARENT_SCOPE)
endfunction()

function(redhat_based _RESULT)
  set(_REDHAT_BASED FALSE)
  if(EXISTS "/etc/redhat-release")
    set(_REDHAT_BASED TRUE)
  elseif(EXISTS "/etc/centos-release")
    set(_REDHAT_BASED TRUE)
  endif()
  set(${_RESULT} ${_REDHAT_BASED} PARENT_SCOPE)
endfunction()

function(get_runtime_dlls_from_target target target_property out_var)
  set(_result "")
  get_target_property(_items ${target} INTERFACE_LINK_LIBRARIES)
  get_target_property(_type ${target} TYPE)
  if(_type STREQUAL "SHARED_LIBRARY" OR _type STREQUAL "MODULE_LIBRARY")
    list(APPEND _result $<${target_property}:${target}>)
  endif()
  foreach(item IN LISTS _items)
    if(TARGET ${item})
      get_target_property(_type ${item} TYPE)
      if(_type STREQUAL "SHARED_LIBRARY" OR _type STREQUAL "MODULE_LIBRARY")
        list(APPEND _result $<${target_property}:${item}>)
      endif()
    endif()
  endforeach()
  set(${out_var} "${_result}" PARENT_SCOPE)
endfunction()
