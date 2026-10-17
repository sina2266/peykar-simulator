/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef MEMBRANEREGIONVARIABLE_H
#define MEMBRANEREGIONVARIABLE_H

#include <VCELL/Variable.h>

class Membrane;

class MembraneRegionVariable : public Variable
{
public:
	MembraneRegionVariable(string& nameStr, Membrane* membrane, long size);

	VariableType getVarType() { return VAR_MEMBRANE_REGION; }
};

#endif
