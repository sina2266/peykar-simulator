/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/SimulationExpression.h>
#include <VCELL/SimTool.h>
#include <VCELL/FieldData.h>
#include <VCELL/VolumeVariable.h>
#include <VCELL/VolumeParticleVariable.h>
#include <VCELL/MembraneVariable.h>
#include <VCELL/MembraneParticleVariable.h>
#include <VCELL/VolumeRegionVariable.h>
#include <VCELL/MembraneRegionVariable.h>
#include <VCELL/Element.h>
#include <VCELL/CartesianMesh.h>
#include <ValueProxy.h>
#include <VCELL/RandomVariable.h>
#include <VCELL/RegionSizeVariable.h>
#include <VCELL/Feature.h>
#include <VCELL/Membrane.h>
#include <VCELL/VCellModel.h>
#include <SimpleSymbolTable.h>
#include <ScalarValueProxy.h>
#include <VCELL/FVDataSet.h>
#include <VCELL/PostProcessingBlock.h>

#include <iostream>
#include <sstream>

#include "VCELL/SerialScheduler.h"
#include "VCELL/SundialsPdeScheduler.h"
using std::endl;
using std::stringstream;

#define RANDOM_VARIABLE_FILE_EXTENSION ".rv"

class ValueProxyVolumeExtrapolate : public ValueProxy
{
public:
	ValueProxyVolumeExtrapolate(double* arg_values, int* arg_indices, Mesh* arg_mesh, Feature* f) 
		: ValueProxy(arg_values,  -1, arg_indices) {
			mesh = arg_mesh;
			feature = f;
	}
	
	double evaluate() {
		MembraneElement* element = mesh->getMembraneElements() + indices[VAR_MEMBRANE_INDEX];
		int nearIndex, farIndex;
		if (mesh->getVolumeElements()[element->vindexFeatureLo].getFeature() == feature) {
			nearIndex = element->vindexFeatureLo;
			farIndex = element->vindexFeatureLoFar;
		} else if (mesh->getVolumeElements()[element->vindexFeatureHi].getFeature() == feature) {
			nearIndex = element->vindexFeatureHi;
			farIndex = element->vindexFeatureHiFar;
		} else {
			throw "ValueProxyVolumeExtrapolate"; 
		}

		if (farIndex<0){
			return values[nearIndex];
		}else{
			return 1.5 * values[nearIndex] - 0.5 * values[farIndex];
		}	
	}

private:
	Mesh* mesh;
	Feature* feature;
};

class VolumeRegionMembraneValueProxy : public ValueProxy
{
public:
	VolumeRegionMembraneValueProxy(double* arg_values, int* arg_indices, CartesianMesh* arg_mesh, Feature* f) 
		: ValueProxy(arg_values,  -1, arg_indices) {
			mesh = arg_mesh;
			feature = f;
	}
	
	double evaluate() {
		MembraneRegion* memRegion = mesh->getMembraneRegion(indices[VAR_MEMBRANE_REGION_INDEX]);
		VolumeRegion* vr = memRegion->getVolumeRegion1();
		if (memRegion->getVolumeRegion2()->getFeature() == feature) {
			vr = memRegion->getVolumeRegion2();
		}
		return values[vr->getIndex()];
	}

private:
	CartesianMesh* mesh;
	Feature* feature;
};

SimulationExpression::SimulationExpression(CartesianMesh *mesh) : Simulation(mesh) {
	symbolTable = NULL;

	indices = new int[NUM_VAR_INDEX];
	for (int i = 0; i < NUM_VAR_INDEX; i ++) {
		indices[i] = -1;
	}

	valueProxyTime = 0;
	valueProxyX = 0;
	valueProxyY = 0;
	valueProxyZ = 0;

	numMemPde = 0;
	numVolPde = 0;
	bHasTimeDependentDiffusionAdvection = false;

	psfFieldDataIndex = -1;

	volVarSize = 0;
	memVarSize = 0;
	volRegionVarSize = 0;
	memRegionVarSize = 0;
	volParticleVarSize = 0;
	memParticleVarSize = 0;

	volVarList = 0;
	memVarList = 0;
	volRegionVarList = 0;
	memRegionVarList = 0;
	volParticleVarList = 0;
	memParticleVarList = 0;

	postProcessingBlock = NULL;

	//(if not set to 0) causes segmentation violation in destructor depending on where solver code exits
	numRegionSizeVars = 0;
	regionSizeVarList = 0;
}

