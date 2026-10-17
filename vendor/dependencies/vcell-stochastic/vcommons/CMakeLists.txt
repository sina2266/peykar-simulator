project(vcommons LANGUAGES CXX)

file(GLOB SRC_FILES src/*cpp)
file(GLOB HEADER_FILES include/*h include/*hpp)

set_source_files_properties(${SRC_FILES} PROPERTIES LANGUAGE CXX)

add_library(vcommons ${SRC_FILES} ${HEADER_FILES})

target_include_directories(vcommons
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

install(TARGETS vcommons
        ARCHIVE DESTINATION lib
)