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

include("generatorprofilesha1values.cmake")
configure_file("${CMAKE_CURRENT_LIST_DIR}/../src/configure/generatorprofilesha1values.in.h" "${CMAKE_CURRENT_LIST_DIR}/../src/generatorprofilesha1values.h")
