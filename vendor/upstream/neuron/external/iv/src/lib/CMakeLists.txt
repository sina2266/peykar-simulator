# =============================================================================
# Library sources list
# =============================================================================
set(DISPATCH_SOURCE_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/Dispatch/dispatcher.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Dispatch/iohandler.cpp)

set(IV26_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-2_6/adjuster2_6.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-2_6/banner.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-2_6/border2_6.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-2_6/box2_6.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-2_6/button2_6.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-2_6/compeditor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-2_6/control.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-2_6/deck2_6.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-2_6/dialog2_6.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-2_6/filebrowser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-2_6/filechooser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-2_6/frame.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-2_6/glue2_6.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-2_6/interactor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-2_6/matcheditor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-2_6/menu2_6.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-2_6/message.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-2_6/painter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-2_6/panner2_6.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-2_6/perspective.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-2_6/rubband.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-2_6/rubcurve.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-2_6/rubgroup.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-2_6/rubline.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-2_6/rubrect.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-2_6/rubverts.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-2_6/scene.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-2_6/scrollbar2_6.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-2_6/scroller2_6.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-2_6/sensor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-2_6/shape.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-2_6/strbrowser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-2_6/strchooser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-2_6/streditor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-2_6/subject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-2_6/textbuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-2_6/textdisplay.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-2_6/texteditor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-2_6/tform2_6.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-2_6/tray.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-2_6/viewport.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-2_6/world.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-2_6/xbitmap2_6.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-2_6/xevent2_6.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-2_6/xinter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-2_6/xpainter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-2_6/xpattern.cpp)

set(INTERVIEWS_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/dialogs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/action.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/adjust.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/aggr.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/align.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/alloctbl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/arrcomp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/background.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/bevel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/border.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/box.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/browser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/button.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/character.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/compositor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/comption.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/debug.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/deck.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/fbrowser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/fchooser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/field.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/geometry.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/glyph.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/group.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/handler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/hit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/image.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/input.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/kit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/label.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/layout.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/lrmarker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/menu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/mf_dialogs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/mf_kit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/mono_kit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/monoglyph.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/observe.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/ol_dialogs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/page.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/patch.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/place.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/polyglyph.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/printer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/psfont.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/regexp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/resource.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/rule.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/scrbox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/shadow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/simpcomp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/slider.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/smf_kit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/stencil.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/stepper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/style.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/superpose.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/target.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/telltale.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/texcomp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/iv3text.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/iv3textbuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/tformsetter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/tile.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/transformer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/winbmp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/xymarker.cpp)

if(NOT IV_WINDOWS_BUILD)
  list(APPEND INTERVIEWS_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/ol_kit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/tiff.cpp)
endif()

set(IVX11_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-X11/session.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-X11/xbitmap.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-X11/xbrush.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-X11/xcanvas.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-X11/xcolor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-X11/xcursor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-X11/xdrag.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-X11/xevent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-X11/xfont.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-X11/xraster.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-X11/xreqerr.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-X11/xselection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-X11/xwindow.cpp)

set(OS_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/OS/directory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/OS/file.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/OS/host.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/OS/listimpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/OS/math.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/OS/memory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/OS/string.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/OS/ustring.cpp)

