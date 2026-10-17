#include <random>
#include <limits>
#include "gtest/gtest.h"
#include <vcellconvert.h>
#include <Logger.h>
#include <World.h> 
	template <typename S, typename D>
	struct MaxConvert {
		MaxConvert( )
			:maxSafeV(0) {}
		S maxNum( ) const {
			return std::numeric_limits<S>::max( );
		}
		S maxSafe( ) {
			if (maxSafeV > 0)  {
				return maxSafeV;
			}
            // for large numbers, incrementing by one is time consuming
            // first check if max of S is safe before looping by 1
            {
                S s = std::numeric_limits<S>::max( );
                D temp = static_cast<D>(s);
                S back = static_cast<S>(temp);
                if (s == back) {
                    maxSafeV = s;
                    return maxSafeV;
                }
            }
            for (S s = upper( ); s <std::numeric_limits<S>::max( ) ; ++s) {
				D temp = static_cast<D>(s);
				S back = static_cast<S>(temp);
				if (s != back) {
					return maxSafeV;
				}
				maxSafeV = s;
			}
			return maxSafeV = maxNum( );
		}
		double proportion( ) {
			double n = static_cast<double>(maxSafe( ));
			double d =  static_cast<double>(maxNum( ));
			return n / d;
		}
	private:
		S upper( ) {
			S lastGood = 0;
			for (S s = 255; s < std::numeric_limits<S>::max( ) && s> 0;  s = static_cast<S>(s * 1.01) ) {
				D temp = static_cast<D>(s);
				S back = static_cast<S>(temp);
				if (s != back) {
					return lastGood;
				}
				lastGood = s;
			}
			return lastGood;
		}
		S maxSafeV;
	};

using moving_boundary::Universe;
using moving_boundary::World;
TEST(universe, basic) {
	Universe<2> &universe = Universe<2>::get( );
	universe.destroy( ); //for testing
	std::array<spatial::GeoLimit,2> limits;
	limits[0] = spatial::GeoLimit(-5,5);
	limits[1] = spatial::GeoLimit(-5,5);
	universe.init(limits);
}

TEST(universe, doubleinit) {
	Universe<2> &universe = Universe<2>::get( );
	universe.destroy( ); //for testing
	std::array<spatial::GeoLimit,2> limits;
	limits[0] = spatial::GeoLimit(-5,5);
	limits[1] = spatial::GeoLimit(-5,5);
	universe.init(limits);
	ASSERT_THROW( universe.init(limits), std::logic_error);
}

TEST(universe, badvalues) {
	ASSERT_THROW( spatial::GeoLimit(5,0), std::domain_error);
}

#ifdef REMOVED
TEST(universe, returnLimits) {
	Universe<2> &universe = Universe<2>::get( );
	const int xLow = 3;
	const int yLow = -4;
	const int xHigh = 5;
	const int yHigh = -2;
	std::array<spatial::GeoLimit,2> limits;
	limits[0] = spatial::GeoLimit(xLow,xHigh);
	//limits[spatial::cY] = spatial::GeoLimit(yLow,yHigh);
	limits[1] = spatial::GeoLimit(yLow,yHigh);
	universe.destroy( ); //for testing
	universe.init(limits);
	ASSERT_TRUE(universe.limitFor(0).low == xLow);
	ASSERT_TRUE(universe.limitFor(0).high == xHigh);
	ASSERT_TRUE(universe.limitFor(spatial::cX).low == xLow);
	ASSERT_TRUE(universe.limitFor(1).low == yLow);
	ASSERT_TRUE(universe.limitFor(1).high == yHigh);
	ASSERT_TRUE(universe.limitFor(spatial::cY).low == yLow);
	ASSERT_TRUE(universe.limits( ) == limits);
	ASSERT_TRUE(universe.limitFor(0) == limits[0]);
	ASSERT_THROW(universe.limitFor(2) == limits[0],std::domain_error);
}

TEST(universe, minAndMax) { 
	Universe<3> &universe = Universe<3>::get( );
	universe.destroy( ); //for testing
	std::array<spatial::GeoLimit,3> limits;
	limits[0] = spatial::GeoLimit(-5,8);
	limits[1] = spatial::GeoLimit(-9,5);
	limits[2] = spatial::GeoLimit(-4,5);
	universe.init(limits);
	ASSERT_TRUE(universe.maxValue( ) == 8);
	ASSERT_TRUE(universe.minValue( ) == -9);
	ASSERT_TRUE(universe.maxAbsolute( ) == 9);
}
#endif

