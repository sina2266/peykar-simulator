#ifndef World_h
#define World_h
#include <MovingBoundaryTypes.h>
#include <limits>
#include <TPoint.h>
#include <Universe.h>
#include <SVector.h>
#include <CoordVect.h>

namespace moving_boundary {
	//forward
	template <typename COORD_TYPE,int NUM_DIM, int A>
	struct WorldToPDCoordinateConverter;

	template <typename COORD_TYPE,int NUM_DIM>
	struct WorldToPDPointConverter;

	/**
	* if a subsytem cannot handle std::numeric_limits<COORD_TYPE>::max( ),
	* they can specify their max value here. This must be done before init.
	* This class is designed to be used as static object to
	* cause execute before main( ). 
	*/
	template <typename COORD_TYPE>
	struct WorldMax {
		WorldMax(COORD_TYPE maxValue); 
	};

	/*****
	* world base clase, for dimensions
	* @ tparam N number of dimensions
	*/
	template<int N>
	struct WorldBase {
		/**
		* destroy (for testing)
		*/
		virtual void destroy( ) = 0;
		/**
		* constructor functionality in separate module
		to allow recreation for tests
		*/
		virtual void init( ) = 0;
		//avoid header dependency by doing linked list old-fashioned way
		WorldBase<N> *nextWorld;
		WorldBase( )
			:nextWorld(nullptr) {}
	};

	/**
	* world base clase, for dimensions. 
	* class to store maximum supported value for a type independent of number
	* of dimensions
	* @tparam COORD_TYPE coordinate type
	*/
	template <typename COORD_TYPE>
	struct WorldTypeBase {
		/**
		* maximum value for this type supported by subsystems
		*/
	protected:
		static COORD_TYPE maxSupported;
		friend struct WorldMax<COORD_TYPE>;
	};

	/**
	* Problem domain values converted to a particular coordinate type.
	* There is a single universe per simulation, but there can be 
	* multiple worlds
	* @tparam NUM_DIM number of dimensions supported
	* @tparam COORD_TYPE type needed for modeling or API
	*/
	template <typename COORD_TYPE,int NUM_DIM>
	struct World : public WorldBase<NUM_DIM>, public WorldTypeBase<COORD_TYPE> {
		/**
		* return singleton instance
		*/
		static World & get( );

		/**
		* convert from problem domain scaling to World coordinates. 
		* integral types truncated
		*/
		spatial::TPoint<COORD_TYPE,NUM_DIM> toWorld(const spatial::TPoint<double,NUM_DIM> & problemDomainPoint) const {
			spatial::TPoint<COORD_TYPE,NUM_DIM> rval; 
			for (spatial::Axis a = spatial::axisInitial; a < NUM_DIM; ++a) {
				double v  = scale * (problemDomainPoint(a) - univ.inputZeroPoint[a]);
				rval(a) = static_cast<COORD_TYPE>(v);
			}
			return rval;
		}

		/**
		* convert from World coordinate to problem domain
		* @param cooord to convert
		* @a axis coordinates is on
		*/
		double toProblemDomain(COORD_TYPE coord, spatial::Axis a) const {
			return coord / scale + univ.inputZeroPoint[a];
		}
		/**
		* convert interval (distance) from World coordinate to problem domain
		* @param distance  to convert
		*/
		double distanceToProblemDomain(COORD_TYPE distance) const {
			return distance / scale;
		}

		/**
		* convert from World coordinates to problem domain
		*/
		spatial::TPoint<double,NUM_DIM> toProblemDomain(const spatial::TPoint<COORD_TYPE,NUM_DIM> & worldPoint) const {
			spatial::TPoint<double,NUM_DIM> rval; 
			for (spatial::Axis a = spatial::axisInitial; a < NUM_DIM; ++a) {
				//rval(a) = worldPoint(a) / scale + Universe<NUM_DIM>::get( ).inputZeroPoint[a];
				rval(a) = toProblemDomain(worldPoint(a),a);
			}
			return rval;
		}
		/**
		* convert from World coordinates *to problem domain
		* @param source input values in World equivalent, array length #NUM_DIM
		* @param destination ouput values in problem, array length #NUM_DIM
		*/
		void toProblemDomain(const double *source, double * destination) const {
			for (spatial::Axis a = spatial::axisInitial; a < NUM_DIM; ++a) {
				destination[a] = source[a] / scale + univ.inputZeroPoint[a];
			}
		}

		CoordVect toProblemDomain(const CoordVect& cv) const
		{
			CoordVect pv = cv / scale + univ.inputZeroPoint;
			return pv;
		}

