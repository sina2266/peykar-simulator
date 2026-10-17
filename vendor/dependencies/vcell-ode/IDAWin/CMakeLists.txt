project(IDAWin)

set (SRC_FILES
		SundialsSolverInterface.cpp
		OdeResultSet.cpp
		StoppedByUserException.cpp
		VCellCVodeSolver.cpp
		VCellIDASolver.cpp
		VCellSundialsSolver.cpp
		SteadyStateDriver.cpp
		VCellSolverFactory.cpp
		VCellSolverInput.cpp
)
set (HEADER_FILES 
		OdeResultSet.h
		StoppedByUserException.h
		VCellSolverFactory.h
		VCellSolver.h
		VCellSolverInput.h
		VCellSundialsSolver.h
		VCellCVodeSolver.h
		VCellIDASolver.h
        SteadyStateDriver.h
		SundialsSolverInterface.h
)

set (EXE_SRC_FILES
	SundialsSolverStandalone.cpp
)


if (OPTION_STATICALLY_LINK AND OPTION_TARGET_PYTHON_BINDING)
	message("Creating Dynamic IDAWin with statically linked dependencies (for python)")
	add_library(IDAWin SHARED ${SRC_FILES} ${HEADER_FILES})
	target_link_libraries(
		IDAWin
		sundials -static
		vcellexpressionparser -static
		vcellmessaging -static
		argparse -static
		spdlog::spdlog -static
		# if MinGW is enabled, add `$<$<BOOL:${MINGW}>:ws2_32>`
	)
elseif(OPTION_STATICALLY_LINK AND NOT OPTION_TARGET_PYTHON_BINDING)
	message("Creating IDAWin for static linkage")
	add_library(IDAWin STATIC ${SRC_FILES} ${HEADER_FILES})
	target_link_libraries(
			IDAWin
			sundials -static
			vcellexpressionparser -static
			vcellmessaging -static
			argparse -static
			spdlog::spdlog -static
			# if MinGW is enabled, add `$<$<BOOL:${MINGW}>:ws2_32>`
	)
else ()
	message("Creating IDAWin for dynamic linkage.")
	add_library(IDAWin SHARED ${SRC_FILES} ${HEADER_FILES})
	target_link_libraries(
			IDAWin
			sundials
			vcellexpressionparser
			vcellmessaging
			argparse
			spdlog::spdlog
			# if MinGW is enabled, add `$<$<BOOL:${MINGW}>:ws2_32>`
	)
endif ()

set(EXE_FILE SundialsSolverStandalone)
if (ARCH_64bit) 
	set(EXE_FILE ${EXE_FILE}_x64)
endif()

target_include_directories(IDAWin PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(IDAWin PRIVATE IDAWIN_BUILDING_DLL)
add_executable(${EXE_FILE} ${EXE_SRC_FILES})	
target_link_libraries(${EXE_FILE} IDAWin)

install(TARGETS ${EXE_FILE} RUNTIME DESTINATION ${OPTION_EXE_DIRECTORY})
install(TARGETS IDAWin ARCHIVE DESTINATION bin)