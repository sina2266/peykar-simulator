/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/Variable.h>
#include <VCELL/Solver.h>
#include <VCELL/Simulation.h>
#include <VCELL/SerialScheduler.h>
#include <VCELL/SimTool.h>
#include <VCELL/CartesianMesh.h>

SerialScheduler::SerialScheduler(Simulation *Asim)
: Scheduler(Asim)
{
}
   
void SerialScheduler::iterate(SimTool* sim_tool)
{
	
	/*
	Contour *contour = NULL;
	int numContours = model->getNumContours();
	for(int i=0; i<numContours; i++){
		contour = model->getContour(i);
		ContourParticleContext *cpc = NULL;
		while (cpc = contour->getNextContourParticleContext(cpc)){
		cpc->releaseParticles();
		}
	}

	for(i=0; i<numContours; i++){
		contour = model->getContour(i);
		ContourParticleContext *cpc = NULL;
		while (cpc = contour->getNextContourParticleContext(cpc)){
		cpc->captureParticles();
		}
	}
	
	for (int i = 0; i < model->getNumFeatures(); i ++) {
		Feature* feature = model->getFeatureFromIndex(i);
		VolumeParticleContext *vpc = feature->getVolumeParticleContext();
		ContourParticleContext *cpc = feature->getContourParticleContext();
		if(vpc != NULL){
			vpc->react();
		}

		if(cpc != NULL){
			cpc->releaseParticles();
			cpc->captureParticles();
		}

	}
	for (int i = 0; i < model->getNumFeatures(); i ++) {
		Feature* feature = model->getFeatureFromIndex(i);
		VolumeParticleContext *vpc = feature->getVolumeParticleContext();
		ContourParticleContext *cpc = feature->getContourParticleContext();
		if(vpc != NULL){
			vpc->move();
		}
		if(cpc != NULL){
			cpc->move();
		}
	}
	*/

	Solver *solver=NULL;
	int volumeSize = sim->getMesh()->getNumVolumeElements(); 
	int membraneSize = sim->getMesh()->getNumMembraneElements(); 
	for (int i = 0; i < sim->getNumSolvers(); i ++) {
		solver = sim->getSolver(i);
		string timername = solver->getVar()->getName() + " Build";
		TimerHandle tHndBuild = sim_tool->getTimerHandle(timername);
		timername = solver->getVar()->getName() + " Solve";
		TimerHandle tHndSolve = sim_tool->getTimerHandle(timername);
		//
		// initialize equations first time around
		//
		solver->initEqn(sim_tool->getModel(), sim->getDT_sec(),0,volumeSize,0,membraneSize, bFirstTime);

		sim_tool->startTimer(tHndBuild);
		solver->buildEqn(sim, sim->getDT_sec(),0,volumeSize,0,membraneSize, bFirstTime);
		sim_tool->stopTimer(tHndBuild);

		sim_tool->startTimer(tHndSolve);
		solver->solveEqn(sim_tool, sim->getDT_sec(),0,volumeSize,0,membraneSize, bFirstTime);
		sim_tool->stopTimer(tHndSolve);
	}
	if(hasFastSystem(sim_tool->getModel())){
		Mesh *mesh = sim->getMesh();
		solveFastSystem(sim_tool, 0, mesh->getNumVolumeElements(), 0, mesh->getNumMembraneElements());
	}
	bFirstTime=false;
}
