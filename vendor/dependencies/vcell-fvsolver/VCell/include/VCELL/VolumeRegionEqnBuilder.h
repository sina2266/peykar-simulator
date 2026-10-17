/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef VOLUMEREGIONEQNBUILDER_H
#define VOLUMEREGIONEQNBUILDER_H

#include <VCELL/EqnBuilder.h>

class CartesianMesh;
class ODESolver;
class VolumeRegionVariable;
class MembraneRegionVariable;

class VolumeRegionEqnBuilder : public EqnBuilder
{
public:
    VolumeRegionEqnBuilder(VolumeRegionVariable *species, CartesianMesh *mesh, ODESolver *solver);
    void initEquation(VCellModel* model, double deltaTime, int volumeIndexStart, int volumeIndexSize, int membraneIndexStart, int membraneIndexSize) override
    {}
    void buildEquation(Simulation* sim, double deltaTime, int volumeIndexStart, int volumeIndexSize, int membraneIndexStart, int membraneIndexSize) override;

private:
	ODESolver* odeSolver;
};

#endif
