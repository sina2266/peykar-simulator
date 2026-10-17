/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/VolumeParticleVariable.h>
#include <VCELL/Feature.h>
#include <fstream>
using std::ofstream;
using std::endl;

VolumeParticleVariable::VolumeParticleVariable(string& nameStr, Feature* feature, long numX, long numY, long numZ)
: ParticleVariable(nameStr, feature, numX*numY*numZ)
{
	sizeX = numX;
	sizeY = numY;
	sizeZ = numZ;
}

void VolumeParticleVariable::show(ofstream& fp)
{
	double *pCurr=curr;
	fp << getName() << endl;
	int i, j, k;
	for (k=0;k<sizeZ;k++){
		if (sizeZ > 1) 
			fp << "z=" << k << ",";
		for (j = 0; j < sizeY; j ++){
			if (sizeY > 1) 
				fp << "y=" << j << ",";
			for (i = 0; i < sizeX; i ++){
				fp << *pCurr << " ";
				pCurr++;
				if (i % 10 == 9) 
					fp << endl;
			}
			if (i % 10 != 0) 
				fp << endl;
			fp << endl;		
		}
	}
}
