project(vcellmessaging CXX)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/GitDescribe.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/GitDescribe.cpp" @ONLY)
list(APPEND SOURCES  GitDescribe.h)

set(HEADER_FILES 
	include/VCELL/SimulationMessaging.h
		include/VCELL/GitDescribe.h
)

set(SRC_FILES 
	src/SimulationMessaging.cpp
		"${CMAKE_CURRENT_BINARY_DIR}/GitDescribe.cpp")

include_directories(include)

add_library(vcellmessaging ${SRC_FILES} ${HEADER_FILES})

target_include_directories(vcellmessaging INTERFACE 
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>  # <prefix>/include
)
if (${OPTION_TARGET_MESSAGING})
	message(STATUS "CURL_LIBRARIES = '${CURL_LIBRARIES}'")
	message(STATUS "CURL_INCLUDE_DIR = '${CURL_INCLUDE_DIR}'")
	
	target_link_libraries(vcellmessaging ${CURL_LIBRARIES} Threads::Threads)
	target_compile_definitions(vcellmessaging PUBLIC USE_MESSAGING=1)
	target_include_directories(vcellmessaging PUBLIC ${CURL_INCLUDE_DIR})
endif()

install(TARGETS vcellmessaging
	ARCHIVE DESTINATION bin
	RUNTIME DESTINATION bin)
