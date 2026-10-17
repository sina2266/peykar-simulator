cmake_minimum_required(VERSION 3.18.0)

set(MATHML_DTD_LOCATION "${PROJECT_SOURCE_DIR}/src/dtds/mathml2/xhtml-math11-f.dtd")
configure_file(compress.in.h compress.h)

add_executable(compress compress.cpp compress.h)
set_target_properties(compress PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE EXCLUDE_FROM_ALL TRUE)
target_include_directories(compress PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
if(MSVC)
  target_compile_definitions(compress PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
if(HAVE_ZLIB_TARGET)
  target_link_libraries(compress PRIVATE zlib)
else()
  target_link_libraries(compress PRIVATE ${ZLIB_LIBRARIES})
  target_include_directories(compress PRIVATE ${ZLIB_INCLUDE_DIRS})
endif()

file(SIZE "${MATHML_DTD_LOCATION}" MATHML_DTD_SIZE)
add_custom_command(OUTPUT mathmldtd.h
  COMMENT "Generating compressed MathML DTD in mathmldtd.{h, cpp}"
  COMMAND compress
  COMMAND ${CMAKE_COMMAND} -P "${PROJECT_SOURCE_DIR}/cmake/binarytochararray.cmake" "${CMAKE_CURRENT_BINARY_DIR}/compressed.dat" ${MATHML_DTD_SIZE}
  DEPENDS compress)

add_custom_target(create_mathml_dtd_files
  DEPENDS mathmldtd.h
  COMMENT "Create mathmldtd files (in source directory, Eek!).")

add_executable(calculatesha1 calculatesha1.cpp ${PROJECT_SOURCE_DIR}/src/generatorprofiletools.cpp)
set_target_properties(calculatesha1 PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE EXCLUDE_FROM_ALL TRUE)
target_include_directories(calculatesha1 PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(calculatesha1 PRIVATE cellml)

add_custom_command(OUTPUT generatorprofilesha1values.cmake
  COMMENT "Updating SHA-1 values for C and Python generator profiles."
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS calculatesha1
  COMMAND calculatesha1
  COMMAND ${CMAKE_COMMAND} -P "${PROJECT_SOURCE_DIR}/cmake/generateprofilesha1header.cmake")
add_custom_target(update_generator_profile_sha1_values
  DEPENDS generatorprofilesha1values.cmake
  COMMENT "Update SHA-1 values for C and Python generator profiles (in source directory, Eek!).")
