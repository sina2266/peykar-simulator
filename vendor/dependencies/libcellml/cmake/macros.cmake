macro(apply_compiler_cache_settings)
  # Use compiler cache.
  if(BUILDCACHE_EXE)
    set(CMAKE_CXX_COMPILER_LAUNCHER ${BUILDCACHE_EXE})
  elseif(CLCACHE_EXE)
    set(CLCACHEWRAPPER ${CMAKE_CURRENT_BINARY_DIR}/clcachewrapper)
    execute_process(COMMAND ${CMAKE_C_COMPILER} /O2 /Fe${CLCACHEWRAPPER} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/clcachewrapper.c
                    RESULT_VARIABLE RESULT OUTPUT_QUIET ERROR_QUIET)
    if(RESULT EQUAL 0)
      set(CMAKE_CXX_COMPILER_LAUNCHER ${CLCACHEWRAPPER})
    endif()
  else()
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_EXE})
  endif()

  # On Windows, if we are using MSVC then replace some compiler flags.
  if(MSVC)
    # Replace the /Zi option (i.e. enable debugging information), if present,
    # with the /Z7 option (i.e. enable old-style debugging information)
    # otherwise caching (be it with buildcache or clcache) won't work.
    if(BUILDCACHE_EXE OR CLCACHE_EXE)
      replace_compiler_flag("/Zi" "/Z7")
    endif()
  endif()
endmacro()
