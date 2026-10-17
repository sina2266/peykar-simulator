/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifdef WIN32
#define _HAS_STD_BYTE 0
#include <Windows.h>
#else
#include <UnixDefs.h>
#endif

#ifdef VCELL_MPI
#include "mpi.h"
#endif

#include <stdlib.h>

#include <VCELL/SimTypes.h>
#include <VCELL/App.h>
#include <VCELL/Mesh.h>
#include <VCELL/Variable.h>
#include <VCELL/Solver.h>
#include <VCELL/Simulation.h>
#include <VCELL/DomainPDEScheduler.h>
#include <VCELL/Feature.h>
#include <VCELL/ParticleContext.h>
#include <VCELL/VCellModel.h>

#define MIN(a,b)   (((a)<(b))?(a):(b))
#define MAX(a,b)   (((a)<(b))?(b):(a))

//----------------------------------------------------------------------------
//
// class DomainPDEScheduler
//
//----------------------------------------------------------------------------
DomainPDEScheduler::DomainPDEScheduler(Simulation *Asim)
: Scheduler(Asim)
{
	numPDEworkers = 0;
	numODEworkers = 0;
}

DomainPDEScheduler::~DomainPDEScheduler()
{
}

   
void DomainPDEScheduler::iterate()
{
	worker->iterate();
}


void DomainPDEScheduler::update()
{
	worker->update();
	  
	Scheduler::update();
}

void DomainPDEScheduler::collectResults(int processRank)
{
	worker->collectResults();
}

boolean DomainPDEScheduler::initValues()
{
	if (!Scheduler::initValues()){
		return FALSE;
	}
#ifdef VCELL_MPI
	//printf("DomainPDEScheduler::initValues(rank=%d),  begin\n", mpiRank);
	numPDEworkers = getNumPDEs();
	numODEworkers = mpiSize - numPDEworkers;
	if (numODEworkers < 1){
		printf("DomainPDEScheduler, bad choice of scheduler (numPDEworkers + 2) > numWorkers\n");
		return FALSE;
	}
   
	if (mpiRank < numPDEworkers){
		//
		// create a PDEworker
		//
		PDESolver *pdeSolver = getPDESolver(mpiRank);
		assert(pdeSolver);
		if (mpiRank == 0){
			worker = new PDEWorkerRoot(mpiRank, sim, this, pdeSolver);  // root of MPI_COMM_WORLD
		}else{
			worker = new PDEWorker(mpiRank, sim, this, pdeSolver);
		}
	}else{
		//
		// create a ODEworker
		//
		worker = new ODEWorker(mpiRank, sim, this);
		ASSERTION(mpiRank>=getODEWorkerRank(0));
	}
#endif
	worker->init();
	return TRUE;
}

int DomainPDEScheduler::getNumPDEs()
{
	int count = 0;
	Solver *solver=NULL;
	while (solver=sim->getNextSolver(solver)){
		if (solver->isPDESolver()){
			count++;
		}
	}
	return count;
}

PDESolver *DomainPDEScheduler::getPDESolver(int worldRank)
{
#ifdef VCELL_MPI
	int PDERank = 0;
	Solver *solver=NULL;
	while (solver=sim->getNextSolver(solver)){
		if (solver->isPDESolver()){
			if (mpiRank == PDERank){
				return (PDESolver*)solver;
			}
			PDERank++;
		}
	}
#endif
	return NULL;
}

int DomainPDEScheduler::getPDEWorkerRank(PDESolver *pdeSolver)
{
#ifdef VCELL_MPI
	//
	// ODE workers first
	//
	int PDErank = 0;
	Solver *solver=NULL;
	while (solver=sim->getNextSolver(solver)){
		if (solver->isPDESolver()){
			if (solver == pdeSolver){
			assert(PDErank < mpiSize);
			return PDErank;
		}
		PDErank++;
		}
	}
	assert(0);
#endif
	return -1;
}

//-----------------------------------------------------------------
// assume N PDE's
//        M workers
//
//   workers [1...N-1] are PDE workers,  [N...M-1] are ODE workers
//
//-----------------------------------------------------------------
int DomainPDEScheduler::getODEWorkerRank(int partitionNumber)
{
#ifdef VCELL_MPI
	return numPDEworkers + partitionNumber;
#else
	return -1;
#endif
}

int DomainPDEScheduler::getPartitionNumber(int odeWorkerRank)
{
	int partitionNumber = odeWorkerRank - numPDEworkers;
	ASSERTION(partitionNumber>=0 && partitionNumber<numODEworkers);
	return partitionNumber;
}

