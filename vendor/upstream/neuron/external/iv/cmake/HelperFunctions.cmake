# =============================================================================
# Check if directory related to DIR exist by compiling code
# =============================================================================
function(iv_check_dir_exists HEADER VARIABLE)
  # code template to check existance of DIR
  set(CONFTEST_DIR_TPL "
    #include <sys/types.h>
    #include <@dir_header@>

    int main () {
      if ((DIR *) 0)
        return 0\;
      return 0\;
    }")
  # first get header file
  check_include_files(${HEADER} HAVE_HEADER)
  if(${HAVE_HEADER})
    # if header is found, create a code from template
    string(REPLACE "@dir_header@"
                   ${HEADER}
                   CONFTEST_DIR
                   "${CONFTEST_DIR_TPL}")
    file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/conftest.c ${CONFTEST_DIR})
    # try to compile
    try_compile(RESULT_VAR ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/conftest.c)
    set(${VARIABLE} ${RESULT_VAR} PARENT_SCOPE)
    file(REMOVE "conftest.c")
  endif()
endfunction()

# =============================================================================
# Check if given type exist by compiling code
# =============================================================================
function(iv_check_type_exists HEADER TYPE DEFAULT_TYPE VARIABLE)
  # code template to check existance of specific type
  set(CONFTEST_TYPE_TPL "
    #include <@header@>

    int main () {
      if (sizeof (@type@))
        return 0\;
      return 0\;
    }")
  string(REPLACE "@header@"
                 ${HEADER}
                 CONFTEST_TYPE
                 "${CONFTEST_TYPE_TPL}")
  string(REPLACE "@type@"
                 ${TYPE}
                 CONFTEST_TYPE
                 "${CONFTEST_TYPE}")
  file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/conftest.c ${CONFTEST_TYPE})

  try_compile(RESULT_VAR ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/conftest.c)
  if(NOT ${RESULT_VAR})
    set(${VARIABLE} ${DEFAULT_TYPE} PARENT_SCOPE)
  endif()
  file(REMOVE "conftest.c")
endfunction()
