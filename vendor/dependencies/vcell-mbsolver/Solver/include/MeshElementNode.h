#ifndef MeshElementNode_h
#define MeshElementNode_h
#include <MPoint.h>
#include <fstream>
#include <vector>
#include <cmath>
#include <cstring>
#include <unordered_set>
#include <queue>
#include <MovingBoundaryCollections.h>
#include <Allocator.h>
#include <Volume.h>
#include <SVector.h>
#include <Segment.h>
#include <Mesh.h>
#include <VCellException.h>
#include <Logger.h>
#include <DiffuseAdvectCache.h>
#include <portability.h>
#include <math.h>
#include <persist.h>
#include <Physiology.h>
#include <vcarray.h>
#include <MBridge/FigureLimits.h>
#ifdef MESH_ELEMENT_NODE_STATE_TRACK
#define setState(x) DEBUG_SET_STATE(x, __FILE__, __LINE__)
#endif

namespace moving_boundary {
	struct VoronoiMesh;
	struct MeshElementNode;
	struct MeshElementNodeIdent;


	/**
	* proxy object to stream identifying information about MeshElementNode
	*/
	struct MeshElementNodeIdent {
		const MeshElementNode &mes;
		MeshElementNodeIdent(const MeshElementNode &m)
			:mes(m) {}
		void write(std::ostream &os) const; 
	};

	inline std::ostream & operator<<(std::ostream & os,const MeshElementNodeIdent& mesi) {
		mesi.write(os);
		return os;
	}

	/**
	* used by #MeshElementNode to store information about neighbors
	*/
	struct MeshElementNeighbor : public vcell_persist::Persistent {
		MeshElementNeighbor( )
			:element(nullptr),
			distanceTo( ),
			edgeLength( ),
			halfAdvectionFlux(nullptr)
			//daAmount(unset)
		{}
		MeshElementNeighbor(MeshElementNode *e);
		MeshElementNeighbor(std::istream & is,  const MeshElementNode & client) ;

		/**
		* compare persistent  parts of structure (only)
		*/
		bool operator==(const MeshElementNeighbor &rhs) const {
			return element == rhs.element 
				&& distanceTo == rhs.distanceTo
				&& edgeLength == rhs.edgeLength;
		}

		MeshElementNode *element; 
		moving_boundary::DistanceType distanceTo;
		moving_boundary::DistanceType edgeLength;
		/**
		* non-persistent; dot product of average velocity and normal vector times edgeLength
		*/
		BioQuanType* halfAdvectionFlux; 

		void persist(std::ostream &os, const MeshElementNode &client) const; 

		static void registerType( ) {
			spatial::ElementOffset<2>::registerType( );
			vcell_persist::Registrar::reg<MeshElementNeighbor>("MeshElementNeighbor");
		}
	};

	namespace MeshElementStateful {
		//if adding state, add to std::ostream & moving_boundary::operator<<(std::ostream &os ,moving_boundary::MeshElementStateful::State state) in *cpp 
		/**
		* describes both position relative to boundary and processing status. Names indicate last step started or completed 
		* -  trans -- tranisitioning
		* -  in -- inside boundary but not adjacent to boundary
		* -  Bnd -- inside and adjacent to boundary 
		* -  Out -- outside boundary 
		* -  DiffAdv -- diffusionAdvection 
		* -  Nbr -- neighbor 
		* -  MU -- mass updated, values copied from amtMassTransient to amtMass
		*
		* comments preceding enumeration indicate which function puts node in that state
		*/
		enum State { 
			initial,

			//stays inside sequence
			/**
			*  - setInitialPos( )
			*/
			initialInside,
			/**
			*  - beginSimulation( )
			*  - endOfCycle( )
			*/
			inStable,
			/**
			*  - react( )
			*/
			inReacted,
			/**
			*  - diffuseAdvect( )
			*/
			inDiffAdvDone,
			/**
			*  - collectMass( ) [sets neighbor to this state]
			*/
			inDiffAdvDoneMU,

