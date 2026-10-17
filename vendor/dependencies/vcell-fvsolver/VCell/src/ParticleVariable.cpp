/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */

#include <VCELL/ParticleVariable.h>
#include <VCELL/Structure.h>
#include <math.h>
#include <string.h>


ParticleVariable::ParticleVariable(string& nameStr, Structure* structure, long size) : Variable(nameStr, structure, size, true)
{
	moleculeCounts = new double[size];
}

ParticleVariable::~ParticleVariable()
{
	delete[] moleculeCounts;
}