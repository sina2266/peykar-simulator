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

if(SPHINX_FOUND)
  set(SPHINX_DOCS_TARGET project_docs)

  if(NOT DEFINED SPHINX_THEME)
    set(SPHINX_THEME theme)
  endif()

  if(NOT DEFINED SPHINX_THEME_DIR)
    set(SPHINX_THEME_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
  endif()

  string(TIMESTAMP SPHINX_YEAR "%Y")

  # configured documentation tools and intermediate build results
  set(BINARY_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/_build")

  # Sphinx cache with pickled ReST documents
  set(SPHINX_CACHE_DIR "${CMAKE_CURRENT_BINARY_DIR}/_doctrees")

  # HTML output directory
  set(SPHINX_HTML_DIR "${CMAKE_CURRENT_BINARY_DIR}/html")

  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/conf.in.py"
    "${BINARY_BUILD_DIR}/conf.py"
    @ONLY)

  add_custom_target(${SPHINX_DOCS_TARGET}
    COMMAND ${SPHINX_EXECUTABLE}
        -q -b html
        -c "${BINARY_BUILD_DIR}"
        -d "${SPHINX_CACHE_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${SPHINX_HTML_DIR}"
    COMMENT "Building HTML documentation with Sphinx")
endif()

if(DOXYGEN_FOUND)
  set(DOXYGEN_DOCS_TARGET api_docs)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in.config ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
  add_custom_target(${DOXYGEN_DOCS_TARGET}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/html/code/
    COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen" VERBATIM)
  set(DOXYGEN_API_XML_DOCS_TARGET api_xml_docs)
  set(_DOXYFILE ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.xml)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.api-xml.in.config ${_DOXYFILE} @ONLY)
  add_custom_target(${DOXYGEN_API_XML_DOCS_TARGET}
    COMMAND ${DOXYGEN_EXECUTABLE} ${_DOXYFILE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation in XML format with Doxygen" VERBATIM)
endif()

if(LIBCELLML_COVERAGE)
  set(COVERAGE_DOCS_TARGET coverage_docs)
  add_custom_target(${COVERAGE_DOCS_TARGET}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/html/coverage/
    COMMAND ${Python_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tests/gcovr-4.1.1.zip --exclude-unreachable-branches -r ${GCOV_PREFIX_PATH} --html --html-details --output=${CMAKE_CURRENT_BINARY_DIR}/html/coverage/index.html --gcov-executable=${GCOV_EXE} ${GCOV_ANALYSIS_PATH}
    DEPENDS prepare_coverage
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running coverage tests with html output")
endif()

if(SPHINX_FOUND OR DOXYGEN_FOUND OR LIBCELLML_COVERAGE)
  add_custom_target(docs
    DEPENDS ${SPHINX_DOCS_TARGET} ${DOXYGEN_DOCS_TARGET} ${COVERAGE_DOCS_TARGET} ${DOXYGEN_API_XML_DOCS_TARGET}
    COMMENT "Generating documentation")
  add_custom_target(clean_docs
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/html
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/doxygen-xml
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/_doctrees
    COMMENT "Cleaning documentation")
endif()
