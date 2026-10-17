#if 0
#include "Slope.h"
#include "Point.h"

using spatial::Slope; 
using spatial::Point; 

void Slope::construct(double deltaX,double deltaY) {
	if (deltaX != 0) {
		value = deltaY / deltaX;
		vertical = not;
		return;
	}
	if (deltaY == 0) {
		throw "Error constructing Slope, both delta X & deltaY are zero";
	}
	vertical = deltaY > 0 ? up : down; 
}

Slope::Slope(const Point & lhs, const Point & rhs) {
	construct(rhs.x( ) - lhs.x( ),rhs.y( ) - lhs.y( ));
}

Slope Slope::perpendicular( ) const {
	if (!vertical) {
		if (value != 0.0) {
			return Slope(-1.0 / value);
		}
		return Slope(true);
	}
	return Slope(0.0);
}
#endif



