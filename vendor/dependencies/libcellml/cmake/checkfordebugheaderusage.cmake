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

foreach(_INDEX RANGE 3 ${CMAKE_ARGC})
  set(_FILE ${CMAKE_ARGV${_INDEX}})
  if (EXISTS ${_FILE})
    file(STRINGS ${_FILE} _FILE_MATCHING_CONTENT REGEX "^[ \\t\\r\\n]*#[ \\t\\r\\n]*include[ \\t\\r\\n]+\"debug\\.h\"")
      string(LENGTH "${_FILE_MATCHING_CONTENT}" _LENGTH)
      if(${_LENGTH} GREATER 0)
        message(FATAL_ERROR "Detected a source file that uses the debug.h header file '${_FILE}'.")
      endif()
  endif()
endforeach()

