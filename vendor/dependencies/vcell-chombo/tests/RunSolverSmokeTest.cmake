# ctest driver for the end-to-end solver runs.  Invoked via `cmake -P`; see
# tests/CMakeLists.txt for the arguments.
#
# Runs one solver against one .fvinput in a scratch directory and checks that it
# produced what VCell expects: a .mesh.hdf5, a .log listing one row per saved
# timepoint, and a .hdf5.zip holding the .sim.hdf5 files those rows name.
#
# Given BASELINE and COMPARATOR as well, it goes on to compare the values in the
# final timepoint against a stored baseline. That is the difference between
# "the pipeline ran" and "the solver still computes what it used to".
#
# Given ANALYTIC_EXPECTED instead, it checks the solver's own error against the
# closed-form solution rather than against its past self.
#
# Expected -D arguments: SOLVER, INPUT, WORK_DIR, BASE_NAME, EXPECTED_TIMEPOINTS
# Optional:              BASELINE, COMPARATOR, RTOL, ATOL
#                        ANALYTIC_EXPECTED, ANALYTIC_BAND, ANALYTIC_DATASET

foreach (required SOLVER INPUT WORK_DIR BASE_NAME EXPECTED_TIMEPOINTS)
	if (NOT DEFINED ${required})
		message(FATAL_ERROR "RunSolverSmokeTest.cmake: -D${required} is required")
	endif ()
endforeach ()

# Start clean: the solver appends to the .log and the .zip, so leftovers from a
# previous run would make the checks below pass for the wrong reason.
file(REMOVE_RECURSE "${WORK_DIR}")
file(MAKE_DIRECTORY "${WORK_DIR}")
get_filename_component(_input_name "${INPUT}" NAME)
file(COPY "${INPUT}" DESTINATION "${WORK_DIR}")

execute_process(
		COMMAND "${SOLVER}" "${_input_name}"
		WORKING_DIRECTORY "${WORK_DIR}"
		OUTPUT_VARIABLE solver_output
		ERROR_VARIABLE solver_output
		RESULT_VARIABLE solver_result)

if (NOT solver_result EQUAL 0)
	message(FATAL_ERROR "${SOLVER} exited ${solver_result}:\n${solver_output}")
endif ()

# The solver catches its own exceptions and can still exit 0 in some paths.
if (solver_output MATCHES "Exception :")
	message(FATAL_ERROR "${SOLVER} reported an exception:\n${solver_output}")
endif ()

if (NOT EXISTS "${WORK_DIR}/${BASE_NAME}.mesh.hdf5")
	message(FATAL_ERROR "no ${BASE_NAME}.mesh.hdf5 was written:\n${solver_output}")
endif ()

# Each .log row is "<iteration> <sim file> <zip file> <time>".
file(STRINGS "${WORK_DIR}/${BASE_NAME}.log" log_lines)
list(LENGTH log_lines timepoints)
if (NOT timepoints EQUAL EXPECTED_TIMEPOINTS)
	message(FATAL_ERROR
			"expected ${EXPECTED_TIMEPOINTS} timepoints in ${BASE_NAME}.log, got ${timepoints}:\n"
			"${log_lines}")
endif ()

set(sim_files "")
set(zip_files "")
foreach (line ${log_lines})
	string(REGEX REPLACE "^ *[0-9]+ +([^ ]+) +([^ ]+) +.*$" "\\1;\\2" fields "${line}")
	list(GET fields 0 sim_file)
	list(GET fields 1 zip_file)
	list(APPEND sim_files "${sim_file}")
	list(APPEND zip_files "${zip_file}")
endforeach ()
list(REMOVE_DUPLICATES zip_files)

# The .sim.hdf5 files are removed from disk once rolled into the archive, so the
# archive is the only place to look for them.
foreach (zip_file ${zip_files})
	if (NOT EXISTS "${WORK_DIR}/${zip_file}")
		message(FATAL_ERROR "${BASE_NAME}.log names ${zip_file}, which was never written")
	endif ()
	file(ARCHIVE_EXTRACT INPUT "${WORK_DIR}/${zip_file}" DESTINATION "${WORK_DIR}/extracted")
endforeach ()

foreach (sim_file ${sim_files})
	if (NOT EXISTS "${WORK_DIR}/extracted/${sim_file}")
		message(FATAL_ERROR "${sim_file} is missing from the zip archive")
	endif ()
	file(SIZE "${WORK_DIR}/extracted/${sim_file}" sim_size)
	if (sim_size EQUAL 0)
		message(FATAL_ERROR "${sim_file} was written empty")
	endif ()
endforeach ()

message(STATUS "${SOLVER}: ${timepoints} timepoints, ${BASE_NAME}.mesh.hdf5 and archive OK")

#############################################
#  Numerical regression, when a baseline is supplied
##############################################
if (DEFINED BASELINE AND DEFINED COMPARATOR)
	if (NOT EXISTS "${BASELINE}")
		message(FATAL_ERROR
				"baseline ${BASELINE} does not exist.\n"
				"Generate it with: ${COMPARATOR} --write <solution.hdf5> ${BASELINE}")
	endif ()

	# The last row of the .log names the final timepoint, which is the most
	# sensitive to a numerical change because error accumulates into it.
	list(GET sim_files -1 _final_sim)
	set(_final "${WORK_DIR}/extracted/${_final_sim}")

	if (NOT DEFINED RTOL)
		set(RTOL 1e-9)
	endif ()
	if (NOT DEFINED ATOL)
		set(ATOL 1e-12)
	endif ()

	execute_process(
			COMMAND "${COMPARATOR}" --check "${_final}" "${BASELINE}" "${RTOL}" "${ATOL}"
			OUTPUT_VARIABLE compare_output
			ERROR_VARIABLE compare_output
			RESULT_VARIABLE compare_result)
	message(STATUS "${compare_output}")

	if (NOT compare_result EQUAL 0)
		message(FATAL_ERROR
				"${_final_sim} does not match ${BASELINE}.\n"
				"If the change is intentional, regenerate with:\n"
				"  ${COMPARATOR} --write ${_final} ${BASELINE}\n"
				"and say in the commit message why the numbers moved.")
	endif ()
endif ()

#############################################
#  Validation against the analytic solution
##############################################
if (DEFINED ANALYTIC_EXPECTED AND DEFINED COMPARATOR)
	list(GET sim_files -1 _final_sim)
	set(_final "${WORK_DIR}/extracted/${_final_sim}")

	if (NOT DEFINED ANALYTIC_BAND)
		set(ANALYTIC_BAND 0.15)
	endif ()
	if (NOT DEFINED ANALYTIC_DATASET)
		set(ANALYTIC_DATASET "solution/U")
	endif ()

	execute_process(
			COMMAND "${COMPARATOR}" --analytic "${_final}" "${ANALYTIC_DATASET}"
					"${ANALYTIC_EXPECTED}" "${ANALYTIC_BAND}"
			OUTPUT_VARIABLE analytic_output
			ERROR_VARIABLE analytic_output
			RESULT_VARIABLE analytic_result)
	message(STATUS "${analytic_output}")

	if (NOT analytic_result EQUAL 0)
		message(FATAL_ERROR
				"${_final_sim} does not agree with the analytic solution.\n"
				"The expected value is what backward Euler should produce at this time step; "
				"see tests/README.md for the derivation. A change here means the solver is "
				"computing something different, not that the number needs updating.")
	endif ()
endif ()