			//stays boundary sequence
			/**
			*  - setInitialPos( )
			*/
			initialBoundary,
			/**
			*  - beginSimulation( )
			*  - endOfCycle( )
			*/
			bndStable,
			/**
			*  - moveFront( )
			*/
			bndFrontMoved,
			/**
			*  - findNeighborEdges( )
			*/
			bndNbrEdgesFound,
			/**
			*  - react( )
			*/
			bndReacted,
			/**
			*  - diffuseAdvect( )
			*/
			bndDiffAdvDone,
			/**
			*  - collectMass( ) [sets neighbor to this state]
			*/
			bndDiffAdvDoneMU,
			/**
			*  - applyFront( ) [from #bndDiffAdvDone(MU), #transInBnd]
			*/
			bndFrontApplied,


			//stays out sequence
			/**
			*  - setInitialPos( )
			*  - distributeMassToNeighors( ) [from boundary]
			*  - endOfCycle( )
			*/
			outStable,

			//in to boundary sequence
			/**
			*  - setPos( ) [applyFront( ) next]
			*/
			transInBnd,

			//boundary to in sequence
			/**
			*  - setPos( )
			*/
			//transBndIn,


			//out to boundary sequence
			/**
			*  - setPos( ) [first call]
			*/
			transOutBndSetIn,
			/**
			*  - setPos( ) [second call]
			*/
			transOutBndSetBnd,
			/**
			*  - updateBoundaryNeighbors( )
			*/
			transOutBndNbrSet,
			/**
			*  - collectMass( )
			*/
			transOutBndMassCollected, 
			/**
			*  - applyFront( )
			*/
			bndMassCollectedFrontApplied,


			//boundary to out sequence
			/**
			*  - setPos( ) [distributeMassToNeighors() next]
			*/
			transBndOut, 
		};
	}


	/**
	* the following is the expected sequence of calls which may transition state
	* - setInitialPos( )
	* - beginSimulation( )
	* - moveFront( )
	*  - findNeighborEdges( )
	* - diffuseAdvect( )
	* - setPos( )
	*  - updateBoundaryNeighbors( )
	* - collectMass( )
	* - applyFront( )
	*  - distributeMassToNeighbors( )
	* - endOfCycle( )
	*/

	struct MeshElementNode : public spatial::MPoint<moving_boundary::CoordinateType,2> , public spatial::VolumeMonitor {
		/**
		* context information needs by MeshElementNode
		*/
		class Environment {
			const spatial::MeshDef<moving_boundary::CoordinateType,2> & md; 
		public:
			const Physiology * physiology;
			const spatial::MeshDef<moving_boundary::CoordinateType,2> & mesh( ) const {
				return md;
			}
			Environment( const spatial::MeshDef<moving_boundary::CoordinateType,2> & meshDefinition_,
				const Physiology * physiology_)
				:md(meshDefinition_),
				physiology(physiology_) {}
		};
		typedef spatial::MeshDef<moving_boundary::CoordinateType,2> MeshDefinition; 
		typedef spatial::Mesh<moving_boundary::CoordinateType,2,MeshElementNode> MeshType; 
		typedef spatial::MPoint<moving_boundary::CoordinateType,2> base;
		typedef MeshElementNode OurType;
		typedef MeshElementNeighbor NeighborType;
		typedef MeshElementStateful::State State;
		typedef spatial::Segment<moving_boundary::CoordinateType,2> SegmentType; 
		typedef unsigned char BoundaryOffsetType;

		static spatial::DiffuseAdvectCache *createCache(const MeshDefinition & meshDef);

		MeshElementNode(const Environment &env_,const size_t *n, const moving_boundary::CoordinateType *values)
			:base(n,values),
			env(env_),
			stateVar(State::initial),
			bndOffset_old(0),
			bndOffset(0),
			interiorVolume(-1),
			vol(0,this),
			segments_( ),
			amtMass(),
			amtMassTransient(),
			concValue(),
			sourceTermValues(concValue),
			indexToTimeVariable(-1),
			interiorNeighbors( ),
			neighbors(interiorNeighbors.data( )), //default to interior, update when id'd as boundary
			boundaryNeighbors(0),
			cornerNeighbors(),
			voronoiVolume( ),
			nOutside(0),
			lastVolume(0)
		{
			std::fill(cornerNeighbors.begin( ),cornerNeighbors.end( ),nullptr);
			if (vcell_util::Logger::get( ).enabled(vcell_util::Logger::trace)) {
				logCreation( );
			}
		}
		//RFR
		bool isInside( ) const; 
		spatial::SurfacePosition mPos( ) const; 
		bool isOutside( ) const {
			return !isInside( );
		}
		/**
		* add operator to support storing in unordered_set
		*@return true if same object
		*/
		bool operator==(const MeshElementNode &rhs) const {
			return this == &rhs;
		}

