#include <MPoint.h>
#include <sstream>
#include <limits>
#include <cstring>
#include <exception>
#include <algorithm>
#include <iomanip>
#include <MeshElementNode.h>
#include <VCellFront.h>
#include <VoronoiResult.h>
#include <VoronoiMesh.h>
#include <algo.h>
#include <intersection.h>
#include <MovingBoundaryParabolicProblem.h>
#include <Edge.h>
#include <LoopIterator.h>
#include <stack_container.h>
#include <vcellutil.h>
#include <Logger.h>
#include <Distance.h>
#include <persistcontainer.h>
#include <NoChangeSentinel.h>

#include <ExplicitSolver.h>

#include <ManagedArrayPtr.h> //StackPtr
#include <MBridge/FronTierAdapt.h>
#include <MBridge/Figure.h>
#include <MBridge/MatlabDebug.h>
//tolerances
namespace {
	/**
	* edge lengths calculated between cells should be this close
	*/
	const double toleranceEdgeLengthException = 1e-8;
	/**
	* diffusion to and from cells must be within this amount
	*/
	const double toleranceDiffuseAdvectException = 1e-10;
	/**
	* distance between neighbors should be within this amount
	*/
	const double toleranceNeighborDistancesAssert = 1e-10;
	/**
	* difference squared difference, relative to mesh.mininumInterval<br>
	* used to see if segments are perpendicular between point and therefore d-a edges
	*/
	const double tolerancePerpendicularSegmentDistanceSquared = 1e-3;
	struct MesDistancePolicy {
		typedef long double DistanceSquaredType;
		typedef moving_boundary::CoordinateType DistanceType;
		static void check(int32_t) {}
		template <typename U>
		static DistanceType convert(U u) {
			return u;
		}
	};
	static_assert(std::is_same<moving_boundary::CoordinateType,MesDistancePolicy::DistanceType>::value, "typedef mismatch");
	typedef spatial::Distance<moving_boundary::CoordinateType,MesDistancePolicy> MesDistance;
}

using spatial::cX;
using spatial::cY;
using spatial::TPoint;
using namespace moving_boundary::MeshElementStateful;
using matlabBridge::MatLabDebug;
using  moving_boundary::MeshElementNode;
using  moving_boundary::VoronoiMesh;
using  moving_boundary::FrontType;

using spatial::SurfacePosition;
using spatial::interiorSurface;
using spatial::boundarySurface;
using spatial::outsideSurface;
using spatial::unsetPosition;

using spatial::MeshDef;


//local definitions
namespace {
	//for file names
	int missCounter;

	int debugAid = 0;

	/**
	* used to compare forward and backward diffuse advect values
	*/
	struct CheckValue {
		CheckValue(bool r = false, double v = 0)
			:reversed(r),
			value(v) {}
		bool reversed; 
		/**
		* mass transferred
		*/
		double value;
	};

	struct DaCache : public spatial::DiffuseAdvectCache { 
		typedef std::pair<const MeshElementNode * ,const MeshElementNode *> MesPair;
		typedef std::map<MesPair,CheckValue> Map;
		DaCache(const spatial::MeshDef<moving_boundary::CoordinateType,2> & md) 
			:diffuseAdvectMap( ),
			edgeLengthTolerance(1),
			meshDef(md) { }

		virtual void start( ) {
			diffuseAdvectMap.clear( );
		}

		virtual void finish( ) {
			std::map<MesPair,CheckValue>::const_iterator iter = diffuseAdvectMap.begin( ); 
			for (;iter != diffuseAdvectMap.end( ); ++iter) {
				const CheckValue & cv = iter->second;
				if (!cv.reversed && cv.value != 0) {
					const MesPair & ePair = iter->first;
#ifdef PENDING_DISCUSSION
					VCELL_EXCEPTION(logic_error,ePair.first->ident( ) 
						<< " to " << ePair.second->ident( ) << " not reversed" << std::endl);
#else
					VCELL_LOG(warn, ePair.first->ident( ) 
						<< " to " << ePair.second->ident( ) << " with mass " << cv.value
						<< " not reversed" << std::endl);
#endif
				}
			}
		}

		Map diffuseAdvectMap;
		const moving_boundary::DistanceType edgeLengthTolerance; 
		const spatial::MeshDef<moving_boundary::CoordinateType,2> & meshDef;
	};

	/**
	* converts mass to concentration by dividing by specified volume 
	*/
	struct MassToConcentration {
		/**
		* @parm volume
		* @throws std::domain_error if volume < = 0
		*/
		MassToConcentration( moving_boundary::VolumeType volume_)
			:volume(volume_) {
				if (volume <= 0) {
					throw std::domain_error("MassToConcentration volume <= 0");
				}
		}

		moving_boundary::BioQuanType operator( )(moving_boundary::BioQuanType mass) {
			return mass / volume;
		}

		moving_boundary::VolumeType volume;
	};
}

void MeshElementNode::logCreation( ) const {
	using vcell_util::Logger;
	Logger & logger = Logger::get( );
	assert(logger.enabled(Logger::trace));

	World<moving_boundary::CoordinateType,2> & world = World<moving_boundary::CoordinateType,2>::get( );
	moving_boundary::CoordinateType x = get(spatial::cX);
	moving_boundary::CoordinateType y = get(spatial::cY);
	double px = world.toProblemDomain(x,spatial::cX);
	double py = world.toProblemDomain(y,spatial::cY);

	std::ostringstream oss;
	oss << "creation " << this->ident( ) << " (" << this->get(spatial::cX) << ',' << this->get(spatial::cY) << "), ("
		<< px << ',' << py << ')';
	logger.report(oss.str( ).c_str());
}

spatial::DiffuseAdvectCache * MeshElementNode::createCache(const spatial::MeshDef<moving_boundary::CoordinateType,2> & meshDef) {
	return new DaCache(meshDef);
}

