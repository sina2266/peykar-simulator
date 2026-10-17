#ifdef NOT_USED
#include <iostream>
#include <cassert>

#include <vector>
#include <algorithm>
#include "Voronoi.h"
#include "Slope.h"
#include "algo.h"
#include "gtest/gtest.h"

class SlopeTest : public ::testing::Test {
protected:
	spatial::Point a;
	spatial::Point b;
	spatial::Slope s;
	SlopeTest( )
		:a(1,2),
		b(5,7),
		s(a,b){}
};

 TEST_F(SlopeTest, general) {
	ASSERT_EQ(s.slope( ),1.25);
 }

 TEST_F(SlopeTest, perpendicular) {
	using namespace spatial;
	Slope per = s.perpendicular( );
	ASSERT_EQ(per.slope( ),-0.8);
 }
 TEST_F(SlopeTest, midPoint) {
	 spatial::Point m = spatial::midPoint(a,b); 
	 ASSERT_EQ(m.x( ),3);
	 ASSERT_EQ(m.y( ),4.5);
 }
/*
	Point origin(50,100);
	Point upRight = pointThrough(origin,s,1000);
	std::cout << "(" << upRight.getLongX( ) << ',' << upRight.getLongY( ) << ')' << std::endl;
	Point p = pointThrough(origin,s,-1000);
	std::cout << "(" << p.getLongX( ) << ',' << p.getLongY( ) << ')' << std::endl;
	p = pointThrough(origin,per,1000);
	std::cout << "(" << p.getLongX( ) << ',' << p.getLongY( ) << ')' << std::endl;
	p = pointThrough(origin,per,-1000);
	std::cout << "(" << p.getLongX( ) << ',' << p.getLongY( ) << ')' << std::endl;
	*/


//		os << "<infinite>";
//	}
//	return os;
//}
//
//void showVertex(const voronoi_diagram<double>::vertex_type & vertex) {
//	std::cout << vertex << std::endl;
//}
///*
//void showVertex(const boost::polygon::voronoi_vertex<double> & vertex) {
//	std::cout << vertex << std::endl;
//}
//*/
//size_t pointOf(const voronoi_diagram<double>::cell_type * const pCell) {
//	assert(pCell != nullptr);
//	const voronoi_diagram<double>::cell_type & cell = *pCell; 
//	assert ( cell.source_category( ) == boost::polygon::SOURCE_CATEGORY_SINGLE_POINT);
//	assert ( cell.contains_point(  ));
//	return cell.source_index( ); 
//
//}
//
///**
//* show half edge if and only if  source index < twin index 
//*/
//void showEdge(const voronoi_diagram<double>::edge_type & edge) {
//	const size_t first = pointOf(edge.cell( )); 
//	const size_t second = pointOf(edge.twin( )->cell( ));
//	if (first < second) {
//		std::cout << edge.vertex0( ) << " to " <<   edge.vertex1() 
//			<< " between " << first 
//			<< " and " << second 
//			<< std::endl;
//	}
//}
//
//int main( ) {
//	std::vector<Point> points;
//	points.push_back(Point(0,0));
//	points.push_back(Point(0,1)); 
//	points.push_back(Point(1,1)); 
//	points.push_back(Point(1,0)); 
//
//	PointScaler s(1000);
//	for_each(points.begin( ),points.end( ),s);
//	voronoi_diagram<double> vd;
//	boost::polygon::construct_voronoi(points.begin( ),points.end( ),&vd);
//	for_each(vd.vertices( ).begin( ),vd.vertices( ).end( ),showVertex);
//	for_each(vd.edges( ).begin( ),vd.edges( ).end( ),showEdge);
//
//	std::cin.get( );
//}
//
//typedef voronoi_diagram<double>::edge_type EdgeType;
//typedef voronoi_diagram<double>::const_edge_iterator EdgeIterator;
//typedef boost::polygon::voronoi_vertex<double> Vertex;
#endif
