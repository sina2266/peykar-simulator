#include <limits>
#include <cstdint>
#include <Volume.h>
#include <Edge.h>
#include <Segment.h>
#include <intersection.h>
#include <VCellException.h>
#include <persist.h>
#include <persistcontainer.h>
#define COMPILE_64
using vcell_persist::binaryWriter;
using vcell_persist::binaryReader;
namespace {
	/**
	* @tparam V vector
	*/
	template <typename V>
	struct vectorWriter {
		std::ostream &os;

		explicit vectorWriter(std::ostream &os_)
			:os(os_) {}
		void operator( )(V v) {
			vcell_persist::save(os,v);
		}
	};
	/**
	* @tparam V vector
	*/
	template<typename V>
	void persists(std::ostream &os,  const std::vector< V >& vec) {
		vcell_persist::binaryWrite(os,vec.size( ));
		std::for_each(vec.begin( ), vec.end( ),vectorWriter<V>(os) );
	}

	/**
	* @tparam V vector
	*/
	template<typename V>
	void readVectors(std::istream &is,  std::vector< V >& vec) {
		typedef typename std::vector<V>::size_type Stype;
		Stype size; 
		vcell_persist::binaryRead(is,size);

		vec.clear( );
		vec.reserve(size);
		for (Stype i = 0; i < size; ++i) {
			V v;
			vcell_persist::restore(is,v);
			vec.push_back( v );
		}

	}
}
namespace spatial {
	//forward
	template <class COORD_TYPE,class VALUE_TYPE>
	struct Polygon; 

	template <class COORD_TYPE,class VALUE_TYPE>
	struct Polygons; 

	/*
	template <class COORD_TYPE,class VALUE_TYPE,int N>
	void VolumeImpl<COORD_TYPE,VALUE_TYPE,N>::registerType( ) {
		vcell_persist::tRegisterTypeToken<COORD_TYPE,VALUE_TYPE,N>(typeid(Volume<COORD_TYPE,VALUE_TYPE,N>),"Volume");
		TPoint<COORD_TYPE,N>::registerType( );
		TPoint<VALUE_TYPE,N>::registerType( );
	}
	*/

	/**
	* support double dispatch virtual functions
	*/
	template <class COORD_TYPE,class VALUE_TYPE>
	struct VolumeImplDoubleDispatch : public VolumeImpl<COORD_TYPE,VALUE_TYPE,2> {
		virtual Volume<COORD_TYPE,VALUE_TYPE,2> intersectionSingle(const Polygon<COORD_TYPE,VALUE_TYPE> &rhs) const = 0; 
		virtual Volume<COORD_TYPE,VALUE_TYPE,2> intersectionMany(const Polygons<COORD_TYPE,VALUE_TYPE> &rhs) const = 0; 
	};


	template <class COORD_TYPE,class VALUE_TYPE>
	struct EmptyVolume2 : public VolumeImplDoubleDispatch<COORD_TYPE,VALUE_TYPE> {
		typedef typename VolumeImpl<COORD_TYPE,VALUE_TYPE,2>::PointVector PointVector; 
		typedef typename VolumeImpl<COORD_TYPE,VALUE_TYPE,2>::VectorOfVectors VectorOfVectors;

		static EmptyVolume2<COORD_TYPE,VALUE_TYPE> *getSingleton( ) {
			return &instance;
		}
		VALUE_TYPE volume() const {
			return 0;
		}
		bool empty( ) const {
			return true;
		}
		void clear( ) {}
		VectorOfVectors points() const {
			return VectorOfVectors( );
		}
		bool more(size_t index) const {
			return false;
		}
		Edge<COORD_TYPE,2> getEdge(size_t index) const {
			throw std::invalid_argument("Invalid index");
		}
		Segment<COORD_TYPE,2> getSegment(size_t index) const {
			throw std::invalid_argument("Invalid index");
		}
		virtual void close( ) {} 

#ifdef VOLUME_IMPLEMENT_INSIDE
		virtual bool inside(const TPoint<COORD_TYPE,2> &point) const {
			return false;
		}
#endif
		/**
		* EmptyVolume doesn't exist if point has been added, so
		* logically we're still on the first section
		*/
		VolumeImpl<COORD_TYPE,VALUE_TYPE,2> *nextSection( ) {
			return this;	
		}

