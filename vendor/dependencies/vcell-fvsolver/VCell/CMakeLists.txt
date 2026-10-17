project(VCell CXX Fortran)

if (NOT NEW_SMOLDYN)
	add_definitions(-DVCELL_HYBRID)
endif (NOT NEW_SMOLDYN)

set(HEADER_FILES 
	include/VCELL/AlgebraicSystem.h
	include/VCELL/CartesianMesh.h
#	include/VCELL/Contour.h
#	include/VCELL/ContourRegionVariable.h
#	include/VCELL/ContourSubdomain.h
#	include/VCELL/ContourVarContext.h
#	include/VCELL/ContourVariable.h
	include/VCELL/DataGenerator.h
	include/VCELL/DataSet.h
#	include/VCELL/DomainPDEScheduler.h
	include/VCELL/DoubleVector3.h
	include/VCELL/Element.h
	include/VCELL/EllipticVolumeEqnBuilder.h
	include/VCELL/EqnBuilder.h
	include/VCELL/EqnBuilderReactionForward.h
	include/VCELL/FastSystem.h
	include/VCELL/FastSystemExpression.h
	include/VCELL/Feature.h
	include/VCELL/FieldData.h
	include/VCELL/FVDataSet.h
	include/VCELL/FVSolver.h
	include/VCELL/FVUtils.h
	include/VCELL/GaussianConvolutionDataGenerator.h
#	include/VCELL/Histogram.h
	include/VCELL/IncidenceMatrix.h
	include/VCELL/JumpCondition.h
	include/VCELL/Membrane.h
	include/VCELL/MembraneEqnBuilderDiffusion.h
	include/VCELL/MembraneEqnBuilderForward.h
	include/VCELL/MembraneParticleVariable.h
	include/VCELL/MembraneRegion.h
	include/VCELL/MembraneRegionEqnBuilder.h
	include/VCELL/MembraneRegionVarContextExpression.h
	include/VCELL/MembraneRegionVariable.h
	include/VCELL/MembraneVarContextExpression.h
	include/VCELL/MembraneVariable.h
	include/VCELL/Mesh.h
	include/VCELL/ODESolver.h
#	include/VCELL/ParticleContext.h
#	include/VCELL/PDEScheduler.h
    include/VCELL/ParticleVariable.h
	include/VCELL/PDESolver.h
	include/VCELL/PostProcessingBlock.h
	include/VCELL/PostProcessingHdf5Writer.h
	include/VCELL/ProjectionDataGenerator.h
	include/VCELL/RandomVariable.h
	include/VCELL/Region.h
	include/VCELL/RegionSizeVariable.h
	include/VCELL/RoiDataGenerator.h
	include/VCELL/Scheduler.h
	include/VCELL/SerialScheduler.h
	include/VCELL/SimTool.h
	include/VCELL/SimTypes.h
	include/VCELL/Simulation.h
	include/VCELL/SimulationExpression.h
#	include/VCELL/SimulationMessaging.h
	include/VCELL/Solver.h
	include/VCELL/SolverMain.h
	include/VCELL/SparseLinearSolver.h
#	include/VCELL/SparseMatrix.h
	include/VCELL/SparseMatrixEqnBuilder.h
	include/VCELL/SparseMatrixPCG.h
	include/VCELL/SparseVolumeEqnBuilder.h
	include/VCELL/SplitScheduler.h
	include/VCELL/Structure.h
	include/VCELL/StructuredPDESolver.h
	include/VCELL/SundialsPdeScheduler.h
#	include/VCELL/TIFFImage.h
	include/VCELL/Timer.h
#	include/VCELL/TriDiagMatrix.h
	include/VCELL/VarContext.h
	include/VCELL/Variable.h
	include/VCELL/VariableStatisticsDataGenerator.h
	include/VCELL/VCellModel.h
	include/VCELL/VolumeParticleVariable.h
	include/VCELL/VolumeRegion.h
	include/VCELL/VolumeRegionEqnBuilder.h
	include/VCELL/VolumeRegionVarContextExpression.h
	include/VCELL/VolumeRegionVariable.h
	include/VCELL/VolumeVarContextExpression.h
	include/VCELL/VolumeVariable.h	
)

