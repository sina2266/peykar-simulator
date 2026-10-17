project(sundials_lib C)    
    
file (GLOB SRC_FILES *.c)    
include_directories(../../include ..)
    
add_library(sundials_lib STATIC ${SRC_FILES})    
target_include_directories(sundials_lib PUBLIC 
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/.>
  $<INSTALL_INTERFACE:.>  # <prefix>/.
)
install(TARGETS sundials_lib 
	ARCHIVE DESTINATION bin)