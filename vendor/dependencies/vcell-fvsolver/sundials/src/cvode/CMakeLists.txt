project(sundials_cvode C)    
    
file (GLOB SRC_FILES *.c)    
include_directories(../../include ../)

add_library(sundials_cvode STATIC ${SRC_FILES})
target_include_directories(sundials_cvode PUBLIC 
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/.>
  $<INSTALL_INTERFACE:.>  # <prefix>/.
)
install(TARGETS sundials_cvode 
	ARCHIVE DESTINATION bin)