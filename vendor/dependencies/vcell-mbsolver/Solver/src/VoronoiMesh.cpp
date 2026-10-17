#include <MPoint.h>
#include <World.h>
#include <VoronoiMesh.h>
#include <Voronoi.h>
#include <MeshElementNode.h>
#include <algo.h>
#include <InsideCache.h>

#include <MBridge/Scatter.h>
#include <MBridge/Figure.h>
#include <MBridge/MBPolygon.h>
#include <MBridge/FronTierAdapt.h>

using spatial::cX;
using spatial::cY;
using spatial::VoronoiResult;
using spatial::Voronoi2D;
using spatial::VoronoiType;
using spatial::cX; 
using spatial::cY; 
using moving_boundary::VoronoiMesh;
using moving_boundary::FrontType;

struct VoronoiMesh::VoronoiMeshImpl {
	typedef moving_boundary::World<moving_boundary::CoordinateType,2> WorldType; 
	typedef std::map<const Element *,int> Map; 
	Voronoi2D<moving_boundary::CoordinateType> vprocessor;
	Map locations;
	VMInsideCache & insideCache;

	VoronoiMeshImpl(WorldType &wt, VMInsideCache & ic)
		:vprocessor(wt.limits( )),
		locations( ),
		insideCache(ic) {}

	void setFront(const MBMesh & mesh, const FrontType &front) {
		assert(insideCache.noValuesSet( ));
		locations.clear( );
		int voronoiIndex = 0;
		vprocessor.clear( );
		for (MBMesh::iterator iter = mesh.begin( ); iter != mesh.end( ); ++iter) {
			Element & e= *iter;
			//if (e.boundaryOffset( ) < 2 &&  spatial::inside<FrontPointType>(front,e)) {}
			if (e.boundaryOffset( ) < 3 &&  insideCache.inside(e) ) {
				vprocessor.add(e(cX),e(cY));
				locations[&e] = voronoiIndex++;
			}
		}
	}

	bool correctIndex(int idx, const Element &e) const {
		if (vprocessor.cell(idx)(cX) != e(cX)) {
			return false;
		}
		if (vprocessor.cell(idx)(cY) != e(cY)) {
			return false; //do this way so breakpoint can be set
		}
		return true;
	}

	void getResult(VoronoiResult & voronoiResult, const MBMesh & mesh, const Element & element) const{
		Map::const_iterator iter = locations.find(&element);
		if (iter == locations.end( )) {
			VCELL_EXCEPTION(invalid_argument, "element " << element.indexInfo( ) << " " << element.mPos( ) 
				<< " not in VoronoiMesh map")
		}
		const int idx = iter->second; 
		if (!correctIndex(idx,element)) throw std::logic_error("bad indexing"); //change to assert after init testing
		vprocessor.getVertices(voronoiResult,idx);
		//std::transform(voronoiResult.vertices.begin( ), voronoiResult.vertices.end( ), voronoiResult.vertices.begin( ), scale(mesh) );
	}

#ifdef NOT_USED
	/**
	* Functor, scale grid coordinates to problem domain coordinates
	*/
	struct scale :public std::unary_function<GhostPoint<double,2>,GhostPoint<double,2>>{
		const MBMesh & mesh;
		scale(const MBMesh &m)
			:mesh(m) {}
		GhostPoint<double,2> operator( )(GhostPoint<double,2> &gin) {
			return mesh.gridToSpatial(gin);
		}

	};
#endif

	/**
	* dump matlab script to show all voronois
	* @param os output stream
	* @param mesh current mesh
	* @param pFront pointer to front (optional, may be null)
	*/
	void matlabPlot(std::ostream &os,const MBMesh & mesh, const FrontType *pFront) {
		matlabBridge::Scatter scatter('b',5,true);
		VoronoiResult vr;
		for (Map::const_iterator iter = locations.begin( ); iter != locations.end( ); ++iter) {
			const Element & e  = *(iter->first);
			std::ostringstream ss;
			ss << e.indexInfo( );
			os << matlabBridge::Text(e(cX),e(cY),ss.str( ).c_str( )) << std::endl;
			scatter.add(e(cX),e(cY));
			vr.vertices.clear( );
			getResult(vr,mesh,e);
			if (vr.type == VoronoiResult::multiOpen) {
				throw std::domain_error("multi open not supported");
			}
			matlabBridge::Polygon voro("-or"); 
			frontTierAdapt::copyVectorInto(voro,vr.vertices);
			os << voro;
		}
		os << scatter;
		if (pFront != nullptr) {
			matlabBridge::Polygon frontP("-.g"); 
			frontTierAdapt::copyVectorInto(frontP,*pFront);
			os << frontP;
		}
	}


