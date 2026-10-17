/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef MEMBRANEREGION_H
#define MEMBRANEREGION_H 

#include <VCELL/Region.h>

class Feature;
class Mesh;
class CartesianMesh;
class VolumeRegion;
class MembraneRegion;
class Membrane;
class VolumeRegion;

class MembraneRegion : public Region
{
public:
	MembraneRegion(int mrindex, string& mrname, Mesh *mrmesh, Membrane* parent, VolumeRegion* volReg1, VolumeRegion* volReg2);
	Membrane* getMembrane() {
		return membrane;
	}
	bool isAdjacentToBoundary();
	bool inBetween(VolumeRegion* vr1, VolumeRegion* vr2);
	VolumeRegion* getVolumeRegion1() {
		return volRegion1;
	}
	VolumeRegion* getVolumeRegion2() {
		return volRegion2;
	}
	double getSize();
	    
private:
	VolumeRegion *volRegion1, *volRegion2;
	Membrane* membrane;
};

#endif
