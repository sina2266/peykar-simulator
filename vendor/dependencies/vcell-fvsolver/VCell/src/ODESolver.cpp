/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/ODESolver.h>
#include <VCELL/Variable.h>
#include <VCELL/VolumeRegion.h>
#include <VCELL/CartesianMesh.h>

ODESolver::ODESolver(Variable *Var, Mesh *Amesh, int numSolveRegions, int *solveRegions) : Solver(Var) {
	size = Var->getSize();
	ASSERTION(size>0);
	rate = new double[size];
	mesh = Amesh;
	arraySize = 0;
	Gridmap = NULL;
	if (numSolveRegions > 0) {
		for(int i = 0; i<numSolveRegions; i++) {
			arraySize += (((CartesianMesh*)mesh)->getVolumeRegion(solveRegions[i]))->getNumElements();
		}
		Gridmap = new long[arraySize];
		long index = 0;
		for(int i = 0; i<numSolveRegions; i++) {
			VolumeRegion *regionToSolve = ((CartesianMesh*)mesh)->getVolumeRegion(solveRegions[i]); 
			long numInRegion = regionToSolve->getNumElements();
			for(long indexInRegion = 0; indexInRegion < numInRegion; indexInRegion++) {
				// find gridindex associated with index
				long gridindex = regionToSolve->getElementIndex(indexInRegion);
				Gridmap[index] = gridindex  ;         // maps gridpoint in new order into natural order 
				index++;
			}
		}
	}
}

ODESolver::~ODESolver()
{
	if (rate) delete[] rate;
}

void ODESolver::solveEqn(SimTool* sim_tool, double deltaTime, int volumeIndexStart, int volumeIndexSize, int membraneIndexStart, int membraneIndexSize, bool bFirstTime)
{
	if(arraySize==0){  
		ASSERTION((volumeIndexStart>=0) && 
					((volumeIndexStart+volumeIndexSize)<=mesh->getNumVolumeElements()));
		ASSERTION((membraneIndexStart>=0) && 
					((membraneIndexStart+membraneIndexSize)<=mesh->getNumMembraneElements()));
		int startIndex=0;
		int calcSize=size;
		if (size == mesh->getNumVolumeElements()){
			startIndex = volumeIndexStart;
			calcSize = volumeIndexSize;
		}else if (size == mesh->getNumMembraneElements()){
			startIndex = membraneIndexStart;
			calcSize = membraneIndexSize;
		}

		double *old = var->getOld() + startIndex;
		double *curr = var->getCurr() + startIndex;
		double *pRate = rate + startIndex;

		for (long i=startIndex;i<(startIndex+calcSize);i++){
			*curr++ = *old++ + (*pRate++)*deltaTime;
		}
	}else{
		for(long i=0; i<arraySize; i++){
			long index = Gridmap[i];
			*(var->getCurr()+index) = *(var->getOld()+index) 
									+ (*(rate + index))*deltaTime;
		}
	}
}
