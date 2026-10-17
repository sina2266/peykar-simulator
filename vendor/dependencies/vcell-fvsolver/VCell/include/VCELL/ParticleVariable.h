/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
//--------------------------------------------
// ParticleVariable.h
//--------------------------------------------
#ifndef PARTICLE_VARIABLE_H
#define PARTICLE_VARIABLE_H

#include <VCELL/Variable.h>

class ParticleVariable : public Variable
{
public:
	ParticleVariable(string& nameStr,  Structure* structure, long size);
	~ParticleVariable();
	double* getMoleculeCounts(){return moleculeCounts;}
private:
	double *moleculeCounts; //list of molecule counts corresponding volume/membrane elements
};

#endif
