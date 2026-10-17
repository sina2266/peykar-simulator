/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/VolumeRegionEqnBuilder.h>
#include <VCELL/VolumeRegionVariable.h>
#include <math.h>
#include <VCELL/Element.h>
#include <VCELL/ODESolver.h>
#include <VCELL/Feature.h>
#include <VCELL/VolumeRegionVarContextExpression.h>
#include <VCELL/VolumeRegion.h>
#include <VCELL/MembraneRegion.h>
#include <VCELL/CartesianMesh.h>

VolumeRegionEqnBuilder::VolumeRegionEqnBuilder(VolumeRegionVariable *Avar, CartesianMesh *Amesh, ODESolver *Asolver) : EqnBuilder(Avar,Amesh) {
	odeSolver = Asolver;
}

void VolumeRegionEqnBuilder::buildEquation(Simulation* sim, double deltaTime, int volumeIndexStart, int volumeIndexSize, int membraneIndexStart, int membraneIndexSize) {
	int size = ((CartesianMesh*)mesh)->getNumVolumeRegions();
	ASSERTION(size==var->getSize());
	double *pRate = odeSolver->getRates();
	for(int i=0; i<size; i++, pRate ++){
		*pRate = 0;    // constant Dirichlet (shortening) is implied
		VolumeRegion* volRegion = ((CartesianMesh*)mesh)->getVolumeRegion(i);
		Feature* feature = volRegion->getFeature();
		VolumeRegionVarContextExpression* volRegionVarContext = feature->getVolumeRegionVarContext((VolumeRegionVariable*)var);

		if (volRegionVarContext == 0) {
			continue;
		}
		*pRate = volRegionVarContext->getUniformRate(volRegion);

		double volume = volRegion->getSize();
		ASSERTION(volume>0);
		int numElements = volRegion->getNumElements();
		double volumeIntegral = 0.0;
		for(int j=0; j<numElements; j++){
			long index = volRegion->getElementIndex(j); 
			volumeIntegral += volRegionVarContext->getReactionRate(index) * mesh->getVolumeOfElement_cu(index);
		}
		*pRate += volumeIntegral/volume;

		int numMembraneRegions = volRegion->getNumMembraneRegions();
		double surfaceIntegral = 0.0;
		for(int k=0; k<numMembraneRegions; k++){
			MembraneRegion *membraneRegion = volRegion->getMembraneRegion(k);
			numElements = membraneRegion->getNumElements();
			for(int j=0; j<numElements; j++){
				MembraneElement *pElement = mesh->getMembraneElements() + membraneRegion->getElementIndex(j);
				double flux = volRegionVarContext->getFlux(pElement); // membrane rate
				surfaceIntegral += flux * pElement->area;
			}
		}
		*pRate += surfaceIntegral/volume; 
	}
}
