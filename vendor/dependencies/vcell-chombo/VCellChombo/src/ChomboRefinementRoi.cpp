#include <VCELL/ChomboRefinementRoi.h>
#include <SimpleSymbolTable.h>

 ChomboRefinementRoi::ChomboRefinementRoi(ChomboGeometry* geometry, int lvl, string& roiExp)
 : chomboGeometry(geometry), level(lvl)
{
	roi = new VCell::Expression(roiExp);
	constantValue = 0;
	try
	{
		double d = roi->evaluateConstant();
		constantValue = new double[1];
		constantValue[0] = d;
	} 
	catch (...)
	{
		roi->bindExpression(chomboGeometry->getGeometrySymbolTable());
	}
}

ChomboRefinementRoi::~ChomboRefinementRoi()
{
	delete constantValue;
}

