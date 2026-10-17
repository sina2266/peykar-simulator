#include "SVector.h"
#include "TPoint.h"

using spatial::SVector; 
using spatial::TPoint; 

void tcompile( ) {
	TPoint<double,2> a(1,2);
	TPoint<double,2> b(3,7);
	SVector<double,2> g(a,b);
	SVector<double,2> unit = g;
	unit.normalize( );
}
