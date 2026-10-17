# Drives the vendored Chombo build (chombo/lib/GNUmakefile) from CMake.
#
# Chombo has no CMake build of its own; it ships a large, self-contained GNU
# make system that also runs the ChomboFortran preprocessor (perl) over the
# .ChF sources.  Rather than reimplement that, we invoke it -- but with every
# configuration variable passed on the `make` command line, where it overrides
# anything in a makefile.  That keeps the source tree free of the generated
# mk/Make.defs.local that the old in-tree build used to copy into place, so two
# builds with different settings can share one checkout.
#
# Two quirks of Chombo's makefiles are worth knowing:
#
#   * Dependency generation is written for csh (`CSHELLCMD`), which is not
#     installed by default on modern Linux or macOS.  The command it runs is a
#     plain pipeline, so /bin/sh executes it identically.
#   * Everything under lib/ (generated *_F.H headers, lib/include, the archives)
#     is shared between dimensions.  The 2D and 3D builds must therefore be
#     serialized, and each one's output snapshotted before the next runs.

set(CHOMBO_SOURCE_DIR "${PROJECT_SOURCE_DIR}/chombo" CACHE PATH "vendored Chombo checkout")
set(CHOMBO_LIB_DIR "${CHOMBO_SOURCE_DIR}/lib")

# Link order matters for static archives: dependants first.
set(CHOMBO_LINK_ORDER
		mftools
		ebamrelliptic
		ebamrtools
		ebtools
		amrelliptic
		amrtools
		workshop
		boxtools
		basetools)

include(ProcessorCount)
ProcessorCount(_chombo_default_jobs)
if (_chombo_default_jobs EQUAL 0)
	set(_chombo_default_jobs 1)
endif ()
set(CHOMBO_BUILD_JOBS "${_chombo_default_jobs}" CACHE STRING "parallelism for the Chombo GNU make build")

find_program(CHOMBO_MAKE_PROGRAM NAMES gmake make REQUIRED
		DOC "GNU make, used to build the vendored Chombo libraries")

# Chombo's makefiles want a current GNU make, and its documentation says as
# much. macOS is the trap: /usr/bin/make is GNU make 3.81, frozen in 2006 over
# the GPLv3 licence change. Homebrew's `make` installs as gmake, which is why
# gmake is searched for first above.
#
# 3.81 was initially suspected of breaking ChomboFortran's header generation.
# That turned out to be util/mkdep/mkdep dying on an absent include directory
# instead, so whether 3.81 would otherwise cope here is untested -- this floor is
# a deliberate requirement rather than a workaround for a known failure.
execute_process(COMMAND "${CHOMBO_MAKE_PROGRAM}" --version
		OUTPUT_VARIABLE _make_version_text
		ERROR_QUIET
		OUTPUT_STRIP_TRAILING_WHITESPACE)
if (NOT _make_version_text MATCHES "GNU Make ([0-9]+)\\.([0-9]+)")
	message(FATAL_ERROR
			"${CHOMBO_MAKE_PROGRAM} is not GNU make. Chombo's build system requires it.")
endif ()
if (CMAKE_MATCH_1 LESS 4)
	message(FATAL_ERROR
			"${CHOMBO_MAKE_PROGRAM} is GNU make ${CMAKE_MATCH_1}.${CMAKE_MATCH_2}; Chombo needs 4.0 or newer.\n"
			"On macOS, /usr/bin/make is 3.81. "
			"Install a current one (`brew install make`, which provides `gmake`) and configure again.")
endif ()
message(STATUS "Chombo will build with ${CHOMBO_MAKE_PROGRAM} (GNU make ${CMAKE_MATCH_1}.${CMAKE_MATCH_2})")
find_program(CHOMBO_PERL_PROGRAM NAMES perl REQUIRED
		DOC "perl, used by Chombo's ChomboFortran preprocessor")
find_program(CHOMBO_BASH_PROGRAM NAMES bash REQUIRED
		DOC "bash, used to run Chombo's pipelines with pipefail set")

# Chombo picks its compiler flag set by matching the basename of $(CXX)/$(FC)
# against names it knows (g++, gfortran, icpc, ...).  CMake often hands us the
# generic /usr/bin/c++ driver, which Chombo would not recognise -- it would fall
# back to a bare `-O` with none of the warning suppressions the sources expect.
function(_chombo_compiler_name out_var compiler_path compiler_id)
	get_filename_component(_name "${compiler_path}" NAME_WE)
	if (_name STREQUAL "c++" OR _name STREQUAL "cc")
		if (compiler_id STREQUAL "GNU")
			set(_name "g++")
		elseif (compiler_id MATCHES "Clang")
			set(_name "clang++")
		endif ()
	endif ()
	set(${out_var} "${_name}" PARENT_SCOPE)
