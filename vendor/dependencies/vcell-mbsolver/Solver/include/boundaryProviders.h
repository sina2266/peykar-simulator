#ifndef boundaryProviders_h
#define boundaryProviders_h
#include <VCellFront.h>
#include <MovingBoundaryTypes.h>
namespace tinyxml2 {
	class XMLElement;
}
namespace moving_boundary {
	spatial::FrontProvider<moving_boundary::CoordinateType> *circleFront(double originx, double originy, double radius, double step, double velocityx);
	spatial::FrontProvider<moving_boundary::CoordinateType> *frontFromXML(const tinyxml2::XMLElement &node); 
	spatial::FrontProvider<moving_boundary::CoordinateType> *restoreFrontProvider(std::istream &is);
	spatial::FrontProvider<moving_boundary::CoordinateType> *expandingCircle(double theta, const char * const expression);
}
#endif
