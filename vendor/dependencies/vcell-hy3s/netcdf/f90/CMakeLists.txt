project(netcdf_f90 Fortran)

set(SRC_FILES 
	netcdf.f90
	typeSizes.f90
)

# The compiler comes from the toolchain. This used to hardcode Intel Fortran,
# which is what the whole of Hy3S was built with before the split.
#
# It also used to set /iface:mixed_str_len_arg here for MSVC. That is not an
# Intel spelling of a preprocessor flag, as the split first assumed, but a
# calling convention this layer must share with the C code it calls -- so it is
# now set once at the top level, where it reaches these sources and the Hy3S
# targets alike. See the comment there.

add_library(netcdf_f90 STATIC ${SRC_FILES})

