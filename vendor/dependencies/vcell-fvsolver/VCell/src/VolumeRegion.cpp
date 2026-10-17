/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/Element.h>
#include <VCELL/VolumeRegion.h>
#include <VCELL/CartesianMesh.h>
#include <VCELL/Feature.h>

VolumeRegion::VolumeRegion(int vrindex, string& vrname, Mesh* mesh, Feature* parent) : Region(vrindex, vrname, mesh)
{
	feature = parent;
	bAdjacentToBoundary = false;
	bBoundaryDirichlet = false;
}

void VolumeRegion::addMembraneRegion(MembraneRegion *membraneRegion)
{
	membraneRegionList.push_back(membraneRegion);
}

double VolumeRegion::getSize()
{
	if(size == 0){
		for(long i=0; i < (long)elementIndices.size(); i++){
			size += mesh->getVolumeOfElement_cu(elementIndices[i]);
		}
	}
	return size;     
}

Feature* VolumeRegion::getFeature()
{
	return feature;     
}

void VolumeRegion::setFeature(Feature* newFeature)
{
	feature = newFeature;     
}