		/**
		* maximum offset value computed, used to eliminate unneeded recursive and computation 
		*/
		BoundaryOffsetType maxOffset( ) const {
			if (isInside( )) {
				return 3;
			}
			return 2;
		}

		/**
		* return distance from boundary, up to #maxOffset( )
		* some computation steps can be skipped if we know node will not be adjacent to boundary.<br>
		*/
		BoundaryOffsetType boundaryOffset( ) const {
#ifdef OLD_BOUNDARY_OFFSETS
			return bndOffset_old;
#else
			return bndOffset;
#endif
		}

#ifndef OLD_BOUNDARY_OFFSETS
		/**
		 * this is a newly added method so only set new bndOffset
		 */
		void setBoundaryOffset(unsigned char offset )
		{
			bndOffset = offset;
		}
		void compareBoundaryOffsets()
		{
			if (bndOffset != bndOffset_old)
			{
				VCELL_LOG_ALWAYS("@" << indexPoint() << " different boundary offset " << bndOffset << " " << bndOffset_old);
			}
		}
#endif

		/**
		* set offset level for boundary value (recursively sets neighbors)
		* @throws std::domain_error if #boundaryOffset( ) != #unsetOffsetValue( ) or ! #isBoundary( )
		*/
		void setBoundaryOffsetValues();

	private:
		
		struct IdentityHash {
				size_t operator( )(const MeshElementNode *node) const {
					size_t raw = reinterpret_cast<size_t>(&node);
					return raw >> 3;
				}
		};
		typedef std::unordered_set<MeshElementNode *,IdentityHash> NodeSet;
		typedef std::queue<MeshElementNode *> NodeQueue;

		/**
		* set neighbor's offset value
		*/
		void propagateBoundaryValue(NodeSet &, NodeQueue &);
	public:

		static BoundaryOffsetType unsetOffsetValue( ) {
			return MeshElementNode::unsetBoundaryOffset;
		}

		bool isBoundary( ) const {
			return mPos( ) == spatial::boundarySurface;
		}

		/**
		* restore from persistent storage. 
		*/
		MeshElementNode(const Environment & , std::istream &is);

		/**
		* return proxy object identifying this for streaming to an ostream
		*/
		MeshElementNodeIdent ident( ) const {
			return MeshElementNodeIdent(*this);
		};

		size_t numVolumeVariables( ) const {
			return env.physiology->numVolumeVariables( );
		}

		/**
		* allocate species storage
		* #setPhysiology( ) must be calle first
		*/
		void allocateSpecies(); 


		/**
		* set initial concentration 
		*/
		void setConcentration(size_t i, moving_boundary::BioQuanType c) {
			using namespace MeshElementStateful;
			/*
			if (state( ) != initialInside && state( ) != initialBoundary) {
				throw std::domain_error("setConcentration");
			}
			*/
			if (vcell_portable::isNotANumber(c)) {
				throw std::invalid_argument("setConcentration not a number");
				std::cout << "nan" << std::endl;
			}
			assert (i < numVolumeVariables( ));
			assert ( c >= 0);
			if (c > 0) {
				amtMass[i] = c * volumePD( );
				concValue[i] = c;
			}
		}
		void setTransConcentration(size_t i, moving_boundary::BioQuanType c) {
			if (vcell_portable::isNotANumber(c)) {
				throw std::invalid_argument("setConcentration not a number");
				std::cout << "nan" << std::endl;
			}
			assert (i < numVolumeVariables( ));
			assert ( c >= 0);
			if (c > 0) {
				amtMassTransient[i] = c * volumePD( );
				concValue[i] = c;
			}
		}

