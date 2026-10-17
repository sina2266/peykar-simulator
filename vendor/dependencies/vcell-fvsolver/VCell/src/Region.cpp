/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/Region.h>

Region::Region(int rindex, string& rname, Mesh* rmesh)
{	
	index = rindex;
	name = rname;
	mesh = rmesh;
	size = 0;
}

Region::~Region()
{      
}

void Region::addElementIndex(long i)
{
	elementIndices.push_back(i);
	size = 0;
}

void Region::setSize(double newSize)
{
	size = newSize;
}
