/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */

#include <VCELL/MembraneEqnBuilderDiffusion.h>
#include <VCELL/VolumeNeighbor.h>
#include <VCELL/MembraneVariable.h>
#include <VCELL/Membrane.h>
#include <VCELL/Element.h>
#include <VCELL/MembraneVarContextExpression.h>
#include <VCELL/Simulation.h>
#include <VCELL/SimTool.h>
#include <VCELL/FVUtils.h>
#include <VCELL/CartesianMesh.h>
#include <VCELL/SparseMatrixPCG.h>
#include <VCELL/VCellModel.h>

#include <string.h>
#include <assert.h>

MembraneEqnBuilderDiffusion::MembraneEqnBuilderDiffusion(MembraneVariable *Aspecies, Mesh *Amesh)
: SparseMatrixEqnBuilder(Aspecies, Amesh) {
	CartesianMesh* carMesh = (CartesianMesh*)mesh;
	long size = mesh->getNumMembraneElements();
	SparseMatrixPCG* membraneElementCoupling = carMesh->getMembraneCoupling();
	A = new SparseMatrixPCG(membraneElementCoupling);
	B = new double[size];
	memset(B, 0, size * sizeof(double));

	bPreProcessed = false;
}

MembraneEqnBuilderDiffusion::~MembraneEqnBuilderDiffusion() {
	delete A;
	delete[] B;
	periodicPairs.clear();
}

//------------------------------------------------------------------
//
// Initializes Matrix only
//
//------------------------------------------------------------------
void MembraneEqnBuilderDiffusion::initEquation(VCellModel* model, double deltaTime, int volumeIndexStart, int volumeIndexSize,
				int membraneIndexStart, int membraneIndexSize)
{   	
	if (!bPreProcessed) {
		preProcess(model);
	}

	if (periodicPairs.size() > 0) {
	   initEquation_Periodic(model, deltaTime, volumeIndexStart, volumeIndexSize, membraneIndexStart, membraneIndexSize);
	   return;
	} 

	const double epsilon = 1.0E-8;

	VolumeElement *pVolumeElement = mesh->getVolumeElements();
	MembraneElement *pMembraneElement = mesh->getMembraneElements();
	
	SparseMatrixPCG* membraneElementCoupling = ((CartesianMesh*)mesh)->getMembraneCoupling();
	MembraneElement* membraneElement = pMembraneElement;
	for (long index = membraneIndexStart; index < membraneIndexStart + membraneIndexSize; index ++, membraneElement ++){		
		int32* columns;
		double* values;
		int numColumns = membraneElementCoupling->getColumns(index, columns, values);

		Membrane* membrane = membraneElement->getMembrane();
		// variable is not defined on this membrane, set diagnol=1, off diagonal=0
		if (var->getStructure() != NULL && membrane != var->getStructure()) {
			A->setDiag(index, 1.0);

			// clear off diagonal
			for (long j = 0; j < numColumns; j ++) {
				int32 neighborIndex = columns[j];
				A->setValue(index, neighborIndex, 0);
			}
			continue;
		}

		MembraneVarContextExpression *varContext = membrane->getMembraneVarContext((MembraneVariable*)var);
		int mask = mesh->getMembraneNeighborMask(membraneElement);
		if (mask & NEIGHBOR_BOUNDARY_MASK && mask & BOUNDARY_TYPE_DIRICHLET){   // boundary and dirichlet
			A->setDiag(index, 1.0);

			// clear off diagonal
			for (long j = 0; j < numColumns; j ++) {
				int32 neighborIndex = columns[j];
				A->setValue(index, neighborIndex, 0);
			}
		} else {			
			double Di = varContext->getMembraneDiffusionRate(membraneElement);
			double volume = membraneElementCoupling->getValue(index, index);
			double Aii = volume/deltaTime;

			for (long j = 0; j < numColumns; j ++) {
				int32 neighborIndex = columns[j];
				double Dj = varContext->getMembraneDiffusionRate(pMembraneElement + neighborIndex);
				double D = (Di + Dj < epsilon)?(0.0):(2 * Di * Dj/(Di + Dj));
				double Aij = D * values[j];
				A->setValue(index, neighborIndex, -Aij);
				Aii += Aij;
			}
		
			A->setDiag(index, Aii);
		}
	} // end index
}

