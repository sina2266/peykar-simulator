/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/VolumeVariable.h>
#include <VCELL/VolumeRegionVariable.h>
#include <VCELL/Simulation.h>
#include <VCELL/Feature.h>
#include <VCELL/VolumeVarContextExpression.h>
#include <VCELL/VolumeRegionVarContextExpression.h>
//#include <VCELL/ParticleContext.h>
#include <VCELL/FastSystem.h>

//-----------------------------------------------------------------
//
//  class Feature
//
//-----------------------------------------------------------------
Feature::Feature(string& name, unsigned char findex, FeatureHandle Ahandle) : Structure(name)
{
	index = findex;
	handle = Ahandle;

	//vpc = NULL;
	//mpc = NULL;
	//cpc = NULL;
	   
	fastSystem = NULL;
}

//double Feature::getMaxIterationTime()
//{
//	return 0.0;
//}

void Feature::resolveReferences(SimulationExpression *sim)
{	
	for (int i = 0; i < (int)volumeVarContextList.size(); i ++) {
		VolumeVarContextExpression* volumeVarContext = volumeVarContextList[i];
		volumeVarContext->resolveReferences(sim);
	}
	
	for (int i = 0; i < (int)volumeRegionVarContextList.size(); i ++) {
		VolumeRegionVarContextExpression *volumeRegionVarContext = volumeRegionVarContextList[i];
		volumeRegionVarContext->resolveReferences(sim);
	}    

	//if(vpc!=NULL){
	//	vpc->resolveReferences(sim);
	//}
	//if(mpc!=NULL){
	//	mpc->resolveReferences(sim);
	//}
	//if(cpc!=NULL){
	//	cpc->resolveReferences(sim);
	//}
	if(fastSystem!=NULL){
		fastSystem->resolveReferences(sim);
	}
}

void Feature::initVolumeValues(long volumeIndex)
{
	for (int i = 0; i < (int)volumeVarContextList.size(); i ++) {
		VolumeVarContextExpression *volumeVarContext = volumeVarContextList[i];
		double value = volumeVarContext->getInitialValue(volumeIndex);
		VolumeVariable* var = (VolumeVariable *)volumeVarContext->getVar();
		var->setOld(volumeIndex, value);
		var->setCurr(volumeIndex, value);
	}
}

void Feature::initVolumeRegionValues(int volumeRegionIndex)
{
	for (int i = 0; i < (int)volumeRegionVarContextList.size(); i ++) {
		VolumeRegionVarContextExpression *volumeRegionVarContext = volumeRegionVarContextList[i];
		double value = volumeRegionVarContext->getInitialValue(volumeRegionIndex);
		VolumeRegionVariable* var = (VolumeRegionVariable*)volumeRegionVarContext->getVar();
		var->setOld(volumeRegionIndex, value);
		var->setCurr(volumeRegionIndex, value);
	}
}

void Feature::addVolumeVarContext(VolumeVarContextExpression *vvc)
{
	volumeVarContextList.push_back(vvc);
}

void Feature::addVolumeRegionVarContext(VolumeRegionVarContextExpression *vrvc)
{
	volumeRegionVarContextList.push_back(vrvc);
}

FeatureHandle Feature::getHandle()
{
	return handle;
}

VolumeVarContextExpression* Feature::getVolumeVarContext(VolumeVariable *volVar)
{
	for (int i = 0; i < (int)volumeVarContextList.size(); i ++) {
		if (volumeVarContextList[i]->getVar() == volVar) {
			return volumeVarContextList[i];
		}
	}
	return 0;
}

VolumeRegionVarContextExpression* Feature::getVolumeRegionVarContext(VolumeRegionVariable *volRegionVar)
{
	for (int i = 0; i < (int)volumeRegionVarContextList.size(); i ++) {
		if (volumeRegionVarContextList[i]->getVar() == volRegionVar) {
			return volumeRegionVarContextList[i];
		}
	}
	return 0;
}

void Feature::reinitConstantValues(SimulationExpression* sim) {
	for (int i = 0; i < (int)volumeVarContextList.size(); i ++) {
		volumeVarContextList[i]->reinitConstantValues(sim);
	}

	for (int i = 0; i < (int)volumeRegionVarContextList.size(); i ++) {
		volumeRegionVarContextList[i]->reinitConstantValues(sim);
	}
}
