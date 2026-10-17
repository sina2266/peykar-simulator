/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef DATASET_H
#define DATASET_H

#include <stdio.h>
#include <hdf5.h>

typedef unsigned int uint32;

typedef enum {
	endian_not_set = -1,
	little_endian,
	big_endian
} Endian;

class SimulationExpression;
class Variable;

class DataSet
{
public:
	static void readDoubles(FILE *fp, double *data, int length);

	static bool isBigEndian();

#ifdef CH_MPI
	static void writeMembraneSolution(SimulationExpression* sim, hid_t h5SimFile, int memIndexOffet, int totalNumMembranePoints);
	static void writeExtrapolatedValues(SimulationExpression* sim, hid_t h5SimFile, int memIndexOffet, int totalNumMembranePoints);
#else
	static void writeMembraneSolution(SimulationExpression* sim, hid_t h5SimFile);
	static void writeExtrapolatedValues(SimulationExpression* sim, hid_t h5SimFile);
	static void write(SimulationExpression *sim, char* filename);
	static void readMembraneSolution(SimulationExpression* sim, hid_t h5SimFile);
	static void readExtrapolatedValues(SimulationExpression* sim, hid_t h5SimFile);
#endif
	
private:
	static Endian endian;
};

#endif
