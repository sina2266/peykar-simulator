#ifndef intersection_h
#define intersection_h
#include <Volume.h>
#include <vector>
namespace spatial {
	/**
	* returns intersection of polygon and polygon 
	* @tparam COORD_TYPE Volume support
	* @tparam VALUE_TYPE Volume support
	* @tparam POINT_TYPE input point type, #TPoint instantiation
	* @param result in/out parameter 
	* @param a polygon (vector of TPoints)
	* @param b other polygon
	*/
	template <typename COORD_TYPE,typename VALUE_TYPE,class POINT_TYPE>
	void intersections(Volume<COORD_TYPE,VALUE_TYPE,2> &result, const std::vector<POINT_TYPE> &a,const std::vector<POINT_TYPE> &b);

	/**
	* returns intersection of many polygons and polygon 
	* @tparam COORD_TYPE Volume support
	* @tparam VALUE_TYPE Volume support
	* @tparam POINT_TYPE input point type, #TPoint instantiation
	* @param result in/out parameter 
	* @param vOfVa vector of polygons 
	* @param b other polygon
	*/
	template <typename COORD_TYPE,typename VALUE_TYPE,class POINT_TYPE>
	void intersectionsManySingle(Volume<COORD_TYPE,VALUE_TYPE,2> &result, const std::vector<std::vector<POINT_TYPE> > & vOfVa,const std::vector<POINT_TYPE> &b);

	/**
	* returns intersection of many polygons and many polygons
	* returns intersection of a and b
	* @tparam COORD_TYPE Volume support
	* @tparam VALUE_TYPE Volume support
	* @tparam POINT_TYPE input point type, #TPoint instantiation
	* @param result in/out parameter 
	* @param vOfVa vector of polygons 
	* @param vOfVb vector of polygons 
	*/
	template <typename COORD_TYPE,typename VALUE_TYPE,class POINT_TYPE>
	void intersectionsManyMany(Volume<COORD_TYPE,VALUE_TYPE,2> &result, const std::vector<std::vector<POINT_TYPE> > & vOfVa,const std::vector<std::vector<POINT_TYPE> > & vOfVb);

	//use in MeshElementNode with call to spatial::intersection 
	const int  MAX_EXPECTED_VORONOI_POINTS = 12;
}
#endif
