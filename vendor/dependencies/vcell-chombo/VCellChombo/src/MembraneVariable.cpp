/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */

#include <VCELL/MembraneVariable.h>
#include <VCELL/Membrane.h>
#include <string.h>
#include <REAL.H>

MembraneVariable::MembraneVariable(string& nameStr, Membrane* membrane, long size)
: Variable(nameStr, membrane, size)
{
}

MembraneVariable::~MembraneVariable()
{
}

#ifndef CH_MPI
void MembraneVariable::createErrorVariables()
{
	if (exactErrorVar == NULL)
	{
		string errorVarName = name + ERROR_VAR_SUFFIX;
		exactErrorVar = new MembraneVariable(errorVarName, (Membrane*)structure, size);
		errorVarName = name + RELATIVE_ERROR_VAR_SUFFIX;
		relativeErrorVar = new MembraneVariable(errorVarName, (Membrane*)structure, size);
	}
}
#endif