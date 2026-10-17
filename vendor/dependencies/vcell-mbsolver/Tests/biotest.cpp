#include <iostream>
#include <functional>
#include <algorithm>
#include "gtest/gtest.h"
#include <Physiology.h>
//#include <Species.h>
//using namespace moving_boundary::biology;
namespace {
//	void eval(const Species &sp, const std::vector<double> & v) {
//		double ns = sp.sourceTerm( ).evaluate(v);
//		std::cout << ns << " " << sp.name( ) << std::endl;
//	}
}
TEST(bio,vsum) {
	std::array<double,3> a = {1, 2, 3};
	std::array<double,3> b = {4, 5, 6};
	std::transform(a.begin( ), a.end( ), b.begin( ), a.begin( ),std::plus<double>( ));
	ASSERT_TRUE(a[0] == 5);
	ASSERT_TRUE(a[1] == 7);
	ASSERT_TRUE(a[2] == 9);
}

namespace {
	struct SpecOp {
		double operator( )(double a, double b) {
			return a + b * 2;
		}
	};
	
}
TEST(bio,specOp) {
	std::array<double,3> a = {1, 2, 3};
	std::array<double,3> b = {4, 5, 6};
	std::transform(a.begin( ), a.end( ), b.begin( ), a.begin( ),SpecOp( ));
	ASSERT_TRUE(a[0] == 9);
	ASSERT_TRUE(a[1] == 12);
	ASSERT_TRUE(a[2] == 15);
}

TEST(bio,physio) {
    GTEST_SKIP() << "framework had been redesigned (no Species), test needs maintenance";
	moving_boundary::Physiology physio;
	ASSERT_THROW(physio.numberSymbols( ),std::domain_error);
//	physio.createSpecies("dog","1 + t * cat", "3");
//	physio.createSpecies("cat","2 * cat", "0.4");
//	physio.createSpecies("cat","2 * cat", "");
//	std::array<std::string,1> syms = {"t"};
//
//	physio.buildSymbolTable<1>(syms);
//	physio.lock( );
//
//	std::vector<double> values(physio.numberSymbols( ));
//	size_t idx = physio.symbolIndex("t");
//	size_t cidx = physio.symbolIndex("cat");
//	values[idx] = 2;
//	values[cidx] = 10;
//	for (int i = 0; i < physio.numberSpecies( ) ; ++i) {
//		eval(physio.species(i), values );
//	}
//	for (auto iter = physio.beginSpecies( ); iter != physio.endSpecies( ); ++iter) {
//		std::cout << iter->name( ) << std::endl;
//		//iter->setTable(syms); correctly fails to compile
//	}
//	ASSERT_THROW(physio.createSpecies("alligator","3", "7"), std::domain_error);
}