bool MeshElementNode::debugSetState( ) {
	return true;
}
void MeshElementNode::setPos(SurfacePosition m)  {
	if (m == mPos( )) {
		return;
	}
	VCELL_LOG(trace,this->ident( ) << " (t) position " << m << " from " << state( ));
	VCELL_KEY_LOG(verbose,Key::setPos,this->ident( ) << " (v) position " << m << " from " << state( ));
	switch (state( )) {
		//case initial:
	case inStable:
	case inDiffAdvDone:
		//case inStableDeep:
		//case inDeepDiffAdvDone:
		if (m == boundarySurface) {
			neighbors = boundaryNeighbors.data( ); 
			setState(transInBnd);
			return;
		}

		//case bndFrontMoved:
		//case bndNbrEdgesFound:
	case bndDiffAdvDone:
		if (m == interiorSurface) {
			neighbors = interiorNeighbors.data( ); 
			vol.clear( );
			getControlVolume( );
			setState(inDiffAdvDone);
			lastVolume = interiorVolume;
			return;
		}
		else {
			assert(m == outsideSurface);
			setState(transBndOut);
			return;
		}
		//case bndFrontApplied:
		//case bndFrontAppliedMU:
		//case bndStable:
	case outStable:
		if (m == interiorSurface) {
			setState(transOutBndSetIn);
			return;
		}
		//case outStableDeep:
		//case transBndOut: 
		//case transInBnd:
		//case transOutBndSetBnd:
		//case transOutBndNbrSet:
		//case transOutBndMassCollected: 
	case transOutBndSetIn:
		if (m != boundarySurface)  {
			badState("setPos");
		}
		neighbors = boundaryNeighbors.data( ); 
		setState(transOutBndSetBnd);
		return;
	}
	VCELL_EXCEPTION(domain_error,this->ident( ) << " position " << m << " from " << state( ));
}

struct MeshElementNode::SetupBoundaryNeighbor {
	OurType &clientElement;
	SetupBoundaryNeighbor(OurType &client)
		:clientElement(client) {}

	NeighborType operator( )(OurType *nb) {
		NeighborType rval;
		rval.element = nb;
		if (rval.halfAdvectionFlux == nullptr)
		{
			rval.halfAdvectionFlux = new BioQuanType[clientElement.numVolumeVariables()];
		}
		std::memset(rval.halfAdvectionFlux, 0, rval.element->numVolumeVariables() * sizeof (BioQuanType));

		const moving_boundary::DistanceType existing = nb->distanceToNeighbor(clientElement);
		if (existing > 0) {
			rval.distanceTo = existing; 
			moving_boundary::DistanceType  da = MesDistance::approximate(*nb,clientElement);
			if (existing != da) {
				std::cout << "oops " << da << std::endl;
			}
		}
		else {
			rval.distanceTo = MesDistance::approximate(*nb,clientElement);
		}

		return rval;
	};
};

void MeshElementNode::processBoundaryNeighbors(const VoronoiMesh & vm, std::vector<OurType *>  & bn) {
	VCELL_LOG(trace,this->ident( ) << " processBoundaryNeighors "); 
	//getting vornoi volume and setting up boundary neighbors in same function due to legacy reasons ...
	{ //clarity scope, building voronoiVolume
		using spatial::VoronoiResult; 
		VoronoiResult vResult;
		VoronoiResult::GhostVector & voronoiVertices = vResult.vertices;
		assert(voronoiVertices.empty( ));
		/*if (matches(47,58))  {
			int y = 3;
		} */
		vm.getResult(vResult,*this);
#ifdef DEBUG_VORONOI_OUTPUT
		if (matches(47,58)) 
		{
		std::ofstream vp("voronoiPoly.m");
		vp << "% " << ident( ) << std::endl;
		vp << "% " << vResult.type << std::endl;
		matlabBridge::TPolygon<long long> p("r-+");
		frontTierAdapt::copyVectorInto(p,vResult.vertices);
		p.close( );
		vp << p;
		}
#endif

		voronoiVolume.clear( );
		Volume2DClass::FillingIteratorType fIter = voronoiVolume.fillingIterator(voronoiVertices.size());
		std::copy(voronoiVertices.begin( ),voronoiVertices.end( ),fIter);

		//check for single open line
		if (vResult.type == spatial::VoronoiResult::straightLine) { 
			assert(voronoiVertices.size( ) == 3);
			spatial::SVector<moving_boundary::CoordinateType,2> delta0(*this,*this);
			spatial::SVector<moving_boundary::CoordinateType,2> delta(*this,voronoiVertices[1]);
			delta *= -3;
			//go other direction to build box
			//delta.reverse( );
			FrontPointType add1 = spatial::displacement(voronoiVertices[2],delta);
			FrontPointType add2 = spatial::displacement(voronoiVertices[0],delta);
			voronoiVolume.add(add1);
			voronoiVolume.add(add2);
		}
		voronoiVolume.close( );
	}
	boundaryNeighbors.resize(bn.size( ));
	std::transform(bn.begin( ),bn.end( ),boundaryNeighbors.begin( ), SetupBoundaryNeighbor(*this));

	neighbors = boundaryNeighbors.data( );
}

void MeshElementNode::moveFront(const FrontType & front) {
	using namespace MeshElementStateful;
	switch (state( )) {
	case bndStable:
		formBoundaryPolygon(front);
		setState(bndFrontMoved);
		return;
	case inStable:
		return;
	}
	badState("moveFront");
}


void MeshElementNode::applyFrontLegacyVoronoiSet( const FrontType & front) {
	assert(0);
	/*
	assert(!voronoiVolume.empty( ));
	assert(state( ) == bndFrontApplied || state( ) == bndFrontAppliedMU); 
	formBoundaryPolygon(front);
	setState(stableUpdated);
	*/
}
void MeshElementNode::applyFront( const FrontType & front, moving_boundary::CoordinateProductType interiorVolume) {
	switch (state( )) {
	case bndDiffAdvDone:
	case bndDiffAdvDoneMU:
	case transInBnd:

		//case bndNbrUpdated:
		formBoundaryPolygon(front);
		setState(bndFrontApplied);
		break;
	case transOutBndMassCollected:
		setState(bndMassCollectedFrontApplied);
		break;
	case outStable:
	case inDiffAdvDone: 
	case inDiffAdvDoneMU: 
	case transBndOut:
		break;
	default:
		badState("applyFront"); //REVISIT
	}
}

