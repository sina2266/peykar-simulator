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

if(LIBCELLML_BINDINGS_PYTHON) # Or any other bindings that use SWIG.
  include(${SWIG_USE_FILE})
endif()

if(LIBCELLML_BINDINGS_PYTHON)
  if(NOT LIBCELLML_BUILD_SHARED)
    message(WARNING "It is **not** recommended to build the Python bindings with static libraries.")
  endif()
  add_subdirectory(python)
endif()

if (EMSCRIPTEN)
  add_subdirectory(javascript)
endif ()
