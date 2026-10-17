/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef REGION_H
#define REGION_H 

#include <VCELL/SimTypes.h>
#include <vector>
#include <string>
using std::string;
using std::vector;

class Mesh;

class Region
{
public:
	Region(int rindex, string& rname, Mesh* rmesh);
	~Region();

	string getName() {
		return name; 
	}
	int getIndex() { 
		return index; 
	}
	long getNumElements() { 
		return (int)elementIndices.size(); 
	} 
	long getElementIndex(long i) { 
		return elementIndices[i]; 
	} 
	void addElementIndex(long i);

	virtual double getSize()=0;
	void setSize(double newSize);

	virtual bool isAdjacentToBoundary()=0;

protected:
	vector<long> elementIndices;
	Mesh* mesh;
	string name;
	int index;
	double size;
};

#endif