void MeshElementNode::formBoundaryPolygon( const FrontType & front) {
	assert(isBoundary( ));
	using spatial::Edge;

	assert(!voronoiVolume.empty( ));
	//VoronoiResult &vResult = *pVoronoiResult;
	VCELL_LOG(debug,this->ident( ) << " formBoundaryPolygon old volume " << vol.volume( ));
	vol = voronoiVolume.intersection(front); 
	if (state( ) == initial) {
		return;
	}
	if (vol.empty( )) {
		matlabBridge::Polygon pFront("r",1);
		frontTierAdapt::copyVectorInto(pFront,front);
		matlabBridge::Polygons vs("-g",2);
		frontTierAdapt::copyVectorsInto(vs,voronoiVolume.points( ));
		{
			std::ofstream eDebug("debugEmpty.m");
			eDebug << pFront << vs;
		}
				throw std::logic_error("empty boundary front");

	}
	if (vol.points( ).size( ) > 1) {
			static int multiCounter = 0;
			std::string mcounter("multiregion");
			mcounter += std::to_string(++multiCounter);
			std::string filename(mcounter);
			filename += ".m";
			std::ofstream mr(filename);
			mr <<  matlabBridge::FigureName(mcounter); 
			genDebugPlot(mr,vol,voronoiVolume,&front);
		}

	VCELL_LOG(debug,this->ident( ) << " new volume " << vol.volume( )) ;
}
#ifdef OLD_STUFF
if (MatLabDebug::on("edgefind")) { 
	matlabBridge::Polygons pPolys("k",3);
	frontTierAdapt::copyVectorsInto(pPolys,vol.points( ));
	matlabBridge::Polygons vs("-r",1);
	frontTierAdapt::copyVectorsInto(vs,voronoiVolume.points( ));
	MatLabDebug::stream() << pPolys << vs << matlabBridge::pause << matlabBridge::clearFigure;
}

matlabBridge::Polygon trace(":+g");
matlabBridge::Polygon pedge("-+r",2);
#endif

void MeshElementNode::findNeighborEdges() {
	static const std::string METHOD = "findNeighborEdges";
	vcell_util::Logger::debugEntry(METHOD);
	switch (state( )) {
	case bndFrontMoved:
		VCELL_LOG(verbose,ident( ) << " findNeighborEdges");
		setState(bndNbrEdgesFound);
		if (vol.empty( )) {
			getControlVolume( );
		}
		break;
	case initialBoundary:
		break;
	default:
		badState("findNeighborEdges");
	}

	typedef TPoint<moving_boundary::CoordinateType,2> MeshPointType;
	bool foundANeighbor = false;
	bool bigSeg = false;
	for (size_t i = 0; i < boundaryNeighbors.size( ); i++) {
		DistanceType edgeLength = 0;
		OurType & nb = *boundaryNeighbors[i].element;
		if (nb.state( ) == bndFrontMoved) {
			if (nb.vol.empty( )) {
				nb.getControlVolume();
			}
		}
		//std::array<SegmentType,8> segs; //this was 4, 11 Apr 2016; not sure why
		std::vector<SegmentType> segs; //this was 4, 11 Apr 2016; not sure why
		int trials = 0;
		while(trials < 2) {
			trials += 2;
			std::set_intersection(segments( ).begin( ), segments( ).end( ), nb.segments( ).begin( ), nb.segments( ).end( ), std::back_inserter(segs));
			for (int s = 0; s< segs.size( ); ++s) {
				if (segs[s].singular( )) {
					if (s == 0 && spatial::taxicabDistance<int>(indexPoint( ), nb.indexPoint( ) ) == 1) {
						VCELL_LOG(warn,ident( ) << ',' << nb.ident( ) << " neighbor edge miss");
						{
							std::ostringstream fn;
							fn << "miss" << missCounter++ << ".m" << std::ends;
							std::ofstream m(fn.str( ));
							m << "% green is "  << nb.ident( ) << std::endl;
							m << "% blue "  << ident( ) << std::endl;
							matlabBridge::TPolygons<CoordinateType> nPolys("g",3);
							frontTierAdapt::copyVectorsInto(nPolys,nb.vol.points( ));
							m << nPolys; 
							matlabBridge::TPolygons<CoordinateType> oPolys("b",3);
							frontTierAdapt::copyVectorsInto(oPolys,vol.points( ));
							m << oPolys; 
						}
						spatial::SegToVerbose<CoordinateType,2> sToV;
						std::ostream_iterator<spatial::VerboseSegment<CoordinateType,2> > oi(std::cout,"\n");
						std::cout << ident( ) << " at " << *this << " segments" << std::endl; 
						std::transform(segments( ).begin( ), segments( ).end( ), oi,sToV); 
						std::cout << nb.ident( ) << " at " << nb << " segments" << std::endl; 
						std::transform( nb.segments( ).begin( ), nb.segments( ).end( ), oi, sToV); 
						trials--;
						std::cout << "rebuilding " << ident( ) << " and " <<  nb.ident( ) << std::endl << std::endl;
						//rebuild segments and try again
						segments_.clear( );
						nb.segments_.clear( );
					}
					break;
				}
				foundANeighbor = true;
				edgeLength += segs[s].magnitude<DistanceType>( ); 
				VCELL_LOG(verbose,ident( ) << ',' << nb.ident( ) << " neighbor edge hit");
				boundaryNeighbors[i].edgeLength = edgeLength; 
			}
		}
		if (segs.size( ) > 7) {
			bigSeg = true;
		}
	}
#ifdef NOT_YET_NOT_YET 
	spatial::EdgeAccessor<moving_boundary::CoordinateType,moving_boundary::CoordinateProductType,2> accsr = vol.accessor( );
	for (;accsr.hasNext( );accsr.next( )) {
		debugAid++;
		Edge<moving_boundary::CoordinateType,2> edge = accsr.get( );
		for (size_t i = 0; i < boundaryNeighbors.size( ); i++) {
			const OurType & nb = *boundaryNeighbors[i].element;
			debugAid++;
			const SquaredType nbHDistSquare = MesDistance::squared(nb,edge.origin( ));
			if (smallRelativeDifference<SquaredType>(hDistSquare - nbHDistSquare,meshMin,tolerancePerpendicularSegmentDistanceSquared)) {
				const SquaredType nbTDistSquare = MesDistance::squared(nb,tail);
				if (smallRelativeDifference<SquaredType>(tDistSquare - nbTDistSquare, meshMin,tolerancePerpendicularSegmentDistanceSquared)) {
					const double mag = edge.edgeVector( ).magnitude( );
					const moving_boundary::DistanceType length = static_cast<moving_boundary::DistanceType>(mag);
					VCELL_KEY_LOG(debug,"MeshElementNode.formBoundaryPolygon",this->ident( ) << " between " << nb.ident( ) << " vector " << edge.edgeVector( ) << " length " 
						<< std::setprecision(12) << length << " <" <<  std::setprecision(12) << edge.origin( ) << " : " 
						<< std::setprecision(12) << tail << " >");
					boundaryNeighbors[i].edgeLength = std::max(boundaryNeighbors[i].edgeLength,length);
					if (MatLabDebug::on("edgefind")) {
						frontTierAdapt::copyPointInto(pedge, edge.origin());
						frontTierAdapt::copyPointInto(pedge,tail);
						std::ostream & os = MatLabDebug::stream( );
						os << pedge;
						pedge.clear( );
					}
				}
			}
		}
		if (MatLabDebug::on("edgefind")) {
			frontTierAdapt::copyPointInto(trace,edge.origin( ));
		}

	}
#endif
	if (!foundANeighbor) {
		std::cout << ident( ) << " is lonely " << std::endl;
	}
	if (MatLabDebug::on("edgefind") && bigSeg) {
		matlabBridge::Scatter nbplot('b',2);
		frontTierAdapt::copyPointInto(nbplot,*this);
		for (size_t i = 0; i < boundaryNeighbors.size( ); i++) {
			const OurType & nb = *boundaryNeighbors[i].element;
			frontTierAdapt::copyPointInto(nbplot,nb);
		}
		std::ostream & os = MatLabDebug::stream( );
		std::stringstream ss;
		ss << this->index[cX] << ',' << this->index[cY];
		os <<  nbplot << matlabBridge::Text(this->coord[cX],this->coord[cY],ss.str( ).c_str( )) 
			<< matlabBridge::pause << matlabBridge::clearFigure;
	}
	vcell_util::Logger::debugExit(METHOD);
}

