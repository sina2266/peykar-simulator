/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef PDESCHEDULER_H
#define PDESCHEDULER_H

#include <VCELL/Scheduler.h>

class PDEScheduler : public Scheduler
{
public:
	PDEScheduler(Simulation *Asim);
	virtual void iterate();
	virtual void    update();
	virtual void    collectResults(int processRank);

private:
#ifdef VCELL_MPI
	int mpiRank;
	int mpiSize;
#endif

};


#endif
