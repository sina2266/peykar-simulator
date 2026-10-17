/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef SIMULATIONEXPRESSION_H
#define SIMULATIONEXPRESSION_H

#include <VCELL/ChomboScheduler.h>
#include <vector>
#include <sstream>
using std::vector;
using std::stringstream;

class MembraneVariable;
class MembraneRegionVariable;
class VolumeRegionVariable;
class SymbolTable;
class VolumeVariable;
class PostProcessingBlock;
class SimTool;

struct TimeInterval
{
	double tstart, tstop, dt;
	int keepEvery;
	int istep, nsteps;

	TimeInterval(double a_tstart, double a_tstop, double a_dt, int a_keepEvery)
	{
		static double eps = 1e-12;
		tstart = a_tstart;
		tstop = a_tstop;
		dt = a_dt;
		keepEvery = a_keepEvery;
		istep = 0;
		double d = (tstop - tstart) / dt;
		nsteps = (int)round(d);
		if (abs((tstop - tstart)- nsteps * dt) > eps)
		{
			stringstream ss;
			ss << "["  << tstart << " " << tstop << "] interval length has to be a multiple of dt";
			throw ss.str();
		}
	}
	void nextStep()
	{
		++ istep;
	}
	bool ended()
	{
		return istep >= nsteps;
	}
	double getTime()
	{
		return tstart + istep * dt;
	}
	bool shouldSave()
	{
		return keepEvery > 0 && istep % keepEvery == 0;
	}
};

class SimulationExpression
{
public:
	SimulationExpression(SimTool* simTool);
	virtual ~SimulationExpression();

	void initSimulation();
	ChomboScheduler* getScheduler()
	{
		return _scheduler;
	}
	void setScheduler(ChomboScheduler *scheduler)
	{
		_scheduler = scheduler;
	}
	ChomboGeometry* getChomboGeometry()
	{
		return _scheduler->getChomboGeometry();
	}
	void iterate(bool bSolve=true);          // computes 1 time step

	double getTime_sec();
	void setCurrIteration(int curriter) { 
		currIteration = curriter; 
	}
	int getCurrIteration() { 
		return currIteration; 
	}
	double  getDT_sec() { 
		return timeIntervals[timeIntervalIndex].dt;
	}
	bool shouldSave()
	{
		return timeIntervals[timeIntervalIndex].shouldSave();
	}
	Variable* getVariable(int index);

	Variable *getVariableFromName(string& name);
	Variable *getVariableFromName(char* name);

	int getNumVariables() {
		return (int)varList.size();
	}
	
	SymbolTable* getSymbolTable() { 
		return symbolTable; 
	};
	
	void addVariable(Variable *var);

	double* getDiscontinuityTimes() { return discontinuityTimes; }
	void setDiscontinuityTimes(double* stopTimes) { discontinuityTimes=stopTimes; }

	int getNumVolVariables() { return volVarSize; }
	VolumeVariable* getVolVariable(int i) { return volVarList[i]; }

	int getNumMemVariables() { return memVarSize; }
	MembraneVariable* getMemVariable(int i) { return memVarList[i]; }

	int getNumVolRegionVariables() { return volRegionVarSize; }
	VolumeRegionVariable* getVolRegionVariable(int i) { return volRegionVarList[i]; }

	int getNumMemRegionVariables() { return memRegionVarSize; }
	MembraneRegionVariable* getMemRegionVariable(int i) { return memRegionVarList[i]; }

	int getNumMemPde() { return numMemPde; }
	int getNumVolPde() { return numVolPde; }

	void setHasTimeDependentDiffusionAdvection() { bHasTimeDependentDiffusionAdvection = true; }
	bool hasTimeDependentDiffusionAdvection() { return bHasTimeDependentDiffusionAdvection; }

	int getNumSymbols();

	char** getOutputVarNames();
	int getOutputVarCount();
	int* getOutputVarTypes();

	void setHasElliptic()
	{
		bHasElliptic = true;
	}
	bool hasElliptic()
	{
		return bHasElliptic;
	}
	void setHasParabolic()
	{
		bHasParabolic = true;
	}
	bool hasParabolic()
	{
		return bHasParabolic;
	}

	PostProcessingBlock* createPostProcessingBlock();
	PostProcessingBlock* getPostProcessingBlock();
	SimTool* getSimTool()
	{
		return simTool;
	}
	void addTimeInterval(TimeInterval& timeInterval)
	{
		timeIntervals.push_back(timeInterval);
	}
	void setHasFastSystem() { bHasFastSystem = true; }
	bool hasFastSystem() { return bHasFastSystem; }
	
private:
	int currIteration;  // first iteration is currIteration=0
  bool bHasElliptic;
	bool bHasParabolic;
	bool bHasFastSystem;

	ChomboScheduler  *_scheduler;
	vector<Variable*> varList;
	
	SymbolTable* symbolTable;
	int numSymbols;
	void createSymbolTable();

	double* discontinuityTimes;

	VolumeVariable** volVarList;
	int volVarSize;
	MembraneVariable** memVarList;
	int memVarSize;
	VolumeRegionVariable** volRegionVarList;
	int volRegionVarSize;
	MembraneRegionVariable** memRegionVarList;
	int memRegionVarSize;

	int numVolPde;
	int numMemPde;
	bool bHasTimeDependentDiffusionAdvection;

	char** outputVarNames;
	int outputVarCnt;
	int* outputVarTypes;

	PostProcessingBlock* postProcessingBlock;
	SimTool* simTool;

	vector<TimeInterval> timeIntervals;
	int timeIntervalIndex;
};

#endif
