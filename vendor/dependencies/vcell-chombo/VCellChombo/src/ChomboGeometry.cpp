
#include <VCELL/ChomboGeometry.h>
#include <VCELL/SimTool.h>
#include <VCELL/VCellModel.h>
#include <VCELL/ChomboIF.h>
#include <Expression.h>
#include <SimpleSymbolTable.h>
#include <ComplementIF.H>

SymbolTable* ChomboGeometry::geometrySymbolTable = 0;

ChomboGeometry::ChomboGeometry()
{ 
}

ChomboGeometry::~ChomboGeometry()
{
  	for (int i = 0; i < (int)chomboIFList.size(); ++ i) {
  		delete chomboIFList[i];
  	}
  	chomboIFList.clear();
}

SymbolTable* ChomboGeometry::getGeometrySymbolTable() {
	if (geometrySymbolTable == 0) {
		string symbols[] = {"x", "y", "z"};
		geometrySymbolTable = new SimpleSymbolTable(symbols, 3);
	}
	return geometrySymbolTable;
}

void ChomboGeometry::addSubdomain(Feature* feature, int phaseIndex, VCell::Expression* ifExp, VCell::Expression* userExp)
{
	ifExp->bindExpression(getGeometrySymbolTable());
	userExp->bindExpression(getGeometrySymbolTable());
	chomboIFList.push_back(new ChomboIF(feature, phaseIndex, ifExp, userExp));
}

void ChomboGeometry::addSubdomain(Feature* feature, int phaseIndex, string& distanceMapFile)
{
	chomboIFList.push_back(new ChomboIF(feature, phaseIndex, distanceMapFile));
}

Feature* ChomboGeometry::getFeature(const RealVect& a_point, bool validate) const
{
	for (int i = 0; i < (int)chomboIFList.size(); i ++) {
		double d = chomboIFList[i]->value(a_point, validate);
		if (d < 0) { // in this subdomain
			return chomboIFList[i]->getFeature();
		}
	}
	return NULL;
}

