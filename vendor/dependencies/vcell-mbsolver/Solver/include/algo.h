#ifndef algo_h
#define algo_h
#include <vector>
#include <NormVector.h>
template <typename T> int sgn(T val) {
	return (T(0) < val) - (val < T(0));
}
namespace spatial {
	template <class T, int N>
	class Edge; 

	template <class T, int N>
	TPoint<T,N> pointThrough(const TPoint<T,N> & origin, const SVector<T,N> & direction, T distance);

	template <class T,int N>
	TPoint<T,N> pointThrough(const TPoint<T,N> & origin, const NormVector<double,N> & direction, T distance);

	template <class T>
	bool below(const TPoint<T,2> & point, const TPoint<T,2> & lhs, const TPoint<T,2> & rhs); 

	template <class TPOINT>
	bool inside(const std::vector<TPOINT> &  polygon, const TPOINT & point); 

	/*
	template<> inline bool inside(const std::vector<TPoint<short,2> > &  polygon, const spatial::TPoint<short,2> & point) {
		throw std::domain_error("not supported");
	}
	*/

	/**
	* @return true if and only if lhs and rhs have same y value
	*/
	template <class T>
	bool horizontal(const TPoint<T,2> & lhs, const TPoint<T,2> & rhs) {
		return lhs(cY) == rhs(cY); 
	}

	/**
	* @return true if and only if lhs and rhs have same x value
	*/
	template <class T>
	bool vertical(const TPoint<T,2> & lhs, const TPoint<T,2> & rhs) {
		return lhs(cX) == rhs(cX); 
	}

	/**
	* return distance along specified axis
	*/
	template <class T, int N>
	T axialDistance(const TPoint<T,N> & lhs, const TPoint<T,N> & rhs, Axis a) {
		assert (a < N);
		T lc = lhs(a);
		T rc = rhs(a);
		return rc > lc ? rc -lc : lc - rc;
	}

	template <class T, int N>
	TPoint<T,N> displacement(const TPoint<T,N> & origin, const SVector<T,N> & disp) {
		TPoint<T,N> rval(origin); 
		for (int i = 0; i < N; i++) {
			Axis a = static_cast<Axis>(i);
			rval(a) += disp(a);
		}
		return rval;
	}

	template <class T, int N>
	inline TPoint<T,N> midPoint(const spatial::TPoint<T,N> & lhs, const spatial::TPoint<T,N> & rhs ) {
		std::array<T,N> values;
		for (int i = 0; i < N ;i++) {
			const Axis a = static_cast<Axis>(i);
			values[i] = ( lhs(a) + rhs(a) )/ 2;
		}
		return  TPoint<T,N>(values);
	}

	template <class T, int N>
	inline TPoint<T,N> integerMidPoint(const spatial::TPoint<T,N> & lhs, const spatial::TPoint<T,N> & rhs ) {
		std::array<T,N> values;
		for (int i = 0; i < N ;i++) {
			const Axis a = static_cast<Axis>(i);
			T op1 = lhs(a);
			T op2 = rhs(a);
			//from stack overflow; prevent overflow, handle case where both values are odd
			values[i] = (op1 >> 1) + (op2 >> 1) + (op1 & op2 & 0x1);
		}
		return  TPoint<T,N>(values);
	}

	template <int N>
	inline TPoint<char,N> midPoint(const spatial::TPoint<char,N> & lhs, const spatial::TPoint<char,N> & rhs ) {
		return integerMidPoint(lhs,rhs);
	}
	template <int N>
	inline TPoint<int,N> midPoint(const spatial::TPoint<int,N> & lhs, const spatial::TPoint<int,N> & rhs ) {
		return integerMidPoint(lhs,rhs);
	}
	template <int N>
	inline TPoint<short,N> midPoint(const spatial::TPoint<short,N> & lhs, const spatial::TPoint<short,N> & rhs ) {
		return integerMidPoint(lhs,rhs);
	}
	template <int N>
	inline TPoint<long,N> midPoint(const spatial::TPoint<long,N> & lhs, const spatial::TPoint<long,N> & rhs ) {
		return integerMidPoint(lhs,rhs);
	}
	template <int N>
	inline TPoint<long long,N> midPoint(const spatial::TPoint<long long,N> & lhs, const spatial::TPoint<long long,N> & rhs ) {
		return integerMidPoint(lhs,rhs);
	}

