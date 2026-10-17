/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/Feature.h>
#include <VCELL/Membrane.h>
#include <VCELL/VolumeVariable.h>
#include <parstream.H>
#include <assert.h>
#include <iostream>
#include <sstream>
using std::stringstream;
using std::endl;

Feature::Feature(string& name, unsigned int findex) : Structure(name)
{
	index = findex;
	ifVar = NULL;
}

Feature::~Feature()
{
	delete ifVar;
}

void Feature::resolveReferences(SimulationExpression *sim)
{
	Structure::resolveReferences(sim);
	std::map<Membrane*, BoundaryType>::iterator it = ebBcTypeMap.begin();
	if (it != ebBcTypeMap.end())
	{
		BoundaryType bt = it->second;
		for (++ it; it != ebBcTypeMap.end(); ++ it)
		{
			if (bt != it->second)
			{
				stringstream ss;
				ss << "Different EB boundary condition types are defined in feature " << getName() << ". Please make correction before proceeding.";
			}
		}
	}
}

void Feature::setEbBcType(Membrane* mem, BoundaryType bcType)
{
	pout() << "Feature " << name << ", Membrane " << mem->getName() << ", bcType="
				<< (bcType == BOUNDARY_VALUE ? "Dirichlet" : "Neumann") << endl;
	ebBcTypeMap[mem] = bcType;
}

BoundaryType Feature::getEbBcType(Membrane* mem)
{
	map<Membrane*, BoundaryType>::iterator iter = ebBcTypeMap.find(mem);
	assert(iter != ebBcTypeMap.end());
	return ebBcTypeMap[mem];
}

BoundaryType Feature::getEbBcType()
{
	// we don't allow mixed boundary condition in one feature for now.
	// so return the first boundary condition type
	std::map<Membrane*, BoundaryType>::iterator it = ebBcTypeMap.begin();
	if (it != ebBcTypeMap.end())
	{
		BoundaryType bt = it->second;
		return bt;
	}
	assert(0);
}