set(TIFF_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/TIFF/tif_aux.c
    ${CMAKE_CURRENT_SOURCE_DIR}/TIFF/tif_ccittrle.c
    ${CMAKE_CURRENT_SOURCE_DIR}/TIFF/tif_close.c
    ${CMAKE_CURRENT_SOURCE_DIR}/TIFF/tif_cmprs.c
    ${CMAKE_CURRENT_SOURCE_DIR}/TIFF/tif_compat.c
    ${CMAKE_CURRENT_SOURCE_DIR}/TIFF/tif_dir.c
    ${CMAKE_CURRENT_SOURCE_DIR}/TIFF/tif_dirinfo.c
    ${CMAKE_CURRENT_SOURCE_DIR}/TIFF/tif_dirread.c
    ${CMAKE_CURRENT_SOURCE_DIR}/TIFF/tif_dirwrite.c
    ${CMAKE_CURRENT_SOURCE_DIR}/TIFF/tif_dumpmode.c
    ${CMAKE_CURRENT_SOURCE_DIR}/TIFF/tif_error.c
    ${CMAKE_CURRENT_SOURCE_DIR}/TIFF/tif_fax3.c
    ${CMAKE_CURRENT_SOURCE_DIR}/TIFF/tif_fax4.c
    ${CMAKE_CURRENT_SOURCE_DIR}/TIFF/tif_flush.c
    ${CMAKE_CURRENT_SOURCE_DIR}/TIFF/tif_getimage.c
    ${CMAKE_CURRENT_SOURCE_DIR}/TIFF/tif_jpeg.c
    ${CMAKE_CURRENT_SOURCE_DIR}/TIFF/tif_lzw.c
    ${CMAKE_CURRENT_SOURCE_DIR}/TIFF/tif_machdep.c
    ${CMAKE_CURRENT_SOURCE_DIR}/TIFF/tif_next.c
    ${CMAKE_CURRENT_SOURCE_DIR}/TIFF/tif_open.c
    ${CMAKE_CURRENT_SOURCE_DIR}/TIFF/tif_packbits.c
    ${CMAKE_CURRENT_SOURCE_DIR}/TIFF/tif_print.c
    ${CMAKE_CURRENT_SOURCE_DIR}/TIFF/tif_read.c
    ${CMAKE_CURRENT_SOURCE_DIR}/TIFF/tif_strip.c
    ${CMAKE_CURRENT_SOURCE_DIR}/TIFF/tif_swab.c
    ${CMAKE_CURRENT_SOURCE_DIR}/TIFF/tif_thunder.c
    ${CMAKE_CURRENT_SOURCE_DIR}/TIFF/tif_tile.c
    ${CMAKE_CURRENT_SOURCE_DIR}/TIFF/tif_version.c
    ${CMAKE_CURRENT_SOURCE_DIR}/TIFF/tif_warning.c
    ${CMAKE_CURRENT_SOURCE_DIR}/TIFF/tif_write.c)

set(IVWIN_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-Win/bitmap.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-Win/brush.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-Win/canvas.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-Win/canvas16.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-Win/color.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-Win/cursor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-Win/display.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-Win/event.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-Win/font.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-Win/ivclean.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-Win/mprinter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-Win/mwapp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-Win/mwlib.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-Win/raster.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-Win/session.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-Win/window.cpp)

set(IVMAC_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-Mac/bitmap.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-Mac/brush.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-Mac/canvas.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-Mac/color.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-Mac/cursor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-Mac/display.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-Mac/event.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-Mac/carbevent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-Mac/font.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-Mac/mprinter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-Mac/raster.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-Mac/session.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IV-Mac/window.cpp)

set(UNIDRAW_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/align.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/brushcmd.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/catalog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/catcmds.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/cglue.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/clipboard.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/colorcmd.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/command.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/component.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/compview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/connect.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/connector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/creator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/csolver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/ctrlinfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/damage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/data.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/datas.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/dialogs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/edit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/editor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/editorinfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/ellipse.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/ellipses.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/externview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/font.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/geomobjs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/globals.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/graphic.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/grblock.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/grcomp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/grcomptool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/grid.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/grview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/gvupdater.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/import.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/iterator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/keymap.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/kybd.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/line.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/lines.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/link.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/macro.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/magnify.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/manip.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/manips.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/move.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/nop.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/pad.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/patcmd.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/path.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/picture.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/pin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/polygon.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/polygons.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/pspaint.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/psview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/rastercomp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/rasterrect.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/rect.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/reshape.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/rotate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/scale.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/select.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/selection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/slot.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/spline.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/splines.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/statevar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/statevars.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/stateview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/stateviews.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/stencilcomp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/stretch.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/text.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/struct.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/tool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/transfn.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/transfns.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/transforms.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/uarray.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/uctrl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/uctrls.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/uhash.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/ulabel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/ulist.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/umap.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/unidraw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/upage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/ustencil.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/vertices.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/verts.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/viewcmds.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Unidraw/viewer.cpp)

set(IVX11DYNAM_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/x11_dynam/ivx11_dynam.cpp)

# =============================================================================
# mkg3states utility
# =============================================================================
# macros required to build
set(
  TIFF_DEFINES
  "-DCOLORIMETRY_SUPPORT -DJPEG_SUPPORT -DYCBCR_SUPPORT -DCMYK_SUPPORT -DHAVE_IEEEFP=1 -DUSE_VARARGS=0 -DUSE_PROTOTYPES=1 -DUSE_CONST=1"
  )

