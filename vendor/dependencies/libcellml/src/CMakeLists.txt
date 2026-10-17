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

set(CLEAN_FILES)

foreach(VERSION_PART PROJECT_VERSION_MAJOR PROJECT_VERSION_MINOR PROJECT_VERSION_PATCH)
  string(LENGTH ${${VERSION_PART}} VERSION_PART_LENGTH)
  if(VERSION_PART_LENGTH EQUAL 1)
    set(${VERSION_PART}_PAD 0)
  endif()
endforeach()
set(LIBCELLML_LIBRARY_VERSION 0x${PROJECT_VERSION_MAJOR_PAD}${PROJECT_VERSION_MAJOR}${PROJECT_VERSION_MINOR_PAD}${PROJECT_VERSION_MINOR}${PROJECT_VERSION_PATCH_PAD}${PROJECT_VERSION_PATCH})
set(LIBCELLML_LIBRARY_VERSION_STRING "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

set(LIBCELLML_EXPORTDEFINITIONS_H "${CMAKE_CURRENT_BINARY_DIR}/api/libcellml/exportdefinitions.h")
set(LIBCELLML_VERSIONCONFIG_H "${CMAKE_CURRENT_BINARY_DIR}/versionconfig.h")
set(LIBCELLML_VERSIONCONFIG_H_IN "${CMAKE_CURRENT_SOURCE_DIR}/configure/versionconfig.in.h")
configure_file(
  ${LIBCELLML_VERSIONCONFIG_H_IN}
  ${LIBCELLML_VERSIONCONFIG_H}
)

list(APPEND CLEAN_FILES ${LIBCELLML_VERSIONCONFIG_H} ${LIBCELLML_EXPORTDEFINITIONS_H})

set(SOURCE_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/analyser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/analyserequation.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/analyserequationast.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/analyserexternalvariable.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/analysermodel.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/analyservariable.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/annotator.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/commonutils.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/component.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/componententity.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/entity.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/enums.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/generator.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/generatorprofile.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/generatorprofiletools.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/generatorvariabletracker.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/importedentity.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/importer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/importsource.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/internaltypes.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/issue.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/logger.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/mathmldtd.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/model.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/namedentity.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/parentedentity.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/parser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/printer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/reset.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/strict.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/types.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/units.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/utilities.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/validator.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/variable.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/version.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/xmlattribute.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/xmldoc.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/xmlnode.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/xmlutils.cpp
)

set(GIT_API_HEADER_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/api/libcellml/analyser.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/libcellml/analyserequation.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/libcellml/analyserequationast.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/libcellml/analyserexternalvariable.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/libcellml/analysermodel.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/libcellml/analyservariable.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/libcellml/annotator.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/libcellml/component.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/libcellml/componententity.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/libcellml/entity.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/libcellml/enums.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/libcellml/generator.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/libcellml/generatorprofile.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/libcellml/generatorvariabletracker.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/libcellml/importedentity.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/libcellml/importer.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/libcellml/importsource.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/libcellml/issue.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/libcellml/logger.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/libcellml/model.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/libcellml/namedentity.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/libcellml/parentedentity.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/libcellml/parser.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/libcellml/printer.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/libcellml/reset.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/libcellml/strict.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/libcellml/types.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/libcellml/undefines.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/libcellml/units.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/libcellml/validator.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/libcellml/variable.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/libcellml/version.h
)

set(API_HEADER_FILES
  ${GIT_API_HEADER_FILES}
  ${LIBCELLML_EXPORTDEFINITIONS_H}
)

set(MODULE_HEADER_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/api/libcellml/module/libcellml
)

set(GIT_HEADER_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/analyser_p.h
  ${CMAKE_CURRENT_SOURCE_DIR}/analyserequation_p.h
  ${CMAKE_CURRENT_SOURCE_DIR}/analyserequationast_p.h
  ${CMAKE_CURRENT_SOURCE_DIR}/analyserexternalvariable_p.h
  ${CMAKE_CURRENT_SOURCE_DIR}/analysermodel_p.h
  ${CMAKE_CURRENT_SOURCE_DIR}/analyservariable_p.h
  ${CMAKE_CURRENT_SOURCE_DIR}/anycellmlelement_p.h
  ${CMAKE_CURRENT_SOURCE_DIR}/commonutils.h
  ${CMAKE_CURRENT_SOURCE_DIR}/component_p.h
  ${CMAKE_CURRENT_SOURCE_DIR}/componententity_p.h
  ${CMAKE_CURRENT_SOURCE_DIR}/debug.h
  ${CMAKE_CURRENT_SOURCE_DIR}/entity_p.h
  ${CMAKE_CURRENT_SOURCE_DIR}/generator_p.h
  ${CMAKE_CURRENT_SOURCE_DIR}/generatorprofile_p.h
  ${CMAKE_CURRENT_SOURCE_DIR}/generatorprofilesha1values.h
  ${CMAKE_CURRENT_SOURCE_DIR}/generatorprofiletools.h
  ${CMAKE_CURRENT_SOURCE_DIR}/generatorvariabletracker_p.h
  ${CMAKE_CURRENT_SOURCE_DIR}/internaltypes.h
  ${CMAKE_CURRENT_SOURCE_DIR}/issue_p.h
  ${CMAKE_CURRENT_SOURCE_DIR}/logger_p.h
  ${CMAKE_CURRENT_SOURCE_DIR}/mathmldtd.h
  ${CMAKE_CURRENT_SOURCE_DIR}/model_p.h
  ${CMAKE_CURRENT_SOURCE_DIR}/namedentity_p.h
  ${CMAKE_CURRENT_SOURCE_DIR}/namespaces.h
  ${CMAKE_CURRENT_SOURCE_DIR}/parentedentity_p.h
  ${CMAKE_CURRENT_SOURCE_DIR}/reset_p.h
  ${CMAKE_CURRENT_SOURCE_DIR}/units_p.h
  ${CMAKE_CURRENT_SOURCE_DIR}/utilities.h
  ${CMAKE_CURRENT_SOURCE_DIR}/variable_p.h
  ${CMAKE_CURRENT_SOURCE_DIR}/xmlattribute.h
  ${CMAKE_CURRENT_SOURCE_DIR}/xmldoc.h
  ${CMAKE_CURRENT_SOURCE_DIR}/xmlnode.h
  ${CMAKE_CURRENT_SOURCE_DIR}/xmlutils.h
)

set(HEADER_FILES
  ${GIT_HEADER_FILES}
  ${LIBCELLML_VERSIONCONFIG_H}
)

# Only does anything if configuration target is Visual Studio.
group_source_to_dir_structure(
  ${SOURCE_FILES}
  ${HEADER_FILES}
  ${MODULE_HEADER_FILES}
  ${API_HEADER_FILES}
)

add_library(cellml
  ${SOURCE_FILES}
  ${HEADER_FILES}
  ${MODULE_HEADER_FILES}
  ${API_HEADER_FILES}
)

set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/xmldoc.cpp PROPERTIES
  COMPILE_DEFINITIONS XML_ERROR_CALLBACK_ARGUMENT_TYPE=${CONST_ERROR_STRUCTURED_ERROR_CALLBACK_TYPE})

