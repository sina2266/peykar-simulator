#include <iostream>
#include <cassert>

#include <vector>
#include <fstream>
#include <algorithm>
#include <array>
#include <random>
#include "MPoint.h"
#include "Volume.h"
#include <Segment.h>
#include <Logger.h>
//#include "SegmentIterator.h"
#include "gtest/gtest.h"
#include "MBridge/MBPolygon.h"
#include "MBridge/Figure.h"
#include "MBridge/FronTierAdapt.h"
TEST(volume,polygons) {
	using namespace spatial;
	using namespace std; 
	Volume<double,double,2> v; 
	v.nextSection( );
	v.add(Point2D(1,2));
	v.add(Point2D(2,2));
	v.add(Point2D(2,1));
	v.nextSection( );
	v.add(Point2D(3,2));
	v.add(Point2D(3,3));
	v.add(Point2D(2,3));
	/*
	Volume<double,double,2>::const_iterator iter = v.begin( );
	for (;iter != v.end( );++iter) {
	cout << *iter << endl;
	}
	matlabBridge::Polygon pPoly("k",3);
	frontTierAdapt::copyInto<TPoint<double,2> >(pPoly,v.begin( ),v.end( ));
	ofstream d("voltest.m");
	d << pPoly;
	*/
}
namespace {
	std::vector<spatial::TPoint<double,2>> randPolygon(size_t coordLimit, size_t maxPoints) {
		typedef spatial::TPoint<double,2> Point;
		typedef spatial::SVector<double,2> Vector; 
		typedef spatial::Edge<double,2> Segment;
		using spatial::DegAngle;

		std::vector<spatial::TPoint<double,2>> result; 
		int nPoints = 3 + rand( ) % (maxPoints -3); 
		double incr = 360.0 / nPoints;

		int x = rand( ) % coordLimit;
		int y = rand( ) % coordLimit;
		Point center(x,y);
		DegAngle degrees;
		for (double d = 0; d < 360.0; d+= incr) {
			bool added = false;
			degrees = d;
				for (int length = 1 + rand( ) % coordLimit;length > 0 && !added;length--) {
				Vector v(degrees,length);
				Point candidate = spatial::displacement(center,v); 
				if (std::abs(candidate(spatial::cX)) < coordLimit && std::abs(candidate(spatial::cY)) < coordLimit) {
					result.push_back(candidate);
					added = true;
				}
			} 

		}
		result.push_back(result.front( ));

		return result;
	}

	spatial::Volume<double,double,2> randVolume(size_t coordLimit, size_t maxPoints) {
		spatial::Volume<double,double,2> rval;
		std::vector<spatial::TPoint<double,2>> poly = randPolygon(coordLimit,maxPoints);
		size_t outSize= poly.size( );
		std::copy(poly.begin( ), poly.end( ),rval.fillingIterator(outSize));
		return rval;
	}


	template <class T>
	struct Offsetter {
		const T x_;
		const T y_;
		Offsetter(T x, T y)
			:x_(x),
			y_(y) {}
		spatial::TPoint<T,2> operator( )(const spatial::TPoint<T,2> & point) {
			T x = point(spatial::cX) + x_; 
			T y = point(spatial::cY) + y_; 
			return spatial::TPoint<T,2>(x,y); 
		}
	};

