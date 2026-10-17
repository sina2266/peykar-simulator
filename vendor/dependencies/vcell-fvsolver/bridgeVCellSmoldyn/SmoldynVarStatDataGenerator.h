/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef SMOLDYN_VAR_STAT_GENERATOR_H
#define SMOLDYN_VAR_STAT_GENERATOR_H

#include "SmoldynDataGenerator.h"

class SmoldynVarStatDataGenerator : public SmoldynDataGenerator
{
public:
	SmoldynVarStatDataGenerator();
	~SmoldynVarStatDataGenerator();

	void initialize(VCellSmoldynOutput* vso);
	void computePPData(VCellSmoldynOutput* vso);

private:
	static const string VariableStatistics_Name;
};

#endif
