#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc.exe
CCC=g++.exe
CXX=g++.exe
FC=gfortran.exe
AS=as.exe

# Macros
CND_PLATFORM=Cygwin_4.x_1-Windows
CND_DLIB_EXT=dll
CND_CONF=Release_Profile_3D
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/src/ChomboDomainBC.o \
	${OBJECTDIR}/src/ChomboEBBC.o \
	${OBJECTDIR}/src/ChomboGeometry.o \
	${OBJECTDIR}/src/ChomboGeometryShop.o \
	${OBJECTDIR}/src/ChomboIF.o \
	${OBJECTDIR}/src/ChomboRefinementRoi.o \
	${OBJECTDIR}/src/ChomboScheduler.o \
	${OBJECTDIR}/src/ChomboSemiImplicitScheduler.o \
	${OBJECTDIR}/src/ChomboSpec.o \
	${OBJECTDIR}/src/DataGenerator.o \
	${OBJECTDIR}/src/DataSet.o \
	${OBJECTDIR}/src/FVSolver.o \
	${OBJECTDIR}/src/Feature.o \
	${OBJECTDIR}/src/FiniteVolume.o \
	${OBJECTDIR}/src/JumpCondition.o \
	${OBJECTDIR}/src/Membrane.o \
	${OBJECTDIR}/src/MembraneRegionVarContextExpression.o \
	${OBJECTDIR}/src/MembraneRegionVariable.o \
	${OBJECTDIR}/src/MembraneVarContextExpression.o \
	${OBJECTDIR}/src/MembraneVariable.o \
	${OBJECTDIR}/src/PostProcessingBlock.o \
	${OBJECTDIR}/src/PostProcessingHdf5Writer.o \
	${OBJECTDIR}/src/SimTool.o \
	${OBJECTDIR}/src/SimulationExpression.o \
	${OBJECTDIR}/src/SimulationMessaging.o \
	${OBJECTDIR}/src/Structure.o \
	${OBJECTDIR}/src/VCellModel.o \
	${OBJECTDIR}/src/VarContext.o \
	${OBJECTDIR}/src/Variable.o \
	${OBJECTDIR}/src/VariableStatisticsDataGenerator.o \
	${OBJECTDIR}/src/VolumeRegionVarContextExpression.o \
	${OBJECTDIR}/src/VolumeRegionVariable.o \
	${OBJECTDIR}/src/VolumeVarContextExpression.o \
	${OBJECTDIR}/src/VolumeVariable.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-L../Chombo/lib -L../numerics/cmake-build-cygwin/bin -L../numerics/cmake-build-cygwin/lib -lmftools3d.CYGWIN.64.g++.gfortran.OPT -lebamrelliptic3d.CYGWIN.64.g++.gfortran.OPT -lebamrtools3d.CYGWIN.64.g++.gfortran.OPT -lebtools3d.CYGWIN.64.g++.gfortran.OPT -lamrelliptic3d.CYGWIN.64.g++.gfortran.OPT -lamrtools3d.CYGWIN.64.g++.gfortran.OPT -lworkshop3d.CYGWIN.64.g++.gfortran.OPT -lboxtools3d.CYGWIN.64.g++.gfortran.OPT -lbasetools3d.CYGWIN.64.g++.gfortran.OPT -lhdf5_cpp -lhdf5 -lExpressionParser -lunzip -lzip

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/vcellchombo_parallel.exe

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/vcellchombo_parallel.exe: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/vcellchombo_parallel ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/src/ChomboDomainBC.o: src/ChomboDomainBC.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -DCH_CYGWIN -DCH_FORT_UNDERSCORE -DCH_LANG_CC -DCH_SPACEDIM=3 -DCH_USE_64 -DCH_USE_COMPLEX -DCH_USE_DOUBLE -DCH_USE_HDF5 -DCH_USE_MEMORY_TRACKING -DCH_USE_SETVAL -DH5_USE_16_API -I../Chombo/lib/include -I../numerics/ExpressionParser -Iinclude -I../numerics/hdf5-1.8.8/src -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ChomboDomainBC.o src/ChomboDomainBC.cpp