generate_export_header(cellml EXPORT_FILE_NAME ${LIBCELLML_EXPORTDEFINITIONS_H} BASE_NAME LIBCELLML)

target_include_directories(cellml
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/api>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/api>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/api/libcellml/module>
  $<INSTALL_INTERFACE:include/>
  $<INSTALL_INTERFACE:include/libcellml/module>
  PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}
)

if(HAVE_ZLIB_TARGET)
  target_link_libraries(cellml PUBLIC zlib)
else()
  target_link_libraries(cellml PUBLIC ${ZLIB_LIBRARIES})
  target_include_directories(cellml PUBLIC ${ZLIB_INCLUDE_DIRS})
endif()

target_link_libraries(cellml PUBLIC ${LIBXML2_TARGET} ${ZLIB_TARGET})

# Use target compile features to propagate features to consuming projects.
target_compile_features(cellml PUBLIC cxx_std_20)

set_target_properties(cellml PROPERTIES
  CXX_VISIBILITY_PRESET hidden
  DEBUG_POSTFIX d
  VISIBILITY_INLINES_HIDDEN 1
  VERSION ${PROJECT_VERSION}${PROJECT_DEVELOPER_VERSION}
  INTERFACE_libCellML_MAJOR_VERSION ${PROJECT_VERSION_MAJOR}
  COMPATIBLE_INTERFACE_STRING ${PROJECT_VERSION_MAJOR}
)

