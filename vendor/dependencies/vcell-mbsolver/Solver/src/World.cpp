#include <World.h>
#include <Logger.h>
#include <NumericConvert.h>
#include <map>
#include <limits>
#include <list>
#include <iomanip>

namespace {
// Local gcd/lcm to avoid dependency on boost/math/common_factor.hpp (removed
// in Boost 1.72) or boost/integer/common_factor.hpp (removed in newer Boost).
template <typename T>
T local_gcd(T a, T b) {
    while (b != 0) { T t = b; b = a % b; a = t; }
    return a;
}
template <typename T>
T local_lcm(T a, T b) {
    return (a / local_gcd(a, b)) * b;
}
} // anonymous namespace
using moving_boundary::Universe;
using spatial::GeoLimit;
using moving_boundary::WorldMax;
using moving_boundary::World;
using moving_boundary::WorldTypeBase;

/********************************************************************
* WorldMax
********************************************************************/
template <typename COORD_TYPE>
WorldMax<COORD_TYPE>::WorldMax(COORD_TYPE maxValue) {
	if (WorldTypeBase<COORD_TYPE>::maxSupported > 0) { 
		WorldTypeBase<COORD_TYPE>::maxSupported = std::min(WorldTypeBase<COORD_TYPE>::maxSupported,maxValue);
	}
	else {
		WorldTypeBase<COORD_TYPE>::maxSupported = maxValue; 
	}
}

/********************************************************************
* World 
********************************************************************/
namespace {
	/**
	* use class template for implementing "init" function because they can be partially specialized
	* the base class is non-functional to ensure appropriate specializations have been created
	*/
	template <typename WORLD_COORD, int N>
	struct WorldInit {
		/**
		* use "init" as creation function instead of constructor to avoid need to define derived class constructors
		*/
		void init( );
	};

	/**
	* double requires no scaling
	*/
	template <int N>
	struct WorldInit<double,N> {
		void init(Universe<N> & universe, double &maxSupported) {
			scale = 1; 
			diagV = universe.diagonal( ); 
			limitsWorldSystem = universe.limits( );
		}

		double scale;
		double diagV;
		std::array<spatial::TGeoLimit<double>,N> limitsWorldSystem;
	};

	/**
	* common implementation for integral types
	*/
	template <typename WORLD_COORD, int N, int SCALE_ADJUST=1>
	struct WorldInitInteger {
		void init(Universe<N> & universe, WORLD_COORD &maxSupported) {
			typedef typename Universe<N>::CountType CountType;
			//if this has not been set by a static WorldMax object, use numerics limits for max
			if (maxSupported  == 0) {
				maxSupported = std::numeric_limits<WORLD_COORD>::max( );
			}
#ifndef SPECIAL_TEST
			double diag =  universe.diagonal( );
			if (diag == 0) {
				VCELL_EXCEPTION(logic_error, "Creating World  " << typeid(WORLD_COORD).name( ) << ',' << N << " with no universe");
			}
			scale = maxSupported / diag; 
#else
			static_assert(N == 2, "2d only");
			if (universe.limitFor(0) != universe.limitFor(1) || universe.numNodes( )[0] != universe.numNodes( )[1] ) {
				throw new std::logic_error("special test requires Square universe (World.cpp)");
			}
			scale = maxSupported / universe.limitFor(0).span( );
#endif
			scale /= SCALE_ADJUST; 

			WORLD_COORD divider = 1;
			{
				//WORLD_COORD iScale = static_cast<WORLD_COORD>(scale); #this doesn't work in cases where span < 1 
				long double iScale = std::floor(scale);

				//find desired divider for iScale -- we want all mesh centers and edges to be on exact integers
				const std::array<CountType,N> & numNodes = universe.numNodes( );
				for (int i = 0; i < N; i++) {
					const CountType spaces = 2*numNodes[i];
					if (spaces > 0) {
						divider = local_lcm<WORLD_COORD>(divider,2*numNodes[i]);
					}
				}
				long double  multiplier = std::floor(iScale / divider);  
				iScale = multiplier * divider;
#				ifndef NDEBUG
				long double junk;
				assert(std::modf(iScale,&junk)== 0);
#				endif
				VCELL_KEY_LOG(trace,Key::nodeScaling,"SD:," << maxSupported << ',' << std::setprecision(16) << universe.diagonal( ) << ',' << std::fixed << scale << ',' << iScale << ','
					<< numNodes[0] << ',' << divider);
				if (iScale == 1) {
					VCELL_EXCEPTION(logic_error, "unable to find decent scale for least common multiplier of " << divider << " and scale " << scale 
						<< ", max supported is " << maxSupported);
				}

				scale = iScale;
			}
			for (int i = 0; i < N; i++) {
				WORLD_COORD low  = vcell_util::ConvertDown<WORLD_COORD>(scale * (universe.limits( )[i].low( )  - universe.zeros( )[i]));
				WORLD_COORD high = vcell_util::ConvertUp<WORLD_COORD>  (scale * (universe.limits( )[i].high( ) - universe.zeros( )[i]));
				WORLD_COORD remainder = (high - low ) % divider;
				high += (divider - remainder);
				remainder = (high - low ) % divider;
				if (remainder != 0) {
					VCELL_EXCEPTION(logic_error, " world scale not to be evenly divisible by desired scale" << scale);
				}
				limitsWorldSystem[i] = spatial::TGeoLimit<WORLD_COORD>(low,high);
			}

			diagV = vcell_util::ConvertUp<WORLD_COORD>(scale * universe.diagonal( ));
			static_assert(std::numeric_limits<WORLD_COORD>::is_integer,"non-integer type"); 
			VCELL_LOG(info,"World type is " <<  std::numeric_limits<WORLD_COORD>::digits << " digits, scale is " << scale);
		}
		typename vcell::Type<WORLD_COORD>::realType scale;
		WORLD_COORD diagV;
		std::array<spatial::TGeoLimit<WORLD_COORD>,N> limitsWorldSystem;
	};