${OBJECTDIR}/src/ChomboEBBC.o: src/ChomboEBBC.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -DCH_CYGWIN -DCH_FORT_UNDERSCORE -DCH_LANG_CC -DCH_SPACEDIM=3 -DCH_USE_64 -DCH_USE_COMPLEX -DCH_USE_DOUBLE -DCH_USE_HDF5 -DCH_USE_MEMORY_TRACKING -DCH_USE_SETVAL -DH5_USE_16_API -I../Chombo/lib/include -I../numerics/ExpressionParser -Iinclude -I../numerics/hdf5-1.8.8/src -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ChomboEBBC.o src/ChomboEBBC.cpp

${OBJECTDIR}/src/ChomboGeometry.o: src/ChomboGeometry.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -DCH_CYGWIN -DCH_FORT_UNDERSCORE -DCH_LANG_CC -DCH_SPACEDIM=3 -DCH_USE_64 -DCH_USE_COMPLEX -DCH_USE_DOUBLE -DCH_USE_HDF5 -DCH_USE_MEMORY_TRACKING -DCH_USE_SETVAL -DH5_USE_16_API -I../Chombo/lib/include -I../numerics/ExpressionParser -Iinclude -I../numerics/hdf5-1.8.8/src -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ChomboGeometry.o src/ChomboGeometry.cpp

${OBJECTDIR}/src/ChomboGeometryShop.o: src/ChomboGeometryShop.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -DCH_CYGWIN -DCH_FORT_UNDERSCORE -DCH_LANG_CC -DCH_SPACEDIM=3 -DCH_USE_64 -DCH_USE_COMPLEX -DCH_USE_DOUBLE -DCH_USE_HDF5 -DCH_USE_MEMORY_TRACKING -DCH_USE_SETVAL -DH5_USE_16_API -I../Chombo/lib/include -I../numerics/ExpressionParser -Iinclude -I../numerics/hdf5-1.8.8/src -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ChomboGeometryShop.o src/ChomboGeometryShop.cpp

${OBJECTDIR}/src/ChomboIF.o: src/ChomboIF.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -DCH_CYGWIN -DCH_FORT_UNDERSCORE -DCH_LANG_CC -DCH_SPACEDIM=3 -DCH_USE_64 -DCH_USE_COMPLEX -DCH_USE_DOUBLE -DCH_USE_HDF5 -DCH_USE_MEMORY_TRACKING -DCH_USE_SETVAL -DH5_USE_16_API -I../Chombo/lib/include -I../numerics/ExpressionParser -Iinclude -I../numerics/hdf5-1.8.8/src -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ChomboIF.o src/ChomboIF.cpp

${OBJECTDIR}/src/ChomboRefinementRoi.o: src/ChomboRefinementRoi.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -DCH_CYGWIN -DCH_FORT_UNDERSCORE -DCH_LANG_CC -DCH_SPACEDIM=3 -DCH_USE_64 -DCH_USE_COMPLEX -DCH_USE_DOUBLE -DCH_USE_HDF5 -DCH_USE_MEMORY_TRACKING -DCH_USE_SETVAL -DH5_USE_16_API -I../Chombo/lib/include -I../numerics/ExpressionParser -Iinclude -I../numerics/hdf5-1.8.8/src -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ChomboRefinementRoi.o src/ChomboRefinementRoi.cpp

${OBJECTDIR}/src/ChomboScheduler.o: src/ChomboScheduler.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -DCH_CYGWIN -DCH_FORT_UNDERSCORE -DCH_LANG_CC -DCH_SPACEDIM=3 -DCH_USE_64 -DCH_USE_COMPLEX -DCH_USE_DOUBLE -DCH_USE_HDF5 -DCH_USE_MEMORY_TRACKING -DCH_USE_SETVAL -DH5_USE_16_API -I../Chombo/lib/include -I../numerics/ExpressionParser -Iinclude -I../numerics/hdf5-1.8.8/src -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ChomboScheduler.o src/ChomboScheduler.cpp

${OBJECTDIR}/src/ChomboSemiImplicitScheduler.o: src/ChomboSemiImplicitScheduler.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -DCH_CYGWIN -DCH_FORT_UNDERSCORE -DCH_LANG_CC -DCH_SPACEDIM=3 -DCH_USE_64 -DCH_USE_COMPLEX -DCH_USE_DOUBLE -DCH_USE_HDF5 -DCH_USE_MEMORY_TRACKING -DCH_USE_SETVAL -DH5_USE_16_API -I../Chombo/lib/include -I../numerics/ExpressionParser -Iinclude -I../numerics/hdf5-1.8.8/src -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ChomboSemiImplicitScheduler.o src/ChomboSemiImplicitScheduler.cpp

