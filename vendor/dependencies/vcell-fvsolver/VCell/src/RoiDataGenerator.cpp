/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/RoiDataGenerator.h>
#include <VCELL/SimulationExpression.h>
#include <VCELL/CartesianMesh.h>
#include <VCELL/FVDataSet.h>
#include <VCELL/Element.h>
#include <VCELL/Variable.h>
#include <VCELL/Feature.h>
#include <VCELL/Membrane.h>
#include <string.h>
#include <Expression.h>
using VCell::Expression;
#include <sstream>
#include <algorithm>
using std::max;
using std::min;
using std::stringstream;

RoiDataGenerator::RoiDataGenerator(string& name,  int* argVolumePoints, int argNumVolPoints, int* argMemPoints, int argNumMemPoints, FieldData* argSampleImage, int argNumImageRegions, int argZSlice) : DataGenerator(name, NULL){
	volumePoints = argVolumePoints;
	membranePoints = argMemPoints;
	numVolumePoints = argNumVolPoints;
	numMembranePoints = argNumMemPoints;
		
	numImageRegions = argNumImageRegions;//image region includes rois and the region that excluded from all rois (usually marked as pixel value 0)
	sampleImage = argSampleImage;
	zSlice = argZSlice;
}

RoiDataGenerator::~RoiDataGenerator() {
	if(volumePoints){
		delete[] volumePoints;
	}
	if(membranePoints){
		delete[] membranePoints;
	}
	if(sampleImage){
		delete sampleImage;
	}
}

void RoiDataGenerator::resolveReferences(SimulationExpression* sim) {
	// compute data size
	dataSize = numImageRegions;
	data = new double[dataSize];
	memset(data, 0, dataSize * sizeof(double));

	hdf5Rank = 1; // 1 dimension and the length is num of ROIs for each time point 
	hdf5Dims[0] =  numImageRegions; // num of ROIs
}

void RoiDataGenerator::computePPData(SimulationExpression* sim) {
	int numVar = sim->getNumVariables();
	CartesianMesh* mesh = (CartesianMesh *)sim->getMesh(); 

	int meshX = mesh->getNumVolumeX();
	int meshY = mesh->getNumVolumeY();
	int meshZ = mesh->getNumVolumeZ();
	int imgX = meshX;
	int imgY = meshY;
	int imgZ = 1;
	int zoffset = zSlice * meshX * meshY;

	double* convolved_values = new double[meshX * meshY * meshZ];
	int* count = new int[numImageRegions];

	memset(data, 0, dataSize * sizeof(double));

	for (int i = 0; i < numVar; i ++) { //should be only one variable for VFRAP
		Variable* var = sim->getVariable(i);
		double* values = new double[numImageRegions];			
		memset(values, 0, numImageRegions * sizeof(double));
		memset(count, 0, numImageRegions * sizeof(int));

		FVDataSet::convolve(sim, var, convolved_values);

		for (int j = 0; j < imgX * imgY; j ++) {
			int index = (int)sampleImage->getData()[j];
			if ((double)index != sampleImage->getData()[j]) {
				stringstream ss;
				ss << "RoiDataGenerator::computePPData(), index (" << sampleImage->getData()[j] << ") is not an integer. ";
				throw ss.str();
			}
			if (index >= numImageRegions || index < 0) {
				stringstream ss;
				ss << "RoiDataGenerator::computePPData(), index (" << index << ") is out of range. should be [" << 0 << "," << numImageRegions << ").";
				throw ss.str();
			}
			values[index] += convolved_values[zoffset + j];
			count[index] ++;			
		}
		for (int j = 0; j < numImageRegions; j ++) {
			if (count[j] != 0) {			
				data[j] = values[j]/count[j];
			}
		}
		delete[] values;
	}
	delete[] convolved_values;
	delete[] count;
}
