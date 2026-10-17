/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/RegionSizeVariable.h>
#include <VCELL/Structure.h>

RegionSizeVariable::RegionSizeVariable(string& nameStr, Structure* structure, int size, bool arg_bVolume)
: Variable(nameStr, structure, size)
{
	bVolume = arg_bVolume;
}
