project(blas Fortran)    

if(MSVC AND (CMAKE_Fortran_COMPILER_ID STREQUAL "Intel" OR
		CMAKE_Fortran_COMPILER_ID STREQUAL "IntelLLVM"))
	set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} /names:lowercase")
elseif(MSVC)
	# Flang defaults to lowercase already, no flag needed
elseif(UNIX)
	set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -static")
endif()

file (GLOB SRC_FILES *.f)    
add_library(blas STATIC ${SRC_FILES})
install(TARGETS blas 
	ARCHIVE DESTINATION bin)