#ifndef Mesh_h
#define Mesh_h
#include <cassert>
#include <vector>
#include <limits>
#include <algorithm>
#include <TPoint.h>
#include <DiffuseAdvectCache.h>
#include <boost/iterator/iterator_facade.hpp>
#include <persist.h>
#include <Universe.h>
#include <GeoLimit.h>
#include <IndexVect.h>
#include <CoordVect.h>
#ifdef TRACK_STORAGE
#define STORAGE(x) std::cout << x << std::endl;
#else
#define STORAGE(x) 
#endif
#ifdef free
#error free macro defined
#endif
namespace spatial {
	//forward
	template <int N> struct ElementOffset; 

	/**
	* helper class for mesh to allow specialization of recursive template
	* for D == 0 case
	*/
	template <int N, int D>
	struct MeshHelper {
		/**
		* return storage size of given dimension
		*/
		template <class MESH>
		static size_t dimsize(const MESH &m) { 
			return m.nPoints[D-1] * MeshHelper<N, D - 1>::dimsize(m); 
		}

		/**
		* return index in storage array of given multi dimension
		* coordinates
		*/
		template <class MESH>
		static size_t index(const MESH & m ,const std::array<size_t,N> & position) {
			int idx =  N - D;
			const size_t current = position[idx] * dimsize(m);
			const size_t lower = MeshHelper<N,D - 1>::index(m,position);
			return lower + current;
		}
	};

	/**
	* specialzation for 0 to stop recursion
	*/
	template <int N>
	struct MeshHelper<N, 0> {
		template <class MESH>
		static size_t dimsize(const MESH &m) { 
			return 1;
		}

		template <class MESH>
		static size_t index(const MESH & m ,const std::array<size_t,N> & position) {
			return position[0];
		}
	};

	//forward
	//struct VoronoiResult;

	//forward
	template<class CT, int N, class TELEMENT> struct Mesh;

	/**
	* definition of mesh geometry
	*/
	template<class CT, int N>
	struct MeshDef {
		typedef CT realType;
		typedef unsigned int species_type;
		static int numDim( ) { return N; }

		//default
		MeshDef( ) 
			:origin( ),
			intervals( ),
			nPoints( ), 
			minInterval() { }

		/**
		* @param origin_ coordinates of "lower left" corner of modeled region
		* @param sizes_ sizes of sides
		* @param nPoints_ how many points in each region
		*/
		MeshDef(const std::array<CT,N> & origin_, const std::array<CT,N> & sizes_, const std::array<size_t,N> & nPoints_, species_type numSpecies_ = 1)
			:origin(origin_),
			intervals( ),
			nPoints(nPoints_), 
			numSpecies(numSpecies_),
			minInterval(std::numeric_limits<CT>::max( )) {
				for (int i = 0; i < N; i++) {
					intervals[i] = static_cast<CT>(sizes_[i]/nPoints[i]);
					minInterval = std::min(minInterval,intervals[i]);
				}
		}
		/**
		* default origin to 0,0,...
		* @param sizes_ sizes of sides
		* @param nPoints_ how many points in each region
		*/
		/*
		MeshDef(const std::array<CT,N> & sizes_, const std::array<size_t,N> & nPoints_)
			:origin( ),
			intervals( ),
			nPoints(nPoints_), 
			minInterval(std::numeric_limits<CT>::max( )) {
				for (int i = 0; i < N; i++) {
					intervals[i] = static_cast<CT>(sizes_[i]/nPoints[i]);
					minInterval = std::min(minInterval,intervals[i]);
				}
		}
		*/

		/**
		* copy ctor
		*/
		MeshDef(const MeshDef & rhs)
			:origin(rhs.origin),
			intervals(rhs.intervals),
			nPoints(rhs.nPoints),
			numSpecies(rhs.numSpecies),
			minInterval(rhs.minInterval) {}

		MeshDef(std::istream & is) {
			using vcell_persist::binaryReader;

			vcell_persist::Token::check<MeshDef<CT,N> >(is); 
			binaryReader<CT> br(is);
			std::for_each(origin.begin( ), origin.end( ), br);
			std::for_each(intervals.begin( ), intervals.end( ), br);
			std::for_each(nPoints.begin( ), nPoints.end( ), binaryReader<size_t>(is) ); 
			br(minInterval);
			binaryReader<species_type> speciesReader(is);
			speciesReader(numSpecies);
		}


		void persist(std::ostream &os) const {
			using vcell_persist::binaryWriter;

			vcell_persist::Token::insert<MeshDef<CT,N> >(os); 
			binaryWriter<CT> bw(os);
			std::for_each(origin.begin( ), origin.end( ), bw);
			std::for_each(intervals.begin( ), intervals.end( ), bw);
			std::for_each(nPoints.begin( ), nPoints.end( ), binaryWriter<size_t>(os) ); 
			bw(minInterval);
			binaryWriter<species_type> speciesWriter(os);
			speciesWriter(numSpecies);
		}