		virtual VolumeImpl <COORD_TYPE,VALUE_TYPE,2> *add(const TPoint<COORD_TYPE,2> & point) {
			Polygon<COORD_TYPE,VALUE_TYPE> *poly = new Polygon<COORD_TYPE,VALUE_TYPE>( );
			poly->add(point);
			return poly;
		}

		virtual VolumeImpl<COORD_TYPE,VALUE_TYPE,2>* prep(typename VolumeImpl<COORD_TYPE,VALUE_TYPE,2>::Operation op) {
			void * ptr = VolumeImpl<COORD_TYPE,VALUE_TYPE,2>::prep(op);
			assert ((op == VolumeImpl< COORD_TYPE,VALUE_TYPE,2>::opFillingIterator));
			return new Polygon<COORD_TYPE,VALUE_TYPE>( ); 
		}
		/**
		* should never be called, as #prep should replace this with 
		* implementation that supports call
		*/
		virtual typename std::vector<TPoint<COORD_TYPE,2> >::iterator fillingIterator(size_t n) { 
			throw std::domain_error("fillingIterator call on EmptyVolume");
		}
		virtual Volume<COORD_TYPE,VALUE_TYPE,2> intersection(const std::vector<TPoint<COORD_TYPE,2> > &rhs) const {
			return Volume<COORD_TYPE,VALUE_TYPE,2>(0); 
		}
		virtual Volume<COORD_TYPE,VALUE_TYPE,2> intersection(const Volume<COORD_TYPE,VALUE_TYPE,2> &rhs) const {
			return Volume<COORD_TYPE,VALUE_TYPE,2>(0); 
		}
		virtual Volume<COORD_TYPE,VALUE_TYPE,2> intersectionSingle(const Polygon<COORD_TYPE,VALUE_TYPE> &rhs) const { 
			return Volume<COORD_TYPE,VALUE_TYPE,2>(0); 
		}
		virtual Volume<COORD_TYPE,VALUE_TYPE,2> intersectionMany(const Polygons<COORD_TYPE,VALUE_TYPE> &rhs) const {
			return Volume<COORD_TYPE,VALUE_TYPE,2>(0); 
		}
		/**
		* don't delete this object
		*/
		virtual bool singleton( ) {
			return true;
		}
		void operator delete(void *ptr) {
			throw std::logic_error("EmptyVolume2 delete");
		}
		virtual void persist(std::ostream &os) const {
			vcell_persist::Token::insert<Volume<COORD_TYPE,VALUE_TYPE,2> >(os);
			vcell_persist::binaryWrite<short>(os,0);
		} 
	private:
		void *operator new[](size_t s); 
		/**
		* prevent allocating new object, as all
		* empty volumes are equivalent
		*/
		void *operator new(size_t s); 
		static EmptyVolume2<COORD_TYPE,VALUE_TYPE> instance;
	};

	template <class COORD_TYPE,class VALUE_TYPE>
	EmptyVolume2<COORD_TYPE,VALUE_TYPE> EmptyVolume2<COORD_TYPE,VALUE_TYPE>::instance;

	template <class COORD_TYPE,class VALUE_TYPE>
	struct Polygon : public VolumeImplDoubleDispatch <COORD_TYPE,VALUE_TYPE> {
		typedef typename VolumeImpl<COORD_TYPE,VALUE_TYPE,2>::PointVector PointVector; 
		typedef typename VolumeImpl<COORD_TYPE,VALUE_TYPE,2>::VectorOfVectors VectorOfVectors;
		typedef TPoint<COORD_TYPE,2> Point;
		Polygon( )
			:pointStorage( ),
			dirty(true),
			vol(0) {}

		Polygon(const std::vector<Point> &points_)
			:pointStorage(points_),
			dirty(true),
			vol(0) {}

