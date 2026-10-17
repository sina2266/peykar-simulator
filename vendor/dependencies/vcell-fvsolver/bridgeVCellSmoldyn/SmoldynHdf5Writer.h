/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef SMOLDYN_HDF5_WRITER_H
#define SMOLDYN_HDF5_WRITER_H

#include <VCELL/SimulationExpression.h>
#include "VCellSmoldynOutput.h"
#include <SymbolTable.h>
#include <vector>
using std::vector;

namespace H5 {
	class H5File;
	class DataSet;
}

class SmoldynHdf5Writer
{
public:
	SmoldynHdf5Writer(char* fileName, VCellSmoldynOutput* vso);
	virtual ~SmoldynHdf5Writer();

	void writeOutput();

	static const char* PPGroupName;
	static const char* TimesDataSetName;

	bool loadFinal(int numTimes);

private:
	vector<double> timeList;
	VCellSmoldynOutput* vso;

	string h5PPFileName;
	H5::H5File* h5PPFile;
	H5::DataSet* timesDataSet;
	void writeDataGenerator(SmoldynDataGenerator* dataGenerator, int timeIndex);
	void createGroups();
};

#endif
