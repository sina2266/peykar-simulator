# Create a build rule that copies a file.
#
# cpp_cc_build_time_copy(INPUT <input_path> OUTPUT <output_path> [NO_TARGET])
#
# This creates a custom target that is always built and depends on `output_path` and a rule to
# create `output_path` by copying `input_path`. This means that changes to the input file
# (`input_path`, presumably in the source tree) can be propagated to `output_path` (presumably in
# the build tree) automatically and without re-running CMake. The existence of a custom command that
# produces `output_path` makes it trivial for other targets to declare that they depend on this
# file, provided that they are working in the same directory (CMakeLists.txt -- see documentation of
# DEPENDS arguments to add_custom_command and add_custom_target). If the NO_TARGET flag is passed
# then no target is added, and the caller takes responsibility for declaring a dependency on the
# output file and causing it to be built. This can be a good idea to avoid the number of top level
# targets growing too large, which causes the Make build system to be very slow.
function(cpp_cc_build_time_copy)
  cmake_parse_arguments(opt "NO_TARGET" "INPUT;OUTPUT" "" ${ARGN})
  if(NOT DEFINED opt_INPUT)
    message(ERROR "build_time_copy missing required keyword argument INPUT.")
  endif()
  if(NOT DEFINED opt_OUTPUT)
    message(ERROR "build_time_copy missing required keyword argument OUTPUT.")
  endif()
  add_custom_command(
    OUTPUT "${opt_OUTPUT}"
    DEPENDS "${opt_INPUT}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${opt_INPUT}" "${opt_OUTPUT}")
  if(NOT opt_NO_TARGET)
    string(SHA256 target_name "${opt_INPUT};${opt_OUTPUT}")
    string(SUBSTRING ${target_name} 0 10 target_name)
    set(target_name "build-time-copy-${target_name}")
    if(NOT TARGET "${target_name}")
      add_custom_target(${target_name} ALL DEPENDS "${opt_OUTPUT}")
    endif()
  endif()
endfunction()