namespace {
	const int16_t int16_tMax = 30000;
	moving_boundary::WorldMax<int16_t> ms(int16_tMax);
}


TEST(universe, cvt3) { 
	{
	MaxConvert<int64_t, double> mc;
	std::cout << "int64, mc " << mc.maxNum( ) << ", " << mc.maxSafe( ) <<", " << mc.proportion( ) << std::endl;
	}
	{
	MaxConvert<int64_t, long double> mc;
	std::cout << "int64, long double " << mc.maxNum( ) << ", " << mc.maxSafe( ) <<", " << mc.proportion( ) << std::endl;
	}
	{
	MaxConvert<int32_t, double> mc;
	std::cout << "int32, mc " << mc.maxNum( ) << ", " << mc.maxSafe( ) <<", " << mc.proportion( ) << std::endl;
	}
}
TEST(universe, cvt1) {
    GTEST_SKIP() << "skip - fails for release builds under Macos with clang";
		std::cout << std::numeric_limits<int64_t>::max( ) << std::endl; 
		vcell_util::LossyConvert<int64_t,double> lossy;
	for (int64_t probe = 1;probe < std::numeric_limits<int64_t>::max( ) && probe > 0 ;probe*=7) {
		double temp = lossy(probe);
		int64_t back = lossy(temp); 
		if (probe!= back) {
            std::cout << probe << " back converts to " << back << std::endl;
            ASSERT_TRUE(probe >= 0 && back >= 0);
		}
	}
	int i = 0;
	for (int64_t probe = std::numeric_limits<int64_t>::max( );i<100;i++, --probe) {
		double temp = lossy(probe);
		int64_t back = lossy(temp); 
		std::cout << probe << std::endl;
	}
}
TEST(universe, cvt2) {
    GTEST_SKIP() << "skip - fails for release builds under Macos with clang";
		std::cout << std::numeric_limits<int64_t>::max( ) << std::endl;
		vcell_util::LossyConvert<int64_t,long double> lossy;
	for (int64_t probe = 1;probe < std::numeric_limits<int64_t>::max( ) && probe > 0 ;probe*=7) {
		long double temp = lossy(probe);
		int64_t back = lossy(temp); 
		if (probe!= back) {
			std::cout << "long double " << std::endl;
			std::cout << probe << " back converts to " << back << std::endl;
            ASSERT_TRUE(probe >= 0 && back >= 0);
        }
	}
	int i = 0;
	for (int64_t probe = std::numeric_limits<int64_t>::max( );i<100;i++, --probe) {
		long double temp = lossy(probe);
		int64_t back = lossy(temp); 
		std::cout << probe << std::endl;
	}
}
TEST(universe, diag) { 
	Universe<2> &universe = Universe<2>::get( );
	universe.destroy( ); //for testing
	std::array<spatial::GeoLimit,2> limits;
	const int16_t xlow = -1;
	const int16_t xhigh= 2; //delta is 3
	limits[0] = spatial::GeoLimit(xlow,xhigh);
	limits[1] = spatial::GeoLimit(-2.5,1.5); //4
	const int16_t maxInWorld = 5;
	universe.destroy( ); //for testing
	universe.init(limits);
	World<double,2> & dworld = World<double,2>::get( );

	ASSERT_TRUE(dworld.diagonal( ) == maxInWorld);
	World<int16_t,2> & sworld = World<int16_t,2>::get( );
	ASSERT_TRUE(sworld.diagonal( ) == int16_tMax);

	const double zeroPoint = (xlow + xhigh)/2.0;
	const double testx = 1;
	const int16_t expected =  static_cast<int16_t>(int16_tMax * (testx - zeroPoint) / maxInWorld );
	spatial::TPoint<double,2> ref(testx,0);
	spatial::TPoint<int16_t,2> spoint = sworld.toWorld(ref);
	ASSERT_TRUE(spoint(spatial::cX) == expected);
	spatial::TPoint<double,2> back = sworld.toProblemDomain(spoint);
	ASSERT_TRUE(ref == back);
	moving_boundary::WorldToPDPointConverter<int16_t,2> pconv = sworld.pointConverter( );
	spatial::TPoint<double,2> back2 = pconv(spoint);
	ASSERT_TRUE(ref == back2);

	//moving_boundary::XtoPDConverter<int16_t,2> xconv1 =  sworld.xconverter( );
	moving_boundary::WorldToPDCoordinateConverter<int16_t,2,spatial::cX> xconv1 = sworld.coordConverter<spatial::cX>( );
	World<int16_t,2>::XConverter xconv = sworld.coordConverter<spatial::cX>( );
	double xPD = xconv(expected);
	ASSERT_TRUE(xPD == testx);

	//should not compile
	//World<int16_t,2>::ZConverter zconv = sworld.coordConverter<spatial::cZ>( );
}

