#include <iostream>
#include <cassert>
#ifdef LATER

#include <vector>
#include <algorithm>
#include <fstream>
#include "Voronoi.h"
#include "SVector.h"
#include "algo.h"
using boost::polygon::voronoi_diagram;
using namespace spatial;

namespace {
	typedef int64_t LocalVoronoiType;
	typedef SVector<LocalVoronoiType,2> DVector;
	typedef double vdVertexType;
	typedef TPoint<LocalVoronoiType,2> LocalVoronoiPoint;
	typedef GhostPoint<LocalVoronoiType,2> LocalGhostPoint;



	const vdVertexType epilson = 0.001;
	inline LocalVoronoiType fixDriftFromInt( vdVertexType in) {
		double junk;
		if (std::modf(in,&junk) == 0) { //no fractional part
			return static_cast<LocalVoronoiType>(in);
		}
		if (in > 0) {
			return static_cast<LocalVoronoiType>(in + epilson);
		}
		return static_cast<LocalVoronoiType>(in - epilson);
	}
	struct ConvertingGhostPoint : public LocalGhostPoint {
		ConvertingGhostPoint(vdVertexType x, vdVertexType y, bool isGhost = false) 
			:LocalGhostPoint(fixDriftFromInt(x), fixDriftFromInt(y),isGhost) {}
	};
	typedef TPoint<LocalVoronoiType,2> VoronoiPoint;
	//
	namespace bp = boost::polygon;
	namespace bpd = boost::polygon::detail;
    typedef long double floatingType;

    struct FloatingConverter {
        template <typename T>
        floatingType operator( )(const T & x)  const {
			return static_cast<floatingType>(x);
        }
		template <int D>
		floatingType operator( )(const bpd::extended_int<D> & x)  const {
			return x.d( );
        }
		floatingType operator( )(const bpd::extended_int<5> & x)  const {
			return x.d( );
        }
		floatingType operator( )(const bpd::extended_int<128> & x)  const {
			return x.d( );
        }
		
    };

    struct compareFloatingType {
        enum Result {
            LESS = -1,
            EQUAL = 0,
            MORE = 1
        };

        Result operator()(floatingType a, floatingType b, unsigned int maxUlps) const {
            if (a > b) {
                return a - b <= maxUlps ? EQUAL : LESS;
            }
            return b - a  <= maxUlps ? EQUAL : MORE;
        }
    };

    struct voronoi_ctype_traits {
        typedef boost::int64_t int_type;
		typedef bpd::extended_int<4> int_x2_type;
        typedef bpd::extended_int<5> uint_x2_type;
		typedef bpd::extended_int<128> big_int_type;
        typedef floatingType fpt_type;
        typedef floatingType efpt_type;
        typedef compareFloatingType ulp_cmp_type;
        typedef FloatingConverter to_fpt_converter_type;
        typedef FloatingConverter to_efpt_converter_type;
    };

    struct vcell_vd_traits{
        typedef voronoi_ctype_traits::fpt_type coordinate_type;
        typedef bp::voronoi_cell<coordinate_type> cell_type;
        typedef bp::voronoi_vertex<coordinate_type> vertex_type;
        typedef bp::voronoi_edge<coordinate_type> edge_type;
        typedef struct {
        public:
            enum { ULPS = 128 };
            bool operator()(const vertex_type &v1, const vertex_type &v2) const {
                return (ulp_cmp(v1.x(), v2.x(), ULPS) == compareFloatingType::EQUAL &&
                    ulp_cmp(v1.y(), v2.y(), ULPS) == compareFloatingType::EQUAL);
            }
        private:
            compareFloatingType ulp_cmp;
        } vertex_equality_predicate_type;


    };
}

#if 0
namespace boost {
	namespace polygon {
	template<>
	struct geometry_concept<LocalVoronoiPoint> { typedef point_concept type; };

	template<>
	struct point_traits<LocalVoronoiPoint> {
		typedef double coordinate_type;
		static inline coordinate_type get(const LocalVoronoiPoint& point, orientation_2d orient) {
				return (orient == HORIZONTAL) ? point.get(spatial::cX) : point.get(spatial::cY);
			}
		};
	}
}
#endif

