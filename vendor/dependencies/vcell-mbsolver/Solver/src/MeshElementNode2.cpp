#include <MeshElementNode.h>
#include <MovingBoundaryCollections.h>
#include <MBridge/FronTierAdapt.h>
#include <MBridge/Figure.h>
#include <MBridge/MatlabDebug.h>
using namespace moving_boundary::MeshElementStateful;
using moving_boundary::MeshElementNode;
using moving_boundary::FrontType;
using moving_boundary::Volume2DClass;
void MeshElementNode::setInitialPos(spatial::SurfacePosition pos) {
	if (state( ) != initial && state( ) != initialInside) {
		badState("setInitialPos"); 
	}
	switch (pos) {
	case spatial::interiorSurface:
		setState(initialInside);
		break;
	case spatial::boundarySurface:
		setState(initialBoundary);
		break;
	case spatial::outsideSurface:
		setState(outStable);
		break;
	}
}

bool MeshElementNode::isInside( ) const {
	switch (state( )) {
	case initialInside:
	case inStable:
	case inReacted:
	case inDiffAdvDone:
	case inDiffAdvDoneMU:
	case initialBoundary: 
	case bndFrontMoved:
	case bndNbrEdgesFound:
	case bndReacted:
	case bndDiffAdvDone:
	case bndDiffAdvDoneMU:
	case bndFrontApplied:
	case bndStable:
	case transInBnd:
	case transOutBndSetBnd:
	case transOutBndNbrSet:
	case transOutBndMassCollected: 
	case transOutBndSetIn:
		return true;
	case outStable:
	case transBndOut: 
		return false;
		break;
	}
	badState("isInside");
	return false;
}
spatial::SurfacePosition MeshElementNode::mPos( ) const {
	switch (state( )) {
	case initialInside:
	case inStable:
	case inReacted:
	case inDiffAdvDone:
	case inDiffAdvDoneMU:
	case transOutBndSetIn: //this has to return true to get second call to setPos
		return spatial::interiorSurface;
	case initialBoundary: 
	case bndFrontMoved:
	case bndStable:
	case bndNbrEdgesFound:
	case bndReacted:
	case bndDiffAdvDone:
	case bndDiffAdvDoneMU:
	case bndFrontApplied:
	case bndMassCollectedFrontApplied:
	case transInBnd:
	case transOutBndSetBnd:
	case transOutBndNbrSet:
	case transOutBndMassCollected: 
		return spatial::boundarySurface;
	case outStable:
	case transBndOut: 
		return spatial::outsideSurface;
	default:
		badState("mPos def");
	}
	return spatial::outsideSurface;
}

void MeshElementNode::setBoundaryOffsetValues( ) {
	if (!isBoundary( ) || bndOffset_old != unsetOffsetValue()) {
		VCELL_EXCEPTION(domain_error, ident( ) << "setBoundaryOffsetValue with value " << bndOffset_old );
	}
	bndOffset_old = 0;

	NodeSet tracker;
	NodeQueue queue;

	propagateBoundaryValue(tracker,queue);
}

/**
* @param nodeSet used to track which elements already queued for processing 
* @param queue used to track order in which to process nodes 
*/
void MeshElementNode::propagateBoundaryValue(NodeSet & nodeSet, NodeQueue & queue) {
	VCELL_LOG(verbose,  ident( ) << " propagate current " << static_cast<unsigned int>(bndOffset_old));
	assert(bndOffset_old != unsetOffsetValue());

	//first pass, set unset neighbors and propagate call to neighbors at same offset
	const bool emptyQueue = queue.empty( );
	size_t setIndex = 0;
	/* 
	* cap offset at maxOffset( ) to minimize unnecesary recursion / computation
	*/
	const BoundaryOffsetType neighborOffset = std::min<BoundaryOffsetType>(bndOffset_old + 1,maxOffset( ));

	for (int i = 0; i < interiorNeighbors.ArraySize; i++) {
		if (interiorNeighbors[i].element != nullptr) {
			OurType & nb = *interiorNeighbors[i].element;
			if (nb.bndOffset_old == unsetOffsetValue( ) || nb.bndOffset_old > neighborOffset) {
				if (!nb.isBoundary( )) {
					nb.bndOffset_old = neighborOffset;
				}
				else {
					nb.bndOffset_old = 0;
				}
				//place in queue of elements to propagate to if and only it's not there already
				if (nodeSet.find(&nb) == nodeSet.end( )) {
					queue.push(&nb);
					nodeSet.insert(&nb);
				}
			}
		}
	}
	/*
	for (int i = 0; i < cornerNeighbors.ArraySize; i++) {
		if (cornerNeighbors[i] != nullptr) {
			OurType & nb = *cornerNeighbors[i];
			if (nb.boundaryOffset( ) == unsetOffsetValue( ) || nb.boundaryOffset( ) > neighborOffset) {
				if (!nb.isBoundary( )) {
					nb.bndOffset = neighborOffset;
				}
				else {
					nb.bndOffset = 0;
				}
				nodesThatWereSet[setIndex++] = &nb;
			}
		}
	}
	*/

	//second pass, tell neighbors that have been set to set their neighbor if the queue was
	//empty when we started; this prevents excessive recursion and subsequent stack overflow
	if (emptyQueue) {
		while (!queue.empty( )) {
			MeshElementNode *node = queue.front( );
			queue.pop( );
			node->propagateBoundaryValue(nodeSet, queue);
		}
	}
}

