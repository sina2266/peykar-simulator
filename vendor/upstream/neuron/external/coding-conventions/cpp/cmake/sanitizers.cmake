set(${CODING_CONV_PREFIX}_SANITIZERS
    ""
    CACHE
      STRING
      "Comma-separated list of runtime sanitizers to enable. Possible values: address, leak, undefined"
)
set(${CODING_CONV_PREFIX}_SANITIZERS_UNDEFINED_EXCLUSIONS
    ""
    CACHE
      STRING
      "Undefined behaviour sanitizer checks **not** to enable if ${CODING_CONV_PREFIX}_SANITIZERS contains 'undefined'"
)

# Find the path of a sanitizer runtime library. This is mainly intended for internal use.
#
# cpp_cc_find_sanitizer_runtime(NAME [<name>] OUTPUT [<output variable>])
function(cpp_cc_find_sanitizer_runtime)
  cmake_parse_arguments("" "" "NAME;OUTPUT" "" ${ARGN})
  if(APPLE AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    list(APPEND name_templates clang_rt.${_NAME}_osx_dynamic)
  elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    list(APPEND name_templates ${_NAME})
  else()
    # Different flavours of Linux / Clang disagree about whether we want the
    # -${CMAKE_SYSTEM_PROCESSOR} suffix
    list(APPEND name_templates clang_rt.${_NAME}-${CMAKE_SYSTEM_PROCESSOR})
    list(APPEND name_templates clang_rt.${_NAME})
  endif()
  foreach(name_template ${name_templates})
    set(name_template ${CMAKE_SHARED_LIBRARY_PREFIX}${name_template}${CMAKE_SHARED_LIBRARY_SUFFIX})
    execute_process(
      COMMAND ${CMAKE_CXX_COMPILER} -print-file-name=${name_template}
      RESULT_VARIABLE compiler_status
      OUTPUT_VARIABLE runtime_library
      ERROR_VARIABLE compiler_stderr
      OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_STRIP_TRAILING_WHITESPACE)
    if(${compiler_status})
      # This is rarely triggered, because -print-file-name=garbage just returns "garbage" and status
      # code zero, but it's good to check
      message(
        FATAL_ERROR
          "Failed to find ${_NAME} runtime library (stdout: ${runtime_library}, stderr: ${compiler_stderr})"
      )
    endif()
    if(NOT "${runtime_library}" STREQUAL "${name_template}")
      # See above; clang -print-file-name=NAME returns NAME if it isn't found. It it returns
      # something else, we can break() and be happy.
      break()
    endif()
  endforeach()
  message(STATUS "Sanitizer runtime library: ${runtime_library}")
  if(NOT IS_ABSOLUTE "${runtime_library}" OR NOT EXISTS "${runtime_library}")
    message(
      WARNING
        "Could not find an absolute path to the ${_NAME} runtime library, trying ${runtime_library}. "
        "This may require you to set LD_LIBRARY_PATH or DYLD_LIBRARY_PATH appropriately.")
  endif()
  set(${_OUTPUT}
      "${runtime_library}"
      PARENT_SCOPE)
endfunction()

# Assemble compiler flags, environment variables and command prefixes needed to enable runtime
# sanitizers (address, leak, undefined behaviour, thread, memory...)
#
# cpp_cc_enable_sanitizers()
#
# Sets:
#
# * ${CODING_CONV_PREFIX}_SANITIZER_COMPILER_FLAGS: compiler flags that should be passed to the
#   compiler and linker.
# * ${CODING_CONV_PREFIX}_SANITIZER_ENABLE_ENVIRONMENT: environment variables that should be set to
#   **enable** sanitizers at runtime.
# * ${CODING_CONV_PREFIX}_SANITIZER_DISABLE_ENVIRONMENT: environment variables that should be set to
#   **disable** sanitizers at runtime. This might be useful if, for example, some part of the
#   instrumented application is used during the build and you don't want memory leaks to cause build
#   failures.
# * ${CODING_CONV_PREFIX}_SANITIZER_PRELOAD_VAR: the environment variable used to load the sanitizer
#   runtime library. This is typically LD_PRELOAD or DYLD_INSERT_LIBRARIES.
# * ${CODING_CONV_PREFIX}_SANITIZER_LIBRARY_PATH: the sanitizer runtime library. This sometimes
#   needs to be added to ${CODING_CONV_PREFIX}_SANITIZER_PRELOAD_VAR.
# * ${CODING_CONV_PREFIX}_SANITIZER_LIBRARY_DIR: the directory where the sanitizer runtime library
#   sits. This is provided separately from the ENVIRONMENT variables to avoid assumptions about the
#   sanitizers being the only thing modifying LD_LIBRARY_PATH
#
# The caller is responsible for using these variables in a manner adapted to their application.
function(cpp_cc_enable_sanitizers)
  message(STATUS "Enabling sanitizers: ${${CODING_CONV_PREFIX}_SANITIZERS}")
  # comma-separated string -> CMake list
  string(REPLACE "," ";" sanitizers "${${CODING_CONV_PREFIX}_SANITIZERS}")
  if(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    set(known_undefined_checks undefined)
  else()
    set(known_undefined_checks
        undefined
        float-divide-by-zero
        unsigned-integer-overflow
        implicit-integer-sign-change
        implicit-signed-integer-truncation
        implicit-unsigned-integer-truncation
        local-bounds
        nullability-arg
        nullability-assign
        nullability-return)
  endif()
  set(compiler_flags -fno-omit-frame-pointer)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # Clang uses static sanitizer runtimes by default, but we want the shared library versions so
    # that we can LD_PRELOAD them when launching via Python and so on.
    list(APPEND compiler_flags -shared-libsan)
  endif()
  if("undefined" IN_LIST sanitizers)
    if(NOT sanitizers STREQUAL "undefined")
      message(
        FATAL_ERROR
          "Enabling the undefined behaviour sanitizer at the same time as other sanitizers is not currently supported (got: ${${CODING_CONV_PREFIX}_SANITIZERS})"
      )
    endif()
    # Enable the undefined behaviour sanitizer
    set(undefined_checks ${known_undefined_checks})
    if(${CODING_CONV_PREFIX}_SANITIZERS_UNDEFINED_EXCLUSIONS)
      message(
        STATUS "Disabling UBSan checks: ${${CODING_CONV_PREFIX}_SANITIZERS_UNDEFINED_EXCLUSIONS}")
      list(REMOVE_ITEM undefined_checks ${${CODING_CONV_PREFIX}_SANITIZERS_UNDEFINED_EXCLUSIONS})
    endif()
    foreach(undefined_check ${undefined_checks})
      list(APPEND compiler_flags -fsanitize=${undefined_check})
    endforeach()
    # If we were asked to disable checks that are not listed in known_undefined_checks then emit
    # -fno-sanitize=XXX for them
    list(REMOVE_ITEM ${CODING_CONV_PREFIX}_SANITIZERS_UNDEFINED_EXCLUSIONS
         ${known_undefined_checks})
    foreach(undefined_check ${${CODING_CONV_PREFIX}_SANITIZERS_UNDEFINED_EXCLUSIONS})
      list(APPEND compiler_flags -fno-sanitize=${undefined_check})
    endforeach()
    string(JOIN " " compiler_flags_str ${compiler_flags})
    # Figure out where the runtime library lives
    if(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
      set(runtime_library_name ubsan)
    else()
      set(runtime_library_name ubsan_standalone)
    endif()
    cpp_cc_find_sanitizer_runtime(NAME ${runtime_library_name} OUTPUT runtime_library)
    if(EXISTS "${PROJECT_SOURCE_DIR}/.sanitizers/undefined.supp")
      set(ubsan_opts "suppressions=${PROJECT_SOURCE_DIR}/.sanitizers/undefined.supp:")
    endif()
    if(LLVM_SYMBOLIZER_PATH)
      set(extra_env "UBSAN_SYMBOLIZER_PATH=${LLVM_SYMBOLIZER_PATH}")
    endif()
    set(enable_env
        ${extra_env}
        "UBSAN_OPTIONS=${ubsan_opts}print_stacktrace=1:halt_on_error=1:report_error_type=1")
    set(disable_env ${extra_env} "UBSAN_OPTIONS=${ubsan_opts}print_stacktrace=0:halt_on_error=0")
  elseif("address" IN_LIST sanitizers)
    list(APPEND compiler_flags -fsanitize=address -fsanitize-address-use-after-scope)
    # Figure out where the runtime library lives
    cpp_cc_find_sanitizer_runtime(NAME asan OUTPUT runtime_library)
    if(APPLE)
      # Suppress and annoying "malloc: nano zone abandoned due to inability to preallocate reserved
      # vm space" message. See https://stackoverflow.com/a/70209891
      list(APPEND extra_env MallocNanoZone=0)
    endif()
    if(LLVM_SYMBOLIZER_PATH)
      list(APPEND extra_env "ASAN_SYMBOLIZER_PATH=${LLVM_SYMBOLIZER_PATH}")
      if("leak" IN_LIST sanitizers)
        list(APPEND extra_env "LSAN_SYMBOLIZER_PATH=${LLVM_SYMBOLIZER_PATH}")
      endif()
    endif()
    set(enable_env ${extra_env}
                   ASAN_OPTIONS=check_initialization_order=1:detect_stack_use_after_return=1)
    if("leak" IN_LIST sanitizers)
      string(APPEND enable_env ":detect_leaks=1")
      list(APPEND enable_env PYTHONMALLOC=malloc)
      if(EXISTS "${PROJECT_SOURCE_DIR}/.sanitizers/leak.supp")
        list(APPEND enable_env
             "LSAN_OPTIONS=suppressions=${PROJECT_SOURCE_DIR}/.sanitizers/leak.supp")
      endif()
    else()
      string(APPEND enable_env ":detect_leaks=0")
    endif()
    set(disable_env ${extra_env} ASAN_OPTIONS=detect_leaks=0)
  elseif("leak" IN_LIST sanitizers)
    message(FATAL_ERROR "LSan not yet supported in standalone mode")
  elseif("thread" IN_LIST sanitizers)
    if(NOT sanitizers STREQUAL "thread")
      message(
        FATAL_ERROR
          "Enabling the thread sanitizer at the same time as other sanitizers is not currently supported (got: ${${CODING_CONV_PREFIX}_SANITIZERS})"
      )
    endif()
    list(APPEND compiler_flags -fsanitize=thread)
    cpp_cc_find_sanitizer_runtime(NAME tsan OUTPUT runtime_library)
    if(LLVM_SYMBOLIZER_PATH)
      set(extra_env "TSAN_SYMBOLIZER_PATH=${LLVM_SYMBOLIZER_PATH}")
    endif()
    if(EXISTS "${PROJECT_SOURCE_DIR}/.sanitizers/thread.supp")
      set(tsan_opts "suppressions=${PROJECT_SOURCE_DIR}/.sanitizers/thread.supp:")
    endif()
    set(enable_env ${extra_env} TSAN_OPTIONS=${tsan_opts}halt_on_error=1)
    set(disable_env ${extra_env} TSAN_OPTIONS=${tsan_opts}report_bugs=0)
  else()
    message(FATAL_ERROR "${sanitizers} sanitizers not yet supported")
  endif()
  if(APPLE)
    set(preload_var DYLD_INSERT_LIBRARIES)
  else()
    set(preload_var LD_PRELOAD)
  endif()
  get_filename_component(runtime_library_directory "${runtime_library}" DIRECTORY)
  set(${CODING_CONV_PREFIX}_SANITIZER_COMPILER_FLAGS
      "${compiler_flags}"
      PARENT_SCOPE)
  set(${CODING_CONV_PREFIX}_SANITIZER_ENABLE_ENVIRONMENT
      "${enable_env}"
      PARENT_SCOPE)
  set(${CODING_CONV_PREFIX}_SANITIZER_DISABLE_ENVIRONMENT
      "${disable_env}"
      PARENT_SCOPE)
  set(${CODING_CONV_PREFIX}_SANITIZER_LIBRARY_DIR
      "${runtime_library_directory}"
      PARENT_SCOPE)
  set(${CODING_CONV_PREFIX}_SANITIZER_LIBRARY_PATH
      "${runtime_library}"
      PARENT_SCOPE)
  set(${CODING_CONV_PREFIX}_SANITIZER_PRELOAD_VAR
      "${preload_var}"
      PARENT_SCOPE)
endfunction(cpp_cc_enable_sanitizers)

# Helper function that modifies targets (executables, libraries, ...) and tests (created by
# add_test) for successful execution when sanitizers are enabled
#
# cpp_cc_configure_sanitizers(TARGET [<target1> ...] TEST [<test1> ...] [PRELOAD])
#
# Arguments:
#
# * TARGET: list of targets to modify
# * TEST: list of tests to modify
# * PRELOAD: if passed, ${CODING_CONV_PREFIX}_SANITIZER_PRELOAD_VAR will be set to the sanitizer
#   runtime library and LD_LIBRARY_PATH will not be modified
function(cpp_cc_configure_sanitizers)
  cmake_parse_arguments("" "PRELOAD" "" "TARGET;TEST" ${ARGN})
  foreach(target ${_TARGET})
    # Make sure that the RPATH to the sanitizer runtime is set, so the library/executable can be run
    # without setting $LD_LIBRARY_PATH
    set_property(
      TARGET ${target}
      APPEND
      PROPERTY BUILD_RPATH "${${CODING_CONV_PREFIX}_SANITIZER_LIBRARY_DIR}")
    set_property(
      TARGET ${target}
      APPEND
      PROPERTY INSTALL_RPATH "${${CODING_CONV_PREFIX}_SANITIZER_LIBRARY_DIR}")
  endforeach()
  foreach(test ${_TEST})
    if(_PRELOAD AND ${CODING_CONV_PREFIX}_SANITIZER_LIBRARY_PATH)
      set_property(
        TEST ${test}
        APPEND
        PROPERTY
          ENVIRONMENT
          ${${CODING_CONV_PREFIX}_SANITIZER_PRELOAD_VAR}=${${CODING_CONV_PREFIX}_SANITIZER_LIBRARY_PATH}
      )
    endif()
    # This should be sanitizer-specific stuff like UBSAN_OPTIONS, so we don't need to worry about
    # merging it with an existing value.
    set_property(
      TEST ${test}
      APPEND
      PROPERTY ENVIRONMENT ${${CODING_CONV_PREFIX}_SANITIZER_ENABLE_ENVIRONMENT})
  endforeach()
endfunction(cpp_cc_configure_sanitizers)

# Helper function strips away Python shims on macOS, so we can launch tests using the actual Python
# binary. Without this, preloading the sanitizer runtimes does not work on macOS. See
# https://jonasdevlieghere.com/sanitizing-python-modules/ and
# https://tobywf.com/2021/02/python-ext-asan/ for more information.
#
# cpp_cc_strip_python_shims(EXECUTABLE <executable> OUTPUT <output_variable>)
#
# Arguments:
#
# * EXECUTABLE: the Python executable/shim to try and resolve
# * OUTPUT: output variable for the actual Python executable
function(cpp_cc_strip_python_shims)
  cmake_parse_arguments("" "" "EXECUTABLE;OUTPUT" "" ${ARGN})
  if(APPLE AND ${CODING_CONV_PREFIX}_SANITIZERS)
    set(python_script
        "import ctypes"
        "dyld = ctypes.cdll.LoadLibrary('/usr/lib/system/libdyld.dylib')"
        "namelen = ctypes.c_ulong(1024)"
        "name = ctypes.create_string_buffer(b'\\000', namelen.value)"
        "dyld._NSGetExecutablePath(ctypes.byref(name), ctypes.byref(namelen))"
        "print(name.value.decode())")
    string(JOIN "; " python_command ${python_script})
    execute_process(
      COMMAND ${_EXECUTABLE} -c "${python_command}"
      RESULT_VARIABLE python_status
      OUTPUT_VARIABLE actual_executable
      ERROR_VARIABLE python_stderr
      OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_STRIP_TRAILING_WHITESPACE)
    if(NOT python_status EQUAL 0)
      message(FATAL_ERROR "stdout: ${actual_executable}, stderr: ${python_stderr}")
    endif()
    if(NOT _EXECUTABLE STREQUAL actual_executable)
      message(STATUS "Resolved shim ${_EXECUTABLE} to ${actual_executable}")
    endif()
  else()
    set(actual_executable "${_EXECUTABLE}")
  endif()
  set(${_OUTPUT}
      "${actual_executable}"
      PARENT_SCOPE)
endfunction()

if(${CODING_CONV_PREFIX}_SANITIZERS)
  cpp_cc_enable_sanitizers()
endif()
