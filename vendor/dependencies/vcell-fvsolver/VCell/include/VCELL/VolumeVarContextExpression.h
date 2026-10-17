/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef VOLUMEVARCONTEXTEXPRESSION_H
#define VOLUMEVARCONTEXTEXPRESSION_H

#include <VCELL/VarContext.h>

class Feature;
class VolumeVariable;

class VolumeVarContextExpression : public VarContext
{
public:	
	VolumeVarContextExpression(Feature *feature, VolumeVariable* var);
	
	void resolveReferences(SimulationExpression *sim) override;

	double getInitialValue(long index) override;
	double getDiffusionRate(long index) const;
	double  getReactionRate(long volumeIndex) const;
    double  getFlux(MembraneElement *element) const;

    double getXmBoundaryValue(long index) const;
    double getXpBoundaryValue(long index) const;
    double getYmBoundaryValue(long index) const;
    double getYpBoundaryValue(long index) const;
    double getZmBoundaryValue(long index) const;
    double getZpBoundaryValue(long index) const;

    double getXmBoundaryFlux(long index) const;
    double getXpBoundaryFlux(long index) const;
    double getYmBoundaryFlux(long index) const;
    double getYpBoundaryFlux(long index) const;
    double getZmBoundaryFlux(long index) const;
    double getZpBoundaryFlux(long index) const;    
    
	double getXBoundaryPeriodicConstant() const;
    double getYBoundaryPeriodicConstant() const;
    double getZBoundaryPeriodicConstant() const;

    double getConvectionVelocity_X(long index) const;
    double getConvectionVelocity_Y(long index) const;
    double getConvectionVelocity_Z(long index) const;

	bool hasConstantDiffusion() const;
	bool hasConstantCoefficients(int dimension) const;
	bool hasXYZOnlyDiffusion() const;

	bool hasGradient(int dir) const;

private:
	bool isNullExpressionOK(int expIndex) const override;
};

#endif
