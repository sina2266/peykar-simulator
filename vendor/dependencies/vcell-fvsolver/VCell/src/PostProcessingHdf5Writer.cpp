/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/PostProcessingHdf5Writer.h>
#include <VCELL/PostProcessingBlock.h>
#include <VCELL/VariableStatisticsDataGenerator.h>
#include <VCELL/Variable.h>
#include <VCELL/CartesianMesh.h>
#include <typeinfo>
#include <H5Cpp.h>
#include <iostream>
#include <fstream>
#include <utility>
using std::cout;
using std::endl;
#include <sys/stat.h>

#define POST_PROCESSING_ROOT "/PostProcessing"

const char* PostProcessingHdf5Writer::PPGroupName  = POST_PROCESSING_ROOT;
const char* PostProcessingHdf5Writer::TimesDataSetName  = POST_PROCESSING_ROOT"/Times";

PostProcessingHdf5Writer::PostProcessingHdf5Writer(std::string fileName, PostProcessingBlock* ppb) {
	this->h5PPFileName = std::move(fileName);
	this->postProcessingBlock = ppb;
	h5PPFile = NULL;
	timesDataSet = NULL;
}

PostProcessingHdf5Writer::~PostProcessingHdf5Writer() {
	delete timesDataSet;
	delete h5PPFile;
}

bool PostProcessingHdf5Writer::loadFinal(int numTimes) {
	//return false;

	struct stat buf;
	// file doesn't exist
	if (stat(h5PPFileName.c_str(), &buf) || buf.st_size == 0) {
		return false;
	}

	bool bSuccess = true;
	// rename the file
	char hdf5CopyFileName[128];
	sprintf(hdf5CopyFileName, "%stmp", h5PPFileName.c_str());
	// remove if exists
	remove(hdf5CopyFileName);
	int r = rename(h5PPFileName.c_str(), hdf5CopyFileName);
	// can't rename, copy the file
	if (r) {
		std::ifstream infile(h5PPFileName.c_str(), std::ios_base::binary);
		std::ofstream outfile(hdf5CopyFileName, std::ios_base::binary);
		outfile << infile.rdbuf();
		infile.close();
		outfile.close();
	}
	try {
		H5::H5File hdf5Copy(hdf5CopyFileName, H5F_ACC_RDONLY);
		createGroups();

		H5::DataSet copyTimesDataSet = hdf5Copy.openDataSet(TimesDataSetName);
		H5::DataSpace copyTimesDataSpace = copyTimesDataSet.getSpace();
		hsize_t timesDim;
		int ndims = copyTimesDataSpace.getSimpleExtentDims(&timesDim, NULL);
		// the max time in log file is numTimes
		// hdf5 is missing times
		if (timesDim < numTimes) {
			std::cout << "post processing hdf5 times don't match" << std::endl;
			return false;
		}		
		double* times = new double[int(timesDim)];
		copyTimesDataSet.read(times, H5::PredType::NATIVE_DOUBLE);
		// add all the times to timeList
		timeList.assign(times, times + numTimes);
		// write all the times to file
		hsize_t size = numTimes;
		timesDataSet->extend(&size);
		hsize_t dim = numTimes;
		hsize_t offset = 0;
		H5::DataSpace fspace = timesDataSet->getSpace();
		fspace.selectHyperslab(H5S_SELECT_SET, &dim, &offset);
		timesDataSet->write(times, H5::PredType::NATIVE_DOUBLE, copyTimesDataSpace, fspace);
		delete[] times;

		for (int timeIndex = 0; timeIndex < numTimes; timeIndex ++) {
			vector<DataGenerator*>::iterator it;
			for(it = postProcessingBlock->dataGeneratorList.begin(); it < postProcessingBlock->dataGeneratorList.end(); ++ it) {
				DataGenerator* dataGenerator = *it;

				// create dataset /PostProcessing/fluor/time0
				char dataSetName[128];
				sprintf(dataSetName, "%s/%s/time%06d", PPGroupName, dataGenerator->getQualifiedName().c_str(), timeIndex);	

				H5::DataSet dataSet = hdf5Copy.openDataSet(dataSetName);
				dataSet.read(dataGenerator->data, H5::PredType::NATIVE_DOUBLE);

				writeDataGenerator(dataGenerator, timeIndex);
			}
		}
		hdf5Copy.close();
	} catch(H5::Exception error ) {
		std::cout << "failed to reload post processing block: " << error.getDetailMsg() << std::endl;
		// recreate the file later
	}

	remove(hdf5CopyFileName);
	if (bSuccess) {
		h5PPFile->flush(H5F_SCOPE_GLOBAL);
	} else {
		timesDataSet = NULL;
		h5PPFile = NULL;
	}	
	return true;
}

