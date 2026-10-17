/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/CartesianMesh.h>
#include <VCELL/VolumeVarContextExpression.h>
#include <VCELL/Simulation.h>
#include <VCELL/SimulationExpression.h>
#include <VCELL/Solver.h>
#include <VCELL/VolumeVariable.h>
#include <VCELL/Feature.h>

VolumeVarContextExpression::VolumeVarContextExpression(Feature *feature, VolumeVariable* var)
: VarContext(feature, var)
{
}

void VolumeVarContextExpression::resolveReferences(SimulationExpression* sim) {
	VarContext::resolveReferences(sim);
	bindAll(sim);
}

double VolumeVarContextExpression::getInitialValue(long volIndex) {
	return evaluateExpression(volIndex, INITIAL_VALUE_EXP);
}

double VolumeVarContextExpression::getDiffusionRate(long volIndex) const
{
	return evaluateExpression(volIndex, DIFF_RATE_EXP);
}

double VolumeVarContextExpression::getReactionRate(long volIndex) const
{
	return evaluateExpression(volIndex, REACT_RATE_EXP);
}

double VolumeVarContextExpression::getXmBoundaryValue(long volIndex) const
{
	return evaluateExpression(volIndex, BOUNDARY_XM_EXP);
}

double VolumeVarContextExpression::getXpBoundaryValue(long volIndex) const
{
	return evaluateExpression(volIndex, BOUNDARY_XP_EXP);
}

double VolumeVarContextExpression::getYmBoundaryValue(long volIndex) const
{
	return evaluateExpression(volIndex, BOUNDARY_YM_EXP);
}

double VolumeVarContextExpression::getYpBoundaryValue(long volIndex) const
{
	return evaluateExpression(volIndex, BOUNDARY_YP_EXP);
}

double VolumeVarContextExpression::getZmBoundaryValue(long volIndex) const
{
	return evaluateExpression(volIndex, BOUNDARY_ZM_EXP);	
}

double VolumeVarContextExpression::getZpBoundaryValue(long volIndex) const
{
	return evaluateExpression(volIndex, BOUNDARY_ZP_EXP);
}

double VolumeVarContextExpression::getXmBoundaryFlux(long volIndex) const
{
	return evaluateExpression(volIndex, BOUNDARY_XM_EXP);
}

double VolumeVarContextExpression::getXpBoundaryFlux(long volIndex) const
{
	return evaluateExpression(volIndex, BOUNDARY_XP_EXP);
}

double VolumeVarContextExpression::getYmBoundaryFlux(long volIndex) const
{
	return evaluateExpression(volIndex, BOUNDARY_YM_EXP);
}

double VolumeVarContextExpression::getYpBoundaryFlux(long volIndex) const
{
	return evaluateExpression(volIndex, BOUNDARY_YP_EXP);
}

double VolumeVarContextExpression::getZmBoundaryFlux(long volIndex) const
{
	return evaluateExpression(volIndex, BOUNDARY_ZM_EXP);	
}

double VolumeVarContextExpression::getZpBoundaryFlux(long volIndex) const
{
	return evaluateExpression(volIndex, BOUNDARY_ZP_EXP);
}   
    
double VolumeVarContextExpression::getXBoundaryPeriodicConstant() const
{
	return evaluateConstantExpression(BOUNDARY_XP_EXP);
}

double VolumeVarContextExpression::getYBoundaryPeriodicConstant() const
{
	return evaluateConstantExpression(BOUNDARY_YP_EXP);
}

double VolumeVarContextExpression::getZBoundaryPeriodicConstant() const
{
	return evaluateConstantExpression(BOUNDARY_ZP_EXP);
}

double VolumeVarContextExpression::getConvectionVelocity_X(long volIndex) const
{
	return evaluateExpression(volIndex, VELOCITY_X_EXP);
}

double VolumeVarContextExpression::getConvectionVelocity_Y(long volIndex) const
{
	return evaluateExpression(volIndex, VELOCITY_Y_EXP);
}

double VolumeVarContextExpression::getConvectionVelocity_Z(long volIndex) const
{
	return evaluateExpression(volIndex, VELOCITY_Z_EXP);
}

double VolumeVarContextExpression::getFlux(MembraneElement *element) const
{
	//return evaluateExpression(element, FLUX_EXP);
	return evaluateJumpCondition(element);
}

bool VolumeVarContextExpression::isNullExpressionOK(int expIndex) const {
	if (expIndex == INITIAL_VALUE_EXP || expIndex == REACT_RATE_EXP) {
		return false;
	}

	Solver* solver = sim->getSolverFromVariable(species);
	if (solver != nullptr && solver->isPDESolver()) {
		if (expIndex == DIFF_RATE_EXP) {
			return false;
		}
		int geodim = sim->getMesh()->getDimension();
		if ((geodim >= 1 && (expIndex == BOUNDARY_XM_EXP || expIndex == BOUNDARY_XP_EXP || expIndex == VELOCITY_X_EXP)) 
			|| (geodim >= 2 && (expIndex == BOUNDARY_YM_EXP || expIndex == BOUNDARY_YP_EXP || expIndex == VELOCITY_Y_EXP))
			|| (geodim >= 3 && (expIndex == BOUNDARY_ZM_EXP || expIndex == BOUNDARY_ZP_EXP || expIndex == VELOCITY_Z_EXP))) {
			return false;
		}
	}
	return true;
}

bool VolumeVarContextExpression::hasConstantDiffusion() const
{
	if (!species->isDiffusing()) {
		throw std::runtime_error("hasConstantDiffusion() is only for PDE variables");
	}
	return isConstantExpression(DIFF_RATE_EXP);
}

bool VolumeVarContextExpression::hasConstantCoefficients(int dimension) const
{
	if (!hasConstantDiffusion() || ((VolumeVariable*)species)->hasGradient()) {
		return false;
	}
	if (!((VolumeVariable*)species)->isAdvecting()) {
		return true;
	}
	return isConstantExpression(VELOCITY_X_EXP)
			&& (dimension < 2 || isConstantExpression(VELOCITY_Y_EXP)) 
			&& (dimension < 3 || isConstantExpression(VELOCITY_Z_EXP));
}

bool VolumeVarContextExpression::hasXYZOnlyDiffusion() const
{
	if (!species->isDiffusing()) {
		throw std::runtime_error("hasConstantDiffusion() is only for PDE variables");
	}
	return isXYZOnlyExpression(DIFF_RATE_EXP);
}

bool VolumeVarContextExpression::hasGradient(int dir) const
{
	int expIndex = GRADIENT_X_EXP;
	if (dir == 1) {
		expIndex = GRADIENT_Y_EXP;
	} else if (dir == 2) {
		expIndex = GRADIENT_Z_EXP;
	}
	return !isNullExpression(expIndex);
}
