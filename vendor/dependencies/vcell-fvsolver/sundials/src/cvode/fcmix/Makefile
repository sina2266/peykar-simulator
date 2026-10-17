# -----------------------------------------------------------------
# $Revision: 1.4 $
# $Date: 2006/08/11 23:03:07 $
# -----------------------------------------------------------------
# Programmer(s): Radu Serban and Aaron Collier @ LLNL
# -----------------------------------------------------------------
# Copyright (c) 2002, The Regents of the University of California.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# -----------------------------------------------------------------
# Makefile for FCVODE module
#
# src/cvode/fcmix/Makefile.  Generated from Makefile.in by configure.
# -----------------------------------------------------------------

SHELL = /bin/sh

srcdir       = .
builddir     = .
abs_builddir = /home/VCELL/fgao/winhome/sundials-2.3.0/src/cvode/fcmix
top_builddir = ../../..
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

top_srcdir = $(srcdir)/../../..

INCLUDES = -I$(top_builddir)/src -I$(top_srcdir)/include -I$(top_srcdir)/src/cvode

LIB_REVISION = 0:1:0

FCVODE_LIB       = libsundials_fcvode.la
FCVODE_LIB_FILES = fcvode.lo fcvband.lo fcvdense.lo fcvjtimes.lo fcvpreco.lo fcvbbd.lo fcvbp.lo fcvroot.lo fcvewt.lo
FCVODE_OBJ_FILES = fcvode.o fcvband.o fcvdense.o fcvjtimes.o fcvpreco.o fcvbbd.o fcvbp.o fcvroot.o fcvewt.o

mkinstalldirs = $(SHELL) $(top_srcdir)/config/mkinstalldirs

all: $(FCVODE_LIB)

$(FCVODE_LIB): $(FCVODE_LIB_FILES)
	$(LIBTOOL) --mode=link $(CC) $(CFLAGS) -o $(FCVODE_LIB) $(FCVODE_LIB_FILES) -rpath $(libdir) $(LDFLAGS) $(LIBS) -static -version-info $(LIB_REVISION)

install: $(FCVODE_LIB)
	$(mkinstalldirs) $(libdir)
	$(LIBTOOL) --mode=install $(INSTALL_LIB) $(FCVODE_LIB) $(libdir)

uninstall:
	$(LIBTOOL) --mode=uninstall rm -f $(libdir)/$(FCVODE_LIB)

clean:
	$(LIBTOOL) --mode=clean rm -f $(FCVODE_LIB)
	rm -f $(FCVODE_LIB_FILES)
	rm -f $(FCVODE_OBJ_FILES)

distclean: clean
	rm -f Makefile

fcvode.lo: $(srcdir)/fcvode.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -c $(srcdir)/fcvode.c
fcvewt.lo: $(srcdir)/fcvewt.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -c $(srcdir)/fcvewt.c
fcvband.lo: $(srcdir)/fcvband.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -c $(srcdir)/fcvband.c
fcvdense.lo: $(srcdir)/fcvdense.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -c $(srcdir)/fcvdense.c
fcvjtimes.lo: $(srcdir)/fcvjtimes.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -c $(srcdir)/fcvjtimes.c
fcvpreco.lo: $(srcdir)/fcvpreco.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -c $(srcdir)/fcvpreco.c
fcvbbd.lo: $(srcdir)/fcvbbd.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -c $(srcdir)/fcvbbd.c
fcvbp.lo: $(srcdir)/fcvbp.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -c $(srcdir)/fcvbp.c
fcvroot.lo: $(srcdir)/fcvroot.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -c $(srcdir)/fcvroot.c

libtool: $(top_builddir)/$(LIBTOOL_DEPS)
	@cd ${top_builddir} ;                 \
	 ${SHELL} ./config.status --recheck ; \
	 cd ${abs_builddir}
