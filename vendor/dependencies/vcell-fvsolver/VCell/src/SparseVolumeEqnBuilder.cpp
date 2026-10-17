/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */

#include <sstream>
#include <algorithm>
using std::stringstream;
using std::max;

#include <VCELL/SparseVolumeEqnBuilder.h>
#include <VCELL/VolumeNeighbor.h>
#include <VCELL/VolumeVariable.h>
#include <VCELL/Feature.h>
#include <VCELL/Element.h>
#include <VCELL/VolumeVarContextExpression.h>
#include <VCELL/Simulation.h>
#include <VCELL/FVUtils.h>
#include <VCELL/SimTool.h>
#include <VCELL/CartesianMesh.h>
#include <VCELL/VolumeRegion.h>
#include <VCELL/VCellModel.h>
#include <VCELL/SparseMatrixPCG.h>

#include <assert.h>
#include <string.h>

static double epsilon = 1e-10;    // zero diffusion threshold at 1e-10 micron^2/second

static int GENERAL_MAX_NONZERO_PERROW[4] = {0, 3, 5, 7};
static int TRIANGULAR_MAX_NONZERO_PERROW[4] = {0, 2, 3, 4};
SparseVolumeEqnBuilder::SparseVolumeEqnBuilder(VolumeVariable *arg_species, CartesianMesh *arg_mesh, bool arg_bNoConvection, int arg_numSolveRegions, int* arg_solveRegions) : SparseMatrixEqnBuilder(arg_species, arg_mesh)
{
	bSymmetricStorage = arg_bNoConvection;
	numSolveRegions = arg_numSolveRegions;
	solveRegions = arg_solveRegions;

	DIM =  arg_mesh->getDimension();
	DELTAX = arg_mesh->getXScale_um();
	DELTAY = arg_mesh->getYScale_um();
	DELTAZ = arg_mesh->getZScale_um();
	AREAX  = arg_mesh->getXArea_squm();
	AREAY  = arg_mesh->getYArea_squm();
	AREAZ  = arg_mesh->getZArea_squm();
	VOLUME = arg_mesh->getVolume_cu();
	SIZEX = arg_mesh->getNumVolumeX();
	SIZEY = arg_mesh->getNumVolumeY();
	SIZEZ = arg_mesh->getNumVolumeZ();
	SIZEXY = SIZEX * SIZEY;

	bPreProcessed = false;

	init();
}

SparseVolumeEqnBuilder::~SparseVolumeEqnBuilder() {
	delete A;
	delete[] B;

	for (int i = 0; i < (int)dirichletNeighbors.size(); i ++) {
		delete dirichletNeighbors[i];
	}
	dirichletNeighbors.clear();
	for (int i = 0; i < (int)periodicNeighbors.size(); i ++) {
		delete periodicNeighbors[i];
	}
	periodicNeighbors.clear();
	for (int i = 0; i < (int)periodicCoupledPairs.size(); i ++) {
		delete periodicCoupledPairs[i];
	}
	periodicCoupledPairs.clear();
	if (numSolveRegions > 0) {
		delete[] LocalToGlobalMap;
		delete[] GlobalToLocalMap;
		delete[] RegionFirstRow;
		delete[] X;
	}
}

