# Fortran Options

# Standard Fortran options
FOPT     = -O -temp=.
# Special Fortran options
SFOPT    =
FFLAGS   = ${FOPT} ${SFOPT}

# Source Code Directories

# Directories for routines that are the same in all versions
# SDIR contains routines without external statements
SDIR     = .

# LIBDIR contains PCGPAK2 library
LIBDIR   = ../../lib
# Precision options -- s(ingle), d(ouble), z(complex)
PRECOPT  = d
# Debug part of library name
DB       =
# Special options part of library name
LBOPT    =
# LIBRARY name is {s,d,z}blas{dbg option}{spec options}.
LIBRARY  = ${LIBDIR}/${PRECOPT}blas${DB}${LBOPT}.a

# Source File Names

FBLAS  =  $(PRECOPT)nrm2.o $(PRECOPT)axpy.o $(PRECOPT)dot.o \
          $(PRECOPT)copy.o $(PRECOPT)scal.o i$(PRECOPT)amax.o \
          $(PRECOPT)swap.o
blas:    ${LIBRARY}
${LIBRARY}: ${FBLAS}
	  ar rv ${LIBRARY} *.o
	  ranlib ${LIBRARY}
${FBLAS}:
	  f77 $(FFLAGS) -c $(SDIR)/$<
