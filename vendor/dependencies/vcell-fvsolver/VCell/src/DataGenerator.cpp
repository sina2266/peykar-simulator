/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <limits>
#include <VCELL/PostProcessingBlock.h>
#include <VCELL/DataGenerator.h>
#include <VCELL/Feature.h>

#include <limits>
const double DataGenerator::double_max = std::numeric_limits<double>::max();
const double DataGenerator::double_min = std::numeric_limits<double>::min();

DataGenerator::DataGenerator(const string& name, Feature* f) {
	this->name = name;
	this->feature = f;
	dataSize = 0;
	data = NULL;
}

DataGenerator::~DataGenerator() {
	delete[] data;
}

string DataGenerator::getQualifiedName(){
	if (feature == NULL){
		return name;
	} else {
		return feature->getName() + "::" + name;
	}
}