void SparseVolumeEqnBuilder::init() {
	int size;
	if (numSolveRegions == 0) {
		bSolveWholeMesh = true;
		GlobalToLocalMap = 0;
		LocalToGlobalMap = 0;
		RegionFirstRow = 0;
		X = var->getCurr();
		size = mesh->getNumVolumeElements();
	} else  {
		bSolveWholeMesh = false;
		try {
			RegionFirstRow = new int[numSolveRegions + 1];
			GlobalToLocalMap = new int[mesh->getNumVolumeElements()];
		} catch (...) {
			throw "Out of Memory";
		}
		for (int globalIndex = 0; globalIndex < mesh->getNumVolumeElements(); globalIndex ++) {
			GlobalToLocalMap[globalIndex] = -1;
		}

		RegionFirstRow[0] = 0;
		// initialize global to local map
		for (int i = 0; i < numSolveRegions; i ++) {
			int rID = solveRegions[i];
			VolumeRegion *regionToSolve = ((CartesianMesh*)mesh)->getVolumeRegion(rID);
			int numElements = regionToSolve->getNumElements();
			RegionFirstRow[i + 1] = RegionFirstRow[i] + numElements;
			for (int j = 0; j < numElements; j ++){
				int globalIndex = regionToSolve->getElementIndex(j);
				int localIndex = RegionFirstRow[i] + j;
				GlobalToLocalMap[globalIndex] = localIndex;
			}
		}

		size = RegionFirstRow[numSolveRegions];
		try {
			LocalToGlobalMap = new int[size];
			X = new double[size];
			memset(X, 0, size * sizeof(double));
		} catch (...) {
			throw "Out of Memory";
		}

		// initialize local to global map
		for (int globalIndex = 0; globalIndex < mesh->getNumVolumeElements(); globalIndex ++){
			int localIndex = GlobalToLocalMap[globalIndex];
			if (localIndex >= 0) {
				LocalToGlobalMap[localIndex] = globalIndex;
			}
		}
	}

	// initialize A and B
	int numNonZeros;
	switch (DIM) {
		case 1:
			if (bSymmetricStorage) {
				numNonZeros = TRIANGULAR_MAX_NONZERO_PERROW[DIM] * size;
			} else {
				numNonZeros = GENERAL_MAX_NONZERO_PERROW[DIM] * size;
			}
			break;
		case 2:
			if (bSymmetricStorage) { // symmetric half-storage
				numNonZeros = TRIANGULAR_MAX_NONZERO_PERROW[DIM] * size;
			}else {  // general storage
				numNonZeros = GENERAL_MAX_NONZERO_PERROW[DIM] * size;
			}
			// if we solve the whole mesh, we are able to reduce the number of nonzeros
			// since we have more information when it's rectangle.
			if (bSolveWholeMesh) {
				if (bSymmetricStorage) { // symmetric half-storage
					numNonZeros -= (SIZEX + SIZEY - 1);
				}else {  // general storage
					numNonZeros -= (SIZEX + SIZEY - 2);
				}
			}
			break;
		case 3:
			if (bSolveWholeMesh) {
				if (bSymmetricStorage) {  // symmetric half-storage
					numNonZeros = TRIANGULAR_MAX_NONZERO_PERROW[2] * SIZEX * SIZEY - SIZEX - SIZEY + 1; //2D
					numNonZeros += (SIZEZ - 1) * (numNonZeros + SIZEX * SIZEY);
				} else { // general storage
					numNonZeros = GENERAL_MAX_NONZERO_PERROW[2] * SIZEX * SIZEY - SIZEX - SIZEY + 2; //2D
					numNonZeros += (SIZEZ - 1) * (numNonZeros + 2 * SIZEX * SIZEY) + 1;
				}
			} else {
				if (bSymmetricStorage) {  // symmetric half-storage
					numNonZeros = TRIANGULAR_MAX_NONZERO_PERROW[DIM] * size;
				} else { // general storage
					numNonZeros = GENERAL_MAX_NONZERO_PERROW[DIM] * size;
				}
			}
	}
	if (bSymmetricStorage) {
		A = new SparseMatrixPCG(size, numNonZeros, MATRIX_SYMMETRIC); // only store upper triangle
	} else {
		A = new SparseMatrixPCG(size, numNonZeros, MATRIX_GENERAL);
	}
	B = new double[size];
	memset(B, 0, size * sizeof(double));
}