SimulationExpression::~SimulationExpression() 
{
	delete symbolTable;
	delete valueProxyTime;
	delete valueProxyX;
	delete valueProxyY;
	delete valueProxyZ;
	for (int i = 0; i < (int)paramValueProxies.size(); i ++) {
		delete paramValueProxies[i];
	}
	for (int i = 0; i < (int)volVariableRegionMap.size(); i ++) {
		delete[] volVariableRegionMap[i];
	}
	volVariableRegionMap.clear();

	delete[] volVarList;
	delete[] memVarList;
	delete[] volRegionVarList;
	delete[] memRegionVarList;
	delete[] volParticleVarList;
	delete[] memParticleVarList;
	for (int i = 0; i < numRegionSizeVars; i ++) {
		delete regionSizeVarList[i];
	}
	delete[] regionSizeVarList;

	delete postProcessingBlock;
}

void SimulationExpression::addVariable(Variable *var, bool* bSolveRegions)
{
	switch (var->getVarType()) {
	case VAR_VOLUME:
		addVolumeVariable((VolumeVariable*)var, bSolveRegions);
		break;
	case VAR_MEMBRANE:
		addMembraneVariable((MembraneVariable*)var);
		break;
	case VAR_VOLUME_REGION:
		addVolumeRegionVariable((VolumeRegionVariable *)var);
		break;
	case VAR_MEMBRANE_REGION:
		addMembraneRegionVariable((MembraneRegionVariable *)var);
		break;
	default:
		stringstream ss;
		ss << "Variable type " << var->getVarType() << " is not supported yet";
		throw ss.str();
	}	
}

void SimulationExpression::addVolumeVariable(VolumeVariable *var, bool* bSolveRegions)
{
	Simulation::addVariable(var);
	volVariableRegionMap.push_back(bSolveRegions);
	volVarSize ++;
	if (var->isDiffusing()) {
		numVolPde ++;
	}
}

void SimulationExpression::addVolumeParticleVariable(VolumeParticleVariable *var)
{
	Simulation::addVariable(var);
	++ volParticleVarSize;
}

void SimulationExpression::addMembraneParticleVariable(MembraneParticleVariable *var)
{
	Simulation::addVariable(var);
	++ memParticleVarSize;
}

void SimulationExpression::addMembraneVariable(MembraneVariable *var)
{
	Simulation::addVariable(var);
	memVarSize ++;
	if (var->isDiffusing()) {
		numMemPde ++;
	}
}

void SimulationExpression::addVolumeRegionVariable(VolumeRegionVariable *var)
{
	Simulation::addVariable(var);
	volRegionVarSize ++;
}

void SimulationExpression::addMembraneRegionVariable(MembraneRegionVariable *var)
{
	Simulation::addVariable(var);
	memRegionVarSize ++;
}

void SimulationExpression::advanceTimeOn(SimTool* sim_tool) {
	Simulation::advanceTimeOn();
	valueProxyTime->setValue(getTime_sec(sim_tool));
}

void SimulationExpression::advanceTimeOff(SimTool* sim_tool) {
	Simulation::advanceTimeOff();
	valueProxyTime->setValue(getTime_sec(sim_tool));
}

void SimulationExpression::update(SimTool* sim_tool) {
	Simulation::update();
	valueProxyTime->setValue(getTime_sec(sim_tool));
}

