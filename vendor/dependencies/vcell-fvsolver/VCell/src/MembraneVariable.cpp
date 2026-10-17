/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */

#include <VCELL/MembraneVariable.h>
#include <VCELL/Membrane.h>

MembraneVariable::MembraneVariable(string& nameStr, Membrane* membrane, long size, bool diff)
: Variable(nameStr, membrane, size, diff)
{
}
