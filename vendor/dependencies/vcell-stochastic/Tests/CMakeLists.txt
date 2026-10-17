project(VCellStochasticTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(TEST_SOURCES
    test_main.cpp
    statstest.cpp
    MultiTrialStatsTest.cpp
)

add_executable(TestVCellStoch ${TEST_SOURCES})
target_include_directories(TestVCellStoch PRIVATE
    ${CMAKE_SOURCE_DIR}/VCellStoch/include
    ${CMAKE_SOURCE_DIR}/vcell-expressionparser/include
    ${CMAKE_SOURCE_DIR}/vcommons/include
    ${CMAKE_SOURCE_DIR}/vcell-messaging/include
)

target_link_libraries(TestVCellStoch PRIVATE
    VCellStochLib
)

add_test(NAME TestVCellStoch COMMAND TestVCellStoch)