struct Voronoi2DImpl64  :public Voronoi2DImpl<LocalVoronoiType> {
	typedef Voronoi2DImpl64 OurType;
	public:



		Voronoi2DImpl64(Voronoi2D<LocalVoronoiType> &owner_, bool & dirtyFlag_, std::vector<VoronoiPoint> & points_)
		:vd( ),
		owner(owner_),
		dirtyFlag(dirtyFlag_),
		points(points_)
		{
		}


		/**
		* remove all points
		*/
		void clear( ) {
			vd.clear( );
		}

		size_t num_cells( ) const {
			makeClean( );
			return vd.num_cells( );
		}

		void makeClean( ) const {
			if (dirtyFlag) {
				OurType & self = const_cast<OurType &>(*this);
				self.calculate( );
				dirtyFlag = false;
			}
		}

	void getVertices(TVoronoiResult<int64_t> & result, size_t cellIndex ) const{
		assert(result.vertices.size( ) == 0);
		assert(cellIndex >= 0);
		assert(cellIndex < num_cells( ));
		makeClean( );
		size_t nPoints = owner.numberPoints( );
		switch (nPoints) {
		case 1:
			throw std::invalid_argument("must have more than one point for voronoi");
		case 2:
			getDoubleInfinite(result,cellIndex); 
			return;
		default:
			{
				result.type = VoronoiBase::closed;
				//boost::polygon::voronoi does not preserve original indexes, so search for correct cell
				boost::polygon::voronoi_diagram<double>::cell_container_type::const_iterator iter = vd.cells( ).begin( );
				for (;iter != vd.cells( ).end( ); ++iter) {
					if (iter->source_index( ) == cellIndex) {
						break;
					}
				}
				assert(iter != vd.cells( ).end( ));
				const boost::polygon::voronoi_cell<double> & cell = *iter;
				const EdgeType * const incident =  cell.incident_edge( );
				assert(incident != nullptr);
				const EdgeType * edge = incident;
				//want to start list of vertices with ghost point (null) vertex, if one exists
				do {
					if (edge->vertex0( ) == nullptr) {
						break; //infinite edge, start here
					}
					edge = edge->next( );
				}
				while (edge != incident); //closed, so start at arbitrary point

				const EdgeType * const start = edge;
				const EdgeType * last; 
				do {
					const Vertex * first = edge->vertex0( );
					const Vertex * second = edge->vertex1( );
					//std::cout << edge->is_primary( ) << ' ' << first << " , " << second << std::endl;
					VoronoiBase::Type t = extractPoint(result.vertices,*edge,first,second, false);
					switch (t) {
					case VoronoiBase::closed:
						break;
					case VoronoiBase::straightLine:
						if (result.vertices.size( ) == 3) {
							result.type = VoronoiBase::straightLine;
						}
						else { //if we have multiple straight lines that's "multiOpen"
							result.type = VoronoiBase::multiOpen; 
						}
						break;
					case VoronoiBase::singleOpen:
					case VoronoiBase::multiOpen:
						result.type = std::max(result.type,t);
						break;
					default:
						throw std::invalid_argument("bad VoronoiResult::Type in switch");
					}
					last = edge;
					edge = edge->next( );
				}
				while (edge != start); //only go around once
				//catch last point if open
				if (result.type == VoronoiBase::singleOpen) {
					const Vertex * first = last->vertex0( );
					const Vertex * second = last->vertex1( );
					extractPoint(result.vertices,*last,second,first,true);
				}
				//if we have multiple open we should have one or two sets of three points 
				assert(result.type != VoronoiBase::multiOpen || result.vertices.size( ) == 3 || result.vertices.size( ) == 6);;
			}
		}
	}


	typedef double ImplementationType;
private:
	//convenience typedefs
	typedef boost::polygon::voronoi_diagram<ImplementationType>::edge_type EdgeType;
	typedef boost::polygon::voronoi_diagram<ImplementationType>::const_edge_iterator EdgeIterator;
	typedef boost::polygon::voronoi_vertex<ImplementationType> Vertex;

