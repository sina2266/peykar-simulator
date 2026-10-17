/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef MEMBRANEPARTICLEVARIABLE_H
#define MEMBRANEPARTICLEVARIABLE_H

#include <VCELL/ParticleVariable.h>

class Membrane;

class MembraneParticleVariable : public ParticleVariable
{
public:
	MembraneParticleVariable(string& nameStr, Membrane* membrane, long size);

	VariableType getVarType() { return VAR_MEMBRANE_PARTICLE; }
};

#endif
