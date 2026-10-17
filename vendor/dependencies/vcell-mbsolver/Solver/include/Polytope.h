//#pragma warning ( disable : 4267 )
#include "boost/polygon/polygon.hpp"
//#pragma warning ( default : 4267 )
#include "TPoint.h"
namespace spatial {

	template <typename TPOINT>
	struct Polytope {
		Polytope( ) 
			:points( ),
			dirty(false) {}

		Polytope(std::vector<T>  ) 
			:points( ),
			dirty(false) {}

		void add(const Point &p) {
			points.push_back(p);
			dirty = true;
		}

		/**
		* @return true if point added
		*/
		bool isClosed( ) const { 
			assert(points.size( ) > 0);
			TPOINT first = points[0];
			TPOINT last = points[points.size( ) - 1];
			return first == last;
		}

		/**
		* closed if not already
		**/
		void close( ) {
			if (!isClosed( )) {
				points.push_back(first);
				dirty = true;
			}
		}

		std::vector<TPOINT>:const_iterator begin( )  const{
			return points.begin( );
		}

		std::vector<TPOINT>:const_iterator end( )  const{
			return points.end( );
		}


	private:
		std::vector<TPOINT> points;
		bool dirty;
	};

	/*
	template <class T>
	bool inside<2>(const Polytope<T,2>
	*/

}
