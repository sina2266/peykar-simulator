#ifdef UNUSED
#include <iostream>
#include <cassert>

#include <vector>
#include <algorithm>
#include "Voronoi.h"
#include "SVector.h"
#include "algo.h"
#include "VCellException.h"

namespace {
	const int SCALE_MARGIN = 1000; 
}
using boost::polygon::voronoi_diagram;
using namespace spatial;
typedef SVector<double,2> DVector;

void spatial::Voronoi2I::getDoubleInfinite(VoronoiResult & result,size_t cellIndex ) const{
		using spatial::convert;
		result.type = VoronoiResult::straightLine;
		assert(result.vertices.size( ) == 0);
		assert(!dirty);
		assert (numberPoints( ) == 2);
		assert(vd.num_edges( ) == 2);
		assert(vd.num_cells( ) > cellIndex);
		const boost::polygon::voronoi_cell<double> & cell = vd.cells( )[cellIndex];
		const EdgeType * const edge =  cell.incident_edge( );
		const size_t srcIdx = edge->cell( )->source_index( );
		const size_t otherIdx = edge->twin( )->cell( )->source_index( );
		const Ghost2D & cellPoint = convert<VoronoiPoint,Ghost2D>( points[srcIdx]  );
		const Ghost2D & otherCellPoint = convert<VoronoiPoint,Ghost2D>( points[otherIdx]  );
		const DVector direction(cellPoint,otherCellPoint);
		DVector perpendicular = direction.perpendicular( ); 
		Ghost2D center = midPoint(cellPoint,otherCellPoint);
		Ghost2D ghost1 = pointThrough<double,2>(center,perpendicular,ghostDistance);
		ghost1.setGhost(true);
		result.vertices.push_back(ghost1);
		result.vertices.push_back(center);

		perpendicular.reverse( );
		Ghost2D ghost2 = pointThrough<double,2>(center,perpendicular,ghostDistance);
		ghost2.setGhost(true);
		result.vertices.push_back(ghost2);
}

#pragma warning ( disable : 4244 ) //converting doubles to long
VoronoiResult::Type spatial::Voronoi2I::extractPoint(std::vector<typename Voronoi2I::Ghost2D> &results,
			 const EdgeType & edge, const Vertex * const anchor, const Vertex * const otherVertex, bool reverse) const {
	assert(!dirty);
	if (anchor != nullptr) {
		Ghost2D t(anchor->x( ),anchor->y( ));
		results.push_back(t);
		return VoronoiResult::closed;
	}
	else  { //infinite point, make ghost point
		const size_t srcIdx = edge.cell( )->source_index( );
		const size_t otherIdx = edge.twin( )->cell( )->source_index( );
		const Ghost2D gridPoint = convert<VoronoiPoint,Ghost2D>( points[srcIdx] );
		const Ghost2D otherGridPoint = convert<VoronoiPoint,Ghost2D>( points[otherIdx] );
		const DVector direction(gridPoint,otherGridPoint);
		const Point2D mid = spatial::midPoint(gridPoint,otherGridPoint);
		DVector perpendicular = direction.perpendicular( ); 
		const Point2D center = midPoint(gridPoint,otherGridPoint);
		if (otherVertex != nullptr) { //single ghost, anchored at real point
			Ghost2D vertexPoint(otherVertex->x( ),otherVertex->y( ));
			if (reverse) {
				perpendicular.reverse( );
			}
			Ghost2D ghost = pointThrough<double,2>(center,perpendicular,ghostDistance);
			ghost.setGhost(true);
			results.push_back(ghost);
			return VoronoiResult::singleOpen; 
		}
		else { //double ghost
			Ghost2D ghost1 = pointThrough<double,2>(center,perpendicular,ghostDistance);
			Ghost2D ghost2 = pointThrough<double,2>(center,perpendicular,-ghostDistance);
			ghost1.setGhost(true);
			ghost2.setGhost(true);
			results.push_back(ghost1);
			results.push_back(center);
			results.push_back(ghost2);
			return VoronoiResult::straightLine;
		}
	}
}

void spatial::Voronoi2I::getVertices(VoronoiResult & result, size_t cellIndex ) const{
	assert(result.vertices.size( ) == 0);
	assert(cellIndex >= 0);
	assert(cellIndex < numberCells( )); 
	makeClean( );
	size_t nPoints = numberPoints( );
	switch (nPoints) {
	case 1:
		throw std::invalid_argument("must have more than one point for voronoi");
	case 2:
		getDoubleInfinite(result,cellIndex); 
		return;
	default:
		{
			result.type = VoronoiResult::closed;
			//boost::polygon::voronoi does not preserve original indexes, so search for correct cell
			boost::polygon::voronoi_diagram<double>::cell_container_type::const_iterator iter = vd.cells( ).begin( );
			for (;iter != vd.cells( ).end( ); ++iter) {
				if (iter->source_index( ) == cellIndex) {
					break;
				}
			}
			if (iter == vd.cells( ).end( )) {
				VCELL_EXCEPTION(logic_error, "can't find index " << cellIndex)
			}
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
				VoronoiResult::Type t = extractPoint(result.vertices,*edge,first,second, false);
				switch (t) {
					case VoronoiResult::closed:
						break;
					case VoronoiResult::straightLine:
						if (result.vertices.size( ) == 3) {
							result.type = VoronoiResult::straightLine;
						}
						else { //if we have multiple straight lines that's "multiOpen"
							result.type = VoronoiResult::multiOpen; 
						}
						break;
					case VoronoiResult::singleOpen:
					case VoronoiResult::multiOpen:
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
			if (result.type == VoronoiResult::singleOpen) {
				const Vertex * first = last->vertex0( );
				const Vertex * second = last->vertex1( );
				extractPoint(result.vertices,*last,second,first,true);
			}
			//if we have multiple open we should have one or two sets of three points 
			assert(result.type != VoronoiResult::multiOpen || result.vertices.size( ) == 3 || result.vertices.size( ) == 6);;
		}
	}
}
#pragma warning ( default : 4244 )

void  spatial::Voronoi2I::calculate( ) {
	assert(dirty);
	vd.clear( );
	boost::polygon::construct_voronoi(points.begin( ),points.end( ),&vd);
	dirty = false;
}
#endif