void SimulationExpression::addParameter(string& param) {
	paramList.push_back(param);
	paramValueProxies.push_back(new ScalarValueProxy());
}

void SimulationExpression::createSymbolTable(SimTool* sim_tool) {
	if (symbolTable != NULL) {
		return;
	}

	if (sim_tool->isSundialsPdeSolver()) {
		if (volParticleVarSize > 0 || memParticleVarSize > 0) {
			throw "Fully implicit solver does not support hybrid simulations";
		}
	}
	CartesianMesh *mesh = (CartesianMesh*)_mesh;
	VCellModel* model = sim_tool->getModel();

	// volume size, membrane size, for each feature
	numRegionSizeVars = 1 + 1 + model->getNumFeatures();
	regionSizeVarList = new RegionSizeVariable*[numRegionSizeVars];

	int regionSizeVarCount = 0;

	// add region size variables for volume region size in each volume region
	{
		int numVolRegions = mesh->getNumVolumeRegions();
		string name = "vcRegionVolume";
		RegionSizeVariable* rsv = new RegionSizeVariable(name, 0, numVolRegions, true);
		regionSizeVarList[regionSizeVarCount ++] = rsv;

		for (int i = 0; i < numVolRegions; i ++) {
			VolumeRegion* vr = mesh->getVolumeRegion(i);
			rsv->getCurr()[i] = vr->getSize();
		}
		rsv->update();
	}
	// add region size variables for membrane region size in each membrane region
	{
		int numMemRegions = mesh->getNumMembraneRegions();
		string name = "vcRegionArea";
		RegionSizeVariable* rsv = new RegionSizeVariable(name, 0, numMemRegions, false);
		regionSizeVarList[regionSizeVarCount ++] = rsv;

		for (int i = 0; i < numMemRegions; i ++) {
			MembraneRegion* mr = mesh->getMembraneRegion(i);
			rsv->getCurr()[i] = mr->getSize();
		}
		rsv->update();
	}
	// for each feature, add a region size variables for size of adjecent volume region of that feature
	{
		int numMemRegions = mesh->getNumMembraneRegions();
		int numFeatures = model->getNumFeatures();
		for (int i = 0; i < numFeatures; i ++) {
			Feature* feature = model->getFeatureFromIndex(i);
			string name = "vcRegionVolume_" + feature->getName();
			RegionSizeVariable* rsv = new RegionSizeVariable(name, 0, numMemRegions, false);
			regionSizeVarList[regionSizeVarCount ++] = rsv;

			for (int i = 0; i < numMemRegions; i ++) {
				MembraneRegion* mr = mesh->getMembraneRegion(i);
				if (mr->getVolumeRegion1()->getFeature() == feature) {
					rsv->getCurr()[i] = mr->getVolumeRegion1()->getSize();
				} else if (mr->getVolumeRegion2()->getFeature() == feature) {
					rsv->getCurr()[i] = mr->getVolumeRegion2()->getSize();
				} else {
					rsv->getCurr()[i] = 0;
				}
			}
			rsv->update();
		}
	}

	int numVariables = (int)varList.size();

	if (volVarSize > 0) {
		volVarList = new VolumeVariable*[volVarSize];
	}
	if (memVarSize > 0) {
		memVarList = new MembraneVariable*[memVarSize];
	}
	if (volRegionVarSize > 0) {
		volRegionVarList = new VolumeRegionVariable*[volRegionVarSize];
	}
	if (memRegionVarSize > 0) {
		memRegionVarList = new MembraneRegionVariable*[memRegionVarSize];
	}
	if (volParticleVarSize > 0) {
		volParticleVarList = new VolumeParticleVariable*[volParticleVarSize];
	}
	if (memParticleVarSize > 0) {
		memParticleVarList = new MembraneParticleVariable*[memParticleVarSize];
	}
	int volVarCount = 0, memVarCount = 0, volRegionVarCount = 0, memRegionVarCount = 0;
	int volParticleVarCount = 0, memParticleVarCount = 0;
	for (int i = 0; i < numVariables; i ++) {
		Variable* var = varList.at(i);
		switch (var->getVarType()) {
		case VAR_VOLUME:
			volVarList[volVarCount ++] = ((VolumeVariable*)var);
			break;
		case VAR_MEMBRANE:
			memVarList[memVarCount ++] = ((MembraneVariable*)var);
			break;
		case VAR_VOLUME_REGION:
			volRegionVarList[volRegionVarCount ++] = (VolumeRegionVariable*)var;
			break;
		case VAR_MEMBRANE_REGION:
			memRegionVarList[memRegionVarCount ++] = (MembraneRegionVariable*)var;
			break;
		case VAR_VOLUME_PARTICLE:
			volParticleVarList[volParticleVarCount ++] = (VolumeParticleVariable*)var;
			break;
		case VAR_MEMBRANE_PARTICLE:
			memParticleVarList[memParticleVarCount ++] = (MembraneParticleVariable*)var;
			break;
		}
	}
	
	bool bSundialsPdeSolver = sim_tool->isSundialsPdeSolver();

	// t, x, y, z, VOL, VOL_Feature1_membrane, VOL_Feature2_membrane, ... (for computing membrane flux for volume variables), 
	// MEM, VOLREG, VOLREG_Feature1_membrane, VOLREG_Feature2_membrane, ... (for computing membrane flux for volume region variables), 
	// MEMREG, REGIONSIZE, field data, random variables, parameters
	numSymbols = 4 + volVarSize * (model->getNumFeatures() + 1) + memVarSize
		+ volRegionVarSize * (model->getNumFeatures() + 1) + memRegionVarSize + volParticleVarSize
		+ memParticleVarSize + numRegionSizeVars
		+ (int)fieldDataList.size() + (int)randomVarList.size() + (int)paramList.size();
	string* variableNames = new string[numSymbols];	

	// value proxy must be preserved in all solver cases.
	ValueProxy** oldValueProxies = new ValueProxy*[numSymbols];

	valueProxyTime = new ScalarValueProxy();
	valueProxyX = new ScalarValueProxy();
	valueProxyY = new ScalarValueProxy();
	valueProxyZ = new ScalarValueProxy();

	symbolIndexOffset_T = 0;
	variableNames[0] = "t";
	oldValueProxies[0] = valueProxyTime;

	symbolIndexOffset_Xyz = 1;
	variableNames[1] = "x";	
	oldValueProxies[1] = valueProxyX;

	variableNames[2] = "y";	
	oldValueProxies[2] = valueProxyY;

	variableNames[3] = "z";	
	oldValueProxies[3] = valueProxyZ;

	int variableIndex = 4;
	symbolIndexOffset_VolVar = variableIndex;
	// Volume PDE/ODE
	for (int i = 0; i < volVarSize; i ++) {
		Variable* var = volVarList[i];
		oldValueProxies[variableIndex] = new ValueProxy(var->getOld(), VAR_VOLUME_INDEX, indices);
		variableNames[variableIndex] = string(var->getName());
		variableIndex ++;

		for (int f = 0; f < model->getNumFeatures(); f ++) {
			Feature* feature = model->getFeatureFromIndex(f);
			oldValueProxies[variableIndex] = new ValueProxyVolumeExtrapolate(var->getOld(), indices, _mesh, feature);
			variableNames[variableIndex] = var->getName() + "_" + feature->getName() + "_membrane";
			variableIndex ++;
		}
	}

	symbolIndexOffset_MemVar = variableIndex;
	// Membrane ODE/PDE
	for (int i = 0; i < memVarSize; i ++) {
		Variable* var = memVarList[i];
		oldValueProxies[variableIndex] = new ValueProxy(var->getOld(), VAR_MEMBRANE_INDEX, indices);
		variableNames[variableIndex] = string(var->getName());
		variableIndex ++;
	}

	symbolIndexOffset_VolRegionVar = variableIndex;
	// Volume Region
	for (int i = 0; i < volRegionVarSize; i ++) {
		Variable* var = volRegionVarList[i];
		oldValueProxies[variableIndex] = new ValueProxy(var->getOld(), VAR_VOLUME_REGION_INDEX, indices);
		variableNames[variableIndex] = string(var->getName());
		variableIndex ++;

		for (int f = 0; f < model->getNumFeatures(); f ++) {
			Feature* feature = model->getFeatureFromIndex(f);
			oldValueProxies[variableIndex] = new VolumeRegionMembraneValueProxy(var->getOld(), indices, mesh, feature);
			variableNames[variableIndex] = var->getName() + "_" + feature->getName() + "_membrane";
			variableIndex ++;
		}
	} 		
		
	symbolIndexOffset_MemRegionVar = variableIndex;
	// Membrane Region
	for (int i = 0; i < memRegionVarSize; i ++) {
		Variable* var = memRegionVarList[i];
		oldValueProxies[variableIndex] = new ValueProxy(var->getOld(), VAR_MEMBRANE_REGION_INDEX, indices);
		variableNames[variableIndex] = string(var->getName());
		variableIndex ++;
	} 
		
		//} else if (var->getVarType() == VAR_CONTOUR) {
		//	oldValueProxies[variableIndex] = new ValueProxy(var->getOld(), VAR_CONTOUR_INDEX, indices);
		//	variableNames[variableIndex ++] = string(var->getName());

		//} else if (var->getVarType() == VAR_CONTOUR_REGION) {
		//	oldValueProxies[variableIndex] = new ValueProxy(var->getOld(), VAR_CONTOUR_REGION_INDEX, indices);
		//	variableNames[variableIndex ++] = string(var->getName());
		//}		

	symbolIndexOffset_VolParticleVar = variableIndex;
	// Volume Particle
	for (int i = 0; i < volParticleVarSize; i ++) {
		Variable* var = volParticleVarList[i];
		oldValueProxies[variableIndex] = new ValueProxy(var->getOld(), VAR_VOLUME_INDEX, indices);
		variableNames[variableIndex] = string(var->getName());
		variableIndex ++;
	}

	symbolIndexOffset_MemParticleVar = variableIndex;
	// Membrane Particle
	for (int i = 0; i < memParticleVarSize; i ++) {
		Variable* var = memParticleVarList[i];
		oldValueProxies[variableIndex] = new ValueProxy(var->getOld(), VAR_MEMBRANE_INDEX, indices);
		variableNames[variableIndex] = string(var->getName());
		variableIndex ++;
	}

	symbolIndexOffset_RegionSizeVariable = variableIndex;
	// add region size variables
	for (int i = 0; i < numRegionSizeVars; i ++) {
		RegionSizeVariable* var = regionSizeVarList[i];
		oldValueProxies[variableIndex] = new ValueProxy(var->getOld(), var->getVarType() == VAR_VOLUME_REGION ? VAR_VOLUME_REGION_INDEX : VAR_MEMBRANE_REGION_INDEX, indices);
		variableNames[variableIndex] = string(var->getName());
		variableIndex ++;
	}

	symbolIndexOffset_FieldData = variableIndex;
	// add field data
	for (int i = 0; i < (int)fieldDataList.size(); i ++) {
		if (fieldDataList[i]->getVariableType() == VAR_VOLUME) {
			oldValueProxies[variableIndex] = new ValueProxy(fieldDataList[i]->getData(), VAR_VOLUME_INDEX, indices);
		} else if (fieldDataList[i]->getVariableType() == VAR_MEMBRANE) {
			oldValueProxies[variableIndex] = new ValueProxy(fieldDataList[i]->getData(), VAR_MEMBRANE_INDEX, indices);
		} else {
			throw "field data is only supported for volume and membrane variables";
		}
		variableNames[variableIndex] = fieldDataList[i]->getID();
		variableIndex ++;
	}

	symbolIndexOffset_RandomVar = variableIndex;
	// add random variable
	if (randomVarList.size() > 0) {
		char rvfile[512];
		sprintf(rvfile, "%s%s", sim_tool->getBaseFileName().c_str(), RANDOM_VARIABLE_FILE_EXTENSION);
		FVDataSet::readRandomVariables(rvfile, this);
		for (int i = 0; i < (int)randomVarList.size(); i ++) {
			RandomVariable* rv = randomVarList[i];

			int indexindex = VAR_VOLUME_INDEX;
			if (rv->getVariableType() == VAR_MEMBRANE) {
				indexindex = VAR_MEMBRANE_INDEX;
			}
			if (rv->getRandomNumbers() == 0) {
				stringstream ss;
				ss << "RandomVariable " << rv->getName() << " doesn't have any random numbers" << endl;
				throw ss.str();
			}
			oldValueProxies[variableIndex] = new ValueProxy(rv->getRandomNumbers(), indexindex, indices);
			variableNames[variableIndex] = rv->getName();
			variableIndex ++;
		}
	}

	symbolIndexOffset_Parameters = variableIndex;
	// add parameters
	for (int i = 0; i < (int)paramList.size(); i ++) {
		oldValueProxies[variableIndex] = paramValueProxies[i];
		variableNames[variableIndex] = paramList[i];
		variableIndex ++;
	}

	assert(numSymbols == variableIndex);
	numSymbols = variableIndex;
	symbolTable = new SimpleSymbolTable(variableNames, variableIndex, oldValueProxies);
	delete[] variableNames;	
}