	static VoronoiMeshImpl * create(VMInsideCache & insideCache) {
		if (moving_boundary::Universe<2>::get( ).locked( )) {
			typedef moving_boundary::World<moving_boundary::CoordinateType,2> WorldType; 
			WorldType & world = WorldType::get( );
			return new VoronoiMeshImpl(world, insideCache);
		}
		else {
			throw std::logic_error("VoronoiMesh created with unlocked world");
		}
	}
};


VoronoiMesh::VoronoiMesh( )
	:mesh_(nullptr),
	insideCache(new VMInsideCache( )) ,
	impl(VoronoiMeshImpl::create(*insideCache))
{ }

VoronoiMesh::VoronoiMesh(MBMesh &m)
	:mesh_(&m),
	insideCache(new VMInsideCache( )) ,
	impl(VoronoiMeshImpl::create(*insideCache))
{ 
	insideCache->setIndexes(m.numCells(spatial::cX),m.numCells(spatial::cY));
}

VoronoiMesh::~VoronoiMesh( ) {
	delete insideCache;
	delete impl;
}

void VoronoiMesh::setMesh(MBMesh &m) {
	if (mesh_ == nullptr) {
		mesh_ = &m;
		insideCache->setIndexes(m.numCells(spatial::cX),m.numCells(spatial::cY));
	}
}

void VoronoiMesh::setFront(const FrontType &front) {
	insideCache->setFront(front);
	impl->setFront(mesh( ),front);
}

void VoronoiMesh::getResult(spatial::VoronoiResult & vr, const Element & e) const{
	impl->getResult(vr,mesh( ),e);
}
void VoronoiMesh::matlabPlot(std::ostream &os, const FrontType *pFront) {
	impl->matlabPlot(os,mesh( ), pFront);
}


/***
** implementation space for classifying Elements
*/
namespace {
	/**
	* how far to go looking for neighbor?
	*/
	const int MAX_VORONOI_STEPS = 5;

	struct Cardinal { 
		int x;
		int y;
		/**
		* perpendicular x
		*/
		int pX; 
		/**
		* perpendicular y
		*/
		int pY; 
		/**
		*  reverse perpendicular x
		*/
		int rpX; 
		/**
		* reverse perpendicular y
		*/
		int rpY; 
	};
	const Cardinal cardinals[] = {
		{ 0, 1,	 1, 0,  -1, 0}, //above
		{ 1, 0,  0,-1,   0, 1}, //right 
		{ 0,-1, -1, 0,   1, 0}, //below
		{-1, 0,  0, 1,   0,-1} //left
	};

	template<typename VRN_MSH>
	class Classifier { 
		typedef spatial::TPoint<int,2> IndexPoint;
		typedef typename VRN_MSH::MBMesh MESH;
		typedef const typename MESH::realType realType; 
		typedef typename MESH::elementType EType;
		typedef EType *BoundaryPointerType; 
		const VRN_MSH  & vmesh;

		/**
		* get element from mesh given point indexes and offset pair
		* @return element or nullptr
		*/
		EType * get(size_t x, size_t y, int *xypair) {
			std::array<size_t,2> lookup;
			lookup[cX] = x + xypair[0];
			lookup[cY] = y + xypair[1];
			return vmesh.mesh( ).query(lookup); 
		}
		/**
		* get element from mesh given point indexes and offset pair
		* @return element or nullptr
		*/
		EType * get(const IndexPoint &ip) { 
			std::array<size_t,2> lookup;
			lookup[cX] = ip(cX); 
			lookup[cY] = ip(cY); 
			return vmesh.mesh( ).query(lookup); 
		}
		enum {notSpecial = -1};

		const realType xDistance; 
		const realType yDistance; 
		bool neighborIsOutside;
		/**
		* work buffer for outside neighbor finding
		*/
		std::vector<BoundaryPointerType> workBuffer;
		/**
		* specify behavior for different classfication times
		*/
	public:
		Classifier(const VRN_MSH & vm)
			:vmesh(vm), 
			xDistance(vm.mesh( ).interval(cX)),
			yDistance(vm.mesh( ).interval(cY))
		{}


	private:
		/**
		* call once at beginning for all cells
		*/
		void setInsideValues(EType & in) {
			size_t index = 0;
			const size_t x = in.indexOf(cX);
			const size_t y = in.indexOf(cY);

			//immediately adjacent
			{
				int xNeighbors[][2] = { 
					{-1,0}, {1,0}, //left & right
				};
				int yNeighbors[][2] = { 
					{0,-1}, {0,+1} //above & below
				};
				for (int i = 0 ; i < sizeof(xNeighbors)/sizeof(xNeighbors[0]); i++) {
					EType * e = get(x, y, xNeighbors[i]);
					if (e != nullptr) { 
						in.setInsideNeighborValue(*e,index++, xDistance,yDistance);
					}
				}
				for (int i = 0 ; i < sizeof(yNeighbors)/sizeof(yNeighbors[0]); i++) {
					EType * e = get(x, y, yNeighbors[i]);
					if (e != nullptr) { 
						in.setInsideNeighborValue(*e,index++, yDistance,xDistance);
					}
				}
			}

			//corners
			{
				int cornerNeighbors[][2] = { 
					{-1,-1}, {-1,1}, {1,1}, {1,-1}
				};
				size_t index = 0;
				for (int i = 0 ; i < sizeof(cornerNeighbors)/sizeof(cornerNeighbors[0]); i++) {
					EType * e = get(x, y, cornerNeighbors[i]);
					if (e != nullptr) { 
						in.setCornerNeighborValue(*e,index++);
					}
				}
			}
		}

