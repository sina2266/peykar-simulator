project(VCellChombo)

set(HEADER_FILES
		include/VCELL/AlgebraicSystem.h
		include/VCELL/ChomboDomainBC.h
		include/VCELL/ChomboEBBC.h
		include/VCELL/ChomboGeometry.h
		include/VCELL/ChomboGeometryShop.h
		include/VCELL/ChomboIF.h
		include/VCELL/ChomboRefinementRoi.h
		include/VCELL/ChomboScheduler.h
		include/VCELL/ChomboSemiImplicitScheduler.h
		include/VCELL/ChomboSpec.h
		include/VCELL/ConnectedComponent.h
		include/VCELL/DataGenerator.h
		include/VCELL/DataSet.h
		include/VCELL/FastSystemExpression.h
		include/VCELL/Feature.h
		include/VCELL/FVSolver.h
		include/VCELL/JumpCondition.h
		include/VCELL/Membrane.h
		include/VCELL/MembraneRegionVarContextExpression.h
		include/VCELL/MembraneRegionVariable.h
		include/VCELL/MembraneVarContextExpression.h
		include/VCELL/MembraneVariable.h
		include/VCELL/PostProcessingBlock.h
		include/VCELL/PostProcessingHdf5Writer.h
		include/VCELL/SimTool.h
		include/VCELL/SimulationExpression.h
		include/VCELL/Structure.h
		include/VCELL/VarContext.h
		include/VCELL/Variable.h
		include/VCELL/VariableStatisticsDataGenerator.h
		include/VCELL/VCellModel.h
		include/VCELL/VolumeRegionVarContextExpression.h
		include/VCELL/VolumeRegionVariable.h
		include/VCELL/VolumeVarContextExpression.h
		include/VCELL/VolumeVariable.h
		include/VCELL/ZipUtils.h
)

set(SRC_FILES
		src/AlgebraicSystem.cpp
		src/ChomboDomainBC.cpp
		src/ChomboEBBC.cpp
		src/ChomboGeometry.cpp
		src/ChomboGeometryShop.cpp
		src/ChomboIF.cpp
		src/ChomboRefinementRoi.cpp
		src/ChomboScheduler.cpp
		src/ChomboSemiImplicitScheduler.cpp
		src/ChomboSpec.cpp
		src/DataGenerator.cpp
		src/DataSet.cpp
		src/FastSystemExpression.cpp
		src/Feature.cpp
		src/FiniteVolume.cpp
		src/FVSolver.cpp
		src/JumpCondition.cpp
		src/Membrane.cpp
		src/MembraneRegionVarContextExpression.cpp
		src/MembraneRegionVariable.cpp
		src/MembraneVarContextExpression.cpp
		src/MembraneVariable.cpp
		src/PostProcessingBlock.cpp
		src/PostProcessingHdf5Writer.cpp
		src/SimTool.cpp
		src/SimulationExpression.cpp
		src/Structure.cpp
		src/VarContext.cpp
		src/Variable.cpp
		src/VariableStatisticsDataGenerator.cpp
		src/VCellModel.cpp
		src/VolumeRegionVarContextExpression.cpp
		src/VolumeRegionVariable.cpp
		src/VolumeVarContextExpression.cpp
		src/VolumeVariable.cpp
		src/ZipUtils.cpp
)

# src/baseIVFABExachange.cpp is a copy of a Chombo source kept for reference; it
# was never part of the build.

##
# add_vcell_chombo_solver(<dim>)
#
# One executable per CH_SPACEDIM, built from the same sources against that
# dimension's Chombo libraries.
##
function(add_vcell_chombo_solver DIM)
	if (OPTION_TARGET_PARALLEL)
		set(exe VCellChombo${DIM}D_x64parallel)
	else ()
		set(exe VCellChombo${DIM}D_x64)
	endif ()

	add_executable(${exe} ${SRC_FILES} ${HEADER_FILES})
	add_dependencies(${exe} chombo_${DIM}d)

	target_include_directories(${exe} PRIVATE
			${CMAKE_CURRENT_SOURCE_DIR}/include
			${CHOMBO_${DIM}D_INCLUDE_DIR})

	target_compile_definitions(${exe} PRIVATE
			CH_SPACEDIM=${DIM}
			# Must match how the Chombo libraries themselves were compiled --
			# several of these change struct layouts and inline function bodies in
			# the Chombo headers, so a mismatch is an ODR violation, not a warning.
			CH_LANG_CC
			CH_FORT_UNDERSCORE
			CH_USE_64
			CH_USE_COMPLEX
			CH_USE_DOUBLE
			CH_USE_HDF5
			CH_USE_SETVAL
			H5_USE_16_API)

	if (LINUX)
		target_compile_definitions(${exe} PRIVATE CH_Linux CH_USE_MEMORY_TRACKING)
	elseif (APPLE)
		# Chombo's memory tracking uses mallinfo(), which macOS does not have.
		target_compile_definitions(${exe} PRIVATE CH_Darwin)
	endif ()

	if (OPTION_TARGET_PARALLEL)
		target_compile_definitions(${exe} PRIVATE CH_MPI)
		target_link_libraries(${exe} PRIVATE MPI::MPI_CXX)
	endif ()

	target_link_libraries(${exe} PRIVATE
			${CHOMBO_${DIM}D_LIBRARIES}
			vcellexpressionparser
			vcellmessaging
			libzip::zip
			HDF5::HDF5
			ZLIB::ZLIB)

	# Chombo is part Fortran; the C++ driver has to pull in the Fortran runtime.
	if (CMAKE_Fortran_COMPILER_ID STREQUAL "GNU")
		target_link_libraries(${exe} PRIVATE gfortran)
	endif ()

	install(TARGETS ${exe} RUNTIME DESTINATION bin)
endfunction()

if (OPTION_TARGET_CHOMBO2D_SOLVER)
	add_vcell_chombo_solver(2)
endif ()
if (OPTION_TARGET_CHOMBO3D_SOLVER)
	add_vcell_chombo_solver(3)
endif ()