void SimulationExpression::initSimulation(SimTool* sim_tool)
{
	if (_scheduler == 0) {
		int odeCount = 0, pdeCount = 0;
		for (int i = 0; i < (int)varList.size(); i ++) {
			Variable* var = varList[i];
			if (var->isDiffusing()){
				pdeCount ++;
			} else {
				odeCount ++;
			}
		}

		printf("pdeCount=%d, odeCount=%d\n", pdeCount, odeCount);

		if (sim_tool->isSundialsPdeSolver()) {
			_scheduler = new SundialsPdeScheduler(this, sim_tool->getSundialsSolverOptions(),
				sim_tool->getNumDiscontinuityTimes(), sim_tool->getDiscontinuityTimes(), sim_tool->isSundialsOneStepOutput());
		} else {
			_scheduler = new SerialScheduler(this);
		}
	}

	_scheduler->initValues(sim_tool->getModel());
	currIteration = 0;
}



string* SimulationExpression::getFieldSymbols() {
	string* symbols = new string[fieldDataList.size()];
	for (int i = 0; i < (int)fieldDataList.size(); i ++) {
		symbols[i] = fieldDataList[i]->getID();
	}
	return symbols;
}

void SimulationExpression::populateFieldValues(double* darray, int index) {
	// here we don't know the index is for Volume or Membrane,
	// so we check array index is in bounds.
	for (int i = 0; i < (int)fieldDataList.size(); i ++) {
		int dataLength = fieldDataList[i]->getDataLength();
		if (index >= 0 && index < dataLength) {
			darray[i] = fieldDataList[i]->getData()[index];
		} else {
			darray[i] = 0;
		}
	}
}

