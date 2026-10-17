set(CURRENT_TEST analyser)
set(${CURRENT_TEST}_CATEGORY utils)

list(APPEND LIBCELLML_TESTS ${CURRENT_TEST})

set(${CURRENT_TEST}_SRCS
  ${CMAKE_CURRENT_LIST_DIR}/analyser.cpp
  ${CMAKE_CURRENT_LIST_DIR}/analyserexternalvariable.cpp
  ${CMAKE_CURRENT_LIST_DIR}/analyserunits.cpp
)