		Polygon(std::istream &is) {
			//factory checks token and reads initial short, so constructor just reads remaining fields
			vcell_persist::restore(is,pointStorage);
			vcell_persist::binaryRead(is,dirty);
			vcell_persist::binaryRead(is,vol);
		}

		/**
		* construct rectangular Polygon
		*/
		Polygon(const std::array<COORD_TYPE,2> &origin, const std::array<COORD_TYPE,2> & lengths)
			:pointStorage(),
			dirty(false),
			vol(static_cast<VALUE_TYPE>(lengths[cX] * lengths[cY])) 
		{
			std::array<COORD_TYPE,2> working(origin);
			pointStorage.push_back(working); //top left
			working[cX] += lengths[cX];
			pointStorage.push_back(working); //top right 
			working[cY] += lengths[cY];
			pointStorage.push_back(working); //bottom right 
			working[cX] -= lengths[cX];
			pointStorage.push_back(working); //bottom left 
			pointStorage.push_back(origin); //top left again (close)
		}

		/**
		* add point to current polygon; 
		*/
		Polygon<COORD_TYPE,VALUE_TYPE> *add(const Point &p) {
			pointStorage.push_back(p);
			dirty = true;
			return this;
		}

		void add(COORD_TYPE x, COORD_TYPE y) {
			add(Point(x,y));
		}

		bool closed( ) const {
			if (!pointStorage.empty( )) {
				return pointStorage.front( ) == pointStorage.back( );
			}
			return false;
		}

#ifdef VOLUME_IMPLEMENT_INSIDE
		virtual bool inside(const TPoint<COORD_TYPE,2> &point) const {
			return spatial::inside(pointStorage,point);
		}
#endif

		/**
		* close polygon
		* @throws std::invalid_argument if polygon empty
		*/
		void close( ) {
			if (!closed( )) {
				if (pointStorage.empty( )) {
					throw std::invalid_argument("close on empty polygon");
				}
				pointStorage.push_back(pointStorage.front( ));
			}
			assert(closed( ));
		}

		/**
		* set internal vector to have n points and return iterator 
		*/
		virtual typename std::vector<TPoint<COORD_TYPE,2> >::iterator fillingIterator(size_t n) { 
			dirty = true;
			pointStorage.reserve(n + 1); //allow extra for closing polygon
			pointStorage.resize(n);
			return pointStorage.begin( );
		}

		/**
		* remove existing vertices
		*/
		void clear( ) {
			dirty = true;
			pointStorage.clear( );
		}

		VolumeImpl<COORD_TYPE,VALUE_TYPE,2> *nextSection( ) {
			if (pointStorage.empty( )) {
				return this;
			}
			assert(pointStorage.size( ) >= 3); //should be at least three points in polygon
			Polygons<COORD_TYPE,VALUE_TYPE> *polys = new Polygons<COORD_TYPE,VALUE_TYPE>(pointStorage);
			delete this;
			return polys->nextSection( );
		}

		VectorOfVectors points() const {
			VectorOfVectors rval;
			rval.push_back(pointStorage);
			return rval;
		};

		/**
		* calculate or return polygon volume
		* evaluated lazily 
		* @throws std::invalid_argument if polygon not closed 
		*/
		VALUE_TYPE volume( ) const {
			if (!closed( )) {
				throw std::invalid_argument("volume call on open polygon");
			}
			if (dirty) { 
				Polygon & us = const_cast<Polygon &>(*this); //"logically" const
				us.calculateVolume( );
			}
			return vol;
		}

		const std::vector<Point> & vertices( ) const {
			return pointStorage;
		}

		bool empty( )  const {
			return pointStorage.empty( );
		}
		virtual Volume<COORD_TYPE,VALUE_TYPE,2> intersection(const Volume<COORD_TYPE,VALUE_TYPE,2> &rhs) const {
			const VolumeImplDoubleDispatch<COORD_TYPE,VALUE_TYPE> & rhsDD = static_cast<const VolumeImplDoubleDispatch<COORD_TYPE,VALUE_TYPE> &>(this->pal(rhs));
			return rhsDD.intersectionSingle(*this); 
		}
		virtual Volume<COORD_TYPE,VALUE_TYPE,2> intersection(const std::vector<TPoint<COORD_TYPE,2> > &rhs) const {
			Volume<COORD_TYPE,VALUE_TYPE,2> rval;
			spatial::intersections(rval,pointStorage,rhs);
			return rval;
		}