void SimulationExpression::populateRandomValues(double* darray, int index) {
	// here we don't know the index is for Volume or Membrane,
	// so we check array index is in bounds.
	for (int i = 0; i < (int)randomVarList.size(); i ++) {
		RandomVariable* rv = randomVarList[i];
		
		if (index >= 0 && index < rv->getSize()) {
			darray[i] = rv->getRandomNumbers()[index];
		} else {
			darray[i] = 0;
		}
	}
}

void SimulationExpression::populateParameterValues(double* darray) {
	if (paramList.size() == 0) {
		return;
	}

	for (int i = 0; i < (int)paramList.size(); i ++) {
		darray[i] = paramValueProxies[i]->evaluate();
	}
}

void SimulationExpression::resolveReferences(SimTool* sim_tool) {
	if (symbolTable == 0) {
		createSymbolTable(sim_tool);
	}
	Simulation::resolveReferences(sim_tool);
}

void SimulationExpression::setParameterValues(SimTool* sim_tool, double* paramValues) {
	if (paramValues == 0) {
		if (paramList.size() != 0) {
			throw "SimulationExpression::setParameterValues(), empty values for parameters";
		}
		return;
	}
	for (int i = 0; i < (int)paramList.size(); i ++) {
		paramValueProxies[i]->setValue(paramValues[i]);
	}
	reinitConstantValues(sim_tool);
}

