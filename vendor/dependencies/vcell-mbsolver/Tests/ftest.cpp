#include <MPoint.h>
#include <sys/stat.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cassert>
#include <Expression.h>
#include <SimpleSymbolTable.h>

#include <vector>
#include <algorithm>
#include "gtest/gtest.h"
#include <Voronoi.h>
#include <VoronoiMesh.h>
#include <algo.h>
#include <vcellutil.h>
#include <Mesh.h>
#include <MeshElementNode.h>
#include <VCellFront.h>
#include <Logger.h>
#include <Timer.h>
#include <Modulo.h>
#include <MovingBoundaryParabolicProblem.h>
#include <MBridge/Scatter.h>
#include <MBridge/Figure.h>
#include <MBridge/MBPolygon.h>
#include <MBridge/MBPatch.h>
#include <MBridge/MBMovie.h>
#include <MBridge/MatlabDebug.h>
#include <MBridge/FronTierAdapt.h>
#include <vhdf5/dataset.h>
#include <vhdf5/attribute.h>
#include <vhdf5/facade.h>
using std::cout;
using std::endl;
/*********************************************
* Frontier library evaluation 
*********************************************/
namespace {
	double evalLevelFunc(POINTER unused, double *in) { 
		double x = in[0];
		double y = in[1];
		double dx = x;
		double dy = y;
		double v = dx*dx + dy*dy - 1; 
		return v;
	}
	int evalVelocityFunction(POINTER userdata,Front* f,FT_POINT* p,HYPER_SURF_ELEMENT*hse, HYPER_SURF*hs,double *in) {
		double inputX = p->_coords[0];
		double inputY = p->_coords[1];
		double R = sqrt(inputX*inputX+inputY*inputY);
		double velocityX = -0.1*sin(f->time)*inputX/R;
		double velocityY = -0.1*sin(f->time)*inputY/R;
		in[0] = velocityX; 
		in[1] = velocityY; 
		return 0; 
	}
}
TEST(frontier,direct) {
	//run to
	const double END_TIME = 0.5;
	//boundary box of problem
	double xStart = -2.5;
	double xEnd= 2.5;
	double yStart = -2.5;
	double yEnd= 2.5;
	//CSV file name
	const char * const filename = "curveinfo.csv";



	using spatial::Point2D;
	using spatial::GeoLimit;
	std::vector<GeoLimit> limits;
	limits.push_back(GeoLimit(xStart,xEnd));
	limits.push_back(GeoLimit(yStart,yEnd));
    GTEST_SKIP() << "Skipping test, need to create MovingBoundarySetup test fixture";
    moving_boundary::MovingBoundarySetup setup; // needs initialization
    //spatial::VCellFront<double> front(limits, 175,1.5, evalLevelFunc,evalVelocityFunction);
    spatial::VCellFront<double> front(limits, setup, evalLevelFunc,evalVelocityFunction);
	std::ofstream csv(filename);

	front.retrieveFront(csv);
	front.propagateTo(END_TIME, csv);
	front.retrieveFront(csv);
}




/*********************************************
* C++ devel testing
*********************************************/
using vcell_util::Logger;
using vcell_util::FileDest;
typedef double TestWorldType;
typedef moving_boundary::MovingBoundaryParabolicProblem TestMovingBoundaryProblem; 
/*
void show(const spatial::Voronoi2D &v) {
using spatial::Point2D;
using spatial::cX;
using spatial::cY;
for (size_t i = 0; i < v.numberCells( ); i++) {
const Point2D &  centerPoint = v.cell(i);
std::cout << "cell " << i << " (" << centerPoint(cX) << ',' << centerPoint(cY) << ')' << std::endl;
std::vector<Point2D> points = v.getVertices(i);
std::vector<Point2D>::iterator iter = points.begin( );
for (;iter != points.end( );++iter) {
Point2D &p = *iter;
std::cout << " (" << p(cX) << ',' << p(cY) << ')' << std::endl;
}
}
}
*/
namespace {
	double levelFunc(POINTER userdata, double *in) { 
		double x = in[0];
		double y = in[1];
		double dx = x - 1.1;
		double dy = y - 1.112857;
		double v = dx*dx + dy*dy - 0.25; 
		return v;
	}
	double levelFuncFingers(POINTER userdata, double *in) { 
		const double Xi = 1;
		//solid block zone
		const double XL = 2;
		//finger zone
		const double Xf = 4;
		const double xShort = 3;

		const double Yi = 1;
		const double Yf = 2;
		const double dY = 0.1;
		const double xLong = 4.9; 

		double x = in[0];
		double y = in[1];
		if (x < Xi  || x >Xf) { 
			return 0;
		}
		if (y < Yi || y > Yf) {
			return 0;
		}
		if (x < XL) {
			return 2;
		}
		int zone = static_cast<int>( (y - Yi) / dY);
		if (zone % 2  == 1) {//odd zones
			if (zone > 1 &&  x > xShort) {
				return 0;
			}
			return 2;
		}
		return 0;
	}