		bool checkNeighborPresent(EType & neighbor) {
			if (neighbor.isInside( )) {
				workBuffer.push_back(&neighbor);
				return true;
			}
			neighborIsOutside = true;
			return false;
		}

		/**
		* scan neighborhood looking for neighbors inside the boundary
		* go out in each cardinal direction until a neighbor found or MAX_VORONOI_STEPS traversed
		* additionally, scan perpendicularly from candidate cardinal point;
		*  go distance as looked for along perpendicular plus (at least) one, but don't go over more than 
		*  point off cardinal which has been already found
		*
		* sets member variable #neighborIsOutside
		*/
		void analyzeNeighbors(EType & in) {
			const int nCardinal = sizeof(cardinals)/sizeof(cardinals[0]);
			const int b = 1000 * MAX_VORONOI_STEPS;
			//cutout is used to record where a perpendicular from cardinal neighbor was found
			int cutout[nCardinal][2] = {b,b,b,b,b,b,b,b};
			const int perpendicularDirectionOne = 0;
			const int reversePerpendicularDirection = 1;

			const int x = static_cast<int>(in.indexOf(cX));
			const int y = static_cast<int>(in.indexOf(cY));
			const IndexPoint inPoint(static_cast<int>(x),static_cast<int>(y));
			neighborIsOutside = false;
			workBuffer.clear( );
			std::set<IndexPoint> added;
			for (int i = 0 ; i < nCardinal; i++) { 
				bool cardinalFoundOrEdgeHit = false;
				for (int d = 1; !cardinalFoundOrEdgeHit && d <=MAX_VORONOI_STEPS;d++) {
					IndexPoint ip(x + cardinals[i].x * d, y + cardinals[i].y * d );
					typename MESH::elementType * eCardinal = get(ip);
					if (eCardinal != nullptr) {
						if (checkNeighborPresent(*eCardinal)) {
							cardinalFoundOrEdgeHit = true;
						}
					} 
					else {
						if (d == 1) {
							typedef moving_boundary::World<moving_boundary::CoordinateType,2> WorldType;
							WorldType & world = WorldType::get( );
							spatial::TPoint<double,2> pd = world.toProblemDomain(in);
							VCELL_EXCEPTION(overflow_error, "ip " << ip << 
								" element " << in.ident() << " at " << in(spatial::cX) << ',' << in(spatial::cY)  <<
								"(" << pd << ") hit boundary" );
						}
						cardinalFoundOrEdgeHit = true;
						continue;
					}

					{   //scan points along perpendicular
						spatial::SVector<int,2> pVector(cardinals[i].pX, cardinals[i].pY);
						for (int p = 1; p<=d + 1 && p < cutout[i][perpendicularDirectionOne] + 1; p++) {
							spatial::SVector<int,2> v = pVector * p; 
							IndexPoint perpendicularTransitPoint =  spatial::displacement(ip,v);
							if (added.find(perpendicularTransitPoint) != added.end( )) {
								cutout[i][perpendicularDirectionOne] = p;
								break;
							}
							typename MESH::elementType * e = get(perpendicularTransitPoint);
							if (e == nullptr) { //hit end of analysis mesh
								cutout[i][perpendicularDirectionOne] = p;
								break;
							}
							if (checkNeighborPresent(*e)) {
								added.insert(perpendicularTransitPoint);
								cutout[i][perpendicularDirectionOne] = p;
								break;
							}
						}
					}

					{  //repeat, but go in opposite perpendicular direction
						spatial::SVector<int,2> pVector(cardinals[i].rpX, cardinals[i].rpY);
						for (int p = 1; p<=d + 1 && p < cutout[i][reversePerpendicularDirection] + 1; p++) {
							IndexPoint perpendicularTransitPoint =  spatial::displacement(ip,pVector * p);
							if (added.find(perpendicularTransitPoint) != added.end( )) {
								cutout[i][reversePerpendicularDirection] = p;
								break;
							}
							typename MESH::elementType * e = get(perpendicularTransitPoint);
							if (e == nullptr) { //hit end of analysis mesh
								cutout[i][reversePerpendicularDirection] = p;
								break;
							}
							if (checkNeighborPresent(*e)) {
								added.insert(perpendicularTransitPoint);
								cutout[i][reversePerpendicularDirection] = p;
								break;
							}
						}
					}
				}
			}
		}
	public:

