
# Set the test name, 'test_' will be prepended to the
# name set here
set(CURRENT_TEST parser)
# Set a category name to enable running commands like:
#    ctest -R <category-label>
# which will run the tests matching this category-label.
# Can be left empty (or just not set)
set(${CURRENT_TEST}_CATEGORY io)
list(APPEND LIBCELLML_TESTS ${CURRENT_TEST})
# Using absolute path relative to this file
set(${CURRENT_TEST}_SRCS
  ${CMAKE_CURRENT_LIST_DIR}/cellml_1_0.cpp
  ${CMAKE_CURRENT_LIST_DIR}/cellml_1_1.cpp
  ${CMAKE_CURRENT_LIST_DIR}/file_parser.cpp
  ${CMAKE_CURRENT_LIST_DIR}/libxml_user.cpp
  ${CMAKE_CURRENT_LIST_DIR}/parser.cpp
)
#set(${CURRENT_TEST}_HDRS
#  ${CMAKE_CURRENT_LIST_DIR}/<test_header_files.h>
#)

set_source_files_properties(${CMAKE_CURRENT_LIST_DIR}/libxml_user.cpp PROPERTIES
  COMPILE_DEFINITIONS XML_ERROR_CALLBACK_ARGUMENT_TYPE=${CONST_ERROR_STRUCTURED_ERROR_CALLBACK_TYPE})
