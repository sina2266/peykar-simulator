project(VCellZipUtils CXX)
cmake_minimum_required(VERSION 3.5)

set(HEADER_FILES 
	include/VCELL/ZipUtils.h
)

set(SRC_FILES 
	src/ZipUtils.cpp
)

set(TEST_SRC_FILES
	test/Test.cpp
)

include_directories(include)

#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
#find_package(LibZip REQUIRED)
#if (LIBZIP_INCLUDE_DIR_ZIP AND LIBZIP_INCLUDE_DIR_ZIPCONF)
#    include_directories(${LIBZIP_INCLUDE_DIR_ZIP} ${LIBZIP_INCLUDE_DIR_ZIPCONF})
#    message(STATUS "Found libzip: ${LIBZIP_LIBRARY} ${LIBZIP_VERSION}")
#else ()
#	message(FATAL_ERROR "can't find libzip")
#endif()

add_library(vcellziputils STATIC ${SRC_FILES} ${HEADER_FILES})
target_include_directories(vcellziputils INTERFACE include)
target_link_libraries(vcellziputils libzippp::libzippp)

#add_executable(testzip ${TEST_SRC_FILES})
#target_link_libraries(testzip vcellziputils)


install(TARGETS vcellziputils
	ARCHIVE DESTINATION bin
	RUNTIME DESTINATION bin)
	
	