		moving_boundary::BioQuanType priorConcentration(size_t i) const { 
			return concValue[i];
		}

		const moving_boundary::BioQuanType* priorConcentrations() const {
			return concValue.data();
		}

		/**
		* return previous concentration value;
		* does not necessarily equal value of last call
		* to #setConcentration
		*/
		moving_boundary::BioQuanType concentration(size_t i) const { 
			assert (i < numVolumeVariables( ));
			if (amtMass[i] > 0) {
				return amtMass[i] / volumePD( ); 
			}
			return 0;
		}


		/**
		* see #concentration
		*/
		moving_boundary::BioQuanType mass(size_t i) const { 
			assert (i < numVolumeVariables( ));
			return amtMass[i];
		}

		void setMass(size_t i, BioQuanType n) {
			amtMassTransient[i] = n;
		}

		/**
		* set position. See sequence diagram and state transition table
		*/
		void setPos(spatial::SurfacePosition m) ; 
		/**
		* set initial position
		*/
		void setInitialPos(spatial::SurfacePosition m); 

		/**
		* legacy from prior state impl
		*/
		void beginSimulation( ) {
			using namespace moving_boundary::MeshElementStateful;
			switch (state( )) {
			case initialInside:
				setState(inStable);
				lastVolume = interiorVolume;
				break;
			case initialBoundary:
				setState(bndStable);
				lastVolume = volumePD( );
				break;
			case outStable:
				break;
			default:
				badState("beginSimulation");
			}
		}

		/**
		* transfer interior neighbor edge length, distance and coordinate info to doppelganger
		* called once for each inside neighbor during initialization regardless of node status
		*/
		void setInsideNeighborValue(OurType & other, size_t slot, moving_boundary::DistanceType dist, moving_boundary::DistanceType length) {
			MeshElementNeighbor & iNbhr = interiorNeighbors[slot]; 
			assert(iNbhr.element == nullptr); 
			iNbhr.element = &other; 
			if (iNbhr.halfAdvectionFlux == nullptr)
			{
				iNbhr.halfAdvectionFlux = new BioQuanType[other.numVolumeVariables()];
			}
			std::memset(iNbhr.halfAdvectionFlux, 0, iNbhr.element->numVolumeVariables() * sizeof (BioQuanType));
			iNbhr.distanceTo = dist;
			iNbhr.edgeLength = length;
		}

		/**
		* called once for each corner neighbor during initialization regardless of node status
		*/
		void setCornerNeighborValue(OurType & other, size_t slot) {
			assert(cornerNeighbors[slot] == nullptr);
			cornerNeighbors[slot] = &other;
		}

		/**
		* initial setting of list of neighbors which are present 
		* this should be a boundary element. 
		* @param vm 
		* @param bn boundary neighbors inside front
		* @param front of interest
		*/
		void setBoundaryNeighbors(
			const VoronoiMesh &vm,
			std::vector<OurType *>  & bn, 
			const FrontType & front) {
				updateBoundaryNeighbors(vm,bn);
				formBoundaryPolygon(front);
		}

		/**
		* update volume to new front. Note the new control volume may not contain
		* center point -- the node is technically "outside" the front.
		* OLD: current (#mu) concentrations are updated
		* @mesh our mesh
		* @param front moved front
		*/
		void moveFront(const FrontType & front); 

		/**
		* update volume to new front. 
		* @mesh our mesh
		* @param front moved front
		* @param interiorVolume volume if node entirely interior 
		*/
		void applyFront( const FrontType & front, moving_boundary::CoordinateProductType interiorVolume); 

		/**
		* update list of neighbors which are present 
		* this should be a boundary element
		* not all calls result in state transition
		* @param vm 
		* @param bn boundary neighbors inside front
		*/
		void updateBoundaryNeighbors( const VoronoiMesh &vm, std::vector<OurType *>  & bn) ;

		/**
		* indicate diffusion-advection processing complete; swap concentration pointers, check interior deepness
		*/
		void endOfCycle( );  

