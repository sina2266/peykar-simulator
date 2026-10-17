/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include "SmoldynVarStatDataGenerator.h"
#include "VCellSmoldynOutput.h"
#include <string.h>
#include <Expression.h>
using VCell::Expression;

#include <algorithm>
using std::max;
using std::min;

const string SmoldynVarStatDataGenerator::VariableStatistics_Name = "VariableStatistics";

SmoldynVarStatDataGenerator::SmoldynVarStatDataGenerator() : SmoldynDataGenerator(VariableStatistics_Name){
}

SmoldynVarStatDataGenerator::~SmoldynVarStatDataGenerator() {
}

void SmoldynVarStatDataGenerator::initialize(VCellSmoldynOutput* vso) {
	int numTotalVar = vso->volVariables.size() + vso->memVariables.size();
	// compute data size
	dataSize = numTotalVar * 2;//for total and average
	// initialize data
	data = new double[dataSize];
	memset(data, 0, dataSize * sizeof(double));

	hdf5Rank = 1;//one dimenaion
	// total and NO average
	hdf5Dims[0] =  dataSize;//data size
}

void SmoldynVarStatDataGenerator::computePPData(VCellSmoldynOutput* vso) {
	int numVolVar = vso->volVariables.size();
	int numMemVar = vso->memVariables.size();
	
	for (int i = 0; i < numVolVar; i ++) {
		int molCount = 0;
		for (int j = 0; j < vso->numVolumeElements; j ++) {
			molCount = molCount + vso->volVarOutputData[i][j];
		}
		// total count
		data[2*i] = molCount;//num of molecules
		// average
		data[2*i+1] = molCount/(vso->volVariables[i]->cmpt->volume * 602.0); //concentration in uM
	}

	int offset = numVolVar * 2;
	for (int i = 0; i < numMemVar; i ++) {
		int molCount = 0;
		for (int j = 0; j < vso->numMembraneElements; j ++) {
			molCount = molCount + vso->memVarOutputData[i][j];
		}
		// total count
		data[offset + 2*i] = molCount;//num of molecules
		// average
		data[offset + 2*i +1] = molCount / vso->memVariables[i]->srf->totarea;//concentration in molecules/um2			
	}
}
