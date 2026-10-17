/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef GAUSSIAN_CONVOLUTION_DATA_GENERATOR_H
#define GAUSSIAN_CONVOLUTION_DATA_GENERATOR_H

#include <VCELL/DataGenerator.h>

namespace VCell {
	class Expression;
}

class GaussianConvolutionDataGenerator : public DataGenerator
{
public:
	GaussianConvolutionDataGenerator(string& name, Feature* f, double sigmaXY, double sigmaZ, VCell::Expression* volFunc, VCell::Expression* memFunc);
	virtual ~GaussianConvolutionDataGenerator();

	void resolveReferences(SimulationExpression* sim);
	void computePPData(SimulationExpression* sim);

private:
	
	double sigmaXY;
	double sigmaZ;
	double sigmaRatio;
	double* gaussianPsfSamples;
	double* functionValues;
	int funcValueSize;
	int gaussianPsfSampleNx;
	int gaussianPsfSampleNy;
	int gaussianPsfSampleNz;
	VCell::Expression* volFunction;
	VCell::Expression* memFunction;
};

#endif