	mutable boost::polygon::voronoi_diagram<ImplementationType> vd;
	//spatial::Voronoi2D<LocalVoronoiType> & owner;
	spatial::Voronoi2D<int64_t> & owner;
	bool &dirtyFlag;
	std::vector<VoronoiPoint> & points;

	void calculate( ) {
	assert(dirtyFlag);

	vd.clear( );
    bp::voronoi_builder<boost::int64_t, voronoi_ctype_traits> vb;
	//boost::polygon::construct_voronoi(points.begin( ),points.end( ),&vd);
	for (std::vector<VoronoiPoint>::iterator iter = points.begin( ); iter != points.end( );++iter) {
		vb.insert_point(iter->get(cX),iter->get(cY));
	}
	vb.construct(&vd);

	dirtyFlag = false;
	//CLEANUP
	const bool dumpDebugOutput = true;
	if (!dumpDebugOutput) {
		return;
	}

	{
		std::ofstream dump("vpoint.txt");
		for (std::vector<VoronoiPoint>::iterator iter = points.begin( ); iter != points.end( );++iter) {
			dump <<  iter->get(spatial::cX) << ',' << iter->get(spatial::cY) << std::endl;
		}
	}

	{
		typedef boost::polygon::voronoi_diagram<ImplementationType>::vertex_container_type VCT;
		std::ofstream dump("vdpoint.txt");
		for (VCT::const_iterator iter = vd.vertices( ).begin( ); iter != vd.vertices( ).end( ); ++iter) {
			dump << iter->x( ) << ',' << iter->y( ) << ',' << iter->is_degenerate( ) << std::endl;
		}
	}

	{
		typedef boost::polygon::voronoi_diagram<ImplementationType>::cell_container_type CCT;
		std::ofstream dump("cell.txt");
		for (CCT::const_iterator iter = vd.cells( ).begin( ); iter != vd.cells( ).end( ); ++iter) {
			dump << iter->source_index( )  << ',' << iter->contains_point( ) << ',' << iter->contains_segment( )
				<< ',' << iter->source_category( ) << std::endl;
		}
	}

	{
		typedef boost::polygon::voronoi_diagram<ImplementationType>::edge_container_type ECT;
		std::ofstream dump("edge.txt");
		for (ECT::const_iterator iter = vd.edges( ).begin( ); iter != vd.edges( ).end( ); ++iter) {
			const Vertex *v0 = iter->vertex0( );
			const Vertex *v1 = iter->vertex1( );
			if (v0 != nullptr && v1 != nullptr) {
				dump << v0->x( ) << ',' << v0->y( ) << ',' << v1->x( ) << ',' << v1->y( ) << std::endl;
			}
		}
	}
}

	/**
	* extract single point from edge at ; actual or ghosted
	* @param results output ; collection to store results in 
	* @param edge to extract vertex from
	* @param anchor pointto extract vertex from; if null, ghost point created
	* @param other vertex at other end of edge, used for ghost building 
	* @return type of point(s) inserted 
	*/
	VoronoiBase::Type extractPoint(std::vector<LocalGhostPoint> & results, const EdgeType & edge, const Vertex * const anchor, const Vertex * const otherVertex, bool reverse) const {
		assert(!dirtyFlag);
	//static LocalVoronoiType debugLimit = 100; 
	if (anchor != nullptr) {
		ConvertingGhostPoint t(anchor->x( ),anchor->y( ));
		results.push_back(t);
		return VoronoiBase::closed;
	}
	else  { //infinite point, make ghost point
		const size_t srcIdx = edge.cell( )->source_index( );
		const size_t otherIdx = edge.twin( )->cell( )->source_index( );
		const LocalVoronoiPoint & gridPoint = owner.cell(srcIdx); 
		const LocalVoronoiPoint & otherGridPoint = owner.cell(otherIdx);
		const DVector direction(gridPoint,otherGridPoint);
		//const VoronoiPoint mid = spatial::midPoint(gridPoint,otherGridPoint);
		DVector perpendicular = direction.perpendicular( ); 
		const VoronoiPoint center = midPoint(gridPoint,otherGridPoint);
		if (otherVertex != nullptr) { //single ghost, anchored at real point
			ConvertingGhostPoint vertexPoint(otherVertex->x( ),otherVertex->y( ));
			if (reverse) {
				perpendicular.reverse( );
			}
			LocalGhostPoint ghost = ghostTo(center,perpendicular);
			ghost.setGhost(true);
			results.push_back(ghost);
			return VoronoiBase::singleOpen; 
		}
		else { //double ghost
			LocalGhostPoint ghost1 = ghostTo(center,perpendicular);
			LocalGhostPoint ghost2 = ghostTo(center,perpendicular.reverse( ));
			ghost1.setGhost(true);
			ghost2.setGhost(true);
			results.push_back(ghost1);
			results.push_back(center);
			results.push_back(ghost2);
			return VoronoiBase::straightLine;
		}
	}
	}

