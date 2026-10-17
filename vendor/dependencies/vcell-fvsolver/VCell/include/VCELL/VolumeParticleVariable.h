/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef VOLUMEPARTICLEVARIABLE_H
#define VOLUMEPARTICLEVARIABLE_H

#include <VCELL/ParticleVariable.h>

class Feature;

class VolumeParticleVariable : public ParticleVariable
{
public:
	VolumeParticleVariable(string& nameStr, Feature* feature, long sizeX, long sizeY, long sizeZ);

	virtual VariableType	getVarType() {return VAR_VOLUME_PARTICLE;}

	virtual void show(ofstream& fp);

	long getSizeX() { return sizeX;} 
	long getSizeY() { return sizeY;} 
	long getSizeZ() { return sizeZ;} 
	
protected:
	long    sizeX;
	long    sizeY;
	long    sizeZ;

};

#endif
