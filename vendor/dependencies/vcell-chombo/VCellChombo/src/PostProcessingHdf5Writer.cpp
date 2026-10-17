/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/PostProcessingHdf5Writer.h>
#include <VCELL/PostProcessingBlock.h>
#include <VCELL/VariableStatisticsDataGenerator.h>
#include <VCELL/Variable.h>
#include <VCELL/SimTool.h>
#include <typeinfo>
#include <hdf5.h>
#include <iostream>
#include <fstream>
using std::cout;
using std::endl;
#include <sys/stat.h>

#define POST_PROCESSING_ROOT "/PostProcessing"

const char* PostProcessingHdf5Writer::PPGroupName  = POST_PROCESSING_ROOT;
const char* PostProcessingHdf5Writer::TimesDataSetName  = POST_PROCESSING_ROOT"/Times";

PostProcessingHdf5Writer::PostProcessingHdf5Writer(char* fileName, PostProcessingBlock* ppb)
{
	this->h5PPFileName = fileName;
	this->postProcessingBlock = ppb;
	h5PPFile = H5I_INVALID_HID;
	timesDataSet = H5I_INVALID_HID;
}

PostProcessingHdf5Writer::~PostProcessingHdf5Writer()
{
	if (timesDataSet != H5I_INVALID_HID)
	{
		H5Dclose(timesDataSet);
	}
	if (h5PPFile != H5I_INVALID_HID)
	{
		H5Fclose(h5PPFile);
	}
}

void PostProcessingHdf5Writer::createGroups()
{
	static string METHOD = "(PostProcessingHdf5Writer:createGroups)";
	pout() << "Entry " << METHOD << endl;
	
	if (h5PPFile != H5I_INVALID_HID)
	{
		return;
	}
	hid_t file_access = H5P_DEFAULT;
#ifdef CH_MPI
	file_access = H5Pcreate(H5P_FILE_ACCESS);
	H5Pset_fapl_mpio(file_access,  MPI_COMM_WORLD, MPI_INFO_NULL);
#endif
	h5PPFile = H5Fcreate(h5PPFileName.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, file_access);
	
	hid_t attributeDataSpace = H5Screate(H5S_SCALAR);
	hid_t attributeStrType = H5Tcreate(H5T_STRING, sizeof(char) * 64);

	// create post processing group /PostProcessing
	hid_t ppGroup = H5Gcreate(h5PPFile, PPGroupName, H5P_DEFAULT);
	H5Gclose(ppGroup);

	// create /PostProcessing/Times
	int timesRank = 1;  // number of dimensions
	hsize_t timesDims = 10;
	hsize_t maxDims = H5S_UNLIMITED;
	hid_t timesDataSpace = H5Screate_simple(timesRank, &timesDims, &maxDims);
	
	// enable chunking
	int ndims = 1; // number of dimensions of each chunk
	hsize_t chunkDims = 500;  // dimensions of each chunk
	// data creation property list
	hid_t dcpl = H5Pcreate(H5P_DATASET_CREATE);
	H5Pset_chunk(dcpl, ndims, &chunkDims);

	//	double fill_val = -1;
	//	H5Pset_fill_value(dcpl, H5T_NATIVE_DOUBLE, &fill_val);
	// create dataset
	timesDataSet = H5Dcreate(h5PPFile, TimesDataSetName, H5T_NATIVE_DOUBLE, timesDataSpace, dcpl);

	// create a group for each data generator
	char dataGeneratorGroupName[128];
	vector<DataGenerator*>::iterator it;
	for(it = postProcessingBlock->dataGeneratorList.begin(); it < postProcessingBlock->dataGeneratorList.end(); ++ it)
	{
		DataGenerator* dataGenerator = *it;

		sprintf(dataGeneratorGroupName, "%s/%s", PPGroupName, dataGenerator->getQualifiedName().c_str());
		hid_t dataGeneratorGroup = H5Gcreate(h5PPFile, dataGeneratorGroupName, H5P_DEFAULT); 
		
		if (typeid(*dataGenerator) == typeid(VariableStatisticsDataGenerator))
		{
			((VariableStatisticsDataGenerator*)dataGenerator)->detailGroup(h5PPFile, dataGeneratorGroup, postProcessingBlock->getSimulation());
		}
		H5Gclose(dataGeneratorGroup);
	}
	pout() << "Exit " << METHOD << endl;
}

void PostProcessingHdf5Writer::writeOutput()
{
	static string METHOD = "(PostProcessingHdf5Writer::writeOutput:writeOutput)";
	pout() << "Entry " << METHOD << endl;

	createGroups();
	
	// write current time
	double currTime = postProcessingBlock->simulation->getTime_sec();
	timeList.push_back(currTime);
	int timeIndex = timeList.size() - 1;
	
	hsize_t size = timeList.size();
	H5Dset_extent(timesDataSet, &size);
	
	int timesRank = 1;
	hsize_t timesDims = 1;
	hsize_t maxDims = H5S_UNLIMITED;
	hid_t timesDataSpace = H5Screate_simple(timesRank, &timesDims, &maxDims);

	if (postProcessingBlock->simulation->getSimTool()->isRootRank())
	{
		hsize_t count = 1;
		hsize_t offset = timeIndex;
		hid_t fspace = H5Dget_space(timesDataSet);
		H5Sselect_hyperslab(fspace, H5S_SELECT_SET, &offset, NULL, &count, NULL);
		H5Dwrite(timesDataSet, H5T_NATIVE_DOUBLE, timesDataSpace, fspace, H5P_DEFAULT, &currTime);
	}
	H5Sclose(timesDataSpace);

	vector<DataGenerator*>::iterator it;
	for(it = postProcessingBlock->dataGeneratorList.begin(); it < postProcessingBlock->dataGeneratorList.end(); ++ it)
	{
		DataGenerator* dataGenerator = *it;
		dataGenerator->computePPData(postProcessingBlock->simulation);
		writeDataGenerator(dataGenerator, timeIndex);
	}

	H5Fflush(h5PPFile, H5F_SCOPE_GLOBAL);
	pout() << "Exit " << METHOD << endl;
}

void PostProcessingHdf5Writer::writeDataGenerator(DataGenerator* dataGenerator, int timeIndex)
{
	// create dataset /PostProcessing/VariableStatistics/time0
	char dataSetName[128];
	sprintf(dataSetName, "%s/%s/time%06d", PPGroupName, dataGenerator->getQualifiedName().c_str(), timeIndex);	

	// create data space
	hid_t dataspace = H5Screate_simple(dataGenerator->hdf5Rank, dataGenerator->hdf5Dims, NULL);
	hid_t dataSet = H5Dcreate(h5PPFile, dataSetName, H5T_NATIVE_DOUBLE, dataspace, H5P_DEFAULT);

	if (postProcessingBlock->getSimulation()->getSimTool()->isRootRank())
	{
		// write dataset
		H5Dwrite(dataSet, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, dataGenerator->getData());
	}
	
	// close dataset
	H5Dclose(dataSet);
	H5Sclose(dataspace);
}