void SparseVolumeEqnBuilder::computeLHS(int index, double* lambdas, double& Aii, int& numCols, int* columnIndices, double* columnValues, bool& bSort)
{
	// here all the indices are global indices.
	string varname = var->getName();
	VolumeElement* pVolumeElement = mesh->getVolumeElements();
	Feature* feature = pVolumeElement[index].getFeature();
	VolumeVarContextExpression* varContext = feature->getVolumeVarContext( (VolumeVariable*)var);
	assert(varContext);
	int mask = pVolumeElement[index].neighborMask;
	numCols = 0;
	Aii = 0.0;
	bSort = false; // for periodic boundary condition, sometimes have to sort to make sure order
	if (mask & BOUNDARY_TYPE_DIRICHLET // dirichlet
		|| (mask & NEIGHBOR_XP_BOUNDARY && feature->getXpBoundaryType() == BOUNDARY_PERIODIC)  // periodic plus direction
		|| (mask & NEIGHBOR_YP_BOUNDARY && feature->getYpBoundaryType() == BOUNDARY_PERIODIC)
		|| (mask & NEIGHBOR_ZP_BOUNDARY && feature->getZpBoundaryType() == BOUNDARY_PERIODIC)) {
		Aii = 1.0;
	} else { // non-dirichlet condition, including neumann or periodic minus or interior
		double volumeScale = 1.0;
		double lambdaX = lambdas[0];
		double lambdaY = lambdas[1];
		double lambdaZ = lambdas[2];
		double lambdaAreaX = lambdas[3];
		double lambdaAreaY = lambdas[4];
		double lambdaAreaZ = lambdas[5];

		double Di = varContext->getDiffusionRate(index);
		validateNumber(varname, index, "Diffusion term", Di);

		if (mask & NEIGHBOR_BOUNDARY_MASK){   // boundary
			volumeScale /= (mask & VOLUME_MASK);

			if (mask & NEIGHBOR_X_BOUNDARY_MASK){
				lambdaY /= 2.0;
				lambdaZ /= 2.0;
				lambdaAreaY /= 2.0;
				lambdaAreaZ /= 2.0;
			}
			if (mask & NEIGHBOR_Y_BOUNDARY_MASK){
				lambdaX /= 2.0;
				lambdaZ /= 2.0;
				lambdaAreaX /= 2.0;
				lambdaAreaZ /= 2.0;
			}
			if (mask & NEIGHBOR_Z_BOUNDARY_MASK){
				lambdaX /= 2.0;
				lambdaY /= 2.0;
				lambdaAreaX /= 2.0;
				lambdaAreaY /= 2.0;
			}
		}

		VolumeNeighbor volumeNeighbors[6] = {
			VolumeNeighbor((DIM < 3 || mask & NEIGHBOR_ZM_MASK) ? -1 : index - SIZEXY),  // neighbor index, -1 if there is no such neighbor
			VolumeNeighbor((DIM < 2 || mask & NEIGHBOR_YM_MASK) ? -1 : index - SIZEX),
			VolumeNeighbor((mask & NEIGHBOR_XM_MASK) ? -1 : index - 1),
			VolumeNeighbor((mask & NEIGHBOR_XP_MASK) ? -1 : index + 1),
			VolumeNeighbor((DIM < 2 || mask & NEIGHBOR_YP_MASK) ? -1 : index + SIZEX),
			VolumeNeighbor((DIM < 3 || mask & NEIGHBOR_ZP_MASK) ? -1 : index + SIZEXY)
		};

		XYZNeighbor startNeighbor, endNeighbor;
		switch (DIM) {
			case 1:
				startNeighbor = XM;
                endNeighbor = XP;
				break;
			case 2:
				startNeighbor = YM;
                endNeighbor = YP;
				break;
			case 3:
				startNeighbor = ZM;
                endNeighbor = ZP;
				break;
		}
		{
			// if I am a periodic boundary point, make minus neighbor of corresponding plus point as one of my neighbors
			// minus directions inherit membrane from plus directions.
			if ((mask & NEIGHBOR_XM_BOUNDARY) && feature->getXmBoundaryType() == BOUNDARY_PERIODIC) {
				// make sure XM and XP are in the same feature for each periodic bounary point
				int xpindex = index + (SIZEX - 1);
				if (feature != pVolumeElement[xpindex].getFeature()) {
					throw "Periodic Boundary Condition (X- and X+): compartments don't match";
				}
				int xpmask = pVolumeElement[xpindex].neighborMask;
				// inherit membrane from XP
				mask |= (xpmask & NEIGHBOR_XM_MEMBRANE);

				// remove boundary (keep membrane) if it's periodic
				volumeNeighbors[XM].index = (mask & NEIGHBOR_XM_MEMBRANE) ? -1 : index + (SIZEX - 2);
				volumeNeighbors[XM].bPeriodic = true;
				volumeScale *= 2;
				lambdaY *= 2.0;
				lambdaZ *= 2.0;
				lambdaAreaY *= 2.0;
				lambdaAreaZ *= 2.0;
				bSort = true;
			}
			if (DIM > 1 && (mask & NEIGHBOR_YM_BOUNDARY) && feature->getYmBoundaryType() == BOUNDARY_PERIODIC) {
				int ypindex = index + (SIZEY - 1) * SIZEX;
				if (feature != pVolumeElement[ypindex].getFeature()) {
					throw "Periodic Boundary Condition (Y- and Y+): compartments don't match";
				}
				int ypmask = pVolumeElement[ypindex].neighborMask;
				mask |= (ypmask & NEIGHBOR_YM_MEMBRANE);

				volumeNeighbors[YM].index = (mask & NEIGHBOR_YM_MEMBRANE) ? -1 : index + (SIZEY - 2) * SIZEX;
				volumeNeighbors[YM].bPeriodic = true;
				volumeScale *= 2;
				lambdaX *= 2.0;
				lambdaZ *= 2.0;
				lambdaAreaX *= 2.0;
				lambdaAreaZ *= 2.0;
				bSort = true;
			}
			if (DIM > 2 && (mask & NEIGHBOR_ZM_BOUNDARY) && feature->getZmBoundaryType() == BOUNDARY_PERIODIC) {
				int zpindex = index + (SIZEZ - 1) * SIZEXY;
				if (feature != pVolumeElement[zpindex].getFeature()) {
					throw "Periodic Boundary Condition (Z- and Z+): compartments don't match";
				}
				int zpmask = pVolumeElement[zpindex].neighborMask;
				mask |= (zpmask & NEIGHBOR_ZM_MEMBRANE);

				volumeNeighbors[ZM].index = (mask & NEIGHBOR_ZM_MEMBRANE) ? -1 : index + (SIZEZ - 2) * SIZEXY;
				volumeNeighbors[ZM].bPeriodic = true;
				volumeScale *= 2;
				lambdaX *= 2.0;
				lambdaY *= 2.0;
				lambdaAreaX *= 2.0;
				lambdaAreaY *= 2.0;
				bSort = true;
			}

			// if my neighbor is a plus periodic boundary point, make the corresponding minus point as one of my neighbors
			int neighborIndex;
			int neighborMask;
			Feature* neighborFeature = 0;
			neighborIndex = volumeNeighbors[XP].index;
			if (neighborIndex >= 0) {
				neighborMask = pVolumeElement[neighborIndex].neighborMask;
				neighborFeature = pVolumeElement[neighborIndex].getFeature();
				if (neighborMask & NEIGHBOR_XP_BOUNDARY && neighborFeature->getXpBoundaryType() == BOUNDARY_PERIODIC) {
					volumeNeighbors[XP].index = index - (SIZEX - 2);
					volumeNeighbors[XP].bPeriodic = true;
					bSort = true;
				}
			}

			neighborIndex = volumeNeighbors[YP].index;
			if (DIM > 1 && neighborIndex >= 0) {
				neighborMask = pVolumeElement[neighborIndex].neighborMask;
				neighborFeature = pVolumeElement[neighborIndex].getFeature();
				if (neighborMask & NEIGHBOR_YP_BOUNDARY && neighborFeature->getYpBoundaryType() == BOUNDARY_PERIODIC) {
					volumeNeighbors[YP].index = index - (SIZEY - 2) * SIZEX;
					volumeNeighbors[YP].bPeriodic = true;
					bSort = true;
				}
			}

			neighborIndex = volumeNeighbors[ZP].index;
			if (DIM > 2 && neighborIndex >= 0) {
				neighborMask = pVolumeElement[neighborIndex].neighborMask;
				neighborFeature = pVolumeElement[neighborIndex].getFeature();
				if (neighborMask & NEIGHBOR_ZP_BOUNDARY && neighborFeature->getZpBoundaryType() == BOUNDARY_PERIODIC) {
					volumeNeighbors[ZP].index = index - (SIZEZ - 2) * SIZEXY;
					volumeNeighbors[ZP].bPeriodic = true;
					bSort = true;
				}
			}
		}

		double neighborLambdas[6] = {lambdaZ, lambdaY, lambdaX, lambdaX, lambdaY, lambdaZ};
		double neighborLambdaAreas[6] = {lambdaAreaZ, lambdaAreaY, lambdaAreaX, lambdaAreaX, lambdaAreaY, lambdaAreaZ};

		// compute these values only when there is convection
		if (!bSymmetricStorage) {
			for (int i = startNeighbor; i <= endNeighbor; i ++) {
				volumeNeighbors[i].setConvectionCoefficients(index, mask, (XYZNeighbor)i, varContext);
			}
		}

		Aii = volumeScale;
		// loop through neighbors to compute Aii and Aij
		// if there is no convection, we only store upper triangle (neighborIndex > index)
		// if one of the neighbors is dirichlet point, we store the value into a vector,
		// then later when building the right hand side, we added the these values to right hand side.
		for (int n = startNeighbor; n <= endNeighbor; n ++) {
			int neighborIndex = volumeNeighbors[n].index;
			if (neighborIndex >= 0) {
				double lambda = neighborLambdas[n];
				double Dj = varContext->getDiffusionRate(neighborIndex);
				double D = (Di + Dj < epsilon) ? (0.0) : (2 * Di * Dj/(Di + Dj));
				double Aij = 0.0;
				if (bSymmetricStorage) {
					Aij = D * lambda;
					Aii += Aij;
				} else { // if there is convection
					double convectionDirection = volumeNeighbors[n].convectionDirection;
					double Vi = volumeNeighbors[n].Vi, Vj = volumeNeighbors[n].Vj;
					double lamdaArea = neighborLambdaAreas[n];
					validateNumber(varname, index, "Velocity term", Vi);
					validateNumber(varname, neighborIndex, "Velocity term", Vj);
					double V = 0.5 * (Vi + Vj);
					Aij = max<double>(D * lambda + convectionDirection * 0.5 * V * lamdaArea, max<double>(convectionDirection * V * lamdaArea, 0));
					Aii += max<double>(D * lambda - convectionDirection * 0.5 * V * lamdaArea, max<double>(- convectionDirection * V * lamdaArea, 0));
				}
				if (Aij != 0.0) {
					int neighborMask = pVolumeElement[neighborIndex].neighborMask;
					validateNumber(varname, index, "LHS", Aij);
					if (neighborMask & BOUNDARY_TYPE_DIRICHLET) { // dirichlet
						dirichletNeighbors.push_back(new CoupledNeighbors(index, neighborIndex, Aij));
					} else if (!bSymmetricStorage || neighborIndex > index) {
						columnIndices[numCols] = neighborIndex;
						columnValues[numCols] = - Aij;
						numCols ++;
					}
					{
						// for periodic boundary condition
						if (volumeNeighbors[n].bPeriodic) {
							switch (n) {
								// if I am a periodic boundary point
								case ZM:
									periodicNeighbors.push_back(new CoupledNeighbors(index, neighborIndex, - Aij * varContext->getZBoundaryPeriodicConstant()));
									break;
								case YM:
									periodicNeighbors.push_back(new CoupledNeighbors(index, neighborIndex, - Aij * varContext->getYBoundaryPeriodicConstant()));
									break;
								case XM:
									periodicNeighbors.push_back(new CoupledNeighbors(index, neighborIndex, - Aij * varContext->getXBoundaryPeriodicConstant()));
									break;
								// if my neighbor is a periodic boundary point
								case XP:
									periodicNeighbors.push_back(new CoupledNeighbors(index, neighborIndex, Aij * varContext->getXBoundaryPeriodicConstant()));
									break;
								case YP:
									periodicNeighbors.push_back(new CoupledNeighbors(index, neighborIndex, Aij * varContext->getYBoundaryPeriodicConstant()));
									break;
								case ZP:
									periodicNeighbors.push_back(new CoupledNeighbors(index, neighborIndex, Aij * varContext->getZBoundaryPeriodicConstant()));
									break;
							}
						}
					}
				} // end if (Aij != 0.0)
			} // end (neighborIndex >= 0)
		} // end for n
		validateNumber(varname, index, "LHS", Aii);
	} // end if else (mask & BOUNDARY_TYPE_DIRICHLET)
}