#ifdef SPECIAL_TEST
//locking will mess up other test, so run this individually
TEST(universe, lock) { 
	Universe<2> &universe = Universe<2>::get( );
	universe.destroy( ); //for testing
	std::array<spatial::GeoLimit,2> limits;
	limits[0] = spatial::GeoLimit(-1,2); //3
	limits[1] = spatial::GeoLimit(-2.5,1.5); //4
	std::array<unsigned int16_t, 2> nnodes = {3, 4}; 
	universe.init(limits,nnodes,true);
	ASSERT_THROW(universe.destroy( ),std::logic_error);
}
#endif 

TEST(universe, intervals) { 
	std::default_random_engine gen;
	std::uniform_int_distribution<int> rdims(3, 90);
	uint16_t nx = static_cast<uint16_t>(rdims(gen)); 
	uint16_t ny =  static_cast<uint16_t>(rdims(gen)); 
	for (int c = 0 ; c < 100; ++c) {
		Universe<2> &universe = Universe<2>::get( );
		universe.destroy( ); //for testing
		std::array<spatial::GeoLimit,2> limits;
		limits[0] = spatial::GeoLimit(-1,2); //3
		limits[1] = spatial::GeoLimit(-2.5,1.5); //4
		std::array<uint16_t, 2> nnodes = {nx, ny}; 
		universe.init(limits,nnodes);
		World<int32_t,2> & lworld = World<int32_t,2>::get( );
		double dscale = lworld.theScale( );
		long scale = static_cast<long>(dscale);
		ASSERT_TRUE(scale == dscale);
		ASSERT_TRUE(scale % nx == 0);
		ASSERT_TRUE(scale % ny == 0);
	}
}

TEST(universe, herror) { 
	using vcell_util::Logger;
	Logger::get( ).set(Logger::Key::nodeScaling,true);
	std::default_random_engine gen;
	std::uniform_int_distribution<int> rdims(3, 450);
	std::uniform_real_distribution<double> lowlimit(-10,0);
	std::uniform_real_distribution<double> highlimit(0,10);

	double low = 0;
	double high = 0;
	double span = 0;

	for (int c = 0 ; c < 10000; ++c) {
		//current implementation has bug, freaks out if span < 1 ...
		/*
		do {
		*/
			low = lowlimit(gen);
			high = highlimit(gen);
			span = high - low;
			/*
		}
		while (span < 1);
		*/
		uint16_t nx = static_cast<uint16_t>(rdims(gen)); 
		//uint16_t ny =  static_cast<uint16_t>(rdims(gen)); 
		uint16_t ny =  nx;
		Universe<2> &universe = Universe<2>::get( );
		universe.destroy( ); //for testing
		std::array<spatial::GeoLimit,2> limits;
		limits[0] = spatial::GeoLimit(low,high); 
		limits[1] = spatial::GeoLimit(low,high);
		ASSERT_TRUE(limits[0].span( ) == span);
		ASSERT_TRUE(limits[1].span( ) == span);
		std::array<uint16_t, 2> nnodes = {nx, ny}; 
		universe.init(limits,nnodes);
		World<int32_t,2> & lworld = World<int32_t,2>::get( );
		double dscale = lworld.theScale( );
		unsigned long long scale = static_cast<unsigned long long>(dscale);
		ASSERT_TRUE(scale == dscale);
		ASSERT_TRUE(scale % nx == 0);
		ASSERT_TRUE(scale % ny == 0);
		int32_t horiz = lworld.limits()[0].span( );
		int32_t vert = lworld.limits()[1].span( );
		double dh = lworld.distanceToProblemDomain(horiz);
		double vh = lworld.distanceToProblemDomain(vert);
		double hdelta = dh - span; 
		double vdelta = vh - span; 
		double maxDelta = std::max(hdelta,vdelta);
		double rel = maxDelta / span;
		std::cout << span << " nx  " << nx << " ny " << ny << " delta " << maxDelta << " rel " << rel << std::endl;

	}
}
