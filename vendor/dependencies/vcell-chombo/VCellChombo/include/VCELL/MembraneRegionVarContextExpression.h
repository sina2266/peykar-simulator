/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef MEMBRANEREGIONVARCONTEXTEXPRESSION_H
#define MEMBRANEREGIONVARCONTEXTEXPRESSION_H

#include <VCELL/VarContext.h>
class Membrane;
class MembraneRegion;
struct MembraneElement;
class MembraneRegionVariable;

class MembraneRegionVarContextExpression : public VarContext
{
public:
  MembraneRegionVarContextExpression(Membrane *membrane, MembraneRegionVariable* var);
	void resolveReferences(SimulationExpression *sim);

protected:
	bool isNullExpressionOK(ExpressionIndex expIndex);
};

#endif