//------------------------------------------------------------------
//
// updates B vector only with reaction rates and boundary conditions
//
//------------------------------------------------------------------
void MembraneEqnBuilderDiffusion::buildEquation(Simulation* sim, double deltaTime, int volumeIndexStart, int volumeIndexSize,
				int membraneIndexStart, int membraneIndexSize)
{    
	if (periodicPairs.size() > 0) {
	   buildEquation_Periodic(sim, deltaTime, volumeIndexStart, volumeIndexSize,
											   membraneIndexStart, membraneIndexSize);
	   return;
	}

	VolumeElement *pVolumeElement = mesh->getVolumeElements();
	MembraneElement *pMembraneElement = mesh->getMembraneElements();

	SparseMatrixPCG* membraneElementCoupling = ((CartesianMesh*)mesh)->getMembraneCoupling();

	MembraneElement* membraneElement = pMembraneElement;
	for (long index = membraneIndexStart; index < membraneIndexStart + membraneIndexSize; index ++, membraneElement ++){
		Membrane* membrane = membraneElement->getMembrane();
		// variable is not defined on this membrane, set rhs=0
		if (var->getStructure() != NULL && membrane != var->getStructure()) {
			B[index] = 0;
			continue;
		}

		MembraneVarContextExpression *varContext = membrane->getMembraneVarContext((MembraneVariable*)var);
		double volume = membraneElementCoupling->getValue(index, index);
		double Ap0 = volume/deltaTime;

		int mask = mesh->getMembraneNeighborMask(membraneElement);
		if (mask & NEIGHBOR_BOUNDARY_MASK){ // boundary condition
			if (mask & BOUNDARY_TYPE_DIRICHLET){   // boundary and dirichlet
				if ((mask & NEIGHBOR_XM_BOUNDARY) && (membrane->getXmBoundaryType() == BOUNDARY_VALUE)){
					sim->advanceTimeOn();
					B[index] = varContext->getXmBoundaryValue(membraneElement);
					sim->advanceTimeOff();

				}else if ((mask & NEIGHBOR_XP_BOUNDARY) && (membrane->getXpBoundaryType() == BOUNDARY_VALUE)){

					sim->advanceTimeOn();
					B[index] = varContext->getXpBoundaryValue(membraneElement);
					sim->advanceTimeOff();

				}else if ((mask & NEIGHBOR_YM_BOUNDARY) && (membrane->getYmBoundaryType() == BOUNDARY_VALUE)){

					sim->advanceTimeOn();
					B[index] = varContext->getYmBoundaryValue(membraneElement);
					sim->advanceTimeOff();

				}else if ((mask & NEIGHBOR_YP_BOUNDARY) && (membrane->getYpBoundaryType() == BOUNDARY_VALUE)){

					sim->advanceTimeOn();
					B[index] = varContext->getYpBoundaryValue(membraneElement);
					sim->advanceTimeOff();

				}else if ((mask & NEIGHBOR_ZM_BOUNDARY) && (membrane->getZmBoundaryType() == BOUNDARY_VALUE)){

					sim->advanceTimeOn();
					B[index] = varContext->getZmBoundaryValue(membraneElement);
					sim->advanceTimeOff();

				}else if ((mask & NEIGHBOR_ZP_BOUNDARY) && (membrane->getZpBoundaryType() == BOUNDARY_VALUE)){

					sim->advanceTimeOn();
					B[index] = varContext->getZpBoundaryValue(membraneElement);
					sim->advanceTimeOff();

				}else{
					throw "MembraneEqnBuilderDiffusion::buildEquation() : invalid boundary type";
				}
			} else if (mask & BOUNDARY_TYPE_PERIODIC) {
				throw "periodic boundary conditon for membrane diffusion is not supported at the moment.";
			} else if (mask & BOUNDARY_TYPE_NEUMANN) { // boundary and neumann
				B[index] = Ap0 * var->getOld(index) + varContext->getMembraneReactionRate(membraneElement) * volume;

				if (mesh->getDimension() == 2) { // all the flux area is 1
					if (mask & NEIGHBOR_XM_BOUNDARY){
						sim->advanceTimeOn();
						B[index] += varContext->getXmBoundaryFlux(membraneElement);
						sim->advanceTimeOff();
					}
					if (mask & NEIGHBOR_XP_BOUNDARY){
						sim->advanceTimeOn();
						B[index] += - varContext->getXpBoundaryFlux(membraneElement);
						sim->advanceTimeOff();
					}
					if (mask & NEIGHBOR_YM_BOUNDARY){
						sim->advanceTimeOn();
						B[index] += varContext->getYmBoundaryFlux(membraneElement);
						sim->advanceTimeOff();
					}
					if (mask & NEIGHBOR_YP_BOUNDARY){
						sim->advanceTimeOn();
						B[index] += - varContext->getYpBoundaryFlux(membraneElement);
						sim->advanceTimeOff();
					}
				} else if (mesh->getDimension() == 3) {										
					double* boundaryFluxArea = mesh->getMembraneFluxArea(index);

					if (mask & NEIGHBOR_XM_BOUNDARY){
						sim->advanceTimeOn();
						B[index] += varContext->getXmBoundaryFlux(membraneElement)*boundaryFluxArea[BL_Xm];
						sim->advanceTimeOff();
					}
					if (mask & NEIGHBOR_XP_BOUNDARY){
						sim->advanceTimeOn();
						B[index] += - varContext->getXpBoundaryFlux(membraneElement)*boundaryFluxArea[BL_Xp];
						sim->advanceTimeOff();
					}
					if (mask & NEIGHBOR_YM_BOUNDARY){
						sim->advanceTimeOn();
						B[index] += varContext->getYmBoundaryFlux(membraneElement)*boundaryFluxArea[BL_Ym];
						sim->advanceTimeOff();
					}
					if (mask & NEIGHBOR_YP_BOUNDARY){
						sim->advanceTimeOn();
						B[index] += - varContext->getYpBoundaryFlux(membraneElement)*boundaryFluxArea[BL_Yp];
						sim->advanceTimeOff();
					}
					if (mask & NEIGHBOR_ZM_BOUNDARY){
						sim->advanceTimeOn();
						B[index] += varContext->getZmBoundaryFlux(membraneElement)*boundaryFluxArea[BL_Zm];
						sim->advanceTimeOff();
					}
					if (mask & NEIGHBOR_ZP_BOUNDARY){
						sim->advanceTimeOn();
						B[index] += - varContext->getZpBoundaryFlux(membraneElement)*boundaryFluxArea[BL_Zp];
						sim->advanceTimeOff();
					}					
				}
			}
		} else { // no boundary condition
			B[index] = Ap0 * var->getOld(index) + varContext->getMembraneReactionRate(membraneElement) * volume;
		}
	} // end index
}

