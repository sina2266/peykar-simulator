project(vcellmessaging CXX)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/GitDescribe.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/GitDescribe.cpp" @ONLY)
list(APPEND SOURCES  GitDescribe.h)

set(HEADER_FILES
	include/VCELL/SimulationMessaging.h
		include/VCELL/GitDescribe.h
		include/VCELL/MessageEventManager.h
		include/VCELL/CurlProxyClasses.h
		include/VCELL/JobEventStatus.h
		include/VCELL/WorkerEvent.h
)

set(SRC_FILES
	src/SimulationMessaging.cpp
		"${CMAKE_CURRENT_BINARY_DIR}/GitDescribe.cpp"
        src/MessageEventManager.cpp
		src/CurlProxyClasses.cpp
		src/JobEventStatus.cpp)

include_directories(include)

add_library(vcellmessaging ${SRC_FILES} ${HEADER_FILES})

# MessageEventManager uses std::thread/std::mutex unconditionally, so threads are
# always required -- not only when messaging is enabled.
if (NOT TARGET Threads::Threads)
	find_package(Threads REQUIRED)
endif ()
target_link_libraries(vcellmessaging PUBLIC Threads::Threads)

# std::format, ranges and friends
target_compile_features(vcellmessaging PUBLIC cxx_std_20)

target_include_directories(vcellmessaging INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>  # <prefix>/include
)
if (OPTION_TARGET_MESSAGING)
	if (OPTION_EXTRA_CONFIG_INFO)
		message(STATUS "CURL_LIBRARIES = '${CURL_LIBRARIES}'")
		message(STATUS "CURL_INCLUDE_DIR = '${CURL_INCLUDE_DIR}'")
	endif ()

	if (NOT CURL_LIBRARIES)
		find_package(CURL REQUIRED)
	endif ()

	target_link_libraries(vcellmessaging PUBLIC ${CURL_LIBRARIES}) # if MinGW is enabled, add `$<$<BOOL:${MINGW}>:ws2_32>`
	target_compile_definitions(vcellmessaging PUBLIC USE_MESSAGING=1)
	target_include_directories(vcellmessaging PUBLIC ${CURL_INCLUDE_DIR})
endif()

install(TARGETS vcellmessaging
	ARCHIVE DESTINATION bin
	RUNTIME DESTINATION bin)