		moving_boundary::Positions<typename MESH::elementType> initialClassification(const FrontType & polygon) {
			moving_boundary::Positions<EType> rval; 
			//first pass - inside / outside
			bool first = true;
			const MESH & mesh = vmesh.mesh( );
			for (typename MESH::iterator iter = mesh.begin( ); iter != mesh.end( ); ++iter) {
				EType & point = *iter;
				setInsideValues(point);

				using spatial::inside;
				spatial::SurfacePosition pos = spatial::inside<moving_boundary::CoordinatePoint>(polygon,point) ? spatial::interiorSurface : spatial::outsideSurface;
				point.setInitialPos(pos);
			}
			//second pass, find boundaries
			for (typename MESH::iterator iter = mesh.begin( ); iter != mesh.end( ); ++iter) {
				EType & point = *iter;
				if (point.mPos( ) == spatial::outsideSurface) {
					rval.outside.push_back(&point);
					continue;
				}
				analyzeNeighbors(point);

				if (!neighborIsOutside) {
					rval.inside.push_back(&point);
				}
				else {
					point.setInitialPos(spatial::boundarySurface);
					point.setBoundaryNeighbors(vmesh,workBuffer,polygon);
					assert(point.mPos( ) == spatial::boundarySurface);
					rval.boundary.push_back(&point);
				}
			}
			return rval;
		}

		/**
		* update classifications following movement of boundary
		* @tparam STL_C standard template library style container
		* @param polygon the front
		* @param container out param which receives set of boundary nodes 
		*/
		//template<class INPOINT, class STL_C>
		template<class STL_C>
		bool updateClassification(const VoronoiMesh::VMInsideCache & insideCache, STL_C &container) {
			using spatial::interiorSurface;
			using spatial::outsideSurface;
			using spatial::boundarySurface;
			const MESH & mesh = vmesh.mesh( );
			//first pass - inside / outside
			bool changed = false;
			for (typename MESH::iterator iter = mesh.begin( ); iter != mesh.end( ); ++iter) {
				EType & point = *iter;

				using spatial::inside;
				if (point.boundaryOffset( ) > 1) { //too far from boundary to change
					continue;
				}
				spatial::SurfacePosition oldPosition = point.mPos( ); 
				//spatial::SurfacePosition pos = inside<INPOINT>(polygon,point) ? interiorSurface : outsideSurface;
				spatial::SurfacePosition pos = insideCache.inside(point) ? interiorSurface : outsideSurface;
				if (pos == oldPosition) { 
					continue;
				}
				if (pos == outsideSurface || oldPosition == outsideSurface) { 
					point.setPos(pos);
					changed = true;
				}
				else {
					VCELL_LOG(trace,"pchange skip " << pos << ',' << oldPosition);
				}
			}
			if (!changed) {
				return false;
			}
			container.erase(container.begin( ),container.end( ) );

			//second pass, find boundaries
			for (typename MESH::iterator iter = mesh.begin( ); iter != mesh.end( ); ++iter) {
				EType & point = *iter;
				size_t i = point.indexOf(cX);
				size_t j = point.indexOf(cY);
				if (point.isOutside( )) {
					continue;
				}
				analyzeNeighbors(point); 
				if (neighborIsOutside) {
					if (point.mPos( ) != boundarySurface) {
						point.setPos(spatial::boundarySurface);
					}
					point.updateBoundaryNeighbors(vmesh,workBuffer);
					container.push_back(&point);
				}
				else {
					if (point.mPos( ) == boundarySurface) {
						point.setPos(interiorSurface);
					}
				}
			}
			return true;
		}
	};
}
//END of classification implementation space


moving_boundary::Positions<typename VoronoiMesh::Element> VoronoiMesh::classify2(const FrontType & polygon) {
	Classifier<VoronoiMesh> classifier(*this);
	return classifier.initialClassification(polygon);
}

/**
* @tparam STL_C standard template library style container
* adjust nodes using front in last #setFront call
* @param container out param which receives set of boundary nodes 
* @param front the front
*/
template<class STL_CONTAINER>
bool VoronoiMesh::adjustNodes(STL_CONTAINER & boundaryContainer) {
	Classifier<VoronoiMesh> classifier(*this);
	const bool changed = classifier.updateClassification(*insideCache,boundaryContainer);
	return changed ; 
}

/**
* template instantiation
*/
namespace moving_boundary {
	typedef std::vector<MeshElementNode *> BoundaryContainer; 
	template bool VoronoiMesh::adjustNodes(BoundaryContainer &);
}
