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