${OBJECTDIR}/src/ChomboSpec.o: src/ChomboSpec.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -DCH_CYGWIN -DCH_FORT_UNDERSCORE -DCH_LANG_CC -DCH_SPACEDIM=3 -DCH_USE_64 -DCH_USE_COMPLEX -DCH_USE_DOUBLE -DCH_USE_HDF5 -DCH_USE_MEMORY_TRACKING -DCH_USE_SETVAL -DH5_USE_16_API -I../Chombo/lib/include -I../numerics/ExpressionParser -Iinclude -I../numerics/hdf5-1.8.8/src -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ChomboSpec.o src/ChomboSpec.cpp

${OBJECTDIR}/src/DataGenerator.o: src/DataGenerator.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -DCH_CYGWIN -DCH_FORT_UNDERSCORE -DCH_LANG_CC -DCH_SPACEDIM=3 -DCH_USE_64 -DCH_USE_COMPLEX -DCH_USE_DOUBLE -DCH_USE_HDF5 -DCH_USE_MEMORY_TRACKING -DCH_USE_SETVAL -DH5_USE_16_API -I../Chombo/lib/include -I../numerics/ExpressionParser -Iinclude -I../numerics/hdf5-1.8.8/src -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/DataGenerator.o src/DataGenerator.cpp

${OBJECTDIR}/src/DataSet.o: src/DataSet.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -DCH_CYGWIN -DCH_FORT_UNDERSCORE -DCH_LANG_CC -DCH_SPACEDIM=3 -DCH_USE_64 -DCH_USE_COMPLEX -DCH_USE_DOUBLE -DCH_USE_HDF5 -DCH_USE_MEMORY_TRACKING -DCH_USE_SETVAL -DH5_USE_16_API -I../Chombo/lib/include -I../numerics/ExpressionParser -Iinclude -I../numerics/hdf5-1.8.8/src -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/DataSet.o src/DataSet.cpp

${OBJECTDIR}/src/FVSolver.o: src/FVSolver.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -DCH_CYGWIN -DCH_FORT_UNDERSCORE -DCH_LANG_CC -DCH_SPACEDIM=3 -DCH_USE_64 -DCH_USE_COMPLEX -DCH_USE_DOUBLE -DCH_USE_HDF5 -DCH_USE_MEMORY_TRACKING -DCH_USE_SETVAL -DH5_USE_16_API -I../Chombo/lib/include -I../numerics/ExpressionParser -Iinclude -I../numerics/hdf5-1.8.8/src -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/FVSolver.o src/FVSolver.cpp

${OBJECTDIR}/src/Feature.o: src/Feature.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -DCH_CYGWIN -DCH_FORT_UNDERSCORE -DCH_LANG_CC -DCH_SPACEDIM=3 -DCH_USE_64 -DCH_USE_COMPLEX -DCH_USE_DOUBLE -DCH_USE_HDF5 -DCH_USE_MEMORY_TRACKING -DCH_USE_SETVAL -DH5_USE_16_API -I../Chombo/lib/include -I../numerics/ExpressionParser -Iinclude -I../numerics/hdf5-1.8.8/src -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/Feature.o src/Feature.cpp

${OBJECTDIR}/src/FiniteVolume.o: src/FiniteVolume.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -DCH_CYGWIN -DCH_FORT_UNDERSCORE -DCH_LANG_CC -DCH_SPACEDIM=3 -DCH_USE_64 -DCH_USE_COMPLEX -DCH_USE_DOUBLE -DCH_USE_HDF5 -DCH_USE_MEMORY_TRACKING -DCH_USE_SETVAL -DH5_USE_16_API -I../Chombo/lib/include -I../numerics/ExpressionParser -Iinclude -I../numerics/hdf5-1.8.8/src -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/FiniteVolume.o src/FiniteVolume.cpp

${OBJECTDIR}/src/JumpCondition.o: src/JumpCondition.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -DCH_CYGWIN -DCH_FORT_UNDERSCORE -DCH_LANG_CC -DCH_SPACEDIM=3 -DCH_USE_64 -DCH_USE_COMPLEX -DCH_USE_DOUBLE -DCH_USE_HDF5 -DCH_USE_MEMORY_TRACKING -DCH_USE_SETVAL -DH5_USE_16_API -I../Chombo/lib/include -I../numerics/ExpressionParser -Iinclude -I../numerics/hdf5-1.8.8/src -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/JumpCondition.o src/JumpCondition.cpp

