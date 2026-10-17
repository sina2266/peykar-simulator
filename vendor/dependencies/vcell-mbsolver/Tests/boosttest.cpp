#include "gtest/gtest.h"
#include <boost/logic/tribool.hpp>
#include <boost/logic/tribool_io.hpp>
using std::cout;
using std::endl;

TEST(boost,lcm) {
	// boost::math::lcm / boost::integer::lcm were removed from modern Boost,
	// and std::lcm is C++17 (this project is C++14), so compute lcm locally.
	auto gcd = [](int a, int b) { while (b != 0) { int t = a % b; a = b; b = t; } return a; };
	int answer = 6 / gcd(6, 4) * 4;
	ASSERT_TRUE(answer == 12);
}

TEST(boost,tribool) {
	using boost::logic::tribool;
	tribool a = true;
	tribool b = false;
	tribool c = boost::logic::indeterminate;
	bool ba = (bool)a;
	bool bb = (bool)b;
	bool bc = (bool)c;
	std::cout << a  << ',' << b  << ',' << c  << std::endl; 
	ASSERT_TRUE(a);
    ASSERT_FALSE(b);
}
