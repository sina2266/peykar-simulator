
# Set the test name, 'test_' will be prepended to the
# name set here
set(CURRENT_TEST connection)
# Set a category name to enable running commands like:
#    ctest -R <category-label>
# which will run the tests matching this category-label.
# Can be left empty (or just not set)
set(${CURRENT_TEST}_CATEGORY entities)
list(APPEND LIBCELLML_TESTS ${CURRENT_TEST})
# Using absolute path relative to this file
set(${CURRENT_TEST}_SRCS
  ${CMAKE_CURRENT_LIST_DIR}/connection.cpp
)
set(${CURRENT_TEST}_HDRS
)