//------------------------------------------------------------------
//
// Left Hand Side
//
//------------------------------------------------------------------
void SparseVolumeEqnBuilder::initEquation(VCellModel* model, double deltaTime, int volumeIndexStart, int volumeIndexSize, int membraneIndexStart, int membraneIndexSize)
{
	/**
	 * we decided to scale both sides with deltaT/VOLUME
	 * we defined volumeScale_i such that VOLUME * volumeScale_i = V_i
	 *
	 * Aii = volumeScale + sum(Aij)
	 * Aij = - Di * lambda, lambda is lambdaX or lambdaY or lambdaZ, for example,
	 * lambdaX = deltaT / (deltaX * deltaX)
	 * lambda is scaled for boundary points
	 *
	 * B_i = U_old * volumeScale_i + R * deltaT + boundary conditions
	 **/

	double LAMBDAX = deltaTime/(DELTAX * DELTAX);
	double LAMBDAY = deltaTime/(DELTAY * DELTAY);
	double LAMBDAZ = deltaTime/(DELTAZ * DELTAZ);
	double LAMBDAAREAX = deltaTime/DELTAX;
	double LAMBDAAREAY = deltaTime/DELTAY;
	double LAMBDAAREAZ = deltaTime/DELTAZ;
	double lambdas[] = {LAMBDAX, LAMBDAY, LAMBDAZ, LAMBDAAREAX, LAMBDAAREAY, LAMBDAAREAZ};

	if (!bPreProcessed) {
		preProcess(model);
	}

	ASSERTION(solver->getVar() == var);

	// if it's time dependent diffusion, we need to start over
	A->clear();
	for (int i = 0; i < (int)dirichletNeighbors.size(); i ++) {
		delete dirichletNeighbors[i];
	}
	dirichletNeighbors.clear();
	for (int i = 0; i < (int)periodicNeighbors.size(); i ++) {
		delete periodicNeighbors[i];
	}
	periodicNeighbors.clear();

	double Aii = 0;
	int numCols = 0;
	int* columnIndices = new int[2 * DIM];
	double* columnValues = new double[2 * DIM];
	bool bSort = false;

	if (bSolveWholeMesh) {
		for (int index = volumeIndexStart; index < volumeIndexStart + volumeIndexSize; index ++){
			A->addNewRow();
			computeLHS(index, lambdas, Aii, numCols, columnIndices, columnValues, bSort);
			if (numCols > 0) {
				if (bSort) {
					sortColumns(numCols, columnIndices, columnValues);
				}
				A->setRow(Aii, numCols, columnIndices, columnValues);
			} else {
				A->setDiag(index, Aii);
			}
		}
	} else {
		for (int localIndex = 0; localIndex < getSize() ; localIndex ++) {
			int globalIndex = LocalToGlobalMap[localIndex];
			A->addNewRow();
			computeLHS(globalIndex, lambdas, Aii, numCols, columnIndices, columnValues, bSort);
			if (numCols > 0) {
				// has to transfer all the global indices to local indices
				for (int k = 0; k < numCols; k ++) {
					int neighborGlobalIndex = columnIndices[k];
					columnIndices[k] = GlobalToLocalMap[neighborGlobalIndex];
					if (columnIndices[k] < 0) {
						char ss[128];
						sprintf(ss, "Index %d, found  a neighbor (index %d) that's not in solved regions", globalIndex, neighborGlobalIndex);
						throw ss;
					}
				}
				sortColumns(numCols, columnIndices, columnValues);
				A->setRow(Aii, numCols, columnIndices, columnValues);
			} else {
				A->setDiag(localIndex, Aii);
			}
		}
		// do global to local mapping for dirichlet points
		for (int i = 0; i < (int)dirichletNeighbors.size(); i ++) {
			CoupledNeighbors* dn = dirichletNeighbors[i];
			dn->centerIndex = GlobalToLocalMap[dn->centerIndex];
			dn->neighborIndex = GlobalToLocalMap[dn->neighborIndex];
			assert(dn->centerIndex >= 0);
			assert(dn->neighborIndex >= 0);
		}
		// do global to local mapping for periodic points
		for (int i = 0; i < (int)periodicNeighbors.size(); i ++) {
			CoupledNeighbors* pn = periodicNeighbors[i];
			pn->centerIndex = GlobalToLocalMap[pn->centerIndex];
			pn->neighborIndex = GlobalToLocalMap[pn->neighborIndex];
			assert(pn->centerIndex >= 0);
			assert(pn->neighborIndex >= 0);
		}
	}

	A->close();

	delete[] columnIndices;
	delete[] columnValues;
}

