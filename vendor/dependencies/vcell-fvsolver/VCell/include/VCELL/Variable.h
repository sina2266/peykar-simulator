/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
//--------------------------------------------
// Variable.h
//--------------------------------------------
#ifndef VARIABLE_H
#define VARIABLE_H

#include <VCELL/SimTypes.h>
#include <string>
using std::string;
using std::ofstream;

class Structure;

class Variable 
{
protected:
	Variable(string& nameStr, Structure* structure, long Asize, bool diff=false);

public:
	virtual ~Variable();

	virtual void show(ofstream&);

	double *getOld()  { return old; }
	double *getCurr() { return curr; }
	long getSize() { return size; }

	double getOld(long index);           // if doesn't exist, return 0.0
	double getCurr(long index);          // if doesn't exist, return 0.0

	void setOld(long index, double value);
	void setCurr(long index, double value);
	
	const string& getName() { return name; }
	string getQualifiedName();

	virtual VariableType getVarType() {	return VAR_UNKNOWN; }

	void update();

	bool isDiffusing() { return bDiffusing; }
	Structure* getStructure() {
		return structure;
	}
   
protected:
	void clear();

	string name;
	Structure* structure;
	    
	long size;
	double *old;
	double *curr;

	bool bDiffusing;
};

#endif
