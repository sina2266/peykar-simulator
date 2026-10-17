# Function: TestUndefinedSymbolsAllowed
#
# Test if the linker allows undefined symbols for shared libraries.
#
# UNDEFINED_SYMBOLS_ALLOWED - true if the linker will allow
#   undefined symbols for shared libraries.
#

function(Test_Undefined_Symbols_Allowed)
  set(_VAR_NAME "UNDEFINED_SYMBOLS_ALLOWED")
  set(_HASH_VAR_NAME "HASH_${_VAR_NAME}")

  # Hash the CMAKE_FLAGS passed and check cache to know if we need to rerun.
  string(MD5 _CMAKE_FLAGS_HASH "${CMAKE_SHARED_LINKER_FLAGS}")

  if(NOT DEFINED "${_HASH_VAR_NAME}")
    unset("${_VAR_NAME}" CACHE)
  elseif(NOT "${${_HASH_VAR_NAME}}" STREQUAL "${_CMAKE_FLAGS_HASH}")
    unset("${_VAR_NAME}" CACHE)
  endif()

  if(NOT DEFINED "${_VAR_NAME}")
    message(STATUS "Performing Test ${_VAR_NAME} - ...")
    set(_TEST_PROJECT_DIR "${PROJECT_BINARY_DIR}/CMakeTmp/${_VAR_NAME}")

    file(WRITE "${_TEST_PROJECT_DIR}/CMakeLists.txt"
"
cmake_minimum_required(VERSION 3.18.0)
project(undefined CXX)
add_library(foo SHARED \"foo.cpp\")
")

    file(WRITE "${_TEST_PROJECT_DIR}/foo.cpp"
"
extern int bar(void);
int foo(void) {return bar()+1;}
")

    if(APPLE)
      set(_rpath_arg "-DCMAKE_MACOSX_RPATH='${CMAKE_MACOSX_RPATH}'")
    else()
      set(_rpath_arg)
    endif()

    try_compile(${_VAR_NAME}
      "${_TEST_PROJECT_DIR}"
      "${_TEST_PROJECT_DIR}"
      undefined
      CMAKE_FLAGS
        "-DCMAKE_SHARED_LINKER_FLAGS='${CMAKE_SHARED_LINKER_FLAGS}'"
        ${_rpath_arg}
      OUTPUT_VARIABLE _OUTPUT)

    set(${_HASH_VAR_NAME} "${_CMAKE_FLAGS_HASH}" CACHE INTERNAL  "hashed try_compile flags")

    if(${_VAR_NAME})
      message(STATUS "Performing Test ${_VAR_NAME} - Success")
    else()
      message(STATUS "Performing Test ${_VAR_NAME} - Failed")
      file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
        "Performing Test ${_VAR_NAME} failed with the following output:\n"
        "${_OUTPUT}\n")
    endif()
  endif()
endfunction()