	/**
	* return infinite edge, ghosted. numberCells must be 2 
	*/
	void getDoubleInfinite(TVoronoiResult<int64_t> & result, size_t cellIndex) const {
		result.type = VoronoiBase::straightLine;
		assert(result.vertices.size( ) == 0);
		assert(!dirtyFlag);
		assert (owner.numberPoints( ) == 2);
		assert(vd.num_edges( ) == 2);
		assert(vd.num_cells( ) > cellIndex);
		const boost::polygon::voronoi_cell<double> & cell = vd.cells( )[cellIndex];
		const EdgeType * const edge =  cell.incident_edge( );
		const size_t srcIdx = edge->cell( )->source_index( );
		const size_t otherIdx = edge->twin( )->cell( )->source_index( );
		const VoronoiPoint & cellPoint = owner.cell(srcIdx); 
		const VoronoiPoint & otherCellPoint = owner.cell(otherIdx); 
		const DVector direction(cellPoint,otherCellPoint);
		DVector perpendicular = direction.perpendicular( ); 
		LocalGhostPoint center = midPoint(cellPoint,otherCellPoint);
		LocalGhostPoint ghost1 = ghostTo(center,perpendicular);
		ghost1.setGhost(true);
		result.vertices.push_back(ghost1);
		result.vertices.push_back(center);

		perpendicular.reverse( );
		LocalGhostPoint ghost2 = ghostTo(center,perpendicular);
		ghost2.setGhost(true);
		result.vertices.push_back(ghost2);
	}

	/**
	* return ghost point going to edge of limits
	*/
	LocalGhostPoint ghostTo(const VoronoiPoint & origin, const spatial::SVector<LocalVoronoiType,2> & direction) const {
	NormVector<double,2> norm(direction.convert<double>( ));
	double length = toWall(origin,norm,cX); 
	double ylength = toWall(origin,norm,cY); 
	length = std::min(length,ylength);

	LocalVoronoiType x = origin(cX) + static_cast<LocalVoronoiType>(norm(cX) * length); 
	LocalVoronoiType y = origin(cY) + static_cast<LocalVoronoiType>(norm(cY) * length); 
	return LocalGhostPoint(x,y,true);
}
	/**
	* distance to wall
	* @param origin starting point 
	* @param norm direction  
	* @param a axis  
	*/
	double toWall(const VoronoiPoint & origin, const NormVector<double,2> norm, const Axis a) const {
		LocalVoronoiType wall = (norm(a) < 0) ? owner.limits( )[a].low( ) : owner.limits( )[a].high( );
		const double toWallDist = (wall - origin(a) ) / norm(a);
		return toWallDist;
	}
};

namespace spatial {
	struct Voronoi2DImpl<int64_t> * ImplFactory<int64_t>::createImplementation(Voronoi2D<int64_t> & owner, bool & dirty, std::vector<VoronoiPoint> & points) {
		return new Voronoi2DImpl64(owner,dirty,points);
	}
}

#if 0
template <class T>
std::ostream &operator<<(std::ostream & os, const boost::polygon::voronoi_vertex<T> &vertex) {
	os << '[' << vertex.x( ) <<  ',' << vertex.y( ) << ']'; 
	return os;
}
std::ostream &operator<<(std::ostream & os, const boost::polygon::voronoi_vertex<double> *vertex) {
	if (vertex != nullptr) {
		os << '[' << vertex->x( ) <<  ',' << vertex->y( ) << ']'; 
	}
	else {
		os << "infinite";
	}
	return os;
}
#endif
#endif
