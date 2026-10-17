/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef EQNBUILDERREACTIONFORWARD_H
#define EQNBUILDERREACTIONFORWARD_H

#include <VCELL/EqnBuilder.h>

class Mesh;
class ODESolver;
class VolumeVariable;

class EqnBuilderReactionForward : public EqnBuilder
{
public:
	EqnBuilderReactionForward(VolumeVariable *species, Mesh *mesh,  ODESolver *solver);

	void initEquation(VCellModel* model, double deltaTime, int volumeIndexStart, int volumeIndexSize, int membraneIndexStart, int membraneIndexSize) override
	{ }
	void buildEquation(Simulation* sim, double deltaTime, int volumeIndexStart, int volumeIndexSize, int membraneIndexStart, int membraneIndexSize) override;

private:
	ODESolver* odeSolver;
};    

#endif