		static void registerType( ) {
			vcell_persist::Registrar::reg<MeshDef<CT,N>,CT,N>("MeshDef");
		}

		CT startCorner(Axis a) const {
			return origin[a];
		}

		CT interval(Axis a) const {
			return intervals[a];
		}

		CT totalDistance(Axis a) const {
			return static_cast<CT>(intervals[a] * nPoints[a]);
		}

		size_t numCells(Axis a ) const {
			return nPoints[a];
		}

		size_t numCells( ) const {
			size_t s = 1;
			for (int i =0; i < N; i++) {
				s *= nPoints[i];
			}
			return s;
		}

		CT minimumInterval( ) const {
			return minInterval;
		}

		/*
		species_type numberSpecies( ) const {
			return numSpecies;
		}
		*/
		/**
		* translate point from grid coordinates to scaled
		*/
		template <class IN_TYPE>
		TPoint<CT,N> gridToSpatial(const TPoint<IN_TYPE,N> & gridReferencedPoint) const {
			TPoint<CT,N> rval; 
			for (int d = 0 ; d < N; d++ ) {
				Axis a = static_cast<Axis>(d);
				rval(a) = origin[d] + (gridReferencedPoint(a) + 0.5) * intervals[d];
			}
			return rval;
		}

		/**
		* return vector of indexes for specified dimension
		*/
		std::vector<CT> coordinateValues(spatial::Axis a) const;

		/**
		* return coordinate on grid > value in dimension 
		* @value to get >
		* @dimension of interest
		*/
		CT greaterGridPoint(CT value, Axis dimension) const {
			CT offset = value - origin[dimension];
			size_t count = static_cast<size_t>(offset / intervals[dimension] );
			CT rval = static_cast<CT>(origin[dimension] + (count + 1) * intervals[dimension]); 
			return rval;
		}

		/**
		* return coordinate on grid <= value in dimension 
		* @value to get <=
		* @dimension of interest
		*/
		CT lesserGridPoint(CT value, Axis dimension) const {
			CT offset = value - origin[dimension];
			size_t count = static_cast<size_t>(offset / intervals[dimension] );
			CT rval = static_cast<CT>(origin[dimension] + count  * intervals[dimension]); 
			if (rval < value) {
				return rval;
			}
			return rval - intervals[dimension];
		}

		/**
		* return a very rough hash value 
		*/
		size_t checkvalue( ) const {
			size_t cv = 0;
			CT coordCv = 0;
			for (int i = 0; i < N; i++) {
				coordCv +=origin[i];
				coordCv +=intervals[i] * 17;
				cv^=nPoints[i];
			}
			coordCv = std::abs(coordCv);
			while (coordCv < ( std::numeric_limits<size_t>::max( ) / 100) ) {
				coordCv *= 10;
			}
			return cv ^ static_cast<size_t>(coordCv);
		}

	protected:

		std::array<CT,N> origin; 
		/**
		* distance between adjacent points 
		*/
		std::array<CT,N> intervals; 
		/**
		* number nodes in one dimension
		*/
		std::array<size_t,N> nPoints; 
		CT minInterval;
		species_type numSpecies;
	};

	/**
	* encapsulate compact storage of element's location 
	*/
	struct MeshPosition {
		MeshPosition( )
			:index(0){}

		template <typename T>
		MeshPosition(const T &i) 
			:index(i) {}

		/**
		* convert to specified type
		* @tparam type to conver to
		*/
		template <typename T>
		T to( ) const {
			check<T>();
			return static_cast<T>(index);
		}

	private:
		typedef size_t IndexType;
		 template<class CT, int N, class TELEMENT> friend struct Mesh;
		 IndexType index;
#ifdef NDEBUG
		template <typename T> void check( ) const {}
#else
		template <typename T> void check( ) const;
#endif
	};

	inline bool operator< (const MeshPosition &lhs, const MeshPosition &rhs) {
		return lhs.to<size_t>( ) < rhs.to<size_t>( ) ;
	}

	template<class CT, int N, class TELEMENT>
	struct Mesh : public MeshDef<CT,N> {
		//default
		Mesh( ) 
			:MeshDef<CT,N>( ),
			storage(nullptr),
			daCache(nullptr) {}

