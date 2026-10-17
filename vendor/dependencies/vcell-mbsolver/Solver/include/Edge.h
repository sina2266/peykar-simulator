#ifndef Edge_h
#define Edge_h
#include <vector>
#include <TPoint.h>
#include <SVector.h>
#include <algo.h> //displacement
namespace spatial {

	/**
	*  a point in space and a vector originating from it
	*/
	template <class REAL, int N>
	class Edge {
	public:
		typedef TPoint<REAL,N> Point;
		typedef SVector<REAL,N>  EdgeVector; 
		typedef Edge<REAL,N> OurType; 

		Edge(const Point & p, const EdgeVector & eVector)
			:orig(p),
			vec(eVector) {}
		Edge(const Point & p1, const Point & p2)
			:orig(p1),
			vec(EdgeVector(p1,p2)) {}

		Edge( )
			:orig( ),
			vec( ) {}
		//default copy constructor, assignment operator

		bool operator==(const Edge &rhs) const {
			return orig == orig && vec == vec;
		}

		bool operator!=(const Edge &rhs) const {
			return !(*this == rhs);
		}

		bool equalOpposite(const Edge &rhs) const {
			for (Axis a = axisInitial; a < N; ++a) {
				REAL otherOrig = orig(a) + vec(a); 
				if (rhs.origin( )(a) != otherOrig) {
					return false;
				}
			}
			return true;
		}

		const Point & origin( ) const {
			return orig;
		}
		/**
		* tail of vector -- constructed on fly
		*/
		Point tail( ) const {
			return displacement(orig,vec);
		}
		const EdgeVector & edgeVector( ) const {
			return vec;
		}
	protected:
		Point orig;
		EdgeVector vec;
	};

	template <class REAL, int N>
	class EdgeStateful :public Edge<REAL,N> {
	public:

		EdgeStateful(const typename Edge<REAL,N>::Point & p, const typename Edge<REAL,N>::EdgeVector & eVector)
			:Edge<REAL,N>(p,eVector), 
			set(true) {}
		EdgeStateful(const typename Edge<REAL,N>::Point & p1, const typename Edge<REAL,N>::Point & p2)
			:Edge<REAL,N>(p1,p2),
			set(true) {}
		EdgeStateful( ) 
			:Edge<REAL,N>(typename Edge<REAL,N>::Point( ),defVector( )),
			set(false) {}
		EdgeStateful(const Edge<REAL,N> &rhs) 
			:Edge<REAL,N>(rhs),
			set(true) {}
		EdgeStateful & operator=(const Edge<REAL,N> & rhs) {
			Edge<REAL,N>::operator=(rhs);
			set = true;
			return *this;
		}
		bool operator==(const Edge<REAL,N> &rhs) const {
			return this->orig == rhs.orig && this->vec == rhs.vec;
		}

		bool operator!=(const Edge<REAL,N> &rhs) const {
			return !(*this == rhs);
		}

		bool isSet( ) const {
			return set;
		}
	private:
		/**
		* return default (non-zero) vector to support default
		* constructor 
		*/
		typename Edge<REAL,N>::EdgeVector defVector( ) {
			typename Edge<REAL,N>::Point p;
			typename Edge<REAL,N>::Point r(p);
			r(cX)++;
			return Edge<REAL,N>::EdgeVector(p,r);
		}
		bool set;
	};

	template <typename T, int N>
	std::ostream & operator<<(std::ostream &os, const Edge<T,N> & e) {
		os << "E{ " << e.origin( ) << " " << e.edgeVector( ) << " }";
		return os;
	}
}
#endif