void MeshElementNode::updateBoundaryNeighbors(const VoronoiMesh & vm, std::vector<OurType *>  & bn) {
	if (bn.size( ) == 0) {
		throw std::logic_error("implement isolated point");
	}

	bool processNeighbors = false;
	switch (state( )) {
	case initialBoundary:
	case bndDiffAdvDone:
	case transInBnd:
		processNeighbors = true;
		//setState(bndStable);
		break;
		processNeighbors = true;
		break;
	case transOutBndSetBnd:
		setState(transOutBndNbrSet);
		processNeighbors = true;
		break;
		break;
	default:
		badState("invalid state updateBoundaryNeighbors");
	}
	if (!processNeighbors) {
		bool changed =  boundaryNeighbors.size( )  != bn.size( ) ;
		if (!changed) {
			//check new and previous set of boundary neighbors to see if changed
			for (int i = 0; !changed && i < bn.size( ) ;i++) {
				NeighborType & oldNb = boundaryNeighbors[i];
				changed =  oldNb.element != bn[i];
			}
		}
		processNeighbors = changed;
	}
	if (processNeighbors) {
		boundaryNeighbors.clear( );
		processBoundaryNeighbors(vm,bn);
	}
	if (neighbors == nullptr) {
		throw std::logic_error("N");
	}
}

void MeshElementNode::writeMatlab(std::ostream  &os , matlabBridge::FigureLimits<moving_boundary::CoordinateType> * limits, bool noPoly, int precision) const {
	World<moving_boundary::CoordinateType,2> & world = World<moving_boundary::CoordinateType,2>::get( );
	World<moving_boundary::CoordinateType,2>::PointConverter pointconverter = world.pointConverter( );

	if (!vol.empty( )) {
		if (!voronoiVolume.empty( )) {
			matlabBridge::TPolygons<moving_boundary::CoordinateType> pVoro("r",1);
			if (precision != 0) {
				pVoro.setPrecision(precision);
			}
			frontTierAdapt::copyVectorsInto(pVoro,voronoiVolume.points( ));
			os << pVoro; 
			if (limits != nullptr) {
				limits->merge(pVoro.figureLimits( ));
			}
		}
		matlabBridge::Scatter scatter('g',30,true);
		frontTierAdapt::copyPointInto(scatter,*this);
		std::stringstream ss;
		ss << this->index[cX] << ',' << this->index[cY];
		os << scatter << matlabBridge::Text(this->coord[cX],this->coord[cY],ss.str( ).c_str( ));
		if (!noPoly) {
			Volume2DClass::VectorOfVectors vOfV = vol.points( );
			for (Volume2DClass::VectorOfVectors::const_iterator vvIter = vOfV.begin( ); vvIter != vOfV.end( );++vvIter) {
				matlabBridge::TPolygon<moving_boundary::CoordinateType> pPoly("k",3);
				if (precision != 0) {
					pPoly.setPrecision(precision);
				}
				frontTierAdapt::copyVectorInto(pPoly,*vvIter);
				os << pPoly; 
				if (limits != nullptr) {
					limits->merge(pPoly.figureLimits( ));
				}
			}
		}
	}
}

moving_boundary::Volume2DClass MeshElementNode::createInsidePolygon() {
	Volume2DClass rval(1);
	const moving_boundary::CoordinateType width = env.mesh( ).interval(cX);
	const moving_boundary::CoordinateType height = env.mesh( ).interval(cY);

	moving_boundary::CoordinateType x = this->coord[cX] - width / 2;
	moving_boundary::CoordinateType y = this->coord[cY] - height  / 2;
	std::array<moving_boundary::CoordinateType,2> origin = { x, y};
	rval.add(origin);
	origin[cX] += width;
	rval.add(origin);
	origin[cY] += height;
	rval.add(origin);
	origin[cX] -= width;
	rval.add(origin);
	rval.close();

	return rval; 
}

