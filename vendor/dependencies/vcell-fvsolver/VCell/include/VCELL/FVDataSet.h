/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef FVDATASET_H
#define FVDATASET_H

class Simulation;
class SimulationExpression;
class Variable;

class FVDataSet
{
public:
	static void read(const char *filename, Simulation *sim);
	static void write(const char *filename, SimulationExpression *sim, bool bCompress);
	static void convolve(SimulationExpression* sim, Variable* var, double* values);
	static void readRandomVariables(char* filename, SimulationExpression* sim);
};

#endif
