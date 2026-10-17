cmake_minimum_required (VERSION 3.22)
project(VCellTest CXX)
set (CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG          v1.14.0
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

if (NOT WINDOWS)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()

set(SRC_FILES
        TestVCell.cpp
        TestSmoldyn.cpp
        testFiles/SmoldynComplexDataStructures/TestSmoldynCHashtable.cpp
        testFiles/SmoldynComplexDataStructures/TestSmoldynCLinkedList.cpp
        testFiles/SmoldynComplexDataStructures/TestSmoldynCVector.cpp
)

#####
# Build Testing File Locations Dynamically
## FV Solver File Locations
get_filename_component(EXAMPLE_FUNCTIONS testFiles/input/FVSolver/SimID_11538992_0_.functions ABSOLUTE)
get_filename_component(EXAMPLE_FV_INPUT testFiles/input/FVSolver/SimID_11538992_0_.fvinput ABSOLUTE)
get_filename_component(EXAMPLE_VCG testFiles/input/FVSolver/SimID_11538992_0_.vcg ABSOLUTE)
get_filename_component(EXAMPLE_SIM_TASK_XML testFiles/input/FVSolver/SimID_11538992_0__0.simtask.xml ABSOLUTE)
get_filename_component(EXAMPLE_OUTPUT_DIR_1 testFiles/output/FVSolver/test_output_1 ABSOLUTE)
get_filename_component(EXAMPLE_OUTPUT_DIR_2 testFiles/output/FVSolver/test_output_2 ABSOLUTE)
## Smoldyn File Locations
get_filename_component(EXAMPLE_SMOLDYN_XML testFiles/input/Smoldyn/input.xml ABSOLUTE)
get_filename_component(EXAMPLE_SMOLDYN_FUNCTIONS testFiles/input/Smoldyn/input.functions ABSOLUTE)
get_filename_component(EXAMPLE_SMOLDYN_MESH testFiles/input/Smoldyn/input.mesh ABSOLUTE)
get_filename_component(EXAMPLE_SMOLDYN_INPUT testFiles/input/Smoldyn/input.smoldynInput ABSOLUTE)
get_filename_component(EXAMPLE_SMOLDYN_SUBDOMAINS testFiles/input/Smoldyn/input.subdomains ABSOLUTE)
## Custom C Hashtable File Locations
get_filename_component(EXAMPLE_SAMPLE_HASH_TEST_INPUT testFiles/input/CustomCHashtableTest/sampleInput.txt ABSOLUTE)
## Dynamically build header for tests to use
configure_file(testFiles/input/testResourceLocations.h.in testFiles/input/testResourceLocations.h @ONLY)
#####

enable_testing()

add_executable(vcell_test ${SRC_FILES} )
target_include_directories(vcell_test PUBLIC "${CMAKE_CURRENT_BINARY_DIR}")
message("Included Directory: ${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries(vcell_test vcell GTest::gtest_main)
#target_link_libraries(TestVCell vcell GTest::gtest_main)
#install(TARGETS TestVCell
#        RUNTIME DESTINATION bin)

include(GoogleTest)
gtest_discover_tests(vcell_test)
# copy tests files to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/testFiles DESTINATION ${CMAKE_CURRENT_BINARY_DIR})