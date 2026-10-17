/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef VCELLMODEL_H
#define VCELLMODEL_H 

#include <VCELL/SimTypes.h>
#include <vector>
using std::vector;
using std::string;

class Feature;
class Membrane;
class SimulationExpression;
struct MembraneElement;

class VCellModel
{
public:
	VCellModel();
	~VCellModel();

	int getNumFeatures() const
	{
		return static_cast<int>(featureList.size());
	}
	Feature* addFeature(string& name, FeatureHandle handle);
	Feature* getFeatureFromHandle(FeatureHandle handle);
	Feature* getFeatureFromName(string&  name);
	Feature* getFeatureFromIndex(int index);

	int getNumMembranes() const
	{
		return static_cast<int>(membraneList.size());
	}
	Membrane* addMembrane(string& name, string& feature1_name, string& feature2_name);
	Membrane* getMembraneFromIndex(int index);
	Membrane* getMembraneFromName(string& mem_name);
	Membrane* getMembrane(Feature* f1, Feature* f2);
	   
	void resolveReferences(SimulationExpression* sim);

	bool hasFastSystem();
   
private:
	vector<Feature*> featureList;
//	vector<Contour*> pContours;
	vector<Membrane*> membraneList;
};

#endif
