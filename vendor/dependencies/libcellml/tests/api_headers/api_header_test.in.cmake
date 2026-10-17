
add_executable(${CURRENT_TEST} ${TEST_SRC})
target_link_libraries(${CURRENT_TEST} cellml gtest_main)

set_target_properties(${CURRENT_TEST} PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
  FOLDER tests)

add_test(NAME api_header_inclusion_${CURRENT_TEST} COMMAND ${CURRENT_TEST})

if(MSVC)
  # The libxml2 DLL and zlib DLL must be on the path otherwise the tests will not be able to run.
  set_tests_properties(api_header_inclusion_${CURRENT_TEST}
    PROPERTIES ENVIRONMENT "PATH=$<TARGET_FILE_DIR:cellml>\;$<TARGET_FILE_DIR:gtest_main>${GEN_EXP_XML2_TARGET_FILE_DIR}${GEN_EXP_ZLIB_TARGET_FILE_DIR}")
endif()

list(APPEND TEST_LIST ${CURRENT_TEST})