double MembraneEqnBuilderDiffusion::computeDiffusionConstant(int meIndex, int neighborIndex) {
	const double epsilon = 1.0e-8;

	MembraneElement *pMembraneElement = mesh->getMembraneElements();
	Membrane* membrane = pMembraneElement[meIndex].getMembrane();
	MembraneVarContextExpression *varContext = membrane->getMembraneVarContext((MembraneVariable*)var);
	double Di = varContext->getMembraneDiffusionRate(pMembraneElement + meIndex);
	double Dj = varContext->getMembraneDiffusionRate(pMembraneElement + neighborIndex);
	return (Di + Dj < epsilon)?(0.0):(2 * Di * Dj/(Di + Dj));
}

void MembraneEqnBuilderDiffusion::initEquation_Periodic(VCellModel* model, double deltaTime, int volumeIndexStart, int volumeIndexSize,
														int membraneIndexStart, int membraneIndexSize) {
	if (!bPreProcessed) {
		preProcess(model);
	}

	VolumeElement *pVolumeElement = mesh->getVolumeElements();
	MembraneElement *pMembraneElement = mesh->getMembraneElements();
	ASSERTION(pVolumeElement);
	SparseMatrixPCG* membraneElementCoupling = ((CartesianMesh*)mesh)->getMembraneCoupling();

	int32* columns_A = 0;
	int numColumns_A = 0;
	double* values_A = 0;

	int32* columns_Coupling;
	double* values_Coupling;
	int numColumns_Coupling = 0;

	MembraneElement* membraneElement = pMembraneElement;
	for (long meIndex = membraneIndexStart; meIndex < membraneIndexStart + membraneIndexSize; meIndex ++){		
		int mask = mesh->getMembraneNeighborMask(meIndex);

		double Aii = 0;

		int bPeriodic = mask & BOUNDARY_TYPE_PERIODIC;
		if ((mask & NEIGHBOR_BOUNDARY_MASK) &&
			((mask & BOUNDARY_TYPE_DIRICHLET) // dirichlet
				|| bPeriodic && ((mask & NEIGHBOR_XP_BOUNDARY) || (mask & NEIGHBOR_YP_BOUNDARY)))) {  // periodic plus direction
			Aii = 1.0;					
		} else if (!bPeriodic) { // skip periodic points	
			numColumns_A = A->getColumns(meIndex, columns_A, values_A);

			numColumns_Coupling = membraneElementCoupling->getColumns(meIndex, columns_Coupling, values_Coupling);

			double volume = pMembraneElement[meIndex].area;
			Aii = volume/deltaTime;

			// natural neighbors
			for (long j = 0; j < numColumns_Coupling; j ++) {
				int32 neighborIndex = columns_Coupling[j];
				double D = computeDiffusionConstant(meIndex, neighborIndex);
				double Aij = D * values_Coupling[j];
				columns_A[j] = neighborIndex;
				values_A[j] = -Aij;
				Aii += Aij;
			}
		}
		A->setDiag(meIndex, Aii);
	} // end meIndex

	for (int i = 0; i < (int)periodicPairs.size(); i ++) {
		int meIndex = periodicPairs[i].first;

		numColumns_A = A->getColumns(meIndex, columns_A, values_A);
		numColumns_Coupling = membraneElementCoupling->getColumns(meIndex, columns_Coupling, values_Coupling);

		double volume = pMembraneElement[meIndex].area;
		double Aii = 0;

		// natural neighbors
		for (long j = 0; j < numColumns_Coupling; j ++) {
			int32 neighborIndex = columns_Coupling[j];
			double D = computeDiffusionConstant(meIndex, neighborIndex);
			double Aij = D * values_Coupling[j];
			columns_A[j] = neighborIndex;
			values_A[j] = -Aij;
			Aii += Aij;
		}

		int numOffDiag = numColumns_Coupling;

		int plusPeriodicIndex = periodicPairs[i].second;
		int32* columns_plusPeriodic;
		double* values_plusPeriodic;
		int numColumns_plusPeriodic = 0;

		numColumns_plusPeriodic = membraneElementCoupling->getColumns(plusPeriodicIndex, columns_plusPeriodic, values_plusPeriodic);

		// change volume
		volume += pMembraneElement[plusPeriodicIndex].area;

		for (long j = 0; j < numColumns_plusPeriodic; j ++) {
			int32 neighborIndex = columns_plusPeriodic[j];

			double D = computeDiffusionConstant(meIndex, neighborIndex);
			double Aij = D * values_Coupling[j];
			columns_A[numOffDiag] = neighborIndex;
			values_A[numOffDiag] = -Aij;
			Aii += Aij;

			numOffDiag ++;

			//get row int A for neighhborindex
			int32* columns_Ann;
			double* values_Ann;
			int numColumns_Ann = 0;

            numColumns_Ann = A->getColumns(neighborIndex, columns_Ann, values_Ann);
            for (long k = 0; k < numColumns_Ann; k ++) {
                if (columns_Ann[k] == plusPeriodicIndex) {
					columns_Ann[k] = meIndex;
					sortColumns(numColumns_Ann, columns_Ann, values_Ann);
					break;
				}
			}           
		}
		sortColumns(numOffDiag, columns_A, values_A);

		Aii += volume/deltaTime;
		A->setDiag(meIndex, Aii);
	} // end i
}

