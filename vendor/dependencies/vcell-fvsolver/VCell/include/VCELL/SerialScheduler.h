/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef SERIALSCHEDULER_H
#define SERIALSCHEDULER_H

#include <VCELL/Scheduler.h>

class Simulation;

class SerialScheduler : public Scheduler
{
public:
	SerialScheduler(Simulation *Asim);
	void iterate(SimTool* sim_tool) override;
};

#endif