		virtual Volume<COORD_TYPE,VALUE_TYPE,2> intersectionSingle(const Polygon<COORD_TYPE,VALUE_TYPE> &rhs) const { 
			Volume<COORD_TYPE,VALUE_TYPE,2> rval;
			spatial::intersections(rval,pointStorage,rhs.pointStorage);
			return rval;
		}
		virtual Volume<COORD_TYPE,VALUE_TYPE,2> intersectionMany(const Polygons<COORD_TYPE,VALUE_TYPE> &rhs) const {
			Volume<COORD_TYPE,VALUE_TYPE,2> rval;
			spatial::intersectionsManySingle(rval,rhs.storage( ), pointStorage);
			return rval;
		}
		virtual void persist(std::ostream &os) const {
			vcell_persist::Token::insert<Volume<COORD_TYPE,VALUE_TYPE,2> >(os);
			vcell_persist::binaryWrite<short>(os,1);
			vcell_persist::save(os,pointStorage);
			vcell_persist::binaryWrite(os,dirty);
			vcell_persist::binaryWrite(os,vol);
		} 
	protected:
		bool more(size_t index) const {
			return index < pointStorage.size( ) - 1;
		}
		Edge<COORD_TYPE,2> getEdge(size_t index) const {
			return Edge<COORD_TYPE,2>(pointStorage[index],pointStorage[index+1]);
		}
		Segment<COORD_TYPE,2> getSegment(size_t index) const {
			return Segment<COORD_TYPE,2>(pointStorage[index],pointStorage[index+1]);
		}

	private:
		void calculateVolume( ) {
			assert(dirty);
			assert(closed());
			using spatial::cX;
			using spatial::cY;
			vol = 0;	
			typename std::vector<Point>::const_iterator iter = pointStorage.begin( );
			typename std::vector<Point>::const_iterator next(iter); 
			++next; //initialize to following point
			for (;next!=pointStorage.end( );++iter,++next) {
				//force type conversions first, then multiply
				const VALUE_TYPE lwX = static_cast<VALUE_TYPE>(iter->get(cX));
				const VALUE_TYPE lwY = static_cast<VALUE_TYPE>(iter->get(cY));
				const VALUE_TYPE hX = static_cast<VALUE_TYPE>(next->get(cX));
				const VALUE_TYPE hY = static_cast<VALUE_TYPE>(next->get(cY));
				vol += lwX * hY - lwY * hX;
			}
			vol = std::abs(vol/2);
			dirty = false;
		}
		std::vector<Point> pointStorage;
		bool dirty;
		VALUE_TYPE vol;
	};

	template <class COORD_TYPE,class VALUE_TYPE>
	struct Polygons : public VolumeImplDoubleDispatch<COORD_TYPE,VALUE_TYPE> {
		typedef typename VolumeImpl<COORD_TYPE,VALUE_TYPE,2>::PointVector PointVector; 
		typedef typename VolumeImpl<COORD_TYPE,VALUE_TYPE,2>::VectorOfVectors VectorOfVectors;
		typedef TPoint<COORD_TYPE,2> Point;

		Polygons( )
			:polys(),
			current(0),
			dirty(true),
			vol(0),
			referenceIndex(std::numeric_limits<size_t>::max( )),
			getVector(nullptr)
		{}

		Polygons(const std::vector<Point> &input)
			:polys(),
			dirty(true),
			vol(0),
			referenceIndex(std::numeric_limits<size_t>::max( )),
			getVector(nullptr)
		{
				polys.push_back(input);
				current = &polys.front( );
		}

