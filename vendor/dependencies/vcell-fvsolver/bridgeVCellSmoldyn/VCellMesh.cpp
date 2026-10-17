#include "VCellMesh.h"
#include <VCELL/CartesianMesh.h>
#include <VCELL/DoubleVector3.h>
#include <VCELL/SimulationExpression.h>

using namespace std;


VCellMesh::VCellMesh(SimTool* simTool)
{
	this->simTool = simTool;
}

void VCellMesh::getCenterCoordinates(int volIndex, double* coords)
{
	CartesianMesh* mesh = ((CartesianMesh*)simTool->getSimulation()->getMesh());
	WorldCoord wc = mesh->getVolumeWorldCoord(volIndex);
	coords[0] = wc.x;
	coords[1] = wc.y;
	coords[2] = wc.z;
}

void VCellMesh::getDeltaXYZ(double* delta)
{
	CartesianMesh* mesh = ((CartesianMesh*)simTool->getSimulation()->getMesh());
	delta[0] = mesh->getXScale_um();
	delta[1] = mesh->getYScale_um();
	delta[2] = mesh->getZScale_um();
}

void VCellMesh::getNumXYZ(int* num)
{
	CartesianMesh* mesh = ((CartesianMesh*)simTool->getSimulation()->getMesh());
	num[0] = mesh->getNumVolumeX();
	num[1] = mesh->getNumVolumeY();
	num[2] = mesh->getNumVolumeZ();
}