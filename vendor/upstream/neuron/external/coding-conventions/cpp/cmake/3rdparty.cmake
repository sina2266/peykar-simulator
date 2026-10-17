find_package(Git QUIET)

if(NOT ${CODING_CONV_PREFIX}_3RDPARTY_DIR)
  set(${CODING_CONV_PREFIX}_3RDPARTY_DIR 3rdparty)
  set(${CODING_CONV_PREFIX}_3RDPARTY_DIR
      3rdparty
      PARENT_SCOPE)
endif()

# initialize submodule with given path
#
# cpp_cc_init_git_submodule(path GIT_ARGS [<arguments>])
#
# Default options passed to the `git submodule update` command are `--init --recursive --depth 1` to
# perform a shallow clone of the submodule. If the GIT_ARGS argument is provided, then its value
# supersedes the default options.
#
function(cpp_cc_init_git_submodule path)
  cmake_parse_arguments(PARSE_ARGV 1 opt "" "" "GIT_ARGS")
  if(NOT opt_GIT_ARGS)
    set(opt_GIT_ARGS --init --recursive)
    # RHEL7-family distributions ship with an old git that does not support the --depth argument to
    # git submodule update
    if(GIT_VERSION_STRING VERSION_GREATER_EQUAL "1.8.4")
      list(APPEND opt_GIT_ARGS --depth 1)
    endif()
  endif()
  if(NOT ${GIT_FOUND})
    message(
      FATAL_ERROR "git not found and ${path} submodule not cloned (use git clone --recursive)")
  endif()
  message(
    STATUS "Fetching git submodule ${path}: running git submodule update ${opt_GIT_ARGS} -- ${path}"
  )
  execute_process(
    COMMAND ${GIT_EXECUTABLE} submodule update ${opt_GIT_ARGS} -- ${path}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    RESULT_VARIABLE git_submodule_status)
  if(NOT git_submodule_status EQUAL 0)
    message(FATAL_ERROR "Could not clone git submodule ${path}")
  endif()
endfunction()

# use a git submodule
#
# cpp_cc_git_submodule(source_dir [DISABLED] [QUIET] SUBDIR path [BUILD] [<arguments>] [PACKAGE]
# [<arguments>] GIT_ARGS [<arguments>])
#
# Add a CMake option in the cache to control whether the submodule is used or not (default ON). The
# option is named after the source directory passed in first argument, for instance:
# cpp_cc_git_submodule(src/eigen) adds the following CMake cached option:
# ${PROJECT_NAME}_3RDPARTY_USE_SRC_EIGEN:BOOL=ON
#
# If enabled, then the submodule is fetched if missing in the working copy.
#
# If the DISABLED argument is provided, then the default value for the CMake option is OFF.
#
# If the QUIET argument is provided then no status message will be printed.
#
# If the BUILD argument is provided then the directory is added to the build through the
# add_subdirectory CMake function. Arguments following the BUILD arguments are passed to the
# add_subdirectory function call.
#
# The optional SUBDIR argument is used by the BUILD argument to determine the path to the directory
# added to the build. The path specified is relative to the path to the git submodule.
#
# If the PACKAGE argument is provided and the CMake option to determine whether the git submodule
# should be used or not is FALSE, then a call to the find_package function is made with the
# arguments specified to the PACKAGE option.
#
# Default options passed to the `git submodule update` command are `--init --recursive --depth 1` to
# perform a shallow clone of the submodule. If the GIT_ARGS argument is provided, then its value
# supersedes the default options.
#
function(cpp_cc_git_submodule name)
  cmake_parse_arguments(PARSE_ARGV 1 opt "DISABLED;QUIET" "SUBDIR" "PACKAGE;BUILD;GIT_ARGS")
  string(MAKE_C_IDENTIFIER "USE_${name}" option_suffix)
  string(TOUPPER "3RDPARTY_${option_suffix}" option_suffix)
  if(opt_DISABLED)
    set(default OFF)
  else()
    set(default ON)
  endif()
  option(${CODING_CONV_PREFIX}_${option_suffix} "Use the git submodule ${name}" ${default})
  set(old_cmake_module_path "${CMAKE_MODULE_PATH}")
  # See if we have been told to skip initialising the submodule (and possibly to use
  # find_package(...)
  if(NOT ${CODING_CONV_PREFIX}_${option_suffix})
    if(opt_PACKAGE)
      find_package(${opt_PACKAGE})
    elseif(PACKAGE IN_LIST opt_KEYWORDS_MISSING_VALUES)
      message(SEND_ERROR "PACKAGE argument requires at least one argument")
    endif()
  else()
    set(submodule_path "${${CODING_CONV_PREFIX}_3RDPARTY_DIR}/${name}")
    if(opt_SUBDIR)
      set(submodule_path "${submodule_path}/${opt_SUBDIR}")
    endif()
    file(GLOB submodule_contents "${PROJECT_SOURCE_DIR}/${submodule_path}/*")
    if("${submodule_contents}" STREQUAL "")
      # The directory was empty
      if(opt_GIT_ARGS)
        cpp_cc_init_git_submodule("${submodule_path}" GIT_ARGS ${opt_GIT_ARGS})
      else()
        cpp_cc_init_git_submodule("${submodule_path}")
      endif()
    endif()
    if(NOT opt_QUIET)
      message(STATUS "3rd party project: using ${name} from \"${submodule_path}\"")
    endif()
    if(opt_BUILD)
      add_subdirectory(${submodule_path} ${opt_BUILD})
    elseif("BUILD" IN_LIST opt_KEYWORDS_MISSING_VALUES)
      add_subdirectory(${submodule_path})
    endif()
  endif()
  if(NOT "${old_cmake_module_path}" STREQUAL "${CMAKE_MODULE_PATH}")
    list(APPEND additions ${CMAKE_MODULE_PATH})
    list(REMOVE_ITEM additions ${old_cmake_module_path})
    message(STATUS "cpp_cc_git_submodule adding ${additions} to CMAKE_MODULE_PATH")
    set(CMAKE_MODULE_PATH
        "${CMAKE_MODULE_PATH}"
        PARENT_SCOPE)
  endif()
endfunction()