void MeshElementNode::listBoundary(std::ostream & os) const {
	using spatial::cX;
	using spatial::cY;
	switch (mPos( )) {
	case spatial::outsideSurface:
		return;
	case spatial::interiorSurface:
	case spatial::boundarySurface:
		{
			const char comma = ',';
			const Volume2DClass & cVol = getControlVolume( );
			typedef Volume2DClass::PointVector PointVector; 
			PointVector pv = cVol.points( ).front( ); 
			for (PointVector::const_iterator iter = pv.begin( ); iter != pv.end( ); ++iter) {
				os << index[0] << comma << index[1] << comma << mPos( ) << comma
					<< iter->get(cX) << comma << iter->get(cY) << std::endl;
			}
		}
		break;
	default:
		assert(false); //unknown state

	}
}
void MeshElementNode::badState(const char * const function) const {
	std::cout <<  ident( ) << " in " << function << std::endl;
	VCELL_EXCEPTION(logic_error, ident( ) << " in " << function);
}

MeshElementNode::OurType *MeshElementNode::neighbor(spatial::ElementOffset<2> & eo) const {
	//nominally unsafe downcast
	const MeshType * m = static_cast<const MeshType *>(&env.mesh( ));
	return m->element(*this,eo);
}

namespace {
	struct NeighborBuilder {
		MeshElementNode & client;
		NeighborBuilder(MeshElementNode &c)
			:client(c) {}

		MeshElementNode::NeighborType generate(std::istream &in) const {
			return MeshElementNode::NeighborType(in,client);
		}
	};
}

/**
* see #persist
*/
MeshElementNode::MeshElementNode(const Environment &env_,std::istream &is)
	:base(is),
	env(env_),
	concValue( ),
	sourceTermValues(concValue),
	vol(0,this) //required to register this as VolumeMonitor of volume
{
	vcell_persist::Token::check<MeshElementNode>(is); 

	NeighborBuilder nb(*this);
	vcell_persist::readFunctor<NeighborBuilder> functor(is,nb);

	vcell_persist::binaryRead(is,stateVar);
	vcell_persist::binaryRead(is,interiorVolume);
	vol = Volume2DClass(is);
	vcell_persist::restore(is,amtMass);
	vcell_persist::restore(is,amtMassTransient);
	vcell_persist::restore(is,concValue);
	vcell_persist::restore(is,interiorNeighbors, vcell_persist::readFunctor<NeighborBuilder>(is,nb) );
	//gcw 10-20-2014 thinking vector of neighbors not used if element not boundary, so no point in storing it
	bool usingNeighborVector;
	vcell_persist::binaryRead(is, usingNeighborVector);
	if (usingNeighborVector) {
		vcell_persist::restore(is,boundaryNeighbors, vcell_persist::InsertFrom<NeighborBuilder>(nb) );
		neighbors = boundaryNeighbors.data( ); 
	}
	else {
		neighbors = interiorNeighbors.data( ); 
	}
	voronoiVolume = Volume2DClass(is);
	vcell_persist::binaryRead(is,nOutside);
	//velocity = spatial::SVector<moving_boundary::VelocityType,2>(is);
}
namespace {
	struct NeighborWriter {
		MeshElementNode & client;
		NeighborWriter(MeshElementNode &c)
			:client(c) {}
		void operator( )(std::ostream &os, const MeshElementNode::NeighborType  & nb) const {
			nb.persist(os,client);
		}
	};
}

