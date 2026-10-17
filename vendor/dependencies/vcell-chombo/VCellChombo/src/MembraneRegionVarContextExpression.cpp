/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/MembraneRegionVarContextExpression.h>
#include <VCELL/SimulationExpression.h>
#include <VCELL/Membrane.h>
#include <VCELL/MembraneRegionVariable.h>
#include <Expression.h>
#include <SimpleSymbolTable.h>

MembraneRegionVarContextExpression:: MembraneRegionVarContextExpression(Membrane *membrane, MembraneRegionVariable* var) 
: VarContext(membrane, var)
{
}

void MembraneRegionVarContextExpression::resolveReferences(SimulationExpression* sim) {
	VarContext::resolveReferences(sim);
	bindAll(sim);
}

bool MembraneRegionVarContextExpression::isNullExpressionOK(ExpressionIndex expIndex) {
	if (expIndex == INITIAL_VALUE_EXP || expIndex == REACT_RATE_EXP || expIndex == UNIFORM_RATE_EXP) {
		return false;
	}
	return true;
}
