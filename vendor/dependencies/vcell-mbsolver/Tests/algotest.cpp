
#include <cassert>
#include <random>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <gtest/gtest.h>
#include "TPoint.h"
#include <VCellFront.h>
#include <cassert>
#include "algo.h"
#include <MBridge/Figure.h>
#include <MBridge/MBPolygon.h>
#include <MBridge/Scatter.h>
#include <MBridge/FronTierAdapt.h>
#include <Modulo.h>
using std::cout;
using std::endl;
using namespace spatial;

TEST(algo,midpoint) {
	TPoint<int,2> a(3,8);
	TPoint<int,2> b(5,12);
	TPoint<int,2> c(5,15);
	TPoint<int,2> ab = spatial::midPoint(a,b);
	ASSERT_TRUE(ab(cX) == 4);
	ASSERT_TRUE(ab(cY) == 10);
	TPoint<int,2> bc = spatial::midPoint(c,b);
	ASSERT_TRUE(bc(cX) == 5);
	ASSERT_TRUE(bc(cY) == 13);
}
TEST(algo,below) {
	Point2D left(0.0,0.0);
	Point2D right(2.0,2.0);
	Point2D sample(1.0,2.0);
	ASSERT_FALSE( below(sample,left,right) );
	ASSERT_FALSE( below(sample,right,left) );

	sample(cY) = 0.5;
	ASSERT_TRUE( below(sample,left,right) );
	ASSERT_TRUE( below(sample,right,left) );
}

void offset(int x, int y, double angle) {
	Point2D origin(0 + x ,0 + y);
	SVector<double,2> up(sin(angle),cos(angle));
	Point2D a(-2 + x, 1 + y);
	Point2D b(-1 + x, 1 + y);
	Point2D c(1 + x, 1 + y);
	Point2D d(2 + x, 1 + y);
	if (angle != 0) {
		using spatial::rotate;
		origin =  rotate(origin,angle);
		a =  rotate(a,angle);
		b =  rotate(b,angle);
		c =  rotate(c,angle);
		d =  rotate(d,angle);
	}
	ASSERT_TRUE(between(origin,up,b,c));
	ASSERT_FALSE(between(origin,up,a,b));
	ASSERT_FALSE(between(origin,up,c,d));
}

TEST(algo,between) {
	for (int i = 0 ; i < 100 ; i++) {
		offset(rand( ),rand( ),rand( )/360.0);
	}
}

