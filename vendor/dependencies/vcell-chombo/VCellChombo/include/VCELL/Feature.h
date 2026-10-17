/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef FEATURE_H
#define FEATURE_H

#include <VCELL/Structure.h>
#include <vector>
#include <map>
using std::vector;
using std::map;

class Membrane;
class VolumeVariable;
class Feature : public Structure
{
public:
	Feature(string& name, unsigned int findex);
	~Feature();

	unsigned int getIndex() {
		return index;
	}
	virtual void resolveReferences(SimulationExpression *sim);
	
	vector<int>& getMemVarIndexesInAdjacentMembranes() {
		return memVarIndexesInAdjacentMembranes;
	}
	
	void addMemVarIndexInAdjacentMembrane(int index) {
		memVarIndexesInAdjacentMembranes.push_back(index);
	}

	void setPhase(int p)
	{
		phase = p;
	}
	
	int getPhase()
	{
		return phase;
	}

	void setEbBcType(Membrane* mem, BoundaryType bcType);
	BoundaryType getEbBcType(Membrane* mem);
	BoundaryType getEbBcType();

	VolumeVariable* getIFVariable()
	{
		return ifVar;
	}
	void setIFVariable(VolumeVariable* var)
	{
		ifVar = var;
	}
private:
	unsigned int index;
	vector<int> memVarIndexesInAdjacentMembranes;

	int phase;
	map<Membrane*, BoundaryType> ebBcTypeMap;
	VolumeVariable* ifVar;
};  

#endif
