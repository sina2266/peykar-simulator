#ifndef MovingBoundaryTypes_h
#define MovingBoundaryTypes_h
#pragma warning ( disable : 4244)
#include <cstdint>

#define DIM 2
#define MAX_DIM 3

namespace vcell {
	template<typename T>
	struct Type{};

	template<>
	struct Type<int16_t> {
		typedef double realType;
	};

	template<>
	struct Type<int32_t> {
		typedef double realType;
	};

	template<>
	struct Type<int64_t> {
		typedef long double realType;
	};


}
//#define COMPILE_64
namespace spatial {
	template <class COORD_TYPE,class VALUE_TYPE,int N>
	struct Volume;
}
/**
* implementation type definition
*/
namespace moving_boundary {
    enum EXTRAPOLATION_METHOD
    {
        NEAREST_NEIGHBOR=1,
    };
    
    static constexpr const int MAXD = 3;

    //PRIMARY TYPES
    /**
    * type used for coordinate system of problem implementation
    */
    typedef int32_t CoordinateType;
    /**
    * type used for distances of problem implementation
    * use double for int32_t, long double for int64_t
    */
    typedef vcell::Type<CoordinateType>::realType DistanceType; 
    /**
    * type used for products/powers of #CoordinateType 
    * necessary to prevent overranging limited value CoordinateTypes
    */
    typedef DistanceType CoordinateProductType; 
    /**
    * type used for #spatial::NormVector
    */
    typedef DistanceType NormalComponent; 
    /**
    * type used for areas/volumes, et. al in problem implementation
    */
    typedef CoordinateProductType VolumeType;
    /**
    * type used for masses, concentrations, diffusion constants et. al. in problem implementation
    */
    typedef DistanceType BioQuanType;
    /**
    * type for times 
    */
    typedef DistanceType TimeType;
    /**
    * type for velocities 
    */
    typedef DistanceType VelocityType;
    //DERIVATIVE TYPES
    /**
    * typedef for object used to represent Volumes
    */
    typedef spatial::Volume<CoordinateType,CoordinateProductType,2> Volume2DClass;

    typedef decltype(VolumeType( ) * TimeType( ) ) VolumeTimeProduct;
}
#endif
