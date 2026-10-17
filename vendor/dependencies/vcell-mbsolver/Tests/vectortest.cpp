#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <cassert>

#include <vector>
#include <algorithm>
#include "gtest/gtest.h"
#include <SVector.h>
#include <Angle.h>
#include <algo.h>
#include <cassert>

using std::cout;
using std::endl;
using spatial::TPoint; 
using spatial::SVector; 
using spatial::DegAngle; 
using spatial::RadAngle; 
using spatial::Angle; 
using spatial::pi; 
namespace {
	const char comma = ',';
}

TEST(angle,basic) {
	{
		DegAngle da(0);
		RadAngle ra(0);
		ASSERT_TRUE(da == ra);
	}
	{
		DegAngle da(180);
		RadAngle ra(pi);
		ASSERT_TRUE(da == ra);
	}
	{
		DegAngle da(360);
		RadAngle ra(2*pi);
		ASSERT_TRUE(da == ra);
	}
	DegAngle zero(0);
	ASSERT_TRUE(zero.sin( ) == 0);
	ASSERT_TRUE(zero.tan( ) == 0);
	ASSERT_TRUE(zero.cos( ) == 1);
	DegAngle thirty(30);
	ASSERT_DOUBLE_EQ(thirty.sin( ),0.5);
	DegAngle fourtyfive(45);
	ASSERT_DOUBLE_EQ(fourtyfive.tan( ),1);
	DegAngle ninety(90);
	ASSERT_DOUBLE_EQ(ninety.sin( ), 1);
	ASSERT_NEAR(ninety.cos( ), 0, 1e-15);
}
TEST(angle,range) {
	DegAngle a(350);
	DegAngle b(-10);
	ASSERT_TRUE(a == b);
	for (int i = 1; i < 11; i++) {
		DegAngle wrap(350 + i *360); 
		DegAngle nwrap(350 - i *360); 
		ASSERT_TRUE(a == nwrap);
	}
}
TEST(angle,leftRight) {
	for (int d = 0; d <= 360; d+=10) {
		for (int a = 0; a <= 170; a+=10) {
			DegAngle base(d);
			DegAngle addition(a);
			Angle right = base + addition;
			for (int s = 10; s < 180 - a; s+=10) {
				//cout << a <<  comma << d << comma << s << endl;
				ASSERT_TRUE(a + s < 180);
				DegAngle subtraction(s);
				Angle left = base - subtraction;
				Angle difference = right - left;
				ASSERT_TRUE(difference.degrees( ) < 180); 
				ASSERT_TRUE(left < right);
				ASSERT_TRUE(right > left);
			}
		}
	}
}
TEST(vector,print) {
	spatial::SVector<double,2> v2(.5,.7);
	cout << v2 << endl;
}
class vectortest : public testing::Test {
protected:
	//3-4-5 triangle
	TPoint<double,2> a;
	TPoint<double,2> b;
	SVector<double,2> vec;
	vectortest( ) 
		:a(1,2),
		b(4,6),
		vec(a,b){}
};

TEST_F(vectortest,basic) {
	ASSERT_TRUE(vec.magnitude( ) == 5);
	SVector<double,2> unit = vec;
	ASSERT_TRUE(unit.magnitude( ) == 5);
	unit.normalize( );
	ASSERT_TRUE(unit.magnitude( ) == 1);
}

TEST_F(vectortest,dot) {
	SVector<double,2> other(10,100); 
	double d = dot(vec,other);
	ASSERT_TRUE(d == 3 * 10 + 4 * 100);
}

TEST_F(vectortest,perpendicular) {
	SVector<double,2> per = vec.perpendicular( );
	double d = dot(vec,per);
	ASSERT_TRUE(d == 0);
}

TEST_F(vectortest,angle) {
	for (int d = 0; d <= 360; d+=10) {
		double length = rand( );
		const double rad = spatial::degToRad(d);
		double dx =  length * cos(rad);
		double dy =  length * sin(rad);
		SVector<double,2> v(dx,dy);
		spatial::Angle rout = v.angle( );
		ASSERT_TRUE(RadAngle(rad) == rout);
	}
}

/*
template <class T>
std::pair<bool,T> intersectParameter(const TPoint<T,2> & a, const SVector<T,2> & b , const TPoint<T,2> & c, const SVector<T,2> & d)  {
	using spatial::cX;
	using spatial::cY;
	T denom = b(cY)*d(cX) - b(cX)*d(cY);
	if (abs(denom) < 1e-10) {
		return std::pair<bool,T>(false,0);
	}

	T num   = a(cX)*b(cY) - a(cY)*b(cX) - b(cY)*c(cX) + b(cX)*c(cY); 
	T w = num / denom;
	return std::pair<bool,T>(true,w);
}

template <class T>
std::pair<bool, TPoint<T,2> > intersection(const TPoint<T,2> & a, const SVector<T,2> & b , const TPoint<T,2> & c, const SVector<T,2> & d)  {
	using spatial::cX;
	using spatial::cY;
	using std::pair;
	pair<bool,T> w = ::intersectParameter(a,b,c,d);
	if (w.first) {
		T x = c(cX) + w.second * d(cX); 
		T y = c(cY) + w.second * d(cY); 
		return pair<bool,TPoint<T,2> >(true,TPoint<T,2>(x,y));
	}
	return pair<bool,TPoint<T,2> >(false,TPoint<T,2>());
}

TEST_F(vectortest,intersect) {
	using spatial::Point2D;
	using spatial::SVector2D;
	//using spatial::intersectParameter;
	{
	cout << "one" << endl;
	Point2D g(0,0);
	SVector2D up(0,1);
	Point2D h(0,1);
	SVector2D right(1,0);
	double u = ::intersectParameter(g,up,h,right).second;
	cout << u << endl;
	using spatial::cY;
	SVector2D uphalf(0,0.5);
	u = ::intersectParameter(g,uphalf,h,right).second;
	cout << u << endl;
	}
	{
	cout << "two" << endl;
	Point2D g(1,0);
	SVector2D up(0,1);
	Point2D h(0,1);
	SVector2D right(1,1);
	double u = ::intersectParameter(g,up,h,right).second;
	cout << u << endl;
	u = ::intersectParameter(h,right,g,up).second;
	cout << u << endl;
	}
	{
	cout << "three" << endl;
	Point2D a(3,4);
	SVector2D b(1,1);
	Point2D c(10,1);
	SVector2D d(-1,1);
	double u = ::intersectParameter(a,b,c,d).second;
	cout << u << endl;
	double w = ::intersectParameter(c,d,a,b).second;
	cout << w << endl;
	}
	{
	cout << "four" << endl;
	Point2D a(3,4);
	SVector2D b(1,2);
	Point2D c(5,10);
	SVector2D d(1,1);
	double u = ::intersectParameter(a,b,c,d).second;
	cout << u << endl;
	double w = ::intersectParameter(c,d,a,b).second;
	cout << w << endl;
	Point2D i1 = intersection(a,b,c,d).second;
	Point2D i2 = intersection(c,d,a,b).second;
	cout << i1 << ',' << i2 << endl;
	}
	{
	cout << "five" << endl;
	Point2D a(3,4);
	SVector2D b(1,2);
	Point2D c(5,10);
	SVector2D d(1,2);
	bool  i = ::intersectParameter(a,b,c,d).first;
	cout << i << endl;
	}
}
*/