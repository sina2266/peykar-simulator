#ifndef VOLUMENEIGHBOR_H
#define VOLUMENEIGHBOR_H

#include <VCELL/VolumeVarContextExpression.h>

struct CoupledNeighbors {
	int centerIndex;
	int neighborIndex;
	double coeff;
	CoupledNeighbors(int arg_centerIndex, int arg_neighborIndex, double arg_coeff) {
		centerIndex = arg_centerIndex;
		neighborIndex = arg_neighborIndex;
		coeff = arg_coeff;
	}
};

typedef enum {ZM=0, YM, XM, XP, YP, ZP} XYZNeighbor;

struct VolumeNeighbor {
	int index; // neighbor index, might be -1 if it is not valid, meaning there is no such neighbor
	bool bPeriodic;
	
	// for convection only
	int convectionDirection;
	double Vi, Vj;

	VolumeNeighbor(int idx) {
		index = idx;		
		bPeriodic = false;
		convectionDirection = 0;
		Vi = 0;
		Vj = 0;
	}

	void setConvectionCoefficients(int myindex, int mymask, XYZNeighbor whichNeighbor, VolumeVarContextExpression* varContext) {
		if (index < 0) {
			return;
		}
		switch (whichNeighbor) { 
			case XM:
			case XP:
				convectionDirection = (whichNeighbor == XM) ? 1 : -1;
				Vi = varContext->getConvectionVelocity_X(myindex);
				Vj = varContext->getConvectionVelocity_X(index);
				break;
			case YM:
			case YP:
				convectionDirection = (whichNeighbor == YM) ? 1 : -1;
				Vi = varContext->getConvectionVelocity_Y(myindex);
				Vj = varContext->getConvectionVelocity_Y(index);
				break;
			case ZM:
			case ZP:
				convectionDirection = (whichNeighbor == ZM) ? 1 : -1;
				Vi = varContext->getConvectionVelocity_Z(myindex);
				Vj = varContext->getConvectionVelocity_Z(index);
				break;
		}
	}
};
#endif
