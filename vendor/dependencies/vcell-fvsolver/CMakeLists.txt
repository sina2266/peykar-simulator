#############################################
#
#  Top Level CMake File
#
##############################################
cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(OPTION_TARGET_PYTHON_BINDING "build Python bindings" OFF)
option(OPTION_TARGET_MESSAGING "Messaging (requires libcurl)" OFF)
option(OPTION_TARGET_DOCS "Generate Doxygen documentation" OFF)
option(OPTION_TARGET_FV_SOLVER ON)
option(OPTION_TARGET_TESTS "Build tests" OFF)
option(OPTION_VCELL "Compile Smoldyn for VCell" ON)
option(OPTION_NSV "Compile Smoldyn with NextSubvolume functionality" OFF)
option(OPTION_USE_OPENGL "Build with OpenGL support" OFF)
option(OPTION_USE_ZLIB "Build with Zlib support" ON)
option(OPTION_USE_LIBTIFF "Build with LibTiff support" OFF)
option(OPTION_USE_ICONV "Build with Libiconv support" OFF)
option(OPTION_BUILD_SHARED_LIBS "Enable Build Shared Libs mode" OFF)

# need to statically link everything for the python bindings and standalone solvers
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
set(BUILD_SHARED_LIBS ${OPTION_BUILD_SHARED_LIBS})

# If there's a toolchain defined in the env, let's get it early
if(DEFINED ENV{CMAKE_TOOLCHAIN_FILE})
	set(CMAKE_TOOLCHAIN_FILE $ENV{CMAKE_TOOLCHAIN_FILE})
endif()

if (OPTION_TARGET_PYTHON_BINDING)
	project(
			${SKBUILD_PROJECT_NAME}
			VERSION ${SKBUILD_PROJECT_VERSION}
			LANGUAGES CXX C Fortran
	)
	set(OPTION_TARGET_MESSAGING OFF)
	set(OPTION_TARGET_FV_SOLVER ON)
	set(OPTION_TARGET_DOCS OFF)
	set(OPTION_VCELL ON)
	set(OPTION_NSV OFF)
	set(OPTION_USE_OPENGL OFF)
	set(OPTION_USE_ZLIB ON)
	set(OPTION_USE_LIBTIFF OFF)
	set(OPTION_USE_ICONV OFF)
else()
	project(
			fvsolver
			LANGUAGES CXX C Fortran
	)
	set(OPTION_TARGET_LIBSMOLDYN ON)
	set(OPTION_VCELL ON)
endif ()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(OPTION_USE_OPENGL OFF CACHE BOOL "")

if(POLICY CMP0057)
	cmake_policy(SET CMP0057 NEW)
endif()

set(LINUX FALSE)
if (${CMAKE_SYSTEM_NAME} MATCHES Linux)
	set(LINUX TRUE)
endif()

set(WINDOWS FALSE)
if (${CMAKE_SYSTEM_NAME} MATCHES Windows)
	set(WINDOWS TRUE)
	set(WIN32 TRUE)
endif()

set (ARCH_64bit FALSE)
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
	set (ARCH_64bit TRUE)
endif()

if (NOT APPLE AND NOT LINUX AND NOT WINDOWS)
	message(FATAL_ERROR "Unsupported Operating System or C/C++ Compiler.")
else()
	if (APPLE)
		message("Performing MacOS installation...")
	endif ()
	if (LINUX)
		message("Performing Linux installation...")
	endif ()
	if (WINDOWS)
		message("Performing Windows installation...")
	endif ()
endif()

if(NOT DEFINED CMAKE_C_COMPILER)
	set(CMAKE_C_COMPILER "clang")
endif()
if(NOT DEFINED CMAKE_CXX_COMPILER)
	set(CMAKE_CXX_COMPILER "clang++")
endif()
if(NOT DEFINED CMAKE_Fortran_COMPILER)
	set(CMAKE_Fortran_COMPILER "flang")
endif()

if (APPLE)
	execute_process(COMMAND uname -m OUTPUT_VARIABLE PROCESSOR_ARCHITECTURE OUTPUT_STRIP_TRAILING_WHITESPACE)
	message("Processor Architecture: ${PROCESSOR_ARCHITECTURE}")
	if (PROCESSOR_ARCHITECTURE STREQUAL "arm64")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__arm64__")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__arm64__")
		set(CMAKE_OSX_ARCHITECTURES "arm64")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__x86_64__")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__x86_64__")
		set(CMAKE_OSX_ARCHITECTURES "x86_64")
	endif()
