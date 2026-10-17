#ifndef VoronoiResult_h
#define VoronoiResult_h
#include <vector>
#include <MovingBoundaryTypes.h>
#include <TPoint.h>
#include <cstdint>
namespace spatial {
	/**
	* implementation type for boost::polygon::voronoi routines
	*/
	typedef int32_t VoronoiType;

	typedef GhostPoint<VoronoiType,2> VoronoiGhostPoint;

	/**
	* POD
	* voronoi result types
	* closed - simple closed polygon
	* straightLine - open polygon with two infinite edges - marked as ghost points and 
	*	at ends of vertices
	* singleOpen - open polygon with two infinite edges - marked as ghost points and 
	*	at ends of vertices
	* multiOpen - multiple segments with infinite edges; in vertices in sets of
	*	three with midpoint non-ghost and ends ghost points
	*/
	struct VoronoiBase {
		enum Type {closed,straightLine,singleOpen, multiOpen};
		VoronoiBase(Type t)
			:type(t) {}
		Type type;
	};

	
	/**
	* make template form to allow different integer size variants to compile
	*/
	template <typename T>
	struct TVoronoiResult : public VoronoiBase {
		TVoronoiResult(Type t = closed)
			:VoronoiBase(t),
			vertices( ) {}
		//OPT: include space for allocation in structure
		typedef std::vector<GhostPoint<T,2> > GhostVector; 
		GhostVector vertices; 
	};

	typedef TVoronoiResult<VoronoiType> VoronoiResult; 
}

#endif
