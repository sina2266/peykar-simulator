/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef VOLUMEVARIABLE_H
#define VOLUMEVARIABLE_H

#include <VCELL/Variable.h>

class VolumeRegion;
class Feature;

class VolumeVariable : public Variable
{
public:
	VolumeVariable(string& nameStr, Feature* feature, long size, long extrapSize);
	~VolumeVariable();
	virtual VariableType	getVarType() {return VAR_VOLUME;}

	bool isAdvecting()
	{
		return bAdvecting;
	}
	double *getExtrapolated() 
	{
		return extrapolated;
	}
	int getExtrapolatedSize()
	{
		return extrapolatedSize;
	}

#ifndef CH_MPI
	void createErrorVariables();
#endif
	
private:
	bool bAdvecting;
	long extrapolatedSize;
	double* extrapolated;

};

#endif
