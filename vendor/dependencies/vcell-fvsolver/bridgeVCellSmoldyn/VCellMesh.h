#ifndef VCELL_MESH_H
#define VCELL_MESH_H

#include <smoldyn.h>
#include <VCELL/SimTool.h>

class VCellMesh : public AbstractMesh {
public:
	VCellMesh(SimTool* simTool);
	void getCenterCoordinates(int volIndex, double* coords);
	void getDeltaXYZ(double* delta);
	void getNumXYZ(int* num);
private:
	SimTool* simTool;
};

#endif