void MembraneEqnBuilderDiffusion::buildEquation_Periodic(Simulation* sim, double deltaTime, int volumeIndexStart, int volumeIndexSize,
														 int membraneIndexStart, int membraneIndexSize) {  
	VolumeElement *pVolumeElement = mesh->getVolumeElements();
	MembraneElement *pMembraneElement = mesh->getMembraneElements();
	ASSERTION(pMembraneElement);	

	SparseMatrixPCG* membraneElementCoupling = ((CartesianMesh*)mesh)->getMembraneCoupling();

	MembraneElement* membraneElement = pMembraneElement;
	for (long index = membraneIndexStart; index < membraneIndexStart + membraneIndexSize; index ++, membraneElement ++){
		ASSERTION(membraneElement->membrane);
		Membrane* membrane = membraneElement->getMembrane();
		MembraneVarContextExpression *varContext = membrane->getMembraneVarContext((MembraneVariable*)var);	

		double volume = membraneElementCoupling->getValue(index, index);
		int mask = mesh->getMembraneNeighborMask(membraneElement);
		if (mask & NEIGHBOR_BOUNDARY_MASK){ // boundary condition
			if (mask & BOUNDARY_TYPE_DIRICHLET){   // boundary and dirichlet
				if ((mask & NEIGHBOR_XM_BOUNDARY) && (membrane->getXmBoundaryType() == BOUNDARY_VALUE)){
					sim->advanceTimeOn();
					B[index] = varContext->getXmBoundaryValue(membraneElement);
					sim->advanceTimeOff();

				} else if ((mask & NEIGHBOR_XP_BOUNDARY) && (membrane->getXpBoundaryType() == BOUNDARY_VALUE)){

					sim->advanceTimeOn();
					B[index] = varContext->getXpBoundaryValue(membraneElement);
					sim->advanceTimeOff();

				} else if ((mask & NEIGHBOR_YM_BOUNDARY) && (membrane->getYmBoundaryType() == BOUNDARY_VALUE)){

					sim->advanceTimeOn();
					B[index] = varContext->getYmBoundaryValue(membraneElement);
					sim->advanceTimeOff();

				} else if ((mask & NEIGHBOR_YP_BOUNDARY) && (membrane->getYpBoundaryType() == BOUNDARY_VALUE)){

					sim->advanceTimeOn();
					B[index] = varContext->getYpBoundaryValue(membraneElement);
					sim->advanceTimeOff();

				} else if ((mask & NEIGHBOR_ZM_BOUNDARY) && (membrane->getZmBoundaryType() == BOUNDARY_VALUE)){

					sim->advanceTimeOn();
					B[index] = varContext->getZmBoundaryValue(membraneElement);
					sim->advanceTimeOff();

				} else if ((mask & NEIGHBOR_ZP_BOUNDARY) && (membrane->getZpBoundaryType() == BOUNDARY_VALUE)){

					sim->advanceTimeOn();
					B[index] = varContext->getZpBoundaryValue(membraneElement);
					sim->advanceTimeOff();

				} else{
					throw "MembraneEqnBuilderDiffusion::buildEquation() : invalid boundary type";
				}
			} else if (((mask & NEIGHBOR_XP_BOUNDARY) && membrane->getXpBoundaryType() == BOUNDARY_PERIODIC)  // periodic and plus direction
					|| ((mask & NEIGHBOR_YP_BOUNDARY) && membrane->getYpBoundaryType() == BOUNDARY_PERIODIC)
					|| ((mask & NEIGHBOR_ZP_BOUNDARY) && membrane->getZpBoundaryType() == BOUNDARY_PERIODIC)) {
				B[index] = 0;
			} else if (mask & BOUNDARY_TYPE_NEUMANN) { // boundary and neumann				
				B[index] = volume/deltaTime * var->getOld(index) + varContext->getMembraneReactionRate(membraneElement) * volume;

				if (mesh->getDimension() == 2) { // all the flux area is 1
					if (mask & NEIGHBOR_XM_BOUNDARY){
						sim->advanceTimeOn();
						B[index] += varContext->getXmBoundaryFlux(membraneElement);
						sim->advanceTimeOff();
					}
					if (mask & NEIGHBOR_XP_BOUNDARY){
						sim->advanceTimeOn();
						B[index] += - varContext->getXpBoundaryFlux(membraneElement);
						sim->advanceTimeOff();
					}
					if (mask & NEIGHBOR_YM_BOUNDARY){
						sim->advanceTimeOn();
						B[index] += varContext->getYmBoundaryFlux(membraneElement);
						sim->advanceTimeOff();
					}
					if (mask & NEIGHBOR_YP_BOUNDARY){
						sim->advanceTimeOn();
						B[index] += - varContext->getYpBoundaryFlux(membraneElement);
						sim->advanceTimeOff();
					}
				} else if (mesh->getDimension() == 3) {										
					double* boundaryFluxArea = mesh->getMembraneFluxArea(index);

					if (mask & NEIGHBOR_XM_BOUNDARY){
						sim->advanceTimeOn();
						B[index] += varContext->getXmBoundaryFlux(membraneElement)*boundaryFluxArea[BL_Xm];
						sim->advanceTimeOff();
					}
					if (mask & NEIGHBOR_XP_BOUNDARY){
						sim->advanceTimeOn();
						B[index] += - varContext->getXpBoundaryFlux(membraneElement)*boundaryFluxArea[BL_Xp];
						sim->advanceTimeOff();
					}
					if (mask & NEIGHBOR_YM_BOUNDARY){
						sim->advanceTimeOn();
						B[index] += varContext->getYmBoundaryFlux(membraneElement)*boundaryFluxArea[BL_Ym];
						sim->advanceTimeOff();
					}
					if (mask & NEIGHBOR_YP_BOUNDARY){
						sim->advanceTimeOn();
						B[index] += - varContext->getYpBoundaryFlux(membraneElement)*boundaryFluxArea[BL_Yp];
						sim->advanceTimeOff();
					}
					if (mask & NEIGHBOR_ZM_BOUNDARY){
						sim->advanceTimeOn();
						B[index] += varContext->getZmBoundaryFlux(membraneElement)*boundaryFluxArea[BL_Zm];
						sim->advanceTimeOff();
					}
					if (mask & NEIGHBOR_ZP_BOUNDARY){
						sim->advanceTimeOn();
						B[index] += - varContext->getZpBoundaryFlux(membraneElement)*boundaryFluxArea[BL_Zp];
						sim->advanceTimeOff();
					}					
				}
			}
		} else { // no boundary condition
			double volume = membraneElementCoupling->getValue(index, index);
			B[index] = volume/deltaTime * var->getOld(index) + varContext->getMembraneReactionRate(membraneElement) * volume;
		}
	} // end index

	for (int i = 0; i < (int)periodicPairs.size(); i ++) {
		int meIndex = periodicPairs[i].first;		

		int plusPeriodicIndex = periodicPairs[i].second;
		double volume = pMembraneElement[meIndex].area + pMembraneElement[plusPeriodicIndex].area;

		MembraneVarContextExpression *varContext = pMembraneElement[meIndex].getMembrane()->getMembraneVarContext((MembraneVariable*)var);

		double reactionRate = varContext->getMembraneReactionRate(pMembraneElement + meIndex);
		double plusReactinoRate = varContext->getMembraneReactionRate(pMembraneElement + plusPeriodicIndex);

		if (fabs(reactionRate - plusReactinoRate) > 1e-3 * fabs(reactionRate)) {
			throw "non periodic reaction rate at periodic membrane.";
		}

		B[meIndex] = volume/deltaTime * var->getOld(meIndex) + reactionRate * volume;		
	} // end i
}

