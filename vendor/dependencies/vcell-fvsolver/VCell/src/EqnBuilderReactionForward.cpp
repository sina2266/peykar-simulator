/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/Simulation.h>
#include <VCELL/ODESolver.h>
#include <VCELL/VolumeVariable.h>
#include <VCELL/Mesh.h>
#include <VCELL/Feature.h>
#include <VCELL/VolumeVarContextExpression.h>
#include <VCELL/EqnBuilderReactionForward.h>
#include <VCELL/SimTool.h>
#include <VCELL/Element.h>

EqnBuilderReactionForward::EqnBuilderReactionForward(VolumeVariable *Avar, Mesh *Amesh, ODESolver *Asolver)
: EqnBuilder(Avar,Amesh)
{
	odeSolver = Asolver;
}

void EqnBuilderReactionForward::buildEquation(Simulation* sim, double deltaTime,
                            int volumeIndexStart, int volumeIndexSize, 
			    int membraneIndexStart, int membraneIndexSize)
{
	Feature *feature;
	VolumeVarContextExpression *varContext;

	long arraySize = odeSolver->getArraySize();
	if(arraySize==0){
		ASSERTION((volumeIndexStart>=0) && ((volumeIndexStart+volumeIndexSize)<=mesh->getNumVolumeElements()));

		double *pRate = odeSolver->getRates() + volumeIndexStart;

		VolumeElement *pVolumeElement = mesh->getVolumeElements() + volumeIndexStart;
		ASSERTION(pVolumeElement);

		for (long volIndex=volumeIndexStart;volIndex<(volumeIndexStart+volumeIndexSize);volIndex++){

			feature = pVolumeElement->getFeature();
			ASSERTION(feature);

			varContext = feature->getVolumeVarContext((VolumeVariable*)var);

		// sim->advanceTimeOn();
			*pRate = varContext->getReactionRate(volIndex);
		// sim->advanceTimeOff();

			pVolumeElement++;
			pRate++;
		} // end volIndex
	}else if(arraySize>0){
		ASSERTION(arraySize<=mesh->getNumVolumeElements());
		for(long i=0; i<arraySize; i++){
			long index = odeSolver->getGlobalIndex(i);
			double *pRate = odeSolver->getRates() + index;
			VolumeElement *pVolumeElement = mesh->getVolumeElements() + index;
			ASSERTION(pVolumeElement);
			varContext = (pVolumeElement->getFeature())->getVolumeVarContext((VolumeVariable*)var);

			// sim->advanceTimeOn();
			*pRate = varContext->getReactionRate(index);
			// sim->advanceTimeOff();

		}
	}
}

