/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/Simulation.h>
#include <VCELL/ODESolver.h>
#include <VCELL/MembraneVariable.h>
#include <VCELL/Mesh.h>
#include <VCELL/Membrane.h>
#include <VCELL/MembraneVarContextExpression.h>
#include <VCELL/MembraneEqnBuilderForward.h>
#include <VCELL/SimTool.h>
#include <VCELL/Element.h>

MembraneEqnBuilderForward::MembraneEqnBuilderForward(MembraneVariable *Avar, Mesh *Amesh, ODESolver *Asolver) : EqnBuilder(Avar,Amesh) {
	odeSolver = Asolver;
}

void MembraneEqnBuilderForward::buildEquation(Simulation* sim, double deltaTime, int volumeIndexStart, int volumeIndexSize, int membraneIndexStart, int membraneIndexSize)
{
	ASSERTION((membraneIndexStart>=0) && ((membraneIndexStart+membraneIndexSize)<=mesh->getNumMembraneElements()));

	MembraneElement *pMembraneElement = mesh->getMembraneElements() + membraneIndexStart;
	double *pRate = odeSolver->getRates() + membraneIndexStart;
	ASSERTION(pMembraneElement);

	for (long memIndex=membraneIndexStart;memIndex<(membraneIndexStart+membraneIndexSize);memIndex++, pMembraneElement++, pRate++){

		Membrane* membrane = pMembraneElement->getMembrane();
		if (var->getStructure() != NULL && var->getStructure() != membrane) {
			continue;
		}

		MembraneVarContextExpression* memVarContext = membrane->getMembraneVarContext((MembraneVariable*)var);

		sim->advanceTimeOn();
		*pRate = memVarContext->getMembraneReactionRate(pMembraneElement);
		sim->advanceTimeOff();
	} // end memIndex
}
