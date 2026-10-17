/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/PostProcessingBlock.h>
#include <VCELL/DataGenerator.h>

PostProcessingBlock::PostProcessingBlock(SimulationExpression* sim) {
	this->simulation = sim;
}

PostProcessingBlock::~PostProcessingBlock() {
	vector<DataGenerator*>::iterator it;
	for(it = dataGeneratorList.begin(); it < dataGeneratorList.end(); ++ it) {
		delete *it;
	}
	dataGeneratorList.clear();
}

void PostProcessingBlock::addDataGenerator(DataGenerator* dataGenerator) {
	dataGeneratorList.push_back(dataGenerator);
}

void PostProcessingBlock::resolveReferences() {
	vector<DataGenerator*>::iterator it;
	for(it = dataGeneratorList.begin(); it < dataGeneratorList.end(); ++ it) {
		(*it)->resolveReferences(simulation);
	}
}
