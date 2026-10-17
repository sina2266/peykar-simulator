/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef CONTOUR_H
#define CONTOUR_H

#include <VCELL/SimTypes.h>
#include <vector>
using namespace std;

typedef enum {
	CONTOUR_INTERIOR, 
	CONTOUR_BEGIN, 
	CONTOUR_END
}ContourBorder ;

class ContourVariable;
class ContourVarContext;
class ContourElement;
class FastSystem;
class Simulation;

class Contour
{
public:
	Contour(int Atype);
	~Contour();

	void resolveReferences(Simulation *sim);
	virtual void initElementValues(long elementIndex);

	string getName() { return names[type]; }
	int getType() { return type; }

	ContourVarContext* getContourVarContext(string& contourVarName);
	ContourVarContext* getContourVarContext(ContourVariable *var);
	ContourVarContext* getContourVarContext(int index);
	int getNumContourVarContext() {
		return (int)contourVarContextList.size();
	}

	void addContourVarContext(ContourVarContext *vc);

	FastSystem * getFastSystem(){ return fastSystem; }

	static void setContourTypes(string *nameArray);
	static inline string getNameFromState(int Astate) { return names[Astate]; }
     
protected:
	vector<ContourVarContext*> contourVarContextList;
	FastSystem               *fastSystem;
   
private:
	int                     type;
	static string          *names;
};

class ContourElement
{
public:
	ContourElement();
	WorldCoord    getCoord(double u);
	double        getDistancetoPoint(WorldCoord wc);
	Contour      *getContour(){return contour;}
	WorldCoord    getBegin(){return wc_begin;}
	WorldCoord    getEnd(){return wc_end;}
	ContourBorder getBorder(){return border;}
	long          getElementIndex(){return elementIndex;}
	long          getVolumeIndex(){return volumeIndex;}
	double        getElementLength(){return elementLength;}
private:
	friend class Mesh;
	   
	Contour      *contour;
	WorldCoord    wc_begin;
	WorldCoord    wc_end;
	ContourBorder border;
	long          elementIndex;
	long          volumeIndex;
	double        elementLength;
};

#endif