void MeshElementNode::volumeToSegments() {
	Volume2DClass::SegAccessor sa = vol.accessor( );
	assert(segments_.empty( ));
	if (vol.empty( )) {
		std::cout<< "vtos on empty volume " << ident( ) << std::endl;
	}
	while (sa.hasNext( )) {
		const SegmentType & st = sa.getAndAdvance( );
		//if horizontal segment longer than cell side, cut into pieces
		if (st.horizontal( ) &&  ( st.axialDistance(cX) > env.mesh( ).interval(cX) ) ) {
			const CoordinateType & y = st.a( )(cY); 
			CoordinateType left = st.a( )(cX); 
			CoordinateType right = st.b( )(cX); 
			assert (left < right);

			CoordinateType cut = 0;
			while (right - left > env.mesh( ).interval(cX)) {
				cut = env.mesh( ).greaterGridPoint(left,cX); 
				assert(cut < right);
				segments_.push_back(SegmentType(CoordinatePoint(left,y), CoordinatePoint(cut,y)) );
				left = cut;
			}
			if (cut != right) {
				segments_.push_back(SegmentType(CoordinatePoint(cut,y), CoordinatePoint(right,y)) );
			}
		}
		//if vertical segment longer than cell side, cut into pieces
		else if (st.vertical( ) && st.axialDistance(cY) > env.mesh( ).interval(cY)) {
			const CoordinateType & x = st.a( )(cX); 
			const CoordinateType & a = st.a( )(cY); 
			const CoordinateType & b = st.b( )(cY); 
			CoordinateType down = a < b ? a : b; 
			CoordinateType up = b > a ? b: a; 
			assert (down < up);

			CoordinateType cut = 0;
			while (up - down > env.mesh( ).interval(cY)) {
				cut = env.mesh( ).greaterGridPoint(down,cY); 
				assert(cut < up);
				segments_.push_back(SegmentType(CoordinatePoint(x,down), CoordinatePoint(x,cut)) );
				down = cut;
			}
			if (cut != up) {
				segments_.push_back(SegmentType(CoordinatePoint(x,cut), CoordinatePoint(x,up)) );
			}
		}
		else {
			segments_.push_back(st);
		}
	}
	std::sort(segments_.begin( ), segments_.end( ));
}

const moving_boundary::Volume2DClass & MeshElementNode::getControlVolume( ) const {
	if (!vol.empty( )) {
		return vol;
	}
	OurType & us = const_cast<OurType &>(*this);
	typedef TPoint<moving_boundary::CoordinateType, 2> MeshPointType;
	switch (state( )) {
		//case initial:
	case initialInside:
	case inStable:
	case inDiffAdvDone:
	case inDiffAdvDoneMU:
		us.vol =  us.createInsidePolygon( );
		break;
	case bndFrontMoved:
	case bndNbrEdgesFound:
	case bndDiffAdvDone:
	case bndDiffAdvDoneMU:
	case bndFrontApplied:
	case bndStable:
		if (isBoundaryElementWithInsideNeighbors( )) {
			us.vol = us.createInsidePolygon( );
		}
		else {
			VCELL_EXCEPTION(logic_error,this->ident( ) << "has no polygon")   ;
		}
		break;
	case outStable:
	case transOutBndNbrSet: 
		break;
		/*
		case transBndOut: 
		case transInBnd:
		case transOutBndSetBnd:
		case transOutBndNbrSet:
		case transOutBndMassCollected: 
		case transOutBndSetIn:
		*/
	default:
		badState("getControlVolume");
		break;
	}
	return vol;
}

inline moving_boundary::CoordinateProductType MeshElementNode::voronoiOverlap(const moving_boundary::Volume2DClass &oldVolume) {
	Volume2DClass intersection = voronoiVolume.intersection(oldVolume);
	return intersection.volume( ); 
}

void MeshElementNode::distributeMassToNeighbors() {
	if (state( ) != transBndOut) {
		return;
	}
	const Volume2DClass & ourVolume = getControlVolume();
	VCELL_LOG(debug,this->ident( ) << " dMtoN mass " << amtMassTransient[0] << " vol " << volumePD( ));

	const size_t nN = numNeighbors( );
	const moving_boundary::CoordinateProductType volumeValue = volumePD( );
	assert(volumeValue == ourVolume.volume( ) / distanceScaledSquared);
	bool distributed = false;
	moving_boundary::BioQuanType massSentinel = amtMassTransient[0];
	for (int i = 0; i < nN; i++) {
		assert(neighbors[i].element != nullptr);
		OurType & nb = *neighbors[i].element;
		if (nb.state( ) == bndFrontApplied) { 
			debugAid++;
			Volume2DClass intersection = ourVolume.intersection(nb.getControlVolume());
			moving_boundary::CoordinateProductType iVol = intersection.volume( ) / distanceScaledSquared;
			assert(iVol>=0);
			if (iVol > 0) {
				distributed = true;
				moving_boundary::CoordinateProductType nbVol = nb.volumePD( );
				for (size_t s = 0; s < numVolumeVariables( ); s++) {
					moving_boundary::BioQuanType m = iVol * amtMassTransient[s] / volumeValue; 
					VCELL_COND_LOG(debug, s == 0 , " " << this->ident( ) << " giving " <<nb.ident( ) << " mass " << m 
						<< " has vol " << nbVol << " existing mass " << nb.amtMassTransient[0] );
					nb.amtMassTransient[s] += m; 
					massSentinel -= m;
					VCELL_COND_LOG(debug, s == 0 , "  " << nb.ident( ) << " new mass " << nb.amtMassTransient[0] );
				}
			}
		}
		/*
		else {
		if (nb.state( ) != transBndOut) {
		std::cout << "dMtoN " << nb.state( ) << std::endl;
		}
		}
		*/
	}
	// mass is gone
	if (massSentinel > 0.001) {
		std::ofstream script("massLost.m");
		script << matlabBridge::FigureName("transBndOut mass");
		writeMatlab(script);
		for (int i = 0; i < nN; i++) {
			script << matlabBridge::pause << matlabBridge::clearFigure;
			neighbors[i].element->writeMatlab(script);
		}
	}
	std::fill(amtMass.begin( ),amtMass.end( ),0);
	std::fill(concValue.begin( ), concValue.begin( ) + numVolumeVariables( ),0);
	vol.clear( );
	setState(outStable);

	if (!distributed) {
        for (int i = 0; i < nN; i++) {
            VCELL_LOG_ALWAYS("Neighbor " << i << " " << neighbors[i].element->ident());
        }
		VCELL_EXCEPTION(logic_error, this->ident( ) << " with " << nN << " neighbors has none to distribute to") ;
	}
}

