/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef VOLUMEREGION_H
#define VOLUMEREGION_H 

#include <VCELL/Region.h>

class Feature;
class CartesianMesh;
class MembraneRegion;


class VolumeRegion : public Region
{
public:
	VolumeRegion(int vrindex, string& vrname, Mesh* mesh, Feature* parent);
	Feature *getFeature();
	void setFeature(Feature* newFeature);
	void addMembraneRegion(MembraneRegion *membraneRegion);
	MembraneRegion *getMembraneRegion(int i) { return membraneRegionList[i]; }
	int getNumMembraneRegions() { return (int)membraneRegionList.size(); }
    
	bool isAdjacentToBoundary() {
		return bAdjacentToBoundary;
	}
	void setAdjacentToBoundary() {
		bAdjacentToBoundary = true;
	}
	void setBoundaryDirichlet() { 
		bBoundaryDirichlet = true; 
	}
	bool isBoundaryDirichlet() { 
		return bBoundaryDirichlet; 
	}
	double getSize();
	
private:
	Feature *feature;
	vector<MembraneRegion*> membraneRegionList;
	bool bAdjacentToBoundary; //denotes whether the region touches the wall
	bool bBoundaryDirichlet;
};

#endif
