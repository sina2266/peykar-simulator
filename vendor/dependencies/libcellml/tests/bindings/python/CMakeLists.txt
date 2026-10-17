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

set(TEST_SRCS
  test_analyser.py
  test_analyser_equation_ast.py
  test_analyser_external_variable.py
  test_annotator.py
  test_component.py
  test_docstrings.py
  test_entity.py
  test_generator.py
  test_generator_profile.py
  test_generator_variable_tracker.py
  test_import_requirement.py
  test_import_source.py
  test_importer.py
  test_issue.py
  test_model.py
  test_parser.py
  test_printer.py
  test_reset.py
  test_units.py
  test_validator.py
  test_variable.py
  test_version.py
)

include(test_resources.cmake)


if(MSVC)
  if(NOT POLICY CMP0078)
    set(_PYTHON_MODULE_TARGET _entity)
      else()
    set(_PYTHON_MODULE_TARGET entity)
  endif()
  set(_RUN_PYTHON_ENVIRONMENT "PATH=$<TARGET_FILE_DIR:cellml>\;$<TARGET_FILE_DIR:${_PYTHON_MODULE_TARGET}>\;%PATH%;PYTHONPATH=${PROJECT_BINARY_DIR}/src/bindings/python${CONFIG_DIR_SUFFIX}")
else()
  set(_RUN_PYTHON_ENVIRONMENT "PYTHONPATH=${PROJECT_BINARY_DIR}/src/bindings/python")
endif()

foreach(_TEST ${TEST_SRCS})
  configure_file(${_TEST} ${_TEST} COPYONLY)
  get_filename_component(_TEST_NAME ${_TEST} NAME_WE)
  set(_TEST_NAME python_${_TEST_NAME})
  add_test(NAME ${_TEST_NAME} COMMAND ${Python_EXECUTABLE} ${_TEST})
  set_tests_properties(${_TEST_NAME} PROPERTIES
    ENVIRONMENT "${_RUN_PYTHON_ENVIRONMENT}")
  set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${_TEST}c
    ${CMAKE_CURRENT_BINARY_DIR}/${_TEST})
endforeach()

if(LIBCELLML_PYTHON_COVERAGE)
  configure_file(runner.py runner.py @ONLY)

  set(_CONFIG_ARGUMENT ".")
  if(IS_MULTI_CONFIG)
    set(_CONFIG_ARGUMENT "/$<CONFIG>")
  endif()

  add_custom_target(python_coverage
    COMMAND ${CMAKE_COMMAND} -E env "${_RUN_PYTHON_ENVIRONMENT}" ${Python_EXECUTABLE} runner.py ${_CONFIG_ARGUMENT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS python_bindings
    COMMENT "Running Python coverage tests")

  add_custom_target(python_coverage_report
    COMMAND ${CMAKE_COMMAND} -E env ${_RUN_PYTHON_ENVIRONMENT} ${Python_EXECUTABLE} runner.py ${_CONFIG_ARGUMENT} html
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS python_bindings
    COMMENT "Running Python coverage tests with html report")
endif()

set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES
    ${CMAKE_CURRENT_BINARY_DIR}/__pycache__)
