/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef ROI_DATA_GENERATOR_H
#define ROI_DATA_GENERATOR_H

#include <VCELL/DataGenerator.h>
#include <VCELL/FieldData.h>
#include <VCELL/SimTool.h>

class RoiDataGenerator : public DataGenerator
{
public:
	RoiDataGenerator(string& name, int* argVolumePoints, int numVolPoints, int* argMemPoints, int numMemPoints, FieldData* argSampleImage, int argNumImageRegions, int argZSlice);
	virtual ~RoiDataGenerator();

	void resolveReferences(SimulationExpression* sim);
	void computePPData(SimulationExpression* sim);

private:
	int* volumePoints;
	int* membranePoints;
	int numVolumePoints;
	int numMembranePoints;

	void parseInput(SimTool* simTool);
	
	FieldData* sampleImage;
	int numImageRegions;
	int zSlice;
};

#endif