#include "VCellValueProvider.h"
#include <stdlib.h>
#include <smoldynfuncs.h>
#include <VCELL/SimulationExpression.h>
#include <VCELL/CartesianMesh.h>
#include <VCELL/Element.h>
#include <VCELL/SimTypes.h>


VCellValueProvider::VCellValueProvider(simptr sim, SimTool* simTool,string& rateExpStr) {
	this->simTool = simTool;
	this->sim = sim;
	rateExp = new VCell::Expression(rateExpStr);
}

double VCellValueProvider::getConstantValue() {
	double d = rateExp->evaluateConstant();
	return d;
}

double VCellValueProvider::getValue(double t, double x, double y, double z, rxnptr rxn) {
	SimulationExpression* vcellSim = simTool->getSimulation();
	WorldCoord wc(x, y, z);
	vcellSim->setCurrentCoordinate(wc);

	int volIndex = ((CartesianMesh*)vcellSim->getMesh())->getVolumeIndex(wc);
	int* indices = vcellSim->getIndices();
	indices[VAR_MEMBRANE_INDEX] = -1;
	indices[VAR_MEMBRANE_REGION_INDEX] = -1;
	indices[VAR_VOLUME_INDEX] = volIndex;
	indices[VAR_VOLUME_REGION_INDEX] = vcellSim->getMesh()->getVolumeElements()[volIndex].getRegionIndex();

	return rateExp->evaluateProxy();	
}

double VCellValueProvider::getValue(double t, double x, double y, double z, rxnptr rxn, char* panelName) {
	
	SimulationExpression* vcellSim = simTool->getSimulation();
	int* indices = vcellSim->getIndices();
	WorldCoord wc(x, y, z);
	vcellSim->setCurrentCoordinate(wc);

	if(panelName == NULL)
	{
		simLog(sim, 10,"Unable to find membrane index(paneName == NULL). Cannot evaluate membrane reaction rate.");
	}
	//find membraneIndex
	string pName(panelName);
	size_t found = pName.find_last_of("_");
	int memIndex = atoi((pName.substr(found+1)).c_str());

	indices[VAR_MEMBRANE_INDEX] = memIndex;
	indices[VAR_MEMBRANE_REGION_INDEX] = vcellSim->getMesh()->getMembraneElements()[memIndex].getRegionIndex();
	indices[VAR_VOLUME_INDEX] = -1;
	indices[VAR_VOLUME_REGION_INDEX] = -1;


	return rateExp->evaluateProxy();	
}

double VCellValueProvider::getValue(double t, double x, double y, double z, surfactionptr actiondetails, char* panelName){
	
	SimulationExpression* vcellSim = simTool->getSimulation();
	int* indices = vcellSim->getIndices();
	WorldCoord wc(x, y, z);
	vcellSim->setCurrentCoordinate(wc);

	if(panelName == NULL)
	{
		simLog(sim, 10, "Unable to find membrane index(paneName == NULL). Cannot evaluate membrane reaction rate.");
	}
	//find membraneIndex
	string pName(panelName);
	size_t found = pName.find_last_of("_");
	int memIndex = atoi((pName.substr(found+1)).c_str());

	indices[VAR_MEMBRANE_INDEX] = memIndex;
	indices[VAR_MEMBRANE_REGION_INDEX] = vcellSim->getMesh()->getMembraneElements()[memIndex].getRegionIndex();
	indices[VAR_VOLUME_INDEX] = -1;
	indices[VAR_VOLUME_REGION_INDEX] = -1;


	return rateExp->evaluateProxy();
}

void VCellValueProvider::bindExpression(SymbolTable* symbolTable)
{
	rateExp->bindExpression(symbolTable);
}


VCellValueProviderFactory::VCellValueProviderFactory(SimTool* simTool){
	this->simTool = simTool;
}

ValueProvider* VCellValueProviderFactory::createValueProvider(string& rateExp){
	return new VCellValueProvider(getSimptr(), simTool, rateExp);
}