	int velFunction(POINTER userdata,Front* f,FT_POINT* p,HYPER_SURF_ELEMENT*hse, HYPER_SURF*hs,double *in) {
		in[0] = 1;
		in[1] = 2;
		return 0; 
	}

	struct mylevel : public spatial::FronTierLevel {
		double  level(double *in) const {
			double x = in[0];
			double y = in[1];
			double dx = x - 1.1;
			double dy = y - 1.112857;
			double v = dx*dx + dy*dy - 0.25; 
			return v;
		}
	};

	struct myvel: public spatial::FronTierVelocity {
		int velocity(Front*,FT_POINT*,HYPER_SURF_ELEMENT*, HYPER_SURF*,double *) const {
			return 1;
		}
	};
}

TEST(frontier,level) {
	mylevel lvl;
	double xy[2];
	xy[1] = 1.5;

	for (double x = 0; x < 5.0 ; x+=0.1) {
		xy[0] = x;
		double v = lvl.level(xy);
		cout << x << ' ' << v << std::endl;
	}

}
TEST(frontier,repro) {
	spatial::FronTierVelocityFunction vf = velFunction;
	using spatial::GeoLimit;
	std::vector<GeoLimit> limits;
	limits.push_back(GeoLimit(0,5));
	limits.push_back(GeoLimit(0,2));
    GTEST_SKIP() << "Skipping test, need to create MovingBoundarySetup test fixture";
    moving_boundary::MovingBoundarySetup setup;
//    spatial::VCellFront<double> front(limits, 175,0, levelFunc,velFunction);
    spatial::VCellFront<double> front(limits, setup, levelFunc,velFunction);
}

TEST(frontier,obj) {
	spatial::FronTierVelocityFunction vf = velFunction;
	using spatial::GeoLimit;
	std::vector<GeoLimit> limits;
	limits.push_back(GeoLimit(0,5));
	limits.push_back(GeoLimit(0,2));
	mylevel lv;
	myvel vel;
    GTEST_SKIP() << "Skipping test, need to create MovingBoundarySetup test fixture";
    moving_boundary::MovingBoundarySetup setup;
//    spatial::VCellFront<double> front(limits, 175,0, lv,vel);
	spatial::VCellFront<double> front(limits, setup, lv,vel);
	using spatial::Point2D; 
	using spatial::cX; 
	using spatial::cY; 
	using std::vector; 
	vector<Point2D> points = front.retrieveSurf( );

	for (vector<Point2D>::const_iterator iter = points.begin( );iter != points.end( );++iter) {
		//cout << iter->get(cX) << ',' << iter->get(cY) << endl;
		cout << *iter << endl; 
	}
}

namespace {
	struct SimpleAlloc : public vcell_util::Allocator<spatial::VoronoiResult> {
		SimpleAlloc( )
			:i(0),
			pool( ) {}
		spatial::VoronoiResult * provide( ) {
			assert (i<number);
			return &pool[i++]; 
		}
	private:
		const static int number = 200;
		int i;
		spatial::VoronoiResult pool[number];
	};
	SimpleAlloc sa;
}