	/**
	* intersection of lines given by points and vectors
	* for a + ub = c + wd
	* @return true, w if exists, or false, unspecified if no intersection
	*/
	template <class T>
	std::pair<bool,T> intersectParameter(const TPoint<T,2> & a, const SVector<T,2> & b , const TPoint<T,2> & c, const SVector<T,2> & d)  {
		using spatial::cX;
		using spatial::cY;
		T denom = b(cY)*d(cX) - b(cX)*d(cY);
		if (fabs(denom) < 1e-10) {
			return std::pair<bool,T>(false,0);
		}

		T num   = a(cX)*b(cY) - a(cY)*b(cX) - b(cY)*c(cX) + b(cX)*c(cY); 
		T w = num / denom;
		return std::pair<bool,T>(true,w);
	}

	/**
	* does line "b" passing through "a" intersect line "d" passing through "c" anywhere?
	@ return bool (true, intersection point) or false, default
	*/
	template <class T>
	std::pair<bool, TPoint<T,2> > intersectionLines(const TPoint<T,2> & a, const SVector<T,2> & b , const TPoint<T,2> & c, const SVector<T,2> & d)  {
		using spatial::cX;
		using spatial::cY;
		using std::pair;
		pair<bool,T> w = intersectParameter(a,b,c,d);
		if (w.first) {
			T x = c(cX) + w.second * d(cX); 
			T y = c(cY) + w.second * d(cY); 
			return pair<bool,TPoint<T,2> >(true,TPoint<T,2>(x,y));
		}
		return pair<bool,TPoint<T,2> >(false,TPoint<T,2>());
	}
	/**
	* does line "line" passing through (intersect) line segment "segment" starting at c
	@ return bool (true, intersection point) or false, default
	*/
	template <class T>
	std::pair<bool, TPoint<T,2> > intersectionLineSegment(const TPoint<T,2> & a, const SVector<T,2> & line , const TPoint<T,2> & c, const SVector<T,2> & segVec)  {
		using spatial::cX;
		using spatial::cY;
		using std::pair;
		pair<bool,T> w = intersectParameter(a,line,c,segVec);
		if (w.first && (w.second >= 0 && w.second <= 1) ) {
			T x = c(cX) + w.second * segVec(cX); 
			T y = c(cY) + w.second * segVec(cY); 
			return pair<bool,TPoint<T,2> >(true,TPoint<T,2>(x,y));
		}
		return pair<bool,TPoint<T,2> >(false,TPoint<T,2>());
	}

	/**
	* does ray intersect line segment 
	@ return bool (true, intersection point) or false, default
	*/
	template <class T>
	std::pair<bool, TPoint<T,2> > intersectionRaySegment(
			const TPoint<T,2> & rayStart, const SVector<T,2> & rayVector , 
			const TPoint<T,2> & c, const SVector<T,2> & segVec)  {

		using spatial::cX;
		using spatial::cY;
		using std::pair;
		pair<bool,T> w = intersectParameter(rayStart,rayVector,c,segVec);
		if (w.first && (w.second >= 0 && w.second <= 1) ) {
			//make sure in intersection is not behind the ray origin
			pair<bool,T> r = intersectParameter(c,segVec,rayStart,rayVector);
			assert (r.first);
			if (r.second >=0) {
				T x = c(cX) + w.second * segVec(cX); 
				T y = c(cY) + w.second * segVec(cY); 
				return pair<bool,TPoint<T,2> >(true,TPoint<T,2>(x,y));
			}
		}
		return pair<bool,TPoint<T,2> >(false,TPoint<T,2>());
	}
	/**
	* does ray intersect line segment 
	@ return bool (true, intersection point) or false, default
	*/
	template <class T>
	std::pair<bool, TPoint<T,2> > intersectionRaySegment(
			const TPoint<T,2> & rayStart, const SVector<T,2> & rayVector , 
			const Edge<T,2> &segVec) {
		return intersectionRaySegment(rayStart,rayVector,segVec.origin( ),segVec.edgeVector( ));
	}