endfunction()

# Chombo takes HDF5 as raw compiler/linker flag strings.  `make lib` only
# compiles and archives, so the link flags are never actually used -- but
# lib/mk/check refuses to build unless it can find libhdf5 in the -L paths, so
# they have to be real.
function(_chombo_hdf5_flags inc_var lib_var)
	set(_inc "-DH5_USE_16_API")
	foreach (dir ${HDF5_INCLUDE_DIRS})
		string(APPEND _inc " -I${dir}")
	endforeach ()

	# Conan's HDF5 package exposes only interface targets, so there is no
	# IMPORTED_LOCATION to read; look next to the include dirs instead, which
	# works the same way for a system HDF5.
	set(_hints "")
	foreach (dir ${HDF5_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS})
		get_filename_component(_root "${dir}" DIRECTORY)
		list(APPEND _hints "${_root}/lib" "${_root}")
	endforeach ()

	find_library(CHOMBO_HDF5_C_LIBRARY NAMES hdf5 HINTS ${_hints}
			DOC "libhdf5 as passed to Chombo's HDFLIBFLAGS")
	find_library(CHOMBO_ZLIB_LIBRARY NAMES z zlib HINTS ${_hints}
			DOC "libz as passed to Chombo's HDFLIBFLAGS")
	if (NOT CHOMBO_HDF5_C_LIBRARY OR NOT CHOMBO_ZLIB_LIBRARY)
		message(FATAL_ERROR
				"Could not locate libhdf5/libz next to ${HDF5_INCLUDE_DIRS}. "
				"Set CHOMBO_HDF5_C_LIBRARY and CHOMBO_ZLIB_LIBRARY explicitly.")
	endif ()

	get_filename_component(_hdf5_dir "${CHOMBO_HDF5_C_LIBRARY}" DIRECTORY)
	get_filename_component(_zlib_dir "${CHOMBO_ZLIB_LIBRARY}" DIRECTORY)

	set(${inc_var} "${_inc}" PARENT_SCOPE)
	set(${lib_var} "-L${_hdf5_dir} -L${_zlib_dir} -lhdf5 -lz" PARENT_SCOPE)
endfunction()

