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
	void resolveReferences(SimulationExpression *sim) override;

	double getInitialValue(long index) override;
    double getReactionRate(long volumeIndex) const;
    double getUniformRate(VolumeRegion *region) const;
    double getFlux(MembraneElement *element) const;

    VolumeRegionVarContextExpression(Feature *feature, VolumeRegionVariable* var);

protected:
	bool isNullExpressionOK(int expIndex) const override;
};

#endif