//----------------------------------------------------------------------------
//
// class Partition
//
//----------------------------------------------------------------------------
Partition::Partition()
{
	volComputeStart=0;
	volComputeSize =0;
	memComputeStart=0;
	memComputeSize =0;
	volDataStart   =0;
	volDataSize    =0;
	memDataStart   =0;
	memDataSize    =0;
}

void Partition::setVolumeCompute(int start, int size)
{
	volComputeStart=start;
	volComputeSize =size;  
}

void Partition::setVolumeData(int start, int size)
{
	volDataStart=start;
	volDataSize =size;      
}

void Partition::setMembraneCompute(int start, int size)
{
	memComputeStart=start;
	memComputeSize =size;      
}

void Partition::setMembraneData(int start, int size)
{
	memDataStart=start;
	memDataSize =size;      
}

void Partition::show(char *buffer)
{
	sprintf(buffer, "volumeCompute (%d,%d)<%d> volumeData (%d,%d)<%d>  membraneCompute (%d,%d)<%d>  membraneData (%d,%d)<%d>\n", 
   					volComputeStart, volComputeSize+volComputeStart-1, volComputeSize, 
					volDataStart,    volDataSize+volDataStart-1,       volDataSize, 
					memComputeStart, memComputeSize+memComputeStart-1, memComputeSize, 
					memDataStart,    memDataSize+memDataStart-1,       memDataSize);
}

//----------------------------------------------------------------------------
//
// class CartesianSubDomain
//
//----------------------------------------------------------------------------
CartesianSubDomain::CartesianSubDomain(CartesianMesh *Amesh,
                                       int AnumPartitions)
{
	numPartitions = AnumPartitions;
	mesh = Amesh;
	    
	ASSERTION(numPartitions > 0);
	    
	int numX = mesh->getNumVolumeX();
	int numY = mesh->getNumVolumeY();
	int numZ = mesh->getNumVolumeZ();
	int dim = mesh->getDimension();
	int MAX_INTERPOLATION_NEAR_MEMBRANE = 2;  //??????
	int sizeofBoundaryLayer=0;
	switch (dim){
		case 1:{  
			sizeofBoundaryLayer = 1; 
			break;
		}
		case 2:{
			sizeofBoundaryLayer = MAX_INTERPOLATION_NEAR_MEMBRANE*numX;
			break;
		}
		case 3:{
			sizeofBoundaryLayer = MAX_INTERPOLATION_NEAR_MEMBRANE*numX*numY;
			break;
		}
		default: assert(0);
	}
    
	partitions = new Partition[numPartitions];
	   
	if (numPartitions==1){
		partitions[0].setVolumeCompute(0, mesh->getNumVolumeElements());
		partitions[0].setVolumeData(0, mesh->getNumVolumeElements());
		partitions[0].setMembraneCompute(0, mesh->getNumMembraneElements());
		partitions[0].setMembraneData(0, mesh->getNumMembraneElements());
	}else{
		//
		// calculate the volume elements within all partitions
		//
		int numVol = mesh->getNumVolumeElements();
		int integerSize = numVol/numPartitions;
		int remainderSize = numVol%numPartitions;
		int volStart = 0;
		int partition = 0;
		for (partition=0;partition<=numPartitions;partition++){
			int volSize = integerSize;
			if (partition == 0){
				volSize = integerSize + remainderSize;   // put extra offset only in root process
			}
			int endBoundary = 0;
			if (partition<(numPartitions-1)){
				endBoundary = sizeofBoundaryLayer;
			}
			int startBoundary = 0;
			if (partition>0){
				startBoundary = sizeofBoundaryLayer;
			}
			partitions[partition].setVolumeData(volStart, volSize);
			partitions[partition].setVolumeCompute(MAX(0, volStart - startBoundary),
													MIN(mesh->getNumVolumeElements()-1, 
									volSize + startBoundary + endBoundary));
			volStart += volSize;
		}
		//
		// calculate the span of all membrane elements within all partitions
		//
		for (partition=0;partition<=numPartitions;partition++){
			int numMem = mesh->getNumMembraneElements();
			int firstRef = numMem;   // past end of array
			int lastRef = -1;        // before start of array
			int volStart = partitions[partition].getVolumeComputeStart();
			int volSize = partitions[partition].getVolumeComputeSize();
			int volEnd = volStart+volSize-1;
			MembraneElement *pMemElement = mesh->getMembraneElements();
			for (int i=0;i<numMem;i++){
				int inside = pMemElement->insideIndexNear;
   				int outside = pMemElement->outsideIndexNear;
				if ((inside >= volStart)  && (inside <= volEnd) &&
						(outside >= volStart) && (outside <= volEnd)) {
					firstRef = MIN(firstRef, i);
					lastRef  = MAX(lastRef, i);
				}
				pMemElement++;
			}
			int memStart;
			int memSize;
			if (lastRef<firstRef){
				memStart = 0;
				memSize = numMem;
			}else{
				memStart = firstRef;
				memSize = lastRef - firstRef + 1;	     
			}
			partitions[partition].setMembraneCompute(memStart, memSize);
			partitions[partition].setMembraneData(memStart, memSize);
		}
	}
}

