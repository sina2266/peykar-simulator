project(netcdf_for)

file (GLOB SRC_FILES *.c)

# fort-nc4.c wraps the functions netCDF-4 added to the C API. Its own header
# says it "is only compiled for netCDF-4 builds", and this is netCDF 3.6.2: the
# C library here has none of those functions, and the F90 layer (netcdf.f90)
# does not include netcdf4_*.f90, so nothing calls the wrappers. The old
# Makefiles compiled it anyway, which only worked while calling an undeclared
# function was a warning; GCC 14 and Clang make it an error.
list(FILTER SRC_FILES EXCLUDE REGEX "fort-nc4\\.c$")

add_library(netcdf_for STATIC ${SRC_FILES})
