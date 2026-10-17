project(netcdf)

set(SRC_FILES 
	attr.c
	dim.c
	error.c
	libvers.c
	nc.c
	ncx.c
	posixio.c
	putget.c
	string.c
	v1hpg.c
	v2i.c
	var.c
)

add_library(netcdf STATIC ${SRC_FILES})
install(TARGETS netcdf 
	ARCHIVE DESTINATION bin)