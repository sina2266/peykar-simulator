#include "MPoint.h"
#include <iostream>
#include <cassert>

#include <vector>
#include <algorithm>
#include <stack_container.h>
#include <vcellutil.h>
#include "gtest/gtest.h"
#include "Mesh.h"
#include "MeshElementNode.h"
#include "algo.h"
#include <cassert>
#include "mockpoint.inc"
using namespace spatial;
using vcell_util::arrayInit; 
TEST(mesh,gridfunctions) {
	{
		const std::array<int,2> orig = {0,0};
		const std::array<int,2> values = {30,40};
		const std::array<size_t,2> s = {3,8};
		MeshDef<int,2> mesh(orig,values,s);
		int x = mesh.greaterGridPoint(1,cX);
		int y = mesh.greaterGridPoint(1,cY);
		ASSERT_TRUE(x == 10);
		ASSERT_TRUE(y == 5);
		x = mesh.greaterGridPoint(10,cX);
		ASSERT_TRUE(x == 20);
		x = mesh.lesserGridPoint(17,cX);
		y = mesh.lesserGridPoint(17,cY);
		ASSERT_TRUE(x == 10);
		ASSERT_TRUE(y == 15);
		y = mesh.lesserGridPoint(15,cY);
		ASSERT_TRUE(y == 10);
	}

	{
		const std::array<int,2> orig = {1,-2};
		const std::array<int,2> values = {30,40};
		const std::array<size_t,2> s = {3,8};
		MeshDef<int,2> mesh(orig,values,s);
		int x = mesh.greaterGridPoint(1,cX);
		int y = mesh.greaterGridPoint(1,cY);
		ASSERT_TRUE(x == 11);
		ASSERT_TRUE(y == 3);
		x = mesh.lesserGridPoint(17,cX);
		y = mesh.lesserGridPoint(17,cY);
		ASSERT_TRUE(x == 11);
		ASSERT_TRUE(y == 13);
	}

}

TEST(mpoint,offset) {
	size_t a[] = {3,4};
	double v[] = {6.2,8,4};
	size_t b[] = {13,2};
	double r[] = {6.2,8,4};
	spatial::MPoint<double,2> mp(a,v);
	spatial::MPoint<double,2> other(b,r);
	spatial::ElementOffset<2> eo = mp.offset(other);
	std::cout << eo.offsets[0] << std::endl;
}
TEST(mpoint,indexpoint) {
	size_t a[] = {3,4};
	double v[] = {6.2,8,4};
	spatial::MPoint<double,2> mp(a,v);
	spatial::TPoint<size_t,2> ip = mp.indexPoint( );
	ASSERT_TRUE(ip(spatial::cX) == 3);
	ASSERT_TRUE(ip(spatial::cY) == 4);

}
TEST(mesh, construct) {
	using spatial::Mesh;
	using spatial::MPoint;
	using spatial::cX;
	using spatial::cY;
	spatial::MeshDef<double,2> small(arrayInit<double>(0,0),arrayInit<double>(1,1), arrayInit<size_t>(3,3));

	Mesh<double,2,MockPoint > snap(small, MockPoint::eref( ));
	std::array<size_t,2> xy;
	xy[0] = 1;
	xy[1] = 2;
	for (xy[0] = 0; xy[0] < 3; xy[0]++) {
		for (xy[1] = 0; xy[1] < 3; xy[1]++) {
			MPoint<double,2> p = snap.get(xy);
			std::cout << p(cX) << ',' << p(cY) << std::endl; 
			std::cout << p.indexInfo( ) << std::endl;
		}
	}
}
TEST(mesh, values) {
	using spatial::Mesh;
	using spatial::MPoint;
	using spatial::cX;
	using spatial::cY;
	spatial::MeshDef<double,2> sample(arrayInit<double>(-3.2,7), arrayInit<double>(0.34,3.4),arrayInit<size_t>(30,40) );
	std::vector<double> xvalues = sample.coordinateValues(cX);
	std::vector<double> yvalues = sample.coordinateValues(cY);

	Mesh<double,2,MockPoint > snap(sample, MockPoint::eref( ));
	spatial::Mesh<double,2, MockPoint>::const_iterator iter = snap.begin( );
	spatial::Mesh<double,2, MockPoint>::const_iterator end = snap.end( );
	for (;!(iter == end); ++iter) {
		const MockPoint& mp = *iter;
		const double mx = mp(cX);
		const double vx =  xvalues[mp.indexOf(cX)];
		const double my = mp(cY);
		const double vy =  yvalues[mp.indexOf(cY)];
		ASSERT_TRUE(spatial::nearlyEqual(mx,vx) );
		ASSERT_TRUE(spatial::nearlyEqual(my,vy) );
	}
}
TEST(mesh, offset) {
	using spatial::Mesh;
	using spatial::MPoint;
	using spatial::cX;
	using spatial::cY;
	spatial::MeshDef<double,2> sample(arrayInit<double>(-3.2,7), arrayInit<double>(0.34,3.4),arrayInit<size_t>(30,40) );


	Mesh<double,2,MockPoint > snap(sample, MockPoint::eref( ));
	std::array<size_t,2> indexes = { 3, 4 };

	MockPoint *one = snap.query(indexes);
	indexes[0] = 7;
	indexes[1] = 12;
	MockPoint *two = snap.query(indexes);
	spatial::ElementOffset<2> eo = one->offset(*two);
	MockPoint *back = snap.element(*one,eo);

	ASSERT_TRUE(two == back);
}
TEST(mesh, size) {
	size_t mSize = sizeof(spatial::MeshElement<double,2>);
	std::cout << "The size of  MeshElement<double,2>  is " << mSize << std::endl;
	size_t theSize = sizeof(moving_boundary::MeshElementNode);
	std::cout << "The size of  MeshElementNode  is " << theSize << std::endl;
	size_t dSize = sizeof(double);
	std::cout << "The size of  double  is " << dSize << std::endl;
	size_t pdSize = sizeof(double *);
	std::cout << "The size of  double * is " << pdSize << std::endl;
}

