/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef FIELDDATA_H
#define FIELDDATA_H

#include <VCELL/DataSet.h>

class FieldData {

private:
	int fdIndex;
	VariableType varType;
	string fdID;
	string fdName;
	string fdVarName;
	double fdTime;
	string fdFile;
	double* data;
	
	FileHeader fileHeader;
	int dataLength;

public:
	FieldData(int arg_fdIndex, VariableType arg_varType, string arg_fdID, string arg_fdName, string arg_fdVarName, double arg_fdTime, string arg_fdFile);
	~FieldData();

	string getID() { return fdID; }	
	VariableType getVariableType() {
		return varType;
	}

	double* getData();
	int getSizeX();
	int getSizeY();
	int getSizeZ();
	int getDataLength();
};

#endif
