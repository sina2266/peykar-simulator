/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/Simulation.h>
#include <VCELL/Scheduler.h>
#include <VCELL/Feature.h>
#include <VCELL/Membrane.h>
//#include <VCELL/ParticleContext.h>
#include <VCELL/VCellModel.h>
#include <VCELL/FastSystem.h>
#include <VCELL/VolumeRegion.h>
#include <VCELL/MembraneRegion.h>
#include <VCELL/SimTool.h>
#include <VCELL/CartesianMesh.h>
#include <VCELL/Element.h>

Scheduler::Scheduler(Simulation *Asim)
{
	sim = Asim;
	bFirstTime = true;
	bHasFastSystem = false;
}

void Scheduler::initValues(VCellModel* model)
{
	bFirstTime = true;
	//
	// tell Features to initialize the variables with 
	// feature specific values
	//
	Mesh *mesh = sim->getMesh();
	VolumeElement *pVolumeElement = mesh->getVolumeElements();
	for (int i=0;i<mesh->getNumVolumeElements();i++){
		Feature* feature = pVolumeElement[i].getFeature();
		if (feature != 0) { 
			feature->initVolumeValues(i);
		} else {
			char errmsg[512];
			sprintf(errmsg, "feature is null for volume element %d", i);
			throw errmsg;
		} 
	}

	MembraneElement *pMembraneElement = mesh->getMembraneElements();
	for (int i=0;i<mesh->getNumMembraneElements();i++){
		Membrane* membrane = pMembraneElement[i].getMembrane();
		if (membrane != 0) { 
			membrane->initMembraneValues(&pMembraneElement[i]);
		} else {
			char errmsg[512];
			sprintf(errmsg, "membrane is null for membrane element %d", i);
			throw errmsg;
		}
	}

	int numVolumeRegions = ((CartesianMesh*)mesh)->getNumVolumeRegions();
	for(int j=0; j<numVolumeRegions; j++){
		VolumeRegion *region = ((CartesianMesh*)mesh)->getVolumeRegion(j);
		Feature* feature = region->getFeature();
		if (feature != 0) { 
			feature->initVolumeRegionValues(j);
		} else {
			char errmsg[512];
			sprintf(errmsg, "feature is null for volume region element %d", j);
			throw errmsg;
		} 
	}

	int numMembraneRegions = ((CartesianMesh*)mesh)->getNumMembraneRegions();
	for(int j=0; j<numMembraneRegions; j++){
		MembraneRegion *membraneRegion = ((CartesianMesh*)mesh)->getMembraneRegion(j);
		Membrane* membrane = membraneRegion->getMembrane();
		if (membrane != 0) { 
			membrane->initMembraneRegionValues(j);
		} else {
			char errmsg[512];
			sprintf(errmsg, "membrane is null for membrane region element %d", j);
			throw errmsg;
		} 
	}

	bHasFastSystem = model->hasFastSystem();
}

void Scheduler::solveFastSystem(SimTool* sim_tool, int volStart, int volSize, int memStart, int memSize)
{
	Feature *feature = NULL;
	FastSystem *fs = NULL;
	Mesh *mesh = sim->getMesh();

	ASSERTION(volSize>=0);
	ASSERTION(memSize>=0);
	ASSERTION(volStart >= 0 && (volStart+volSize) <= mesh->getNumVolumeElements());
	ASSERTION(memStart >= 0 && (memStart+memSize) <= mesh->getNumMembraneElements());
	//
	// perform pseudo-steady approximation on volume elements
	//
	VolumeElement *pVolumeElement = mesh->getVolumeElements();
	long i;
	for (i=volStart;i < volStart+volSize;i++){
		feature = pVolumeElement->getFeature();
		ASSERTION(feature);
		if (fs = feature->getFastSystem()){
			fs->setCurrIndex(i);
			WorldCoord wc = mesh->getVolumeWorldCoord(i);
			fs->setCoordinates(sim->getTime_sec(sim_tool), wc);
			fs->initVars(sim_tool);
			fs->solveSystem();
			fs->updateVars();			
		}
		pVolumeElement++;
	}
	//
	// perform pseudo-steady approximation on membrane elements
	//
	MembraneElement *pMembraneElement = mesh->getMembraneElements();
	for (i=memStart;i < memStart+memSize;i++){
		Membrane* membrane = pMembraneElement->getMembrane();
		ASSERTION(membrane);
		if (fs = membrane->getFastSystem()){
			fs->setCurrIndex(i);
			WorldCoord wc = mesh->getMembraneWorldCoord(i);
			fs->setCoordinates(sim->getTime_sec(sim_tool), wc);
			fs->initVars(sim_tool);
			fs->solveSystem();
			fs->updateVars();			
		}
		pMembraneElement++;
	}
}
