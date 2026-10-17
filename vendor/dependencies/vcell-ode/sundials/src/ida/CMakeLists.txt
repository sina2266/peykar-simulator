project(sundials_ida C)    
    
file (GLOB SRC_FILES *.c)    
include_directories(../../include ..)

add_library(sundials_ida STATIC ${SRC_FILES})
target_include_directories(sundials_ida PUBLIC 
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/.>
  $<INSTALL_INTERFACE:.>  # <prefix>/.
)
install(TARGETS sundials_ida 
	ARCHIVE DESTINATION bin)