void MeshElementNode::collectMassFromNeighbors(const FrontType & front) {
	if (state( ) != transOutBndNbrSet) {
		badState("collectMassFromNeighbors");
	}
	allocateSpecies( );
	if (amtMass[0] != 0) {
		VCELL_EXCEPTION(domain_error,"non zero new mass");
	}
	VCELL_LOG(debug,this->ident( ) << "collecting Mass");
	formBoundaryPolygon(front);
	const Volume2DClass & ourVolume = getControlVolume();
	VCELL_LOG(debug,this->ident( ) << " collecting with volume " << ourVolume.volume( ) / distanceScaledSquared);
	const size_t nN = numNeighbors( );
	moving_boundary::CoordinateProductType totalVolume = 0;
	for (int i = 0; i < nN; i++) {
		assert(neighbors[i].element != nullptr);
		OurType & nb = *neighbors[i].element;
		//if (nb.state( ) == bndFrontApplied || nb.state( ) == legacyInteriorSet ) {
		switch (nb.state( )) {
			//here we copy the transOutBndSetIn mass to the base -- at this point in the cycle we don't need the original any more
			//this allows us to use original as basis for pre-collection concentration 
		case inDiffAdvDone:
			std::copy(nb.amtMassTransient.begin ( ), nb.amtMassTransient.end( ), nb.amtMass.begin( ));
			nb.setState(inDiffAdvDoneMU);
			break;
		case bndDiffAdvDone:
			std::copy(nb.amtMassTransient.begin ( ), nb.amtMassTransient.end( ), nb.amtMass.begin( ));
			nb.setState(bndDiffAdvDoneMU);
			break;
		}

		const MeshElementStateful::State nbState = nb.state( );
		//if (nbState == bndFrontAppliedMU || nbState == legacyInteriorSetCollected) {
		if (nbState == inDiffAdvDoneMU || nbState == bndDiffAdvDoneMU ) {
			Volume2DClass intersection = ourVolume.intersection(nb.getControlVolume( ));
			CoordinateProductType intersectVolume = intersection.volume( );
			intersectVolume /= distanceScaledSquared;  //World -> pd conversion
			VCELL_LOG(debug, this->ident( ) << " overlap volume with " <<nb.ident( ) << " is " << intersectVolume);
			assert(intersectVolume>=0);
			if (intersectVolume > 0) {
				moving_boundary::CoordinateProductType donorVolume = nb.volumePD( );
				for (size_t s = 0; s < numVolumeVariables( ); s++) {
					const moving_boundary::BioQuanType mu = nb.amtMass[s] / donorVolume; 
					VCELL_LOG(verbose,this->ident( ) << ':' << nb.ident( ) << " volume " << nb.volumePD( ) << " conc " << mu); 
					moving_boundary::BioQuanType m =  mu * intersectVolume; 
					amtMass[s] += m;
					nb.amtMassTransient[s] -= m;
					VCELL_COND_LOG(debug, s == 0, this->ident( ) << " adding mass " <<  m  << " from " <<nb.ident( ) << ' ' << nb.state( ));
					if (s== 0 && matlabBridge::MatLabDebug::on("collectmass")) {
						const char semi = ';';
						static int collectCounter = 1;
						std::ostringstream oss;
						oss << "collection" << std::setfill('0') << std::setw(2) <<collectCounter++ << ".m";
						std::ofstream mlScript(oss.str( ));

						matlabBridge::TPolygons<CoordinateType> nbpolys(":+b",2);
						frontTierAdapt::copyVectorsInto(nbpolys,nb.getControlVolume( ).points( ));
						nbpolys.setPolyAreaName("neighborArea");

						matlabBridge::TPolygons<CoordinateType> mepolys(":xg",2);
						frontTierAdapt::copyVectorsInto(mepolys,ourVolume.points( ));
						mepolys.setPolyAreaName("nodeArea");

						matlabBridge::TPolygons<CoordinateType> ipolys("-+r",1);
						frontTierAdapt::copyVectorsInto(ipolys,intersection.points( ));
						ipolys.setPolyAreaName("intersectArea");

						mlScript << nbpolys << mepolys << ipolys;
						mlScript << matlabBridge::Text(get(cX),get(cY),indexInfo( ).str( ).c_str( ));
						mlScript << matlabBridge::Text(nb(cX),nb(cY),nb.indexInfo( ).str( ).c_str( ));
						using std::endl;
						mlScript << "nodevol = " <<std::setprecision(20) << ourVolume.volume( ) <<  semi << endl;
						mlScript << "nbvol = " << std::setprecision(20) << nb.getControlVolume( ).volume( ) <<  semi << endl;
						mlScript << "ivol = " << std::setprecision(20) << intersection.volume( ) <<  semi << endl;
						mlScript << "sourcemass = " << nb.amtMass[s] << semi << endl;
						mlScript << "transfermass = " << m << semi << endl;
						mlScript << "%transOutBndMassCollected " << indexInfo( ).str( ) << " vol " << std::setprecision(20) << ourVolume.volume( ) << endl;
						mlScript << "%neighbor " << nb.indexInfo( ).str( ) << " vol " << std::setprecision(20) << nb.getControlVolume( ).volume( ) << endl;
						mlScript << "%intersection vol " << std::setprecision(20) << intersection.volume( ) << endl;
					}
				}
			}
		}
		else if (nbState != transOutBndNbrSet && nbState != transOutBndMassCollected) {
			VCELL_EXCEPTION (logic_error, ident( ) << " not collecting from " << nb.ident( )); //REVISIT

			VCELL_KEY_LOG(debug,Key::notCollecting,"Not collecting from " << nb.ident( ));
		}
		//testing show some overlap with other neighbor states, but it was ~ 1 part in 10^6 of volume
	}
	VCELL_LOG(debug, this->ident( ) << " final mass is " << amtMass[0] << " vol " << ourVolume.volume( ) / distanceScaledSquared << " conc " << concentration(0)); 
	setState(transOutBndMassCollected); 
}