		/**
		* volume, in problem domain units
		*/
		moving_boundary::CoordinateProductType volumePD(  ) const {
			//OPTIMIZATION? 
			using MeshElementStateful::State;
			switch(this->state( ) ) {
			case State::initialInside:
			case State::inStable:
			case State::inReacted:
			case State::inDiffAdvDone:
			case State::inDiffAdvDoneMU:
				assert(interiorVolume > 0); //should be set externally 
				return interiorVolume;
			case State::transBndOut: 
			case State::transInBnd:
			case State::transOutBndSetBnd:
			case State::transOutBndNbrSet: 
			case State::transOutBndMassCollected: 
			case State::initialBoundary:
			case State::bndFrontMoved:
			case State::bndReacted:
			case State::bndNbrEdgesFound:
			case State::bndDiffAdvDone:
			case State::bndDiffAdvDoneMU:
			case State::bndFrontApplied:
			case State::bndStable:
			case State::bndMassCollectedFrontApplied:
				return vol.volume( ) /distanceScaledSquared; 
			case State::transOutBndSetIn:
			default:
				badState("volumePD");
			}
			return 0;
		}

		void setInteriorVolume(moving_boundary::CoordinateProductType v) {
			interiorVolume = v;
		}

		/**
		* boundary element whose voronoi doesn't intersect front
		*/
		bool isBoundaryElementWithInsideNeighbors( ) const {
			return this->mPos( ) == spatial::boundarySurface && neighbors == interiorNeighbors.data( );
		}

		/**
		* distribute mass to neighbors 
		* (implies this element is being transBndOut)
		*/
		void distributeMassToNeighbors( ); 

		/**
		* collect mass from neighbors , if applicable
		*/
		void collectMass(const FrontType & front) {
			using namespace MeshElementStateful;
			if (state( ) == transOutBndNbrSet) {
				collectMassFromNeighbors(front);
			}
		}

		/**
		* return control volume for this -- possibly "expensive" for inside polygons as
		* will cause internal polygons to be constructed (if not already done)
		* @param mesh owning mesh
		*/
		const Volume2DClass & getControlVolume( ) const;


		const CoordVect& getAdvection(int sIndex) const {
			return advectionValue[sIndex];
		}

		void setAdvection(int sIndex, CoordVect& rhs) {
			advectionValue[sIndex] = rhs;
		}

		/**
		* apply source terms at specified time (expressions may contain "x" and "y" ... the nodes know that already)
		*/
		void react(moving_boundary::TimeType time, moving_boundary::TimeType timeStep); 

		template <class SOLVER>
		void diffuseAdvect(SOLVER &solver, unsigned int species);

		/**
		* notify node advect - diffuse cycle is complete
		*/
		template <class SOLVER>
		void advectComplete(SOLVER & solver, unsigned int species); 
			/*
			std::copy(amtMassTransient.begin( ),amtMassTransient.end( ),amtMass.begin( ));
		}
		*/

		/**
		* debug dump polygon && voronoi
		* @param os destination
		* @param limits optional outparam for polygon limits 
		* @param noPoly if true, don't write polygon out
		* @param precision std::setprecision( ), 0 uses a default
		*/
		void writeMatlab(std::ostream & os , matlabBridge::FigureLimits<moving_boundary::CoordinateType> * limits = nullptr,bool noPoly = false, int precision = 0) const;

		/**
		* set scale used to convert distances from problem domain to world coordinates;
		* used in DiffuseAdvect
		* @param pToW problem to world factor 
		*/
		static void setProblemToWorldDistanceScale(moving_boundary::BioQuanType pToW) {
			distanceScaled        = pToW;
			distanceScaledSquared = pToW * pToW;
		}

		/**
		* debug support
		*/
		bool matches (size_t xIndex, size_t yIndex, MeshElementStateful::State  particularState = static_cast<State>(-1)) const {
			if (! (this->index[spatial::cX] == xIndex && this->index[spatial::cY] == yIndex) ) {
				return false;
			}
			if (particularState != -1 && stateVar != particularState) {
				return false;
			}
			return true;
		}

		/**
		* write coordinates of boundary in text format 
		* @param dest 
		*/
		void listBoundary(std::ostream & os) const; 

