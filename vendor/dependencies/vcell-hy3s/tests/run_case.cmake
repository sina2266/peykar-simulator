# Drives one solver run and then checks what it produced.
#
# Hy3S writes its solution back into the model file it is handed, so each case
# works on a private copy -- otherwise the tests would consume their own input
# and only pass once.
#
# Invoked as:
#   cmake -DSOLVER=... -DCHECKER=... -DMODEL=... -DWORKDIR=... -DNAME=...
#         -DSOLVER_ARGS=... -DCHECK_ARGS=... -P run_case.cmake
# with the two ARGS variables semicolon-separated lists.

if (NOT SOLVER OR NOT CHECKER OR NOT MODEL OR NOT WORKDIR OR NOT NAME)
	message(FATAL_ERROR "run_case.cmake: SOLVER, CHECKER, MODEL, WORKDIR and NAME are all required")
endif ()

if (NOT REPEAT)
	set(REPEAT 1)
endif ()

set(work "${WORKDIR}/${NAME}.nc")
file(MAKE_DIRECTORY "${WORKDIR}")

# Running more than once is not redundancy. The defect that prompted these
# tests fired on only some runs of the same binary with the same arguments --
# an uninitialised flag whose value depended on what happened to be on the
# stack -- so a single run is not evidence. Repeating also asserts the property
# that failure violated: a fixed seed must give the same answer every time.
set(first_hash "")
foreach (attempt RANGE 1 ${REPEAT})
	file(REMOVE "${work}")
	file(COPY_FILE "${MODEL}" "${work}")

	execute_process(
			COMMAND "${SOLVER}" "${work}" ${SOLVER_ARGS}
			RESULT_VARIABLE solver_rc
			OUTPUT_VARIABLE solver_out
			ERROR_VARIABLE solver_err)

	# The solver exits 0 even when it fails, which is the whole reason the
	# checker below exists. A non-zero code is still worth reporting.
	if (NOT solver_rc EQUAL 0)
		message("---- solver output ----\n${solver_out}\n${solver_err}")
		message(FATAL_ERROR "run ${attempt}: solver exited ${solver_rc}")
	endif ()

	# "Program is stopping" is how the solver reports a modelling failure on
	# its way to exiting 0. Catching it here gives a clearer message than the
	# checker would, and covers refusals that leave the file untouched.
	if (solver_out MATCHES "Program is stopping" OR solver_out MATCHES "Error in propogater")
		message("---- solver output ----\n${solver_out}")
		message(FATAL_ERROR "run ${attempt}: solver reported a failure while still exiting 0")
	endif ()

	file(MD5 "${work}" this_hash)
	if (first_hash STREQUAL "")
		set(first_hash "${this_hash}")
	elseif (NOT this_hash STREQUAL first_hash)
		message("---- solver output ----\n${solver_out}")
		message(FATAL_ERROR
				"run ${attempt} of ${REPEAT} differs from the first run with the same seed: "
				"${this_hash} vs ${first_hash}")
	endif ()
endforeach ()

if (REPEAT GREATER 1)
	message("  ok   reproducible: ${REPEAT} runs with the same seed agree exactly")
endif ()

execute_process(
		COMMAND "${CHECKER}" "${work}" ${CHECK_ARGS}
		RESULT_VARIABLE check_rc
		OUTPUT_VARIABLE check_out
		ERROR_VARIABLE check_err)

message("${check_out}")
if (NOT check_rc EQUAL 0)
	message("${check_err}")
	message(FATAL_ERROR "hy3s_check reported failures (exit ${check_rc})")
endif ()
