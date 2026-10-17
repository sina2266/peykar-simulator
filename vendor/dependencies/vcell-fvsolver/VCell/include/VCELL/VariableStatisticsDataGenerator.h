/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef VARIABLE_STATISTICS_DATA_GENERATOR_H
#define VARIABLE_STATISTICS_DATA_GENERATOR_H

#include <VCELL/DataGenerator.h>
#include <hdf5.h>

struct StatData
{
	double average;
	double total;
	double min;
	double max;

	static H5::CompType getType() 
	{
		H5::PredType dtype = H5::PredType::NATIVE_DOUBLE;
		H5::CompType compType(sizeof(StatData));
		compType.insertMember("average", HOFFSET(StatData, average), dtype);
		compType.insertMember("total", HOFFSET(StatData, total), dtype);
		compType.insertMember("min", HOFFSET(StatData, min), dtype);
		compType.insertMember("max", HOFFSET(StatData, max), dtype);
		return compType;
	}
};

class VariableStatisticsDataGenerator : public DataGenerator
{
public:
	VariableStatisticsDataGenerator();
	virtual ~VariableStatisticsDataGenerator();

	void resolveReferences(SimulationExpression* sim);
	void computePPData(SimulationExpression* sim);

	void detailGroup(H5::H5File* h5PPFile, H5::Group& dataGeneratorGroup, SimulationExpression* sim);

private:
	void writeAttributeComponent(H5::Group& dataGeneratorGroup, const char* varName, 
		int componentIndex, const char* dataName, const string& unit);
	static const string VariableStatistics_Name;
	StatData* statData;
};

#endif
