/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/VolumeRegionVarContextExpression.h>
#include <VCELL/VolumeRegion.h>
#include <VCELL/VolumeRegionVariable.h>
#include <VCELL/Feature.h>

VolumeRegionVarContextExpression:: VolumeRegionVarContextExpression(Feature *feature, VolumeRegionVariable* var) 
: VarContext(feature, var)
{
}

void VolumeRegionVarContextExpression::resolveReferences(SimulationExpression* sim) {
	VarContext::resolveReferences(sim);
	bindAll(sim);
}

double VolumeRegionVarContextExpression::getInitialValue(long regionIndex) {
	return evaluateVolumeRegionExpression(regionIndex, INITIAL_VALUE_EXP);
}

double VolumeRegionVarContextExpression::getReactionRate(long volumeIndex) const
{
	return evaluateExpression(volumeIndex, REACT_RATE_EXP);
}

double VolumeRegionVarContextExpression::getUniformRate(VolumeRegion *region) const
{
	return evaluateVolumeRegionExpression(region->getIndex(), UNIFORM_RATE_EXP);
}

double VolumeRegionVarContextExpression::getFlux(MembraneElement *element) const
{
	return evaluateJumpCondition(element);
}

bool VolumeRegionVarContextExpression::isNullExpressionOK(int expIndex) const {
	if (expIndex == INITIAL_VALUE_EXP || expIndex == REACT_RATE_EXP || expIndex == UNIFORM_RATE_EXP) {
		return false;
	}
	return true;
}
