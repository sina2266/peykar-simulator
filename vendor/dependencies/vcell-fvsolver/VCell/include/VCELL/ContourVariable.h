/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef CONTOURVARIABLE_H
#define CONTOURVARIABLE_H

#include <VCELL/Variable.h>

class ContourVariable : public Variable
{
public:
	ContourVariable(long size, string& nameStr, string& Aunits);

	virtual bool isContourVar() { return true; }
	virtual VariableType getVarType() {return VAR_CONTOUR;}
};

#endif
