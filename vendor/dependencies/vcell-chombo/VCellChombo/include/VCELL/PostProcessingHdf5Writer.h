/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef POST_PROCESSING_HDF5_WRITER_H
#define POST_PROCESSING_HDF5_WRITER_H

#include <VCELL/SimulationExpression.h>
#include <SymbolTable.h>
#include <vector>
using std::vector;

class DataGenerator;
class PostProcessingBlock;

class PostProcessingHdf5Writer
{
public:
	PostProcessingHdf5Writer(char* fileName, PostProcessingBlock* postProcessingBlock);
	virtual ~PostProcessingHdf5Writer();

	void writeOutput();

	static const char* PPGroupName;
	static const char* TimesDataSetName;

	bool loadFinal(int numTimes);

private:
	vector<double> timeList;
	PostProcessingBlock* postProcessingBlock;

	string h5PPFileName;
	hid_t h5PPFile;
	hid_t timesDataSet;
	void writeDataGenerator(DataGenerator* dataGenerator, int timeIndex);
	void createGroups();
};

#endif