${OBJECTDIR}/src/Membrane.o: src/Membrane.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -DCH_CYGWIN -DCH_FORT_UNDERSCORE -DCH_LANG_CC -DCH_SPACEDIM=3 -DCH_USE_64 -DCH_USE_COMPLEX -DCH_USE_DOUBLE -DCH_USE_HDF5 -DCH_USE_MEMORY_TRACKING -DCH_USE_SETVAL -DH5_USE_16_API -I../Chombo/lib/include -I../numerics/ExpressionParser -Iinclude -I../numerics/hdf5-1.8.8/src -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/Membrane.o src/Membrane.cpp

${OBJECTDIR}/src/MembraneRegionVarContextExpression.o: src/MembraneRegionVarContextExpression.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -DCH_CYGWIN -DCH_FORT_UNDERSCORE -DCH_LANG_CC -DCH_SPACEDIM=3 -DCH_USE_64 -DCH_USE_COMPLEX -DCH_USE_DOUBLE -DCH_USE_HDF5 -DCH_USE_MEMORY_TRACKING -DCH_USE_SETVAL -DH5_USE_16_API -I../Chombo/lib/include -I../numerics/ExpressionParser -Iinclude -I../numerics/hdf5-1.8.8/src -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/MembraneRegionVarContextExpression.o src/MembraneRegionVarContextExpression.cpp

${OBJECTDIR}/src/MembraneRegionVariable.o: src/MembraneRegionVariable.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -DCH_CYGWIN -DCH_FORT_UNDERSCORE -DCH_LANG_CC -DCH_SPACEDIM=3 -DCH_USE_64 -DCH_USE_COMPLEX -DCH_USE_DOUBLE -DCH_USE_HDF5 -DCH_USE_MEMORY_TRACKING -DCH_USE_SETVAL -DH5_USE_16_API -I../Chombo/lib/include -I../numerics/ExpressionParser -Iinclude -I../numerics/hdf5-1.8.8/src -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/MembraneRegionVariable.o src/MembraneRegionVariable.cpp

${OBJECTDIR}/src/MembraneVarContextExpression.o: src/MembraneVarContextExpression.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -DCH_CYGWIN -DCH_FORT_UNDERSCORE -DCH_LANG_CC -DCH_SPACEDIM=3 -DCH_USE_64 -DCH_USE_COMPLEX -DCH_USE_DOUBLE -DCH_USE_HDF5 -DCH_USE_MEMORY_TRACKING -DCH_USE_SETVAL -DH5_USE_16_API -I../Chombo/lib/include -I../numerics/ExpressionParser -Iinclude -I../numerics/hdf5-1.8.8/src -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/MembraneVarContextExpression.o src/MembraneVarContextExpression.cpp

${OBJECTDIR}/src/MembraneVariable.o: src/MembraneVariable.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -DCH_CYGWIN -DCH_FORT_UNDERSCORE -DCH_LANG_CC -DCH_SPACEDIM=3 -DCH_USE_64 -DCH_USE_COMPLEX -DCH_USE_DOUBLE -DCH_USE_HDF5 -DCH_USE_MEMORY_TRACKING -DCH_USE_SETVAL -DH5_USE_16_API -I../Chombo/lib/include -I../numerics/ExpressionParser -Iinclude -I../numerics/hdf5-1.8.8/src -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/MembraneVariable.o src/MembraneVariable.cpp

${OBJECTDIR}/src/PostProcessingBlock.o: src/PostProcessingBlock.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -DCH_CYGWIN -DCH_FORT_UNDERSCORE -DCH_LANG_CC -DCH_SPACEDIM=3 -DCH_USE_64 -DCH_USE_COMPLEX -DCH_USE_DOUBLE -DCH_USE_HDF5 -DCH_USE_MEMORY_TRACKING -DCH_USE_SETVAL -DH5_USE_16_API -I../Chombo/lib/include -I../numerics/ExpressionParser -Iinclude -I../numerics/hdf5-1.8.8/src -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/PostProcessingBlock.o src/PostProcessingBlock.cpp

