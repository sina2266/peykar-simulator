/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef SPLITSCHEDULER_H
#define SPLITSCHEDULER_H

#include <VCELL/SerialScheduler.h>

class Simulation;

class SplitScheduler : public SerialScheduler
{
public:
	SplitScheduler(Simulation *Asim);
	void iterate();
};

#endif
