cmake_minimum_required(VERSION 3.13)
project(NFsim)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build settings
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Enable CXX and C
enable_language(CXX)
enable_language(C)

# Architecture flags
if (APPLE)
    # Build for the host architecture unless the caller asked for something else
    # (the release build passes -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" for a universal binary).
    if (NOT CMAKE_OSX_ARCHITECTURES)
        execute_process(COMMAND uname -m OUTPUT_VARIABLE PROCESSOR_ARCHITECTURE OUTPUT_STRIP_TRAILING_WHITESPACE)
        set(CMAKE_OSX_ARCHITECTURES "${PROCESSOR_ARCHITECTURE}")
    endif()
elseif (UNIX AND CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64|amd64")
    # Default to 64-bit on x86 Linux/Unix (-m64 is not an aarch64 option)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64")
endif()

# Source directories for NFsim
set(SUB_DIRS 
    src 
    src/nauty24 
    src/NFcore 
    src/NFcore/moleculeLists 
    src/NFcore/reactionSelector 
    src/NFfunction 
    src/NFfunction/muParser
    src/NFinput 
    src/NFinput/TinyXML
    src/NFoutput 
    src/NFreactions
    src/NFreactions/mappings
    src/NFreactions/reactantLists
    src/NFreactions/reactions
    src/NFreactions/transformations
    src/NFscheduler 
    src/NFutil 
    src/NFutil/MTrand
    src/NFtest
    src/NFtest/agentcell
    src/NFtest/agentcell/cell
    src/NFtest/simple_system
    src/NFtest/tlbr
    src/NFtest/transcription
)

# Use aux_source_directory for better control if needed, but file(GLOB) is also okay here
set(LIB_SRC_FILES)
foreach(DIR ${SUB_DIRS})
    file(GLOB FILES ${DIR}/*.cpp ${DIR}/*.c ${DIR}/*.hh ${DIR}/*.h)
    list(APPEND LIB_SRC_FILES ${FILES})
endforeach()

# Exclude Python binding file from the library (it's only for the Python module)
list(FILTER LIB_SRC_FILES EXCLUDE REGEX ".*pyvcell-nfsim-main\\.cpp$")

# Add a simple main if NFsim.cpp doesn't have one
set(MAIN_SRC ${CMAKE_BINARY_DIR}/nfsim_standalone_main.cpp)
file(WRITE ${MAIN_SRC} "
int NFsimMain(int argc, char *argv[]);
int main(int argc, char *argv[]) {
    return NFsimMain(argc, argv);
}
")

# Compile flags
if (UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
endif()

if (MSVC)
    add_definitions(-DHAVE_UNISTD_H=0)
endif()

# Define the library
add_library(NFsim_lib STATIC ${LIB_SRC_FILES})

# Enable position independent code for the library (required for Python binding)
set_target_properties(NFsim_lib PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Define the executable
add_executable(NFsim ${MAIN_SRC})
target_link_libraries(NFsim PRIVATE NFsim_lib)

# Include directories
foreach(DIR ${SUB_DIRS})
    target_include_directories(NFsim_lib PUBLIC ${DIR})
endforeach()

# -----------------------------------------------------------------------------
# NFsim_x64 — the executable VCell runs (desktop and HPC), see SOLVER-RELEASE.md.
# It is NFsim with VCell's main (VCell/src/VCellNFSim.cpp): progress markers on stdout
# ([[[progress:..%]]]) and, when built with OPTION_VCELL_MESSAGING=ON, status messages
# to VCell's broker when a trailing "-tid <n>" is given (the broker is read from the
# <jms> element of the -xml input). Messaging needs libcurl (HTTP only).
# -----------------------------------------------------------------------------
option(OPTION_VCELL_EXECUTABLE "Build NFsim_x64, the VCell-facing executable" ON)
option(OPTION_VCELL_MESSAGING "NFsim_x64 reports status to VCell's broker with -tid (requires libcurl)" OFF)
if (OPTION_VCELL_EXECUTABLE AND NOT SKBUILD)
    if (NOT DEFINED VCELL_NFSIM_VERSION OR VCELL_NFSIM_VERSION STREQUAL "")
        set(VCELL_NFSIM_VERSION "unknown")
    endif()
    set(GIT_DESCRIBE "${VCELL_NFSIM_VERSION}")
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/VCell/messaging/src/GitDescribe.cpp.in
                   ${CMAKE_CURRENT_BINARY_DIR}/GitDescribe.cpp @ONLY)

    add_library(vcell_nfsim_support STATIC
        VCell/messaging/src/SimulationMessaging.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/GitDescribe.cpp
        VCell/vcommons/src/tinyxml2.cpp
        VCell/vcommons/src/vcellxml.cpp)
    target_include_directories(vcell_nfsim_support PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/VCell/messaging/include
        ${CMAKE_CURRENT_SOURCE_DIR}/VCell/vcommons/include)
    if (WIN32)
        # windows.h (via SimulationMessaging.h) must not define min/max macros
        target_compile_definitions(vcell_nfsim_support PUBLIC NOMINMAX)
    endif()
    if (OPTION_VCELL_MESSAGING)
        find_package(CURL REQUIRED)
        find_package(Threads REQUIRED)
        target_compile_definitions(vcell_nfsim_support PUBLIC USE_MESSAGING=1)
        target_link_libraries(vcell_nfsim_support PUBLIC CURL::libcurl Threads::Threads)
    endif()

    add_executable(NFsim_x64 VCell/src/VCellNFSim.cpp VCell/src/NFMonitor.cpp)
    target_link_libraries(NFsim_x64 PRIVATE NFsim_lib vcell_nfsim_support)
endif()

# --- Windows compatibility for BioNetGen ---
# BioNetGen's findExec() looks in the repo's top-level "bin/" directory and,
# on Windows, tries "NFsim_MSWin32.exe". Copy our built NFsim there after build.
if(WIN32)
    add_custom_command(TARGET NFsim POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/bin"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:NFsim>"
            "${CMAKE_SOURCE_DIR}/bin/NFsim_MSWin32.exe"
            COMMENT "Copying NFsim to repo bin/ as NFsim_MSWin32.exe for BioNetGen"
    )
endif()

# Optional: Add smoke tests if they exist
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests/smoke/smoke.py)
    enable_testing()
    set(python_cmd python3)
    set(test_dir ${CMAKE_CURRENT_SOURCE_DIR}/tests/smoke)

    # Use the actual built executable path. On Windows this includes the .exe suffix.
    if(WIN32)
        set(_nfsim_smoke_exe $<TARGET_FILE:NFsim>)
    else()
        set(_nfsim_smoke_exe ${CMAKE_BINARY_DIR}/bin/NFsim)
    endif()

    add_test(
            NAME NFsim_smoke_python
            COMMAND ${python_cmd} ${test_dir}/smoke.py "${_nfsim_smoke_exe}"
            WORKING_DIRECTORY ${test_dir}
    )
endif()

# Architecture flags for macOS symlink
if (APPLE)
    # Create symbolic link from the built NFsim executable to NFsim_i386_darwin in the bin directory
    add_custom_command(TARGET NFsim POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink $<TARGET_FILE_NAME:NFsim> NFsim_i386_darwin
        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        COMMENT "Creating symbolic link to NFsim_i386_darwin"
    )
endif()

# -----------------------------------------------------------------------------
# Python bindings (optional, enabled when called via scikit-build-core)
# -----------------------------------------------------------------------------
if(SKBUILD)
    # Find pybind11
    find_package(pybind11 CONFIG REQUIRED)

    # Create Python module
    pybind11_add_module(_core src/pyvcell-nfsim-main.cpp)

    # Link against NFsim library
    target_link_libraries(_core PRIVATE NFsim_lib)

    # Include directories
    foreach(DIR ${SUB_DIRS})
        target_include_directories(_core PRIVATE ${DIR})
    endforeach()

    # Install the module
    install(TARGETS _core DESTINATION pyvcell_nfsim)
endif()

# -----------------------------------------------------------------------------
# Model tests (generated from models/* READMEs)
# Each test runs BioNetGen (BNG2.pl in repo root) to generate XML, then runs NFsim.
# We set the working directory to each model's folder so relative paths in RNF files work.
# -----------------------------------------------------------------------------
enable_testing()

# Paths to tools
set(BNG2_PL ${CMAKE_SOURCE_DIR}/BNG2.pl)
# NFsim executable for tests:
# - Windows: always use the real built target path (includes .exe, config folder)
# - Others: keep the previous bin/NFsim layout
if(WIN32)
    set(NFSIM_EXE $<TARGET_FILE:NFsim>)
else()
    set(NFSIM_EXE ${CMAKE_BINARY_DIR}/bin/NFsim)
endif()

# Use a deterministic Perl (avoid accidentally picking up Cygwin via PATH)
find_package(Perl REQUIRED)
message(STATUS "Using Perl: ${PERL_EXECUTABLE}")

if(WIN32)
    string(TOLOWER "${PERL_EXECUTABLE}" _perl_exe_lc)
    if(_perl_exe_lc MATCHES "cygwin" OR _perl_exe_lc MATCHES "cygdrive")
        message(FATAL_ERROR
                "Cygwin Perl detected (${PERL_EXECUTABLE}). "
                "Please put Strawberry Perl earlier in PATH or set PERL_EXECUTABLE explicitly "
                "(e.g., -DPERL_EXECUTABLE=C:/Strawberry/perl/bin/perl.exe), then reconfigure from a clean build directory.")
    endif()
endif()

# Helper macro to create a test that first generates XML then runs NFsim
# Usage: add_nfsim_model_test(TestName ModelDir BnglFile [RNF|XML] ArgFile [ExtraArgs...])
macro(add_nfsim_model_test TEST_NAME MODEL_DIR BNGL_FILE MODE ARG_FILE)
    if("${MODE}" STREQUAL "RNF")
        add_test(
            NAME ${TEST_NAME}
            COMMAND "${PERL_EXECUTABLE}" "${BNG2_PL}" "${BNGL_FILE}"
        )
        add_test(
            NAME ${TEST_NAME}_run
            COMMAND "${NFSIM_EXE}" -rnf "${ARG_FILE}"
        )
    else()
        # MODE == XML: run NFsim directly on the generated XML
        add_test(
            NAME ${TEST_NAME}
            COMMAND "${PERL_EXECUTABLE}" "${BNG2_PL}" "${BNGL_FILE}"
        )
        add_test(
            NAME ${TEST_NAME}_run
            COMMAND "${NFSIM_EXE}" -xml "${ARG_FILE}"
        )
    endif()
    set_tests_properties(${TEST_NAME} ${TEST_NAME}_run PROPERTIES WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/${MODEL_DIR})
    set_tests_properties(${TEST_NAME}_run PROPERTIES DEPENDS ${TEST_NAME})
endmacro()


# actin: use actin_simple + RNF
add_nfsim_model_test(Model_actin_simple models/actin actin_simple.bngl RNF run_actin_simple.rnf)

# tlbr: use tlbr.bngl + tlbr.rnf
add_nfsim_model_test(Model_tlbr models/tlbr tlbr.bngl RNF tlbr.rnf)

# chemotaxisAdaptation: AN.bngl + run_AN.rnf
add_nfsim_model_test(Model_chemotaxis_AN models/chemotaxisAdaptation AN.bngl RNF run_AN.rnf)

# multisite_phos: e1.bngl then run via -xml e1.xml
add_nfsim_model_test(Model_multisite_e1 models/multisite_phos e1.bngl XML e1.xml)

# flagellar_motor: motor.bngl run via -xml motor.xml
add_nfsim_model_test(Model_flagellar_motor models/flagellar_motor motor.bngl XML motor.xml)

# oscillatory_system: oscSystem.bngl -> oscSystem.xml
add_nfsim_model_test(Model_oscillatory_system models/oscillatory_system oscSystem.bngl XML oscSystem.xml)

# simple_PushPull: PushPull.bngl -> PushPull.xml
add_nfsim_model_test(Model_simple_PushPull models/simple_PushPull PushPull.bngl XML PushPull.xml)

# fceRI_compendium: choose a representative small model (fceri_fyn.bngl)
add_nfsim_model_test(Model_fceRI_fyn models/fceRI_compendium fceri_fyn.bngl XML fceri_fyn.xml)

# performance_test_models: push_pull example with modest steps to limit runtime
# Note: Keeping README defaults (100 steps/seconds). Adjust if CI timeouts occur.
add_test(
    NAME Model_performance_egfr_gen
    COMMAND "${PERL_EXECUTABLE}" "${BNG2_PL}" egfr_net.bngl
)
add_test(
    NAME Model_performance_egfr
    COMMAND "${NFSIM_EXE}" -xml egfr_net.xml -oSteps 100 -sim 100 -notf -utl 2
)
set_tests_properties(Model_performance_egfr_gen Model_performance_egfr PROPERTIES WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/models/performance_test_models)
set_tests_properties(Model_performance_egfr PROPERTIES DEPENDS Model_performance_egfr_gen)
