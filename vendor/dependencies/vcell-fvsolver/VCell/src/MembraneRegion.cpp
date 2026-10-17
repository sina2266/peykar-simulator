/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/MembraneRegion.h>
#include <VCELL/VolumeRegion.h>
#include <VCELL/CartesianMesh.h>
#include <VCELL/Element.h>

MembraneRegion::MembraneRegion(int mrindex, string& mrname, Mesh *mrmesh, Membrane* parent, VolumeRegion* volReg1, VolumeRegion* volReg2) : Region(mrindex, mrname, mrmesh)
{
	membrane = parent;
	volRegion1 = volReg1;
	volRegion2 = volReg2;	
}

double MembraneRegion::getSize()
{
	if(size == 0){
		for(long i = 0; i < (long)elementIndices.size(); i ++){
			size += (mesh->getMembraneElements()+getElementIndex(i))->area;
		}
	}
	return size;
}

bool MembraneRegion::inBetween(VolumeRegion* vr1, VolumeRegion* vr2) {
	return (vr1 == volRegion1 && vr2 == volRegion2 || vr2 == volRegion1 && vr1 == volRegion2);
}

bool MembraneRegion::isAdjacentToBoundary() {
	return volRegion1->isAdjacentToBoundary() && volRegion2->isAdjacentToBoundary();
}
