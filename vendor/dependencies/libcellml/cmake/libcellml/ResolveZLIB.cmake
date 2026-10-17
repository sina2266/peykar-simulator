# ResolveZLIB.cmake
#
# Provides:
#   resolve_zlib(libxml2_target <out_var>)
#
# Guarantees:
#   - <out_var> is set to a valid target
#   - canonical target ZLIB::ZLIB exists
#
function(resolve_zlib libxml2_target out_var)

  set(_zlib_target "")

  if(TARGET ${libxml2_target})
    get_target_property(_xml2_links ${libxml2_target} INTERFACE_LINK_LIBRARIES)
    foreach(item IN LISTS _xml2_links)
      if(TARGET ${item})
        if(item STREQUAL "ZLIB::ZLIB" OR item STREQUAL "z")
          set(_zlib_target ${item})
          break()
        endif()

        get_target_property(_name ${item} NAME)
        if(_name MATCHES "[Zz][Ll][Ii][Bb]")
          set(_zlib_target ${item})
          break()
        endif()
      endif()
    endforeach()
  endif()

  if(NOT _zlib_target)
    find_package(ZLIB CONFIG QUIET)
    if(NOT ZLIB_FOUND)
      find_package(ZLIB REQUIRED)
    endif()
    if(TARGET ZLIB::ZLIB)
      set(_zlib_target ZLIB::ZLIB)
    elseif(TARGET z)
      set(_zlib_target z)
    endif()
  endif()

  if(NOT _zlib_target)
    if(DEFINED ZLIB_LIBRARIES AND DEFINED ZLIB_INCLUDE_DIRS)
      add_library(_zlib_legacy INTERFACE)
      target_include_directories(_zlib_legacy INTERFACE
        ${ZLIB_INCLUDE_DIRS}
      )
      target_link_libraries(_zlib_legacy INTERFACE
        ${ZLIB_LIBRARIES}
      )
      set(_zlib_target _zlib_legacy)
    else()
      message(FATAL_ERROR "ZLIB found but no usable targets or variables.")
    endif()
  endif()

  if(NOT TARGET ZLIB::ZLIB)
    add_library(ZLIB::ZLIB INTERFACE IMPORTED)
    target_link_libraries(ZLIB::ZLIB INTERFACE
      ${_zlib_target}
    )
  endif()

  if(NOT _ZLIB_RESOLUTION_REPORTED)
    set(_ZLIB_RESOLUTION_REPORTED TRUE CACHE INTERNAL "Flag for reporting on what ZLIB was found.")
    message(STATUS "ZLIB resolved to target: ${_zlib_target}")
    message(STATUS "Canonical target: ZLIB::ZLIB (Version: ${ZLIB_VERSION})")
  endif()

  set(${out_var} ZLIB::ZLIB PARENT_SCOPE)
endfunction()
