/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef MEMBRANEREGIONEQNBUILDER_H
#define MEMBRANEREGIONEQNBUILDER_H

#include <VCELL/EqnBuilder.h>

class Mesh;
class ODESolver;
class VolumeRegionVariable;
class MembraneRegionVariable;

class MembraneRegionEqnBuilder : public EqnBuilder
{
public:
    MembraneRegionEqnBuilder(MembraneRegionVariable *var, CartesianMesh *mesh, ODESolver *solver);
    void initEquation(VCellModel* model, double deltaTime, int volumeIndexStart, int volumeIndexSize, int membraneIndexStart, int membraneIndexSize) override
    { }
    void buildEquation(Simulation* sim, double deltaTime, int volumeIndexStart, int volumeIndexSize, int membraneIndexStart, int membraneIndexSize) override;

private:
	ODESolver* odeSolver;
};

#endif
