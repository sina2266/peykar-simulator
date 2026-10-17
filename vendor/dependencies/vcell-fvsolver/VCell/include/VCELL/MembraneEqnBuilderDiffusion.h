/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef MEMBRANEEQNBUILDERDIFFUSION_H
#define MEMBRANEEQNBUILDERDIFFUSION_H

#include <VCELL/SparseMatrixEqnBuilder.h>
#include <vector>
using std::vector;
#include <map>
using std::pair;

class Mesh;
class MembraneVariable;


class MembraneEqnBuilderDiffusion : public SparseMatrixEqnBuilder
{
public:
	MembraneEqnBuilderDiffusion(MembraneVariable *species, Mesh *mesh);
	~MembraneEqnBuilderDiffusion() override;

	void initEquation(VCellModel* model, double deltaTime, int volumeIndexStart, int volumeIndexSize, int membraneIndexStart, int membraneIndexSize) override;
	void buildEquation(Simulation* sim, double deltaTime, int volumeIndexStart, int volumeIndexSize, int membraneIndexStart, int membraneIndexSize) override;
	void postProcess() override;

private:
	vector<pair<int, int> > periodicPairs; // map of minus and plus pairs of periodic boundary points.

	void initEquation_Periodic(VCellModel* model, double deltaTime, int volumeIndexStart, int volumeIndexSize, int membraneIndexStart, int membraneIndexSize);
	void buildEquation_Periodic(Simulation* sim, double deltaTime, int volumeIndexStart, int volumeIndexSize, int membraneIndexStart, int membraneIndexSize);

	bool bPreProcessed;
	void preProcess(VCellModel* model);

private:
	double computeDiffusionConstant(int meIndex, int neighborIndex);
};    

#endif