${OBJECTDIR}/src/PostProcessingHdf5Writer.o: src/PostProcessingHdf5Writer.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -DCH_CYGWIN -DCH_FORT_UNDERSCORE -DCH_LANG_CC -DCH_SPACEDIM=3 -DCH_USE_64 -DCH_USE_COMPLEX -DCH_USE_DOUBLE -DCH_USE_HDF5 -DCH_USE_MEMORY_TRACKING -DCH_USE_SETVAL -DH5_USE_16_API -I../Chombo/lib/include -I../numerics/ExpressionParser -Iinclude -I../numerics/hdf5-1.8.8/src -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/PostProcessingHdf5Writer.o src/PostProcessingHdf5Writer.cpp

${OBJECTDIR}/src/SimTool.o: src/SimTool.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -DCH_CYGWIN -DCH_FORT_UNDERSCORE -DCH_LANG_CC -DCH_SPACEDIM=3 -DCH_USE_64 -DCH_USE_COMPLEX -DCH_USE_DOUBLE -DCH_USE_HDF5 -DCH_USE_MEMORY_TRACKING -DCH_USE_SETVAL -DH5_USE_16_API -I../Chombo/lib/include -I../numerics/ExpressionParser -Iinclude -I../numerics/hdf5-1.8.8/src -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/SimTool.o src/SimTool.cpp

${OBJECTDIR}/src/SimulationExpression.o: src/SimulationExpression.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -DCH_CYGWIN -DCH_FORT_UNDERSCORE -DCH_LANG_CC -DCH_SPACEDIM=3 -DCH_USE_64 -DCH_USE_COMPLEX -DCH_USE_DOUBLE -DCH_USE_HDF5 -DCH_USE_MEMORY_TRACKING -DCH_USE_SETVAL -DH5_USE_16_API -I../Chombo/lib/include -I../numerics/ExpressionParser -Iinclude -I../numerics/hdf5-1.8.8/src -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/SimulationExpression.o src/SimulationExpression.cpp

${OBJECTDIR}/src/SimulationMessaging.o: src/SimulationMessaging.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -DCH_CYGWIN -DCH_FORT_UNDERSCORE -DCH_LANG_CC -DCH_SPACEDIM=3 -DCH_USE_64 -DCH_USE_COMPLEX -DCH_USE_DOUBLE -DCH_USE_HDF5 -DCH_USE_MEMORY_TRACKING -DCH_USE_SETVAL -DH5_USE_16_API -I../Chombo/lib/include -I../numerics/ExpressionParser -Iinclude -I../numerics/hdf5-1.8.8/src -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/SimulationMessaging.o src/SimulationMessaging.cpp

${OBJECTDIR}/src/Structure.o: src/Structure.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -DCH_CYGWIN -DCH_FORT_UNDERSCORE -DCH_LANG_CC -DCH_SPACEDIM=3 -DCH_USE_64 -DCH_USE_COMPLEX -DCH_USE_DOUBLE -DCH_USE_HDF5 -DCH_USE_MEMORY_TRACKING -DCH_USE_SETVAL -DH5_USE_16_API -I../Chombo/lib/include -I../numerics/ExpressionParser -Iinclude -I../numerics/hdf5-1.8.8/src -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/Structure.o src/Structure.cpp

${OBJECTDIR}/src/VCellModel.o: src/VCellModel.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -DCH_CYGWIN -DCH_FORT_UNDERSCORE -DCH_LANG_CC -DCH_SPACEDIM=3 -DCH_USE_64 -DCH_USE_COMPLEX -DCH_USE_DOUBLE -DCH_USE_HDF5 -DCH_USE_MEMORY_TRACKING -DCH_USE_SETVAL -DH5_USE_16_API -I../Chombo/lib/include -I../numerics/ExpressionParser -Iinclude -I../numerics/hdf5-1.8.8/src -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/VCellModel.o src/VCellModel.cpp

${OBJECTDIR}/src/VarContext.o: src/VarContext.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -DCH_CYGWIN -DCH_FORT_UNDERSCORE -DCH_LANG_CC -DCH_SPACEDIM=3 -DCH_USE_64 -DCH_USE_COMPLEX -DCH_USE_DOUBLE -DCH_USE_HDF5 -DCH_USE_MEMORY_TRACKING -DCH_USE_SETVAL -DH5_USE_16_API -I../Chombo/lib/include -I../numerics/ExpressionParser -Iinclude -I../numerics/hdf5-1.8.8/src -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/VarContext.o src/VarContext.cpp

