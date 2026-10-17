/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include "SmoldynHdf5Writer.h"
#include "SmoldynVarStatDataGenerator.h"
#include <VCELL/Variable.h>
#include <VCELL/CartesianMesh.h>
#include <typeinfo>
#include <H5Cpp.h>
#include <iostream>
#include <fstream>
using std::cout;
using std::endl;
#include <sys/stat.h>

#define POST_PROCESSING_ROOT "/PostProcessing"

const char* SmoldynHdf5Writer::PPGroupName  = POST_PROCESSING_ROOT;
const char* SmoldynHdf5Writer::TimesDataSetName  = POST_PROCESSING_ROOT"/Times";

SmoldynHdf5Writer::SmoldynHdf5Writer(char* fileName, VCellSmoldynOutput* vso) {
	this->h5PPFileName = fileName;
	this->vso = vso;
	h5PPFile = NULL;
	timesDataSet = NULL;
}

SmoldynHdf5Writer::~SmoldynHdf5Writer() {
	delete timesDataSet;
	delete h5PPFile;
}

void SmoldynHdf5Writer::createGroups() {
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
	vector<SmoldynDataGenerator*>::iterator it;
	for(it = vso->dataGeneratorList.begin(); it < vso->dataGeneratorList.end(); ++ it) {
		SmoldynDataGenerator* dataGenerator = *it;

		sprintf(dataGeneratorGroupName, "%s/%s", PPGroupName, dataGenerator->getName().c_str());
		H5::Group dataGeneratorGroup = h5PPFile->createGroup(dataGeneratorGroupName);
		
		if (typeid(*dataGenerator) == typeid(SmoldynVarStatDataGenerator)) {
			// attributes : all the components
			int numVolVar = vso->volVariables.size();
			int numMemVar = vso->memVariables.size();
			// write volume var
			for (int i = 0; i < numVolVar; i ++) {
				SmoldynVariable* volvar = vso->volVariables[i];
				const char* varName = volvar->name.c_str();
				char attrName[64];
				char compName[64];
				char compUnit[64];
				//var total name and unit
				sprintf(attrName, "comp_%d_name", 2*i);
				H5::Attribute attribute = dataGeneratorGroup.createAttribute(attrName, attributeNameStrType, attributeDataSpace);
				sprintf(compName, "%s_total", varName);
				attribute.write(attributeNameStrType, compName);

				sprintf(attrName, "comp_%d_unit", 2*i);
				attribute = dataGeneratorGroup.createAttribute(attrName, attributeUnitStrType, attributeDataSpace);
				sprintf(compUnit, "molecules");
				attribute.write(attributeUnitStrType, compUnit);

				//var average name and unit
				sprintf(attrName, "comp_%d_name", 2*i + 1);
				attribute = dataGeneratorGroup.createAttribute(attrName, attributeNameStrType, attributeDataSpace);
				sprintf(compName, "%s_average", varName);
				attribute.write(attributeNameStrType, compName);

				sprintf(attrName, "comp_%d_unit", 2*i + 1);
				attribute = dataGeneratorGroup.createAttribute(attrName, attributeUnitStrType, attributeDataSpace);
				sprintf(compUnit, "uM");
				attribute.write(attributeUnitStrType, compUnit);
			}
			//write membrane var 
			int offset = 2*numVolVar;
			for (int i = 0; i < numMemVar; i ++) {
				SmoldynVariable* memvar = vso->memVariables[i];
				const char* varName = memvar->name.c_str();
				char attrName[64];
				char compName[64];
				char compUnit[64];
				//var total name and unit
				sprintf(attrName, "comp_%d_name", offset + 2*i);
				H5::Attribute attribute = dataGeneratorGroup.createAttribute(attrName, attributeNameStrType, attributeDataSpace);
				sprintf(compName, "%s_total", varName);
				attribute.write(attributeNameStrType, compName);

				sprintf(attrName, "comp_%d_unit", offset + 2*i);
				attribute = dataGeneratorGroup.createAttribute(attrName, attributeUnitStrType, attributeDataSpace);
				sprintf(compUnit, "molecules");
				attribute.write(attributeUnitStrType, compUnit);
				//var average name and unit
				sprintf(attrName, "comp_%d_name", offset + 2*i + 1);
				attribute = dataGeneratorGroup.createAttribute(attrName, attributeNameStrType, attributeDataSpace);
				sprintf(compName, "%s_average", varName);
				attribute.write(attributeNameStrType, compName);

				sprintf(attrName, "comp_%d_unit", offset + 2*i + 1);
				attribute = dataGeneratorGroup.createAttribute(attrName, attributeUnitStrType, attributeDataSpace);
				sprintf(compUnit, "molecules.um-2");
				attribute.write(attributeUnitStrType, compUnit);
			}
		}
	}
}

void SmoldynHdf5Writer::writeOutput()
{
	try {
		int timesRank = 1;
		hsize_t timesDims = 1;	
		hsize_t maxDims = H5S_UNLIMITED;
		H5::DataSpace timesDataSpace(timesRank, &timesDims, &maxDims);

		createGroups();

		// write current time
		double currTime = vso->smoldynSim->time;
		hsize_t size = timeList.size() + 1;
		timesDataSet->extend(&size);
		hsize_t dim = 1;
		hsize_t offset = timeList.size();
		H5::DataSpace fspace = timesDataSet->getSpace();
		fspace.selectHyperslab(H5S_SELECT_SET, &dim, &offset);
		timesDataSet->write(&currTime, H5::PredType::NATIVE_DOUBLE, timesDataSpace, fspace);

		timeList.push_back(currTime);

		int timeIndex = timeList.size() - 1;
		vector<SmoldynDataGenerator*>::iterator it;
		for(it = vso->dataGeneratorList.begin(); it < vso->dataGeneratorList.end(); ++ it) {
			SmoldynDataGenerator* dataGenerator = *it;
			dataGenerator->computePPData(vso);
			writeDataGenerator(dataGenerator, timeIndex);
		}
		
		h5PPFile->flush(H5F_SCOPE_GLOBAL);
	} catch(H5::Exception error ) {
		throw error.getDetailMsg();
	}
}

void SmoldynHdf5Writer::writeDataGenerator(SmoldynDataGenerator* dataGenerator, int timeIndex) {
	H5::DataSpace attributeDataSpace(H5S_SCALAR);
	H5::StrType attributeNameStrType(0, 64);

	// create dataset /PostProcessing/fluor/time0
	char dataSetName[128];
	sprintf(dataSetName, "%s/%s/time%06d", PPGroupName, dataGenerator->getName().c_str(), timeIndex);	

	// create dataspace
	H5::DataSpace dataspace(dataGenerator->hdf5Rank, dataGenerator->hdf5Dims); 
	H5::DataSet dataSet = h5PPFile->createDataSet(dataSetName, H5::PredType::NATIVE_DOUBLE, dataspace);

	// write dataset
	dataSet.write(dataGenerator->getData(), H5::PredType::NATIVE_DOUBLE, H5S_ALL, H5S_ALL);

	// close dataset
	dataspace.close();
	dataSet.close();
}
