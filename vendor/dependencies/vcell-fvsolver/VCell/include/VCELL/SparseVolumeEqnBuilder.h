/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef SPARSEVOLUMEEQNBUILDER_H
#define SPARSEVOLUMEEQNBUILDER_H

#include <VCELL/VolumeVarContextExpression.h>
#include <VCELL/SparseMatrixEqnBuilder.h>
#include <vector>
using std::vector;

class CartesianMesh;
class VolumeVariable;
struct CoupledNeighbors;

/*
 * We always use symmetric matrix when there is no convection no matter
 * what the boundary conditions are.
 * We always use non-symmetric matrix when there is convection, of course.
 *
 */
class SparseVolumeEqnBuilder : public SparseMatrixEqnBuilder
{
public:
	SparseVolumeEqnBuilder(VolumeVariable *species, CartesianMesh *mesh, bool bNoConvection, int numSolveRegions=0, int* solveRegions=nullptr);
	~SparseVolumeEqnBuilder() override;

	void initEquation(VCellModel* model, double deltaTime, int volumeIndexStart, int volumeIndexSize, int membraneIndexStart, int membraneIndexSize) override;
	void buildEquation(Simulation* sim, double deltaTime, int volumeIndexStart, int volumeIndexSize, int membraneIndexStart, int membraneIndexSize) override;
	void postProcess() override;

private:
	bool bSymmetricStorage; // no convection, A would be symmetric
	vector<CoupledNeighbors*> dirichletNeighbors; // Aij, j is dirichlet points, computed in LHS, used it RHS
	vector<CoupledNeighbors*> periodicNeighbors; // Aij * periodicConstant, j is periodic points, computed in LHS, used it RHS
	vector<CoupledNeighbors*> periodicCoupledPairs; // list of minus and plus pairs of periodic boundary points, used to update the solutions of these points.	
	bool bPreProcessed;

	int DIM;
	double DELTAX, DELTAY, DELTAZ;
	double AREAX, AREAY, AREAZ;
	double VOLUME;
	int SIZEX, SIZEY, SIZEZ, SIZEXY;	

	int numSolveRegions;
	int* solveRegions; // list of IDs of solve regions
	int* GlobalToLocalMap; // global to local mapping, total number of elements is mesh size
	int* LocalToGlobalMap; // local to global mapping, total number of elements is sum of region sizes.
	int* RegionFirstRow; // list of indexes of first row of each region (cummulative sum of number of nodes);
	bool bSolveWholeMesh;	

	void init();
	void computeLHS(int index, double* lambdas, double& Aii, int& numCols, int* columnIndices, double* columnValues, bool& bSort);
	double computeRHS(Simulation* sim, int index, double deltaTime, double* lambdas, double bInit);
	void preProcess(VCellModel* model);
	bool checkPeriodicCoupledPairsInRegions(int indexm, int indexp);
};    

#endif
