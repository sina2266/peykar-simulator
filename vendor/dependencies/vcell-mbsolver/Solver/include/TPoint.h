#ifndef Point_h
#define Point_h
#include <cassert>
#include <iostream>
#include <array>
#include <cmath>
#include <algorithm>
#include <persistcontainer.h>
namespace spatial {


	/**
	* use symbols for axes to avoid confusion with array locations
	*/
	enum Axis {cX=0,cY=1,cZ=2, 
		/**
		* conceptual alias for cX
		*/
		axisInitial=0};

	inline Axis & operator++(Axis &a) {
		a = static_cast<Axis>(a + 1);
		return a;
	}

#   pragma warning ( disable : 4351 )
	/**
	* basic point definition
	* @param T primitive type
	* @param N number of dimensions
	*/
	template<class T, int N>
	class TPoint : public vcell_persist::Persistent {
	protected:
		std::array<T,N> coord; 
		double normal[N];
	public:
		typedef T value_type; 
		TPoint(const std::array<T,N> &val)  
			:coord(val) {}
		TPoint(T val)  {
			static_assert(N==1,  "N arg required");
			coord[0] = val;
		}

		TPoint(T a, T b)  {
			static_assert(N==2,  "N args required");
			coord[0] = a;
			coord[1] = b;
		}

		TPoint(T a, T b, T c) {
			static_assert(N==3,   "N args required");
			coord[0] = a;
			coord[1] = b;
			coord[2] = c;
		}
		TPoint(std::istream &is)
			:coord( ) 
		{
			vcell_persist::Token::check<TPoint<T,N> >(is); 
			vcell_persist::restore(is,coord);
		}


		/**
		* initialize from C style array
		* @deprecated -- std::array constructor preferred
		*/
		TPoint(const T * a) {
			for (int i = 0; i < N; i++) {
				coord[i] = a[i];
			}
		}

		TPoint(const T * a, const double* n) {
			for (int i = 0; i < N; i++) {
				coord[i] = a[i];
				normal[i] = n[i];
			}
		}

		/**
		* zero
		*/
		TPoint( )
			:coord(){}

		/**
		* construct scaled copy. 
		* constructed point is cast to implementing T with
		* implicit loss of precision converting from
		* type U to type T
		*/
		template <class U>
		TPoint(const TPoint<U,N> & rhs, T scale) { 
			for (int i = 0; i < N; i++ ) {
				coord[i] = static_cast<T>(rhs(static_cast<Axis>(i)) * scale);
			}
		}

		/**
		* type converting constructor
		* @tparam U point type to convert from
		*/
		template <class U>
		explicit TPoint(const TPoint<U,N> & rhs) { 
			for (int i = 0; i < N; i++ ) {
				coord[i] = static_cast<T>(rhs(static_cast<Axis>(i)));
			}
		}
		/**
		* @param x scaled long
		* @param y scaled long
		* @param scale current scale 
		*/
		/*
		TPoint(T x, T y, double scale) 
		{
		coord[cX] = x * scale;
		coord[cY] = y * scale;
		}
		*/

		TPoint(T (&values)[N] ) {
			for (int i = 0; i < N; i++) {
				coord[i] = values[i];
			}
		}
		
		const T* getCoords() const
		{
			return coord.data();
		}

		const double* getNormal() const
		{
			return normal;
		}

		T operator( )(Axis a) const {
			return coord[a];
		}
		T &operator( )(Axis a) {
			return coord[a];
		}
		T get(Axis a) const {
			return coord[a];
		}
		/**
		* < operator to allow insertion in set
		* comparison based on coordinates in sequence
		*/
		bool operator <(const TPoint & rhs) const {
			for (int i = 0; i < N; i++) {
				if (coord[i] < rhs.coord[i]) {
					return true;
				}
				if (coord[i] > rhs.coord[i]) {
					return false;
				}
			}
			return false;
		}

		void persist(std::ostream &os) const {
			vcell_persist::Token::insert<TPoint<T,N> >(os); 
			vcell_persist::save(os,coord);
		}

		static int numDim( ) {
			return N;
		}

		static void registerType( ) {
			vcell_persist::Registrar::reg<TPoint<T,N>, T,N>("TPoint");
		}

	};

//#pragma warning ( disable : 4351 )
	template<class T, int N>
	class GhostPoint :public TPoint<T,N> {
		typedef TPoint<T,N> base;
	protected:
		bool ghost;
	public:
		explicit GhostPoint(T val)
			:base(val),
			ghost(false)  {}

