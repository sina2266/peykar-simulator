/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef VCELLMODEL_H
#define VCELLMODEL_H 

#include <string>
#include <vector>
using std::vector;
using std::string;

class Feature;
class Membrane;
class SimTool;

class VCellModel
{
public:
	VCellModel(SimTool* simTool);
	~VCellModel();

	int getNumFeatures() {
		return (int)featureList.size();
	}
	Feature* addFeature(string& name);
	Feature* getFeatureFromName(const string&  name);
	Feature* getFeatureFromIndex(int index);

	int getNumMembranes() {
		return (int)membraneList.size();
	}
	Membrane* addMembrane(string& name, string& feature1_name, string& feature2_name);
	Membrane* getMembraneFromIndex(int index);
	Membrane* getMembraneFromName(string& mem_name);
	Membrane* getMembrane(Feature* f1, Feature* f2);
	int getMembraneIndex(Feature* f1, Feature* f2);
	   
	void resolveReferences();

	SimTool* getSimTool()
	{
		return simTool;
	}
	
private:
	vector<Feature*> featureList;
	vector<Membrane*> membraneList;

	SimTool* simTool;
};

#endif
