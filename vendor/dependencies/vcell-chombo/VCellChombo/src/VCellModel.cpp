/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/VCellModel.h>
#include <VCELL/Feature.h>
#include <VCELL/SimTool.h>
#include <VCELL/Membrane.h>

#include <assert.h>

VCellModel::VCellModel(SimTool* a_simTool) : simTool(a_simTool)
{
}

VCellModel::~VCellModel()
{      
	for (int i = 0; i < (int)featureList.size(); i ++) {
		delete featureList[i];
	}
	featureList.clear();
	for (int i = 0; i < (int)membraneList.size(); i ++) {
		delete membraneList[i];
	}
	membraneList.clear();
 }
                         
Feature* VCellModel::addFeature(string& name)
{
	unsigned char findex = (unsigned char)featureList.size();
	Feature* feature = new Feature(name, findex);
	featureList.push_back(feature);
	return feature;
}

Feature *VCellModel::getFeatureFromIndex(int index)
{
	if (index < 0 || index >= (int)featureList.size()) {
		throw "VCellModel: getFeature(index) : index out of bounds";
	}
	return featureList.at(index);
}

Feature *VCellModel::getFeatureFromName(const string& name)
{
	for (int i = 0; i < (int)featureList.size(); i ++) {
		Feature* feature = featureList[i];
		if (name == feature->getName()) {
			return feature;
		}
	}
	return NULL;
}

void VCellModel::resolveReferences()
{	
	SimulationExpression* sim = SimTool::getInstance()->getSimulation();
	for (int i = 0; i < (int)featureList.size(); i ++) {
		Feature* feature = featureList[i];
		feature->resolveReferences(sim);		
	}	
	for (int i = 0; i < (int)membraneList.size(); i ++) {
		membraneList[i]->resolveReferences(sim);		
	}
}

Membrane* VCellModel::getMembrane(Feature* f1, Feature* f2)
{
	for (int i = 0; i < (int)membraneList.size(); i ++) {
		Membrane* membrane = membraneList[i];
		if (membrane->inBetween(f1, f2)) {
			return membrane;
		}
	}
	return 0;
}

int VCellModel::getMembraneIndex(Feature* f1, Feature* f2)
{
	for (int i = 0; i < (int)membraneList.size(); i ++) {
		Membrane* membrane = membraneList[i];
		if (membrane->inBetween(f1, f2)) {
			return i;
		}
	}
	return -1;
}

Membrane *VCellModel::getMembraneFromIndex(int index)
{
	if (index < 0 || index >= (int)membraneList.size()) {
		throw "VCellModel: getMembrane(index) : index out of bounds";
	}
	return membraneList.at(index);
}

Membrane* VCellModel::getMembraneFromName(string& mem_name)
{
	for (int i = 0; i < (int)membraneList.size(); i ++) {
		Membrane* membrane = membraneList[i];
		if (membrane->getName() == mem_name) {
			return membrane;
		}
	}
	//assert(0);
	return 0;
}

Membrane* VCellModel::addMembrane(string& name, string& feature1_name, string& feature2_name) {
	Feature* f1 = getFeatureFromName(feature1_name);
	Feature* f2 = getFeatureFromName(feature2_name);

	assert(f1 != 0 && f2 != 0);

	Membrane* membrane = new Membrane(name, f1, f2);
	membraneList.push_back(membrane);

	return membrane;
}
