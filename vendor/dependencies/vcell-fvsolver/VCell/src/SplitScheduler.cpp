/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/SplitScheduler.h>
#include <VCELL/Solver.h>
#include <VCELL/Simulation.h>
#include <VCELL/Feature.h>
#include <VCELL/Mesh.h>
#include <VCELL/Variable.h>
#include <VCELL/SimTool.h>

SplitScheduler::SplitScheduler(Simulation *Asim)
: SerialScheduler(Asim)
{
}

void SplitScheduler::iterate()
{

	//VCellModel *model = SimTool::getInstance()->getModel();

	//Solver *solver=NULL;
	//int volumeSize = sim->getMesh()->getNumVolumeElements();
	//int membraneSize = sim->getMesh()->getNumMembraneElements();
	//while (solver=sim->getNextSolver(solver)){
	//	if (solver->isPDESolver()){
	//		TimerHandle tHndBuild = SimTool::getInstance()->getTimerHandle(solver->getVar()->getName() + " Build");
	//		TimerHandle tHndSolve = SimTool::getInstance()->getTimerHandle(solver->getVar()->getName() + " Solve");
	//		if (!solver->initEqn(sim->getDT_sec(),0,volumeSize,0,membraneSize,bFirstTime)){
	//			throw "Simulation::iterate() - error init'ing equation";
	//		}
	//		SimTool::getInstance()->startTimer(tHndBuild);
	//		if (!solver->buildEqn(sim->getDT_sec(),0,volumeSize,0,membraneSize, bFirstTime)){
	//			throw "Simulation::iterate() - error building equation";
	//		}
	//		SimTool::getInstance()->stopTimer(tHndBuild);
	//		SimTool::getInstance()->startTimer(tHndSolve);
	//		if (!solver->solveEqn(sim->getDT_sec(),0,volumeSize,0,membraneSize, bFirstTime)){
	//			throw "Simulation::iterate() - error solving equation";
	//		}
	//		update();
	//		SimTool::getInstance()->stopTimer(tHndSolve);
	//	}	
	//}
	//while (solver=sim->getNextSolver(solver)){
	//	if (!(solver->isPDESolver())){
	//		TimerHandle tHndBuild = SimTool::getInstance()->getTimerHandle(solver->getVar()->getName() + " Build");
	//		TimerHandle tHndSolve = SimTool::getInstance()->getTimerHandle(solver->getVar()->getName() + " Solve");
	//		if (!solver->initEqn(sim->getDT_sec(),0,volumeSize,0,membraneSize,bFirstTime)){
	//			throw "Simulation::iterate() - error init'ing equation";             
	//		}
	//		SimTool::getInstance()->startTimer(tHndBuild);
	//		if (!solver->buildEqn(sim->getDT_sec(),0,volumeSize,0,membraneSize, bFirstTime)){
	//			throw "Simulation::iterate() - error building equation";
	//		}
	//		SimTool::getInstance()->stopTimer(tHndBuild);
	//		SimTool::getInstance()->startTimer(tHndSolve);
	//		if (!solver->solveEqn(sim->getDT_sec(),0,volumeSize,0,membraneSize, bFirstTime)){
	//			throw "Simulation::iterate() - error solving equation";
	//		}
	//		SimTool::getInstance()->stopTimer(tHndSolve);
	//	}	
	//}
	bFirstTime=false;
}
