#include <cassert>
#include <iostream>
#include <cstdint>
#include "SVector.h"
#include "NormVector.h"
#include "TPoint.h"
#include "algo.h"

//using spatial::Slope; 
using spatial::TPoint; 
using spatial::cX; 
using spatial::cY; 
using std::cout;
using std::endl;

template <class T,int N>
TPoint<T,N> spatial::pointThrough(const spatial::TPoint<T,N> & origin, const spatial::NormVector<double,N> & direction, T distance) {
	std::array<T,N> coords;
	for (size_t i = 0; i < N; i++) {
		const Axis a = static_cast<Axis>(i);
		coords[i] = origin(a) + static_cast<T>(direction(a) * distance);
	}
	return TPoint<T,N>(coords);
}

template <class T,int N>
TPoint<T,N> spatial::pointThrough(const spatial::TPoint<T,N> & origin, const spatial::SVector<T,N> & direction, T distance) {
	NormVector<double,N> norm(direction.template convert<double>( ));
	return spatial::pointThrough(origin,norm,distance);
}


template <class T>
bool spatial::below(const TPoint<T,2> & point, const TPoint<T,2> & a, const TPoint<T,2> & b) {
	//static_assert(T::numDim( ) == 2,"for 2d only");
	using spatial::cX;
	using spatial::cY;
	//sort out which is left and which is right
	const bool swapped = a(cX) > b(cX); 
	const TPoint<T,2> & lhs = swapped ? b : a; 
	const TPoint<T,2> & rhs = swapped ? a : b; 

	T px = point(cX);
	if ( (px < lhs(cX) ) || (px > rhs(cX) ) ) {
		return false;
	}
	T py = point(cY);
	T leftY = lhs(cY);
	T rightY = rhs(cY);
	if ( (py < leftY) && (py < rightY) ) {
		return true;
	}
	if ( (py > leftY) && (py > rightY) ) {
		return false;
	}
	T leftPointX = px - lhs(cX); 
	T leftPointY = py - lhs(cY); 
	T leftPointSlope = leftPointY  / leftPointX;   

	T edgeX = rhs(cX) - lhs(cX); 
	T edgeY = rhs(cY) - lhs(cY); 
	T edgeSlope = edgeY / edgeX; 

	return leftPointSlope < edgeSlope;
}



/*
base algorithm for impelementation below
int pnpoly(int nvert, float *vertx, float *verty, float testx, float testy)
{
int i, j, c = 0;
for (i = 0, j = nvert-1; i < nvert; j = i++) {
if ( ((verty[i]>testy) != (verty[j]>testy)) &&
(testx < (vertx[j]-vertx[i]) * (testy-verty[i]) / (verty[j]-verty[i]) + vertx[i]) )
c = !c;
}
return c;
}`
*/
namespace {
	/**
	* return true if (x,y) is to the left of segment given by (ix,iy), (jx,jy)
	* @param x point to test 
	* @param y point to test 
	* @param ix one x of segment
	* @param iy one y of segment
	* @param jx other x of segment
	* @param jy other y of segment
	*/
	template <class TYPE>
	inline bool leftOf(TYPE x, TYPE y, TYPE ix, TYPE iy, TYPE jx, TYPE jy) {
		if ((iy > y) == (jy > y))   {
			return false;
		}
		const double runRise = static_cast<double>(jx-ix) / (jy-iy);
		const TYPE up = y - iy;
		const TYPE xPointOnSegment = static_cast<TYPE>(up * runRise + ix); 
		return x < xPointOnSegment;
		return false;
	}
}
/**
* right ray casting inside algorithm
* @tparam TPOINT type of spatial::TPoint
*/
template <class TPOINT>
bool spatial::inside(const std::vector<TPOINT> &  polygon, const TPOINT & point) {
	typedef const TPOINT PT;
	const typename TPOINT::value_type x = point(cX);
	const typename TPOINT::value_type y = point(cY);
	bool ins = false;
	typename std::vector<TPOINT>::const_iterator iter = polygon.begin( );
	typename std::vector<TPOINT>::const_iterator end = polygon.end( );

	PT * i = &(*iter);
	++iter;
	PT * j = &(*iter);
	for (;;) {
		const typename TPOINT::value_type ix =i->get(cX);
		const typename TPOINT::value_type iy =i->get(cY);
		const typename TPOINT::value_type jx =j->get(cX);
		const typename TPOINT::value_type jy =j->get(cY);
		if (leftOf(x,y, ix,iy,jx,jy)) {
			ins = !ins;
		}
		i = j ;
		++iter;
		if (iter == end) {
			break;
		}
		j = &(*iter);
	}
	{
		j = &(polygon.front( ));
		const typename TPOINT::value_type ix =i->get(cX);
		const typename TPOINT::value_type iy =i->get(cY);
		const typename TPOINT::value_type jx =j->get(cX);
		const typename TPOINT::value_type jy =j->get(cY);
		if (leftOf(x,y, ix,iy,jx,jy)) {
			ins = !ins;
		}
	}
	return ins;
}
template <class T>
bool spatial::between(const TPoint<T,2> & origin, const SVector<T,2> & direction,const TPoint<T,2> & vertex, const TPoint<T,2> & otherVertex) {
	SVector<T,2>  vertexVector(vertex,otherVertex);
	std::pair<bool,T> w = intersectParameter(origin,direction,vertex,vertexVector);
	if (w.first) {
		return w.second >= 0 && w.second <= 1;
	}
	return false;
}