TEST(algo,intersect) {
	using spatial::Point2D;
	using spatial::SVector2D;
	using spatial::intersectionLines;
	using spatial::intersectionLineSegment;
	using spatial::cX;
	using spatial::cY;
	Point2D a(0.0,0.0);
	SVector2D av(1,1);
	Point2D b(10,0);
	SVector2D bv(-10,10);
	std::pair<bool,Point2D> ip = intersectionLines(a,av,b,bv);
	std::pair<bool,Point2D> lineSeg = intersectionLineSegment(a,av,b,bv);
	matlabBridge::Polygon polyA("k");
	polyA.add(a(cX),a(cY));
	polyA.add(a(cX) + av(cX),a(cY) + av(cY));

	matlabBridge::Polygon polyB("k");
	polyB.add(b(cX),b(cY));
	polyB.add(b(cX) + bv(cX),b(cY) + bv(cY));

	std::ofstream itest("intersectTest.m");
	using matlabBridge::clearFigure;
	using matlabBridge::FigureName;
	itest << FigureName("intersect test") << polyA << polyB; 
	if (lineSeg.first) {
		matlabBridge::Scatter sp('g',20);
		Point2D &iSect = lineSeg.second;
		sp.add(iSect(cX),iSect(cY));
		itest << sp;
	}
	else if (ip.first) {
		matlabBridge::Scatter sp('r',20);
		Point2D &iSect = ip.second;
		sp.add(iSect(cX),iSect(cY));
		itest << sp;
	}

}
TEST(algo,developOld) {
	using spatial::Point2D;
	using spatial::SVector2D;
	using spatial::intersectionLines;
	using spatial::intersectionLineSegment;
	using spatial::cX;
	using spatial::cY;
	using spatial::Angle;
	Point2D a(0.0,0.0);
	SVector2D av(6,6);
	Point2D b(10,0);
	SVector2D bv(-10,10);
	std::pair<bool,Point2D> lineSeg = intersectionLineSegment(a,av,b,bv);
	ASSERT_TRUE(lineSeg.first);
	Point2D &iSect = lineSeg.second;
	Point2D origin(4,3); 
	SVector2D vectorA0(origin,a);
	Point2D aend = spatial::displacement(a,av);
	SVector2D vectorA1(origin,aend);

	SVector2D vectorB0(origin,b);
	Point2D bend = spatial::displacement(b,bv);
	SVector2D vectorB1(origin,bend);

	SVector2D vectorI(origin,iSect);
	Angle a0 = vectorA0.angle( );
	Angle b0 = vectorB0.angle( );
	Angle a1 = vectorA1.angle( );
	Angle b1 = vectorB1.angle( );
	Angle iAng = vectorI.angle( );
	cout << iAng.degrees() << endl;
	cout << a0.degrees() << ',' << a1.degrees( ) << endl;
	cout << b0.degrees() << ',' << b1.degrees( ) << endl;

	matlabBridge::Polygon polyA("k");
	polyA.add(a(cX),a(cY));
	polyA.add(aend(cX),aend(cY));

	matlabBridge::Polygon polyB("b");
	polyB.add(b(cX),b(cY));
	polyB.add(bend(cX),bend(cY));

	std::ofstream itest("developTest.m");
	using matlabBridge::clearFigure;
	using matlabBridge::FigureName;
	itest << FigureName("develop test") << polyA << polyB; 

	matlabBridge::Scatter sp('g',20);
	sp.add(iSect(cX),iSect(cY));
	itest << sp;

	matlabBridge::Scatter op('r',20);
	op.add(origin(cX),origin(cY));
	itest << op;

}



namespace {
	std::default_random_engine generator;
	std::uniform_int_distribution<int> distribution(0,10);
	int r10( ) {
		return distribution(generator);
	}

	spatial::Point2D rPoint( ) {
		return spatial::Point2D(r10( ),r10( ));
	}

	spatial::SVector2D rVector( ) {
		for (;;) {
			int x = r10( );
			int y = r10( );
			if (x !=0 || y!=0) {
				return spatial::SVector2D(x,y);
			}
		}
	}
}