	template <int N>
	struct WorldInit<int16_t,N> : public WorldInitInteger<int16_t,N> {
	};
	/**
	* adjust scale by 4 to keep doubles produced by Voronoi32.cpp within range
	*/
	template <int N>
	struct WorldInit<int32_t,N> : public WorldInitInteger<int32_t,N,4> {
	};
	template <int N>
	struct WorldInit<int64_t,N> : public WorldInitInteger<int64_t,N> {
	};
}

template <typename WORLD_COORD, int N>
World<WORLD_COORD,N>::World( )
	:univ(Universe<N>::get( )),
	diagV( ),
	limitsWorldSystem( ),
	scale( )
{ 
	WorldBase<N>::nextWorld = univ.worlds;
	univ.worlds = this;
	init( ); 
}

template <typename WORLD_COORD, int N>
void World<WORLD_COORD,N>::init( ) {
	WorldInit<WORLD_COORD,N> helper;
	helper.init(univ,WorldTypeBase<WORLD_COORD>::maxSupported);
	scale = helper.scale;
	diagV = helper.diagV;
	limitsWorldSystem = helper.limitsWorldSystem;
}

template <typename WORLD_COORD, int N>
void World<WORLD_COORD,N>::destroy( ) {
	diagV = 0;
	scale = 0;
	limitsWorldSystem.fill(spatial::TGeoLimit<WORLD_COORD>( ) );
}

template <typename WORLD_COORD, int N>
void World<WORLD_COORD,N>::persist(std::ostream &os) const {
	vcell_persist::Token::insert<World<WORLD_COORD,N> >(os);
	vcell_persist::binaryWrite(os, WorldTypeBase<WORLD_COORD>::maxSupported);
	vcell_persist::binaryWrite(os,diagV);
	vcell_persist::save(os,limitsWorldSystem);
	vcell_persist::binaryWrite(os,scale);
}

template <typename WORLD_COORD, int N>
void World<WORLD_COORD,N>::restore(std::istream &is) {
	vcell_persist::Token::check<World<WORLD_COORD,N> >(is);
	WORLD_COORD maxReadback;
	
	vcell_persist::binaryRead(is,maxReadback);
	vcell_persist::binaryRead(is,diagV);
	vcell_persist::restore(is,limitsWorldSystem);
	vcell_persist::binaryRead(is,scale);
	if (maxReadback != WorldTypeBase<WORLD_COORD>::maxSupported) {
		VCELL_EXCEPTION(out_of_range, vcell_persist::getTypeToken(typeid(*this)) << " max supported readback " << maxReadback
			<< " differs from existing " << WorldTypeBase<WORLD_COORD>::maxSupported)
	}
}
template <typename WORLD_COORD, int N>
void World<WORLD_COORD,N>::registerType( ) {
	vcell_persist::Registrar::reg<World<WORLD_COORD,N>,WORLD_COORD,N>("World");
}

template <typename WORLD_COORD, int N>
World<WORLD_COORD,N> & World<WORLD_COORD,N>::get( ) {
	static World<WORLD_COORD,N> w;
	return w;
}

template <typename COORD_TYPE>
COORD_TYPE WorldTypeBase<COORD_TYPE>::maxSupported;

/********************************************************************
* instantations
********************************************************************/
//template struct World<moving_boundary::CoordinateType,2>;
namespace moving_boundary {
	template struct World<double,2>;

	template struct World<int16_t,2>;
	template struct World<int32_t,2>;
	template struct World<int64_t,2>;

	template struct WorldMax<int16_t>;
	template struct WorldMax<int32_t>;
	template struct WorldMax<long long>;
	
	#ifdef COMPILE_64
	template struct World<int64_t,2>;
	template struct WorldMax<int64_t>;
	#endif
}