	/**
	* @param numberSections 2 to 4
	*/
	spatial::Volume<double,double,2> multiple(size_t numberSections, size_t coordLimit, int nPoints) {
		spatial::Volume<double,double,2> rval;
		size_t halfLimit = coordLimit / 2;
		double halfLimitD = static_cast<double>(halfLimit);
		double limitD = static_cast<double>(coordLimit);
		for (size_t s = 0; s< numberSections; ++s) {
			rval.nextSection( );
			std::vector<spatial::TPoint<double,2>> poly = randPolygon(halfLimit,nPoints);
			size_t outSize= poly.size( );
			switch (s) {
			case 0:
				std::copy(poly.begin( ), poly.end( ),rval.fillingIterator(outSize));
				break;
			case 1:
				{
					Offsetter<double> offset(limitD,0);
					std::transform(poly.begin( ), poly.end( ),rval.fillingIterator(outSize), offset);
					break;
				}
			case 2:
				{
					Offsetter<double> offset(0,limitD);
					std::transform(poly.begin( ), poly.end( ),rval.fillingIterator(outSize), offset);
					break;
				}
			case 3:
				{
					Offsetter<double> offset(limitD,limitD);
					std::transform(poly.begin( ), poly.end( ),rval.fillingIterator(outSize), offset);
					break;
				}
			default:
				assert(0);
			}
		}
		return rval;
	}
}

TEST(volume,multipoly) {
	typedef spatial::Volume<double,double,2>::VectorOfVectors VectorOfVectors;
	std::ofstream d("multipoly.m");
	d << matlabBridge::FigureName("Random multiple polygons");
	for (int i = 0; i < 5; i++) {
		for (int s = 2; s <= 4; s++) {
			spatial::Volume<double,double,2> result(multiple(s,1000,100) ); 
			VectorOfVectors vOfv= result.points();
			/*
			for (VectorOfVectors::const_iterator iter = vOfv.begin( ); iter != vOfv.end( ); ++iter) {
			matlabBridge::Polygon pPoly("g+-",1);
			frontTierAdapt::copyVectorInto(pPoly,*iter);
			d << pPoly;
			}
			*/
			matlabBridge::Polygons gons("g+-",1);
			frontTierAdapt::copyVectorsInto(gons,vOfv);
			d <<  gons << matlabBridge::pause << matlabBridge::clearFigure ; 
		}
	}
}
TEST(volume,polyaccessor) {
	 spatial::Volume<double,double,2> result(multiple(4,1000,100) ); 
	 spatial::Volume<double,double,2>::SegAccessor accessor = result.accessor( );
	 long double mSquared = 0;
	 while (accessor.hasNext( )) {
		const spatial::Segment<double,2> seg = accessor.getAndAdvance( );
		mSquared += seg.magnitude<long double>( );
	 }
	 std::cout << mSquared << std::endl;
}

namespace {
	//include in both "volume" and "persist" test suites
	void testVolumePolygonsPersistence( ) {
		using std::ios;
		using vcell_util::Logger;
		Logger & logger = Logger::get( );
		spatial::Volume<double,double,2>::registerType( );
		spatial::Volume<double,double,2> result(multiple(4,1000,100) ); 
		{
		 std::ofstream out("polyVolume.dat",ios::trunc|ios::binary);
		 Logger::Level level = logger.currentLevel( );
		 logger.set(Logger::info);
		 vcell_persist::WriteFormatter wf(out,1,true);
		 logger.set(level);
		 out.exceptions(ios::badbit|ios::failbit|ios::eofbit);
		 result.persist(out);
		 result.volume( );
		 result.persist(out);
		}
		std::ifstream in("polyVolume.dat",ios::binary);
		in.exceptions(ios::badbit|ios::failbit);
		vcell_persist::ReadFormatter wf(in,1);
		spatial::Volume<double,double,2> back(in); 
		spatial::Volume<double,double,2> back2(in); 
		double orig = result.volume( );
		double restored = back.volume( );
		double restored2 = back2.volume( );
		ASSERT_TRUE(orig == restored);
		ASSERT_TRUE(orig == restored2);
	}
}
TEST(volume,persistpoly) {
	testVolumePolygonsPersistence( );
}
TEST(persist,complexVolume) {
	testVolumePolygonsPersistence( );
}

