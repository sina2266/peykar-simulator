project(BridgeVCellSmoldyn CXX)

set(VCELL_SMOL_MAIN ${CMAKE_CURRENT_SOURCE_DIR}/vcell_smoldyn_entrypoint.cpp)
file (GLOB HEADER_FILES *.h)    
file (GLOB SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
list(REMOVE_ITEM SOURCE_FILES ${VCELL_SMOL_MAIN}) 

add_library(vcellsmoldynbridge ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(vcellsmoldynbridge vcell smoldyn_static)
target_include_directories(vcellsmoldynbridge PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(vcellsmoldynbridge PUBLIC -DVCELL_HYBRID -DVCELL -D_CRT_SECURE_NO_WARNINGS)
set_property(TARGET vcellsmoldynbridge PROPERTY COMPILE_FLAGS "-DVCELL_HYBRID")

if(OPTION_TARGET_SMOLDYN_SOLVER)

	set(SMOLDYN_EXE smoldyn)
	if (ARCH_64bit)
		set(SMOLDYN_EXE ${SMOLDYN_EXE}_x64)	
	endif (ARCH_64bit)
	
	add_executable(${SMOLDYN_EXE} ${VCELL_SMOL_MAIN})
	target_link_libraries(${SMOLDYN_EXE} vcellsmoldynbridge)
	
	install(TARGETS ${SMOLDYN_EXE} RUNTIME DESTINATION
			${OPTION_EXE_DIRECTORY})

endif()
