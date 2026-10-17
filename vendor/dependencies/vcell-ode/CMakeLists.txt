#############################################
#
#  Top Level CMake File
#
##############################################
cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 20)
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif ()

#############################################
#  If this isn't explicitly set in cmake 3.x.x, we get warnings
##############################################
if(POLICY CMP0057)
	cmake_policy(SET CMP0057 NEW)
endif()

if (WIN32)
	cmake_policy(SET CMP0091 NEW)
endif ()

#############################################
#  Declare Options
##############################################
option(OPTION_TARGET_PYTHON_BINDING "build Python bindings" OFF)
option(OPTION_TARGET_MESSAGING "Messaging (requires libcurl)" OFF)
option(OPTION_TARGET_DOCS "Generate Doxygen documentation" OFF)
option(OPTION_TEST_WITH_LOCALHOST "Use localhost as server rather than vcell.cam.uchc.edu" OFF)
option(OPTION_EXTRA_CONFIG_INFO "Print useful cmake debug info while configuring" OFF)
option(OPTION_STATICALLY_LINK "Decided whether to statically link, or use (and generate) dynamic shared libs" OFF)

if(OPTION_TARGET_PYTHON_BINDING AND OPTION_TARGET_MESSAGING)
	message(FATAL_ERROR "Can not build python bindings and enable messaging in the same build")
endif()

#############################################
#  MinGW check
##############################################
if (MINGW)
	message("ERROR: Due to the requirements of the project, mingw builds are currently disabled. Native windows builds should still work.")
	return()
endif ()

#############################################
#  Static link check
##############################################

if (OPTION_STATICALLY_LINK)
	#set(BUILD_SHARED_LIBS OFF)
	set(CMAKE_EXE_LINKER_FLAGS "-static")
else ()
	set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif ()

#############################################
#  Python Binding check
##############################################
#if (OPTION_TARGET_PYTHON_BINDING)
#	project(
#			${SKBUILD_PROJECT_NAME}
#			VERSION ${SKBUILD_PROJECT_VERSION}
#			LANGUAGES CXX C
#	)
#	set(OPTION_TARGET_MESSAGING OFF)
#else()
#
#endif ()

project(
		vcell-ode-numerics
		LANGUAGES CXX C
)

#############################################
#
#  Build 64bit binaries on Mac and target Macos 10.7 or later
#
#	set(CMAKE_OSX_DEPLOYMENT_TARGET "10.7" CACHE PATH "")
#
#  Choose 32bit or 64bit target arch on Linux
#
##############################################
if (LINUX)
	option(LINUX_32bit_BINARIES "Build 32bit Linux BINARIES" OFF)
	option(LINUX_64bit_BINARIES "Build 64bit Linux BINARIES" ON)

	if (LINUX_32bit_BINARIES AND LINUX_64bit_BINARIES)
		message(FATAL_ERROR "It is required to select either 32bit or 64bit Linux Binaries, not both")
	endif()

	if (LINUX_32bit_BINARIES) 
		set (ARCH_64bit FALSE)
		set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
		set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
	endif()
	
	if (LINUX_64bit_BINARIES) 
		set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
		set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64")
	endif()
endif()

# Settings
set(RULE_MESSAGES OFF)
set(CMAKE_COLOR_MAKEFILE OFF)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)  # .exe, .dll
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)  # .dylib, .so (non-plugin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)  # .lib, .a (static libs / import libs)

include(GetGitRevisionDescription)
# VCELL_ODE_VERSION (e.g. -DVCELL_ODE_VERSION=0.9.5) overrides `git describe`: release and image
# builds pass it, since the Docker build context has no .git and CI checkouts may lack tags.
if (DEFINED VCELL_ODE_VERSION AND NOT VCELL_ODE_VERSION STREQUAL "")
	set(GIT_DESCRIBE "${VCELL_ODE_VERSION}")
else()
	git_describe(GIT_DESCRIBE)
endif()