	/**
	* do segments intersect? 
	@ return bool (true, intersection point) or false, default
	*/
	template <class T>
	std::pair<bool, TPoint<T,2> > intersectionSegments(
			const TPoint<T,2> & a, const SVector<T,2> & segVecA , 
			const TPoint<T,2> & b, const SVector<T,2> & segVecB )  {

		using spatial::cX;
		using spatial::cY;
		using std::pair;
		pair<bool,T> w = intersectParameter(a,segVecA,b,segVecB);
		if (w.first && (w.second >= 0 && w.second <= 1) ) {
			pair<bool,T> r = intersectParameter(b,segVecB,a,segVecA);
			assert (r.first);
			if (r.second >=0 && r.second <= 1) {
				T x = b(cX) + w.second * segVecB(cX); 
				T y = b(cY) + w.second * segVecB(cY); 
				return pair<bool,TPoint<T,2> >(true,TPoint<T,2>(x,y));
			}
		}
		return pair<bool,TPoint<T,2> >(false,TPoint<T,2>());
	}
	/**
	* do segments intersect? 
	@ return bool (true, intersection point) or false, default
	*/
	template <class T>
	std::pair<bool, TPoint<T,2> > intersectionSegments(const Edge<T,2> & a, const Edge<T,2> &b) {
		return intersectionSegments(a.origin( ),a.edgeVector( ),b.origin( ),b.edgeVector( ));
	}
	/**
	* do segments intersect? 
	@ return bool 
	*/
	template <class T>
	bool intersectionSegmentsQuery(
			const TPoint<T,2> & a, const SVector<T,2> & segVecA , 
			const TPoint<T,2> & b, const SVector<T,2> & segVecB )  {

		using spatial::cX;
		using spatial::cY;
		using std::pair;
		pair<bool,T> w = intersectParameter(a,segVecA,b,segVecB);
		if (w.first && (w.second >= 0 && w.second <= 1) ) {
			pair<bool,T> r = intersectParameter(b,segVecB,a,segVecA);
			assert (r.first);
			if (r.second >=0 && r.second <= 1) {
				return true;
			}
		}
		return false; 
	}

	/**
	* do segments intersect? 
	@ return bool 
	*/
	template <class T>
	bool intersectionSegmentsQuery(const Edge<T,2> & a, const Edge<T,2> &b) {
		return intersectionSegmentsQuery(a.origin( ),a.edgeVector( ),b.origin( ),b.edgeVector( ));
	}
	/**
	* rotate about origin 
	* @param in point to rotate
	* @param radianAngle how much to rotate
	*/
	template <class T>
	TPoint<T,2> rotate(const TPoint<T,2> & in, double radianAngle) {
		T x = cos(radianAngle) *in(cX) + sin(radianAngle) * in(cY);
		T y = sin(radianAngle) *in(cX) + cos(radianAngle) * in(cY);
		return TPoint<T,2>(x,y);
	}


	/**
	* does the vector from origin pass through the edge defined by vertex, otherVertex?
	*/
	template <class T>
	bool between(const TPoint<T,2> & origin, const SVector<T,2> & direction,const TPoint<T,2> & vertex, const TPoint<T,2> & otherVertex);

	/**
	* near zero, in absolute terms
	*/
	template <class REAL>
	bool nearZero(REAL v) {
		return std::abs(v) < 1e-10;
	}


	/**
	* difference is small compared to reference value 
	* @param value to evaluate (may be negative) 
	* @param reference value to use as relative baseline (may be negative) 
	* @param tolerance relative precision required (relative to reference) 
	*/
	template <class REAL>
	REAL relativeDifference(REAL value, REAL reference) {
		REAL diff = value - reference;

		if (diff != 0) {
			return  std::abs(diff/reference);
		}
		//else, implied
		return 0;
	}
	/**
	* difference is small compared to reference value 
	* @param v difference value to evaluate (may be negative) 
	* @param reference value to use as relative baseline (may be negative) 
	* @param tolerance relative precision required (relative to reference) must be positive
	*/
	template <class VAL_TYPE, class TOLERANCE_TYPE>
	bool smallRelativeDifference(VAL_TYPE v, VAL_TYPE reference, TOLERANCE_TYPE tolerance=1e-5) {
		assert(tolerance > 0);
		if (v != 0) {
			const VAL_TYPE limit = static_cast<VAL_TYPE>(tolerance * std::abs(reference));
			return std::abs(v) < limit; 
		}
		//else, implied
		return true;
	}