//TEST(frontier,classify) {
//	using moving_boundary::FrontPointType;
//	spatial::FronTierVelocityFunction vf = velFunction;
//	using spatial::GeoLimit;
//	std::vector<GeoLimit> limits;
//	limits.push_back(GeoLimit(0,5));
//	limits.push_back(GeoLimit(0,3));
//	mylevel lv;
//	myvel vel;
//	spatial::VCellFront<moving_boundary::CoordinateType> front(limits, 175,1.5, lv,vel); 
//	using spatial::Point2D; 
//	using spatial::cX; 
//	using spatial::cY; 
//	using spatial::Mesh;
//	using spatial::MeshElement;
//	using vcell_util::arrayInit;
//	using std::vector; 
//	vector<FrontPointType> points = front.retrieveSurf( );
//	matlabBridge::Polygon pgon("k");
//	for (vector<FrontPointType>::const_iterator iter = points.begin( );iter != points.end( );++iter) {
//		const FrontPointType & p = *iter;
//		pgon.add(p(cX),p(cY));
//	}
//	moving_boundary::VoronoiMesh::MBMeshDef trial(arrayInit<moving_boundary::CoordinateType>(0,0),arrayInit<moving_boundary::CoordinateType>(5,2), arrayInit<size_t>(35,35));
//
//
//	typedef moving_boundary::MeshElementNode MPoint2;
//	moving_boundary::VoronoiMesh::MBMesh mView(trial);
//	moving_boundary::VoronoiMesh vm(mView);
//	moving_boundary::Positions<MPoint2> positions = vm.classify2(points);
//	std::ofstream matlabScript("showit.m");
//	matlabBridge::Scatter iScat('b',10);
//
//
//	for (vector<MPoint2 *>::const_iterator iter = positions.inside.begin( );iter != positions.inside.end( );++iter) {
//		const MPoint2 & mp = **iter;
//		iScat.add(mp(cX), mp(cY));
//	}
//
//	matlabBridge::Scatter bScat('r',20);
//	for (vector<MPoint2 *>::const_iterator iter = positions.boundary.begin( );iter != positions.boundary.end( );++iter) {
//		const MPoint2 & mp = **iter;
//		bScat.add(mp(cX), mp(cY));
//	}
//
//	matlabBridge::Scatter oScat('g',10);
//	for (vector<MPoint2 *>::const_iterator iter = positions.outside.begin( );iter != positions.outside.end( );++iter) {
//		const MPoint2 & mp = **iter;
//		oScat.add(mp(cX), mp(cY));
//	}
//	matlabScript << matlabBridge::FigureName("demo") << iScat << bScat << oScat << pgon;
//}
//TEST(frontier,fronttest) {
//	using spatial::GeoLimit;
//	using spatial::Point2D;
//	using spatial::TPoint;
//	spatial::FronTierVelocityFunction vf = velFunction;
//	std::vector<GeoLimit> limits;
//	limits.push_back(GeoLimit(0,5));
//	limits.push_back(GeoLimit(0,3));
//	mylevel lv;
//	myvel vel;
//	//spatial::VCellFront<double> front(limits, 175,1.5, lv,vel); 
//	//spatial::VCellFront<double> front(limits, 175,1.5, levelFuncFingers,vf );
//	spatial::VCellFront<double> front(limits, 100,1.5, levelFunc,vf );
//	using spatial::cX; 
//	using spatial::cY; 
//	using spatial::Mesh;
//	using spatial::MeshElement;
//	typedef std::vector<TPoint<double,2> > DVector; 
//	typedef std::vector<DVector> vectorVector; 
//	std::ofstream matlabScript("polys.m");
//	matlabScript << matlabBridge::FigureName("curves") << matlabBridge::clearFigure;
//	vectorVector curves = front.retrieveCurves( );
//	const char *colors[] = {"k","r","g","b","y"};
//	int c = 0;
//	std::cout << "Number curves " << curves.size( ) << std::endl;
//	for (vectorVector::iterator iter = curves.begin( ); iter != curves.end( );++iter) {
//
//		matlabBridge::Polygon pgon(colors[c++]);
//		assert (c <= sizeof(colors)/sizeof(colors[0]) );
//
//		for (DVector::iterator piter = iter->begin( );piter != iter->end( );++piter) {
//			const Point2D & p = *piter;
//			pgon.add(p(cX),p(cY));
//		}
//		matlabScript << pgon; 
//	}
//}
TEST(frontier,propagate) {
	//const int NUMBER_STEPS = 1;
	//const double TIME_INCREMENT = 0.1;
	const double END_TIME = 0.5;
	using spatial::Point2D; 
	spatial::FronTierVelocityFunction vf = velFunction;
	using spatial::GeoLimit;
	std::vector<GeoLimit> limits;
	limits.push_back(GeoLimit(0,5));
	limits.push_back(GeoLimit(0,3));
    GTEST_SKIP() << "Skipping test, need to create MovingBoundarySetup test fixture";
    moving_boundary::MovingBoundarySetup setup;
//    spatial::VCellFront<double> front(limits, 175,1.5, levelFunc,vf );
	spatial::VCellFront<double> front(limits, setup, levelFunc,vf );
	using spatial::cX; 
	using spatial::cY; 
	using spatial::Mesh;
	using spatial::MeshElement;
	typedef std::vector<std::vector<Point2D> > vectorVector; 
	std::ofstream matlabScript("propagate.m");
	matlabScript << matlabBridge::FigureName("curves") << matlabBridge::clearFigure;
	vectorVector curves = front.retrieveCurves( );
	const char *colors[] = {"k","r","g","b","y"};
	vcell_util::Modulo<int> c(0,sizeof(colors));
	for (vectorVector::iterator iter = curves.begin( ); iter != curves.end( );++iter) {

		matlabBridge::Polygon pgon(colors[c++]);
		assert (c <= sizeof(colors)/sizeof(colors[0]) );
		std::vector<Point2D> points = *iter;

		for (std::vector<Point2D>::iterator piter = points.begin( );piter != points.end( );++piter) {
			const Point2D & p = *piter;
			pgon.add(p(cX),p(cY));
		}
		matlabScript << pgon; 
	}
	 {
		matlabScript << matlabBridge::pause; 
		front.propagateTo(END_TIME);
		curves = front.retrieveCurves( );
		for (vectorVector::iterator iter = curves.begin( ); iter != curves.end( );++iter) {

			matlabBridge::Polygon pgon(colors[++c]);
			assert (c <= sizeof(colors)/sizeof(colors[0]) );
			std::vector<Point2D> points = *iter;

			for (std::vector<Point2D>::iterator piter = points.begin( );piter != points.end( );++piter) {
				const Point2D & p = *piter;
				pgon.add(p(cX),p(cY));
			}
			matlabScript << pgon; 
		}
	}
}

