#GNU Makefile for Intel Linux compiler (v8.1)
#Howard Salis (3/10/05)

#set src directory into variable

srcdir = $(HOME)/Hy3S/src

#set lib and bin directories into variable

libdir = $(HOME)/Hy3S/lib
bindir = $(HOME)/Hy3S/bin

#Set Fortran compiler - if different, insert your own here or
#use an environmental variable

F90 ?= ifort
MPI ?= mpif90

#Set Fortran Flags here or from environment

F90FLAGS ?= -I$(libdir) -L$(libdir) -lnetcdf

#Aggressive optimization flags
F90Fast = -O3  -ipo -ipo_c -fast

#Intel Fortran compiler's flag to define pre-processing variables
#Substitute as necessary
DEF ?= -D

#List of necessary libraries
LIBS = $(libdir)/netcdf.o $(libdir)/typeSizes.o $(libdir)/libnetcdf.a

#default make option is aggressive optimization

all:	HyJCMSS-ser-Fixed-EM HyJCMSS-MPI-Fixed-EM HyJCMSS-ser-Fixed-Mil HyJCMSS-MPI-Fixed-Mil \
	HyJCMSS-ser-Adapt-EM HyJCMSS-MPI-Adapt-EM HyJCMSS-ser-Adapt-Mil HyJCMSS-MPI-Adapt-Mil \
	BenchMarkMaker SSA-MPI-opt SSA-serial-opt
	@echo Compiled all programs with aggressive optimizations. Done! 


debug: 	HyJCMSS-Debug SSA-Debug HyJCMSS-MPI-Debug SSA-MPI-Debug
	@echo Compiled HyJCMSS and SSA in Debug mode. No optimization. Done!


