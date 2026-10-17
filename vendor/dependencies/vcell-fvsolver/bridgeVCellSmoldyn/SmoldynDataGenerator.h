/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef SMOLDYN_DATA_GENERATOR_H
#define SMOLDYN_DATA_GENERATOR_H

#include <H5Cpp.h>
#include <string>
using std::string;

class VCellSmoldynOutput;
class SmoldynDataGenerator
{
public:
	SmoldynDataGenerator(const string& name);
	virtual ~SmoldynDataGenerator();
	
	virtual void computePPData(VCellSmoldynOutput* vso)=0; //get data ready for Hdf5 writer
	virtual void initialize(VCellSmoldynOutput* vso)=0; //initialized for data length
	
	int getHdf5Rank() {return hdf5Rank;}
	hsize_t* getHdf5Dims() {return hdf5Dims;}
	string getName(){return name;};//return the hdf5 data block name
	
	long getDataSize() {
		return dataSize;
	}
	double* getData() {
		return data;
	}

	const static double double_max;
	const static double double_min;

protected:
	double* data;
	long dataSize;
	string name;
	int hdf5Rank;
	hsize_t hdf5Dims[3];

	friend class SmoldynHdf5Writer;
};

#endif
