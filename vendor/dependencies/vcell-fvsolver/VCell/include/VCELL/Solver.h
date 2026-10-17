/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef SOLVER_H
#define SOLVER_H

class Variable;
class VolumeVariable;
class MembraneVariable;
class EqnBuilder;
class Mesh;
class CartesianMesh;
class SimTool;
class Simulation;
class VCellModel;

class Solver
{ 
public:
	explicit Solver(Variable *variable);
	virtual ~Solver() = default;

	virtual void initEqn(VCellModel* model, double deltaTime, int volumeIndexStart, int volumeIndexSize, int membraneIndexStart, int membraneIndexSize, bool bFirstTime);
	virtual void buildEqn(Simulation* sim, double deltaTime, int volumeIndexStart, int volumeIndexSize, int membraneIndexStart, int membraneIndexSize, bool bFirstTime);
	virtual void solveEqn(SimTool* sim_tool, double deltaTime, int volumeIndexStart, int volumeIndexSize, int membraneIndexStart, int membraneIndexSize, bool bFirstTime)=0;

	Variable *getVar() const { return var; }
	void setEqnBuilder(EqnBuilder *builder) { eqnBuilder = builder; }
	EqnBuilder *getEqnBuilder() const { return eqnBuilder; }
	virtual bool isPDESolver() { return false; }

protected:
	Variable     *var;
	EqnBuilder   *eqnBuilder;

};

#endif
