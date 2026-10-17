#############################################
#
#  vcell-chombo -- the Chombo (embedded boundary, adaptive mesh) finite volume
#  solver used by the Virtual Cell framework.
#
##############################################
cmake_minimum_required(VERSION 3.20)

if (POLICY CMP0057)
	cmake_policy(SET CMP0057 NEW)
endif ()

project(vcell-chombo LANGUAGES CXX C Fortran)

#############################################
#  Declare options
##############################################
option(OPTION_TARGET_CHOMBO2D_SOLVER "Build the 2D solver" ON)
option(OPTION_TARGET_CHOMBO3D_SOLVER "Build the 3D solver" ON)
option(OPTION_TARGET_MESSAGING "Progress messaging (requires libcurl)" OFF)
option(OPTION_TARGET_PARALLEL "Build the MPI-parallel solver" OFF)
option(OPTION_EXTRA_CONFIG_INFO "Print useful cmake debug info while configuring" OFF)

if (NOT OPTION_TARGET_CHOMBO2D_SOLVER AND NOT OPTION_TARGET_CHOMBO3D_SOLVER)
	message(FATAL_ERROR "Nothing to build: enable OPTION_TARGET_CHOMBO2D_SOLVER and/or OPTION_TARGET_CHOMBO3D_SOLVER")
endif ()

if (OPTION_TARGET_PARALLEL)
	# The MPI plumbing survives from the in-tree build but has not been exercised
	# since this repo was split out.  Left as an explicit opt-in rather than a
	# silent fallback.
	message(WARNING "OPTION_TARGET_PARALLEL is not currently covered by CI -- expect to do some work")
endif ()

#############################################
#  Platform
##############################################
set(LINUX FALSE)
if (CMAKE_SYSTEM_NAME MATCHES "Linux")
	set(LINUX TRUE)
endif ()

if (NOT APPLE AND NOT LINUX)
	message(FATAL_ERROR "Unsupported operating system: ${CMAKE_SYSTEM_NAME}. "
			"Chombo's build system needs GNU make and a Unix shell; Windows builds went through Cygwin.")
endif ()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if (NOT DEFINED CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
	set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build" FORCE)
endif ()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
include(GetGitRevisionDescription)
git_describe(GIT_DESCRIBE)

#############################################
#  Dependencies
##############################################
find_package(HDF5 REQUIRED COMPONENTS C)
find_package(ZLIB REQUIRED)
find_package(libzip REQUIRED)

if (OPTION_TARGET_MESSAGING)
	find_package(CURL REQUIRED)
	set(THREADS_PREFER_PTHREAD_FLAG ON)
	find_package(Threads REQUIRED)
endif ()

if (OPTION_TARGET_PARALLEL)
	find_package(MPI REQUIRED)
endif ()

#############################################
#  Chombo
##############################################
include(cmake/BuildChomboLibs.cmake)

set(_chombo_previous "")
if (OPTION_TARGET_CHOMBO2D_SOLVER)
	add_chombo_dimension(2)
	set(_chombo_previous chombo_2d)
endif ()
if (OPTION_TARGET_CHOMBO3D_SOLVER)
	# 2D and 3D share chombo/lib -- generated headers included -- so they cannot
	# run concurrently.
	add_chombo_dimension(3 DEPENDS ${_chombo_previous})
endif ()

#############################################
#  Subdirectories
##############################################
if (NOT EXISTS "${CMAKE_SOURCE_DIR}/vcell-expressionparser/CMakeLists.txt"
		OR NOT EXISTS "${CMAKE_SOURCE_DIR}/vcell-messaging/CMakeLists.txt")
	message(FATAL_ERROR
			"The vcell-expressionparser and/or vcell-messaging submodule is empty. "
			"Run: git submodule update --init --recursive")
endif ()

add_subdirectory(vcell-expressionparser)
add_subdirectory(vcell-messaging)
add_subdirectory(VCellChombo)

enable_testing()
add_subdirectory(tests)

#############################################
#  Debug output
##############################################
if (OPTION_EXTRA_CONFIG_INFO)
	get_cmake_property(_variableNames VARIABLES)
	list(SORT _variableNames)
	foreach (_variableName ${_variableNames})
		message(STATUS "${_variableName}=${${_variableName}}")
	endforeach ()

	include(CMakePrintHelpers)
	cmake_print_variables(OPTION_TARGET_CHOMBO2D_SOLVER OPTION_TARGET_CHOMBO3D_SOLVER
			OPTION_TARGET_MESSAGING OPTION_TARGET_PARALLEL)
	cmake_print_variables(CMAKE_CXX_FLAGS CMAKE_C_FLAGS CMAKE_Fortran_FLAGS)
	cmake_print_variables(CMAKE_CXX_COMPILER CMAKE_C_COMPILER CMAKE_Fortran_COMPILER)
	cmake_print_variables(HDF5_INCLUDE_DIRS HDF5_LIBRARIES)
endif ()
