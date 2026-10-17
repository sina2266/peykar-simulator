# ResolveLibXml2.cmake
#
# Provides:
#   resolve_libxml2(<out_var>)
#
# Guarantees:
#   - <out_var> is set to a valid target
#   - canonical target LibXml2::LibXml2 exists
#
function(resolve_libxml2 out_var)

  find_package(LibXml2 CONFIG QUIET)
  if(NOT LibXml2_FOUND)
    find_package(LibXml2 REQUIRED)
  endif()

  set(_libxml2_target "")

  if(TARGET LibXml2::LibXml2)
    set(_libxml2_target LibXml2::LibXml2)
  elseif(TARGET LibXml2)
    set(_libxml2_target LibXml2)
  elseif(TARGET xml2)
    set(_libxml2_target xml2)
  endif()

  if(NOT _libxml2_target)
    if(DEFINED LIBXML2_LIBRARIES AND DEFINED LIBXML2_INCLUDE_DIRS)
      message(STATUS "LibXml2: Creating compatibility interface target (legacy config)")
      add_library(_libxml2_legacy INTERFACE)
      target_include_directories(_libxml2_legacy INTERFACE
        ${LIBXML2_INCLUDE_DIRS}
      )
      target_link_libraries(_libxml2_legacy INTERFACE
        ${LIBXML2_LIBRARIES}
      )
      if(DEFINED LIBXML2_DEFINITIONS AND LIBXML2_DEFINITIONS)
        set(_defs "${LIBXML2_DEFINITIONS}")
        list(TRANSFORM _defs REPLACE "^-D" "")
        target_compile_definitions(_libxml2_legacy INTERFACE ${_defs})
      endif()
      set(_libxml2_target _libxml2_legacy)
    else()
      message(FATAL_ERROR "LibXml2 found but no usable targets or variables.")
    endif()
  endif()


  if(NOT TARGET LibXml2::LibXml2)
    add_library(LibXml2::LibXml2 INTERFACE IMPORTED)
    target_link_libraries(LibXml2::LibXml2 INTERFACE
      ${_libxml2_target}
    )
  endif()

  foreach(_INC_DIR IN LISTS LIBXML2_INCLUDE_DIRS)
    if(EXISTS "${_INC_DIR}/libxml/xmlversion.h")
      file(STRINGS "${_INC_DIR}/libxml/xmlversion.h" libxml2_version_str
           REGEX "^#define[\t ]+LIBXML_DOTTED_VERSION[\t ]+\".*\"")

      string(REGEX REPLACE "^#define[\t ]+LIBXML_DOTTED_VERSION[\t ]+\"([^\"]*)\".*" "\\1"
            LibXml2_VERSION "${libxml2_version_str}")
      set(LIBXML2_VERSION_STRING "${LibXml2_VERSION}" PARENT_SCOPE)
      unset(libxml2_version_str)
      break()
    endif()
  endforeach()

  if(NOT _LIBXML2_RESOLUTION_REPORTED)
    set(_LIBXML2_RESOLUTION_REPORTED TRUE CACHE INTERNAL "Flag for reporting on what LibXml2 was found.")
    message(STATUS "LibXml2 resolved to target: ${_libxml2_target}")
    message(STATUS "Canonical target: LibXml2::LibXml2 (Version: ${LibXml2_VERSION})")
  endif()

  if(LibXml2_VERSION VERSION_GREATER_EQUAL "2.12.0")
    set(CONST_XMLERROR CONST PARENT_SCOPE)
  else()
    set(CONST_XMLERROR NON_CONST PARENT_SCOPE)
  endif()

  set(${out_var} LibXml2::LibXml2 PARENT_SCOPE)

endfunction()
