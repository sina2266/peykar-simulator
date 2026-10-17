/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef ALGEBRAICSYSTEM_H
#define ALGEBRAICSYSTEM_H

class SimTool;

class AlgebraicSystem
{
public:
	virtual ~AlgebraicSystem() = default;
	void solveSystem();
	virtual void initVars(SimTool* sim_tool)=0;
	double getX(int index) const {return x[index];}
	int getDimension() const {return dimension;}
	void setTolerance(double tol) {tolerance = tol;}

protected:
    AlgebraicSystem(int dimension);
    virtual void updateMatrix()=0;
    void setMatrix(int i, int j, double value); 
    void setX(int i, double value); 
	int dimension;

private:    
    double *varIncrements;
    double tolerance;
    double   *x;
    double **matrix;
    void solveGauss();  
};

#endif