TEST(volume,randpoly) {
	std::ofstream d("randpoly.m");
	d << matlabBridge::FigureName("Random polygons");
	for (int i = 0; i < 20; i++) {
		std::vector<spatial::TPoint<double,2>> poly = randPolygon(10000,100); 
		matlabBridge::Polygon pPoly("k+-",3);
		frontTierAdapt::copyVectorInto(pPoly,poly);
		d << pPoly <<  matlabBridge::pause << matlabBridge::clearFigure ; 
	}
}
TEST(volume,intersections) {
#ifdef VOLUME_IMPLEMENT_INSIDE
	//const int nTests = 83883495; 
	const int nTests = 100; 
	const int coordLimit = 1000;
	const int maxPoints = 200; 
	const int numberTPoints = 100; 
	std::ofstream d("volintersections.m");
	d << matlabBridge::FigureName("Intersections");
	using spatial::Volume;
	srand(static_cast<unsigned>(0));
	Volume<double,double,2> a;
	Volume<double,double,2> b;
	for (int i = 0; i < nTests;i++) {
		if (i%1000 == 1) {
			d << "%test " << i << std::endl;
		}
		int distro = rand( ) % 10;
		switch (distro) {
		case 8:
			{
				int s = 2 + rand( ) %3 ;
				a = multiple(s,coordLimit,maxPoints);
				b = randVolume(coordLimit,maxPoints);
				break;
			}
		case 9:
			{
				int s1 = 2 + rand( ) %3 ;
				int s2 = 2 + rand( ) %3 ;
				a = multiple(s1,coordLimit,maxPoints);
				a = multiple(s2,coordLimit,maxPoints);
				break;
			}
		default:
			a = randVolume(coordLimit,maxPoints);
			b = randVolume(coordLimit,maxPoints);
		}
		Volume<double,double,2> inter = a.intersection(b); 
		bool miss = false;
		for (int t = 0; t<numberTPoints;t++) {
			int x = -coordLimit + rand( ) % coordLimit / 2; 
			int y = -coordLimit + rand( ) % coordLimit / 2; 
			spatial::TPoint<double,2> rpoint(x,y);
			bool inA = a.inside(rpoint);
			bool inB = b.inside(rpoint);
			bool inInter = inter.inside(rpoint);
			if (inA && inB && !inInter) {
				d << "% Point " << rpoint << " not in intersection " << std::endl;
				miss = true;
			}
			if (inInter && !inA && !inB) {
				d << "% Point " << rpoint << " in intersection " << std::endl;
				miss = true;
			}
		}
		if (miss) {
			matlabBridge::Polygons ap("g-+",1);
			frontTierAdapt::copyVectorsInto(ap,a.points( ));
			matlabBridge::Polygons bp("b-+",1);
			frontTierAdapt::copyVectorsInto(bp,b.points( ));
			matlabBridge::Polygons intersp("r:o",3);
			frontTierAdapt::copyVectorsInto(intersp,inter.points( ));
			d << ap << bp << intersp; 
			d<<  matlabBridge::pause << matlabBridge::clearFigure ; 
		}
	}
#endif
}

TEST(volume,swaptest) {
	using spatial::Volume;
	Volume<double,double,2> a = randVolume(1000,20);
	const double theVol = a.volume( );
	Volume<double,double,2> b;
	ASSERT_TRUE(b.volume( ) == 0);
	std::swap(a,b);
	ASSERT_TRUE(a.volume( ) == 0);
	ASSERT_TRUE(b.volume( ) == theVol);
}

TEST(volume,persist) {
	std::ofstream out("volume.dat");
	using spatial::Volume;
    GTEST_SKIP() << "test needs configuration (e.g. registration of spatial::Volume)";
	Volume<double,double,2> a = randVolume(1000,20);
	a.persist(out);

	std::ifstream in("volume.dat");
	Volume<double,double,2> back(in);


}

TEST(array,exists) {
	std::array<int,4> a = {1,2,3,4}; 
	std::array<int,4> b = {1,2,3,4}; 
	bool ans = (a == b);
	std::cout << " a = b " << ans <<  std::endl;
}
