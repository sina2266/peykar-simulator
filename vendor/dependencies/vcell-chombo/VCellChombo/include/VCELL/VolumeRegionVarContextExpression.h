/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef VOLUMEREGIONVARCONTEXTEXPRESSION_H
#define VOLUMEREGIONVARCONTEXTEXPRESSION_H

#include <VCELL/VarContext.h>
class Feature;
class VolumeRegion;
class VolumeRegionVariable;
struct MembraneElement;

class VolumeRegionVarContextExpression : public VarContext
{
public:
	void resolveReferences(SimulationExpression *sim);

	double getInitialValue(long index);
    double getReactionRate(long volumeIndex);
    double getUniformRate(VolumeRegion *region);
    double getFlux(MembraneElement *element);

    VolumeRegionVarContextExpression(Feature *feature, VolumeRegionVariable* var);

protected:
	bool isNullExpressionOK(ExpressionIndex expIndex);
};

#endif
