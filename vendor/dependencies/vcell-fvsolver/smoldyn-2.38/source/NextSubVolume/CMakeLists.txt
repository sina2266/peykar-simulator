set(KAIROS_SRC_FILES
					StructuredGrid.cpp
					Species.cpp
					ReactionEquation.cpp
					NextSubvolumeMethod.cpp
					Kairos.cpp
					nsv_Io.cpp
					Geometry.cpp
#					../lib/math2.c
#					../lib/random2.c
#					../lib/SFMT/SFMT.c
					)
set(KAIROS_HEADER_FILES Vector.h
						StructuredGrid.h
						Species.h
						ReactionEquation.h
						NextSubvolumeMethod.h
						Log.h
						Kairos.h
						nsv_Io.h
						Geometry.h
						Constants.h
#						../lib/math2.h
#						../lib/random2.h
#						../lib/SFMT/SFMT.h
					        )
						
set(WRAPPER_SRC nsvc.cpp)
set(WRAPPER_HEADER nsvc.h)						

if(NOT HAVE_VTK)
	add_definitions(-DHAVE_VTK)
	find_package(VTK REQUIRED)
endif()

#FIND_PACKAGE(Boost REQUIRED)

#include_directories(${VTK_INCLUDE_DIRS} ${Boost_INCLUDE_DIR})
include_directories(${VTK_INCLUDE_DIRS})
add_definitions("-Wno-deprecated")

add_library(nsv OBJECT ${KAIROS_SRC_FILES} ${KAIROS_HEADER_FILES})

#if(APPLE)
#	SET(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "-dynamiclib -undefined suppress")
#endif(APPLE)

#if(OPTION_STATIC)
#	add_library(nsv STATIC ${KAIROS_SRC_FILES} ${KAIROS_HEADER_FILES})
#else()
#	add_library(nsv SHARED ${KAIROS_SRC_FILES} ${KAIROS_HEADER_FILES})
#endif(OPTION_STATIC)

#TARGET_LINK_LIBRARIES(nsv ${VTK_LIBRARIES})


