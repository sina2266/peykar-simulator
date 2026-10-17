cmake_minimum_required(VERSION 3.13)

# Google Test: prefer system installation, fall back to FetchContent
find_package(GTest QUIET)
if(NOT GTest_FOUND)
    message(STATUS "GTest not found via find_package; fetching with FetchContent")
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.17.0
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

set(SRC_FILES
    algotest.cpp
    alloctest.cpp
    biotest.cpp
    boosttest.cpp
    chronotest.cpp
    clippertest.cpp
    compiletest.cpp
    distancetest.cpp
    example_2d.cpp
    exptest.cpp
    fronttest.cpp
    ftest.cpp
    hdf5test.cpp
    insidetest.cpp
    logtest.cpp
    loopiteratortest.cpp
    meshelementnodetest.cpp
    movingboundarysetuptest.cpp
    numerictest.cpp
    segmenttest.cpp
    slopetest.cpp
    v64.cpp
    vcellpersist.cpp
    vcellutil.cpp
    vdemo.cpp
    vectortest.cpp
    vi64.cpp
    voltest.cpp
    worldtest.cpp
)

add_executable(TestMovingBoundary ${SRC_FILES})

target_link_libraries(TestMovingBoundary
    PRIVATE
        MovingBoundaryLib
        GTest::gtest_main
        ${HDF5_LIBRARIES}
        ${HDF5_HL_LIBRARIES}
)

include(GoogleTest)
gtest_discover_tests(TestMovingBoundary)

install(TARGETS TestMovingBoundary
    RUNTIME DESTINATION bin
)
