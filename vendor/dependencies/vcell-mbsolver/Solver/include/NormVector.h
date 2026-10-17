#ifndef NormVector_h
#define NormVector_h
#include <SVector.h>

namespace spatial {
	/**
	* an immutable, normalized vector
	* @tparam T implementing type (floating point)
	* @tparam N number of dimesions 
	* @tparam MAG_TYPE type used for magnitued et. al. 
	*/
	template <class T, int N>
	class NormVector : private SVector<T,N> {
		typedef SVector<T,N> base;
	public:
		NormVector(const SVector<T,N> & in)
			:base(in) 
		{  this->normalize( ); }

		NormVector(const TPoint<T, N> & lhs, const TPoint<T, N> & rhs) 
			:base(lhs,rhs)
		{  this->normalize( ); }

		SVector<T,N> copy( ) const {
			return SVector<T,N>(*this);
		}

		NormVector<T,2> perpendicular( ) const {
			return base::perpendicular( );
		}

		NormVector & reverse( ) {
			return base::reverse( );
		}
		T operator( )(Axis a) const {
			return base::operator( )(a); 
		}
		double magnitude( ) const {
			return base::magnitude( );
		}
	};

	template <int N>
	class NormVector<long,N> {
	};

	template <class T, class U, int N>
	inline double dot(const spatial::SVector<T,N> & lhs, const spatial::NormVector<U,N> & rhs) {
		double rval = 0;
		for (int i = 0 ; i < N; i++) {
			const Axis a = static_cast<Axis>(i);
			rval += lhs(a) * rhs(a);
		}
		return rval;
	}

	template <class T, class U, int N>
	inline double dot(const spatial::NormVector<T,N> & lhs, const spatial::SVector<U,N> & rhs) {
		double rval = 0;
		for (int i = 0 ; i < N; i++) {
			const Axis a = static_cast<Axis>(i);
			rval += lhs(a) * rhs(a);
		}
		return rval;
	}

	template <class T, class U, int N>
	inline double dot(const spatial::NormVector<T,N> & lhs, const spatial::NormVector<U,N> & rhs) {
		double rval = 0;
		for (int i = 0 ; i < N; i++) {
			const Axis a = static_cast<Axis>(i);
			rval += lhs(a) * rhs(a);
		}
		return rval;
	}
}
#endif
