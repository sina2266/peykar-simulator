set(CURRENT_TEST generator)
set(${CURRENT_TEST}_CATEGORY io)

list(APPEND LIBCELLML_TESTS ${CURRENT_TEST})

set(${CURRENT_TEST}_SRCS
  ${CMAKE_CURRENT_LIST_DIR}/generator.cpp
  ${CMAKE_CURRENT_LIST_DIR}/generatorprofile.cpp
  ${CMAKE_CURRENT_LIST_DIR}/generatorvariabletracker.cpp
)