void PostProcessingHdf5Writer::createGroups() {
	if (h5PPFile != NULL) {
		return;
	}
	H5::DataSpace attributeDataSpace(H5S_SCALAR);
	H5::StrType attributeNameStrType(0, 64);
	H5::StrType attributeUnitStrType(0,64);

	h5PPFile = new H5::H5File(h5PPFileName.c_str(), H5F_ACC_TRUNC);

	// create post processing group /PostProcessing
	h5PPFile->createGroup(PPGroupName);

	// create /PostProcessing/Times
	int timesRank = 1;
	hsize_t timesDims = 10;	
	hsize_t maxDims = H5S_UNLIMITED;
	H5::DataSpace timesDataSpace(timesRank, &timesDims, &maxDims);
	// enable chunking
	H5::DSetCreatPropList cparms;
	hsize_t chunkDims = 500;
	cparms.setChunk(timesRank, &chunkDims);
	int fill_val = -1;
	cparms.setFillValue(H5::PredType::NATIVE_INT, &fill_val);
	// create dataset
	timesDataSet = new H5::DataSet(h5PPFile->createDataSet(TimesDataSetName, H5::PredType::NATIVE_DOUBLE, timesDataSpace, cparms));

	// create a group for each data generator
	char dataGeneratorGroupName[128];
	vector<DataGenerator*>::iterator it;
	for(it = postProcessingBlock->dataGeneratorList.begin(); it < postProcessingBlock->dataGeneratorList.end(); ++ it) {
		DataGenerator* dataGenerator = *it;

		sprintf(dataGeneratorGroupName, "%s/%s", PPGroupName, dataGenerator->getQualifiedName().c_str());
		H5::Group dataGeneratorGroup = h5PPFile->createGroup(dataGeneratorGroupName);
		
		if (typeid(*dataGenerator) == typeid(VariableStatisticsDataGenerator)) {
			((VariableStatisticsDataGenerator*)dataGenerator)->detailGroup(h5PPFile, dataGeneratorGroup, postProcessingBlock->simulation);
		}
		else 
		{
			// adding origin and extent as attributes for all variable
			string varAttNames[6] = {"OriginX", "OriginY", "OriginZ", "ExtentX", "ExtentY", "ExtentZ"};
			CartesianMesh* mesh = ((CartesianMesh*)postProcessingBlock->getSimulation()->getMesh());
			float origin[3] = {(float)mesh->getDomainOriginX(), (float)mesh->getDomainOriginY(), (float)mesh->getDomainOriginZ()};
			float extent[3] = {(float)mesh->getDomainSizeX(), (float)mesh->getDomainSizeY(), (float)mesh->getDomainSizeZ()};
			int dim = mesh->getDimension();
			for (int i = 0; i < dim; i ++) { // add origin
				H5::FloatType float_type(H5::PredType::NATIVE_FLOAT);
				H5::Attribute attribute = dataGeneratorGroup.createAttribute(varAttNames[i], float_type, attributeDataSpace);
				attribute.write(float_type, origin+i);
			}
			for (int i = 0; i < dim; i ++) { // add extent
				H5::FloatType float_type(H5::PredType::NATIVE_FLOAT);
				H5::Attribute attribute = dataGeneratorGroup.createAttribute(varAttNames[3+i], float_type, attributeDataSpace);
				attribute.write(float_type, extent+i);
			}
		}
	}
}

void PostProcessingHdf5Writer::writeOutput(SimTool* sim_tool) {
	try {
		int timesRank = 1;
		hsize_t timesDims = 1;	
		hsize_t maxDims = H5S_UNLIMITED;
		H5::DataSpace timesDataSpace(timesRank, &timesDims, &maxDims);

		createGroups();

		// write current time
		double currTime = postProcessingBlock->simulation->getTime_sec(sim_tool);
		hsize_t size = timeList.size() + 1;
		timesDataSet->extend(&size);
		hsize_t dim = 1;
		hsize_t offset = timeList.size();
		H5::DataSpace fspace = timesDataSet->getSpace();
		fspace.selectHyperslab(H5S_SELECT_SET, &dim, &offset);
		timesDataSet->write(&currTime, H5::PredType::NATIVE_DOUBLE, timesDataSpace, fspace);

		timeList.push_back(currTime);

		int timeIndex = timeList.size() - 1;
		vector<DataGenerator*>::iterator it;
		for(it = postProcessingBlock->dataGeneratorList.begin(); it < postProcessingBlock->dataGeneratorList.end(); ++ it) {
			DataGenerator* dataGenerator = *it;
			dataGenerator->computePPData(postProcessingBlock->simulation);
			writeDataGenerator(dataGenerator, timeIndex);
		}
		
		h5PPFile->flush(H5F_SCOPE_GLOBAL);
	} catch(H5::Exception error ) {
		throw std::runtime_error(error.getDetailMsg());
	}
}

void PostProcessingHdf5Writer::writeDataGenerator(DataGenerator* dataGenerator, int timeIndex) {
	H5::DataSpace attributeDataSpace(H5S_SCALAR);
	H5::StrType attributeNameStrType(0, 64);

	// create dataset /PostProcessing/fluor/time0
	char dataSetName[128];
	sprintf(dataSetName, "%s/%s/time%06d", PPGroupName, dataGenerator->getQualifiedName().c_str(), timeIndex);	

	// create dataspace
	H5::DataSpace dataspace(dataGenerator->hdf5Rank, dataGenerator->hdf5Dims);
	H5::DataSet dataSet = h5PPFile->createDataSet(dataSetName, H5::PredType::NATIVE_DOUBLE, dataspace);

	// write dataset
	dataSet.write(dataGenerator->getData(), H5::PredType::NATIVE_DOUBLE, H5S_ALL, H5S_ALL);

	// close dataset
	dataspace.close();
	dataSet.close();
}
