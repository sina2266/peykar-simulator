/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef PDESOLVER_H
#define PDESOLVER_H

#include <VCELL/Solver.h>

class Variable;
class VolumeVariable;
class MembraneVariable;
class EqnBuilder;
class Mesh;
class CartesianMesh;
class VCellModel;

class PDESolver : public Solver
{
public:
	PDESolver(Variable *var, bool bTimeDependent);

	void initEqn(VCellModel* model, double deltaTime, int volumeIndexStart, int volumeIndexSize, int membraneIndexStart, int membraneIndexSize, bool bFirstTime) override;

	bool isPDESolver() override { return true; }
	bool isTimeDependent() const { return bTimeDependent; }
private:
	bool  bTimeDependent;
};

#endif
