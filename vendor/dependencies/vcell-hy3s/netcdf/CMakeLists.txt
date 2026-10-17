# libsrc is C, fortran/ and f90/ are Fortran. The original declared only
# Fortran, which happened to work inside the mega-repo because C was already
# enabled at the top level.
project(netcdf C Fortran)

include_directories(libsrc cxx)

if (MSVC) 
add_definitions(-DPowerStationFortran)
include_directories(win32)
else()
include_directories(linux)
endif()

add_subdirectory(libsrc)
#add_subdirectory(cxx)
add_subdirectory(fortran)
add_subdirectory(f90)