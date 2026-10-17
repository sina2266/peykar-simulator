/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef STRUCTUREDPDESOLVER_H
#define STRUCTUREDPDESOLVER_H

#include <stdio.h>
#include <VCELL/PDESolver.h>

typedef struct {
	void zero() { 
		Ap=Ax_minus=Ax_plus=Ay_minus=Ay_plus=Az_minus=Az_plus=B=Ap0=0.0;
		valid = 0;
	}
	//void show(char *buffer) {
	//	sprintf(buffer,"Ap=%lf Axm=%lf Axp=%lf,Aym=%lf,Ayp=%lf,Azm=%lf,Azp=%lf,B=%lf,Ap0=%lf\n",
	//			Ap,Ax_minus,Ax_plus,Ay_minus,Ay_plus,Az_minus,Az_plus,B,Ap0);
	//}
	double Ap;
	double Ax_minus;
	double Ax_plus;
	double Ay_minus;
	double Ay_plus;
	double Az_minus;
	double Az_plus;
	double B;
	double Ap0;
	bool valid;
} DiscreteEqn;

class VolumeVariable;
class CartesianMesh;

class StructuredPDESolver : public PDESolver
{
public:
	StructuredPDESolver(VolumeVariable *var, 
		CartesianMesh *mesh, 
		bool bTimeDependent);
	virtual ~StructuredPDESolver();

	DiscreteEqn *getEqns() { return eqn; } 
	void showEqn(FILE *fp);
	long getSize() { return size; }
	bool setEqn(long index, DiscreteEqn eqn); 
	bool setB(long index, double B); 
	bool addB(long index, double B); 
	void    resize(int sizeX, int sizeY, int sizeZ);
protected:
	CartesianMesh *mesh;
	long         size;
	long         sizeX;
	long         sizeY;
	long         sizeZ;
	long         sizeXY;
	DiscreteEqn  *eqn;
};

typedef enum {
	AXIS_X, 
	AXIS_Y, 
	AXIS_Z
} Axis;

#endif
