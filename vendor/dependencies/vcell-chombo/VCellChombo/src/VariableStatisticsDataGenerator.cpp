/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/VariableStatisticsDataGenerator.h>
#include <VCELL/Variable.h>
#include <VCELL/Feature.h>
#include <VCELL/Membrane.h>
#include <string.h>
#include <algorithm>
#include <VCELL/SimulationExpression.h>

const string VariableStatisticsDataGenerator::VariableStatistics_Name = "VariableStatistics";

VariableStatisticsDataGenerator::VariableStatisticsDataGenerator() 
	: DataGenerator(VariableStatistics_Name, NULL){
}

VariableStatisticsDataGenerator::~VariableStatisticsDataGenerator() {
}

void VariableStatisticsDataGenerator::resolveReferences(SimulationExpression* sim) {
	int numVar = sim->getNumVariables();

	// compute data size
	dataSize = 0;
	for (int i = 0; i < numVar; ++ i)
	{
		dataSize += 2; // mean, total

#ifndef CH_MPI
		dataSize += 2; // min, max

		Variable *var = sim->getVariable(i);
		if (var->getExactErrorVariable() != NULL)
		{
			dataSize += 2; // L2Error, maxError
		}
#endif
	}
	data = new double[dataSize];
	memset(data, 0, dataSize * sizeof(double));
	hdf5Rank = 1;
	hdf5Dims[0] = dataSize;
}

void VariableStatisticsDataGenerator::computePPData(SimulationExpression* sim) {
	int numVar = sim->getNumVariables();

	memset(data, 0, dataSize * sizeof(double));
	int dataCount = 0;
	for (int i = 0; i < numVar; i ++) {
		Variable* var = sim->getVariable(i);
		data[dataCount ++] = var->getMean(); // mean
		data[dataCount ++] = var->getTotal(); // total
#ifndef CH_MPI
		data[dataCount ++] = var->getMin(); // min
		data[dataCount ++] = var->getMax(); // max
		if (var->getExactErrorVariable() != NULL)
		{
			// l2Error, maxError, mean
			data[dataCount ++] = var->getL2Error(); // L2 error
			data[dataCount ++] = var->getMaxError(); // max error
		}
#endif
	}
}

void VariableStatisticsDataGenerator::writeAttributeComponent(const hid_t& dataGeneratorGroup,
		const char* varName, int componentIndex, const char* dataName, const string& unit)
{

	hid_t attributeDataSpace = H5Screate(H5S_SCALAR);
	hid_t attributeStrType = H5Tcreate(H5T_STRING, sizeof(char) * 64);

	char attrName[64];
	char attrValue[64];

	sprintf(attrName, "comp_%d_name", componentIndex);
	hid_t attribute = H5Acreate(dataGeneratorGroup, attrName, attributeStrType, attributeDataSpace, H5P_DEFAULT);
	sprintf(attrValue, "%s_%s", varName, dataName);
	H5Awrite(attribute, attributeStrType, attrValue);
	H5Aclose(attribute);

	sprintf(attrName, "comp_%d_unit", componentIndex);
	attribute = H5Acreate(dataGeneratorGroup, attrName, attributeStrType, attributeDataSpace, H5P_DEFAULT);
	H5Awrite(attribute, attributeStrType, unit.c_str());
	H5Aclose(attribute);

	H5Tclose(attributeStrType);
	H5Sclose(attributeDataSpace);
}

void VariableStatisticsDataGenerator::detailGroup(const hid_t& h5PPFile, const hid_t& dataGeneratorGroup, SimulationExpression* sim)
{
	// attributes : all the components, e.g. comp_0_name = varName, comp_0_unit = varUnit, comp_1_name = ... comp_1_unit = ...
	int numVar = sim->getNumVariables();
	int componentIndex = 0;
	for (int i = 0; i < numVar; i ++) {
		Variable* var = sim->getVariable(i);
		const char* varName = var->getName().c_str();
		std::string unit;

		//write var average name and unit
		unit = (var->getVarType() == VAR_MEMBRANE || var->getVarType() == VAR_MEMBRANE_REGION)
				? "molecules.um-2" : "uM";
		writeAttributeComponent(dataGeneratorGroup, varName, componentIndex ++, "mean", unit);

		//write var total name and unit
		unit = "molecules";
		writeAttributeComponent(dataGeneratorGroup, varName, componentIndex ++, "total", unit);

#ifndef CH_MPI
		unit = "uM";
		writeAttributeComponent(dataGeneratorGroup, varName, componentIndex ++, "min", unit);
		unit = "uM";
		writeAttributeComponent(dataGeneratorGroup, varName, componentIndex ++, "max", unit);

		if (var->getExactErrorVariable() != NULL)
		{
			unit = "molecules";
			writeAttributeComponent(dataGeneratorGroup, varName, componentIndex ++, "L2Error", unit);
			unit = "molecules";
			writeAttributeComponent(dataGeneratorGroup, varName, componentIndex ++, "maxError", unit);
		}
#endif
	}
}
