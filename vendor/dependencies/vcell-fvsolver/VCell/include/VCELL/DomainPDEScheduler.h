/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef DOMAINPDESCHEDULER_H
#define DOMAINPDESCHEDULER_H

#include <stdio.h>
#include <VCELL/SimTypes.h>
#include <VCELL/Scheduler.h>

class Simulation;
class DomainPDEScheduler;
class CartesianSubDomain;
class Worker;


class DomainPDEScheduler : public Scheduler
{
public:
	DomainPDEScheduler(Simulation *Asim);
	~DomainPDEScheduler();
	virtual void iterate();
	virtual void    update();
	virtual void    collectResults(int processRank);
	virtual bool initValues();
	int getPDEWorkerRank(PDESolver *pdeSolver);
	int getODEWorkerRank(int partitionNumber);
	int getPartitionNumber(int odeWorkerRank);
	int getNumPartitions() { return numODEworkers; }
protected:
	int getNumPDEs();
	PDESolver *getPDESolver(int worldRank);
	
	Worker *worker;
	    
	int numPDEworkers;
	int numODEworkers;
};

class Worker 
{
public:
	Worker(int worldRank, Simulation *sim, DomainPDEScheduler *scheduler);
	~Worker();

	virtual bool iterate() = 0;
	virtual bool init();
	virtual void update() = 0;
	virtual void collectResults() = 0;

	int getWorldRank() { return worldRank; }
  
protected:
	static const int    ODE_COLLECT_TAG;
	static const int    PDE_COLLECT_TAG;
	static const int    ODE_UPDATE_TAG;
	static const int    PDE_UPDATE_TAG;
	static const int    PDE_SOURCE_TAG;
	Simulation *sim; 
	double     *commBuffer;
	DomainPDEScheduler *scheduler;
	bool    bInitEquations;

protected:
	CartesianSubDomain *subDomain;

private:
	int worldRank;
};


class ODEWorker : public Worker
{
public:
	ODEWorker(int worldRank, Simulation *sim, DomainPDEScheduler *scheduler);
	virtual bool iterate();
	virtual bool init();
	virtual void update();
	virtual void collectResults();
protected:
	int partitionNumber;
};


class PDEWorker : public Worker
{
public:
	PDEWorker(int worldRank, Simulation *sim, DomainPDEScheduler *scheduler, PDESolver *pdeSolver);
	virtual bool iterate();
	virtual bool init();
	virtual void update();
	virtual void collectResults();
protected:
	PDESolver  *pdeSolver;
};

class PDEWorkerRoot : public PDEWorker
{
public:
	PDEWorkerRoot(int worldRank, Simulation *sim, DomainPDEScheduler *scheduler, PDESolver *pdeSolver);
	virtual void collectResults();
};

class Partition
{
public:
    Partition();
    
    void show(char *buffer);
    
    void setVolumeCompute(int start, int size);
    void setVolumeData(int start, int size);
    void setMembraneCompute(int start, int size);
    void setMembraneData(int start, int size);
    
    int getVolumeComputeStart()   { return volComputeStart; }
    int getVolumeComputeSize()    { return volComputeSize; }
    int getVolumeDataStart()      { return volDataStart; }
    int getVolumeDataSize()       { return volDataSize; }
    int getMembraneComputeStart() { return memComputeStart; }
    int getMembraneComputeSize()  { return memComputeSize; }
    int getMembraneDataStart()    { return memDataStart; }
    int getMembraneDataSize()     { return memDataSize; }
     
protected:
    int volComputeStart;
    int volComputeSize;
    int memComputeStart;
    int memComputeSize;
    int volDataStart;
    int volDataSize;
    int memDataStart;
    int memDataSize;
};

class CartesianSubDomain 
{
public:
	CartesianSubDomain(CartesianMesh *mesh, 
						int numPartitions); 

	Partition *getPartition(int partitionNumber);
	int getNumPartitions()      { return numPartitions;   }

protected:
	int numPartitions;
	CartesianMesh *mesh;
	Partition *partitions;
};

#endif
