/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/VolumeRegionVariable.h>
#include <VCELL/Feature.h>

VolumeRegionVariable::VolumeRegionVariable(string& nameStr, Feature* feature, int size)
: Variable(nameStr, feature, size)
{
}

#ifndef CH_MPI
void VolumeRegionVariable::createErrorVariables()
{
	if (exactErrorVar == NULL)
	{
		string errorVarName = name + ERROR_VAR_SUFFIX;
		exactErrorVar = new VolumeRegionVariable(errorVarName, (Feature*)structure, size);
		errorVarName = name + RELATIVE_ERROR_VAR_SUFFIX;
		relativeErrorVar = new VolumeRegionVariable(errorVarName, (Feature*)structure, size);
	}
}
#endif