# command to generate g3states.h
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/g3states.h
                   COMMAND ${IV_PROJECT_BINARY_DIR}/bin/mkg3states -o=g3states.h
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                   VERBATIM
                   DEPENDS mkg3states)

set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/g3states.h
							PROPERTIES
							GENERATED
							TRUE)
# creates an explicit dependency between tif_fax3.c and g3states.h which cannot be
# handled automatically because g3states.h is autogenerated
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/TIFF/tif_fax3.c
                            PROPERTIES
                            OBJECT_DEPENDS
                            ${CMAKE_CURRENT_BINARY_DIR}/g3states.h)
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/TIFF/mkg3states.c
                            PROPERTIES
                            COMPILE_FLAGS
                            ${TIFF_DEFINES})

add_executable(mkg3states ${CMAKE_CURRENT_SOURCE_DIR}/TIFF/mkg3states.c)
target_include_directories(mkg3states PUBLIC ${IV_PROJECT_SOURCE_DIR}/src/include/TIFF)

# =============================================================================
# interviews IV library
# =============================================================================

if(NOT IV_WINDOWS_BUILD)
	set(INTERVIEWS_SOURCES
	    ${DISPATCH_SOURCE_FILES}
	    ${IV26_SOURCE_FILES}
	    ${INTERVIEWS_SOURCE_FILES}
	    ${TIFF_SOURCE_FILES}
	    ${OS_SOURCE_FILES}
	    ${IVX11_SOURCE_FILES})
else()
	set(INTERVIEWS_SOURCES
		${INTERVIEWS_SOURCE_FILES}
		${OS_SOURCE_FILES}
		${IVWIN_SOURCE_FILES})
endif()

if(IV_ENABLE_X11_DYNAMIC)
    list(APPEND INTERVIEWS_SOURCES ${IVX11DYNAM_SOURCE_FILES})
    set_source_files_properties(${INTERVIEWS_SOURCES}
        PROPERTIES
        COMPILE_DEFINITIONS "IVX11_DYNAM")
endif()

if(NOT IV_WINDOWS_BUILD)
  set(KIT_DEFINES "-Dmotif_kit -Dsgi_motif_kit -Dopenlook_kit -Dbw_kit -Ddefault_kit=SMFKit")
else()
  set(KIT_DEFINES "-Dmotif_kit -Dsgi_motif_kit")
endif()

set(DIV_DEFINES "-DIV_LIBALL='\"${CMAKE_INSTALL_PREFIX}/share\"' -DX_LIBDIR='\"${X11_LIBRARY_DIR}\"'")

set_source_files_properties(${TIFF_SOURCE_FILES} PROPERTIES COMPILE_FLAGS ${TIFF_DEFINES})

set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/InterViews/dialogs.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/InterViews/kit.cpp
                            PROPERTIES
                            COMPILE_FLAGS
                            ${KIT_DEFINES})
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/IV-X11/session.cpp
                            PROPERTIES
                            COMPILE_FLAGS
                            ${DIV_DEFINES})

add_library(interviews ${IV_LIB_TYPE} ${INTERVIEWS_SOURCES})
if (IV_ENABLE_X11_DYNAMIC)
    target_link_libraries(interviews dl)
else()
    target_link_libraries(interviews ${X11_LIBRARIES})
endif()
target_include_directories(interviews
                           PRIVATE ${X11_INCLUDE_DIR}
                                   $<BUILD_INTERFACE:${IV_PROJECT_SOURCE_DIR}/src/include/TIFF>
                                   $<INSTALL_INTERFACE:include/TIFF>)
set_property(TARGET interviews PROPERTY POSITION_INDEPENDENT_CODE ON)

if(IV_WINDOWS_BUILD)
  target_link_libraries(interviews gdi32 comdlg32)
endif()
if(NOT IV_WINDOWS_BUILD)
    set_target_properties(interviews PROPERTIES SOVERSION 0.0.0)
endif()

# =============================================================================
# ivx11dynam
# =============================================================================

# ivx11dynam is built before interviews
set(x11dynam_srcdir ${CMAKE_CURRENT_SOURCE_DIR}/x11_dynam)

