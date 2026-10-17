/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/MembraneRegionVariable.h>
#include <VCELL/Membrane.h>

MembraneRegionVariable::MembraneRegionVariable(string& nameStr, Membrane* membrane, long size)
: Variable(nameStr, membrane, size)
{
}

#ifndef CH_MPI
void MembraneRegionVariable::createErrorVariables()
{
	if (exactErrorVar == NULL)
	{
		string errorVarName = name + ERROR_VAR_SUFFIX;
		exactErrorVar = new MembraneRegionVariable(errorVarName, (Membrane*)structure, size);
		errorVarName = name + RELATIVE_ERROR_VAR_SUFFIX;
		relativeErrorVar = new MembraneRegionVariable(errorVarName, (Membrane*)structure, size);
	}
}
#endif