if(NOT LIBCELLML_BUILD_SHARED AND CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set_target_properties(cellml PROPERTIES POSITION_INDEPENDENT_CODE 1)
endif()

set(DEBUG_SOURCE_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/debug.cpp
)

set(DEBUG_HEADER_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/debug.h
)

add_library(cellml_debug_utilities STATIC ${DEBUG_SOURCE_FILES} ${DEBUG_HEADER_FILES})

target_compile_features(cellml_debug_utilities PUBLIC cxx_std_20)

target_include_directories(cellml_debug_utilities
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/api>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/api>
)

target_link_libraries(cellml_debug_utilities PUBLIC ${LIBXML2_TARGET})

set_target_properties(cellml_debug_utilities PROPERTIES
  CXX_VISIBILITY_PRESET hidden
  DEBUG_POSTFIX d
  POSITION_INDEPENDENT_CODE 1
  VISIBILITY_INLINES_HIDDEN 1
)

target_link_libraries(cellml PRIVATE debug $<BUILD_INTERFACE:cellml_debug_utilities>)

group_source_to_dir_structure(
  ${DEBUG_SOURCE_FILES}
  ${DEBUG_HEADER_FILES}
)

if(LIBCELLML_CLANG_TIDY)
  configure_clang_and_clang_tidy_settings(cellml)
endif()

if(NOT UNIX AND NOT CYGWIN AND NOT MINGW)
  set_target_properties(cellml PROPERTIES
    IMPORT_PREFIX "lib"
    PREFIX "lib"
  )
endif()

if(LIBCELLML_TREAT_WARNINGS_AS_ERRORS)
  target_warnings_as_errors(cellml)
endif()

add_subdirectory(bindings)

function(append_target_property _TARGET _PROPERTY _VALUE)
  get_target_property(_CURRENT_PROPERTY_VALUE ${_TARGET} ${_PROPERTY})
  if(_CURRENT_PROPERTY_VALUE)
    set(_APPENDED_PROPERTY "${_CURRENT_PROPERTY_VALUE} ${_VALUE}")
  else()
    set(_APPENDED_PROPERTY "${_VALUE}")
  endif()
  set_target_properties(${_TARGET} PROPERTIES ${_PROPERTY} "${_APPENDED_PROPERTY}")
endfunction()

if(LIBCELLML_COVERAGE)
  append_target_property(cellml COMPILE_FLAGS "-fprofile-arcs -ftest-coverage")
  append_target_property(cellml LINK_FLAGS "-fprofile-arcs -ftest-coverage")

  # Share some paths with interested parties (tests)
  set(GCOV_ANALYSIS_PATH "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/cellml.dir" PARENT_SCOPE)
  set(GCOV_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}" PARENT_SCOPE)
  set(GCOV_BUILD_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}" PARENT_SCOPE)

  set(GCOV_CLEAN_FILES)
  foreach(src_file ${SOURCE_FILES})
    get_filename_component(_tmp_file "${src_file}" NAME)
    list(APPEND GCOV_CLEAN_FILES "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/cellml.dir/${_tmp_file}.gcno")
    list(APPEND GCOV_CLEAN_FILES "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/cellml.dir/${_tmp_file}.gcda")
  endforeach()
  set_directory_properties(${GCOV_ANALYSIS_PATH} PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${GCOV_CLEAN_FILES}")
endif()

if(LIBCELLML_LLVM_COVERAGE)
  append_target_property(cellml COMPILE_FLAGS "-fprofile-instr-generate -fcoverage-mapping")
  append_target_property(cellml LINK_FLAGS "-fprofile-instr-generate")
endif()

install(TARGETS cellml EXPORT libcellml-targets
  COMPONENT runtime
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

if(SKBUILD)
  install(TARGETS cellml
    COMPONENT wheel
    RUNTIME DESTINATION libcellml
    LIBRARY DESTINATION libcellml
    NAMELINK_SKIP
    EXCLUDE_FROM_ALL
  )
endif()

install(FILES
  ${API_HEADER_FILES}
  DESTINATION include/libcellml
  COMPONENT development
)

install(FILES
  ${MODULE_HEADER_FILES}
  DESTINATION include/libcellml/module
  COMPONENT development
)

include(CMakePackageConfigHelpers)
set(BUILD_TREE_CONFIG_DIR ${PROJECT_BINARY_DIR})
set(LIBCELLML_CONFIG_VERSION_FILE "${BUILD_TREE_CONFIG_DIR}/libcellml-config-version.cmake")
write_basic_package_version_file(
  ${LIBCELLML_CONFIG_VERSION_FILE}
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)
list(APPEND CLEAN_FILES ${LIBCELLML_CONFIG_VERSION_FILE})

set(RESOLVE_LIBXML2_FILE "${BUILD_TREE_CONFIG_DIR}/ResolveLibXml2.cmake")
set(RESOLVE_ZLIB_FILE "${BUILD_TREE_CONFIG_DIR}/ResolveZLIB.cmake")
configure_file("${PROJECT_SOURCE_DIR}/cmake/libcellml/ResolveLibXml2.cmake" "${RESOLVE_LIBXML2_FILE}" COPYONLY)
configure_file("${PROJECT_SOURCE_DIR}/cmake/libcellml/ResolveZLIB.cmake" "${RESOLVE_ZLIB_FILE}" COPYONLY)
set(LIBCELLML_TARGETS_FILE "${BUILD_TREE_CONFIG_DIR}/libcellml-targets.cmake")
export(EXPORT libcellml-targets FILE ${LIBCELLML_TARGETS_FILE})
list(APPEND CLEAN_FILES ${LIBCELLML_TARGETS_FILE} ${RESOLVE_LIBXML2_FILE} ${RESOLVE_ZLIB_FILE})

set(LIBCELLML_CONFIG_CMAKE_FILE "${BUILD_TREE_CONFIG_DIR}/libcellml-config.cmake")
configure_file("${PROJECT_SOURCE_DIR}/cmake/libcellml-config.cmake"
  ${LIBCELLML_CONFIG_CMAKE_FILE}
  @ONLY
)
list(APPEND CLEAN_FILES ${LIBCELLML_CONFIG_CMAKE_FILE})

if(WIN32)
  set(ConfigPackageLocation ${PROJECT_NAME}/CMake)
else()
  set(ConfigPackageLocation lib/cmake/${PROJECT_NAME})
endif()
install(EXPORT libcellml-targets
  FILE libcellml-targets.cmake
  DESTINATION ${ConfigPackageLocation}
  COMPONENT development
)

install(
  FILES
    ${LIBCELLML_CONFIG_CMAKE_FILE}
    ${LIBCELLML_CONFIG_VERSION_FILE}
    ${PROJECT_SOURCE_DIR}/cmake/libcellml/ResolveLibXml2.cmake
    ${PROJECT_SOURCE_DIR}/cmake/libcellml/ResolveZLIB.cmake
  DESTINATION ${ConfigPackageLocation}
  COMPONENT development
)

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${CLEAN_FILES}")

set(SOURCE_FILES ${SOURCE_FILES} PARENT_SCOPE)
set(API_HEADER_FILES ${GIT_API_HEADER_FILES} PARENT_SCOPE)
set(MODULE_HEADER_FILES ${MODULE_HEADER_FILES} PARENT_SCOPE)
set(HEADER_FILES ${GIT_HEADER_FILES} PARENT_SCOPE)
set(GENERATED_HEADER_FILES
  ${LIBCELLML_VERSIONCONFIG_H}
  PARENT_SCOPE
)
set(CONFIG_FILES
  ${LIBCELLML_VERSIONCONFIG_H_IN}
  PARENT_SCOPE
)
