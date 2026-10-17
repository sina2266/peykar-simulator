project(sundials C)    

add_subdirectory(src/cvode)
add_subdirectory(src/ida)
add_subdirectory(src/nvec_ser)
add_subdirectory(src/sundials)

file(WRITE ${CMAKE_BINARY_DIR}/dummy.c "const char* sundials_dummy = \"dummy\";")

add_library(sundials ${CMAKE_BINARY_DIR}/dummy.c)
target_link_libraries(sundials sundials_cvode sundials_ida sundials_nvecserial sundials_lib)
target_include_directories(sundials INTERFACE 
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
	$<INSTALL_INTERFACE:include>  # <prefix>/include
	$<INSTALL_INTERFACE:src>  # <prefix>/src
)
SET_TARGET_PROPERTIES(sundials PROPERTIES LINKER_LANGUAGE C)
    
