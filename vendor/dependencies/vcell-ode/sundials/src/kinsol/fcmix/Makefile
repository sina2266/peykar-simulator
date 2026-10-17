# -----------------------------------------------------------------
# $Revision: 1.4 $
# $Date: 2006/08/11 23:03:21 $
# -----------------------------------------------------------------
# Programmer(s): Radu Serban and Aaron Collier @ LLNL
# -----------------------------------------------------------------
# Copyright (c) 2002, The Regents of the University of California.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# -----------------------------------------------------------------
# Makefile for FKINSOL module
#
# src/kinsol/fcmix/Makefile.  Generated from Makefile.in by configure.
# -----------------------------------------------------------------

SHELL = /bin/sh

srcdir       = .
builddir     = .
abs_builddir = /home/VCELL/fgao/winhome/sundials-2.3.0/src/kinsol/fcmix
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

INCLUDES = -I$(top_builddir)/src -I$(top_srcdir)/include -I$(top_srcdir)/src/kinsol

LIB_REVISION = 0:1:0

FKINSOL_LIB       = libsundials_fkinsol.la
FKINSOL_LIB_FILES = fkinsol.lo fkinpreco.lo fkinjtimes.lo fkinbbd.lo fkindense.lo
FKINSOL_OBJ_FILES = fkinsol.o fkinpreco.o fkinjtimes.o fkinbbd.o fkindense.o

mkinstalldirs = $(SHELL) $(top_srcdir)/config/mkinstalldirs

all: $(FKINSOL_LIB)

$(FKINSOL_LIB): $(FKINSOL_LIB_FILES)
	$(LIBTOOL) --mode=link $(CC) $(CFLAGS) -o $(FKINSOL_LIB) $(FKINSOL_LIB_FILES) -rpath $(libdir) $(LDFLAGS) $(LIBS) -static -version-info $(LIB_REVISION)

install: $(FKINSOL_LIB)
	$(mkinstalldirs) $(libdir)
	$(LIBTOOL) --mode=install $(INSTALL_LIB) $(FKINSOL_LIB) $(libdir)

uninstall:
	$(LIBTOOL) --mode=uninstall rm -f $(libdir)/$(FKINSOL_LIB)

clean:
	$(LIBTOOL) --mode=clean rm -f $(FKINSOL_LIB)
	rm -f $(FKINSOL_LIB_FILES)
	rm -f $(FKINSOL_OBJ_FILES)

distclean: clean
	rm -f Makefile

fkinsol.lo: $(srcdir)/fkinsol.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -c $(srcdir)/fkinsol.c
fkinpreco.lo: $(srcdir)/fkinpreco.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -c $(srcdir)/fkinpreco.c
fkinjtimes.lo: $(srcdir)/fkinjtimes.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -c $(srcdir)/fkinjtimes.c
fkinbbd.lo: $(srcdir)/fkinbbd.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -c $(srcdir)/fkinbbd.c
fkindense.lo: $(srcdir)/fkindense.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -c $(srcdir)/fkindense.c

libtool: $(top_builddir)/$(LIBTOOL_DEPS)
	@cd ${top_builddir} ;                 \
	 ${SHELL} ./config.status --recheck ; \
	 cd ${abs_builddir}
