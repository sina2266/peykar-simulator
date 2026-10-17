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

include(GenerateExportHeader)

# include gtest so we can make use of the gtest_main target
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/gtest)
if(APPLE)
  set_target_properties(gtest gtest_main PROPERTIES MACOSX_RPATH ON)
endif()
set_target_properties(gtest gtest_main PROPERTIES FOLDER gtest)
mark_as_advanced(gtest_disable_pthreads)
mark_as_advanced(gtest_force_shared_crt)

# Any tests included here must append the test name
# to the LIBCELLML_TESTS list.  Any source files for the
# test must be set to <test_name>_SRCS, likewise for
# header files <test_name>_HDRS.
include(analyser/tests.cmake)
include(annotator/tests.cmake)
include(clone/tests.cmake)
include(component/tests.cmake)
include(connection/tests.cmake)
include(coverage/tests.cmake)
include(equality/tests.cmake)
include(generator/tests.cmake)
include(import_source/tests.cmake)
include(importer/tests.cmake)
include(isolated/tests.cmake)
include(math/tests.cmake)
include(model/tests.cmake)
include(parser/tests.cmake)
include(printer/tests.cmake)
include(reset/tests.cmake)
include(units/tests.cmake)
include(validator/tests.cmake)
include(variable/tests.cmake)
include(version/tests.cmake)

set(TEST_EXPORTDEFINITIONS_H "${CMAKE_CURRENT_BINARY_DIR}/test_exportdefinitions.h")

set(TEST_UTILS_SRC ${CMAKE_CURRENT_SOURCE_DIR}/test_utils.cpp)
set(TEST_UTILS_HDR ${CMAKE_CURRENT_SOURCE_DIR}/test_utils.h)

if(NOT DEFINED LIBXML2_VERSION_STRING)
  set(LIBXML2_VERSION_STRING "2.9.10")
elseif(LIBXML2_VERSION_STRING VERSION_GREATER_EQUAL "2.9.13")
  set(LIBXML2_VERSION_STRING "2.9.13")
elseif(LIBXML2_VERSION_STRING VERSION_GREATER_EQUAL "2.9.11")
  set(LIBXML2_VERSION_STRING "2.9.11")
endif()

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/libxml2issues.${LIBXML2_VERSION_STRING}.h
  ${CMAKE_CURRENT_BINARY_DIR}/libxml2issues.h
  COPYONLY
)

