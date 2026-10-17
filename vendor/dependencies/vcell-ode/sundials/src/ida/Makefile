# -----------------------------------------------------------------
# $Revision: 1.4 $
# $Date: 2006/08/11 23:03:11 $
# -----------------------------------------------------------------
# Programmer(s): Radu Serban and Aaron Collier @ LLNL
# -----------------------------------------------------------------
# Copyright (c) 2002, The Regents of the University of California.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# -----------------------------------------------------------------
# Makefile for IDA module
#
# src/ida/Makefile.  Generated from Makefile.in by configure.
# -----------------------------------------------------------------

SHELL = /bin/sh



srcdir       = .
builddir     = .
abs_builddir = /home/VCELL/fgao/winhome/sundials-2.3.0/src/ida
top_builddir = ../..
prefix       = /usr/local
exec_prefix  = ${prefix}
includedir   = ${prefix}/include
libdir       = ${exec_prefix}/lib

INSTALL        = /usr/bin/install -c
INSTALL_LIB    = ${INSTALL}
INSTALL_HEADER = ${INSTALL} -m 644

LIBTOOL      = $(SHELL) $(top_builddir)/libtool
LIBTOOL_DEPS = config/ltmain.sh

CPP      = cc -E
CPPFLAGS = 
CC       = cc
CFLAGS   = 
LDFLAGS  = 
LIBS     = -lm 

top_srcdir = $(srcdir)/../..

INCLUDES = -I$(top_builddir)/src -I$(top_srcdir)/include

LIB_REVISION = 1:0:0

IDA_LIB       = libsundials_ida.la
IDA_LIB_FILES = ida.lo ida_ic.lo ida_io.lo ida_dense.lo ida_band.lo ida_spils.lo ida_spbcgs.lo ida_spgmr.lo ida_sptfqmr.lo ida_bbdpre.lo
IDA_OBJ_FILES = ida.o ida_ic.o ida_io.o ida_dense.o ida_band.o ida_spils.o ida_spbcgs.o ida_spgmr.o ida_sptfqmr.o ida_bbdpre.o

SHARED_LIB_FILES = $(top_builddir)/src/sundials/sundials_band.lo \
                   $(top_builddir)/src/sundials/sundials_dense.lo \
                   $(top_builddir)/src/sundials/sundials_iterative.lo \
                   $(top_builddir)/src/sundials/sundials_math.lo \
                   $(top_builddir)/src/sundials/sundials_nvector.lo \
                   $(top_builddir)/src/sundials/sundials_smalldense.lo \
                   $(top_builddir)/src/sundials/sundials_spgmr.lo \
                   $(top_builddir)/src/sundials/sundials_spbcgs.lo \
                   $(top_builddir)/src/sundials/sundials_sptfqmr.lo

mkinstalldirs = $(SHELL) $(top_srcdir)/config/mkinstalldirs

all: $(IDA_LIB)

$(IDA_LIB): shared $(IDA_LIB_FILES)
	$(LIBTOOL) --mode=link $(CC) $(CFLAGS) -o $(IDA_LIB) $(IDA_LIB_FILES) $(SHARED_LIB_FILES) -rpath $(libdir) $(LDFLAGS) $(LIBS) -version-info $(LIB_REVISION)

install: $(IDA_LIB)
	$(mkinstalldirs) $(includedir)/ida
	$(mkinstalldirs) $(libdir)
	$(LIBTOOL) --mode=install $(INSTALL_LIB) $(IDA_LIB) $(libdir)
	$(INSTALL_HEADER) $(top_srcdir)/include/ida/ida.h         $(includedir)/ida/
	$(INSTALL_HEADER) $(top_srcdir)/include/ida/ida_dense.h   $(includedir)/ida/
	$(INSTALL_HEADER) $(top_srcdir)/include/ida/ida_band.h    $(includedir)/ida/
	$(INSTALL_HEADER) $(top_srcdir)/include/ida/ida_spbcgs.h  $(includedir)/ida/
	$(INSTALL_HEADER) $(top_srcdir)/include/ida/ida_spgmr.h   $(includedir)/ida/
	$(INSTALL_HEADER) $(top_srcdir)/include/ida/ida_sptfqmr.h $(includedir)/ida/
	$(INSTALL_HEADER) $(top_srcdir)/include/ida/ida_bbdpre.h  $(includedir)/ida/
	$(INSTALL_HEADER) $(top_srcdir)/include/ida/ida_spils.h   $(includedir)/ida/
	$(INSTALL_HEADER) $(top_srcdir)/src/ida/ida_impl.h        $(includedir)/ida/

uninstall:
	$(LIBTOOL) --mode=uninstall rm -f $(libdir)/$(IDA_LIB)
	rm -f $(includedir)/ida/ida.h
	rm -f $(includedir)/ida/ida_dense.h
	rm -f $(includedir)/ida/ida_band.h
	rm -f $(includedir)/ida/ida_spbcgs.h
	rm -f $(includedir)/ida/ida_spgmr.h
	rm -f $(includedir)/ida/ida_sptfqmr.h
	rm -f $(includedir)/ida/ida_bbdpre.h
	rm -f $(includedir)/ida/ida_spils.h
	rm -f $(includedir)/ida/ida_impl.h
	@if test -d ${includedir}/ida ; then \
	   rmdir ${includedir}/ida ;         \
	 fi

shared:
	@cd ${top_builddir}/src/sundials ; \
	 ${MAKE} ;                         \
	 cd ${abs_builddir}

clean:
	$(LIBTOOL) --mode=clean rm -f $(IDA_LIB)
	rm -f $(IDA_LIB_FILES)
	rm -f $(IDA_OBJ_FILES)

distclean: clean
	rm -f Makefile

ida.lo: $(srcdir)/ida.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -c $(srcdir)/ida.c
ida_ic.lo: $(srcdir)/ida_ic.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -c $(srcdir)/ida_ic.c
ida_io.lo: $(srcdir)/ida_io.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -c $(srcdir)/ida_io.c
ida_dense.lo: $(srcdir)/ida_dense.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -c $(srcdir)/ida_dense.c
ida_band.lo: $(srcdir)/ida_band.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -c $(srcdir)/ida_band.c
ida_spils.lo: $(srcdir)/ida_spils.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -c $(srcdir)/ida_spils.c
ida_spbcgs.lo: $(srcdir)/ida_spbcgs.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -c $(srcdir)/ida_spbcgs.c
ida_spgmr.lo: $(srcdir)/ida_spgmr.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -c $(srcdir)/ida_spgmr.c
ida_sptfqmr.lo: $(srcdir)/ida_sptfqmr.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -c $(srcdir)/ida_sptfqmr.c
ida_bbdpre.lo: $(srcdir)/ida_bbdpre.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -c $(srcdir)/ida_bbdpre.c

libtool: $(top_builddir)/$(LIBTOOL_DEPS)
	@cd ${top_builddir} ;                 \
	 ${SHELL} ./config.status --recheck ; \
	 cd ${abs_builddir}
