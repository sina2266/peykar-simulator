/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef MEMBRANEVARIABLE_H
#define MEMBRANEVARIABLE_H

#include <VCELL/Variable.h>

class Membrane;

class MembraneVariable : public Variable
{
public:
	MembraneVariable(string& nameStr, Membrane* membrane, long size, bool diff=false);

	VariableType getVarType() { return VAR_MEMBRANE; }
};

#endif
