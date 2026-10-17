
set(SOURCE_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/analyser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/analyserequation.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/analyserequationast.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/analyserexternalvariable.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/analysermodel.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/analyservariable.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/annotator.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/component.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/componententity.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/entity.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/enums.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/generator.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/generatorprofile.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/generatorvariabletracker.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/importedentity.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/importer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/importsource.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/issue.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/logger.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/model.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/namedentity.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/parentedentity.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/parser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/printer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/reset.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/units.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/types.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/validator.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/variable.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/version.cpp
)

add_library(javascript-bindings OBJECT ${SOURCE_FILES})
target_include_directories(javascript-bindings PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(javascript-bindings PUBLIC cellml)

# I want to add '-fwasm-exceptions' to the COMPILE_FLAGS but Node doesn't support
# this yet.  This is a reminder to enable it when Node does support exceptions.
set_target_properties(javascript-bindings PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
  CXX_VISIBILITY_PRESET hidden
  VISIBILITY_INLINES_HIDDEN 0
)

add_executable(libcellml $<TARGET_OBJECTS:javascript-bindings>)

target_link_libraries(libcellml PUBLIC javascript-bindings)

# Don't need this anymore (breaks linking)? -s USE_ES6_IMPORT_META=0
set_target_properties(libcellml PROPERTIES
  LINK_FLAGS "--bind -s ASSERTIONS=1 -s 'EXPORT_NAME=\"createLibCellML\"' -s EXPORT_ES6=1 -s ALLOW_MEMORY_GROWTH=1"
)

add_executable(libcellml.common $<TARGET_OBJECTS:javascript-bindings>)

target_link_libraries(libcellml.common PUBLIC javascript-bindings)

set_target_properties(libcellml.common PROPERTIES
  LINK_FLAGS "--bind -s 'EXPORT_NAME=\"libCellML\"' -s MODULARIZE=1 -s ALLOW_MEMORY_GROWTH=1"
)

configure_file(README.md README.md COPYONLY)
configure_file(vue-app-with-libcellml.png vue-app-with-libcellml.png COPYONLY)
configure_file(.npmrc .npmrc COPYONLY)

set(_PACKAGE_JSON "\{
  \"name\": \"libcellml.js\",
  \"version\": \"${PROJECT_VERSION}${PROJECT_DEVELOPER_VERSION}\",
  \"repository\": \{
    \"type\": \"git\",
    \"url\": \"git+https://github.com/cellml/libcellml.git\"
  },
  \"description\": \"libcellml.js javascript web assembly for libCellML.\",
  \"main\": \"libcellml.js\",
  \"files\": [\"libcellml.common.js\", \"libcellml.common.wasm\", \"libcellml.js\", \"libcellml.wasm\", \"README.md\", \"vue-app-with-libcellml.png\"],
  \"author\": \"libCellML Developers <developers@libcellml.org>\",
  \"license\": \"Apache-2.0\"
}
")

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/package.json" "${_PACKAGE_JSON}")
