/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/VolumeVariable.h>
#include <VCELL/Feature.h>
#include <fstream>
#include <REAL.H>
using std::ofstream;
using std::endl;

VolumeVariable::VolumeVariable(string& nameStr, Feature* feature, long size, long extrapSize)
:	Variable(nameStr, feature, size)
{
	bAdvecting = false;
	extrapolatedSize = extrapSize;
	extrapolated = NULL;
	if (extrapolatedSize > 0)
	{
		extrapolated = new double[extrapolatedSize];
		for (int i = 0; i < extrapolatedSize; ++ i)
		{
			extrapolated[i] = BASEFAB_REAL_SETVAL;
		}
	}
}

VolumeVariable::~VolumeVariable()
{
	delete[] extrapolated;
}

#ifndef CH_MPI
void VolumeVariable::createErrorVariables()
{
	if (exactErrorVar == NULL)
	{
		string errorVarName = name + ERROR_VAR_SUFFIX;
		exactErrorVar = new VolumeVariable(errorVarName, (Feature*)structure, size, 0);
		errorVarName = name + RELATIVE_ERROR_VAR_SUFFIX;
		relativeErrorVar = new VolumeVariable(errorVarName, (Feature*)structure, size, 0);
	}
}
#endif