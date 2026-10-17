/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef MEMBRANEVARCONTEXTEXPRESSION_H
#define MEMBRANEVARCONTEXTEXPRESSION_H

#include <VCELL/VarContext.h>
class Membrane;
class MembraneVariable;
struct MembraneElement;

class MembraneVarContextExpression : public VarContext
{
public:
	MembraneVarContextExpression(Membrane *membrane, MembraneVariable* var);

	void resolveReferences(SimulationExpression *sim);

	double getInitialValue(long index)
	{
		throw "MembraneVarContextExpression::getInitialValue(long index) should never be called";
	}

protected:
	bool isNullExpressionOK(ExpressionIndex expIndex);
};

#endif
