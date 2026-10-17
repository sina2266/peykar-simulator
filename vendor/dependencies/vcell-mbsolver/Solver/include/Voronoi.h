#ifndef Voronoi_h
#define Voronoi_h
#include <TPoint.h>
#include <VoronoiResult.h>
#include <GeoLimit.h>
#include <SVector.h>
#pragma warning ( disable : 4267 4244 )
#include <boost/polygon/voronoi.hpp>
#pragma warning ( default : 4267 4244)
namespace spatial {

	template <class T, int N>
	class Voronoi {
	};


	template <typename I> class Voronoi2D;

	template <typename I>
	struct Voronoi2DImpl {
		virtual ~Voronoi2DImpl( ) {} 
		virtual size_t num_cells( ) const=0;
		virtual void clear( )=0;
		virtual void getVertices(TVoronoiResult<I> & result, size_t cellIndex) const=0;
		/*
	protected:
		Voronoi2DImpl(Voronoi2D<I> & owner_)
			:owner(owner_) {}
		Voronoi2D<I> &owner;
		bool & dirty( ) {
			return owner.dirty;
		}
		*/
	};
	template <typename I>
	struct ImplFactory {
		static struct Voronoi2DImpl<I> *createImplementation(Voronoi2D<I> &, bool &, std::vector<spatial::TPoint<I,2> > & );
	};
	/**
	* @tparam I input point integer type
	*/
	template <typename I>
	class Voronoi2D {
	public:
		typedef TPoint<I, 2> VoronoiPoint;
		typedef GhostPoint<I, 2> GhostVPoint;
		Voronoi2D(const std::array<TGeoLimit<I> ,2> & limits)
			:points(),
			dirty(true),
			limits_(limits),
			impl(ImplFactory<I>::createImplementation(*this,dirty, points))
		{ }

		~Voronoi2D( ) {
			delete impl;
		}

		void add(const VoronoiPoint &p) {
			points.push_back(p);
			dirty = true;
		}
		void add(I x, I y) {
			points.push_back(VoronoiPoint(x,y) );
			dirty = true;
		}

		/**
		* remove all points
		*/
		void clear( ) {
			points.clear( );
			impl->clear( );
			dirty = true;
		}

		const VoronoiPoint & cell(size_t index) const {
			return points[index];
		}

		size_t numberPoints( ) const {
			return points.size( );
		}
		//return copy of current points
		std::vector<VoronoiPoint> currentPoints( ) const {
			return points;
		}

		size_t numberCells( ) const {
			return impl->num_cells( );
		}

		void getVertices(TVoronoiResult<I> & result, size_t cellIndex) const {
			impl->getVertices(result,cellIndex);
		}

		const std::array<TGeoLimit<I> ,2>  & limits( ) const {
			return limits_;
		}

protected:

	std::vector<VoronoiPoint> points;
	mutable bool dirty;
	const std::array<TGeoLimit<I> ,2> limits_;
	Voronoi2DImpl<I> *impl;
	friend Voronoi2DImpl<I>;

private:
	//private, not implemented
	Voronoi2D(const Voronoi2D& other);
	Voronoi2D & operator=(const Voronoi2D& other);
	};
}
#endif