if (IV_ENABLE_X11_DYNAMIC)
    list(APPEND INTERVIEWS_SOURCES ${x11dynam_srcdir}/ivx11_dynam.cpp)
    add_library(ivx11dynam SHARED ${x11dynam_srcdir}/ivx11_dynamlib.cpp)
    target_link_libraries(ivx11dynam ${X11_LIBRARIES})
    add_dependencies(interviews ivx11dynam)
    if (APPLE)
        set_target_properties(ivx11dynam PROPERTIES
            LINK_FLAGS "-undefined dynamic_lookup")
    endif()
    target_include_directories(ivx11dynam
        PRIVATE ${X11_INCLUDE_DIR})
    set_property(TARGET ivx11dynam PROPERTY POSITION_INDEPENDENT_CODE ON)
    install(TARGETS ivx11dynam
        EXPORT iv
        DESTINATION ${IV_LIB_INSTALL_DIR}
        INCLUDES
        DESTINATION ${IV_HEADERS_INSTALL_DIR})
endif()

# =============================================================================
# mkdynam creates four .h files when IV_ENABLE_X11_DYNAM=ON
# =============================================================================
if (IV_ENABLE_X11_DYNAMIC_MAKE_HEADERS)
    set(x11dynam_incdir ${IV_PROJECT_SOURCE_DIR}/src/include/IV-X11)
    set(x11dynam_files
        ${x11dynam_incdir}/ivx11_declare.h
        ${x11dynam_incdir}/ivx11_redef.h
        ${x11dynam_incdir}/ivx11_define.h
        ${x11dynam_incdir}/ivx11_assign.h)
    set(IV_X11_DYNAMIC_INCLUDE_FILES ${x11dynam_files} PARENT_SCOPE)

    # if the x11dynam_files exist and have a date later than mkdynam.py,
    # the custom command will not execute mkdynam.py . Delete the
    # x11dynam_files manually, or touch mkdynam.py if you wish to
    # regenerate the headers. Given the stability of the X11 API, it is
    # unlikely that IV_ENABLE_X11_DYNAMIC_MAKE_HEADERS will ever need to be ON.
    add_custom_command(OUTPUT ${x11dynam_files}
        COMMAND ${PYTHON_EXECUTABLE} '${x11dynam_srcdir}/mkdynam.py' '${x11dynam_incdir}' '${X11_INCLUDE_DIR}'
        DEPENDS ${x11dynam_srcdir}/mkdynam.py
        WORKING_DIRECTORY ${x11dynam_srcdir})

    add_custom_target(x11dynamheaders
        DEPENDS ${x11dynam_files})

    add_dependencies(interviews x11dynamheaders)
    if (IV_ENABLE_X11_DYNAMIC)
      add_dependencies(ivx11dynam x11dynamheaders)
    endif()
endif()

# =============================================================================
# Unidraw library
# =============================================================================
if(NOT IV_WINDOWS_BUILD)
  if(IV_ENABLE_SHARED)
    add_library(unidraw SHARED ${UNIDRAW_SOURCE_FILES})
    set_target_properties(unidraw PROPERTIES SOVERSION 0.0.0)
  else()
    add_library(unidraw STATIC ${UNIDRAW_SOURCE_FILES})
  endif()
  target_include_directories(unidraw PUBLIC ${X11_INCLUDE_DIR})
  target_link_libraries(unidraw interviews)
endif()

# =============================================================================
# Install targets
# =============================================================================
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/app-defaults/ DESTINATION ${IV_LIB_INSTALL_DIR}/share/app-defaults)

if(NOT IV_WINDOWS_BUILD)
  install(TARGETS interviews
          EXPORT iv
          DESTINATION ${IV_LIB_INSTALL_DIR}
          INCLUDES
          DESTINATION ${IV_HEADERS_INSTALL_DIR})
  install(TARGETS unidraw
          EXPORT iv
          DESTINATION ${IV_LIB_INSTALL_DIR}
          INCLUDES
          DESTINATION ${IV_HEADERS_INSTALL_DIR})
else()
  install(TARGETS interviews
          EXPORT iv
          DESTINATION ${IV_LIB_INSTALL_DIR}
          INCLUDES
          DESTINATION ${IV_HEADERS_INSTALL_DIR})
endif()