		Polygons(std::istream &is) 
			:polys(),
			dirty(true),
			vol(0),
			referenceIndex(std::numeric_limits<size_t>::max( )), //see #persist( )
			getVector(nullptr) 
		{
			//factory checks token and reads initial short, so constructor just reads remaining fields
			readVectors(is,polys);
			unsigned short cIndex;
			vcell_persist::binaryRead(is,cIndex);
			vcell_persist::binaryRead(is,dirty);
			vcell_persist::binaryRead(is,vol);
			if (cIndex >= polys.size( ) ) {
				VCELL_EXCEPTION(out_of_range,"Retrieved index " << cIndex << " >= " << polys.size( ));
			}
			current = &polys[cIndex];
		}

		bool empty( )  const {
			return polys.empty( );
		}

		/**
		* remove existing vertices
		*/
		void clear( ) {
			polys.clear( );
		}
		/**
		* close polygon
		* @throws std::invalid_argument if polygon empty
		*/
		void close( ) {
			if (!closed( )) {
				if (!current || current->size( ) < 3) {
					throw std::invalid_argument("close with less than 3 points"); 
				}
				current->push_back(current->front( ));
			}
			assert(closed( ));
		}

		bool closed( ) const {
			if (!current) {
				return false;
			}
			return current->front( ) == current->back( );
		}

#ifdef VOLUME_IMPLEMENT_INSIDE
		virtual bool inside(const TPoint<COORD_TYPE,2> &point) const {
			for (typename VectorOfVectors::const_iterator iter = polys.begin( ); iter != polys.end( ); ++iter) {
				const PointVector &pv = *iter;
				bool in = spatial::inside(pv,point);
				if (in) {
					return true;
				}
			}
			return false;
		}
#endif
		/**
		* move to next polygon (if current has points)
		*/
		Polygons<COORD_TYPE,VALUE_TYPE> *nextSection( ) {
			assert(empty( ) || current->size( ) >= 3); //should be at least three points in polygon
			if (empty( )) {
				return this;
			}
			close( ); //close current if necessary
			polys.push_back(std::vector<Point>( ));
			current = &polys.back( );
			return this;
		}
		/**
		* add point to current polygon; 
		*/
		Polygons<COORD_TYPE,VALUE_TYPE> * add(const TPoint<COORD_TYPE,2> & point) {
			if (current) {
				current->push_back(point);
				return this;
			}
			polys.push_back(std::vector<Point>( ));
			current = &polys.back( );
			current->push_back(point);
			return this;
		}

		virtual typename std::vector<TPoint<COORD_TYPE,2> >::iterator fillingIterator(size_t n) { 
			current->reserve(n + 1); //allow extra for closing polygon
			current->resize(n);
			return current->begin( );
		}

		VectorOfVectors points() const {
			return polys;
		}

		/**
		* calculate or return polygon volume
		* evaluated lazily 
		* @throws std::invalid_argument if polygon not closed 
		*/
		VALUE_TYPE volume( ) const {
			if (dirty) { 
				Polygons & us = const_cast<Polygons &>(*this); //"logically" const
				us.calculateVolume( );
			}
			return vol;
		}
		virtual Volume<COORD_TYPE,VALUE_TYPE,2> intersection(const Volume<COORD_TYPE,VALUE_TYPE,2> &rhs) const {
			const VolumeImplDoubleDispatch<COORD_TYPE,VALUE_TYPE> & rhsDD = static_cast<const VolumeImplDoubleDispatch<COORD_TYPE,VALUE_TYPE> &>(this->pal(rhs));
			return rhsDD.intersectionMany(*this); 
		}
		virtual Volume<COORD_TYPE,VALUE_TYPE,2> intersection(const std::vector<TPoint<COORD_TYPE,2> > &rhs) const {
			Volume<COORD_TYPE,VALUE_TYPE,2> rval;
			spatial::intersectionsManySingle(rval,polys,rhs);
			return rval;
		}
		virtual Volume<COORD_TYPE,VALUE_TYPE,2> intersectionSingle(const Polygon<COORD_TYPE,VALUE_TYPE> &rhs) const { 
			Volume<COORD_TYPE,VALUE_TYPE,2> rval;
			spatial::intersectionsManySingle(rval,polys,rhs.vertices( ));
			return rval;
		}
		virtual Volume<COORD_TYPE,VALUE_TYPE,2> intersectionMany(const Polygons<COORD_TYPE,VALUE_TYPE> &rhs) const {
			Volume<COORD_TYPE,VALUE_TYPE,2> rval;
			spatial::intersectionsManyMany(rval,polys,rhs.polys);
			return rval;
		}

