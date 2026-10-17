
#ifndef CHOMBOIF_H
#define CHOMBOIF_H

#include "BaseIF.H"
#include <assert.h>

#include <string>
#include <vector>
using std::string;
using std::vector;

#include <VCELL/Feature.h>

namespace VCell {
	class Expression;
}

///
/**
    This implicit function specifies a union of spheres.
 */
class ChomboIF: public BaseIF
{
public: 
	ChomboIF(Feature* f, int phaseIndex, string& file);
	ChomboIF(Feature* f, int phaseIndex, VCell::Expression* ifExp, VCell::Expression* userExp);

	// copy constructor used by Chombo
	ChomboIF(const ChomboIF*);
	virtual ~ChomboIF();
	
	/** 
	 * the level set value = 0 represnets the boundary and value < 0 is inside the fluid
	 * 
	 * */
	Real value(const RealVect& a_point) const;
	Real value(const RealVect& a_point, bool validate) const;
	BaseIF* newImplicitFunction() const;

	VCell::Expression* getIFExpression() {
		return ifExp;
	}
	Feature* getFeature() {
		return feature;
	}
	int getPhaseIndex() {
		return phaseIndex;
	}

private:
	Feature* feature;
	int phaseIndex;
	VCell::Expression* ifExp;
	VCell::Expression* userExp;

	int dimension;
	IntVect sampleN;
	RealVect sampleH;
	RealVect firstSamplePoint;
	double* distanceMap;
	int sampleOffsetXY;

	void readDistanceMap(string& distanceMapFile);
	double interpolateDistance(const RealVect& a_point) const;
};

#endif
