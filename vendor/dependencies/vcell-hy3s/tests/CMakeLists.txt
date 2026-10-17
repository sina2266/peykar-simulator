# Tests for the Hy3S solvers.
#
# The model is Michaelis-Menten enzyme kinetics, S + E <-> ES -> E + P, which
# is what the sample file shipped with the original sources describes. It is a
# good test case because its stoichiometry gives two exact conservation laws --
# total enzyme E + ES, and total substrate S + ES + P -- that hold at every
# timepoint no matter which trajectory the random number generator produces.
# Those checks therefore need no baseline and no tolerance, and mean the same
# thing on every platform.
#
# The regression check against a committed baseline is separate and deliberately
# narrow: it only holds while the trajectory is reproducible, which it is with a
# fixed seed, and only against a baseline from the same compiler. See
# tests/README.md.

set(HY3S_TEST_MODEL "${CMAKE_CURRENT_SOURCE_DIR}/resources/enzyme.nc")
set(HY3S_TEST_WORKDIR "${CMAKE_CURRENT_BINARY_DIR}/work")

# The baseline is per compiler, not per platform. Fortran's RANDOM_NUMBER has
# no specified algorithm, so gfortran and Intel seed and advance different
# generators: the same seed gives a different stream and therefore a different,
# equally valid trajectory. Measured, once Windows started working -- Linux and
# both macOS architectures agree with each other to the bit, all being gfortran,
# while Intel diverges immediately and by a lot (State[0][1][0] 482 against
# 522). That is the RNG differing, not the solver.
#
# Only the baseline is affected. Every other check here follows from the
# reaction stoichiometry and holds on all four jobs regardless of trajectory.
if (CMAKE_Fortran_COMPILER_ID MATCHES "Intel")
	set(HY3S_BASELINE "${CMAKE_CURRENT_SOURCE_DIR}/resources/enzyme_baseline_intel.nc")
else ()
	set(HY3S_BASELINE "${CMAKE_CURRENT_SOURCE_DIR}/resources/enzyme_baseline_gfortran.nc")
endif ()
message(STATUS "Hy3S test baseline: ${HY3S_BASELINE}")

add_executable(hy3s_check hy3s_check.c)
target_link_libraries(hy3s_check PRIVATE netcdf)
# The netcdf target sets no usage requirements of its own -- the vendored build
# relies on directory-scoped include_directories -- so name the headers here.
target_include_directories(hy3s_check PRIVATE
		"${CMAKE_SOURCE_DIR}/netcdf/libsrc")
if (MSVC)
	target_include_directories(hy3s_check PRIVATE "${CMAKE_SOURCE_DIR}/netcdf/win32")
	target_compile_definitions(hy3s_check PRIVATE _CRT_SECURE_NO_WARNINGS)
else ()
	target_include_directories(hy3s_check PRIVATE "${CMAKE_SOURCE_DIR}/netcdf/linux")
	target_link_libraries(hy3s_check PRIVATE m)
endif ()

# Both conservation laws, non-negativity, and the assertion that the run
# actually happened. The last one is not a formality: a defect fixed during the
# repo split left the solver exiting 0 having written nothing at all, so
# "did not crash" was not evidence of anything.
set(HY3S_INVARIANTS
		--ran
		--nonneg
		--conserve E_Cellular,ES_Cellular
		--conserve S_Cellular,ES_Cellular,P_Cellular
		--monotonic P_Cellular)

##
# add_hy3s_test(<name> SOLVER <target> ARGS <solver args...> [CHECKS <extra checks...>])
##
function(add_hy3s_test NAME)
	cmake_parse_arguments(ARG "" "SOLVER;REPEAT" "ARGS;CHECKS" ${ARGN})

	if (NOT ARG_REPEAT)
		set(ARG_REPEAT 1)
	endif ()

	add_test(NAME ${NAME}
			COMMAND ${CMAKE_COMMAND}
			-DSOLVER=$<TARGET_FILE:${ARG_SOLVER}>
			-DCHECKER=$<TARGET_FILE:hy3s_check>
			-DMODEL=${HY3S_TEST_MODEL}
			-DWORKDIR=${HY3S_TEST_WORKDIR}
			-DNAME=${NAME}
			-DREPEAT=${ARG_REPEAT}
			"-DSOLVER_ARGS=${ARG_ARGS}"
			"-DCHECK_ARGS=${HY3S_INVARIANTS};${ARG_CHECKS}"
			-P "${CMAKE_CURRENT_SOURCE_DIR}/run_case.cmake")
endfunction()

# Executable names carry the _x64 suffix on 64-bit builds, the same way the
# solver targets are named.
set(_sfx "")
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(_sfx "_x64")
endif ()

# epsilon and lambda are set high enough that every reaction stays a discrete
# jump, so the run is an exact stochastic simulation and the SDE integrator --
# the only thing that differs between the three binaries -- is not entered.
# That is what lets all three be checked against the same baseline.
set(HY3S_FIXED_ARGS    100000 1000000 0.01 1e-5       -R 12345 -OV)
# The adaptive binary takes an SDE tolerance ahead of the step size.
set(HY3S_ADAPTIVE_ARGS 100000 1000000 0.01 1e-3 1e-5  -R 12345 -OV)

add_hy3s_test(euler_maruyama
		SOLVER Hybrid_EM${_sfx}
		ARGS ${HY3S_FIXED_ARGS}
		CHECKS --baseline "${HY3S_BASELINE}")

add_hy3s_test(milstein
		SOLVER Hybrid_MIL${_sfx}
		ARGS ${HY3S_FIXED_ARGS}
		CHECKS --baseline "${HY3S_BASELINE}")

add_hy3s_test(milstein_adaptive
		SOLVER Hybrid_MIL_Adaptive${_sfx}
		ARGS ${HY3S_ADAPTIVE_ARGS}
		CHECKS --baseline "${HY3S_BASELINE}")

# A different seed must still satisfy every invariant, which is what makes the
# invariants worth having: they constrain the solver rather than one trajectory.
# No baseline here, because the trajectory legitimately differs.
add_hy3s_test(euler_maruyama_other_seed
		SOLVER Hybrid_EM${_sfx}
		ARGS 100000 1000000 0.01 1e-5 -R 999 -OV)

# The defect that prompted all of this fired on roughly one run in five of the
# same binary. Checking a single run missed it: with the fix reverted the three
# cases above still passed. Twenty runs makes the odds of missing it
# negligible, and asserts the property directly -- same seed, same answer.
add_hy3s_test(reproducibility
		SOLVER Hybrid_EM${_sfx}
		REPEAT 20
		ARGS ${HY3S_FIXED_ARGS}
		CHECKS --baseline "${HY3S_BASELINE}")
