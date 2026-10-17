#include <vector>
#include <fstream>
#include <boundaryProviders.h>
#include "gtest/gtest.h"
#include <boundaryProviders.h>
#include <Universe.h>
#include "MBridge/MBPolygon.h"
#include "MBridge/Figure.h"
#include "MBridge/FronTierAdapt.h"
#include <type_traits> 
TEST(boundary,circle) {
	moving_boundary::Universe<2>::get( ).destroy( );
	std::array<spatial::GeoLimit,2> limits;
	limits[0] = spatial::GeoLimit(-10,10);
	limits[1] = limits[0];
	moving_boundary::Universe<2>::get( ).init(limits);

	std::ofstream d("circle.m");
	d << matlabBridge::FigureName("Circle");
	std::unique_ptr<spatial::FrontProvider<moving_boundary::CoordinateType> > circle( moving_boundary::circleFront(1,2,0.5,.01,0.7) );
	matlabBridge::Polygon c("k+-",1);
	frontTierAdapt::copyVectorInto(c,circle->retrieveFront( ));
	matlabBridge::Polygon c2("r+-",1);
	circle->propagateTo(1);
	frontTierAdapt::copyVectorInto(c2,circle->retrieveFront( ));
	d << c << c2;
}

TEST(boundary,expanding) {
	moving_boundary::Universe<2>::get( ).destroy( );
	std::array<spatial::GeoLimit,2> limits;
	limits[0] = spatial::GeoLimit(-10,10);
	limits[1] = limits[0];
	moving_boundary::Universe<2>::get( ).init(limits);

	std::ofstream d("ecircle.m");
	d << matlabBridge::FigureName("Expanding Circle");
	std::unique_ptr<spatial::FrontProvider<moving_boundary::CoordinateType> > circle( moving_boundary::expandingCircle(.05,"1 + sin(t)") );
	matlabBridge::Polygon c("k+-",1);
	auto f = circle->retrieveFront( );
	frontTierAdapt::copyVectorInto(c,f);
	matlabBridge::Polygon c2("r+-",1);
	circle->propagateTo(0.1);
	frontTierAdapt::copyVectorInto(c2,circle->retrieveFront( ));
	d << c << c2;
}

class Marker {};

struct  A : public Marker {
	int cow() const { return 7; }
};

struct B {
	int dog( ) const { return 4; }
};

template <typename T>
typename std::enable_if<std::is_base_of<Marker,T>::value,void>::type call(const T & t) {
	t.cow( );
}

template <typename T>
typename std::enable_if<!std::is_base_of<Marker,T>::value,void>::type call(const T & t) {
	t.dog( );
}



/*
template <typename T>
typename std::enable_if<is_marker<T>::value,void>::type call(const T & t) {
	t.cow( );
}

template <typename T>
typename std::enable_if<!is_marker<T>::value,void>::type call(const T & t) {
	t.dog( );
}
*/

TEST(trait,basic) {
	A a;
	B b;
	call(a);
	call(b);
}

