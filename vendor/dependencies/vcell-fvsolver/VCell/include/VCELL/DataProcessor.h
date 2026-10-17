/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef DATAPROCESSOR_H
#define DATAPROCESSOR_H

#include <string>
using std::string;

#define DATAPROCOUTPUT_EXT ".dataProcOutput"

class SimTool;

class DataProcessor
{
public:	    
	DataProcessor(string& n, string& i) {
		name = n;
		input = i;
	}

	virtual void onStart(SimTool* simTool)=0;
	virtual void onWrite(SimTool* simTool)=0;
	virtual void onComplete(SimTool* simTool)=0;
	virtual bool checkComplete(SimTool* simTool)=0;

protected:
	string name;
	string input;
};

#endif
