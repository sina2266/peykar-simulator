project(qhull)

file (GLOB HEADER_FILES *.h)
file (GLOB SRC_FILES *.c)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-strict-aliasing")

if (MINGW)
	add_definitions(-fpermissive)
endif (MINGW)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../VCell/include)
set_source_files_properties( ${SRC_FILES} PROPERTIES LANGUAGE CXX )
add_library(qhull STATIC ${SRC_FILES} ${HEADER_FILES})
install(TARGETS qhull 
	ARCHIVE DESTINATION bin)