TEST(algo,develop) {
	using spatial::Point2D;
	using spatial::SVector2D;
	using spatial::intersectionLines;
	using spatial::intersectionLineSegment;
	using spatial::cX;
	using spatial::cY;
	using spatial::Angle;
	using spatial::EdgeFindResult;
	std::ofstream itest("developTest.m");
	using matlabBridge::clearFigure;
	using matlabBridge::FigureName;
	itest << FigureName("develop test"); 
	int i = 0;
	int attempt = 1;
	while (i < 100) {
		/*
		Point2D a(0,0);
		SVector2D av(6,6);
		Point2D b(10,0);
		SVector2D bv(-10,10);
		Point2D origin(4,3); 
		*/
		Point2D a = rPoint( );
		SVector2D av = rVector( );
		Point2D b = rPoint( );
		SVector2D bv = rVector( );
		Point2D origin= rPoint( );
		attempt++;
		if (pointOn(a,av,origin) || pointOn(b,bv,origin) ){
			continue;
		}

		EdgeFindResult<double> efr;
		edgeFind(efr,origin,a,av,b,bv);
		if (!efr.found) {
			continue;
		}

		std::pair<bool,Point2D> lineSeg = intersectionLineSegment(b,bv,a,av);
		if (!lineSeg.first) {
			continue;
		}
		itest << "% attempt " << attempt << std::endl;
		attempt = 0;
		i++;

		Point2D aend  = spatial::displacement(a,av);
		Point2D bend  = spatial::displacement(b,bv);
		matlabBridge::Polygon polyA("k",2);
		polyA.add(a(cX),a(cY));
		polyA.add(aend(cX),aend(cY));

		matlabBridge::Polygon polyB("b",2);
		polyB.add(b(cX),b(cY));
		polyB.add(bend(cX),bend(cY));

		itest << clearFigure << polyA << polyB; 
		matlabBridge::Scatter op('b',20, true);
		op.add(origin(cX),origin(cY));
		itest << op;

		matlabBridge::Polygon polyE("r");
		polyE.add(efr.intersection(cX),efr.intersection(cY));
		polyE.add(efr.end(cX),efr.end(cY));
		itest << polyE;
		std::stringstream ss;
		ss << "case " << i;
		if (pointOn(a,av,origin) || pointOn(b,bv,origin) ){
			ss << " on line";
		}


		itest << matlabBridge::Text(efr.intersection(cX), efr.intersection(cY), ss.str( ).c_str( ));
		itest << matlabBridge::pause;
	}
}

#ifdef OLD_TESTS
TEST(algo,polyintersect) {
	using spatial::DegAngle;
	using spatial::Point2D;
	const int radius = 2;
	std::vector<Point2D> circle;
	for (int i = 0; i <= 360; i+=10) {
		DegAngle ang(i);
		const double x = radius*ang.cos(  );
		const double y = radius*ang.sin(  );
		circle.push_back(Point2D(x,y));
	}
	const int height = 1;
	const int width = 3;
	std::vector<Point2D> ellipse;
	for (int i = 0; i <= 360; i+=10) {
		DegAngle ang(i);
		const double x = width * ang.cos(  );
		const double y = height *ang.sin(  );
		ellipse.push_back(Point2D(x,y));
	}
	std::ofstream itest("pintersect.m");
	matlabBridge::Polygon mCircle("-+g"); 
	frontTierAdapt::copyVectorInto(mCircle,circle);
	matlabBridge::Polygon mEllipse("-+b"); 
	frontTierAdapt::copyVectorInto(mEllipse,ellipse);

	std::vector <Point2D> isect = spatial::intersection<Point2D>(Point2D(0,0), circle,ellipse);
	ASSERT_FALSE(isect.empty( ));
	matlabBridge::Polygon mIntr("--or"); 
	frontTierAdapt::copyVectorInto(mIntr,isect);

	itest << matlabBridge::FigureName("poly intersection") << mCircle << mEllipse << mIntr << std::endl;
}

std::vector<spatial::Point2D> createPoly(double pointsArray[][2], int nPoints) {
	std::vector<spatial::Point2D> rval; 
	for (int i = 0 ; i < nPoints; i++) {
		double x= pointsArray[i][0];
		double y= pointsArray[i][1];
		rval.push_back(spatial::Point2D(x,y));
	}
	return rval;
}

