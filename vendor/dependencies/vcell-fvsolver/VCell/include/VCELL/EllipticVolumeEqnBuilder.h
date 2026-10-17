/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef ELLIPTICVOLUMEEQNBUILDER_H
#define ELLIPTICVOLUMEEQNBUILDER_H

#include <VCELL/SparseMatrixEqnBuilder.h>
#include <vector>
using std::vector;

class CartesianMesh;
class VolumeVariable;
struct CoupledNeighbors;
struct VolumeNeighbor;

/*
 * We always use symmetric matrix when there is no convection no matter
 * what the boundary conditions are.
 * We always use non-symmetric matrix when there is convection, of course.
 *
 */
class EllipticVolumeEqnBuilder : public SparseMatrixEqnBuilder
{
public:
	EllipticVolumeEqnBuilder(VolumeVariable *species, CartesianMesh *mesh, int numSolveRegions=0, int* solveRegions=nullptr);
	~EllipticVolumeEqnBuilder() override;

	void initEquation(VCellModel* model, double deltaTime, int volumeIndexStart, int volumeIndexSize, int membraneIndexStart, int membraneIndexSize);
	void buildEquation(Simulation *sim, double deltaTime, int volumeIndexStart, int volumeIndexSize, int membraneIndexStart, int membraneIndexSize);
	void postProcess();

	bool isElliptic() { 
		return true; 
	}

private:
	//bool bSymmetricStorage; // always no convection
	vector<CoupledNeighbors*> dirichletNeighbors; // Aij, j is dirichlet points, computed in LHS, used it RHS
	vector<CoupledNeighbors*> periodicNeighbors; // Aij * periodicConstant, j is dirichlet points, computed in LHS, used it RHS
	vector<CoupledNeighbors*> periodicCoupledPairs; // list of minus and plus pairs of periodic boundary points, used to update the solutions of these points.	
	bool bPreProcessed;

	int DIM;
	double DELTAX, DELTAY, DELTAZ;
	double AREAX, AREAY, AREAZ;
	double VOLUME;
	int SIZEX, SIZEY, SIZEZ, SIZEXY;
	double eLambdas[3];

	int numSolveRegions;
	int* solveRegions; // list of IDs of solve regions
	int* GlobalToLocalMap; // global to local mapping, total number of elements is mesh size
	int* LocalToGlobalMap; // local to global mapping, total number of elements is sum of region sizes.
	int* RegionFirstRow; // list of indexes of first row of each region (cummulative sum of number of nodes);
	bool bSolveWholeMesh;	

	void init();
	void computeLHS(int index, double& Aii, int& numCols, int* columnIndices, double* columnValues, bool& bSort);
	double computeRHS(Simulation* sim, int index);
	void preProcess(VCellModel *model);
	bool checkPeriodicCoupledPairsInRegions(int indexm, int indexp);
};    

#endif