		/**
		* find neighbor edges. We don't do this until after all the nodes have received the
		* new front #FrontMoved( )
		*/
		void findNeighborEdges(); 
		/**
		* clear segments
		*/
		void volumeChanged( ) override {
			segments_.clear( );
		}

		/**
		* return neighbor with specified offset. See also
		* Mesh#element( ...) 
		*/
		OurType *neighbor(spatial::ElementOffset<2> & eo) const;

		static void registerType( ) {
			base::registerType( );
			Volume2DClass::registerType( );
			NeighborType::registerType( );
			spatial::SVector<moving_boundary::VelocityType,2>::registerType( );
			SegmentType::registerType( );
			vcell_persist::Registrar::reg<MeshElementNode>("MeshElementNode");
		}

		void persist(std::ostream &);

		moving_boundary::CoordinateProductType getLastVolume(  ) const {
			return lastVolume;
		}

	private:
		void badState(const char * const) const;
		/**
		* log creation information to trace file
		*/
		void logCreation( ) const ;
		/*
		VCELL_LOG(trace, "creation " << this->indexInfo( ) << " (" << this->get(spatial::cX) << ',' << this->get(spatial::cY) << ')'
		*/
		/**
		* rebuild and sort #segments from volume
		*/
		void volumeToSegments();
		/**
		* access #segments_; use function in case we want to optimize in future
		*/
		std::vector<SegmentType> & segments( ) {
			if (!segments_.empty( )) {
				return segments_;
			}
			volumeToSegments( );
			return segments_;
		}
		/**
		* const access #segments_; use function in case we want to optimize in future
		*/
		const std::vector<SegmentType> & segments( ) const {
			return segments_;
		}
		/**
		* @mesh our mesh
		* @param front moved front
		*/
		void applyFrontLegacyVoronoiSet(const FrontType & front);
		/**
		* collect mass to from neighbors 
		* (implies this element is new for this generation 
		*/
		void collectMassFromNeighbors(const FrontType & front); 

		/**
		* set list of neighbors which are present 
		* this should be a boundary element. 
		* @param vm  
		* @param bn boundary neighbors inside front
		*/
		void processBoundaryNeighbors(const VoronoiMesh & vm, std::vector<OurType *>  & bn);
		/**
		* return amount of overlap betwen current voronoi region and
		* specified previous volume
		* @param oldVolume volume to find overlap of
		* @return volume of overlap
		*/
		moving_boundary::CoordinateProductType voronoiOverlap(const Volume2DClass& oldVolume); 

		/**
		* return number of neighbors
		*/
		size_t numNeighbors( )  const {
			if (neighbors == interiorNeighbors.data( )) {
				return interiorNeighbors.ArraySize;
			}
			return boundaryNeighbors.size( );
		}
		/**
		* create inside polygon -- used for inside elements and boundary elements whose vornoi
		* doesn't cross front
		* return is stored in vol
		* @param mesh mesh of simulation
		*/
		Volume2DClass createInsidePolygon(); 

		/**
		* form polygon from intersection of voronoi result and front, and 
		* calculate distances and edge lengths to neighbors
		* @param mesh ourMesh
		* @param front the front
		*/
		void formBoundaryPolygon(const FrontType & front); 

		/**
		* find neighbor's record of us, if it exists
		*/
		const NeighborType * findUs(OurType &us) const {
			for (size_t i = 0; i < numNeighbors( ); i++) {
				if (neighbors[i].element == &us) {
					return &neighbors[i];
				}
			}
			return nullptr;
		}

		/**
		* return distance to neighbor, if it's been calculated; otherwise return 0
		*/
		moving_boundary::DistanceType distanceToNeighbor(OurType &us) const {
			const NeighborType * nt = findUs(us);
			if (nt != nullptr) {
				return nt->distanceTo;
			}
			return 0;
		}
#ifdef MESH_ELEMENT_NODE_STATE_TRACK
		void DEBUG_SET_STATE(MeshElementStateful::State s, const char *file, int line);
#else 
		void setState(MeshElementStateful::State s) {
			using namespace MeshElementStateful;
			stateVar = s;
			VCELL_LOG(trace, ident( ) << " new state" ); 
			assert(debugSetState( ));
		}
#endif

