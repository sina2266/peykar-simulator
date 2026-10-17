/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */

#include <VCELL/MembraneParticleVariable.h>
#include <VCELL/Membrane.h>

MembraneParticleVariable::MembraneParticleVariable(string& nameStr, Membrane* membrane, long size)
: ParticleVariable(nameStr, membrane, size)
{
}