RandomVariable* SimulationExpression::getRandomVariableFromName(char* varName)
{
	for (int i = 0; i < (int)randomVarList.size(); i ++) {
		RandomVariable* var = randomVarList[i];
		if (string(varName) == var->getName()){
			return var;
		}
	}
	return NULL;
}

void SimulationExpression::setCurrentCoordinate(WorldCoord& wc) {
	valueProxyX->setValue(wc.x);
	valueProxyY->setValue(wc.y);
	valueProxyZ->setValue(wc.z);
}

bool SimulationExpression::isParameter(string& symbol) {

	for (int i = 0; i < (int)paramList.size(); i ++) {
		if (symbol == paramList[i]) {
			return true;
		}
	}
	return false;
}

void SimulationExpression::reinitConstantValues(SimTool* sim_tool) {
	for (int i = 0; i < sim_tool->getModel()->getNumFeatures(); i ++) {
		Feature* feature = sim_tool->getModel()->getFeatureFromIndex(i);
		feature->reinitConstantValues(sim_tool->getSimulation());
	}
	for (int i = 0; i < sim_tool->getModel()->getNumMembranes(); i ++) {
		Membrane* membrane = sim_tool->getModel()->getMembraneFromIndex(i);
		membrane->reinitConstantValues(sim_tool->getSimulation());
	}

}

