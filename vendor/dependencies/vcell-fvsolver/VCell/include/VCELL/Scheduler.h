/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef SCHEDULER_H
#define SCHEDULER_H

class SimTool;
class VCellModel;
class Simulation;

class Scheduler
{
public:
	Scheduler(Simulation *Asim);
	virtual ~Scheduler() = default;

	virtual void iterate(SimTool* sim_tool)=0;
	virtual void initValues(VCellModel* model);
	void solveFastSystem(SimTool* sim_tool, int startVolIndex, int VolSize, int startMemIndex, int MemSize);
	bool hasFastSystem(VCellModel* model) const { return bHasFastSystem; }

protected:
	Simulation *sim;
	bool    bFirstTime;
	bool    bHasFastSystem;
};

#endif
