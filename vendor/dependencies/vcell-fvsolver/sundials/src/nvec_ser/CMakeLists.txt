project(sundials_nvecserial C)    
    
file (GLOB SRC_FILES *.c)    
include_directories(../../include ../)

add_library(sundials_nvecserial STATIC ${SRC_FILES})   
target_include_directories(sundials_nvecserial PUBLIC 
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/.>
  $<INSTALL_INTERFACE:.>  # <prefix>/.
)
install(TARGETS sundials_nvecserial 
	ARCHIVE DESTINATION bin)