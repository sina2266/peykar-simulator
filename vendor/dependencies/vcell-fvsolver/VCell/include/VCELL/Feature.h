/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef FEATURE_H
#define FEATURE_H

#include <VCELL/Structure.h>
#include <vector>
using std::vector;

class VolumeVarContextExpression;
class VolumeRegionVarContextExpression;
class FastSystem;
class Feature;
class SimulationExpression;
class VolumeVariable;
class MembraneVariable;
class VolumeRegionVariable;
class MembraneRegionVariable;

class Feature : public Structure
{
public:
	Feature(string& name, unsigned char findex, FeatureHandle handle);

	void resolveReferences(SimulationExpression *sim);
	virtual void initVolumeValues(long volumeIndex);
	virtual void initVolumeRegionValues(int volumeRegionIndex);

	FeatureHandle   getHandle();
	unsigned char getIndex() const { return index; }

	VolumeVarContextExpression *getVolumeVarContext(VolumeVariable *var);
	VolumeRegionVarContextExpression *getVolumeRegionVarContext(VolumeRegionVariable *var);
	   
	void addVolumeVarContext(VolumeVarContextExpression *vc);	
	void addVolumeRegionVarContext(VolumeRegionVarContextExpression *vc);	
	   
	void reinitConstantValues(SimulationExpression* sim);

	//VolumeParticleContext     *getVolumeParticleContext(){return vpc;}
	//MembraneParticleContext   *getMembraneParticleContext(){return mpc;}
	//ContourParticleContext    *getContourParticleContext(){return cpc;}
	//VolumeParticleContext     *setVolumeParticleContext(VolumeParticleContext *Avpc);
	//MembraneParticleContext   *setMembraneParticleContext(MembraneParticleContext *Ampc);
	//ContourParticleContext    *setContourParticleContext(ContourParticleContext *Acpc);	
   
protected:

	//VolumeParticleContext     *vpc;
	//MembraneParticleContext   *mpc;
	//ContourParticleContext    *cpc;
	
	vector<VolumeVarContextExpression*> volumeVarContextList;
	vector<VolumeRegionVarContextExpression*> volumeRegionVarContextList;
	   
	FeatureHandle handle;
	unsigned char index;
};  

#endif