Partition *CartesianSubDomain::getPartition(int partitionNumber)
{
	ASSERTION(partitionNumber>=0);
	ASSERTION(partitionNumber<numPartitions);
	return partitions + partitionNumber;
}

//========================================================================
//
//  class Worker
//
//========================================================================
const int Worker::ODE_COLLECT_TAG = 1;
const int Worker::PDE_COLLECT_TAG = 2;
const int Worker::ODE_UPDATE_TAG = 3;
const int Worker::PDE_UPDATE_TAG = 4;
const int Worker::PDE_SOURCE_TAG = 5;

Worker::Worker(int AworldRank, Simulation *Asim, DomainPDEScheduler *Ascheduler)
{
	worldRank = AworldRank;
	sim = Asim;
	scheduler = Ascheduler;
	commBuffer = NULL;
	bInitEquations = FALSE;
	subDomain = NULL;
}

Worker::~Worker()
{
	if (commBuffer) { delete[] commBuffer; }
	if (subDomain) { delete[] subDomain; }
}

boolean Worker::init()
{
	commBuffer = new double[3*sim->getMesh()->getNumVolumeElements()];
	return TRUE;
}
//========================================================================
//
//  class ODEWorker
//
//========================================================================
ODEWorker::ODEWorker(int AworldRank, Simulation *Asim, DomainPDEScheduler *Ascheduler)
: Worker(AworldRank,Asim,Ascheduler)
{
	partitionNumber = scheduler->getPartitionNumber(AworldRank);
}

boolean ODEWorker::iterate()
{
#ifdef VCELL_MPI
	//printf("          ODEWorker::iterate()[worldRank=%d]\n",getWorldRank());
	//
	// first solve for all PDE source terms
	//
	Partition *partition = subDomain->getPartition(partitionNumber);
	Solver *solver=NULL;
	while (solver=sim->getNextSolver(solver)){
		if (solver->isPDESolver()){
			//
			// build source terms for PDE
			//
			if (!solver->buildEqn(sim->getDT_sec(),
						partition->getVolumeComputeStart(),
						partition->getVolumeComputeSize(),
						partition->getMembraneComputeStart(), 
						partition->getMembraneComputeSize())){
				printf("DomainPDEScheduler::iterate() - error building equation\n");
				return FALSE;
			}
			//
			// pack source terms into a buffer and send to the PDE worker
			//
			double *pBuff = commBuffer;
			int size = partition->getVolumeDataSize();
			int i;
			double *old = solver->getVar()->getOld();
			old += partition->getVolumeDataStart();
			for (i=0;i<size;i++){
				*pBuff++ = *old++;
			}
			double *curr = solver->getVar()->getCurr();
			curr += partition->getVolumeDataStart();
			for (i=0;i<size;i++){
				*pBuff++ = *curr++;
			}
			DiscreteEqn *eqn = ((PDESolver *)solver)->getEqns();
			eqn += partition->getVolumeDataStart();
			for (i=0;i<size;i++){
				*pBuff++ = (eqn++)->B;
			}
			int pdeWorker = scheduler->getPDEWorkerRank((PDESolver*)solver);
			//printf("          ODEWorker::iterate()[partition=%d worldRank=%d] sending %d source terms 'doubles' to PDE worker %d\n",
			//partitionNumber,getWorldRank(),size,pdeWorker);
			MPI_Send(commBuffer,
				3*size,        //?????????????????????????????????? ? ? ? ? ? ? ? ? ?
				MPI_DOUBLE,
				pdeWorker,
				PDE_SOURCE_TAG,
				MPI_COMM_WORLD);
		}
	}
	//
	// next, solve for all ODEs
	//
	solver=NULL;
	while (solver=sim->getNextSolver(solver)){
		if (!solver->isPDESolver()){
			if (!solver->buildEqn(sim->getDT_sec(),
						partition->getVolumeComputeStart(),
						partition->getVolumeComputeSize(),
						partition->getMembraneComputeStart(), 
						partition->getMembraneComputeSize())){
				printf("ODEWorker::iterate() - error building equation\n");
				return FALSE;
			}
			if (!solver->solveEqn(sim->getDT_sec(),
						partition->getVolumeComputeStart(),
						partition->getVolumeComputeSize(),
						partition->getMembraneComputeStart(), 
						partition->getMembraneComputeSize())){
				printf("ODEWorker::iterate() - error solving equation\n");
				return FALSE;
			}
		}
	}  
	//printf("          ODEWorker::iterate()[worldRank=%d] COMPLETED ITERATION\n",getWorldRank());
#endif
	return TRUE;
}

