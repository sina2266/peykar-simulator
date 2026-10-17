/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef CONTOURVARCONTEXT_H
#define CONTOURVARCONTEXT_H

#include <VCELL/VarContext.h>

class Contour;
class ContourElement;

class ContourVarContext : public VarContext
{
public:
    virtual double  getContourReactionRate(ContourElement *element)=0;
    virtual double  getContourDiffusionRate(ContourElement *element)=0;
    Contour* getContour() { return contour; }
    
protected:
    ContourVarContext(Contour *contour, Feature *feature, string& speciesName);

private:
    Contour *contour;
};

#endif
