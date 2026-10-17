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
# limitations under the License.cmake_minimum_required (VERSION 3.1)

set(TEST_SRCS
  analyser.test.js
  analyserequation.test.js
  analyserequationast.test.js
  analyserexternalvariable.test.js
  analysermodel.test.js
  analyservariable.test.js
  annotator.test.js
  component.test.js
  componententity.test.js
  entity.test.js
  enums.test.js
  generator.test.js
  generatorvariabletracker.test.js
  generatorprofile.test.js
  importsource.test.js
  model.test.js
  namedentity.test.js
  parentedentity.test.js
  parser.test.js
  printer.test.js
  reset.test.js
  types.test.js
  units.test.js
  validator.test.js
  variable.test.js
  version.test.js
)

foreach(_TEST ${TEST_SRCS})
  configure_file(${_TEST} ${_TEST} COPYONLY)
endforeach()

# Copy over the resources file required by some tests.
configure_file(resources.js resources.js COPYONLY)

file(RELATIVE_PATH _RELATIVE_PATH ${CMAKE_CURRENT_BINARY_DIR} ${PROJECT_BINARY_DIR}/src/bindings/javascript/)

set(TEST_PACKAGE_JSON "{
  \"name\": \"libcellml.js-test\",
  \"version\": \"${PROJECT_VERSION}\",
  \"description\": \"Tests for libcellml.js\",
  \"main\": \"index.js\",
  \"scripts\": {
    \"test\": \"jest --detectOpenHandles\"
  },
  \"author\": \"\",
  \"license\": \"Apache-2.0\",
  \"devDependencies\": {
    \"jest\": \"^30.2.0\"
  },
  \"dependencies\": {
      \"libcellml.js\": \"file:${_RELATIVE_PATH}\"
    }
}
")

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/package.json" "${TEST_PACKAGE_JSON}")

add_custom_command(
    OUTPUT "package-lock.json"
    COMMAND ${NPM_EXE} install)

add_custom_target(install_modules DEPENDS "package-lock.json;libcellml;libcellml.common")

add_custom_target(jest_test COMMAND ${NPM_EXE} run test DEPENDS "install_modules")
set(_TEST_NAME javascript_api_tests)
add_test(NAME ${_TEST_NAME} COMMAND ${CMAKE_COMMAND} --build . --target jest_test)
