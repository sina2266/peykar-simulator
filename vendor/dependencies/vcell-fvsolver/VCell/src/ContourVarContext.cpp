/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/Feature.h>
#include <VCELL/Contour.h>
#include <VCELL/ContourVarContext.h>
#include <VCELL/MembraneRegion.h>
#include <VCELL/CartesianMesh.h>

ContourVarContext::ContourVarContext(Contour *Acontour, Feature *Afeature, string& AspeciesName)
: VarContext(Afeature, AspeciesName)
{
	contour = Acontour;
}