if (NOT OPTION_TARGET_PYTHON_BINDING)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

	#include (VCellCMakeShowVariables.txt)

	if(DEFINED CMAKE_BUILD_TYPE)
	   SET(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel.")
	else()
	   SET(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel.")
	endif()


	if (${OPTION_TARGET_DOCS})
	#	if (DOXYGEN_FOUND AND ${CMAKE_VERSION} GREATER_EQUAL 3.9)
	#		find_package(Doxygen
	#				REQUIRED dot
	#				OPTIONAL_COMPONENTS mscgen dia)
	#		set(DOXYGEN_GENERATE_HTML YES)
	#		set(DOXYGEN_GENERATE_MAN NO)
	#		doxygen_add_docs(
	#   			doxygen
	#    		${PROJECT_SOURCE_DIR}
	#    		COMMENT "Generate man pages"
	#    	)
	#	else()
			add_custom_target(graphviz ALL
					  "${CMAKE_COMMAND}" "--graphviz=dependencies.dot" .
					  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
	#    endif()
	endif()
endif(NOT OPTION_TARGET_PYTHON_BINDING)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.7" CACHE STRING "Choose minimum deploy target for Macos machines")
endif()

set(HDF5_BUILD_CPP_LIB OFF CACHE BOOL "" )


#if (WIN32) # Only linux is designed at this point to support messaging, but unix should work too (and our devs generally work on macs)
#	set(OPTION_TARGET_MESSAGING OFF)
#endif()

set (ARCH_64bit FALSE)
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
	set (ARCH_64bit TRUE)
endif()

if (NOT APPLE AND NOT LINUX AND NOT WIN32)
	message(FATAL_ERROR "Unsupported Operating System")
endif()

#---------------------------
#  IDE SUPPORT
#---------------------------
if (WIN32)
	set(NETBEANS_WINDOWS TRUE)
	set(CMAKE_RC_FLAGS "-DGCC_WINDRES")
endif(WIN32)

#add_definitions(-DFORTRAN_UNDERSCORE)

#############################################
# installation directories
#############################################
# You can install spdlog with the following:
# > Debian: sudo apt install libspdlog-dev
# > Homebrew: brew install spdlog
# > Conan: conan install --requires=spdlog/[*] ( it may be easier just to add to the conan file!)
# > vcpkg: vcpkg install spdlog

if(NOT TARGET spdlog)
	# Stand-alone build
	find_package(spdlog REQUIRED)
endif()
if (NOT OPTION_TARGET_PYTHON_BINDING)
	if (NOT OPTION_TARGET_MESSAGING)
		set(OPTION_EXE_DIRECTORY "bin" CACHE PATH "installation directory")
	else (NOT OPTION_TARGET_MESSAGING)
		set(OPTION_EXE_DIRECTORY "../bin" CACHE PATH "installation directory")
	endif (NOT OPTION_TARGET_MESSAGING)

	set(OPTION_LIB_DIRECTORY "lib" CACHE PATH "library directory")
	set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}")
endif (NOT OPTION_TARGET_PYTHON_BINDING)

add_definitions(-DUNIX)


if (OPTION_TARGET_MESSAGING)
	#don't need graphics for server side solvers
	set(OPTION_USE_OPENGL OFF CACHE BOOL "")
	
	find_package(CURL REQUIRED)
	set(THREADS_PREFER_PTHREAD_FLAG ON)
	find_package(Threads REQUIRED)
	add_definitions(-DUSE_MESSAGING)
endif()

if (OPTION_TEST_WITH_LOCALHOST)
	add_definitions(-DTEST_WITH_LOCALHOST)
endif()

######################################
#
# Add subdirectories
# 
#######################################

add_subdirectory(vcell-messaging)
add_subdirectory(vcell-expressionparser)
add_subdirectory(sundials)
add_subdirectory(IDAWin)
enable_testing()
add_subdirectory(tests)

set(DOWNLOAD_EXTRACT_TIMESTAMP true)
cmake_policy(SET CMP0135 NEW)
#############################################
#
#  Declare and dynamically net-install dependencies
#
##############################################
include(FetchContent)
#############################################
#  Argparse
##############################################
FetchContent_Declare(
		argparse
		GIT_REPOSITORY https://github.com/p-ranav/argparse.git
		GIT_TAG          v3.2
)
FetchContent_MakeAvailable(argparse)


#############################################
#
#  Display all sorts of detailed debug information
#
##############################################
if (OPTION_EXTRA_CONFIG_INFO)
	get_cmake_property(_variableNames VARIABLES)
	list (SORT _variableNames)
	foreach (_variableName ${_variableNames})
		message(STATUS "${_variableName}=${${_variableName}}")
	endforeach()

	include(CMakePrintHelpers)
	cmake_print_variables(OPTION_TARGET_MESSAGING OPTION_TARGET_DOCS)
	cmake_print_variables(CMAKE_CXX_FLAGS CMAKE_C_FLAGS)
	cmake_print_variables(CMAKE_SYSTEM_NAME WIN32 APPLE LINUX ARCH_64bit ARCH_32bit)
	cmake_print_variables(CMAKE_CPP_COMPILER CMAKE_C_COMPILER CMAKE_CXX_COMPILER)
endif ()

if (OPTION_TARGET_PYTHON_BINDING)
	# Find the module development requirements (requires FindPython from 3.17 or
	# scikit-build-core's built-in backport)
	find_package(Python REQUIRED COMPONENTS Interpreter)
	# The install directory is the output (wheel) directory
	install(TARGETS IDAWin DESTINATION pyvcell_odesolver)

endif (OPTION_TARGET_PYTHON_BINDING)