		explicit GhostPoint(T a, T b)
			:base(a,b),
			ghost(false)  {}
		explicit GhostPoint(T a, T b, bool isGhost)
			:base(a,b),
			ghost(isGhost)  {}

		explicit GhostPoint(T a, T b, T c)
			:base(a,b,c),
			ghost(false)  {}

		GhostPoint(const std::array<T,N> &in)
			:base(in),
			ghost(false) {}

		/*
		GhostPoint(const Array &a) 
		:base(a),
		ghost(false) {}
		*/

		/**
		* zero, non-ghost point
		*/
		GhostPoint( )
			:base(),
			ghost(false) {}

		GhostPoint(const TPoint<T,N> & rhs)
			:base(rhs),
			ghost(false) {}

		/**
		* construct scaled copy
		*/
		GhostPoint(const TPoint<T,N> & rhs, double scale, bool ghost_)  
			:base(rhs,scale),
			ghost(ghost_) {}

		/**
		* @param x scaled long
		* @param y scaled long
		* @param scale current scale 
		* @param ghost_ if true, mark as ghost point 
		*/
		GhostPoint(T x, T y, double scale, bool ghost_) 
			:base(x *scale, y *scale),
			ghost(ghost_) {}

		void setGhost(bool ghost) {
			this->ghost = ghost;
		}

		bool isGhost( ) const {
			return ghost;
		}
	};

	template <class T, int N>
	inline bool operator==(const spatial::TPoint<T,N> & lhs, const spatial::TPoint<T,N> & rhs ) {
		for (int i = 0; i < N ;i++) {
			const Axis a = static_cast<Axis>(i);
			if (lhs(a) != rhs(a) ) {
				return false;
			}
		}
		return true;
	}

	template <class T, int N>
	inline bool operator!=(const spatial::TPoint<T,N> & lhs, const spatial::TPoint<T,N> & rhs ) {
		return ! (lhs == rhs);
	}


	template <class T, int N>
	inline std::ostream &operator<<(std::ostream & os, const spatial::TPoint<T,N> & point) {
		os << "(";
		os << point(static_cast<Axis>(0)); 
		for (int i = 1; i < N ;i++) {
			const Axis a = static_cast<Axis>(i);
			os << "," << point(a);
		}
		os << ")";
		return os;
	}

	typedef TPoint<double,2> Point2D;

	template<class FROM, typename TO>
	inline TO convert(const FROM &from) {
		assert(TO::numDim( ) == FROM::numDim( ));
		TO rval;
		for (int i = 0; i < TO::numDim( ) ;i++) {
			const Axis a = static_cast<Axis>(i);
			rval(a) = from(a);
		}
		return rval;
	}
	/**
	* Euclidean distance between points squared
	* @tparam M calculation / return type of magnitude. Must be selected to prevent overflow of integral types
	* @tparam T point coordinate type 
	* @tparam N number of dimensions 
	* @return Euclidean distance squared
	*/
	template <typename M, typename T, int N>
	M magnitudeSquared(const TPoint<T,N> &p1,const TPoint<T,N> &p2) { 
		M m = 0;
		for (Axis a = axisInitial; a < N; ++a) {
			M e1 = static_cast<M>(p1(a));
			M e2 = static_cast<M>(p2(a));
			M diff = e1 - e2; 
			m += diff*diff;
		}
		return m;
	}

	/**
	* Euclidean distance between points 
	* @tparam M calculation / return type of magnitude. Must be selected to prevent overflow of integral types
	* @tparam T point coordinate type 
	* @tparam N number of dimensions 
	* @return Euclidean distance 
	*/
	template <typename M, typename T, int N>
	M magnitude(const TPoint<T,N> &p1,const TPoint<T,N> &p2) { 
		return static_cast<M>(std::sqrt(magnitudeSquared<M>(p1,p2)) );
	}
	/**
	* taxicab / L1Norm / Manhattan distance between points  <a href="http://en.wikipedia.org/wiki/Taxicab_geometry" Taxicab Geometry>
	* @tparam M calculation / return type of magnitude. Must be selected to prevent overflow of integral types
	* @tparam T point coordinate type 
	* @tparam N number of dimensions 
	* @return taxicab distance 
	*/
	template <typename M, typename T, int N>
	M taxicabDistance(const TPoint<T,N> &p1,const TPoint<T,N> &p2) { 
		M m = 0;
		for (Axis a = axisInitial; a < N; ++a) {
			//std::abs not overloaded for all types, so  ...
			if (p1(a) > p2(a) ) {
				m += static_cast<M>(p1(a) - p2(a)); 
			}
			else {
				m += static_cast<M>(p2(a) - p1(a)); 
			}
		}
		return m;
	}
}
#endif
