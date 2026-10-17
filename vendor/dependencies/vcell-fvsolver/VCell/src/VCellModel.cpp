/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/VCellModel.h>
#include <VCELL/Feature.h>
//#include <VCELL/Contour.h>
#include <VCELL/Element.h>
#include <VCELL/Simulation.h>
#include <VCELL/SimTool.h>
#include <VCELL/Membrane.h>

#include <assert.h>
#include <sstream>
using std::stringstream;

VCellModel::VCellModel()
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
                         
Feature* VCellModel::addFeature(string& name, FeatureHandle handle)
{
	unsigned char findex = (unsigned char)featureList.size();
	Feature* feature = new Feature(name, findex, handle);
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

Feature *VCellModel::getFeatureFromHandle(FeatureHandle handle)
{
	for (int i = 0; i < (int)featureList.size(); i ++) {
		Feature* feature = featureList[i];
		if (handle == feature->getHandle()) {
			return feature;
		}
	}
	return NULL;
}

Feature *VCellModel::getFeatureFromName(string& name)
{
	if (name == "null") {
		return 0;
	}
	for (int i = 0; i < (int)featureList.size(); i ++) {
		Feature* feature = featureList[i];
		if (name == feature->getName()) {
			return feature;
		}
	}
	stringstream ss;
	ss << "Compartment subdomain '" << name << "' doesn't exist!";
	throw ss.str();
}

void VCellModel::resolveReferences(SimulationExpression* sim)
{
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
	stringstream ss;
	ss << "With current mesh sampling, unexpected membrane found in between " 
		<< f1->getName() << " and " << f2->getName() << ", considering using finer mesh.";
	throw ss.str();
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
	if (mem_name == "null") {
		return 0;
	}
	for (int i = 0; i < (int)membraneList.size(); i ++) {
		Membrane* membrane = membraneList[i];
		if (membrane->getName() == mem_name) {
			return membrane;
		}
	}
	stringstream ss;
	ss << "Membrane subdomain '" << mem_name << "' doesn't exist!";
	throw ss.str();
}

bool VCellModel::hasFastSystem() {
	for (int i = 0; i < (int)featureList.size(); i ++) {
		if(featureList[i]->getFastSystem()){
			return true;
		}
	}	

	for (int i = 0; i < (int)membraneList.size(); i ++) {
		if(membraneList[i]->getFastSystem()){
			return true;
		}
	}
	return false;
}

Membrane* VCellModel::addMembrane(string& name, string& feature1_name, string& feature2_name) {
	Feature* f1 = getFeatureFromName(feature1_name);
	Feature* f2 = getFeatureFromName(feature2_name);

	assert(f1 != 0 && f2 != 0);

	Membrane* membrane = new Membrane(name, f1, f2);
	membraneList.push_back(membrane);

	return membrane;
}

//void VCellModel::addContour(Contour *contour)
//{
//	pContours.push_back(contour);
//}
//
//Contour *VCellModel::getContour(int index)
//{
//	return pContours[index];
//}
//
//int VCellModel::getNumContours()
//{
//	return (int)pContours.size();
//}