set(SRC_FILES 
	src/AlgebraicSystem.cpp
	src/CartesianMesh.cpp
#	src/Contour.cpp
#	src/ContourRegionVariable.cpp
#	src/ContourSubdomain.cpp
#	src/ContourVarContext.cpp
#	src/ContourVariable.cpp
	src/DataGenerator.cpp
	src/DataSet.cpp
#	src/DomainPDEScheduler.cpp
	src/EllipticVolumeEqnBuilder.cpp
	src/EqnBuilder.cpp
	src/EqnBuilderReactionForward.cpp
	src/FastSystem.cpp
	src/FastSystemExpression.cpp
	src/Feature.cpp
	src/FieldData.cpp
	src/FVDataSet.cpp
	src/FVSolver.cpp
	src/FVUtils.cpp
	src/GaussianConvolutionDataGenerator.cpp
#	src/Histogram.cpp
	src/JumpCondition.cpp
	src/Membrane.cpp
	src/MembraneEqnBuilderDiffusion.cpp
	src/MembraneEqnBuilderForward.cpp
	src/MembraneParticleVariable.cpp
	src/MembraneRegion.cpp
	src/MembraneRegionEqnBuilder.cpp
	src/MembraneRegionVarContextExpression.cpp
	src/MembraneRegionVariable.cpp
	src/MembraneVarContextExpression.cpp
	src/MembraneVariable.cpp
	src/Mesh.cpp
	src/ODESolver.cpp
#	src/ParticleContext.cpp
#	src/PDEScheduler.cpp
	src/ParticleVariable.cpp
	src/PDESolver.cpp
	src/PostProcessingBlock.cpp
	src/PostProcessingHdf5Writer.cpp
	src/ProjectionDataGenerator.cpp
	src/RandomVariable.cpp
	src/Region.cpp
	src/RegionSizeVariable.cpp
	src/RoiDataGenerator.cpp
	src/Scheduler.cpp
	src/SerialScheduler.cpp
	src/SimTool.cpp
	src/Simulation.cpp
	src/SimulationExpression.cpp
#	src/SimulationMessaging.cpp
	src/Solver.cpp
	src/SolverMain.cpp
	src/SparseLinearSolver.cpp
#	src/SparseMatrix.cpp
	src/SparseMatrixEqnBuilder.cpp
	src/SparseMatrixPCG.cpp
	src/SparseVolumeEqnBuilder.cpp
	src/SplitScheduler.cpp
	src/Structure.cpp
	src/StructuredPDESolver.cpp
	src/SundialsPdeScheduler.cpp
#	src/TIFFImage.cpp
	src/Timer.cpp
#	src/TriDiagMatrix.cpp
	src/VarContext.cpp
	src/Variable.cpp
	src/VariableStatisticsDataGenerator.cpp
	src/VCellModel.cpp
	src/VolumeParticleVariable.cpp
	src/VolumeRegion.cpp
	src/VolumeRegionEqnBuilder.cpp
	src/VolumeRegionVarContextExpression.cpp
	src/VolumeRegionVariable.cpp
	src/VolumeVarContextExpression.cpp
	src/VolumeVariable.cpp	
)

set (PCGWRAPPER_SRC_FILES
	src/pcgwrapper.f
)
if (APPLE)
	#set (CMAKE_Fortran_COMPILER gfortran)
	set (CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -m64")
endif()
 
set (EXE_SRC_FILES
	src/FiniteVolume.cpp
)
#include_directories(include ../ExpressionParser ${HDF5_BINARY_DIR} ../${HDF_VERSION}/c++/src ../${HDF_VERSION}/src ../IDAWin)
#include_directories(include ../bridgeVCellSmoldyn)
#if (NOT ZLIB_FOUND)
#	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../zlib)
#endif()


#include_directories(include ../${SMOL_DIR}/source/Smoldyn ../${SMOL_DIR}/source/lib)
#if (APPLE)
#	include_directories(../${SMOL_DIR}/mac)
#elseif (LINUX)
#	include_directories(../${SMOL_DIR}/source)
#endif()

if(UNIX)
	set (CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -static")
	if (APPLE) 
		add_definitions(-DMACOSX)
	else()
		if (LINUX_64bit_BINARIES AND NOT TARGET_SOLVERS_WITH_MESSAGING AND NOT MINGW)
		message("v linux64")
#			set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
		endif()	
	endif()
endif()	

#if (UNIX OR APPLE)
#	include(FindZLIB)
#	find_library(zlib REQUIRED)
#endif()

#
# http://floooh.github.io/2016/01/12/cmake-dependency-juggling.html
#
add_library(pcgwrapper STATIC ${PCGWRAPPER_SRC_FILES})
target_link_libraries(pcgwrapper PCGPack blas)

add_library(vcell STATIC ${SRC_FILES} ${HEADER_FILES})
target_link_libraries(vcell pcgwrapper sundials ExpressionParser smoldyn_static vcellsmoldynbridge ${HDF5_LIBRARIES} qhull vcellmessaging vcellziputils ${ZLIB_LIBRARIES})
target_include_directories(vcell PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  ${HDF5_INCLUDE_DIRS}
  $<INSTALL_INTERFACE:include>  # <prefix>/include
)

if (OPTION_TARGET_FV_SOLVER)
	set(EXE_FILE FiniteVolume)
	if (ARCH_64bit)
		set(EXE_FILE ${EXE_FILE}_x64)
	endif()

	if (APPLE)
		list(APPEND DEP_LIBS gfortran)
	endif()
	add_executable(${EXE_FILE} ${EXE_SRC_FILES})	
	target_link_libraries(${EXE_FILE} vcell)

	install(TARGETS ${EXE_FILE} 
            RUNTIME DESTINATION ${OPTION_EXE_DIRECTORY})
	
endif()

# add google tests
if (OPTION_TARGET_TESTS)
	enable_testing()
	if (MINGW)
		set(test_fvsolver_exe ${CMAKE_BINARY_DIR}/bin/${EXE_FILE}.exe)
		set(python_cmd py)
	else (MINGW)
		set(test_fvsolver_exe ${CMAKE_BINARY_DIR}/bin/${EXE_FILE})
		set(python_cmd python3)
	endif (MINGW)
	set(test_dir ${CMAKE_CURRENT_SOURCE_DIR}/tests/smoke)

	# smoke test as a python script, for bash test example, see NFsim/tests/smoke
	add_test(NAME ${EXE_FILE}_smoke COMMAND ${python_cmd} ${test_dir}/smoke.py ${test_fvsolver_exe} WORKING_DIRECTORY ${test_dir})

	add_subdirectory(tests)
endif()