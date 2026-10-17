set(_FAILED_FILES)
set(_FILES @FILES_TO_FORMAT@)

if(WIN32)
  # Windows uses CRLF at line endings but files are actually stored with
  # LF line ending, UNIX style. So we will ignore differences in line
  # endings on Windows.
  set(GIT_DIFF_OPTION_FOR_WINDOWS --ignore-space-at-eol)
endif()

foreach(file ${_FILES})
  execute_process(COMMAND "@CLANG_FORMAT_EXE@" "${file}"
    COMMAND "@GIT_EXE@" diff --no-index ${GIT_DIFF_OPTION_FOR_WINDOWS} -- "${file}" -
    RESULT_VARIABLE _RESULT
    OUTPUT_VARIABLE _STDOUT
    ERROR_VARIABLE _STDERR
  )

  if(NOT _RESULT EQUAL 0)
    list(APPEND _FAILED_FILES ${file})
  endif()
endforeach()

list(LENGTH _FILES _FILES_COUNT)
if(NOT _FAILED_FILES)
  message(STATUS "${_FILES_COUNT} files out of ${_FILES_COUNT} files are properly formatted.")
else()
  list(LENGTH _FAILED_FILES _FAILED_FILES_COUNT)
  if(_FAILED_FILES_COUNT EQUAL 1)
    message(STATUS "1 file out of ${_FILES_COUNT} files is not properly formatted:")
  else()
    message(STATUS "${_FAILED_FILES_COUNT} files out of ${_FILES_COUNT} files are not properly formatted:")
  endif()
  foreach(file ${_FAILED_FILES})
    string(REPLACE "@PROJECT_SOURCE_DIR@/" "" file "${file}")
    message(STATUS " - ${file}")
  endforeach()
  message(FATAL_ERROR "Please correct the formatting of the code by running the 'format_code' target.")
endif()