TEST(mesh, iterate) {
	using spatial::Mesh;
	using spatial::MPoint;
	using spatial::cX;
	using spatial::cY;
	spatial::MeshDef<double,2> small(arrayInit<double>(0,0),arrayInit<double>(1,1), arrayInit<size_t>(3,3));

	typedef MPoint<double,2> MPoint2;
	typedef Mesh<double,2,MockPoint> MMesh; 

	Mesh<double,2,MockPoint > snap(small, MockPoint::eref( ));
	MMesh::const_iterator iter = snap.begin( );
	MMesh::const_iterator  end = snap.end( );
	for (;!(iter == end); ++iter) {
		const MPoint2& mp = *iter;
		std::cout << mp << std::endl; 
	}
}

#ifdef DELETED_TEST
TEST(mesh, translate) {
	using spatial::Mesh;
	using spatial::MPoint;
	using spatial::cX;
	using spatial::cY;
	spatial::MeshDef<double,2> big(arrayInit<double>(0.2,0.3),arrayInit<size_t>(100,100));

	typedef MPoint<double,2> MPoint2;
	typedef Mesh<double,2,MockPoint> MMesh; 

	MMesh snap = big.view<MockPoint>( );
	MMesh::const_iterator iter = snap.begin( );
	MMesh::const_iterator  end = snap.end( );
	for (;!(iter == end); ++iter) {
		const MPoint2& mp = *iter;
		const size_t * idxs = mp.indexes( );
		const double x = static_cast<double>(idxs[0]);
		const double y = static_cast<double>(idxs[1]);
		spatial::TPoint<double,2> in(x,y);
		spatial::TPoint<double,2> out = snap.gridToSpatial(in); 
		/*
		if (out(cX) != mp(cX)) {
		std::cout << out(cX) << " - " << mp(cX) << " = " << (out(cX) - mp(cX) ) << std::endl;
		}
		*/
		ASSERT_TRUE( spatial::nearlyEqual<double>(out(cX),(cX), 1e-15) );
		ASSERT_TRUE( spatial::nearlyEqual<double>(out(cY),(cY), 1e-15) );
	}
}
#endif

TEST(mesh, construct3) {
	using spatial::Mesh;
	using spatial::MPoint;
	spatial::MeshDef<double,3> small(arrayInit<double>(0,0,0), arrayInit<double>(1,1,1),arrayInit<size_t>(3,3,3));
	Mesh<double,3,MockPoint3> snap(small,MockPoint3::eref( ));
	std::array<size_t,3> xy;
	xy[0] = 1;
	xy[1] = 2;
	xy[2] = 1;
	for (xy[0] = 0; xy[0] < 3; xy[0]++) {
		for (xy[1] = 0; xy[1] < 3; xy[1]++) {
			for (xy[2] = 0; xy[2] < 3; xy[2]++) {
				MPoint<double,3> p = snap.get(xy);
				std::cout << p << std::endl; 
			}
		}
	}

	std::cout << "hello" << std::endl;
}

namespace {
	struct Tracker {
		Tracker(int v=0) :value(v) {
			total++;
		}
		Tracker(const Tracker & rhs) 
			:value(rhs.value) {
				total++;
		}
		~Tracker( ) {
			assert(total >= 0);
			if (--total == 0) {
				std::cout << "all trackers deleted" << std::endl;
			}
		}
		void *operator new(size_t t) {
			return ::operator new(t);
		}
		int value;
		static int total;
	};
	int Tracker::total = 0;
}
TEST(vcellutil,chunkallocator) {
	using vcell_util::ChunkAllocator;
	{
		ChunkAllocator<Tracker,5> alloc;
		Tracker *store[100];
		for (int i = 0; i < 12; i++) {
			Tracker * t = alloc.provide( );
			t->value = i*i;
			store[i] = t;
		}
		for (int i = 0; i < 12; i++) {
			ASSERT_TRUE(store[i]->value == i*i);
		}
	}
	std::cout << "post scope" << std::endl;
}

TEST(vcellutil,stackcontainer) {
	chromium::StackAllocator<Tracker,5>::Source source;
	chromium::StackAllocator<Tracker,5> SAlloc(&source);
	typedef std::vector<Tracker,chromium::StackAllocator<Tracker,5> > Vector; 
	Vector tvec(SAlloc);
	tvec.reserve(5);
	{
		Tracker a(1);
		tvec.push_back(a);
		Tracker b(2);
		tvec.push_back(b);
	}
	std::cout << "stack values" << std::endl;
	for (Vector::const_iterator iter = tvec.begin( ); iter != tvec.end( ); ++iter) {
		std::cout << iter->value << std::endl;
	}

}
