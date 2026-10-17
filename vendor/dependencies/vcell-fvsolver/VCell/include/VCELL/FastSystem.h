/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef FASTSYSTEM_H
#define FASTSYSTEM_H

#include <VCELL/SimTypes.h>
#include <VCELL/AlgebraicSystem.h>

class SimulationExpression;
class Variable;

class FastSystem : public AlgebraicSystem 
{
public:
    FastSystem(int dimension, int numDependents);
    void setCurrIndex(long index){currIndex = index;}
    void updateVars();
    int getNumDependents() const {return numDependents;}
    void setDependencyMatrix(int i, int j, double value); 
    virtual void updateDependentVars();
    void showVars();
    virtual void resolveReferences(SimulationExpression *sim)=0;
	virtual void setCoordinates(double time_sec, WorldCoord& wc){};

protected:
    long currIndex;
    Variable **pVars;
    int numDependents;
    Variable **pDependentVars;
    double **dependencyMatrix;
};
#endif
