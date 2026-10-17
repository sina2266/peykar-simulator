@PACKAGE_INIT@

include(CMakeFindDependencyMacro)
include("${CMAKE_CURRENT_LIST_DIR}/ResolveLibXml2.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/ResolveZLIB.cmake")

resolve_libxml2(LIBXML2_TARGET)

if (NOT "${CONST_XMLERROR}" STREQUAL "@CONST_XMLERROR@")
    message(FATAL_ERROR "LibXml2 found but does not have the required API (xmlSetStructuredErrorFunc with const xmlError*) for this version of libCellML.")
endif()

resolve_zlib(${LIBXML2_TARGET} ZLIB_TARGET)

include("${CMAKE_CURRENT_LIST_DIR}/libcellml-targets.cmake")