		/*
		* hook to allow setting specific break /log conditions in *cpp 
		* @returns true
		*/
		bool debugSetState( );

		MeshElementStateful::State state( ) const {
			return stateVar;
		}

		/**
		* debug only 
		*/
		void genDebugPlot(std::ostream & dest, const Volume2DClass &ourVolume, const Volume2DClass & intersection, const moving_boundary::FrontType *front);
		void genDebugPlot(std::ostream & dest, const Volume2DClass &ourVolume, const Volume2DClass & intersection);

		const Environment &env;

		MeshElementStateful::State stateVar;
		/**
		* distance from boundary (more or less)
		*/
		BoundaryOffsetType bndOffset_old;
		BoundaryOffsetType bndOffset;
		/**
		* volume if this is an inside element -- otherwise volume comes from vol object
		*/
		moving_boundary::CoordinateProductType interiorVolume;
		/**
		* our control volume
		*/
		Volume2DClass vol;
		std::vector<SegmentType> segments_;
		std::vector<moving_boundary::BioQuanType> amtMass; //amount of mass
		std::vector<moving_boundary::BioQuanType> amtMassTransient; //amount of mass, working copy
		std::vector<moving_boundary::BioQuanType> concValue; //concentration at beginning of cycle
		std::vector<moving_boundary::BioQuanType> diffusionValue; //cached diffusion constant values
		/**
		* values needed to evaluate source term
		* for now, assume the indexing of species in the Physiology is the same as the local values
		* and use the same vector as #concValue
		* { must be declared after concValue }
		*/
		static_assert(std::is_same<moving_boundary::BioQuanType,double>::value, "update implementation");
		std::vector<double> & sourceTermValues;
		size_t indexToTimeVariable;


		vcell_util::vcarray<NeighborType,4> interiorNeighbors; 
		NeighborType * neighbors;
		std::vector<NeighborType> boundaryNeighbors;
		vcell_util::vcarray<OurType *,4> cornerNeighbors; 
		Volume2DClass voronoiVolume;
		int nOutside;
		std::vector<CoordVect> advectionValue;
		/**
		* cached last stable volume
		*/
		moving_boundary::CoordinateProductType lastVolume; 

		/**
		* problem domain to solution coordinates scaled
		*/
		static moving_boundary::BioQuanType distanceScaled;
		/**
		* problem domain to solution coordinates scale, squared
		*/
		static moving_boundary::BioQuanType distanceScaledSquared;
		/**
		* value for unset boundary offset 
		*/
		static BoundaryOffsetType unsetBoundaryOffset;

		/**
		* boundary neighbor functor; declared as part of class to give acccess to typedefs
		*/
		struct SetupBoundaryNeighbor;


	private:
		/**
		* not implemented
		*/
		MeshElementNode & operator=(const MeshElementNode &rhs);
		friend struct MeshElementNodeIdent;
	};

	/**
	* exception when reverse lengths don't match
	*/
	struct ReverseLengthException : public std::logic_error {
		ReverseLengthException(const std::string & msg, MeshElementNode &a, const MeshElementNode &b) 
			:std::logic_error(msg),
			aElement(a),
			bElement(b) {}
		const MeshElementNode &aElement;
		const MeshElementNode &bElement;
	};

	/**
	* when element skips boundary state
	*/
	struct SkipsBoundary : public std::exception {
		MeshElementNode &mes;
		const spatial::SurfacePosition newPosition;
		std::string str;
		SkipsBoundary(MeshElementNode &m, spatial::SurfacePosition np) 
			:mes(m),
			newPosition(np),
			str( )
		{
			std::ostringstream oss;
			oss << "Skipped Boundary " << mes.ident( ) << " to " << newPosition << std::ends;
			str = oss.str( );
		}
		virtual ~SkipsBoundary( ) throw( ) {}
		virtual const char* what() const throw() {
			return str.c_str( );
		}
	};

	std::ostream & operator<<(std::ostream &,MeshElementStateful::State);

	inline void MeshElementNodeIdent::write(std::ostream &os) const {
		os << mes.indexInfo( ) << ' ' << mes.state( ); 
	}
}
#endif