${OBJECTDIR}/src/Variable.o: src/Variable.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -DCH_CYGWIN -DCH_FORT_UNDERSCORE -DCH_LANG_CC -DCH_SPACEDIM=3 -DCH_USE_64 -DCH_USE_COMPLEX -DCH_USE_DOUBLE -DCH_USE_HDF5 -DCH_USE_MEMORY_TRACKING -DCH_USE_SETVAL -DH5_USE_16_API -I../Chombo/lib/include -I../numerics/ExpressionParser -Iinclude -I../numerics/hdf5-1.8.8/src -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/Variable.o src/Variable.cpp

${OBJECTDIR}/src/VariableStatisticsDataGenerator.o: src/VariableStatisticsDataGenerator.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -DCH_CYGWIN -DCH_FORT_UNDERSCORE -DCH_LANG_CC -DCH_SPACEDIM=3 -DCH_USE_64 -DCH_USE_COMPLEX -DCH_USE_DOUBLE -DCH_USE_HDF5 -DCH_USE_MEMORY_TRACKING -DCH_USE_SETVAL -DH5_USE_16_API -I../Chombo/lib/include -I../numerics/ExpressionParser -Iinclude -I../numerics/hdf5-1.8.8/src -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/VariableStatisticsDataGenerator.o src/VariableStatisticsDataGenerator.cpp

${OBJECTDIR}/src/VolumeRegionVarContextExpression.o: src/VolumeRegionVarContextExpression.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -DCH_CYGWIN -DCH_FORT_UNDERSCORE -DCH_LANG_CC -DCH_SPACEDIM=3 -DCH_USE_64 -DCH_USE_COMPLEX -DCH_USE_DOUBLE -DCH_USE_HDF5 -DCH_USE_MEMORY_TRACKING -DCH_USE_SETVAL -DH5_USE_16_API -I../Chombo/lib/include -I../numerics/ExpressionParser -Iinclude -I../numerics/hdf5-1.8.8/src -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/VolumeRegionVarContextExpression.o src/VolumeRegionVarContextExpression.cpp

${OBJECTDIR}/src/VolumeRegionVariable.o: src/VolumeRegionVariable.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -DCH_CYGWIN -DCH_FORT_UNDERSCORE -DCH_LANG_CC -DCH_SPACEDIM=3 -DCH_USE_64 -DCH_USE_COMPLEX -DCH_USE_DOUBLE -DCH_USE_HDF5 -DCH_USE_MEMORY_TRACKING -DCH_USE_SETVAL -DH5_USE_16_API -I../Chombo/lib/include -I../numerics/ExpressionParser -Iinclude -I../numerics/hdf5-1.8.8/src -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/VolumeRegionVariable.o src/VolumeRegionVariable.cpp

${OBJECTDIR}/src/VolumeVarContextExpression.o: src/VolumeVarContextExpression.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -DCH_CYGWIN -DCH_FORT_UNDERSCORE -DCH_LANG_CC -DCH_SPACEDIM=3 -DCH_USE_64 -DCH_USE_COMPLEX -DCH_USE_DOUBLE -DCH_USE_HDF5 -DCH_USE_MEMORY_TRACKING -DCH_USE_SETVAL -DH5_USE_16_API -I../Chombo/lib/include -I../numerics/ExpressionParser -Iinclude -I../numerics/hdf5-1.8.8/src -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/VolumeVarContextExpression.o src/VolumeVarContextExpression.cpp

${OBJECTDIR}/src/VolumeVariable.o: src/VolumeVariable.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -DCH_CYGWIN -DCH_FORT_UNDERSCORE -DCH_LANG_CC -DCH_SPACEDIM=3 -DCH_USE_64 -DCH_USE_COMPLEX -DCH_USE_DOUBLE -DCH_USE_HDF5 -DCH_USE_MEMORY_TRACKING -DCH_USE_SETVAL -DH5_USE_16_API -I../Chombo/lib/include -I../numerics/ExpressionParser -Iinclude -I../numerics/hdf5-1.8.8/src -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/VolumeVariable.o src/VolumeVariable.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/vcellchombo_parallel.exe

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