void MeshElementNode::endOfCycle( ) {
	VCELL_LOG(verbose,this->ident( ) << " begin eoc"); 
	bool copyMass = true;
	bndOffset_old = unsetOffsetValue( );
	switch (state( )) {
	case outStable:
		copyMass = false;
		vol.clear( );
		break;
	case inDiffAdvDone:
	case inDiffAdvDoneMU:
		setState(inStable);
		break;
	case bndFrontApplied:
		setState(bndStable);
		lastVolume = volumePD( );
		break;
	case bndMassCollectedFrontApplied:
		copyMass = false;
		lastVolume = volumePD( );
		setState(bndStable);
		break;
	default:
		badState("illegal endOfCycle state");
	}
	if (copyMass) {
		VCELL_LOG(verbose,this->ident( ) << " eoc prior copy " << amtMassTransient[0] << " mass " << amtMass[0]);
		std::copy(amtMassTransient.begin( ), amtMassTransient.end( ), amtMass.begin( )); 
	} else {
		VCELL_LOG(verbose,this->ident( ) << " eoc not copying ");
	}
	const bool inside = isInside( );
	if (inside) {
		moving_boundary::CoordinateProductType vol = this->volumePD( );
		if (vol > 0) {
			std::transform(amtMass.begin( ),amtMass.end( ),concValue.begin( ), MassToConcentration(vol) );
		}
		else {
			VCELL_EXCEPTION(logic_error, ident( ) << " insided with zero / neg volume " << vol);
			//std::fill(concValue.begin( ),concValue.begin( ) + physiology( ).numberSpecies( ), 0); 
		}
	}
	if (vcell_util::Logger::get( ).enabled(vcell_util::Logger::info) ) { 
		if (inside) {
			VCELL_LOG_ALWAYS(this->ident( ) << " eoc end mass " << this->mass(0) << " vol " << volumePD(  ) << " conc " << concentration(0));
		}
		else {
			VCELL_LOG_ALWAYS(this->ident( ) << " eoc outside"); 
		}
	}
}

void MeshElementNode::allocateSpecies( ) {
	assert(concValue == sourceTermValues); //if this is no longer term, allocate sourceTermValues separately

	const size_t i = numVolumeVariables( );
	amtMass.resize(i);
	amtMassTransient.resize(i);
	diffusionValue.resize( i );
	advectionValue.resize( i );

	const Physiology * physio = env.physiology;
	concValue.resize( physio->numberSymbols( ) );
	indexToTimeVariable = physio->symbolIndex("t");
	const size_t xIndex = physio->symbolIndex("x");
	const size_t yIndex = physio->symbolIndex("y");

	World<moving_boundary::CoordinateType,2> & world = World<moving_boundary::CoordinateType,2>::get( );
	moving_boundary::CoordinateType x = get(spatial::cX);
	moving_boundary::CoordinateType y = get(spatial::cY);
	const double px = world.toProblemDomain(x,spatial::cX);
	const double py = world.toProblemDomain(y,spatial::cY);
	sourceTermValues[xIndex] = px;
	sourceTermValues[yIndex] = py;
}

namespace {
	struct SourceTermEvaluator {
		typedef std::vector <moving_boundary::BioQuanType> ValueVector; 
		SourceTermEvaluator (ValueVector & v)
			:values(v) {}

		moving_boundary::BioQuanType operator( )(const moving_boundary::VolumeVariable* sp) {
			moving_boundary::BioQuanType r = sp->evaluateExpression(moving_boundary::expr_source, values.data());
			return r;
		}

		ValueVector &values;
	};

	/**
	* converts mass to concentration by dividing by specified volume 
	*/
	struct ConcToMassAndAdd {
		typedef moving_boundary::BioQuanType BioQuan; 
		/**
		* @param n client node (for logging) 
		* @param volume
		* @param timeStep_
		* @throws std::domain_error if volume < = 0
		*/
		ConcToMassAndAdd(MeshElementNode & n ,moving_boundary::VolumeType volume_, moving_boundary::TimeType timeStep_)
			:node(n),
			volTimeProduct(volume_ * timeStep_) {
				if (volume_ <= 0) {
					throw std::domain_error("ConcToMassAndAdd volume <= 0");
				}
				VCELL_KEY_LOG(verbose,Key::reactionTerm,node.ident( ) << " ConcToMass vol: " <<  volume_ << ", timeStep:  " << timeStep_ << " = " << volTimeProduct);
		}

		BioQuan operator( )(BioQuan mass, BioQuan sourceConcentration) {
			BioQuan newMass = mass + sourceConcentration * volTimeProduct;
			VCELL_KEY_LOG(verbose,Key::reactionTerm,node.ident( ) << " react " << mass << " + " << sourceConcentration << " * " << volTimeProduct << " = " << newMass) ;
			return newMass;
		}

		const moving_boundary::VolumeTimeProduct volTimeProduct;
		const MeshElementNode & node;
	};
}

