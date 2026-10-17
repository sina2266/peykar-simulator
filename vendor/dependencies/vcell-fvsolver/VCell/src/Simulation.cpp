/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/SimTypes.h>
#include <VCELL/Element.h>
#include <VCELL/Variable.h>
#include <VCELL/Solver.h>
#include <VCELL/Simulation.h>
#include <VCELL/FVDataSet.h>
#include <VCELL/VCellModel.h>
#include <VCELL/SimTool.h>
#include <VCELL/SundialsPdeScheduler.h>
#include <VCELL/PostProcessingBlock.h>

Simulation::Simulation(CartesianMesh *mesh)
{
	ASSERTION(mesh);
	//
	// initialize size of voxel array to 0
	//
	_dT_sec = 0;   // seconds
	currIteration = 0;
	_mesh = mesh;
	_advanced = false;
	_initEquations = false;
	_scheduler = NULL;
	//globalParticleList.clear();
}

Simulation::~Simulation()
{
	for (int i = 0; i < (int)varList.size(); i ++) {
		delete varList[i];
	}
	varList.clear();
	for (int i = 0; i < (int)solverList.size(); i ++) {
		delete solverList[i];
	}
	solverList.clear();
	delete _scheduler;
}

void Simulation::advanceTimeOn() {
	if (_advanced) {
		throw "Simulation::advanceTimeOn() : time has already been advanced on";
	}
	_advanced=true;
}

void Simulation::advanceTimeOff() {
	if (!_advanced) {
		throw "Simulation::advanceTimeOff() : time has already been advanced off";
	}
	_advanced=false;
}

void Simulation::iterate(SimTool* sim_tool)
{
	_scheduler->iterate(sim_tool);
}

void Simulation::update()
{
	for (int i = 0; i < (int)varList.size(); i ++) {
		Variable* var = varList[i];
		var->update();
	}
	currIteration ++;
}

void Simulation::addSolver(Solver *solver)
{
	solverList.push_back(solver);
}

Solver *Simulation::getSolver(int index)
{
	if (index < 0 || index >= (int)solverList.size()) {
		throw "Simulation: getSolver(index) : index out of bounds";
	}
	return solverList.at(index);
}

void Simulation::addVariable(Variable *var)
{
	varList.push_back(var);
}

Variable* Simulation::getVariable(int index) {
	if (index < 0 || index >= (int)varList.size()) {
		throw "Simulation: getVariable(index) : index out of bounds";
	}
	return varList.at(index);
}

Variable *Simulation::getVariableFromName(string& varName)
{
	ASSERTION(varName);
	for (int i = 0; i < (int)varList.size(); i ++) {
		Variable* var = varList[i];
		if (varName == var->getName()){
			return var;
		}
	}
	return NULL;
}

Variable *Simulation::getVariableFromName(char* varName)
{
	string vn(varName);
	return getVariableFromName(vn);
}

Solver *Simulation::getSolverFromVariable(Variable *var)
{
   ASSERTION(var);
   for (int i = 0; i < (int)solverList.size(); i ++) {
		Solver* solver = solverList[i];
		if (solver->getVar()==var){
			return solver;
		}
   }
   return NULL;
}

//void Simulation::synchronize()
//{
//	//
//	// if applicable, collect results into root process
//	//
//	_scheduler->collectResults(0);
//}

//void Simulation::addParticle(Particle *particle)
//{
//	globalParticleList.push_back(particle);
//}

void Simulation::readData(const char *filename)
{
	//
	// all processes read data
	// (in the future, root could read and then distribute)
	//
	FVDataSet::read(filename, this);
}

//-------------------------------------------------------
// determines scheduler and resets _time_sec and initializes var's
//-------------------------------------------------------
void Simulation::resolveReferences(SimTool* sim_tool) {
	sim_tool->getModel()->resolveReferences(sim_tool->getSimulation());
	if (postProcessingBlock != NULL) {
		postProcessingBlock->resolveReferences();
	}
}

double Simulation::getTime_sec(SimTool* sim_tool) {
	if (sim_tool->isSundialsPdeSolver()) {
		return ((SundialsPdeScheduler*)_scheduler)->getCurrentTime();
	}
	return _advanced ? (currIteration + 1) * _dT_sec : currIteration * _dT_sec;
}

void Simulation::setSimStartTime(SimTool* sim_tool, double st) {
	if (sim_tool->isSundialsPdeSolver()) {
		((SundialsPdeScheduler*)_scheduler)->setSimStartTime(st);
	}
}