elseif (LINUX)
	option(LINUX_32bit_BINARIES "Build 32bit Linux BINARIES" OFF)
	option(LINUX_64bit_BINARIES "Build 64bit Linux BINARIES" ON)

	if (LINUX_32bit_BINARIES AND LINUX_64bit_BINARIES)
		message(FATAL_ERROR "It is required to select either 32bit or 64bit Linux Binaries, not both")
	endif()

	if (LINUX_32bit_BINARIES) 
		set (ARCH_64bit FALSE)
		set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
		set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
		set (CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -m32")
	endif()
	
	# -m64 is an x86 flag: gcc rejects it on aarch64 (clang tolerates it)
	if (LINUX_64bit_BINARIES AND CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
		set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
		set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64")
		set (CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -m64")
	endif()
elseif (WINDOWS)
	if(NOT DEFINED CMAKE_C_COMPILER)
		set(CMAKE_C_COMPILER "clang.exe")
	endif()
	if(NOT DEFINED CMAKE_CXX_COMPILER)
		set(CMAKE_CXX_COMPILER "clang++.exe")
	endif()
	if(NOT DEFINED CMAKE_Fortran_COMPILER)
		set(CMAKE_CXX_COMPILER "flang.exe")
	endif()
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/libzippp/cmake/")

include(GetGitRevisionDescription)
git_describe(GIT_DESCRIBE ALLOW_LOOKING_ABOVE_CMAKE_SOURCE_DIR)
# Release builds made without a .git directory (the container build) pass the version in.
set(VCELL_VERSION_STRING "" CACHE STRING "Version reported by the executables (overrides git describe)")
if (VCELL_VERSION_STRING)
	set(GIT_DESCRIBE "${VCELL_VERSION_STRING}")
endif()

# help CMake find libzip
if (APPLE)
	execute_process(COMMAND brew --prefix libzip OUTPUT_VARIABLE LIBZIP_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
	set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${LIBZIP_PREFIX})
endif()

include (FindZLIB)
include (FindLIBZIP)

find_package(LIBZIP 1.10 REQUIRED)


if (NOT OPTION_TARGET_PYTHON_BINDING)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

	if(DEFINED CMAKE_BUILD_TYPE)
	   SET(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel.")
	else()
	   SET(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel.")
	endif()

	if (${OPTION_TARGET_DOCS})
		add_custom_target(graphviz ALL
					  "${CMAKE_COMMAND}" "--graphviz=dependencies.dot" .
					  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
	endif()
endif(NOT OPTION_TARGET_PYTHON_BINDING)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Choose minimum deploy target for Macos machines")
endif()

add_definitions(-DFORTRAN_UNDERSCORE)

# No fused multiply-add contraction: gcc and clang fuse by default on arm64 (never on x86-64 without
# -march), which changes FiniteVolume results -- membrane areas, then the fluxes through them --
# by up to a few percent against the x86-64 builds and the 0.9.7 reference. Off, arm64 reproduces
# them bit for bit.
if (NOT WIN32)
	add_compile_options(-ffp-contract=off)
endif()

#############################################
# installation directories
#############################################
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
	find_package(CURL REQUIRED)
	set(THREADS_PREFER_PTHREAD_FLAG ON)
	find_package(Threads REQUIRED)
	add_definitions(-DUSE_MESSAGING)
endif()

set(HDF5_USE_STATIC_LIBRARIES ON)
if(WINDOWS OR WIN32 OR WIN64)
	set(HDF5_USE_STATIC_LIBRARIES OFF)
	add_definitions(-DZIP_STATIC)
endif ()
if (LINUX AND OPTION_TARGET_PYTHON_BINDING)
	set(HDF5_USE_STATIC_LIBRARIES OFF)
endif()

#CMakeList.txt that comes with hdf5 switches library name if compiled with debug
if (APPLE)
	execute_process(COMMAND brew --prefix libaec OUTPUT_VARIABLE LIBAEC_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
	set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${LIBAEC_PREFIX})
	find_package(libaec REQUIRED)
endif()
find_package(HDF5 COMPONENTS HL CXX C)
set(HDF_VERSION ${HDF5_VERSION})
######    ${HDF5_HL_LIBRARIES} ${HDF5_CXX_LIBRARIES}

if (OPTION_TARGET_TESTS)
	enable_testing()
	set(LIBZIPPP_BUILD_TESTS ON)
endif ()

add_subdirectory(VCellMessaging)

add_subdirectory(VCellZipUtils)

add_subdirectory(libzippp)

add_subdirectory(ExpressionParser)

add_subdirectory(sundials)

if (NOT APPLE)
	add_subdirectory(blas)
endif()

SET(HAVE_ZLIB TRUE)
add_subdirectory(bridgeVCellSmoldyn)
add_subdirectory(smoldyn-2.38)

add_subdirectory(VCell)
add_subdirectory(PCGPack)
add_subdirectory(qhull)

if (OPTION_TARGET_PYTHON_BINDING)
	# Find the module development requirements (requires FindPython from 3.17 or
	# scikit-build-core's built-in backport)
	find_package(Python REQUIRED COMPONENTS Interpreter Development.Module)
	find_package(pybind11 CONFIG QUIET)
	if(NOT pybind11_FOUND)
		message(STATUS "System pybind11 not found, using bundled version")
		add_subdirectory(extern/pybind11)
	endif()

	# Add a library using FindPython's tooling (pybind11 also provides a helper like this)
	python_add_library(_core MODULE src/main.cpp WITH_SOABI)
	target_link_libraries(_core PRIVATE vcell pybind11::headers)
	# This is passing in the version as a define just as an example
	target_compile_definitions(_core PRIVATE VERSION_INFO=${PROJECT_VERSION})

	# The install directory is the output (wheel) directory
	install(TARGETS _core DESTINATION pyvcell_fvsolver)

endif (OPTION_TARGET_PYTHON_BINDING)