void SimulationExpression::populateRegionSizeVariableValues(double *darray, bool bVolumeRegion, int regionIndex) {
	for (int i = 0; i < numRegionSizeVars; i ++) {
		RegionSizeVariable* rsv = regionSizeVarList[i];
		if (rsv->getVarType() == VAR_VOLUME_REGION) {
			darray[i] = bVolumeRegion ? rsv->getCurr()[regionIndex] : 0;
		} else {
			darray[i] = bVolumeRegion ? 0 : rsv->getCurr()[regionIndex];
		}
	}
}

void SimulationExpression::writeData(const char *filename, bool bCompress)
{
	//bool hasParticles = false;
	//VCellModel *model = SimTool::getInstance()->getModel();
	//for (int i = 0; i < model->getNumFeatures(); i ++) {
	//	Feature* feature = model->getFeatureFromIndex(i);
	//	if (feature->getVolumeParticleContext()!=NULL){
	//		hasParticles = true;
	//	}
	//	if (feature->getMembraneParticleContext()!=NULL){
	//		hasParticles = true;
	//	}
	//	if (feature->getContourParticleContext()!=NULL){
	//		hasParticles = true;
	//	}
	//}

	//if (hasParticles){
	//	FILE *fp = NULL;
	//	string newFname = string(filename) + ".particle";
	//	if ((fp = fopen(newFname.c_str(),"w"))==NULL){
	//		char errmsg[512];
	//		sprintf(errmsg, "Simulation::writeData(), error opening file %s for writing", newFname.c_str());
	//		throw errmsg;
	//	}

	//	fprintf(fp,"%lg %lg %lg\n", ((CartesianMesh*)_mesh)->getDomainSizeX(),
	//					((CartesianMesh*)_mesh)->getDomainSizeY(),
	//					((CartesianMesh*)_mesh)->getDomainSizeZ());
	//	fprintf(fp,"%d %d %d\n", ((CartesianMesh*)_mesh)->getNumVolumeX(),
	//				((CartesianMesh*)_mesh)->getNumVolumeY(),
	//				((CartesianMesh*)_mesh)->getNumVolumeZ());

	//	//
	//	//write particle file (if needed)
	//	//
	//	vector<Particle*>::iterator particleIterator;
	//	for (particleIterator = globalParticleList.begin();particleIterator != globalParticleList.end();particleIterator++){
	//		(*particleIterator)->write(fp);
	//	}

	//	fclose(fp);
	//}

	FVDataSet::write(filename, this, bCompress);
}

