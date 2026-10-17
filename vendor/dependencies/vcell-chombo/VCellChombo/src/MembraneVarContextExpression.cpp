/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/MembraneVarContextExpression.h>
#include <VCELL/SimulationExpression.h>
#include <VCELL/ChomboGeometry.h>
#include <VCELL/Membrane.h>
#include <VCELL/MembraneVariable.h>
#include <Expression.h>
#include <VCELL/SimTool.h>
#include <SimpleSymbolTable.h>

MembraneVarContextExpression::MembraneVarContextExpression(Membrane *membrane, MembraneVariable* var)
: VarContext(membrane, var)
{
}

void MembraneVarContextExpression::resolveReferences(SimulationExpression* sim) {
	VarContext::resolveReferences(sim);
	bindAll(sim);
}

bool MembraneVarContextExpression::isNullExpressionOK(ExpressionIndex expIndex) {
	if (expIndex == INITIAL_VALUE_EXP || expIndex == REACT_RATE_EXP) {
		return false;
	}
	if (variable->isDiffusing()) {
		if (expIndex == DIFF_RATE_EXP) {
			return false;
		}
		int geodim = sim->getChomboGeometry()->getDimension();
		if ((geodim >= 1 && structure->getXmBoundaryType() != BOUNDARY_PERIODIC && (expIndex == BOUNDARY_XM_EXP || expIndex == BOUNDARY_XP_EXP)) 
			|| (geodim >= 2 && structure->getYmBoundaryType() != BOUNDARY_PERIODIC && (expIndex == BOUNDARY_YM_EXP || expIndex == BOUNDARY_YP_EXP))
			|| (geodim >= 3 && structure->getZmBoundaryType() != BOUNDARY_PERIODIC && (expIndex == BOUNDARY_ZM_EXP || expIndex == BOUNDARY_ZP_EXP))) {
			return false;
		}
	}
	return true;
}
