/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/VolumeVarContextExpression.h>
#include <Expression.h>
#include <VCELL/Feature.h>
#include <VCELL/SimulationExpression.h>
#include <VCELL/SimTool.h>
#include <SimpleSymbolTable.h>
#include <VCELL/VolumeVariable.h>
#include <VCELL/ChomboGeometry.h>

VolumeVarContextExpression::VolumeVarContextExpression(Feature *feature, VolumeVariable* var)
: VarContext(feature, var)
{
}

void VolumeVarContextExpression::resolveReferences(SimulationExpression* sim) {
	VarContext::resolveReferences(sim);
	bindAll(sim);
}

bool VolumeVarContextExpression::isNullExpressionOK(ExpressionIndex expIndex) {
	if (expIndex == INITIAL_VALUE_EXP || expIndex == REACT_RATE_EXP) {
		return false;
	}

	if (variable->isDiffusing()) {
		if (expIndex == DIFF_RATE_EXP) {
			return false;
		}
		int geodim = sim->getChomboGeometry()->getDimension();
		if ((geodim >= 1 && (expIndex == BOUNDARY_XM_EXP || expIndex == BOUNDARY_XP_EXP || expIndex == VELOCITY_X_EXP)) 
			|| (geodim >= 2 && (expIndex == BOUNDARY_YM_EXP || expIndex == BOUNDARY_YP_EXP || expIndex == VELOCITY_Y_EXP))
			|| (geodim >= 3 && (expIndex == BOUNDARY_ZM_EXP || expIndex == BOUNDARY_ZP_EXP || expIndex == VELOCITY_Z_EXP))) {
			return false;
		}
	}
	return true;
}

bool VolumeVarContextExpression::hasConstantDiffusion() {
	if (!variable->isDiffusing()) {
		throw "hasConstantDiffusion() is only for PDE variables";
	}
	return isConstantExpression(DIFF_RATE_EXP);
}

bool VolumeVarContextExpression::hasConstantDiffusionAdvection(int dimension) {
	if (!hasConstantDiffusion()) {
		return false;
	}
	if (!((VolumeVariable*)variable)->isAdvecting()) {
		return true;
	}
	return isConstantExpression(VELOCITY_X_EXP)
			&& (dimension < 2 || isConstantExpression(VELOCITY_Y_EXP)) 
			&& (dimension < 3 || isConstantExpression(VELOCITY_Z_EXP));
}
