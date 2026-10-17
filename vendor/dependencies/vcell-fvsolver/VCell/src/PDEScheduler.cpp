/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifdef VCELL_MPI
#include "mpi.h"
#endif

#include <VCELL/PDEScheduler.h>
#include <VCELL/Variable.h>
#include <VCELL/Solver.h>
#include <VCELL/Simulation.h>
#include <VCELL/Scheduler.h>
#include <VCELL/Feature.h>
#include <VCELL/ParticleContext.h>
#include <VCELL/VCellModel.h>
#include <VCELL/FastSystem.h>
#include <VCELL/VolumeRegion.h>
#include <VCELL/MembraneRegion.h>
#include <VCELL/SimTool.h>
#include <VCELL/CartesianMesh.h>

PDEScheduler::PDEScheduler(Simulation *Asim)
: Scheduler(Asim)
{
#ifdef VCELL_MPI
	MPI_Comm_rank(MPI_COMM_WORLD, &mpiRank);
	MPI_Comm_size(MPI_COMM_WORLD, &mpiSize);
#endif
}

   
void PDEScheduler::iterate()
{
	int volumeSize = sim->getMesh()->getNumVolumeElements();
	int membraneSize = sim->getMesh()->getNumMembraneElements();

#ifdef VCELL_MPI
   int PDEcount = 0;
#endif
	Solver *solver=NULL;
	while (solver=sim->getNextSolver(solver)){
		TimerHandle tHndBuild = SimTool::getInstance()->getTimerHandle(solver->getVar()->getName() + " Build");
		TimerHandle tHndSolve = SimTool::getInstance()->getTimerHandle(solver->getVar()->getName() + " Solve");
		if (!solver->initEqn(sim->getDT_sec(),0,volumeSize,0,membraneSize,bFirstTime)){
			throw "Simulation::iterate() - error init'ing equation";
		}
#ifdef VCELL_MPI
		if (solver->isPDESolver()){
			int workerForVariable = PDEcount++ % mpiSize;
			if (workerForVariable != mpiRank){
				continue;
			}
		}
#endif
		SimTool::getInstance()->startTimer(tHndBuild);
		if (!solver->buildEqn(sim->getDT_sec(),0,volumeSize,0,membraneSize, bFirstTime)){
			throw "Simulation::iterate() - error building equation";
		}
		SimTool::getInstance()->stopTimer(tHndBuild);
		SimTool::getInstance()->startTimer(tHndSolve);
		if (!solver->solveEqn(sim->getDT_sec(),0,volumeSize,0,membraneSize, bFirstTime)){
			throw "Simulation::iterate() - error solving equation";
		}
		SimTool::getInstance()->stopTimer(tHndSolve);
	}
	bFirstTime=false;
}

void PDEScheduler::update()
{
#ifdef VCELL_MPI
	//
	// exchange data
	//   
	Solver *solver=NULL;
	int PDEcount=0;
	while (solver=sim->getNextSolver(solver)){
		if (solver->isPDESolver()){
			int workerForVariable = PDEcount++ % mpiSize;
			Variable *var = solver->getVar();
			MPI_Bcast(var->getCurr(),var->getSize(),MPI_DOUBLE,workerForVariable,MPI_COMM_WORLD);
		}
	}
#endif
  
	if(hasFastSystem()){
		Mesh *mesh = sim->getMesh();
		solveFastSystem(0, mesh->getNumVolumeElements(), 0, mesh->getNumMembraneElements());
	}

	Scheduler::update();
}

void PDEScheduler::collectResults(int processRank)
{
   Scheduler::collectResults(processRank);
}