void MeshElementNode::react(moving_boundary::TimeType time, moving_boundary::TimeType timeStep) {
	/*
	auto ms = vcell_util::makeSentinel("mass", ident( ), amtMass[0]);
	auto cs = vcell_util::makeSentinel("concentration" ,ident( ), concValue[0]);
	*/

	switch (state( )) {
	case inStable:
		setState(inReacted);
		break;
	case bndFrontMoved: 
		findNeighborEdges();
		//fall through
	case bndNbrEdgesFound:
		setState(bndReacted);
		break;
	case inDiffAdvDone:
	case bndDiffAdvDone:
		break;
	default:
		VCELL_EXCEPTION(domain_error,"Invalid react state " << ident( ));
	}
	if (lastVolume <= 0) {
		VCELL_EXCEPTION(logic_error, ident( ) << " lastVolume " << lastVolume << " in react");
	}

	assert(concValue == sourceTermValues); //if this is no longer term, copy values from concValue ->sourceTermValues

	sourceTermValues[indexToTimeVariable] = time;
	const Physiology * physio = env.physiology;
	//const std::vector<const biology::Species> & species = physiology( ).species( );
	const size_t nVolumeVariables = physio->numVolumeVariables( );
	std::vector <moving_boundary::BioQuanType> sourceTermConcentrations(nVolumeVariables);

	//evaluate source terms
	std::transform(physio->beginVolumeVariable(), physio->endVolumeVariable(), sourceTermConcentrations.begin( ), SourceTermEvaluator(sourceTermValues) );

	//convert concentrations to mass, add to existing mass
	assert(sourceTermValues.size( ) >= amtMass.size( ));
	std::transform(amtMass.begin( ), amtMass.end( ),sourceTermConcentrations.begin( ),amtMass.begin( ), ConcToMassAndAdd(*this,lastVolume, timeStep) );
	std::transform(amtMass.begin( ),amtMass.end( ),concValue.begin( ), MassToConcentration(lastVolume) );

	//diffusion Prep
	//fix states in a bit
	switch (state( )) {
	case inReacted: 
		setState(inDiffAdvDone);
		break;
	case bndReacted:
		setState(bndDiffAdvDone); 
		break;
	case inDiffAdvDone:
	case bndDiffAdvDone:
		break;
	default:
		badState("Invalid diffuseAdvect state ");
	}

	if (this->isInside( )) {
		//DaCache::Map & diffuseAdvectMap = ourCache.diffuseAdvectMap; 
		std::copy(amtMass.begin( ), amtMass.end( ), amtMassTransient.begin( ));

		const size_t nVolumeVariables = numVolumeVariables( );
		//pre-compute all diffusion constants
		for (size_t s = 0; s < nVolumeVariables;s++) {
			const BioQuanType diffusionConstant = env.physiology->getVolumeVariable(s)->evaluateExpression(moving_boundary::expr_diffusion, sourceTermValues.data());
			diffusionValue[s] = diffusionConstant;
		}

		for (int i = 0 ; i < numNeighbors( ); i++) {
			NeighborType & nbData = neighbors[i];
			if (nbData.element == nullptr) {
				VCELL_EXCEPTION(logic_error,ident( ) << " neighbor " << i << " is null");
			}

			const moving_boundary::DistanceType edgeLength = neighbors[i].edgeLength; 
			if (edgeLength == 0) { //if no edge, don't bother
				continue;
			}
			OurType  & nbUpdateRef = *(nbData.element);
			const OurType & nb = nbUpdateRef;

			if (nb.state( ) == bndFrontMoved) {
				if (nb.vol.empty( )) {
					std::cout << nb.ident( ) << " empty vol " << std::endl;
				}
				nbUpdateRef.findNeighborEdges(); //change neighbor, use non-const pointer
			}
			const NeighborType * usToThem = nb.findUs(*this);
			if (usToThem == nullptr) {
				VCELL_EXCEPTION(logic_error, this->ident( ) << " neighbor " << nb.ident( ) << " has no record of 'this'"); 
			}
			const moving_boundary::DistanceType edgeLengthFrom = usToThem->edgeLength;
			if (edgeLengthFrom != edgeLength) {
				std::ostringstream oss;
				oss << this->ident( ) << " to " << nb.ident( ) << " length " << edgeLength
					<< " different from reverse length " << edgeLengthFrom; 
				throw  ReverseLengthException(oss.str( ),*this,nb); 
			}

			for (size_t s = 0; s < nVolumeVariables; ++ s)
			{
				if (physio->getVolumeVariable(s)->isAdvecting())
				{
					CoordVect av = (getAdvection(s) +  nb.getAdvection(s)) / 2;
					spatial::SVector<moving_boundary::VelocityType,2> averageVelocity(av[cX], av[cY]);
					spatial::NormVector<double,2> normalVector(spatial::Point2D(nb),spatial::Point2D(*this) );
					// this coefficient divided by problem domain to world scaling factor twice; one because velocity is scaled,
					// secondly because it's going to be multiplied by edgeLength (which is squared)
					BioQuanType averageVelComponent = dot(averageVelocity,normalVector) / distanceScaledSquared;
					nbData.halfAdvectionFlux[s] = averageVelComponent * edgeLength / 2;
					VCELL_LOG(info, std::setprecision(12) << this->ident( ) << " to " << nb.ident( )
						<< " avg vel coefficient " << averageVelComponent << " edgeLength " << edgeLength
						<< " dist sc sq " << distanceScaledSquared
						<< " half flux " << nbData.halfAdvectionFlux[s]);
				}
				else
				{
					nbData.halfAdvectionFlux[s] = 0;
				}
			}
		}
	}
}
				/*
				VCELL_COND_LOG(info, s == 0, std::setprecision(12) << this->ident( ) << " da " << transferAmount << " from " << nb.ident( ) 
					<< " u(i) = " << cUs << " u(j) " << cOther << " distance " << std::setw(10) << neighbors[i].distanceTo 
					<< " D = " << diffusionConstant << " u(avg) = " << averageConcentration << " advectCoeff " << advectCoeff 
					<< " advectTerm = " << advectTerm << " edge length " << std::setw(10) << edgeLength 
					<< " timeStep " << timeStep
					<< " m(i) " << amtMass[0] << " vol(i) = " << volumePD( ) << " m(j) " << nb.amtMass[0] << " vol(j) " << nb.volumePD( ) 
					); 
					*/

using moving_boundary::BioQuanType;
using moving_boundary::TimeType;
template <class SOLVER>
void MeshElementNode::diffuseAdvect(SOLVER &solver, unsigned int species) {
	if (this->isInside( )) {
		if (matches(1,9)) {
			int x = 4;
		}

		BioQuanType iCoefficient = 0; //this node's coefficent 
		CoordinateProductType ourVolume = volumePD( );

		for (int i = 0 ; i < numNeighbors( ); i++) {
			NeighborType & nbData = neighbors[i];
			OurType & nb = *(nbData.element);

			BioQuanType diffusionConstant = diffusionValue[species]; 
			BioQuanType edgeLength = nbData.edgeLength; 
			/*
			BioQuanType cUs = concValue[species]; 
			BioQuanType cOther = nb.concValue[species];
			*/
			BioQuanType Dsld = diffusionConstant * edgeLength / nbData.distanceTo;
			BioQuanType jCoefficient = (Dsld + nbData.halfAdvectionFlux[species]) / ourVolume;
			const BioQuanType iTerm = (Dsld - nbData.halfAdvectionFlux[species]) / ourVolume;
			solver.setCoefficent(*this,nb,jCoefficient, iTerm);
			iCoefficient += iTerm; 
			VCELL_COND_LOG(info, species == 0, std::setprecision(12) << this->ident( ) << " da with " <<  nb.ident( ) 
				<< " diff constant " << diffusionConstant  << " edgeLength " << edgeLength << " dist to " << nbData.distanceTo
				<< " Ds / d " << Dsld << " half flux " << nbData.halfAdvectionFlux[species]
				<< " jC " << jCoefficient << " our volume " << ourVolume << " iTerm " << iTerm << " iC " << iCoefficient);

		}
		solver.setSolvingFor(*this,iCoefficient, concentration(species));
	} //isInside
}


template void MeshElementNode::diffuseAdvect(moving_boundary::ExplicitSolver &,unsigned int species);


MatLabDebug MatLabDebug::instance;
moving_boundary::BioQuanType MeshElementNode::distanceScaled        = 1;
moving_boundary::BioQuanType MeshElementNode::distanceScaledSquared = 1;
MeshElementNode::BoundaryOffsetType MeshElementNode::unsetBoundaryOffset = std::numeric_limits<MeshElementNode::BoundaryOffsetType>::max( );
