/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/Variable.h>
#include <VCELL/Mesh.h>
#include <VCELL/EqnBuilder.h>

EqnBuilder::EqnBuilder(Variable *Avar, Mesh *Amesh)
{
	var     = Avar;
	mesh    = Amesh;
}
