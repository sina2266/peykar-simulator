/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef VOLUMEREGIONVARIABLE_H
#define VOLUMEREGIONVARIABLE_H

#include <VCELL/Variable.h>

class Feature;

class VolumeRegionVariable : public Variable
{
public:
	VolumeRegionVariable(string& nameStr, Feature* feature, int size);

	VariableType getVarType() { return VAR_VOLUME_REGION; }
};

#endif
