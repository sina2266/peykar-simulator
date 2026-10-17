/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef CONTOURSUBDOMAIN_H
#define CONTOURSUBDOMAIN_H

#include <VCELL/SimTypes.h>

#include <vector>
using namespace std;

class Contour;
class Feature;

class ContourSubdomain
{
public:
    ContourSubdomain(ContourHandle AcontourHandle, 
                    FeatureHandle AfeatureHandle, 
		    double AcharacteristicLength);
    double getCharacteristicLength(){return characteristicLength;}
    ContourHandle getContourHandle(){return contourHandle;}
    FeatureHandle getFeatureHandle(){return featureHandle;}
    virtual WorldCoord getCoord(double u) = 0;
    virtual double getLength() = 0;

protected:
    FeatureHandle featureHandle;
    ContourHandle contourHandle; 
    double characteristicLength;
};

class ListContour : public ContourSubdomain
{
public:
    ListContour(ContourHandle AcontourHandle, 
                FeatureHandle AfeatureHandle, 
				double characteristicLength);
    void addPoint(WorldCoord wc);
    double getLength();
    long getNumPoints(){return (long)contourPoints.size();}
    virtual WorldCoord getCoord(double u);
protected:
	vector<WorldCoord> contourPoints; //assumed to be ordered
	vector<double> length;
};


#endif