TEST(algo,polybox) {
	double box1[][2] = 
	{ 
		3,0,
		6,0,
		6,3,
		6,8,
		3,8,
		3,3,
		3,0 };
	double box2[][2] =
	{
		0,2,
		4,2,
		8,2,
		8,5,
		4,5,
		0,5,
		0,2};


	std::vector<Point2D> p1 = createPoly(box1,sizeof(box1)/sizeof(box1[0])); 
	std::vector<Point2D> p2 = createPoly(box2,sizeof(box2)/sizeof(box2[0])); 
	std::ofstream itest("boxintersect.m");
	matlabBridge::Polygon mBox1("-og"); 
	frontTierAdapt::copyVectorInto(mBox1,p1);
	matlabBridge::Polygon mBox2("-ob"); 
	frontTierAdapt::copyVectorInto(mBox2,p2);

	std::vector <Point2D> isect = spatial::intersection<Point2D>(Point2D(4.5,3.5), p1,p2);
	ASSERT_FALSE(isect.empty( ));
	matlabBridge::Polygon mIntr("--+r"); 
	frontTierAdapt::copyVectorInto(mIntr,isect);
	itest << matlabBridge::FigureName("box intersection") << mBox1 << mBox2 << mIntr << matlabBridge::pause << std::endl; 

	std::reverse(p1.begin( ),p1.end( ));
	mBox1.clear( );
	frontTierAdapt::copyVectorInto(mBox1,p1);
	isect = spatial::intersection<Point2D>(Point2D(4.5,3.5), p1,p2);
	mIntr.clear( );
	frontTierAdapt::copyVectorInto(mIntr,isect);
	itest <<  matlabBridge::clearFigure << mBox1 << mBox2 << mIntr << matlabBridge::pause << std::endl; 

	std::reverse(p2.begin( ),p2.end( ));
	mBox2.clear( );
	frontTierAdapt::copyVectorInto(mBox2,p2);
	isect = spatial::intersection<Point2D>(Point2D(4.5,3.5), p1,p2);
	mIntr.clear( );
	frontTierAdapt::copyVectorInto(mIntr,isect);
	itest <<  matlabBridge::clearFigure << mBox1 << mBox2 << mIntr << matlabBridge::pause << std::endl; 

	std::reverse(p1.begin( ),p1.end( ));
	mBox1.clear( );
	frontTierAdapt::copyVectorInto(mBox1,p1);
	isect = spatial::intersection<Point2D>(Point2D(4.5,3.5), p1,p2);
	mIntr.clear( );
	frontTierAdapt::copyVectorInto(mIntr,isect);
	itest <<  matlabBridge::clearFigure << mBox1 << mBox2 << mIntr << std::endl; 
}

