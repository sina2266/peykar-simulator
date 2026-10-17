#ifndef FronTierAdapt_h
#define FronTierAdapt_h
#include <iostream>
#include <MBridge/MBPolygon.h>
#include <MBridge/Scatter.h>
#include <TPoint.h>
#include <algo.h>
namespace frontTierAdapt {

	/**
	* TARGET e.g. matlabBridge::Polygon, matlabBridge::Scatter
	* INPOINT: spatial::TPoint<REAL,2> or derived
	*/
	template <class TARGET, class INPOINT>
	void copyPointInto(TARGET & tgt, const INPOINT &point)  {
		tgt.add(point(spatial::cX),point(spatial::cY));
	}
	
	/**
	* TARGET e.g. matlabBridge::Polygon, matlabBridge::Scatter
	* INPOINT: spatial::TPoint<REAL,2> or derived
	*/
	template <class TARGET, class INPOINT>
	void copyVectorInto(TARGET & tgt, const std::vector<INPOINT>&polygon)  {
		typename std::vector<INPOINT>::const_iterator iter = polygon.begin( );
		for (;iter != polygon.end( );++iter) {
			copyPointInto(tgt,*iter);
		}
	}

	/**
	* TARGET e.g. matlabBridge::Polygons
	* INPOINT: spatial::TPoint<REAL,2> or derived
	*/
	template <class TARGET, class INPOINT>
	void copyVectorsInto(TARGET & tgt, const std::vector<std::vector<INPOINT> >&polygons)  {
		typedef std::vector<std::vector<INPOINT> > VV;
		for (typename VV::const_iterator vIter = polygons.begin( ); vIter != polygons.end( ); ++vIter) {
			const std::vector<INPOINT> & polygon = *vIter;
			typename std::vector<INPOINT>::const_iterator iter = polygon.begin( );
			for (;iter != polygon.end( );++iter) {
				copyPointInto(tgt,*iter);
			}
			tgt.nextPolygon( );
		}
	}

	template <class REAL>
	void plotVector(std::ostream &os,  const spatial::TPoint<REAL,2> &point, const spatial::SVector<REAL,2> & vector,
			const char * const lineSpec , int lineWidth = 0, int sequenceNumber = 0) { 

		matlabBridge::Polygon poly(lineSpec,lineWidth,sequenceNumber);
		poly.add(point(spatial::cX),point(spatial::cY));
		spatial::TPoint<REAL,2> end = spatial::displacement(point,vector);
		poly.add(end(spatial::cX),end(spatial::cY));
		poly.write(os);
	}

}
#endif