add_library(test_utils SHARED ${TEST_UTILS_SRC} ${TEST_UTILS_HDR} ${TEST_EXPORTDEFINITIONS_H})
target_link_libraries(test_utils PUBLIC cellml gtest)
target_include_directories(test_utils PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(test_utils PROPERTIES
  CXX_VISIBILITY_PRESET hidden
  VISIBILITY_INLINES_HIDDEN 1
  FOLDER tests)
generate_export_header(test_utils EXPORT_FILE_NAME ${TEST_EXPORTDEFINITIONS_H} BASE_NAME TEST)

set(TESTS_SOURCE_FILES ${TEST_UTILS_SRC})
set(TESTS_HEADER_FILES ${TEST_UTILS_HDR})

include(test_resources.cmake)

if(MSVC)
  get_runtime_dlls_from_target(${LIBXML2_TARGET} TARGET_FILE_DIR LIBXML2_FILE_DIRS)
  get_runtime_dlls_from_target(${ZLIB_TARGET} TARGET_FILE_DIR ZLIB_FILE_DIRS)
  set(ALL_FILE_DIRS ${LIBXML2_FILE_DIRS} ${ZLIB_FILE_DIRS})
  list(REMOVE_DUPLICATES ALL_FILE_DIRS)
  list(JOIN ALL_FILE_DIRS "\\;" ALL_FILE_DIRS_STRING)
  set(ALL_FILE_DIRS_STRING "\;${ALL_FILE_DIRS_STRING}")
endif()

# Cycle through all the tests 'included' above
set(TEST_LIST)
foreach(TEST ${LIBCELLML_TESTS})
  set(CURRENT_TEST test_${TEST})
  set(CURRENT_CATEGORY ${${TEST}_CATEGORY})
  if(CURRENT_CATEGORY)
    set(CURRENT_CATEGORY "${CURRENT_CATEGORY}_")
  endif()
  list(APPEND TESTS_SOURCE_FILES ${${TEST}_SRCS})
  list(APPEND TESTS_HEADER_FILES ${${TEST}_HDRS})
  add_executable(${CURRENT_TEST} ${${TEST}_SRCS} ${${TEST}_HDRS})
  target_include_directories(${CURRENT_TEST} PUBLIC ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
  target_link_libraries(${CURRENT_TEST} cellml gtest_main test_utils)

  set_target_properties(${CURRENT_TEST} PROPERTIES FOLDER tests)

  if(LIBCELLML_CLANG_TIDY)
    configure_clang_and_clang_tidy_settings(${CURRENT_TEST})
  endif()

  if(LIBCELLML_TREAT_WARNINGS_AS_ERRORS)
    target_warnings_as_errors(${CURRENT_TEST})
  endif()

  add_test(NAME ${CURRENT_CATEGORY}unit_${CURRENT_TEST} COMMAND ${CURRENT_TEST})
  if(MSVC)
    set(_TEST_ENV_PROPERTIES "PATH=$<TARGET_FILE_DIR:cellml>\;$<TARGET_FILE_DIR:gtest_main>${ALL_FILE_DIRS_STRING}")
    set_tests_properties(${CURRENT_CATEGORY}unit_${CURRENT_TEST} PROPERTIES ENVIRONMENT "${_TEST_ENV_PROPERTIES}" TIMEOUT 120)
  endif()

  list(APPEND TEST_LIST ${CURRENT_TEST})
  unset(CURRENT_CATEGORY)
endforeach()

set(ALL_LIBRARY_SOURCE_FILES ${SOURCE_FILES} ${API_HEADER_FILES} ${MODULE_HEADER_FILES} ${GENERATED_HEADER_FILES} ${HEADER_FILES})
add_test(NAME are_debug_utilities_used COMMAND ${CMAKE_COMMAND} -P ${CMAKE_SOURCE_DIR}/cmake/checkfordebugheaderusage.cmake ${ALL_LIBRARY_SOURCE_FILES})

# Generate tests for header files.
include(api_headers/tests.cmake)

add_subdirectory(bindings)

if(LIBCELLML_COVERAGE)
  # Clean away old gcov arc files
  add_custom_target(clean_coverage
    COMMAND ${FIND_EXE} ${GCOV_ANALYSIS_PATH} -type f -name '*.gcda' -print | xargs /bin/rm -f
    COMMENT "Clean old coverage data")

  # Clean away any coverage files when the libCellML is built.
  add_dependencies(cellml clean_coverage)

  # Prepare for test coverage by generating the gcov note files
  add_custom_target(prepare_coverage
    DEPENDS clean_coverage ${TEST_LIST}
    COMMAND ${CMAKE_CTEST_COMMAND} > /dev/null
    COMMENT "Prepare for coverage tests")

  add_custom_target(coverage
    COMMAND ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/gcovr-4.1.1.zip -r ${GCOV_PREFIX_PATH} --gcov-executable=${GCOV_EXE} ${GCOV_ANALYSIS_PATH}
    DEPENDS prepare_coverage
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running coverage tests")

  add_custom_target(coverage_branches
    COMMAND ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/gcovr-4.1.1.zip -k -b --exclude-unreachable-branches -r ${GCOV_PREFIX_PATH} --gcov-executable=${GCOV_EXE} ${GCOV_ANALYSIS_PATH}
    DEPENDS prepare_coverage
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running coverage tests with branches output")
endif()

if(LIBCELLML_LLVM_COVERAGE)
  set(RAW_PROFILE_FILENAME "${CMAKE_CURRENT_BINARY_DIR}/libcellml-%m.profraw")
  set(RAW_GATHER_PROFILE_FILENAME "${CMAKE_CURRENT_BINARY_DIR}/libcellml-*.profraw")
  set(DATA_PROFILE_FILENAME "${CMAKE_CURRENT_BINARY_DIR}/libcellml.profdata")

  unset(_CMD_STRING)
  foreach(_TEST ${TEST_LIST})
    list(APPEND _CMD_STRING COMMAND LLVM_PROFILE_FILE=${RAW_PROFILE_FILENAME} ./${_TEST} > /dev/null 2> /dev/null)
  endforeach()

  # Clean away old profile files
  add_custom_target(clean_llvm_coverage
    COMMAND ${FIND_EXE} ${CMAKE_CURRENT_BINARY_DIR} -type f -name '*.profraw' -print | xargs /bin/rm -f
    COMMAND ${FIND_EXE} ${CMAKE_CURRENT_BINARY_DIR} -type f -name '*.profdata' -print | xargs /bin/rm -f
    COMMENT "Clean old LLVM coverage data")

  add_custom_target(prepare_llvm_coverage
    ${_CMD_STRING}
    COMMAND ${LLVM_PROFDATA_EXE} merge -sparse ${RAW_GATHER_PROFILE_FILENAME} -o ${DATA_PROFILE_FILENAME}
    DEPENDS clean_llvm_coverage ${TEST_LIST}
    COMMENT "Prepare for LLVM coverage tests")

  # Unfortunately we cannot get an output in html format at this time as it is not supported at version 8.
  add_custom_target(llvm_coverage
    COMMAND ${LLVM_COV_EXE} report $<TARGET_FILE:cellml> -instr-profile=${DATA_PROFILE_FILENAME} -format=text -use-color
    DEPENDS prepare_llvm_coverage
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running LLVM coverage tests")

  add_custom_target(llvm_coverage_report
    COMMAND ${LLVM_COV_EXE} show $<TARGET_FILE:cellml> -instr-profile=${DATA_PROFILE_FILENAME} -format=html -Xdemangler c++filt -Xdemangler -n -o coverage_report --show-expansions --show-branches=count
    COMMAND echo "The coverage report can be found at ${CMAKE_CURRENT_BINARY_DIR}/${COVERAGE_REPORT_DIR}coverage_report/index.html."
    DEPENDS prepare_llvm_coverage
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running LLVM coverage missing lines command")
endif()

if(LIBCELLML_MEMCHECK)
  add_custom_target(prepare_memcheck
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/memcheck
    COMMENT "Prepare for memcheck tests")

  add_custom_target(memcheck
    COMMAND ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/valgrindmemcheck.py ${CMAKE_CURRENT_BINARY_DIR} ${TEST_LIST}
    DEPENDS ${TEST_LIST} prepare_memcheck
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/memcheck
    COMMENT "Running memcheck tests")
endif()

set(TESTS_SOURCE_FILES ${TESTS_SOURCE_FILES} PARENT_SCOPE)
set(TESTS_HEADER_FILES ${TESTS_HEADER_FILES} PARENT_SCOPE)
