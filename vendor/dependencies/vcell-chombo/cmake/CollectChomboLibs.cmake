# Run via `cmake -P` from the custom command in BuildChomboLibs.cmake, right
# after Chombo's own GNUmakefile finishes.
#
# Chombo bakes its whole configuration into the archive name --
# libboxtools2d.Linux.64.g++.gfortran.OPT.a -- so the file name depends on the
# host OS, the pointer size, the compiler basenames, and the DEBUG/OPT/PROFILE
# settings.  Reproducing that string in CMake is what made the old in-tree build
# so brittle, so instead we let Chombo name the archives however it likes and
# glob for them here, copying each one to a stable path the CMake targets can
# reference.
#
# Chombo also writes its public headers (including the ChomboFortran-generated
# *_F.H prototypes) to lib/include, and those ARE dimension-dependent -- a 3D
# build overwrites what the 2D build put there.  Snapshot them per dimension so
# both solvers can be compiled from one configure, in any order.
#
# Expected -D arguments: CHOMBO_LIB_DIR, DIM, OUT_DIR, EXPECTED_LIBS

foreach (required CHOMBO_LIB_DIR DIM OUT_DIR EXPECTED_LIBS)
	if (NOT DEFINED ${required})
		message(FATAL_ERROR "CollectChomboLibs.cmake: -D${required} is required")
	endif ()
endforeach ()

file(MAKE_DIRECTORY "${OUT_DIR}/lib")

foreach (base ${EXPECTED_LIBS})
	file(GLOB candidates "${CHOMBO_LIB_DIR}/lib${base}${DIM}d.*.a")
	list(LENGTH candidates count)
	if (count EQUAL 0)
		message(FATAL_ERROR
				"Chombo did not produce lib${base}${DIM}d.*.a in ${CHOMBO_LIB_DIR}. "
				"The `make lib` step above should have failed -- check its output.")
	elseif (count GREATER 1)
		# Stale archives from an earlier configuration (a different compiler, or
		# a DEBUG build) would otherwise be picked up at random.
		message(FATAL_ERROR
				"Found ${count} candidates for lib${base}${DIM}d in ${CHOMBO_LIB_DIR}: ${candidates}. "
				"Remove the stale ones (or run `make realclean` in chombo/lib) and configure again.")
	endif ()
	file(COPY_FILE "${candidates}" "${OUT_DIR}/lib/lib${base}${DIM}d.a" ONLY_IF_DIFFERENT)
endforeach ()

file(REMOVE_RECURSE "${OUT_DIR}/include")
file(COPY "${CHOMBO_LIB_DIR}/include/" DESTINATION "${OUT_DIR}/include")
