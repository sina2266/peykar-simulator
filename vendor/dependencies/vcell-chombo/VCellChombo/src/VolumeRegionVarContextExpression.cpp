/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/VolumeRegionVarContextExpression.h>
#include <VCELL/SimulationExpression.h>
#include <VCELL/VolumeRegionVariable.h>
#include <VCELL/Feature.h>
#include <Expression.h>
#include <SimpleSymbolTable.h>

VolumeRegionVarContextExpression:: VolumeRegionVarContextExpression(Feature *feature, VolumeRegionVariable* var) 
: VarContext(feature, var)
{
}

void VolumeRegionVarContextExpression::resolveReferences(SimulationExpression* sim) {
	VarContext::resolveReferences(sim);
	bindAll(sim);
}

bool VolumeRegionVarContextExpression::isNullExpressionOK(ExpressionIndex expIndex) {
	if (expIndex == INITIAL_VALUE_EXP || expIndex == REACT_RATE_EXP || expIndex == UNIFORM_RATE_EXP) {
		return false;
	}
	return true;
}