		Mesh(const MeshDef<CT,N> &definition, const typename TELEMENT::Environment & env) 
			:MeshDef<CT,N>(definition),
			storage(nullptr),
			daCache(TELEMENT::createCache(definition))
		{
			std::array<size_t,N> loop;
			std::array<CT,N> startPoint; 
			for (int i =0; i < N; i++) {
				loop[i] = 0; 
				startPoint[i] = this->origin[i] + this->intervals[i] / 2;
			}
			const size_t needed =  this->numCells( ) * sizeof(TELEMENT);
			storage = static_cast<TELEMENT *>(malloc(needed));
			STORAGE("meshdef allocated " << needed << " at " << storage)
			
			do {
				std::array<CT,N> values;
				for (int d = 0; d < N; d++) {
					values[d] = static_cast<CT>(loop[d] * this->intervals[d] + startPoint[d]);
				}
				size_t idx = index<N-1>(loop);
				void * addr = &storage[idx];
				new (addr) TELEMENT(env,loop.data( ) ,values.data( )); //placement new
			}
			while (increment(0,loop));

			const std::array<spatial::GeoLimit, 2> & limits = moving_boundary::Universe<2>::get().limits( );
			originVect = CoordVect(limits[0].low(), limits[1].low());
			nxVect = IndexVect(this->nPoints);
			dxVect = CoordVect(limits[0].span(), limits[1].span())/nxVect;
		}

		/*
		The following would require updating each element with a new reference to "this";
		not worth the effort
		Mesh(std::istream &is)
		Mesh & operator=(Mesh &rhs) {
		 */

		void restore(std::istream &is, const typename TELEMENT::Environment &env) {
			static_cast<MeshDef<CT,N> &>(*this) = MeshDef<CT,N>(is);
			daCache = TELEMENT::createCache(*this);
			vcell_persist::Token::check<Mesh<CT,N,TELEMENT> >(is); 
			const size_t nCells =  this->numCells( );
			const size_t needed =  nCells * sizeof(TELEMENT);
			storage = static_cast<TELEMENT *>(malloc(needed));
			STORAGE("istream allocated " << needed << " at " << storage)
			for (size_t i = 0; i < nCells; i++) {
			void * addr = &storage[i];
				new (addr) TELEMENT(env,is); //placement new
			}
		}
		
		void persist(std::ostream &os) const {
			typedef MeshDef<CT,N> base;
			//note the base:: qualifier need for gcc
			base::persist(os);
			vcell_persist::Token::insert<Mesh<CT,N,TELEMENT> >(os); 
			size_t nCells = base::numCells( );
			for (size_t i = 0; i < nCells; i++) {
				storage[i].persist(os);
			}
			//daCache is not persisted as it's transient data -- easier just to rebuild from scratch
		}

		static void registerType( ) {
			TELEMENT::registerType( );
			MeshDef<CT,N>::registerType( );
			vcell_persist::Registrar::reg<Mesh<CT,N, TELEMENT>,CT,TELEMENT,N>("Mesh");
		}
	private:

		CoordVect originVect;
		CoordVect dxVect;
		IndexVect nxVect;

		/**
		* increment loop to next set of coordinates
		* @param dimension to increment
		* @loop in/out coordinates to increment, should be of length N
		*/
		bool increment(int dim, std::array<size_t,N> & loop) const {
			if (dim == N) {
				return false;
			}

			if (loop[dim] < this->nPoints[dim] - 1) {
				loop[dim]++;
				return true;
			}
			if (dim < N && increment(dim+1,loop)) {
				loop[dim] = 0;
				return true;
			}
			return false;
		}


		/**
		* return storage size of given dimension
		*/
		template <int D>
		size_t dimsize( ) const { 
			return MeshHelper<N,D>::dimsize(*this);
		}

		/**
		* return index in storage array of given multi dimension
		* coordinates
		*/
		template <int D>
		size_t index(const std::array<size_t,N> & position) const {
			return MeshHelper<N, D>::index(*this,position);
		}

		TELEMENT *storage;
		DiffuseAdvectCache *daCache;

		static void callDestructor(TELEMENT &element)  {
			element.~TELEMENT( );
		}

	public:
		typedef TELEMENT elementType;

		~Mesh( ) {
			std::for_each(begin( ),end( ),callDestructor);
			STORAGE("destructor freeing " << storage)
			free(storage);
			delete(daCache);
		}


		DiffuseAdvectCache &diffuseAdvectCache( ) { 
			return *daCache;
		}

		/**
		* retrieve element given set of coordinates
		* @param position ( length N ) to retrieve
		* undefined behavior if invalid coordinates
		*/
		TELEMENT &get(const std::array<size_t,N> & position) const {
			return storage[index<N-1>(position)];
		}
		/**
		* retrieve element
		* this is provided for compactness / persistence
		* @param index -- stored index
		*/

		TELEMENT &get(const MeshPosition &mp) const {
			return storage[mp.index];
		}