		const VectorOfVectors &storage( ) const {
			return polys;
		}
	protected:
		bool more(size_t index) const {
			/*
			* if index is greater than or equal to last queried value, first look in
			currently iterator vector ("getVector"). 
			*/
			if (index >= referenceIndex)  {
				const size_t adjusted = index - referenceIndex;
				if (adjusted < getVector->size( ) - 1) {
					return true;
				}
			}
			/*
			* otherwise, start over from beginning, counting edges as we go
			*/

			size_t base = 0; 
			for (size_t pIndex = 0; pIndex < polys.size( ); ++pIndex) {
				const size_t adjusted = index - base;
				if (adjusted < polys[pIndex].size( ) - 1) {
					//set reference index and getVector for next query
					referenceIndex = base;
					getVector = &polys[pIndex];
					return true;
				}
				base += polys[pIndex].size( ) - 1;
			}
			return false;
		}

		/**
		* @tparam BCT type with ctor than takes two adjacent points "binary container type"
		*/
		template <class BCT>
		BCT getBCT(size_t index) const {
			/*
			* if index is greater than or equal to last queried value, first look in
			* currently iterator vector ("getVector"). 
			* this is called through a protected method, and it is expected that "more( )" will
			* be called prior, such that referenceIndex and getVector are set to something
			* reasonable
			*/
			if (index >= referenceIndex)  {
				const size_t adjusted = index - referenceIndex;
				if (adjusted < getVector->size( ) - 1) {
					const PointVector & pv = *getVector;
					return BCT(pv[adjusted],pv[adjusted+1]);
				}
			}
			/*
			* otherwise, start over from beginning, counting edges as we go
			*/
			size_t adjusted = index;
			for (size_t pIndex = 0; pIndex < polys.size( ); ++pIndex) {
				if (adjusted < polys[pIndex].size( ) - 1) {
					return BCT(polys[pIndex][adjusted],polys[pIndex][adjusted+1]);
				}
			}
			VCELL_EXCEPTION(invalid_argument, "invalid call to get, index = " << index);
		}
		/*
		Edge<COORD_TYPE,2> getEdge(size_t index) const {
			return getBCT<Edge<COORD_TYPE, 2> >(index);
		}
		*/
		Segment<COORD_TYPE,2> getSegment(size_t index) const {
			return getBCT<Segment<COORD_TYPE, 2> >(index);
		}
		virtual void persist(std::ostream &os) const {
			//convert current pointer into unsigned short
			if (polys.size( ) >> std::numeric_limits<unsigned short>::max( )) {
				VCELL_EXCEPTION(logic_error, "number of polygons " << polys.size( ) 
					<< " exceeds storage design limit " << std::numeric_limits<unsigned short>::max( ));
			}
			unsigned short cIndex = 0;
			bool found = false;
			while (cIndex < polys.size( ) ) {
				found = (&polys[cIndex] == current); 
				if (found)  {
					break;
				}
				else {
					++cIndex;
				}
			}

			if (!found)  {
				VCELL_EXCEPTION(domain_error,"Can't find index of current pointer");
			}

			vcell_persist::Token::insert<Volume<COORD_TYPE,VALUE_TYPE,2> >(os);
			vcell_persist::binaryWrite<short>(os,2);

			persists(os,polys);
			vcell_persist::binaryWrite(os,cIndex);
			vcell_persist::binaryWrite(os,dirty);
			vcell_persist::binaryWrite(os,vol);
			//we do not write #referenceIndex and #getVector, as these are 
			//performance optimization used only during iteration and
			//do not represent actual object state
		} 

