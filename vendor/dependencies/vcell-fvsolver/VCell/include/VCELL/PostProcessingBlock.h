/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef POST_PROCESSING_BLOCK_H
#define POST_PROCESSING_BLOCK_H

#include <VCELL/SimulationExpression.h>
#include <SymbolTable.h>
#include <vector>
using std::vector;

class DataGenerator;

class PostProcessingBlock/* : public SymbolTable*/
{
	friend class PostProcessingHdf5Writer;

public:
	PostProcessingBlock(SimulationExpression* sim);
	virtual ~PostProcessingBlock();

	void addDataGenerator(DataGenerator* dataGenerator);

	void resolveReferences();

	int getNumDataGenerators() {
		return dataGeneratorList.size();
	}

	const vector<DataGenerator*>& getDataGeneratorList() const {
		return dataGeneratorList;
	}

	SimulationExpression* getSimulation(){
		return simulation;
	}
	//SymbolTableEntry* getEntry(string identifierString); 

private:
	SimulationExpression* simulation;
	vector<DataGenerator*> dataGeneratorList;
};

#endif
