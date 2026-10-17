#ifndef SVector_h
#define SVector_h
#include <iostream>
#include <stdexcept>
#include <TPoint.h>
#include <Angle.h>
#include <persist.h>

namespace spatial {
	template <class T, int N>
	class NormVector; 

	/**
	* vector. If instantatiated with integral types, non-exact operations
	* will use truncated (floor) values.
	* @tparam T implementing type of coordinates
	* @tparam N number of dimensions 
	*/
	template <class T, int N>
	class SVector : public vcell_persist::Persistent {
		std::array<T,N> component;
	public:
		SVector( ) {
			std::fill(component.begin( ),component.end( ), 0);
		}
		SVector(T x, T y) {
			static_assert(N==2, "need 2");
			component[cX] = x;
			component[cY] = y;
		}
		SVector(Angle a, T length) {
			static_assert(N==2, "need 2");
			if (length <= 0) {
				throw std::invalid_argument("negative length");
			}
			component[cX] = length * a.cos( ); 
			component[cY] = length * a.sin( ); 
		}

		SVector(const TPoint<T, N> & lhs, const TPoint<T, N> & rhs) {
			for (int i = 0 ; i < N; i++) {
				const Axis a = static_cast<Axis>(i);
				component[i] = rhs(a) - lhs(a);
			}
		}

		SVector(std::istream &is) {
			vcell_persist::Token::check<SVector<T,N> >(is); 
			vcell_persist::restore(is,component);
		}

		bool operator==(const SVector &rhs) const {
			for (int i = 0 ; i < N; i++) {
				if (component[i] != rhs.component[i]) {
					return false;
				}
			}
			return true;
		}

		bool operator!=(const SVector &rhs) const {
			for (int i = 0 ; i < N; i++) {
				if (component[i] != rhs.component[i]) {
					return true;
				}
			}
			return false;
		}

		SVector(const NormVector<T, N> & in) {
			for (int i = 0 ; i < N; i++) {
				const Axis a = static_cast<Axis>(i);
				component[i] = in(a); 
			}
		}

		bool isZero( ) const {
			for (int i = 0 ; i < N; i++) {
				if (component[i] != 0) {
					return false;
				}
			}
			return true;
		}

		Angle angle( ) const {
			static_assert(N==2, "need 2");
			if (isZero( ) ) {
				throw std::domain_error("angle called on zero vector");
			}
			return RadAngle(atan2(component[cY] , component[cX]));
		}

		/**
		* return new vector perpendicular to *this
		*/
		SVector<T,2> perpendicular( ) const {
			static_assert(N==2, "need 2");
			return SVector(component[cY],-component[cX]);
		}

		/**
		* reverse existing vector
		*/
		SVector & reverse( ) {
			operator*=(-1);
			return *this;
		}
		SVector & operator*=(T scaler) {
			for (int i = 0 ; i < N; i++) {
				component[i] *= scaler;
			}
			return *this;
		}
		SVector & operator/=(T scaler) {
			for (int i = 0 ; i < N; i++) {
				component[i] /= scaler;
			}
			return *this;
		}
		SVector & operator+=(const SVector &rhs) {
			for (int i = 0 ; i < N; i++) {
				component[i] += rhs.component[i]; 
			}
			return *this;
		}
		T operator( )(Axis a) const {
			return component[a];
		}
		/**
		* allow assignment
		*/
		T & operator( )(Axis a) {
			return component[a];
		}
		double magnitude( ) const {
			const double mag2 = dot(*this,*this);
			return sqrt(mag2);
		}
		double magnitudeSquared( ) const {
			return dot(*this,*this);
		}
		void normalize( ) {
			if (isZero( ) ) {
				throw std::domain_error("normalize called on zero vector");
			}
			const double mag = magnitude( ); 
			for (int i = 0 ; i < N; i++) {
				const Axis a = static_cast<Axis>(i);
				component[i] = static_cast<T>(component[i] / mag);
			}
			assert(dot(*this,*this) < 1e15); 
		}
		T x( ) const { return component[cX]; }
		T y( ) const { 
			static_assert(N>=2,"2D"); 
			return component[cY]; 
		}
		T z( ) const { 
			static_assert(N>=3,"3D"); 
			return component[cY]; 
		}
		/**
		* convert to new vector of different type by static_casting each component
		* @tparam U type of vector to convert to
		*/
		template <typename U>
		SVector<U,N> convert( ) const {
			SVector<U,N> other;
			for (Axis a = axisInitial; a < N; ++a) {
				other(a) = static_cast<U>(component[a]);
			}
			return other;
		}
		void persist(std::ostream &os) const {
			vcell_persist::Token::insert<SVector<T,N> >(os); 
			vcell_persist::save(os,component);
		}

		static void registerType( ) {
			vcell_persist::Registrar::reg< SVector<T,N>, T,N>("SVector");
		}
	};
	
	/*
		template <class T, int N>
		SVector<T,N>  SVector<T,N>::convert<T>( ) const {
			return *this;
		}
		*/

	template <class T, int N>
	inline double dot(const spatial::SVector<T,N> & lhs, const spatial::SVector<T,N> & rhs) {
		double rval = 0;
		for (int i = 0 ; i < N; i++) {
			const Axis a = static_cast<Axis>(i);
			const double left = lhs(a);
			const double right = rhs(a);
			rval += left * right; 
		}
		return rval;
	}

	template <class T, int N, class U>
	inline SVector<T,N> operator*(const spatial::SVector<T,N> & lhs, U rhs) {
		spatial::SVector<T,N> copy(lhs);
		copy *= rhs;
		return copy;
	}

	template <class T, int N, class U>
	inline SVector<T,N> operator/(const spatial::SVector<T,N> & lhs, U rhs) {
		spatial::SVector<T,N> copy(lhs);
		copy /= rhs;
		return copy;
	}

	template <class T, int N>
	inline SVector<T,N> operator+(const spatial::SVector<T,N> & lhs, const spatial::SVector<T,N> & rhs) {
		spatial::SVector<T,N> copy(lhs);
		copy += rhs;
		return copy;
	}

	typedef SVector<double,2> SVector2D;

	template <class T>
	std::ostream & operator<<(std::ostream &os, const SVector<T,1> &v2) {
		os << '<' << v2(cX) << '>';
		return os;
	}

	template <class T>
	std::ostream & operator<<(std::ostream &os, const SVector<T,2> &v2) {
		os << '<' << v2(cX) << ',' << v2(cY) << '>';
		return os;
	}

}
#endif