	/**
	* difference is relatively small 
	* @tparam VAL_TYPE type of values to compare
	* @tparam TOLERANCE_TYPE type of tolerance specification 
	* @param newVal new value 
	* @param refVal reference value 
	* @param tolerance relative precision required (relative to #refVal) 
	*/
	template <class VAL_TYPE, class TOLERANCE_TYPE>
	bool nearlyEqual(VAL_TYPE newVal, VAL_TYPE refVal, TOLERANCE_TYPE tolerance) {
		return smallRelativeDifference(newVal - refVal, refVal, tolerance); 
	}

	/**
	* difference is relatively small (default tolerance) 
	* @tparam VAL_TYPE type of values to compare
	* @param newVal new value 
	* @param refVal reference value 
	* @param tolerance relative precision required (relative to #refVal) 
	*/
	template <class VAL_TYPE>
	bool nearlyEqual(VAL_TYPE newVal, VAL_TYPE refVal) {
		return nearlyEqual<VAL_TYPE,double>(newVal,refVal,1e-5);
	}

	/**
	* difference is relatively small 
	* @param a value 
	* @param b value 
	* @param tolerance maximum difference 
	*/
	template <class REAL>
	bool differenceLessThan(REAL a, REAL b, REAL tolerance=1e-5) {
		return std::abs(a-b) < tolerance; 
	}

	/**
	* is "point" on line given by origin & vector ?
	*/
	template <class T>
	bool pointOn(const TPoint<T,2> & origin, const SVector<T,2> & vector, const TPoint<T,2> & point) {
		double dx = point(cX) - origin(cX);
		if (nearZero(vector(cX)) ){ 
			return nearZero(dx); 
		}
		double dy = point(cY) - origin(cY);
		if (nearZero(vector(cY)) ){  
			return nearZero(dy); 
		}

		double ux = dx / vector(cX);
		double uy = dy / vector(cY);
		return nearZero(ux - uy);
	}

	/**
	* closerEnd result
	*/
	template <class T>
	struct EndFindResult {
		TPoint<T,2> end;
	};

	/**
	* edgeFindResult
	*/
	template <class T>
	struct EdgeFindResult : public EndFindResult<T> {
		bool found;
		TPoint<T,2> intersection;
		EdgeFindResult( )
			:found(false) {}
	};

