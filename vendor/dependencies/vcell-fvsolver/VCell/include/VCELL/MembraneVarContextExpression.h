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

	void resolveReferences(SimulationExpression *sim) override;

    double  getInitialValue(MembraneElement *element);
    double  getMembraneReactionRate(MembraneElement *element);
    double  getMembraneDiffusionRate(MembraneElement *element);

    double getXmBoundaryValue(MembraneElement *element);
    double getXpBoundaryValue(MembraneElement *element);
    double getYmBoundaryValue(MembraneElement *element);
    double getYpBoundaryValue(MembraneElement *element);
    double getZmBoundaryValue(MembraneElement *element);
    double getZpBoundaryValue(MembraneElement *element);

    double getXmBoundaryFlux(MembraneElement *element);
    double getXpBoundaryFlux(MembraneElement *element);
    double getYmBoundaryFlux(MembraneElement *element);
    double getYpBoundaryFlux(MembraneElement *element);
    double getZmBoundaryFlux(MembraneElement *element);
    double getZpBoundaryFlux(MembraneElement *element);

protected:
	bool isNullExpressionOK(int expIndex) const override;
};

#endif
