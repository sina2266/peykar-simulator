/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/VolumeVariable.h>
#include <VCELL/EqnBuilder.h>
#include <VCELL/Simulation.h>
#include <VCELL/StructuredPDESolver.h>
#include <VCELL/CartesianMesh.h>

StructuredPDESolver::StructuredPDESolver(VolumeVariable *Var, CartesianMesh *Amesh, bool AbTimeDependent) : PDESolver(Var, AbTimeDependent) {
	sizeX = Amesh->getNumVolumeX();
	sizeY = Amesh->getNumVolumeY();
	sizeZ = Amesh->getNumVolumeZ();
	size = sizeX * sizeY * sizeZ;
	sizeXY = sizeX*sizeY;
	ASSERTION(size>2);
	eqn = new DiscreteEqn[size];
	mesh = Amesh;
}

StructuredPDESolver::~StructuredPDESolver()
{
   if (eqn) delete[] eqn;
}

void StructuredPDESolver::resize(int x, int y, int z)
{
    ASSERTION(x>0 && y>0 && z>0);
    sizeX = x;
    sizeY = y;
    sizeZ = z;
    size = sizeX * sizeY * sizeZ;
    ASSERTION(size);
    if (eqn) delete[] eqn;
    eqn = new DiscreteEqn[size];
}

void StructuredPDESolver::showEqn(FILE *fp)
{
   for (long index=0;index<size;index++){
      fprintf(fp,"index=%ld, "
	       "Ap=%9lg, Ap0=%9lg, B=%9lg, "
	       "Axm=%9lg, Axp=%9lg, "
	       "Aym=%9lg, Ayp=%9lg, "
	       "Azm=%9lg, Azp=%9lg \n", 
	        index,eqn[index].Ap,eqn[index].Ap0,eqn[index].B,
                eqn[index].Ax_minus,eqn[index].Ax_plus,
                eqn[index].Ay_minus,eqn[index].Ay_plus,
                eqn[index].Az_minus,eqn[index].Az_plus);
   }
}

bool StructuredPDESolver::setEqn(long index, DiscreteEqn equation)
{
	bool valid = false;
	const double epsilon = 1.0E-8;

	ASSERTION(index >= 0 && index < size);
	    
	eqn[index] = equation; //????????
	valid = true;

	return valid;
}

bool StructuredPDESolver::setB(long index, double B)
{
    eqn[index].B = B;
    return true;
}

bool StructuredPDESolver::addB(long index, double B)
{
    eqn[index].B += B;
    return true;
}