##
# add_chombo_dimension(<dim> [DEPENDS <target>])
#
# Defines a target `chombo_<dim>d` that builds the Chombo libraries for
# CH_SPACEDIM=<dim>, and sets in the caller's scope:
#
#   CHOMBO_<dim>D_INCLUDE_DIR  -- snapshot of Chombo's public headers
#   CHOMBO_<dim>D_LIBRARIES    -- the archives, in link order
##
function(add_chombo_dimension DIM)
	cmake_parse_arguments(ARG "" "" "DEPENDS" ${ARGN})

	set(_out "${CMAKE_BINARY_DIR}/chombo/${DIM}d")
	set(_stamp "${_out}/chombo${DIM}d.stamp")

	set(_libs "")
	foreach (base ${CHOMBO_LINK_ORDER})
		list(APPEND _libs "${_out}/lib/lib${base}${DIM}d.a")
	endforeach ()

	_chombo_compiler_name(_cxx_name "${CMAKE_CXX_COMPILER}" "${CMAKE_CXX_COMPILER_ID}")
	_chombo_compiler_name(_fc_name "${CMAKE_Fortran_COMPILER}" "${CMAKE_Fortran_COMPILER_ID}")

	# The Fortran compiler has to run the C preprocessor over ChomboFortran's
	# output.  Chombo generates it with `g++ -E -P -C`, keeping comments on
	# purpose -- stripping them would eat Fortran's `//` string concatenation
	# operator -- so the C comment blocks that survive (starting with gcc's
	# implicit stdc-predef.h) are only removed on this second pass.  Chombo splits
	# $(FC) on whitespace to recover the compiler name, so appending here is safe.
	if (CMAKE_Fortran_COMPILER_ID STREQUAL "GNU")
		set(_fc_name "${_fc_name} -cpp")
	endif ()

	_chombo_hdf5_flags(_hdf_inc_flags _hdf_lib_flags)

	if (CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(_use64 TRUE)
	else ()
		set(_use64 FALSE)
	endif ()

	add_custom_command(
			OUTPUT ${_libs} ${_stamp}
			COMMENT "Building Chombo ${DIM}D libraries (this takes a few minutes)"
			COMMAND ${CHOMBO_MAKE_PROGRAM} -j${CHOMBO_BUILD_JOBS} lib
			# --- what gets built ---
			DIM=${DIM}
			USE_EB=TRUE          # embedded boundary code -- the whole point for VCell
			USE_MF=TRUE          # multifluid, requires USE_EB
			USE_HDF=TRUE
			USE_64=${_use64}
			MPI=FALSE
			DEBUG=FALSE
			OPT=TRUE
			# --- toolchain ---
			CXX=${_cxx_name}
			FC=${_fc_name}
			PERL=${CHOMBO_PERL_PROGRAM}
			# Chombo runs its dependency generation and ChomboFortran passes
			# through this shell. It is written for csh, which is not installed by
			# default on modern Linux or macOS, and the commands are plain
			# pipelines that bash runs identically.
			#
			# pipefail is the point of using bash rather than sh here. Every one of
			# those commands has the shape `producer | filter > output`, and a
			# shell reports only the LAST component's status, so a failed producer
			# looks like success and leaves a well-formed empty file behind. Three
			# separate macOS failures hid behind exactly that: mkdep dying into an
			# empty .d, and cpp failing into an empty .cpre and .f. Chombo half-saw
			# this coming -- the .d rules delete the output afterwards if it is
			# empty -- but deleting it does not fail the build either, and the
			# ChomboFortran rules have no such guard at all.
			#
			# pipefail has been in bash since 3.0, so even the 3.2 that macOS still
			# ships is fine.
			CSHELLCMD=${CHOMBO_BASH_PROGRAM}\ -o\ pipefail\ -c
			# The C preprocessor Chombo runs over ChomboFortran's output. Set
			# explicitly because the Darwin block in lib/mk/Make.defs forces
			# CH_CPP=/usr/bin/cpp -E, working around g77 not supporting -E; g77 is
			# long gone and Apple's cpp is the wrong tool. It preprocesses
			# traditionally, and against Chombo's indented directives it recognises
			# an indented #else/#endif while ignoring an indented #ifdef, so the
			# nesting desynchronises and BaseNamespaceHeader.H fails with "#else
			# without #if".
			#
			# This is what Linux resolves to anyway ($(CXX) -E -P from
			# Make.defs.defaults, plus -C from Make.defs.GNU), so both platforms now
			# take the same path. -C is essential and not cosmetic: it keeps
			# comments, without which the preprocessor eats Fortran's // operator.
			CH_CPP=${_cxx_name}\ -E\ -P\ -C
			# --- HDF5 ---
			HDFINCFLAGS=${_hdf_inc_flags}
			HDFLIBFLAGS=${_hdf_lib_flags}
			# Deliberately NOT setting fcppflags=-cpp here, which VCell's old
			# Make.defs.local.linux did and which was carried over for parity.
			# fcppflags is appended to $(CH_CPP), the C preprocessor run over
			# ChomboFortran's output, where -cpp means nothing -- it is a compiler
			# flag telling gfortran to preprocess, and the one that matters is the
			# -cpp appended to $(FC) above. Linux tolerated it because CH_CPP is
			# `g++ -E -P -C` there and g++ accepts the flag; on macOS the Darwin
			# block sets CH_CPP to Apple's /usr/bin/cpp, which rejects it outright.
			#
			# It failed quietly, too: that step is `$(CH_CPP) ... | awk ... > out`,
			# and a shell pipeline reports awk's exit status, not cpp's. So the
			# .cpre came out empty, then the .f, then an object file with no
			# symbols, and the build only fell over at link with undefined Fortran
			# references far from the cause.
			WORKING_DIRECTORY "${CHOMBO_LIB_DIR}"
			COMMAND ${CMAKE_COMMAND}
			-DCHOMBO_LIB_DIR=${CHOMBO_LIB_DIR}
			-DDIM=${DIM}
			-DOUT_DIR=${_out}
			"-DEXPECTED_LIBS=${CHOMBO_LINK_ORDER}"
			-P "${PROJECT_SOURCE_DIR}/cmake/CollectChomboLibs.cmake"
			COMMAND ${CMAKE_COMMAND} -E touch "${_stamp}"
			VERBATIM)

	add_custom_target(chombo_${DIM}d DEPENDS ${_libs} ${_stamp})
	if (ARG_DEPENDS)
		add_dependencies(chombo_${DIM}d ${ARG_DEPENDS})
	endif ()

	set(CHOMBO_${DIM}D_INCLUDE_DIR "${_out}/include" PARENT_SCOPE)
	set(CHOMBO_${DIM}D_LIBRARIES "${_libs}" PARENT_SCOPE)
endfunction()
