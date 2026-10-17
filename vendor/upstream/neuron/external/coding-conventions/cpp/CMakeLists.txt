list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    PARENT_SCOPE)
set(CMAKE_EXPORT_COMPILE_COMMANDS
    ON
    PARENT_SCOPE)
if(NOT CODING_CONV_PREFIX)
  set(CODING_CONV_PREFIX "${PROJECT_NAME}")
  set(CODING_CONV_PREFIX
      "${PROJECT_NAME}"
      PARENT_SCOPE)
endif(NOT CODING_CONV_PREFIX)
include(cmake/bob.cmake)
include(cmake/3rdparty.cmake)
include(cmake/build-time-copy.cmake)
include(cmake/bbp-find-python-module.cmake)

function(cpp_cc_enable_precommit)
  find_package(PythonInterp 3.5 REQUIRED)
  find_package(PreCommit REQUIRED)
  if(NOT EXISTS ${CMAKE_SOURCE_DIR}/.git/hooks/pre-commit)
    execute_process(COMMAND ${PreCommit_EXECUTABLE} install)
  endif()
  if(NOT EXISTS ${CMAKE_SOURCE_DIR}/.git/hooks/pre-push)
    execute_process(COMMAND ${PreCommit_EXECUTABLE} install --hook-type pre-push)
  endif()
  if(${CODING_CONV_PREFIX}_GIT_COMMIT_HOOKS MATCHES "courtesy-msg"
     OR ${CODING_CONV_PREFIX}_GIT_PUSH_HOOKS MATCHES "courtesy-msg")
    if(EXISTS ${PROJECT_SOURCE_DIR}/.git-push-message.cmake.in)
      configure_file(${PROJECT_SOURCE_DIR}/.git-push-message.cmake.in
                     ${PROJECT_BINARY_DIR}/git-push-message.cmake @ONLY)
    else()
      configure_file(cmake/git-push-message.cmake.in ${PROJECT_BINARY_DIR}/git-push-message.cmake
                     @ONLY)
    endif()
  endif()
  execute_process(
    COMMAND
      ${PYTHON_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/bbp-setup-pre-commit-config.py"
      --commit-checks=${${CODING_CONV_PREFIX}_GIT_COMMIT_HOOKS}
      --push-checks=${${CODING_CONV_PREFIX}_GIT_PUSH_HOOKS} ${PROJECT_SOURCE_DIR}
      ${PROJECT_BINARY_DIR})
  add_custom_target(git-pre-commits ${PreCommit_EXECUTABLE} run --all-files)
endfunction(cpp_cc_enable_precommit)

function(cpp_cc_disable_precommit)
  if(EXISTS ${PROJECT_SOURCE_DIR}/.pre-commit-config.yaml)
    find_package(PythonInterp 3.5 REQUIRED)
    execute_process(
      COMMAND
        ${PYTHON_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/bbp-setup-pre-commit-config.py"
        --commit-checks= --push-checks= ${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR})
  endif()
  file(REMOVE ${PROJECT_BINARY_DIR}/git-push-message.cmake)
endfunction(cpp_cc_disable_precommit)

function(cpp_cc_setup_tool_config name path)
  execute_process(COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/bbp-${name}.py
                          --executable ${path} config RESULT_VARIABLE ${name}_config)
  if(NOT ${name}_config EQUAL 0)
    message(SEND_ERROR "Could not set up ${name} configuration")
  endif()
endfunction()

function(cpp_cc_add_tool_target name)
  add_custom_target(
    ${name}-${PROJECT_NAME}
    ${ARGN}
    JOB_POOL console
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
  if(TARGET ${name})
    add_dependencies(${name} ${name}-${PROJECT_NAME})
  else()
    add_custom_target(
      ${name}
      DEPENDS ${name}-${PROJECT_NAME}
      JOB_POOL console)
  endif()
endfunction()

function(cpp_cc_enable_static_analysis)
  find_package(PythonInterp 3.5 REQUIRED)
  set(${CODING_CONV_PREFIX}_ClangTidy_DEPENDENCIES
      ""
      CACHE STRING "list of CMake targets to build before checking C/C++ code")
  mark_as_advanced(${CODING_CONV_PREFIX}_ClangTidy_DEPENDENCIES)
  set(clang_tidy_command ${PYTHON_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/../bin/clang-tidy" -v -p
                         ${PROJECT_BINARY_DIR}/compile_commands.json)
  cpp_cc_add_tool_target(clang-tidy ${clang_tidy_command})
  if(${CODING_CONV_PREFIX}_TEST_STATIC_ANALYSIS)
    add_test(NAME ClangTidy_${PROJECT_NAME} COMMAND ${clang_tidy_command})
  endif()
  if(${CODING_CONV_PREFIX}_ClangTidy_DEPENDENCIES)
    add_dependencies(clang-tidy_${PROJECT_NAME} ${${CODING_CONV_PREFIX}_ClangTidy_DEPENDENCIES})
  endif()
endfunction(cpp_cc_enable_static_analysis)

bob_option(${CODING_CONV_PREFIX}_TEST_FORMATTING "Add CTest formatting test" OFF)

bob_option(${CODING_CONV_PREFIX}_GIT_HOOKS
           "Enable automatic checks when committing and pushing changes" OFF)
bob_input(${CODING_CONV_PREFIX}_GIT_COMMIT_HOOKS "" STRING
          "Comma-separated list of checks to perform when committing changes")
bob_input(${CODING_CONV_PREFIX}_GIT_PUSH_HOOKS "courtesy-msg" STRING
          "Comma-separated list of checks to perform when pushing changes")

bob_option(${CODING_CONV_PREFIX}_STATIC_ANALYSIS "Enable C++ static analysis during compilation"
           OFF)
bob_option(${CODING_CONV_PREFIX}_TEST_STATIC_ANALYSIS "Add CTest static analysis test" OFF)

if(${CODING_CONV_PREFIX}_TEST_FORMATTING)
  add_test(
    NAME check_formatting_${PROJECT_NAME}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    COMMAND ${PYTHON_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/../bin/format" --dry-run -v)
endif()

if(${CODING_CONV_PREFIX}_GIT_HOOKS)
  cpp_cc_enable_precommit()
else()
  cpp_cc_disable_precommit()
endif()

if(${CODING_CONV_PREFIX}_STATIC_ANALYSIS)
  cmake_minimum_required(VERSION 3.6)
  cpp_cc_enable_static_analysis()
  set(CMAKE_CXX_CLANG_TIDY ${PYTHON_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/../bin/clang-tidy" -p
                           ${PROJECT_BINARY_DIR}/compile_commands.json)

endif(${CODING_CONV_PREFIX}_STATIC_ANALYSIS)
