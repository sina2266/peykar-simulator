# =============================================================================
# Even when IV_ENABLE_X11_DYNAMIC, explicitly link to X11 if static build.
# =============================================================================
if (IV_ENABLE_X11_DYNAMIC AND NOT IV_ENABLE_SHARED)
    set(_ivx11 ${CMAKE_CURRENT_SOURCE_DIR}/../lib/x11_dynam/ivx11_dynamlib.cpp)
    include_directories("${X11_INCLUDE_DIR}")
else()
    set(_ivx11 "")
endif()

# =============================================================================
# idemo
# =============================================================================
set(IDEMO_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/idemo/main.cpp ${_ivx11})
add_executable(idemo ${IDEMO_SOURCE_FILES})
target_link_libraries(idemo interviews ${X11_LIBRARIES})
install(TARGETS idemo DESTINATION ${IV_BIN_INSTALL_DIR})

# =============================================================================
# iclass
# =============================================================================
set(ICLASS_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/iclass/classbuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/iclass/classeditor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/iclass/classinfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/iclass/dialogs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/iclass/direct.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/iclass/globals.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/iclass/iclass.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/iclass/main.cpp
    ${_ivx11})

if(NOT IV_WINDOWS_BUILD)
  add_executable(iclass ${ICLASS_SOURCE_FILES})
  target_include_directories(iclass BEFORE PUBLIC ${IV_PROJECT_SOURCE_DIR}/src/include/IV-2_6)
  set_source_files_properties(${ICLASS_SOURCE_FILES} PROPERTIES COMPILE_FLAGS "-Div2_6_compatible")
  target_link_libraries(iclass unidraw interviews ${X11_LIBRARIES})
endif()

# =============================================================================
# idraw
# =============================================================================
set(IDRAW_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/idraw/idarrow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/idraw/idarrowhead.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/idraw/idarrows.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/idraw/idcatalog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/idraw/idcmds.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/idraw/idcomp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/idraw/idcreator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/idraw/iddialogs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/idraw/ided.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/idraw/idkybd.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/idraw/idvars.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/idraw/main.cpp
    ${_ivx11})

if(NOT IV_WINDOWS_BUILD)
  add_executable(idraw ${IDRAW_SOURCE_FILES})
  target_include_directories(idraw BEFORE PUBLIC ${IV_PROJECT_SOURCE_DIR}/src/include/IV-2_6)
  set_source_files_properties(${IDRAW_SOURCE_FILES} PROPERTIES COMPILE_FLAGS "-Div2_6_compatible")
  target_link_libraries(idraw unidraw interviews ${X11_LIBRARIES})
endif()

# =============================================================================
# Install various targets
# =============================================================================
if(NOT IV_WINDOWS_BUILD)
    install(TARGETS iclass DESTINATION ${IV_BIN_INSTALL_DIR})
    install(TARGETS idraw DESTINATION ${IV_BIN_INSTALL_DIR})
endif()