bool SimulationExpression::isVariable(string& symbol) {

	for (int i = 0; i < (int)varList.size(); i ++) {
		if (symbol == varList[i]->getName()) {
			return true;
		}
	}
	// parameter, serial scan parameter is like constant
	// field data, random variables, 
	// region size variables are only space dependent
	return false;
}

void SimulationExpression::createPostProcessingBlock() {
	if (postProcessingBlock == NULL) {
		postProcessingBlock = new PostProcessingBlock(this);
	}
}

void SimulationExpression::populateFieldValuesNew(double* darray, int index) {
	int indexOffset = symbolIndexOffset_FieldData;
	for (int i = 0; i < (int)fieldDataList.size(); i ++, ++ indexOffset) {
		int dataLength = fieldDataList[i]->getDataLength();
		if (index >= 0 && index < dataLength) {
			darray[indexOffset] = fieldDataList[i]->getData()[index];
		} else {
			darray[indexOffset] = 0;
		}
	}
}

void SimulationExpression::populateRandomValuesNew(double* darray, int index) {
	int indexOffset = symbolIndexOffset_RandomVar;
	for (int i = 0; i < (int)randomVarList.size(); i ++, ++ indexOffset) {
		RandomVariable* rv = randomVarList[i];

		if (index >= 0 && index < rv->getSize()) {
			darray[indexOffset] = rv->getRandomNumbers()[index];
		} else {
			darray[indexOffset] = 0;
		}
	}
}

void SimulationExpression::populateRegionSizeVariableValuesNew(double *darray, bool bVolumeRegion, int regionIndex) {
	int indexOffset = symbolIndexOffset_RegionSizeVariable;
	for (int i = 0; i < numRegionSizeVars; i ++, ++ indexOffset) {
		RegionSizeVariable* rsv = regionSizeVarList[i];
		if (rsv->getVarType() == VAR_VOLUME_REGION) {
			darray[indexOffset] = bVolumeRegion ? rsv->getCurr()[regionIndex] : 0;
		} else {
			darray[indexOffset] = bVolumeRegion ? 0 : rsv->getCurr()[regionIndex];
		}
	}
}

void SimulationExpression::populateParameterValuesNew(double* darray) {
	if (paramList.size() == 0) {
		return;
	}

	int indexOffset = symbolIndexOffset_RandomVar;
	for (int i = 0; i < (int)paramList.size(); i ++, ++ indexOffset) {
		darray[indexOffset] = paramValueProxies[i]->evaluate();
	}
}

void SimulationExpression::populateParticleVariableValuesNew(double* array, bool bVolume, int index)
{
	if (bVolume)
	{
		for (int i = 0; i < volParticleVarSize; i ++)
		{
			Variable* var = volParticleVarList[i];
			array[symbolIndexOffset_VolParticleVar + i] = var->getCurr(index);
		}
	}
	else
	{
		// Membrane Particle
		for (int i = 0; i < memParticleVarSize; i ++)
		{
			Variable* var = memParticleVarList[i];
			array[symbolIndexOffset_MemParticleVar + i] = var->getCurr(index);
		}
	}
}