TEST(algo,longbox) {
	double box1[][2] = 
	{ 
		3,0,
		6,0,
		//6,3,
		6,8,
		3,8,
		//3,3,
		3,0 };
	double box2[][2] =
	{
		0,2,
		4,2,
		8,2, //2
		8,5,
		4,5, //4
		0,5,
		0,2};


	std::vector<Point2D> p1 = createPoly(box1,sizeof(box1)/sizeof(box1[0])); 
	std::vector<Point2D> p2 = createPoly(box2,sizeof(box2)/sizeof(box2[0])); 
	std::ofstream itest("longboxintersect.m");
	matlabBridge::Polygon mBox1("-og"); 
	frontTierAdapt::copyVectorInto(mBox1,p1);
	matlabBridge::Polygon mBox2("-ob"); 
	frontTierAdapt::copyVectorInto(mBox2,p2);

	std::vector <Point2D> isect = spatial::intersection<Point2D>(Point2D(4.5,3.5), p1,p2);
	ASSERT_FALSE(isect.empty( ));
	matlabBridge::Polygon mIntr("--+r"); 
	frontTierAdapt::copyVectorInto(mIntr,isect);
	itest << matlabBridge::FigureName("long box intersection") << mBox1 << mBox2 << mIntr << matlabBridge::pause << std::endl; 

	std::reverse(p1.begin( ),p1.end( ));
	mBox1.clear( );
	frontTierAdapt::copyVectorInto(mBox1,p1);
	isect = spatial::intersection<Point2D>(Point2D(4.5,3.5), p1,p2);
	mIntr.clear( );
	frontTierAdapt::copyVectorInto(mIntr,isect);
	itest <<  matlabBridge::clearFigure << mBox1 << mBox2 << mIntr << matlabBridge::pause << std::endl; 

	std::reverse(p2.begin( ),p2.end( ));
	mBox2.clear( );
	frontTierAdapt::copyVectorInto(mBox2,p2);
	isect = spatial::intersection<Point2D>(Point2D(4.5,3.5), p1,p2);
	mIntr.clear( );
	frontTierAdapt::copyVectorInto(mIntr,isect);
	itest <<  matlabBridge::clearFigure << mBox1 << mBox2 << mIntr << matlabBridge::pause << std::endl; 

	std::reverse(p1.begin( ),p1.end( ));
	mBox1.clear( );
	frontTierAdapt::copyVectorInto(mBox1,p1);
	isect = spatial::intersection<Point2D>(Point2D(4.5,3.5), p1,p2);
	mIntr.clear( );
	frontTierAdapt::copyVectorInto(mIntr,isect);
	itest <<  matlabBridge::clearFigure << mBox1 << mBox2 << mIntr << matlabBridge::pause << std::endl; 

	itest <<  matlabBridge::ConsoleMessage("erasing") << std::endl; 
	p2.erase(p2.begin( ) + 4);
	p2.erase(p2.begin( ) + 2);
	mBox1.clear( );
	mBox2.clear( );
	frontTierAdapt::copyVectorInto(mBox1,p1);
	frontTierAdapt::copyVectorInto(mBox2,p2);
	isect = spatial::intersection<Point2D>(Point2D(4.5,3.5), p1,p2);
	mIntr.clear( );
	frontTierAdapt::copyVectorInto(mIntr,isect);
	itest <<  matlabBridge::clearFigure << mBox1 << mBox2 << mIntr << matlabBridge::pause << std::endl; 

	itest <<  matlabBridge::ConsoleMessage("2 reversed") << std::endl; 
	std::reverse(p2.begin( ),p2.end( ));
	mBox2.clear( );
	frontTierAdapt::copyVectorInto(mBox2,p2);
	isect = spatial::intersection<Point2D>(Point2D(4.5,3.5), p1,p2);
	mIntr.clear( );
	frontTierAdapt::copyVectorInto(mIntr,isect);
	itest <<  matlabBridge::clearFigure << mBox1 << mBox2 << mIntr << matlabBridge::pause << std::endl; 

	itest <<  matlabBridge::ConsoleMessage("1 reversed") << std::endl; 
	std::reverse(p1.begin( ),p1.end( ));
	mBox1.clear( );
	frontTierAdapt::copyVectorInto(mBox1,p1);
	isect = spatial::intersection<Point2D>(Point2D(4.5,3.5), p1,p2);
	mIntr.clear( );
	frontTierAdapt::copyVectorInto(mIntr,isect);
	itest <<  matlabBridge::clearFigure << mBox1 << mBox2 << mIntr << std::endl; 
}
#endif

TEST(modulo,basic) {
	vcell_util::Modulo<int> m(0,5);
	for (int i = 0 ; i< 10 ;i ++) {
		cout << ++m << ',';
		ASSERT_TRUE(m<5);
		ASSERT_TRUE(m>=0);
	}
	cout << endl;
	for (int i = 0 ; i< 10 ;i ++) {
		cout << --m << ',';
		ASSERT_TRUE(m<5);
		ASSERT_TRUE(m>=0);
	}
	for (int i = 0 ; i< 100 ;i ++) {
		m -= rand( );
		ASSERT_TRUE(m<5);
		ASSERT_TRUE(m>=0);
		m -= rand( );
		ASSERT_TRUE(m<5);
		ASSERT_TRUE(m>=0);
	}
	m = 3;
	ASSERT_TRUE( m == 3);
	m = 6;
	ASSERT_FALSE( m == 6);
	ASSERT_TRUE( m == 1);
	vcell_util::Modulo<int> m2(0,37);
	m2 += -1;
	ASSERT_TRUE( m2 == 36);
	vcell_util::Modulo<size_t, int> m3(0,37);
	//vcell_util::Modulo<size_t> m4(0,37);
	//vcell_util::Modulo<double> m5(0,37);

}
