project(vcellexpressionparser LANGUAGES CXX)

file(GLOB HEADER_FILES include/*.h)
file(GLOB SRC_FILES src/*.cpp)

list(REMOVE_ITEM HEADER_FILES include/ExpressionTest.h)
list(REMOVE_ITEM SRC_FILES src/ExpressionTest.cpp)

add_library(vcellexpressionparser STATIC ${SRC_FILES} ${HEADER_FILES})

# std::format, used by the AST nodes for error messages
target_compile_features(vcellexpressionparser PUBLIC cxx_std_20)

target_include_directories(vcellexpressionparser
		PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
)

install(TARGETS vcellexpressionparser
		ARCHIVE DESTINATION lib
)