boolean ODEWorker::init()
{
	//printf("ODEWorker::init(), entering\n");
	if (!Worker::init()){
		printf("ODEWorker::init(), Worker::init() returned false\n");
		return FALSE;
	}
   
#ifdef VCELL_MPI
       
	//
	// find which subdomain this worker will solve
	//
	CartesianMesh *mesh = (CartesianMesh *)sim->getMesh();
	subDomain = new CartesianSubDomain(mesh, scheduler->getNumPartitions());

	//????
	Partition *partition = subDomain->getPartition(partitionNumber);
	char buffer[1000];
	partition->show(buffer);
	printf("\nODEWorker::init(), partitionNumber=%d, numPartitions=%d  %s\n", 
	partitionNumber, scheduler->getNumPartitions(), buffer);
	//?????

#endif
	//printf("ODEWorker::init(), leaving\n");
	return TRUE;
}

void ODEWorker::update()
{
#ifdef VCELL_MPI
	//printf("          ODEWorker::update()[worldRank=%d]\n",getWorldRank());
	MPI_Status status;
	//
	// collect results from PDEs
	//
	Partition *partition = subDomain->getPartition(partitionNumber);
	Solver *solver=NULL;
	while (solver=sim->getNextSolver(solver)){
		Variable *var = solver->getVar();
		if (solver->isPDESolver()){
			int pdeWorker = scheduler->getPDEWorkerRank((PDESolver*)solver);
			//printf("          ODEWorker::update()[partitionNumber=%d worldRank=%d] WAITING FOR <%d> of variable %s from PDE worker %d\n",
			//partitionNumber,getWorldRank(),partition->getVolumeComputeSize(),var->getName(),pdeWorker);
			MPI_Recv(var->getCurr()+partition->getVolumeComputeStart(),
						partition->getVolumeComputeSize(),
						MPI_DOUBLE,
						pdeWorker,
						PDE_UPDATE_TAG,
						MPI_COMM_WORLD,
						&status);
			//printf("          ODEWorker::update()[partitionNumber=%d worldRank=%d] received <%d> of variable %s from PDE worker %d\n",
			//partitionNumber,getWorldRank(),partition->getVolumeComputeSize(),var->getName(),pdeWorker);
		}
	}
	//printf("          ODEWorker::update()[worldRank=%d] COMPLETED UPDATE\n",getWorldRank());
	if(scheduler->hasFastSystem()){
		int volStart = partition->getVolumeComputeStart();
		int volSize =  partition->getVolumeComputeSize();
		int memStart = partition->getMembraneComputeStart();
		int memSize = partition->getMembraneComputeSize();
		if(!scheduler->solveFastSystem(volStart,volSize,memStart,memSize)){
			printf("ODEWorker::update() - error solving FastSystem\n");
		}
	}

#endif
}

void ODEWorker::collectResults()
{
#ifdef VCELL_MPI
	int rc;
	CartesianMesh *mesh = (CartesianMesh *)sim->getMesh();
	Partition *partition = subDomain->getPartition(partitionNumber);
	Solver *solver = NULL;
	while (solver=sim->getNextSolver(solver)){
		//
		// send ODE results to root
		//
		if (!solver->isPDESolver()){
			int start, size;
			Variable *var = solver->getVar();
			if (var->isVolumeVar()){
				start = partition->getVolumeDataStart();
				size = partition->getVolumeDataSize();
			}else{
				start = partition->getMembraneDataStart();
				size = partition->getMembraneDataSize();
			}
			rc = MPI_Send(var->getCurr()+start, 
				size, 
				MPI_DOUBLE, 
				0, 
				ODE_COLLECT_TAG, 
				MPI_COMM_WORLD);
			if (rc != MPI_SUCCESS){
				printf("ODEWorker(%d)::collectResults(), MPI_Send returned rc=%d\n",getWorldRank(),rc);
			}
		}
	}
#endif
}

