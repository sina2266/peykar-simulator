#include <vector>
#include <algorithm>
#include "gtest/gtest.h"
#include <Segment.h>
using namespace spatial;
void compileTest( )  {
	//uncomment to verify compile time error
	//Segment<long,0> bad;
}

TEST(segment,construct) {
	TPoint<long,2> zero;
	Segment<long,2> s;
	ASSERT_TRUE(zero == s.a( ));
	ASSERT_TRUE(zero == s.b( ));
	TPoint<long,2> something(-4,5); 
	Segment<long,2> s2(something);
	Segment<long,2> copy(s2); 
	ASSERT_TRUE(copy == s2);
}

TEST(segment,equal) {
	TPoint<long,2> zero;
	Segment<long,2> zSeg;
	TPoint<long,2> lesser(3,4);
	TPoint<long,2> greater(4,3); 
	Segment<long,2> a(lesser,greater);
	Segment<long,2> b(greater,lesser);
	ASSERT_TRUE(a == b);
	ASSERT_FALSE(a == zSeg);
}

TEST(segment,lessthan) {
	TPoint<long,2> a(3,3);
	TPoint<long,2> b(3,3); 
	TPoint<long,2> c(4,3); 
	TPoint<long,2> d(3,4); 
	ASSERT_FALSE(a < b);
	ASSERT_FALSE(b < a);
	ASSERT_TRUE(a < c);
	ASSERT_TRUE(b < d);
	ASSERT_TRUE(d < c);

	Segment<long,2> s1(a,c);
	Segment<long,2> s2(c,a);
	Segment<long,2> s3(c,d);
	Segment<long,2> s4(a,d);

	ASSERT_FALSE(s1 < s1);
	ASSERT_FALSE(s1 < s2);
	ASSERT_FALSE(s2 < s1);
	ASSERT_TRUE(s1 < s3);
	ASSERT_FALSE(s3 < s1);
	ASSERT_TRUE(s4 < s1);
	ASSERT_FALSE(s1 < s4);

}

TEST(segment,assign) {
	TPoint<long,2> b(3,3); 
	TPoint<long,2> c(4,3); 
	Segment<long,2> s0; 
	Segment<long,2> s1(b,c);
	s0 = s1;
	ASSERT_TRUE(s0 == s1);
}

TEST(segment,sort) {
	typedef Segment<long,2> LSeg;
	TPoint<long,2> b(3,3); 
	TPoint<long,2> c(4,3); 
	TPoint<long,2> d(5,7); 
	LSeg x(b,c);
	LSeg y(c,d); 
	LSeg z(b,d); 
	std::array<LSeg,3> yin = {x ,y ,z};
	std::array<LSeg,3> yang = {x ,z,y};
	std::array<LSeg,3> zen = {z,y,x};
	std::sort(yin.begin( ),yin.end( ));
	std::sort(yang.begin( ),yang.end( ));
	std::sort(zen.begin( ),zen.end( ));
	ASSERT_TRUE(yin == yang);
	ASSERT_TRUE(yin == zen);
	ASSERT_TRUE(yang == zen);
}

TEST(segment,taxicab) {
	TPoint<long,2> b(3,3); 
	TPoint<long,2> c(4,3); 
	TPoint<long,2> d(5,7); 
	ASSERT_TRUE(taxicabDistance<int>(b,c) == 1);
	ASSERT_TRUE(taxicabDistance<int>(c,d) == 5);
}

TEST(segment,singular) {
	Segment<long,2> s0; 
	ASSERT_TRUE(s0.singular( ));
	TPoint<long,2> d(5,7); 
	TPoint<long,2> e(5,7); 
	Segment<long,2> s1(d,e); 
	ASSERT_TRUE(s1.singular( ));
	TPoint<long,2> f(3,3); 
	Segment<long,2> s2(e,f); 
	ASSERT_FALSE(s2.singular( ));
	ASSERT_TRUE(s1.singular( ));
}

TEST(segment,magnitude) {
	TPoint<char,2> b(3,2); 
	TPoint<char,2> c(7,-1); 
	const Segment<char,2> five(b,c); //3 -4 -5 triangle
	double mag = five.magnitude<double>( ); 
	ASSERT_TRUE(mag == 5);
}

TEST(segment,output) {
	TPoint<short,2> b(3,2); 
	TPoint<short,2> c(7,-1); 
	const Segment<short,2> five(b,c); //3 -4 -5 triangle
	std::cout << five << std::endl;
}

TEST(segment,handv) {
	//horizontal and vertical
	TPoint<short,2> a(3,2); 
	TPoint<short,2> b(5,2); 
	TPoint<short,2> c(5,10); 
	const Segment<short,2> one(a,b); 
	const Segment<short,2> two(c,b); 
	const Segment<short,2> three(c,a); 
	ASSERT_TRUE(one.horizontal( ));
	ASSERT_FALSE(two.horizontal( ));
	ASSERT_TRUE(two.vertical( ));
	ASSERT_FALSE(one.vertical( ));
	ASSERT_FALSE(three.vertical( ));
	ASSERT_FALSE(three.horizontal( ));

	ASSERT_TRUE(one.axialDistance(cX) == 2);
	ASSERT_TRUE(one.axialDistance(cY) == 0);
	ASSERT_TRUE(two.axialDistance(cX) == 0);
	ASSERT_TRUE(two.axialDistance(cY) == 8);
	ASSERT_TRUE(three.axialDistance(cX) == 2);
	ASSERT_TRUE(three.axialDistance(cY) == 8);



}