.PHONY: clean
clean:
	rm $(srcdir)/*.o ; rm $(srcdir)/*.mod ; rm $(srcdir)/*.il

#List of Intel Declaration variables:
#Set with -D<Variable>

#If USING_MPI is defined, MPI is turned on (default is serial).
#If Adaptive is defined, adaptive time stepping is turned on (default is fixed).
#If Milstein is defined, Milstein method is used (default is Euler-Maruyama).

BenchMarkMaker:	f2kcli.f90 globalvariables.f90 dataio.f90 Program-BenchMarkMaker.f90
		$(F90) $(F90FLAGS) $(LIBS) f2kcli.f90 globalvariables.f90 ratelaws.f90 dataio.f90 Program-BenchMarkMaker.f90 \
		-o $(bindir)/BenchMarkMarker

HyJCMSS-ser-Fixed-EM:	f2kcli.f90 globalvariables.f90 ratelaws.f90 dataio.f90 randomgen.f90 priorityqueue.f90 \
			specialevents.f90 SDEintegration.f90 propagators-HyJCMSS.f90 mainprogram-HyJCMSS.f90
			$(F90) $(F90FLAGS) $(F90Fast) f2kcli.f90 globalvariables.f90 ratelaws.f90 dataio.f90 randomgen.f90 \
			priorityqueue.f90 specialevents.f90 SDEintegration.f90 propagators-HyJCMSS.f90 mainprogram-HyJCMSS.f90
			$(F90) $(F90FLAGS) $(LIBS) ipo_out.o -o $(bindir)/HyJCMSS-EM

HyJCMSS-MPI-Fixed-EM:	f2kcli.f90 globalvariables.f90 ratelaws.f90 dataio.f90 randomgen.f90 priorityqueue.f90 specialevents.f90 \
			SDEintegration.f90 propagators-HyJCMSS.f90 mainprogram-HyJCMSS.f90
			$(MPI) $(F90FLAGS) $(F90Fast) $(DEF)USING_MPI f2kcli.f90 globalvariables.f90 ratelaws.f90 dataio.f90 randomgen.f90 \
			priorityqueue.f90 specialevents.f90 SDEintegration.f90 propagators-HyJCMSS.f90 mainprogram-HyJCMSS.f90
			$(MPI) $(F90FLAGS) $(LIBS) ipo_out.o -o $(bindir)/HyJCMSS-MPI-EM

HyJCMSS-ser-Fixed-Mil:	f2kcli.f90 globalvariables.f90 ratelaws.f90 dataio.f90 randomgen.f90 priorityqueue.f90 specialevents.f90 \
			SDEintegration.f90 propagators-HyJCMSS.f90 mainprogram-HyJCMSS.f90
			$(F90) $(F90FLAGS) $(F90Fast) $(DEF)MILSTEIN f2kcli.f90 globalvariables.f90 ratelaws.f90 dataio.f90 randomgen.f90 \
			priorityqueue.f90 specialevents.f90 SDEintegration.f90 propagators-HyJCMSS.f90 mainprogram-HyJCMSS.f90
			$(F90) $(F90FLAGS) $(LIBS) ipo_out.o -o $(bindir)/HyJCMSS-Mil

HyJCMSS-MPI-Fixed-Mil:	f2kcli.f90 globalvariables.f90 ratelaws.f90 dataio.f90 randomgen.f90 priorityqueue.f90 specialevents.f90 \
			SDEintegration.f90 propagators-HyJCMSS.f90 mainprogram-HyJCMSS.f90
			$(MPI) $(F90FLAGS) $(F90Fast) $(DEF)USING_MPI $(DEF)MILSTEIN f2kcli.f90 globalvariables.f90 ratelaws.f90 \
			dataio.f90 randomgen.f90 priorityqueue.f90 specialevents.f90 SDEintegration.f90 propagators-HyJCMSS.f90 \
			mainprogram-HyJCMSS.f90
			$(MPI) $(F90FLAGS) $(LIBS) ipo_out.o -o $(bindir)/HyJCMSS-MPI-Mil

HyJCMSS-ser-Adapt-EM:	f2kcli.f90 globalvariables.f90 ratelaws.f90 dataio.f90 randomgen.f90 priorityqueue.f90 specialevents.f90 \
			SDEintegration.f90 propagators-HyJCMSS.f90 mainprogram-HyJCMSS.f90
			$(F90) $(F90FLAGS) $(F90Fast) $(DEF)ADAPTIVE f2kcli.f90 globalvariables.f90 ratelaws.f90 dataio.f90 randomgen.f90 \
			priorityqueue.f90 specialevents.f90 SDEintegration.f90 propagators-HyJCMSS.f90 mainprogram-HyJCMSS.f90
			$(F90) $(F90FLAGS) $(LIBS) ipo_out.o -o $(bindir)/HyJCMSS-EM-Adaptive

HyJCMSS-MPI-Adapt-EM:	f2kcli.f90 globalvariables.f90 ratelaws.f90 dataio.f90 randomgen.f90 priorityqueue.f90 specialevents.f90 \
			SDEintegration.f90 propagators-HyJCMSS.f90 mainprogram-HyJCMSS.f90
			$(MPI) $(F90FLAGS) $(F90Fast) $(DEF)ADAPTIVE $(DEF)USING_MPI f2kcli.f90 globalvariables.f90 \
			ratelaws.f90 dataio.f90 randomgen.f90 priorityqueue.f90 specialevents.f90 SDEintegration.f90 \
			propagators-HyJCMSS.f90 mainprogram-HyJCMSS.f90
			$(MPI) $(F90FLAGS) $(LIBS) ipo_out.o -o $(bindir)/HyJCMSS-MPI-EM-Adaptive

HyJCMSS-ser-Adapt-Mil:	f2kcli.f90 globalvariables.f90 ratelaws.f90 dataio.f90 randomgen.f90 priorityqueue.f90 specialevents.f90 \
			SDEintegration.f90 propagators-HyJCMSS.f90 mainprogram-HyJCMSS.f90
			$(F90) $(F90FLAGS) $(F90Fast) $(DEF)ADAPTIVE $(DEF)MILSTEIN f2kcli.f90 globalvariables.f90 \
			ratelaws.f90 dataio.f90 randomgen.f90 priorityqueue.f90 specialevents.f90 SDEintegration.f90 \
			propagators-HyJCMSS.f90 mainprogram-HyJCMSS.f90
			$(F90) $(F90FLAGS) $(LIBS) ipo_out.o -o $(bindir)/HyJCMSS-Mil-Adaptive

HyJCMSS-MPI-Adapt-Mil:	f2kcli.f90 globalvariables.f90 ratelaws.f90 dataio.f90 randomgen.f90 priorityqueue.f90 specialevents.f90 \
			SDEintegration.f90 propagators-HyJCMSS.f90 mainprogram-HyJCMSS.f90
			$(MPI) $(F90FLAGS) $(F90Fast) $(DEF)USING_MPI $(DEF)ADAPTIVE $(DEF)MILSTEIN f2kcli.f90 \
			globalvariables.f90 ratelaws.f90 dataio.f90 randomgen.f90 \
			priorityqueue.f90 specialevents.f90 SDEintegration.f90 propagators-HyJCMSS.f90 mainprogram-HyJCMSS.f90
			$(MPI) $(F90FLAGS) $(LIBS) ipo_out.o -o $(bindir)/HyJCMSS-MPI-Mil-Adaptive

SSA-MPI-opt:	  	f2kcli.f90 globalvariables.f90 ratelaws.f90 dataio.f90 randomgen.f90 priorityqueue.f90 specialevents.f90 \
			propagators-SSA.f90 mainprogram-SSA.f90
			$(MPI) $(F90FLAGS) $(F90Fast) $(DEF)USING_MPI f2kcli.f90 globalvariables.f90 ratelaws.f90 dataio.f90 randomgen.f90 \
			priorityqueue.f90 specialevents.f90 propagators-SSA.f90 mainprogram-SSA.f90
			$(MPI) $(F90FLAGS) $(LIBS) ipo_out.o -o $(bindir)/SSA-MPI

SSA-serial-opt:	  	f2kcli.f90 globalvariables.f90 ratelaws.f90 dataio.f90 randomgen.f90 priorityqueue.f90 specialevents.f90 \
			propagators-SSA.f90 mainprogram-SSA.f90
			$(F90) $(F90FLAGS) $(F90Fast) f2kcli.f90 globalvariables.f90 ratelaws.f90 dataio.f90 randomgen.f90 \
			priorityqueue.f90 specialevents.f90 propagators-SSA.f90 mainprogram-SSA.f90
			$(F90) $(F90FLAGS) $(LIBS) ipo_out.o -o $(bindir)/SSA-serial

                
HyJCMSS-Debug:		f2kcli.f90 globalvariables.f90 ratelaws.f90 dataio.f90 randomgen.f90 priorityqueue.f90 specialevents.f90 SDEintegration.f90 \
			propagators-HyJCMSS.f90 mainprogram-HyJCMSS.f90			
			$(F90) $(F90FLAGS) $(LIBS) \
			$(DEF)ADAPTIVE $(DEF)MILSTEIN \
			-g f2kcli.f90 globalvariables.f90 ratelaws.f90 dataio.f90 randomgen.f90 priorityqueue.f90 specialevents.f90 \
			SDEintegration.f90 propagators-HyJCMSS.f90 mainprogram-HyJCMSS.f90 -o $(bindir)/HyJCMSS-Debug
			
SSA-Debug:		f2kcli.f90 globalvariables.f90 ratelaws.f90 dataio.f90 randomgen.f90 priorityqueue.f90 specialevents.f90 \
			propagators-SSA.f90 mainprogram-SSA.f90
			$(F90) $(F90FLAGS) $(LIBS) -g f2kcli.f90 globalvariables.f90 ratelaws.f90 dataio.f90 randomgen.f90 \
			priorityqueue.f90 specialevents.f90 propagators-SSA.f90 mainprogram-SSA.f90 -o $(bindir)/SSA-Debug

HyJCMSS-MPI-Debug:	f2kcli.f90 globalvariables.f90 ratelaws.f90 dataio.f90 randomgen.f90 priorityqueue.f90 specialevents.f90 SDEintegration.f90 \
			propagators-HyJCMSS.f90 mainprogram-HyJCMSS.f90			
			$(MPI) $(F90FLAGS) $(LIBS) \
			$(DEF)USING_MPI $(DEF)MILSTEIN $(DEF)ADAPTIVE \
			-g f2kcli.f90 globalvariables.f90 ratelaws.f90 dataio.f90 randomgen.f90 priorityqueue.f90 specialevents.f90 \
			SDEintegration.f90 propagators-HyJCMSS.f90 mainprogram-HyJCMSS.f90 -o $(bindir)/HyJCMSS-MPI-Debug
			
SSA-MPI-Debug:		f2kcli.f90 globalvariables.f90 ratelaws.f90 dataio.f90 randomgen.f90 priorityqueue.f90 specialevents.f90 \
			propagators-SSA.f90 mainprogram-SSA.f90
			$(MPI) $(F90FLAGS) $(LIBS) \
			$(DEF)USING_MPI \
			-g f2kcli.f90 globalvariables.f90 ratelaws.f90 dataio.f90 randomgen.f90 \
			priorityqueue.f90 specialevents.f90 propagators-SSA.f90 mainprogram-SSA.f90 -o $(bindir)/SSA-MPI-Debug

