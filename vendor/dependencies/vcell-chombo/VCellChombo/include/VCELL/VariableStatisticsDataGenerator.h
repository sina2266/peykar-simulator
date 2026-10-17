/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef VARIABLE_STATISTICS_DATA_GENERATOR_H
#define VARIABLE_STATISTICS_DATA_GENERATOR_H

#include <VCELL/DataGenerator.h>
#include <vector>
using std::vector;

#include <hdf5.h>

class VariableStatisticsDataGenerator : public DataGenerator
{
public:
	VariableStatisticsDataGenerator();
	virtual ~VariableStatisticsDataGenerator();

	void resolveReferences(SimulationExpression* sim);
	void computePPData(SimulationExpression* sim);

	void detailGroup(const hid_t& h5PPFile, const hid_t& dataGeneratorGroup, SimulationExpression* sim);

private:
	void writeAttributeComponent(const hid_t& dataGeneratorGroup,
			const char* varName, int componentIndex, const char* dataName, const string& unit);
	static const string VariableStatistics_Name;
};

#endif