		/**
		* convert position to persistable object 
		*/
		MeshPosition indexOf(const std::array<size_t,N> & position) const {
			return MeshPosition( index<N-1>(position) );
		}

		/**
		* retrieve element given set of coordinates
		* @param position ( length N ) to retrieve
		* return null pointer if coordinates out of range
		*/
		TELEMENT *query(const std::array<size_t,N> & position) const {
			for (int i = 0; i < N; i++) {
				if (position[i] < 0 || position[i] >= this->nPoints[i]) {
					return nullptr;
				}
			}
			return &get(position);
		}

		const CoordVect& geoOrigin() const
		{
			return originVect;
		}

		const CoordVect& Dx() const
		{
			return dxVect;
		}

		const IndexVect& Nx() const
		{
			return nxVect;
		}

		int getElementIndex(const IndexVect& gridIndex) const
		{
			return gridIndex[1] * this->nPoints[0] + gridIndex[0];
		}
		/**
		* retrieve element given set of coordinates
		* @param position ( length N ) to retrieve
		* return null pointer if coordinates out of range
		*/
		TELEMENT *query(const IndexVect& gridIndex) const
		{
			for (int i = 0; i < N; i++)
			{
				if (gridIndex[i] < 0 || gridIndex[i] >= this->nPoints[i])
				{
					return nullptr;
				}
			}
			int elementIndex = getElementIndex(gridIndex);
			return &storage[elementIndex];
		}


		/**
		* iterator base class
		*/
		template<class ETYPE> 
		class ibase : public boost::iterator_facade<ibase<ETYPE>, ETYPE,std::forward_iterator_tag> {
		public:
			typedef const Mesh<CT,N,TELEMENT> OwnerType; 
			OwnerType & owner;
		private:
			std::array<size_t,N> position;
		public:
			ibase(OwnerType &own)
				: owner(own), 
				position( ) {}
			ibase(OwnerType &own, const size_t *pos)
				: owner(own) {
					for (int i = 0; i <N; i++) {
						position[i] = pos[i];
					}
			}
			template <class ALT>
			ibase(ibase<ALT> const & rhs)
				:owner(rhs.owner),
				position(rhs.getPosition( )) {}

			const std::array<size_t,N> & getPosition( ) const {
				return position;
			}

			//ideally these would be private but I couldn't get the proper "friend" declaration to work
			//in VS2012
			bool increment(int dim) {
				if (dim < N) {
					if (position[dim] < owner.nPoints[dim] -1 ) { 
						position[dim]++;
						return true;
					}
					const bool istat = increment(dim + 1);
					if (istat) {
						position[dim] = 0;
					}
					return istat;
				}
				//we're going to use position[N-1] == owner.nPoints[N-1] for "end" position
				position[dim-1]++;
				return false;
			}
			template <class ALT>
			bool equal(const ibase<ALT> & rhs) const {
				for (int i = 0; i < N ; i++) {
					if (position[i] != rhs.getPosition( )[i]) {
						return false;
					}
				}
				return true;
			}

			void increment( ) {
				increment(0);
			}

			ETYPE & dereference( ) const {
				ETYPE & rval = owner.get(position);
				return rval;
			}

		};
		//helper classes
		template<int A, int B> friend struct MeshHelper;
		friend class ibase<TELEMENT>;
		friend class ibase<const TELEMENT>;
		/**
		* forward iterator
		*/
		typedef ibase<TELEMENT> iterator;
		/**
		* const forward iterator
		*/
		typedef ibase<const TELEMENT> const_iterator;

		const_iterator begin( ) const {
			const_iterator rval(*this);
			return rval;
		}

		const_iterator end( ) const {
			size_t construct[N];
			for (int i = 0; i < N; i++) {
				construct[i] = this->nPoints[i] - 1; 
			}
			construct[N-1]++;
			return const_iterator(*this,construct);
		}

		iterator begin( ) {
			iterator rval(*this);
			return rval;
		}

		iterator end( )  {
			size_t construct[N];
			for (int i = 0; i < N; i++) {
				auto current = this->nPoints[i];
				if (current == 0) {
					return begin( );
				}
				construct[i] = current - 1; 
			}
			construct[N-1]++;
			return iterator(*this,construct);
		}

		/**
		* return element with given offset from provided element.
		* see also MPoint#offset( ... )
		* @param src element from
		* @param eo offset 
		*/
		TELEMENT * element(const TELEMENT & src, const ElementOffset<N> & eo) const {
			const std::array<size_t,N> & from = src.indexes( );
			std::array<size_t,N> indexes;
			for (int i = 0; i < N; ++i) {
				indexes[i] = from[i] + eo.offsets[i];
			}
			return query(indexes);
		}
	};


}
#endif