double SparseVolumeEqnBuilder::computeRHS(Simulation* sim, int index, double deltaTime, double* lambdas, double bInit) {
	string varname = var->getName();
	double b = bInit;

	VolumeElement *pVolumeElement = mesh->getVolumeElements();
	MembraneElement *pMembraneElement = mesh->getMembraneElements();

	Feature* feature = pVolumeElement[index].getFeature();
	VolumeVarContextExpression* varContext = feature->getVolumeVarContext((VolumeVariable*)var);
	int mask = pVolumeElement[index].neighborMask;

	if (mask & BOUNDARY_TYPE_DIRICHLET){
		if ((mask & NEIGHBOR_XM_BOUNDARY) && (feature->getXmBoundaryType() == BOUNDARY_VALUE)){
			sim->advanceTimeOn();
			b = varContext->getXmBoundaryValue(index);
			sim->advanceTimeOff();

		} else if ((mask & NEIGHBOR_XP_BOUNDARY) && (feature->getXpBoundaryType() == BOUNDARY_VALUE)){
			sim->advanceTimeOn();
			b = varContext->getXpBoundaryValue(index);
			sim->advanceTimeOff();

		} else if ((mask & NEIGHBOR_YM_BOUNDARY) && (feature->getYmBoundaryType() == BOUNDARY_VALUE)){
			sim->advanceTimeOn();
			b = varContext->getYmBoundaryValue(index);
			sim->advanceTimeOff();

		} else if ((mask & NEIGHBOR_YP_BOUNDARY) && (feature->getYpBoundaryType() == BOUNDARY_VALUE)){
			sim->advanceTimeOn();
			b = varContext->getYpBoundaryValue(index);
			sim->advanceTimeOff();

		} else if ((mask & NEIGHBOR_ZM_BOUNDARY) && (feature->getZmBoundaryType() == BOUNDARY_VALUE)){
			sim->advanceTimeOn();
			b = varContext->getZmBoundaryValue(index);
			sim->advanceTimeOff();

		} else if ((mask & NEIGHBOR_ZP_BOUNDARY) && (feature->getZpBoundaryType() == BOUNDARY_VALUE)){
			sim->advanceTimeOn();
			b = varContext->getZpBoundaryValue(index);
			sim->advanceTimeOff();

		} else {
			assert(0);
		}

	} else if (((mask & NEIGHBOR_XP_BOUNDARY) && feature->getXpBoundaryType() == BOUNDARY_PERIODIC)  // periodic and plus direction
			|| ((mask & NEIGHBOR_YP_BOUNDARY) && feature->getYpBoundaryType() == BOUNDARY_PERIODIC)
			|| ((mask & NEIGHBOR_ZP_BOUNDARY) && feature->getZpBoundaryType() == BOUNDARY_PERIODIC)) {
		b = 0;
	} else { // no Dirichlet conditions (interior or neumann or minus periodic)
		double volumeScale = 1.0;
		double lambdaAreaX = lambdas[0];
		double lambdaAreaY = lambdas[1];
		double lambdaAreaZ = lambdas[2];

		double reactionRate = varContext->getReactionRate(index);
		b += reactionRate * deltaTime;

		bool bPeriodic = false;
		int plusPeriodicNeighbors[3];
		
		if (mask & NEIGHBOR_BOUNDARY_MASK){   // boundary
			volumeScale /= (mask & VOLUME_MASK);

			if (mask & NEIGHBOR_X_BOUNDARY_MASK){
				lambdaAreaY /= 2.0;
				lambdaAreaZ /= 2.0;
			}
			if (mask & NEIGHBOR_Y_BOUNDARY_MASK){
				lambdaAreaX /= 2.0;
				lambdaAreaZ /= 2.0;
			}
			if (mask & NEIGHBOR_Z_BOUNDARY_MASK){
				lambdaAreaX /= 2.0;
				lambdaAreaY /= 2.0;
			}
			if ((mask & NEIGHBOR_XM_BOUNDARY) && (feature->getXmBoundaryType() == BOUNDARY_PERIODIC)) {
				volumeScale *= 2;
				bPeriodic = true;
				plusPeriodicNeighbors[0] = index + SIZEX - 1;
			} else {
				plusPeriodicNeighbors[0] = -1;
			}
			if ((mask & NEIGHBOR_YM_BOUNDARY) && (feature->getYmBoundaryType() == BOUNDARY_PERIODIC)) {
				volumeScale *= 2;
				bPeriodic = true;
				plusPeriodicNeighbors[1] = index + (SIZEY - 1) * SIZEX;
			} else {
				plusPeriodicNeighbors[1] = -1;
			}
			if ((mask & NEIGHBOR_ZM_BOUNDARY) && (feature->getZmBoundaryType() == BOUNDARY_PERIODIC)) {
				volumeScale *= 2;
				bPeriodic = true;
				plusPeriodicNeighbors[2] = index + (SIZEZ - 1) * SIZEXY;
			} else {
				plusPeriodicNeighbors[2] = -1;
			}

			if (bPeriodic) {
				for (int k = 0; k < 3; k ++) {
					if (plusPeriodicNeighbors[k] < 0) {
						continue;
					}

					double plusReactinoRate = varContext->getReactionRate(plusPeriodicNeighbors[k]);
					if (fabs(reactionRate - plusReactinoRate) > 1e-3 * fabs(reactionRate)) {
						throw "non periodic reaction rate at periodic volume.";
					}
				}
			}

			b *= volumeScale;

			if ((mask & BOUNDARY_TYPE_MASK) == BOUNDARY_TYPE_NEUMANN) { // for corners, it might be both neuman and periodic, but periodic wins.
				if (mask & NEIGHBOR_XM_BOUNDARY && feature->getXmBoundaryType() == BOUNDARY_FLUX){
					sim->advanceTimeOn();
					b += varContext->getXmBoundaryFlux(index) * lambdaAreaX;
					sim->advanceTimeOff();
				}
				if (mask & NEIGHBOR_XP_BOUNDARY && feature->getXpBoundaryType() == BOUNDARY_FLUX){
					sim->advanceTimeOn();
					b += - varContext->getXpBoundaryFlux(index) * lambdaAreaX;
					sim->advanceTimeOff();
				}
				if (mask & NEIGHBOR_YM_BOUNDARY && feature->getYmBoundaryType() == BOUNDARY_FLUX){
					sim->advanceTimeOn();
					b += varContext->getYmBoundaryFlux(index) * lambdaAreaY;
					sim->advanceTimeOff();
				}
				if (mask & NEIGHBOR_YP_BOUNDARY && feature->getYpBoundaryType() == BOUNDARY_FLUX){
					sim->advanceTimeOn();
					b += - varContext->getYpBoundaryFlux(index) * lambdaAreaY;
					sim->advanceTimeOff();
				}
				if (mask & NEIGHBOR_ZM_BOUNDARY && feature->getZmBoundaryType() == BOUNDARY_FLUX){
					sim->advanceTimeOn();
					b += varContext->getZmBoundaryFlux(index) * lambdaAreaZ;
					sim->advanceTimeOff();
				}
				if (mask & NEIGHBOR_ZP_BOUNDARY && feature->getZpBoundaryType() == BOUNDARY_FLUX){
					sim->advanceTimeOn();
					b += - varContext->getZpBoundaryFlux(index) * lambdaAreaZ;
					sim->advanceTimeOff();
				}
			}
		}
		if (mask & NEIGHBOR_MEMBRANE_MASK){ // if there is membrane
			int numAdjacentME = (int)pVolumeElement[index].adjacentMembraneIndexes.size();
			for (int i = 0; i < numAdjacentME; i ++) {
				MembraneElement *me = pMembraneElement + pVolumeElement[index].adjacentMembraneIndexes[i];
				sim->advanceTimeOn();
				double flux = varContext->getFlux(me);
				sim->advanceTimeOff();
				b += flux * me->area * deltaTime / VOLUME;
			}
			if (bPeriodic) {
				for (int k = 0; k < 3; k ++) {
					if (plusPeriodicNeighbors[k] < 0) {
						continue;
					}
					// periodic pairs should be in the same feature, so use the same varContext
					int numAdjacentME = (int)pVolumeElement[plusPeriodicNeighbors[k]].adjacentMembraneIndexes.size();
					for (int i = 0; i < numAdjacentME; i ++) {
						MembraneElement *me = pMembraneElement + pVolumeElement[plusPeriodicNeighbors[k]].adjacentMembraneIndexes[i];
						sim->advanceTimeOn();
						double flux = varContext->getFlux(me);
						sim->advanceTimeOff();
						b += flux * me->area * deltaTime / VOLUME;
					}
				}
			}
		} // end if (mask & NEIGHBOR_MEMBRANE_MASK)
	} // end if else (mask & BOUNDARY_TYPE_DIRICHLET)
	validateNumber(varname, index, "RHS", b);
	return b;
}

