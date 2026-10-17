project(netcdf_cxx)

set(SRC_FILES 
	ncvalues.cpp
	netcdf.cpp
)

add_library(netcdf_cxx STATIC ${SRC_FILES})
install(TARGETS netcdf_cxx 
	ARCHIVE DESTINATION bin)