		/**
		* convert from problem domain coordinates to World 
		* (vector conversion does not user zero offset)
		*/
		template <typename T>
		spatial::SVector<T,NUM_DIM> toWorld(const spatial::SVector<double,NUM_DIM> & pdVelocity) const {
			spatial::SVector<T,NUM_DIM> rval; 
			for (spatial::Axis a = spatial::axisInitial; a < NUM_DIM; ++a) {
				rval(a) = static_cast<T>(scale * pdVelocity(a));
			}
			return rval;
		}

		/**
		* convert from problem domain coordinates to World
		* (vector conversion does not user zero offset)
		*/
		CoordVect toWorld(CoordVect& cv) const
		{
			CoordVect pv = cv * scale;
			return pv;
		}

		/**
		* convert from these coordinates to other world coordinates
		*/
		template <typename OTHER_TYPE>
		spatial::TPoint<OTHER_TYPE,NUM_DIM> toOtherWorld(const spatial::TPoint<COORD_TYPE,NUM_DIM> & ourPoint) const {
			static double cFactor = World<OTHER_TYPE,NUM_DIM>::get( ).scale / scale; 
			using spatial::Axis;
			spatial::TPoint<OTHER_TYPE,NUM_DIM> rval; 
			for (Axis a; a < NUM_DIM; ++a) {
				rval(a) = cFactor * ourPoint(a); 
			}
			return rval;
		}


		/**
		* @return length of diagonal to world
		*/
		COORD_TYPE diagonal( ) const {
			return diagV;
		}

		const std::array<spatial::TGeoLimit<COORD_TYPE>,NUM_DIM> limits( ) const { 
			return limitsWorldSystem;
		}

		virtual void destroy( ); 

		double theScale( ) const {
			return scale;
		}

		const Universe<NUM_DIM> & universe( ) const {
			return univ;
		}

		WorldToPDPointConverter<COORD_TYPE,NUM_DIM> pointConverter( ) const {
			return WorldToPDPointConverter<COORD_TYPE,NUM_DIM>(*this); 
		}

		/**
		* @tparam A axis to convert
		*/
		template <int A>
		WorldToPDCoordinateConverter<COORD_TYPE,NUM_DIM,A> coordConverter( ) const {
			return WorldToPDCoordinateConverter<COORD_TYPE,NUM_DIM,A>(*this);
		}
		//convenience typedefs
		typedef WorldToPDCoordinateConverter<COORD_TYPE,NUM_DIM,spatial::cX> XConverter; 
		typedef WorldToPDCoordinateConverter<COORD_TYPE,NUM_DIM,spatial::cY> YConverter; 
		typedef WorldToPDCoordinateConverter<COORD_TYPE,NUM_DIM,spatial::cZ> ZConverter; 
		typedef WorldToPDPointConverter<COORD_TYPE,NUM_DIM> PointConverter;

		void persist(std::ostream &) const;
		void restore(std::istream &);
		static void registerType( ); 

	private:

		World( );

		/**
		* constructor functionality in separate module
		to allow recreation for tests
		*/
		virtual void init( );
		Universe<NUM_DIM> & univ;
		COORD_TYPE diagV;
		std::array<spatial::TGeoLimit<COORD_TYPE>,NUM_DIM> limitsWorldSystem;
		/**
		* double to #COORD_TYPE factor
		*/
		double scale;
	};

	/**
	* Functor to convert x coordinates converted to problem domain 
	* @tparam NUM_DIM number of dimensions supported
	* @tparam COORD_TYPE type needed for modeling or API
	* @tparam A axis converted 
	*/
	template <typename COORD_TYPE,int NUM_DIM, int A>
	struct WorldToPDCoordinateConverter{
		static_assert(A<NUM_DIM, "unsupported dimension");
		WorldToPDCoordinateConverter(const World<COORD_TYPE,NUM_DIM> & w)
			:world(w) {}
		double operator( )(COORD_TYPE x) const {
			return world.toProblemDomain(x,static_cast<spatial::Axis>(A) );
		}
		const World<COORD_TYPE,NUM_DIM> & world;
	};

	/**
	* Functor to convert point to problem domain 
	* @tparam NUM_DIM number of dimensions supported
	* @tparam COORD_TYPE type needed for modeling or API
	*/
	template <typename COORD_TYPE,int NUM_DIM>
	struct WorldToPDPointConverter{
		WorldToPDPointConverter(const World<COORD_TYPE,NUM_DIM> & w)
			:world(w) {}
		spatial::TPoint<double,NUM_DIM> operator( )(const spatial::TPoint<COORD_TYPE,NUM_DIM> &pt) const { 
			return world.toProblemDomain(pt);
		}
		const World<COORD_TYPE,NUM_DIM> & world;
	};
}
#endif
