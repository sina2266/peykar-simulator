#ifndef MovingBoundaryCollections_h
#define MovingBoundaryCollections_h
#include <vector>
#include <TPoint.h>
#include <MovingBoundaryTypes.h>
/**
* implementation types which require additional collection types
* for definition
*/
namespace moving_boundary {
	/**
	* base class of #MeshElementNode and #FrontPointType
	*/
	typedef spatial::TPoint<CoordinateType,2> CoordinatePoint; 
	/**
	* component type of front
	*/
	typedef spatial::TPoint<CoordinateType,2> FrontPointType;
	/**
	* front as collection
	*/
	typedef std::vector<FrontPointType> FrontType;
}
#endif
