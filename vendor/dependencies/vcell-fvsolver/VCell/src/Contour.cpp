/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/Contour.h>
#include <VCELL/ContourVarContext.h>
#include <VCELL/ParticleContext.h>
#include <VCELL/FastSystem.h>
#include <VCELL/ContourVariable.h>
#include <string>
using namespace std;

//-----------------------------------------------------------
//
//   class Contour
//
//------------------------------------------------------------
string *Contour::names = NULL;

Contour::Contour(int Atype)
{
	type = Atype;
	fastSystem = NULL;
}

Contour::~Contour()
{
}

void Contour::resolveReferences(Simulation *sim)
{
	//
	// initialize ContourVarContexts
	//
	for (int i = 0; i < (int)contourVarContextList.size(); i ++) {
		contourVarContextList[i]->resolveReferences(sim);
	}

	//
	// initialize FastSystem
	//
	if(fastSystem!=NULL){
		fastSystem->resolveReferences(sim);
	}
}

void Contour::initElementValues(long elementIndex)
{
	//
	// initialize ContourVariables
	//
	for (int i = 0; i < (int)contourVarContextList.size(); i ++) {
		ContourVarContext *contourVarContext=contourVarContextList[i];
		double value = contourVarContext->getInitialValue(elementIndex);
		ContourVariable* var = (ContourVariable *)contourVarContext->getVar();
		var->setOld(elementIndex, value);
		var->setCurr(elementIndex, value);
	}
}

ContourVarContext* Contour::getContourVarContext(string& contourVarName)
{
	for (int i = 0; i < (int)contourVarContextList.size(); i ++) {
		ContourVarContext *contourVarContext = contourVarContextList[i];
		if (contourVarContext->getVarName() == contourVarName) {
			return contourVarContext;
		}
	}

	return 0;
}

ContourVarContext* Contour::getContourVarContext(ContourVariable *var)
{

	for (int i = 0; i < (int)contourVarContextList.size(); i ++) {
		ContourVarContext *contourVarContext = contourVarContextList[i];
		if (contourVarContext->getVar() == var) {
			return contourVarContext;
		}		
	}
	return 0;
}

ContourVarContext* Contour::getContourVarContext(int index)
{
	if (index < 0 || index >= (int)contourVarContextList.size()) {
		throw "Contour::getContourVarContext(int index): index out of bounds";
	}

	return contourVarContextList.at(index);
}

void Contour::addContourVarContext(ContourVarContext *vc)
{
	contourVarContextList.push_back(vc);
}

void Contour::setContourTypes(string *nameArray)
{
	names = nameArray; 
}

//-----------------------------------------------------------
//
//   class ContourElement
//
//------------------------------------------------------------
ContourElement:: ContourElement()
{
	contour = NULL;

	wc_begin.x = 0;
	wc_begin.y = 0;
	wc_begin.z = 0;
	wc_end.x = 0;
	wc_end.y = 0;
	wc_end.z = 0;

	border = CONTOUR_BEGIN;

	elementIndex = 0;
	volumeIndex = 0;
	elementLength = 0;
}

WorldCoord ContourElement::getCoord(double u)
{
	ASSERTION((u>=0)&&(u<=elementLength));
	WorldCoord wc;

	wc.x = wc_begin.x + (wc_end.x - wc_begin.x)*u/elementLength;
	wc.y = wc_begin.y + (wc_end.y - wc_begin.y)*u/elementLength;
	wc.z = wc_begin.z + (wc_end.z - wc_begin.z)*u/elementLength;

	return wc;  
}

double ContourElement::getDistancetoPoint(WorldCoord wc)
{
	double u;

	ASSERTION(!((wc_begin.x==wc_end.x)
			&&(wc_begin.y==wc_end.y)
			&&(wc_begin.z==wc_end.z)));

	u = ((wc_end.x - wc_begin.x)*(wc.x - wc_begin.x)
			+ (wc_end.y - wc_begin.y)*(wc.y - wc_begin.y)
			+ (wc_end.z - wc_begin.z)*(wc.z - wc_begin.z))
			/((wc_end.x - wc_begin.x)*(wc_end.x - wc_begin.x)
			+(wc_end.y - wc_begin.y)*(wc_end.y - wc_begin.y) 
			+(wc_end.z - wc_begin.z)*(wc_end.z - wc_begin.z));
	if(u<0){
		return sqrt((wc_begin.x - wc.x)*(wc_begin.x - wc.x)
				+(wc_begin.y - wc.y)*(wc_begin.y - wc.y)
				+(wc_begin.z - wc.z)*(wc_begin.z - wc.z));
	}else if(u>1){
		return sqrt((wc_end.x - wc.x)*(wc_end.x - wc.x)
				+(wc_end.y - wc.y)*(wc_end.y - wc.y)
				+(wc_end.z - wc.z)*(wc_end.z - wc.z));
	}else{
		u*=getElementLength();
		return  sqrt((getCoord(u).x - wc.x)*(getCoord(u).x - wc.x)
				+(getCoord(u).y - wc.y)*(getCoord(u).y - wc.y)
				+(getCoord(u).z - wc.z)*(getCoord(u).z - wc.z));
	} 
}