//------------------------------------------------------------------
//
// Right Hand side
//
//------------------------------------------------------------------
void SparseVolumeEqnBuilder::buildEquation(Simulation* sim, double deltaTime, int volumeIndexStart, int volumeIndexSize, int membraneIndexStart, int membraneIndexSize)
{
	double lambdaAreaX = deltaTime/DELTAX;
	double lambdaAreaY = deltaTime/DELTAY;
	double lambdaAreaZ = deltaTime/DELTAZ;
	double lambdas[3] = {lambdaAreaX, lambdaAreaY, lambdaAreaZ};

	if (bSolveWholeMesh) {
		memcpy(B, var->getCurr(), var->getSize() * sizeof(double));
		for (int index = volumeIndexStart; index < volumeIndexStart + volumeIndexSize; index ++){
			B[index] = computeRHS(sim, index, deltaTime, lambdas, B[index]);
		}
	} else {
		double* currVal = var->getCurr();
		for (int localIndex = 0; localIndex < getSize() ; localIndex ++) {
			int globalIndex = LocalToGlobalMap[localIndex];
			// to initialize X, which will be passed to solver as intial guess and final solution.
			// or set initial guess to zero (need to revisit, we also want to revisit fill-in parameter)
			X[localIndex] = currVal[globalIndex];
			B[localIndex] = computeRHS(sim, globalIndex, deltaTime, lambdas, X[localIndex]);
		}
	}
	// to make the matrix symmetric
	// for the points who have dirichlet neighbors
	// we have to change the right hand side
	// in here, if we solve for regions, all the indices are already local indices.
	for (int i = 0; i < (int)dirichletNeighbors.size(); i ++) {
		CoupledNeighbors* dn = dirichletNeighbors[i];
		B[dn->centerIndex] += dn->coeff * B[dn->neighborIndex];
	}
	for (int i = 0; i < (int)periodicNeighbors.size(); i ++) {
		CoupledNeighbors* pn = periodicNeighbors[i];
		B[pn->centerIndex] += pn->coeff;
	}
}

