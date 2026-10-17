/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/PDESolver.h>
#include <VCELL/Variable.h>
#include <VCELL/EqnBuilder.h>

PDESolver::PDESolver(Variable *Var, bool AbTimeDependent) : Solver(Var)
{
	bTimeDependent = AbTimeDependent;
}


void PDESolver::initEqn(VCellModel* model, double deltaTime, int volumeIndexStart, int volumeIndexSize, int membraneIndexStart, int membraneIndexSize, bool bFirstTime) {
	if (!bFirstTime && !isTimeDependent()) {
		return;
	}

	ASSERTION(eqnBuilder);

	eqnBuilder->initEquation(model, deltaTime, volumeIndexStart, volumeIndexSize, membraneIndexStart, membraneIndexSize);
}