/**
* save essential state information necessary to restore from a file later
*/
void MeshElementNode::persist(std::ostream &os) {
	base::persist(os);

	NeighborWriter nw(*this);
	vcell_persist::writeFunctor<NeighborWriter> functor(os,nw);

	vcell_persist::Token::insert<MeshElementNode>(os); 
	vcell_persist::binaryWrite(os,stateVar);
	vcell_persist::binaryWrite(os,interiorVolume);
	vol.persist(os);
	//vcell_persist::save(os,segments_); //segments can be recreated on the fly
	vcell_persist::save(os,amtMass);
	vcell_persist::save(os,amtMassTransient);
	vcell_persist::save(os,concValue);
	vcell_persist::save(os,interiorNeighbors, functor);
	//vector of neighbors not used if element not boundary, so no point in storing it
	const bool usingNeighborVector = (neighbors == boundaryNeighbors.data( )); 
	vcell_persist::binaryWrite(os, usingNeighborVector);
	if (usingNeighborVector) {
		vcell_persist::save(os,boundaryNeighbors, functor); 
	}
	voronoiVolume.persist(os);
	vcell_persist::binaryWrite(os,nOutside);
	//velocity.persist(os);
}

std::ostream & moving_boundary::operator<<(std::ostream &os ,moving_boundary::MeshElementStateful::State state) {
#define CASE(x) case x: os << #x; break;
	switch (state) {
		CASE(initial);
		CASE(initialInside);
		CASE(initialBoundary);
		CASE(inStable);
		CASE(inReacted);
		CASE(inDiffAdvDone);
		CASE(inDiffAdvDoneMU);
		CASE(bndFrontMoved);
		CASE(bndReacted);
		CASE(bndNbrEdgesFound);
		CASE(bndDiffAdvDone);
		CASE(bndDiffAdvDoneMU);
		CASE(bndFrontApplied);
		CASE(bndMassCollectedFrontApplied);
		CASE(bndStable);
		CASE(outStable);
		CASE(transBndOut); 
		CASE(transInBnd);
		CASE(transOutBndSetBnd);
		CASE(transOutBndNbrSet);
		CASE(transOutBndMassCollected); 
		CASE(transOutBndSetIn);
	default:
		os << "missing state " << static_cast<int>(state); //cast required to avoid recursive call
	}
#undef CASE
	return os;
}
#ifdef MESH_ELEMENT_NODE_STATE_TRACK
std::ofstream slog("states.txt");
//void MeshElementNode::setState(moving_boundary::MeshElementStateful::State s) {
void MeshElementNode::DEBUG_SET_STATE(moving_boundary::MeshElementStateful::State s, const char *file, int line) {
	std::string full(file);
	size_t idx = full.find_last_of("/\\"); //just get file name, not path
	const char comma = ',';

	//to trace individual element calls
	//slog << ident( ) << comma << s << comma << full.substr(idx + 1) << comma << line << std::endl;

	//just to find active calls
	slog << stateVar << comma << s << comma << full.substr(idx + 1) << comma << line << std::endl;
	stateVar = s;
	VCELL_LOG(trace, ident( ) << " new state" ); 
	assert(debugSetState( ));
}
#endif

void MeshElementNode::genDebugPlot(std::ostream & dest, const Volume2DClass &ourVolume, const Volume2DClass & intersection, const FrontType * front) {
	using spatial::Axis;
	matlabBridge::Scatter nbplot('b',2);
	frontTierAdapt::copyPointInto(nbplot,*this);
	std::stringstream ss;
	ss << indexOf(Axis::cX) << ',' << indexOf(Axis::cY);
	dest <<  nbplot << matlabBridge::Text( (Axis::cX),(Axis::cY),ss.str( ).c_str( ));
	{
		matlabBridge::Polygons vs("-g",2);
		frontTierAdapt::copyVectorsInto(vs,ourVolume.points( ));
		dest << vs;
	}

	{
		matlabBridge::Polygons vi("+-r",2);
		frontTierAdapt::copyVectorsInto(vi,intersection.points( ));
		dest << vi;
	}
	if (front != nullptr) {
		matlabBridge::Polygons ni("-m",2);
		frontTierAdapt::copyVectorInto(ni, *front);
		dest << ni;
	}
}