	/**
	* given a ray and a segment known to intersect find which end of segment
	* is on same side of ray as origin
	*/
	template <class T>
	void closerEnd(spatial::EndFindResult<T> & result,const spatial::TPoint<T,2> &intersection, const spatial::TPoint<T,2> & origin,
		const spatial::TPoint<T,2> & rayOrigin, const spatial::SVector<T,2> & rayVector,
		const spatial::TPoint<T,2> &segmentOrigin, const spatial::SVector<T,2> & segmentVector) {
		using spatial::cX;
		using spatial::cY;
		using spatial::Axis;
		typedef spatial::SVector<T,2> VectorType;
		typedef spatial::TPoint<T,2> PointType;

		//project from intersection in direction of segment 
		PointType sProj = spatial::displacement(intersection,segmentVector);
		VectorType sProjVector = VectorType(origin,sProj);  
		//see if segment from origin projection crosses other line
		std::pair<bool,PointType> projIntersectionS = intersectionLineSegment(rayOrigin,rayVector,origin,sProjVector);
		if (projIntersectionS.first) {
			result.end = segmentOrigin; 
		}
		else {
			PointType sEnd = spatial::displacement(segmentOrigin,segmentVector);
			result.end = sEnd; 
		}
	}
	/**
	* given a ray and a segment known to intersect find which end of segment
	* is on same side of ray as origin
	*/
	template <class T>
	void closerEnd(spatial::EndFindResult<T> & result,const spatial::TPoint<T,2> &intersection, const spatial::TPoint<T,2> & origin,
		const Edge<T,2> &ray, const Edge<T,2> &edge) {
			closerEnd(result,intersection,origin,ray.origin( ), ray.edgeVector( ),edge.origin( ), edge.edgeVector( ));
	}
	/**
	* given a ray and a segment, find if they intersect and which end of segment
	* is on same side of ray as origin
	*/
	template <class T>
	void edgeFind(spatial::EdgeFindResult<T> & result, const spatial::TPoint<T,2> & origin,
		const spatial::TPoint<T,2> & rayOrigin, const spatial::SVector<T,2> & rayVector,
		const spatial::TPoint<T,2> &segmentOrigin, const spatial::SVector<T,2> & segmentVector) {
		using spatial::cX;
		using spatial::cY;
		using spatial::Axis;
		typedef spatial::SVector<T,2> VectorType;
		typedef spatial::TPoint<T,2> PointType;

		std::pair<bool,PointType> lineSeg = intersectionRaySegment(rayOrigin,rayVector,segmentOrigin,segmentVector);
		if (!lineSeg.first) {
			result.found = false;
			return;
		}
		result.found = true;
		Point2D &iSect = lineSeg.second;
		//intersection is intersection
		result.intersection = iSect;
		closerEnd<T>(result,iSect,origin,rayOrigin,rayVector,segmentOrigin,segmentVector);
	}

	template <class T>
	struct InsideCrossResult {
		bool found;
		TPoint<T,2> intersection;
		bool aBeginIsInside;
		bool bBeginIsInside;
		InsideCrossResult( )
			:found(false) {}
	};
	template <class T>
	void insideCrossingFind(InsideCrossResult<T> & result, const spatial::TPoint<T,2> & origin,
		const spatial::TPoint<T,2> & segVecA, const spatial::SVector<T,2> & aVector,
		const spatial::TPoint<T,2> &segVecB, const spatial::SVector<T,2> & bVector) {
		using spatial::cX;
		using spatial::cY;
		using spatial::Axis;
		//using spatial::Edge2;
		typedef spatial::SVector<T,2> VectorType;
		typedef spatial::TPoint<T,2> PointType;

		std::pair<bool,PointType> lineSeg = intersectionSegments(segVecA,aVector,segVecB,bVector);
		if (!lineSeg.first) {
			result.found = false;
			return;
		}
		result.found = true;
		Point2D &iSect = lineSeg.second;
		//middle is intersection
		result.intersection = iSect;

		//project from intersection in direction of a 
		PointType aProj = spatial::displacement(iSect,aVector);
		VectorType aProjVector = VectorType(origin,aProj);  
		//see if segment from origin projection crosses other line
		std::pair<bool,PointType> projIntersectionA = intersectionLineSegment(segVecB,bVector,origin,aProjVector);

		//repeat for b
		PointType bProj = spatial::displacement(iSect,bVector);
		VectorType bProjVector = VectorType(origin,bProj);  
		//see if segment from origin projection crosses other line
		std::pair<bool,PointType> projIntersectionB = intersectionLineSegment(segVecA,aVector,origin,bProjVector);
		//if there's an intersection, projection was away from origin point
		result.aBeginIsInside = projIntersectionA.first; 
		result.bBeginIsInside = projIntersectionB.first; 
	}

	/**
	* returns single polygon intersection of a and b
	* returns empty vector if there are 0 or > 1 polygons from by intersection
	*/
	template <class FT_POINT,class POINTA, class POINTB>
	std::vector<FT_POINT> intersection(const FT_POINT & insidePoint, const std::vector<POINTA> &a,const std::vector<POINTB> &b);


	/**
	* template instantiation
	*/
	template <>
	inline std::vector<TPoint<double,2> > intersection(const TPoint<double,2> & insidePoint, const std::vector<GhostPoint<double,2> > &a,
		const std::vector<TPoint<double,2> > &b) {
			return std::vector<TPoint<double,2> >( );
	}

}
#endif
