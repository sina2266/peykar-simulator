# Optional messaging dependency
if(OPTION_TARGET_MESSAGING)
    find_package(CURL REQUIRED)
endif()

# 3.15+: CMP0091, so CMAKE_MSVC_RUNTIME_LIBRARY (the static CRT of the Windows release) applies here too
cmake_minimum_required(VERSION 3.15)
project(VCellStoch LANGUAGES CXX)

set(LIB_SRC_FILES
        src/Gibson.cpp
        src/IndexedTree.cpp
        src/Jump.cpp
        src/StochModel.cpp
        src/StochVar.cpp
        src/MultiTrialStats.cpp
)

set(HEADER_FILES
        include/Gibson.h
        include/IndexedTree.h
        include/Jump.h
        include/StochModel.h
        include/StochVar.h
        include/MultiTrialStats.h
)

set(SRC_MAIN_FILE src/VCellStoch.cpp)

add_library(VCellStochLib STATIC ${LIB_SRC_FILES} ${HEADER_FILES})

target_include_directories(VCellStochLib
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# HDF5 for multi-trial output
target_compile_definitions(VCellStochLib PUBLIC USE_HDF5=1)
target_link_libraries(VCellStochLib PUBLIC ${HDF5_LIBRARIES})
target_include_directories(VCellStochLib PUBLIC ${HDF5_INCLUDE_DIRS})

target_link_libraries(VCellStochLib
        PUBLIC
        vcellexpressionparser
        vcommons
        vcellmessaging
)

add_executable(VCellStoch ${SRC_MAIN_FILE})
target_link_libraries(VCellStoch PRIVATE VCellStochLib)

install(TARGETS VCellStochLib VCellStoch
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
)

install(FILES ${HEADER_FILES} DESTINATION include/VCellStoch)