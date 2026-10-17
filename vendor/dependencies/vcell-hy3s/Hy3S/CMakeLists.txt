project(Hy3S LANGUAGES Fortran)

# One source set, three executables. They differ only in which SDE integrator is
# compiled in, selected by preprocessor definitions:
#
#   Hybrid_EM            Euler-Maruyama
#   Hybrid_MIL           Milstein, fixed step
#   Hybrid_MIL_Adaptive  Milstein, adaptive step
#
# The definitions must stay upper case. Intel Fortran's !DEC$ IF (Defined(X))
# matched case-insensitively, so the original sources wrote `Milstein` while the
# build defined `MILSTEIN`; the C preprocessor those directives were converted
# to draws no such equivalence, and the conversion normalised the sources to
# upper case to match this.
set(F90_SRC_FILES
		src/f2kcli.f90
		src/globalvariables.f90
		src/ratelaws.f90
		src/dataio.f90
		src/randomgen.f90
		src/priorityqueue.f90
		src/specialevents.f90
		src/SDEintegration.f90
		src/propagators-HyJCMSS.f90
		src/mainprogram-HyJCMSS.f90
)

# src/propagators-SSA.f90, src/mainprogram-SSA.f90 and
# src/Program-BenchMarkMaker.f90 are present but were not in the build before
# the split either; they are kept for reference rather than compiled.

if (OPTION_TARGET_MESSAGING)
	# msgwrapper is C++ despite being called from Fortran: it drives
	# SimulationMessaging. The names it exports carry a trailing underscore
	# because that is how gfortran mangles a module-less external.
	add_library(hy3s_messaging STATIC src/msgwrapper.cpp)
	target_link_libraries(hy3s_messaging PUBLIC vcellmessaging)
	target_compile_definitions(hy3s_messaging PUBLIC USE_MESSAGING)
endif ()

# mainprogram-HyJCMSS.f90 refuses to compile without SVNVERSION:
#
#     #ifndef SVNVERSION
#     #error "SVNVERSION required"
#     #endif
#
# and it goes into the version banner. It is defined already quoted, because
# gfortran's -cpp preprocesses in traditional mode and cannot stringify.
# Nothing in vcell-solvers ever
# defined it, so Hy3S could not have been built there either -- consistent with
# it being switched off in every CI recipe. The intent was to stamp the binary
# with a VCS revision, so the git describe already computed at the top level
# serves, falling back when this is not a git checkout.
#
# A release build passes -DHY3S_VERSION_STAMP=<X.Y.Z> explicitly: the container
# build has no .git to describe.
set(HY3S_VERSION_STAMP "" CACHE STRING "Version printed in the solver banner (default: git describe)")
if (NOT HY3S_VERSION_STAMP)
	if (GIT_DESCRIBE AND NOT GIT_DESCRIBE MATCHES "NOTFOUND")
		set(HY3S_VERSION_STAMP "${GIT_DESCRIBE}")
	else ()
		set(HY3S_VERSION_STAMP "unknown")
	endif ()
endif ()
message(STATUS "Hy3S version stamp: ${HY3S_VERSION_STAMP}")

##
# add_hy3s_solver(<name> [DEFINITIONS ...])
##
function(add_hy3s_solver NAME)
	cmake_parse_arguments(ARG "" "" "DEFINITIONS" ${ARGN})

	set(exe ${NAME})
	if (CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(exe ${NAME}_x64)
	endif ()

	add_executable(${exe} ${F90_SRC_FILES})
	# Each target compiles the same sources, so each generates the same .mod
	# files. Without a private module directory they collide and ninja refuses:
	# "multiple rules generate Hy3S/f2kcli.mod".
	set_target_properties(${exe} PROPERTIES
			LINKER_LANGUAGE Fortran
			Fortran_MODULE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/modules/${exe}")
	target_compile_options(${exe} PRIVATE ${HY3S_Fortran_FLAGS})

	# netcdf_f90 provides the module Hy3S does `USE netcdf` against; the module
	# file lands in that target's binary directory.
	target_include_directories(${exe} PRIVATE "${CMAKE_BINARY_DIR}/netcdf/f90")
	target_link_libraries(${exe} PRIVATE netcdf_f90 netcdf_for netcdf)

	target_compile_definitions(${exe} PRIVATE SVNVERSION="${HY3S_VERSION_STAMP}")

	if (ARG_DEFINITIONS)
		target_compile_definitions(${exe} PRIVATE ${ARG_DEFINITIONS})
	endif ()

	if (OPTION_TARGET_MESSAGING)
		target_compile_definitions(${exe} PRIVATE USE_MESSAGING)
		target_link_libraries(${exe} PRIVATE hy3s_messaging)
	endif ()

	install(TARGETS ${exe} RUNTIME DESTINATION bin)
endfunction()

add_hy3s_solver(Hybrid_EM)
add_hy3s_solver(Hybrid_MIL           DEFINITIONS MILSTEIN)
add_hy3s_solver(Hybrid_MIL_Adaptive  DEFINITIONS MILSTEIN ADAPTIVE)
