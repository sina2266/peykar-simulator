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