//========================================================================
//
//  class PDEWorker
//
//========================================================================
PDEWorker::PDEWorker(int AworldRank, Simulation *Asim, DomainPDEScheduler *Ascheduler, PDESolver *ApdeSolver)
: Worker(AworldRank,Asim,Ascheduler)
{
	pdeSolver = ApdeSolver;
}

boolean PDEWorker::iterate()
{
#ifdef VCELL_MPI
	//printf("PDEWorker::iterate()[worldRank=%d]\n",getWorldRank());
	int volumeSize = sim->getMesh()->getNumVolumeElements();
	int membraneSize = sim->getMesh()->getNumMembraneElements();
	MPI_Status status;
	if (!bInitEquations){
		if (!pdeSolver->initEqn(sim->getDT_sec(),0,volumeSize,0,membraneSize)){
			printf("DomainPDEScheduler::iterate() - error init'ing equation\n");
			return FALSE;
		}
		bInitEquations=TRUE;
	}
	TimerHandle tHndSourceComm;
	TimerHandle tHndSolveMatrix;
	if (getWorldRank()==0){
		tHndSourceComm = sim->getTimerHandle("PDE Source Comm");
		tHndSolveMatrix = sim->getTimerHandle("PDE Solve Matrix");
	}

	//
	// receive all source terms, so ODEs can continue
	//
	if (getWorldRank()==0)    sim->startTimer(tHndSourceComm);
	for (int i=0;i<subDomain->getNumPartitions();i++){
		Partition *partition = subDomain->getPartition(i);
		int odeWorker = scheduler->getODEWorkerRank(i);
		//printf("PDEWorker::iterate()[worldRank=%d] WAITING FOR %d source terms from ODE worker[worldRank=%d]\n",
		//getWorldRank(),partition->getVolumeDataSize(),odeWorker);
		MPI_Recv(commBuffer+partition->getVolumeDataStart(),
			3*partition->getVolumeDataSize(),   //?????????????????????????????????????????????????????????????????????????????
			MPI_DOUBLE,
			odeWorker,
			PDE_SOURCE_TAG,
			MPI_COMM_WORLD,
			&status);
		//printf("PDEWorker::iterate()[worldRank=%d] received %d source terms from ODE worker[worldRank=%d]\n",
		//getWorldRank(),partition->getVolumeDataSize(),odeWorker);
	}
	if (getWorldRank()==0)    sim->stopTimer(tHndSourceComm);
         
	//
	// unpack initial values and source terms into equation structures
	//
	//  first N = variable value (updated for fast system)
	//  last N  = rates
	//
	double *pBuff = commBuffer;
	double *old = pdeSolver->getVar()->getOld();
	for (i=0;i<volumeSize;i++){
		*old++ = *pBuff++;
	}  
	double *curr = pdeSolver->getVar()->getCurr();
	for (i=0;i<volumeSize;i++){
		*curr++ = *pBuff++;
	}  
	DiscreteEqn *eqn = pdeSolver->getEqns();
	for (i=0;i<volumeSize;i++){
		(eqn++)->B = *pBuff++;
	}  

	/*?????
	if (getWorldRank()==0){
	DiscreteEqn *eqn = pdeSolver->getEqns();
	char buffer[10000];
	char tmpBuf[200];
	sprintf(buffer, "source terms for PDEWorker(%d)\n", getWorldRank());
	for (i=0;i<volumeSize;i++){   
	//sprintf(tmpBuf,"%d)  Ap=B=%lf\n", i, eqn[i].B);
	eqn[i].show(tmpBuf);
	strcat(buffer,tmpBuf);
	}printf(buffer);}
	return FALSE;
	*/

	//
	// solve matrix
	//
	if (getWorldRank()==0) sim->startTimer(tHndSolveMatrix);
	if (!pdeSolver->solveEqn(sim->getDT_sec(),0,volumeSize,0,membraneSize)){
		printf("Simulation::iterate() - error solving equation\n");
		return FALSE;
	}
	if (getWorldRank()==0) sim->stopTimer(tHndSolveMatrix);
	//printf("PDEWorker::iterate()[worldRank=%d] COMPLETED ITERATION\n",getWorldRank());
#endif
	return TRUE;
}


