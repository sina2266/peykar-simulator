project(matlabUtil)


file(GLOB SRC_FILES *cpp) 

file(GLOB HEADER_FILES *h) 

set_source_files_properties( ${SRC_FILES} PROPERTIES LANGUAGE CXX)
#include_directories(include 
#	../ccamHDF5
#	../ExpressionParser)
#include_directories(SYSTEM 
#	../FronTierLib 
#	../FronTierLib/util 
#	../boost_1_53_0
#	${CMAKE_BINARY_DIR}/${HDF_VERSION} 
#	../${HDF_VERSION}/src 
#	../${HDF_VERSION}/c++/src)
#
add_library(matlabUtilLib ${SRC_FILES} ${HEADER_FILES} ${BRIDGE_HEADER_FILES})
#add_dependencies(MovingBoundaryLib FronTierLib ExpressionParser)

install(TARGETS matlabUtilLib 
	ARCHIVE DESTINATION lib)
