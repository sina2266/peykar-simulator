/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef EQNBUILDER_H
#define EQNBUILDER_H

class Mesh;
class Simulation;
class Variable;
class VCellModel;

class EqnBuilder
{
public:
	EqnBuilder(Variable *var, Mesh *mesh);
	virtual ~EqnBuilder() = default;

	virtual void initEquation(VCellModel* model, double deltaTime, int volumeIndexStart, int volumeIndexSize, int membraneIndexStart, int membraneIndexSize)=0;
	virtual void buildEquation(Simulation* sim, double deltaTime, int volumeIndexStart, int volumeIndexSize, int membraneIndexStart, int membraneIndexSize)=0;

	Mesh* getMesh() const { return mesh; };
	virtual bool isElliptic() { return false; }

protected:
	Variable   *var;
	Mesh       *mesh;
};    

#endif