template bool spatial::below(const spatial::TPoint<double,2> &, const spatial::TPoint<double,2> &, const spatial::TPoint<double,2> &);
template bool spatial::inside(const std::vector<TPoint<double,2> > &  polygon, const TPoint<double,2> & point);

/*
typedef spatial::TPoint<long,2> LPType;
template bool spatial::below(const spatial::TPoint<long,2> &, const spatial::TPoint<long,2> &, const spatial::TPoint<long,2> &);
template bool spatial::inside(const std::vector<TPoint<long,2> > &  polygon, const TPoint<long,2> & point);
template LPType spatial::pointThrough(const LPType & origin, const spatial::SVector<long,2> & direction, long distance); 

typedef spatial::TPoint<int,2> IPType;
template bool spatial::below(const spatial::TPoint<int,2> &, const spatial::TPoint<int,2> &, const spatial::TPoint<int,2> &);
template bool spatial::inside(const std::vector<TPoint<int,2> > &  polygon, const TPoint<int,2> & point);
template IPType spatial::pointThrough(const IPType & origin, const spatial::SVector<int,2> & direction, int distance); 
*/

typedef spatial::TPoint<double,2> LPoint;
template LPoint spatial::pointThrough(const LPoint & origin, const spatial::SVector<double,2> & direction, double distance); 
template bool spatial::between(const LPoint & origin, const SVector<double,2> & direction,const LPoint & vertex, const LPoint & otherVertex); 

typedef spatial::TPoint<int64_t,2> Point64;
template bool spatial::below(const spatial::TPoint<int64_t,2> &, const spatial::TPoint<int64_t,2> &, const spatial::TPoint<int64_t,2> &);
template bool spatial::inside(const std::vector<TPoint<int64_t,2> > &  polygon, const TPoint<int64_t,2> & point64_t);
template Point64 spatial::pointThrough(const Point64 & origin, const spatial::SVector<int64_t,2> & direction, int64_t distance); 
//template bool spatial::between(const Point64 & origin, const SVector<double,2> & direction,const Point64 & vertex, const Point64 & otherVertex); 

typedef spatial::TPoint<int32_t,2> Point32;
template bool spatial::below(const spatial::TPoint<int32_t,2> &, const spatial::TPoint<int32_t,2> &, const spatial::TPoint<int32_t,2> &);
template bool spatial::inside(const std::vector<TPoint<int32_t,2> > &  polygon, const TPoint<int32_t,2> & point32_t);
template Point32 spatial::pointThrough(const Point32 & origin, const spatial::SVector<int32_t,2> & direction, int32_t distance); 

typedef spatial::TPoint<int16_t,2> Point16;
template bool spatial::below(const spatial::TPoint<int16_t,2> &, const spatial::TPoint<int16_t,2> &, const spatial::TPoint<int16_t,2> &);
template bool spatial::inside(const std::vector<TPoint<int16_t,2> > &  polygon, const TPoint<int16_t,2> & point16_t);
template Point16 spatial::pointThrough(const Point16 & origin, const spatial::SVector<int16_t,2> & direction, int16_t distance); 
