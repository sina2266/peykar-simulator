#############################################
#  GoogleTest
##############################################
include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG          v1.17.0
)
if (WIN32)
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()

FetchContent_MakeAvailable(googletest)
include(GoogleTest)

# Add test executables
add_executable(unit_tests
        unit/smoke_test.cpp
        unit/hello_test.cpp
        unit/message_processing_test.cpp
)
target_link_libraries(unit_tests PRIVATE
        IDAWin
        GTest::gtest_main
        spdlog::spdlog
        # if MinGW is enabled, add `$<$<BOOL:${MINGW}>:ws2_32>`
)
gtest_discover_tests(unit_tests)

target_compile_definitions(unit_tests PRIVATE RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/unit/resources")
if (OPTION_TEST_WITH_LOCALHOST)
    target_compile_definitions(unit_tests PRIVATE TEST_WITH_LOCALHOST=1)
else()
    target_compile_definitions(unit_tests PRIVATE TEST_WITH_LOCALHOST=0)
endif ()

#############################################
#  Python smoke test
#  Runs `tests/unit/smoke_test.py` via PYTHON_TEST_INTERPRETER. Registration
#  is skipped if `pyvcell_odesolver` is not importable from that interpreter,
#  so CI's native ctest run (system Python without the wheel) stays green.
#  cibuildwheel exercises the same file independently via pytest.
##############################################
find_package(Python3 COMPONENTS Interpreter QUIET)
set(PYTHON_TEST_INTERPRETER "${Python3_EXECUTABLE}" CACHE FILEPATH
    "Python interpreter for pytest-based tests (must have pyvcell_odesolver installed)")
if (PYTHON_TEST_INTERPRETER)
    execute_process(
        COMMAND ${PYTHON_TEST_INTERPRETER} -c "import pyvcell_odesolver"
        RESULT_VARIABLE _pyvcell_import_result
        OUTPUT_QUIET ERROR_QUIET
    )
    if (_pyvcell_import_result EQUAL 0)
        add_test(
            NAME python_smoke_test
            COMMAND ${PYTHON_TEST_INTERPRETER} -m pytest -v
                    ${CMAKE_CURRENT_SOURCE_DIR}/unit/smoke_test.py
        )
    else()
        message(STATUS
            "Skipping python_smoke_test: pyvcell_odesolver not importable from "
            "${PYTHON_TEST_INTERPRETER}. Install the wheel into a venv and "
            "rerun cmake with -DPYTHON_TEST_INTERPRETER=<venv>/bin/python.")
    endif()
endif()


#add_executable(integration_tests
#        integration/test_integration.cpp
#)
#target_link_libraries(integration_tests PRIVATE
#        IDAWin
#        GTest::gtest_main
#)
#gtest_discover_tests(integration_tests)


