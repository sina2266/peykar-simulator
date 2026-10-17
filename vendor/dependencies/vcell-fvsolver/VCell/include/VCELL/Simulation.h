/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef SIMULATION_H
#define SIMULATION_H

#include <vector>
#include <string>
using std::vector;
using std::string;

class Simulation;
class VCellModel;

class VolumeVariable;
class Variable;
class CartesianMesh;
class SimTool;
class Solver;
class Scheduler;
class PostProcessingBlock;

class Simulation
{
public:
	explicit Simulation(CartesianMesh *mesh);
	virtual ~Simulation();

	virtual void resolveReferences(SimTool* sim_tool);
	virtual void initSimulation(SimTool* sim_tool) = 0;   // initializes to t=0
	void iterate(SimTool* sim_tool);          // computes 1 time step
	virtual void update();           // copies new to old values 

	double getTime_sec(SimTool* sim_tool);
	void setCurrIteration(int curriter) { 
		currIteration = curriter; 
	}
	int getCurrIteration() const
	{
		return currIteration; 
	}
	double getDT_sec() const
	{
		return _dT_sec; 
	}
	void setDT_sec(double dT) { 
		_dT_sec = dT; 
	}
	virtual void advanceTimeOn();
	virtual void advanceTimeOff();

	virtual void writeData(const char *filename, bool bCompress)=0;
	virtual void readData(const char *filename);

	Variable* getVariable(int index);

	Variable *getVariableFromName(string& name);
	Variable *getVariableFromName(char* name);
	Solver   *getSolverFromVariable(Variable *var);
	CartesianMesh *getMesh() const
	{
		return _mesh; 
	}

	Solver* getSolver(int index);

	int getNumVariables() const
	{
		return static_cast<int>(varList.size());
	}
	int getNumSolvers() const
	{
		return static_cast<int>(solverList.size());
	}
	void addVariable(Variable *var);
	void addSolver(Solver *solver);
	void setSimStartTime(SimTool* sim_tool, double st);

	virtual void createPostProcessingBlock()=0;
	PostProcessingBlock* getPostProcessingBlock() const
	{
		return postProcessingBlock;
	}

protected:
	int currIteration;  // first iteration is currIteration=0

	double _dT_sec;                  // seconds
	Scheduler  *_scheduler;
	vector<Solver*> solverList;
	vector<Variable*> varList;
	//vector<Particle*> globalParticleList; 
	CartesianMesh *_mesh;
	bool _advanced;
	bool _initEquations;

	PostProcessingBlock* postProcessingBlock;
};

#endif