bool SparseVolumeEqnBuilder::checkPeriodicCoupledPairsInRegions(int indexm, int indexp) {
	if (bSolveWholeMesh) {
		return true;
	}

	int localIndexM = GlobalToLocalMap[indexm];
	int localIndexP = GlobalToLocalMap[indexp];
	if (localIndexM != -1 && localIndexP != -1) { // both of them are in the solved regions, add them to the list
		return true;
	}
	if (localIndexM == -1 && localIndexP != -1 	|| localIndexM != -1 && localIndexP == -1) { // one of them is not in the solved regions
		char ss[128];
		sprintf(ss, "one of two periodic neighbors (index %d) and (index %d) is not in solved regions", indexm, indexp);
		throw ss;
	}
	return false; // both of them are not in the solved regions, so we don't have to add them to the list
}

#define PERIODIC_GEOMETRY_ERROR_MESSAGE "geometry is not compatible with periodic boundary conditions. Couldn't find corresponding volume element."

void SparseVolumeEqnBuilder::preProcess(VCellModel* model) {
	if (bPreProcessed) {
		return;
	}

	bPreProcessed = true;

	// check if there is periodic boundary condition in the model
	bool bPeriodic = false;
	for (int i = 0; i < model->getNumFeatures(); i ++) {
		Feature* feature = model->getFeatureFromIndex(i);
		if (feature->getXmBoundaryType() == BOUNDARY_PERIODIC
			|| feature->getYmBoundaryType() == BOUNDARY_PERIODIC
			|| feature->getZmBoundaryType() == BOUNDARY_PERIODIC) {
			bPeriodic = true;
			break;
		}
	}

	if (!bPeriodic) {
		return;
	}

	// intialize periodic minus and plus pair, which is used to update plus points at each time step
	VolumeElement* pVolumeElement = mesh->getVolumeElements();

	// X direction
	if (SIZEX > 1) {
		for (int k = 0; k < SIZEZ; k ++){
			for (int j = 0; j < SIZEY; j ++){
				int indexm = k * SIZEXY + j * SIZEX;
				int indexp = k * SIZEXY + j * SIZEX + SIZEX - 1;
				int mask = pVolumeElement[indexm].neighborMask;
				VolumeVarContextExpression* varContext = pVolumeElement[indexm].getFeature()->getVolumeVarContext((VolumeVariable*)var);
				if (mask & NEIGHBOR_XM_BOUNDARY && pVolumeElement[indexm].getFeature()->getXmBoundaryType() == BOUNDARY_PERIODIC) {
					if (pVolumeElement[indexm].getFeature() != pVolumeElement[indexp].getFeature()) {
						throw PERIODIC_GEOMETRY_ERROR_MESSAGE;
					}
					if (bSolveWholeMesh || checkPeriodicCoupledPairsInRegions(indexm, indexp)) {
						periodicCoupledPairs.push_back(new CoupledNeighbors(indexm, indexp, varContext->getXBoundaryPeriodicConstant()));
					}
				}
			}
		}
	}
	// Y direction
	if (SIZEY > 1) {
		for (int k = 0; k < SIZEZ; k ++){
			for (int i = 0; i < SIZEX; i ++){
				int indexm = k * SIZEXY + i;
				int indexp = k * SIZEXY + (SIZEY - 1) * SIZEX + i;
				int mask = pVolumeElement[indexm].neighborMask;
				VolumeVarContextExpression* varContext = pVolumeElement[indexm].getFeature()->getVolumeVarContext((VolumeVariable*)var);
				if (mask & NEIGHBOR_YM_BOUNDARY && pVolumeElement[indexm].getFeature()->getYmBoundaryType() == BOUNDARY_PERIODIC) {
					if (pVolumeElement[indexm].getFeature() != pVolumeElement[indexp].getFeature()) {
						throw PERIODIC_GEOMETRY_ERROR_MESSAGE;
					}
					if (bSolveWholeMesh || checkPeriodicCoupledPairsInRegions(indexm, indexp)) {
						periodicCoupledPairs.push_back(new CoupledNeighbors(indexm, indexp, varContext->getYBoundaryPeriodicConstant()));
					}
				}
			}
		}
	}

	// Z direction
	if (SIZEZ > 1) {
		for (int j = 0; j < SIZEY; j ++){
			for (int i = 0; i < SIZEX; i ++){
				int indexm = j * SIZEX + i;
				int indexp = (SIZEZ - 1) * SIZEXY + j * SIZEX + i;
				int mask = pVolumeElement[indexm].neighborMask;
				VolumeVarContextExpression* varContext = pVolumeElement[indexm].getFeature()->getVolumeVarContext((VolumeVariable*)var);
				if (mask & NEIGHBOR_ZM_BOUNDARY && pVolumeElement[indexm].getFeature()->getZmBoundaryType() == BOUNDARY_PERIODIC) {
					if (pVolumeElement[indexm].getFeature() != pVolumeElement[indexp].getFeature()) {
						throw PERIODIC_GEOMETRY_ERROR_MESSAGE;
					}
					if (bSolveWholeMesh || checkPeriodicCoupledPairsInRegions(indexm, indexp)) {
						periodicCoupledPairs.push_back(new CoupledNeighbors(indexm, indexp, varContext->getZBoundaryPeriodicConstant()));
					}
				}
			}
		}
	}
}

void SparseVolumeEqnBuilder::postProcess() {
	double* currSol = var->getCurr();

	// if solve for regions, do local to global mapping
	if (!bSolveWholeMesh) {
		for (int localIndex = 0; localIndex < getSize() ; localIndex ++) {
			int globalIndex = LocalToGlobalMap[localIndex];
			currSol[globalIndex] = X[localIndex];
		}
	}

	// update plus periodic points
	for (int i = 0; i < (int)periodicCoupledPairs.size(); i ++){
		CoupledNeighbors* pcp = periodicCoupledPairs[i];
		currSol[pcp->neighborIndex] = currSol[pcp->centerIndex] + pcp->coeff;
	}
}
