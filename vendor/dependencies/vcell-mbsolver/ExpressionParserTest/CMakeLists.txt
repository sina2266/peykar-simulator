add_executable(ExpressionParserTest ExpressionParserTest.cpp)
target_link_libraries(ExpressionParserTest PRIVATE vcellexpressionparser)
target_include_directories(ExpressionParserTest PRIVATE ${CMAKE_SOURCE_DIR}/vcell-expressionparser/include)

# Register with CTest. The executable has no gtest harness and always exits 0,
# so use FAIL_REGULAR_EXPRESSION to catch the error messages it prints to stderr.
add_test(NAME ExpressionParserTest COMMAND ExpressionParserTest)
set_tests_properties(ExpressionParserTest PROPERTIES
    FAIL_REGULAR_EXPRESSION "^ExpressionParserTest failed: ((?!FunctionDomainException).)*$"
)

install(TARGETS ExpressionParserTest
    RUNTIME DESTINATION bin
)