TEST(frontier,store) {
	//const int NUMBER_STEPS = 1;
	//const double TIME_INCREMENT = 0.1;
	const double MID_TIME = 0.5;
	const double END_TIME = 1.0;
	using spatial::Point2D; 
	spatial::FronTierVelocityFunction vf = velFunction;
	using spatial::GeoLimit;
	std::vector<GeoLimit> limits;
	limits.push_back(GeoLimit(0,5));
	limits.push_back(GeoLimit(0,3));
    GTEST_SKIP() << "Skipping test, need to create MovingBoundarySetup test fixture";
    moving_boundary::MovingBoundarySetup setup;
//    spatial::VCellFront<double> front(limits, 175,1.5, levelFunc,vf );
	spatial::VCellFront<double> front(limits, setup, levelFunc,vf );
	using spatial::cX; 
	using spatial::cY; 
	using spatial::Mesh;
	using spatial::MeshElement;
	typedef std::vector<std::vector<Point2D> > vectorVector; 
	std::ofstream matlabScript("store.m");
	const char *colors[] = {"k","r","g","b","y"};
	vcell_util::Modulo<int> c(0,sizeof(colors));
	matlabScript << matlabBridge::FigureName("store") << matlabBridge::clearFigure;
	front.propagateTo(MID_TIME);
	vectorVector curves = front.retrieveCurves( );
	for (vectorVector::iterator iter = curves.begin( ); iter != curves.end( );++iter) {

		matlabBridge::Polygon pgon(colors[++c]);
		assert (c <= sizeof(colors)/sizeof(colors[0]) );
		std::vector<Point2D> points = *iter;

		for (std::vector<Point2D>::iterator piter = points.begin( );piter != points.end( );++piter) {
			const Point2D & p = *piter;
			pgon.add(p(cX),p(cY));
		}
		matlabScript << pgon; 
	}
}
