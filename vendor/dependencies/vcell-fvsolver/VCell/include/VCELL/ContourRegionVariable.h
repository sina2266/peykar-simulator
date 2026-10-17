/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef CONTOURREGIONVARIABLE_H
#define CONTOURREGIONVARIABLE_H

#include <VCELL/Variable.h>

class ContourRegionVariable : public Variable
{
public:
	ContourRegionVariable(int size, string& nameStr, string& Aunits);

	virtual bool isContourRegionVar() { return true; }
	virtual VariableType getVarType() {return VAR_CONTOUR_REGION;}
};

#endif
