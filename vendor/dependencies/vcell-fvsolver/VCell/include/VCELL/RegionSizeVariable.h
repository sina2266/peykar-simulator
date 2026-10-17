/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef REGIONSIZEVARIABLE_H
#define REGIONSIZEVARIABLE_H

#include <VCELL/Variable.h>

class Structure;

class RegionSizeVariable : public Variable
{
public:
	RegionSizeVariable(string& nameStr, Structure* structure, int size, bool bVolume);

	VariableType	getVarType() {
		return bVolume ? VAR_VOLUME_REGION : VAR_MEMBRANE_REGION;
	}

private:
	bool bVolume;
};

#endif