boolean PDEWorker::init()
{
   if (!Worker::init()){
       printf("PDEWorker::init(), Worker::init() returned false\n");
       return FALSE;
   }
   
   //
   // get copy of subDomain for use in determining partition sizes for communication
   //
   CartesianMesh *mesh = (CartesianMesh *)sim->getMesh();
   subDomain = new CartesianSubDomain(mesh, scheduler->getNumPartitions());

   return TRUE;
}



void PDEWorker::update()
{
#ifdef VCELL_MPI
	//printf("PDEWorker::update()[worldRank=%d]\n",getWorldRank());
	//
	// send results to ODEs
	//
	Variable *var = pdeSolver->getVar();
	for (int i=0;i<subDomain->getNumPartitions();i++){
		Partition *partition = subDomain->getPartition(i);
		int odeWorker = scheduler->getODEWorkerRank(i);
		//printf("PDEWorker::update()[worldRank=%d] sending <%d> var %s to ODE worker[worldRank=%d]\n",
		//getWorldRank(),partition->getVolumeComputeSize(),var->getName(),odeWorker);
		MPI_Send(var->getCurr()+partition->getVolumeComputeStart(), 
			partition->getVolumeComputeSize(),
			MPI_DOUBLE,
			odeWorker,
			PDE_UPDATE_TAG,
			MPI_COMM_WORLD);
	}     
	//printf("PDEWorker::update()[worldRank=%d] COMPLETE \n",getWorldRank());
#endif
}


void PDEWorker::collectResults()
{
#ifdef VCELL_MPI
	int rc;
	Variable *var = pdeSolver->getVar();
	rc = MPI_Send(var->getCurr(), 
		var->getSize(), 
		MPI_DOUBLE, 
		0, 
    	PDE_COLLECT_TAG, 
		MPI_COMM_WORLD); 
	if (rc != MPI_SUCCESS){
		printf("PDEWorker(%d)::collectResults(), MPI_Send returned rc=%d\n",getWorldRank(),rc);
	}
#endif
}

//========================================================================
//
//  class PDEWorkerRoot
//
//========================================================================
PDEWorkerRoot::PDEWorkerRoot(int AworldRank, Simulation *Asim, DomainPDEScheduler *Ascheduler, PDESolver *ApdeSolver)
: PDEWorker(AworldRank,Asim,Ascheduler,ApdeSolver)
{
}

void PDEWorkerRoot::collectResults()
{
#ifdef VCELL_MPI
	int rc;
	MPI_Status status;
	//
	// gather PDE results
	//
	Solver *solver = NULL;
	while (solver=sim->getNextSolver(solver)){
		if (solver->isPDESolver()){
			//
			// collect PDE results
			// 
			if (solver == pdeSolver){
			//
			// root worker doesn't have to collect it's own work
			//
			//	    printf("PDEWorkerRoot::collectResults(), root worker doesn't have to collect it's own work\n");
			}else{
				int worker = scheduler->getPDEWorkerRank((PDESolver *)solver);
				rc = MPI_Recv(solver->getVar()->getCurr(), 
					solver->getVar()->getSize(), 
					MPI_DOUBLE, 
					worker, 
					PDE_COLLECT_TAG, 
					MPI_COMM_WORLD,
					&status);
				if (rc != MPI_SUCCESS){
					printf("PDEWorkerRoot(%d)::collectResults(), getting PDE results from worker %d, MPI_Recv rc=%d\n",getWorldRank(),worker,rc);
				} 
			}
		}
	}
	solver = NULL;
	while (solver=sim->getNextSolver(solver)){
		if (!solver->isPDESolver()){
			//
			// collect ODE results
			// 
			for (int i=0;i<subDomain->getNumPartitions();i++){
				int odeWorker = scheduler->getODEWorkerRank(i);
				Partition *partition = subDomain->getPartition(i);
				int start, size;
				Variable *var = solver->getVar();
				if (var->isVolumeVar()){
					start = partition->getVolumeDataStart();
					size = partition->getVolumeDataSize();
				}else{
					start = partition->getMembraneDataStart();
					size = partition->getMembraneDataSize();
				}
				rc = MPI_Recv(var->getCurr()+start, 
					size, 
					MPI_DOUBLE, 
					odeWorker, 
					ODE_COLLECT_TAG, 
					MPI_COMM_WORLD,
					&status);
				if (rc != MPI_SUCCESS){
					printf("PDEWorkerRoot(%d)::collectResults(), getting ODE results from worker %d, MPI_Recv rc=%d\n",getWorldRank(),odeWorker,rc);
				}
			}
		}
	}
#endif
}