	private:
		void calculateVolume( ) {
			assert(dirty);
			vol = 0;
			typename std::vector<std::vector<Point> >::const_iterator iter = polys.begin( );
			for (;iter != polys.end( );++iter) {
				spatial::Polygon<COORD_TYPE,VALUE_TYPE> p(*iter);
				vol += p.volume( );
			}

			dirty = false;
		}
		VectorOfVectors polys;
		/**
		*support for nextSection, add
		*/
		PointVector * current; 
		bool dirty;
		VALUE_TYPE vol;
		/**
		* more and get support
		* base index of #getVector
		*/
		mutable size_t referenceIndex; 
		/**
		* cached pointer
		*/
		mutable const PointVector *getVector;
	};
	template <class COORD_TYPE,class VALUE_TYPE,int N>
	VolumeImpl<COORD_TYPE,VALUE_TYPE,N> * VolumeImplCreator<COORD_TYPE,VALUE_TYPE,N>::create(size_t nPolygons) {
		if (N == 2) {
			switch (nPolygons) {
			case 0:
				return EmptyVolume2<COORD_TYPE,VALUE_TYPE>::getSingleton( );
				break;
			case 1:
				return new Polygon<COORD_TYPE,VALUE_TYPE>( );
				break;
			default:
				return new Polygons<COORD_TYPE,VALUE_TYPE>( );
				break;
			}
		}
		return 0;
	}
	template <class COORD_TYPE,class VALUE_TYPE,int N>
	VolumeImpl<COORD_TYPE,VALUE_TYPE,N> * VolumeImplCreator<COORD_TYPE,VALUE_TYPE,N>::read(std::istream &is) {
		vcell_persist::Token::check<Volume<COORD_TYPE,VALUE_TYPE,2> >(is);
		short nPolygons;
		vcell_persist::binaryRead(is,nPolygons);
		switch (nPolygons) {
			case 0:
				return EmptyVolume2<COORD_TYPE,VALUE_TYPE>::getSingleton( );
				break;
			case 1:
				return new Polygon<COORD_TYPE,VALUE_TYPE>(is);
				break;
			case 2:
				return new Polygons<COORD_TYPE,VALUE_TYPE>(is);
				break;
			default:
				VCELL_EXCEPTION(out_of_range,"Volume readback number polys " << nPolygons << "out of range 0 - 2")
		}
	}
	template <class COORD_TYPE,class VALUE_TYPE,int N>
	VolumeImpl<COORD_TYPE,VALUE_TYPE,N> * VolumeImplCreator<COORD_TYPE,VALUE_TYPE,N>::rectangle(const std::array<COORD_TYPE,N> &origin, const std::array<COORD_TYPE,N> & lengths) {
		if (N == 2) {
			return  new Polygon<COORD_TYPE,VALUE_TYPE>(origin,lengths);
		}
		return 0;
}

/*
template <class REAL, int N>
typename std::vector<TPoint<REAL,N> >::iterator fillingIterator(size_t n) {
}
*/
}

template struct spatial::Volume<double,double,2>;
template struct spatial::VolumeImplCreator<double,double,2>;

template struct spatial::Polygon<int16_t,double>;
template struct spatial::Polygons<int16_t,double>;
template struct spatial::Volume<int16_t,double,2>;
template struct spatial::VolumeImplCreator<int16_t,double,2>;

template struct spatial::Polygon<int32_t,double>;
template struct spatial::Polygons<int32_t,double>;
template struct spatial::Volume<int32_t,double,2>;
template struct spatial::VolumeImplCreator<int32_t,double,2>;

#ifdef COMPILE_64
template struct spatial::Polygon<int64_t,double>;
template struct spatial::Polygons<int64_t,double>;
template struct spatial::Volume<int64_t,double,2>;
template struct spatial::VolumeImplCreator<int64_t,double,2>;
template struct spatial::Polygon<int64_t,long double>;
template struct spatial::Polygons<int64_t,long double>;
template struct spatial::Volume<int64_t,long double,2>;
template struct spatial::VolumeImplCreator<int64_t,long double,2>;
#endif