#define PERIODIC_GEOMETRY_ERROR_MESSAGE "Geometry is not compatible with periodic boundary conditions. Couldn't find corresponding membrane element."
#define PERIODIC_NORMAL_ERROR_MESSAGE "Non periodic surface at periodic membrane (normals don't agree)."; 
void MembraneEqnBuilderDiffusion::preProcess(VCellModel* model) {
	if (bPreProcessed) {
		return;
	}

	bPreProcessed = true;
	CartesianMesh* carMesh = (CartesianMesh*)mesh;

	// check if there is periodic boundary condition in the model
	bool bHasPeriodicBC = false;
	for (int i = 0; i < model->getNumMembranes(); i ++) {
		Membrane *membrane = model->getMembraneFromIndex(i);
		if (membrane->getXmBoundaryType() == BOUNDARY_PERIODIC
			|| membrane->getYmBoundaryType() == BOUNDARY_PERIODIC
			|| membrane->getZmBoundaryType() == BOUNDARY_PERIODIC) {
			bHasPeriodicBC = true;
			break;
		}
	}

	if (bHasPeriodicBC) {
		if (mesh->getDimension() > 2) {
			throw "periodic boundary conditon for membrane diffusion is not supported for 3d spatial simulations at the moment.";
		}			
		if (carMesh->getNumVolumeX() <= 3 || carMesh->getNumVolumeY() <= 3) {
			throw "mesh is too coarse, please refine mesh.";
		}

		long size = mesh->getNumMembraneElements();
		SparseMatrixPCG* membraneElementCoupling = carMesh->getMembraneCoupling();

		//
		// preallocate memory without column indexes and values 
		//
		A->clear();

		double normalTestEps = 1e-2;

		// intialize periodic minus and plus pair, which is used to update plus points at each time step
		MembraneElement* pMembraneElement = mesh->getMembraneElements();
		VolumeElement* pVolumeElement = mesh->getVolumeElements();

		for (int mi = 0; mi < mesh->getNumMembraneElements(); mi ++) {
			int mask = mesh->getMembraneNeighborMask(mi);

			int32* columns_Coupling;
			double* values_Coupling;
			int numColumns_Coupling = 0;		

			int numColumns_A = 0;
			int bPeriodic = mask & BOUNDARY_TYPE_PERIODIC;
			if ((mask & NEIGHBOR_BOUNDARY_MASK) && 
				((mask & BOUNDARY_TYPE_DIRICHLET) // dirichlet
				|| bPeriodic && ((mask & NEIGHBOR_XP_BOUNDARY) || (mask & NEIGHBOR_YP_BOUNDARY)))) {  // periodic plus direction
				numColumns_A = 0;		
			} else {
				numColumns_Coupling = membraneElementCoupling->getColumns(mi, columns_Coupling, values_Coupling);

				// natural neighbors
				numColumns_A += numColumns_Coupling;

				if (bPeriodic) { 
					int plusPeriodicIndex = -1;
					if ((mask & NEIGHBOR_XM_BOUNDARY)) {
						int loVolIndex = pMembraneElement[mi].vindexFeatureLo;
						int theOtherSide = loVolIndex + carMesh->getNumVolumeX() - 1;
						vector<long>& mems = pVolumeElement[theOtherSide].adjacentMembraneIndexes;
						for (int k = 0; k < (int)mems.size(); k ++) {
							int anotherMask = mesh->getMembraneNeighborMask(mems[k]);
							if ((anotherMask & NEIGHBOR_XP_BOUNDARY) && (anotherMask & BOUNDARY_TYPE_PERIODIC)) {
								plusPeriodicIndex = mems[k];
								if (pMembraneElement[mi].getMembrane() != pMembraneElement[plusPeriodicIndex].getMembrane()) {
									throw PERIODIC_GEOMETRY_ERROR_MESSAGE;
								}
								if ((pMembraneElement[mi].unitNormal - pMembraneElement[plusPeriodicIndex].unitNormal).length() > normalTestEps) {
									throw PERIODIC_NORMAL_ERROR_MESSAGE;
								}
								periodicPairs.push_back(pair<int, int>(mi, plusPeriodicIndex));
								break;
							}
						}
					} else if ((mask & NEIGHBOR_YM_BOUNDARY)) {
						int loVolIndex = pMembraneElement[mi].vindexFeatureLo;
						MeshCoord meshCoord = carMesh->getMeshCoord(loVolIndex);
						int theOtherSide = (carMesh->getNumVolumeY() - 1) * carMesh->getNumVolumeX() + meshCoord.x;
						vector<long>& mems = pVolumeElement[theOtherSide].adjacentMembraneIndexes;
						for (int k = 0; k < (int)mems.size(); k ++) {
							int anotherMask = mesh->getMembraneNeighborMask(mems[k]);
							if ((anotherMask & NEIGHBOR_YP_BOUNDARY) && (anotherMask & BOUNDARY_TYPE_PERIODIC)) {
								plusPeriodicIndex = mems[k];
								if (pMembraneElement[mi].getMembrane() != pMembraneElement[plusPeriodicIndex].getMembrane()) {
									throw PERIODIC_GEOMETRY_ERROR_MESSAGE;
								}
								if ((pMembraneElement[mi].unitNormal - pMembraneElement[plusPeriodicIndex].unitNormal).length() > normalTestEps) {
									throw PERIODIC_NORMAL_ERROR_MESSAGE; 
								} 
								periodicPairs.push_back(pair<int, int>(mi, plusPeriodicIndex));
								break;
							}
						}
					}
					assert(plusPeriodicIndex != -1);

					int32* columns_plusPeriodic;
					double* values_plusPeriodic;
					int numColumns_plusPeriodic = 0;
					numColumns_plusPeriodic = membraneElementCoupling->getColumns(plusPeriodicIndex, 
						columns_plusPeriodic, values_plusPeriodic);
					// periodic neighbors
					// for 3D case, check if neighborIndex is on the plus wall, 
					// and if it is, check if the corresponding point on the minus 
					// wall has been added to the list for index
					// for 2D case, this is ok
					numColumns_A += numColumns_plusPeriodic;
				}
			}
			assert(numColumns_A <= 2);

			A->addNewRow();				
			A->setRow(1.0, numColumns_A, 0, 0);
		}
		A->close();
	}
}

void MembraneEqnBuilderDiffusion::postProcess() {
	double* currSol = var->getCurr();

	// update plus periodic points
	for (int i = 0; i < (int)periodicPairs.size(); i ++){
		pair<int, int>& ppair = periodicPairs[i];
		currSol[ppair.second] = currSol[ppair.first];
	}
}
