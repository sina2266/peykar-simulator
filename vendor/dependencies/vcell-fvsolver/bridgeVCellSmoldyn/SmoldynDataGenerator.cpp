/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include "SmoldynDataGenerator.h"
#include <limits>
const double SmoldynDataGenerator::double_max = std::numeric_limits<double>::max();
const double SmoldynDataGenerator::double_min = std::numeric_limits<double>::min();

SmoldynDataGenerator::SmoldynDataGenerator(const string& name) {
	this->name = name;
	dataSize = 0;
	data = NULL;
}

SmoldynDataGenerator::~SmoldynDataGenerator() {
	delete[] data;
}

