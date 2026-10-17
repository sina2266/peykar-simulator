#############################################
#
#  vcell-hy3s -- the Hy3S hybrid stochastic solvers used by the Virtual Cell
#  framework. Three executables from one Fortran source set, differing only in
#  the SDE integrator they compile in.
#
##############################################
cmake_minimum_required(VERSION 3.20)

if (POLICY CMP0057)
	cmake_policy(SET CMP0057 NEW)
endif ()

project(vcell-hy3s LANGUAGES C CXX Fortran)

#############################################
#  Declare options
##############################################
option(OPTION_TARGET_MESSAGING "Progress messaging (requires libcurl)" OFF)
option(OPTION_TARGET_TESTS "Build the solver tests" ON)
option(OPTION_EXTRA_CONFIG_INFO "Print useful cmake debug info while configuring" OFF)

#############################################
#  Platform
##############################################
set(LINUX FALSE)
if (CMAKE_SYSTEM_NAME MATCHES "Linux")
	set(LINUX TRUE)
endif ()

# On Windows the intended pairing is Intel Fortran with MSVC, which is what
# this was originally built with. MSVC has no Fortran compiler of its own, so
# project() above will already have failed if none was supplied; this only
# catches the case where some other Fortran compiler was found, since the
# Windows-specific flags below assume Intel.
if (WIN32 AND NOT MINGW AND NOT CMAKE_Fortran_COMPILER_ID MATCHES "Intel")
	message(WARNING
			"On Windows this expects Intel Fortran (ifx) alongside MSVC, but found "
			"'${CMAKE_Fortran_COMPILER_ID}'. See README.md.")
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
#  Fortran dialect
##############################################
# Hy3S was an Intel Fortran project. Its conditional compilation used Intel's
# !DEC$ IF directives, which gfortran does not implement -- it treats them as
# comments and compiles the guarded code regardless, so a messaging-free build
# would still try to link the messaging calls. Those 114 directives were
# converted to standard #if/#else/#endif when this repo was split out, which
# means the Fortran now has to go through the C preprocessor.
# The flag is applied to the Hy3S targets only, not globally. The vendored
# NetCDF does not need preprocessing, and running cpp over it turns every
# apostrophe in a Fortran comment -- doesn't, can't, haven't -- into a warning
# about an unterminated character constant.
if (CMAKE_Fortran_COMPILER_ID STREQUAL "GNU")
	# -cpp is gfortran's spelling of Intel's -fpp. The line-length flag is for
	# the original sources, which assume no 132-column limit.
	set(HY3S_Fortran_FLAGS -cpp -ffree-line-length-none)
elseif (CMAKE_Fortran_COMPILER_ID MATCHES "Intel")
	# The original Makefiles passed no preprocessor flag at all: Intel handled
	# !DEC$ IF natively as compiler directives, with -D supplying the symbols.
	# Converting them to #if is what makes a preprocessor pass necessary.
	if (WIN32)
		set(HY3S_Fortran_FLAGS /fpp)
	else ()
		set(HY3S_Fortran_FLAGS -fpp)
	endif ()
else ()
	message(WARNING
			"Unrecognised Fortran compiler ${CMAKE_Fortran_COMPILER_ID}; the sources need "
			"the C preprocessor run over them and no flag is being added for that.")
	set(HY3S_Fortran_FLAGS "")
endif ()

# /iface:mixed_str_len_arg governs where Fortran puts the hidden length that
# accompanies every character argument: immediately after the string, rather
# than all lengths appended at the end of the argument list. The vendored
# NetCDF F77 layer is C compiled with -DPowerStationFortran, and cfortran.h
# under that setting expects the first of those. Both sides have to agree.
#
# Unlike the preprocessor flag above this is set globally, and has to be: it is
# a calling convention, so netcdf_f90 and the Hy3S targets that call into it
# must be compiled the same way. The original build did exactly this, in two
# places -- CMAKE_Fortran_FLAGS for the F90 layer and FOPTIONS in
# Makefile.win32 for Hy3S itself.
#
# Dropping it during the repo split is what made all three Windows binaries
# die with an access violation on their first NetCDF call: nf90_open passed a
# path whose length the C side then read from the wrong place.
if (WIN32 AND CMAKE_Fortran_COMPILER_ID MATCHES "Intel")
	set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} /iface:mixed_str_len_arg")
endif ()

#############################################
#  Dependencies
##############################################
# NetCDF is vendored rather than taken from Conan. Conan Center publishes the C
# library only -- no .mod files and no netcdf-fortran recipe -- and Hy3S does
# `USE netcdf`. Building Unidata's netcdf-fortran through FetchContent does not
# work either: it references CMAKE_SOURCE_DIR internally and so insists on being
# the top-level project. The vendored 3.6.2 tree carries its own C, F77 and F90
# layers, is what this code was written against, and builds clean with modern
# gcc/gfortran.
add_subdirectory(netcdf)

if (OPTION_TARGET_MESSAGING)
	if (NOT EXISTS "${CMAKE_SOURCE_DIR}/vcell-messaging/CMakeLists.txt")
		message(FATAL_ERROR
				"The vcell-messaging submodule is empty. Run: git submodule update --init --recursive")
	endif ()
	find_package(CURL REQUIRED)
	set(THREADS_PREFER_PTHREAD_FLAG ON)
	find_package(Threads REQUIRED)
	add_subdirectory(vcell-messaging)
endif ()

add_subdirectory(Hy3S)

if (OPTION_TARGET_TESTS)
	enable_testing()
	add_subdirectory(tests)
endif ()

#############################################
#  Debug output
##############################################
if (OPTION_EXTRA_CONFIG_INFO)
	include(CMakePrintHelpers)
	cmake_print_variables(OPTION_TARGET_MESSAGING)
	cmake_print_variables(CMAKE_Fortran_COMPILER CMAKE_Fortran_COMPILER_ID CMAKE_Fortran_FLAGS)
	cmake_print_variables(CMAKE_C_COMPILER CMAKE_CXX_COMPILER)
endif ()
