/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/ChomboScheduler.h>
#include <VCELL/SimulationExpression.h>
#include <VCELL/SimTool.h>
#include <VCELL/VolumeVariable.h>
#include <VCELL/MembraneVariable.h>
#include <VCELL/VolumeRegionVariable.h>
#include <VCELL/MembraneRegionVariable.h>
#include <VCELL/Feature.h>
#include <VCELL/Membrane.h>
#include <VCELL/VCellModel.h>
#include <VCELL/DataSet.h>
#include <VCELL/PostProcessingBlock.h>
#include <SimpleSymbolTable.h>
#include <sstream>
#include <assert.h>

SimulationExpression::SimulationExpression(SimTool* a_simTool) : simTool(a_simTool)
{
	currIteration = 0;
	_scheduler = NULL;

	bHasElliptic = false;
	bHasParabolic = false;
	bHasFastSystem = false;
	
	symbolTable = NULL;	

	numMemPde = 0;
	numVolPde = 0;
	bHasTimeDependentDiffusionAdvection = false;

	volVarSize = 0;
	memVarSize = 0;
	volRegionVarSize = 0;
	memRegionVarSize = 0;

	volVarList = 0;
	memVarList = 0;
	volRegionVarList = 0;
	memRegionVarList = 0;

	outputVarNames = 0;
	outputVarCnt = 0;
	outputVarTypes = 0;

	postProcessingBlock = NULL;
	timeIntervalIndex = 0;
}

SimulationExpression::~SimulationExpression() 
{
	for (int i = 0; i < (int)varList.size(); i ++) {
		delete varList[i];
	}
	varList.clear();
	delete _scheduler;
	
	delete symbolTable;

	delete[] volVarList;
	delete[] memVarList;
	delete[] volRegionVarList;
	delete[] memRegionVarList;

	for (int i = 0; i < outputVarCnt; ++ i)
	{
		delete[] outputVarNames[i];
	}
	delete[] outputVarNames;
	delete[] outputVarTypes;
}

void SimulationExpression::iterate(bool bSolve)
{
	if (bSolve)
	{
		_scheduler->iterate();
	}
	currIteration ++;
	timeIntervals[timeIntervalIndex].nextStep();
	if (timeIntervals[timeIntervalIndex].ended() && timeIntervalIndex < timeIntervals.size() - 1)
	{
		++ timeIntervalIndex;
	}
}

Variable* SimulationExpression::getVariable(int index) {
	if (index < 0 || index >= (int)varList.size()) {
		throw "Simulation: getVariable(index) : index out of bounds";
	}
	return varList.at(index);
}

Variable *SimulationExpression::getVariableFromName(string& varName)
{
	for (int i = 0; i < (int)varList.size(); i ++) {
		Variable* var = varList[i];
		if (varName == var->getName()){
			return var;
		}
	}
	return NULL;
}

Variable *SimulationExpression::getVariableFromName(char* varName)
{
	string vn(varName);
	return getVariableFromName(vn);
}

void SimulationExpression::initSimulation()
{
	if (symbolTable == 0)
	{
		createSymbolTable();
	}
	VCellModel *model = SimTool::getInstance()->getModel();
	model->resolveReferences();
	if (postProcessingBlock != NULL)
	{
		postProcessingBlock->resolveReferences();
	}
		_scheduler->initValues();
	currIteration = 0;
}

double SimulationExpression::getTime_sec() 
{
	return timeIntervals[timeIntervalIndex].getTime();
}

void SimulationExpression::addVariable(Variable *var)
{
	varList.push_back(var);
	switch (var->getVarType()) {
	case VAR_VOLUME:
		volVarSize ++;
		if (var->isDiffusing()) {
			numVolPde ++;
		}
		break;
	case VAR_MEMBRANE:
		memVarSize ++;
		if (var->isDiffusing()) {
			numMemPde ++;
		}
		break;
	case VAR_VOLUME_REGION:
		volRegionVarSize ++;
		break;
	case VAR_MEMBRANE_REGION:
		memRegionVarSize ++;
		break;
	default:
		std::stringstream ss;
		ss << "Variable type " << var->getVarType() << " is not supported yet";
		throw ss.str();
	}	
}

void SimulationExpression::createSymbolTable() {	
	if (symbolTable != NULL) {
		return;
	}

	VCellModel* model = SimTool::getInstance()->getModel();

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
	int volVarCount = 0, memVarCount = 0, volRegionVarCount = 0, memRegionVarCount = 0;
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
		}
	}
	
	// t, x, y, z, VOL, VOL_Feature1_membrane, VOL_Feature2_membrane, ... (for computing membrane flux for volume variables), 
	// MEM, VOLREG, VOLREG_Feature1_membrane, VOLREG_Feature2_membrane, ... (for computing membrane flux for volume region variables), 
	// MEMREG
	numSymbols = 4 + volVarSize * (model->getNumFeatures() + 1) + memVarSize
		+ volRegionVarSize * (model->getNumFeatures() + 1) + memRegionVarSize;
	string* variableNames = new string[numSymbols];	

	variableNames[0] = "t";
	variableNames[1] = "x";	
	variableNames[2] = "y";	
	variableNames[3] = "z";	

	int variableIndex = 4;

	// Volume PDE/ODE
	for (int i = 0; i < volVarSize; i ++) {
		Variable* var = volVarList[i];
		variableNames[variableIndex] = string(var->getName());
		variableIndex ++;

		for (int f = 0; f < model->getNumFeatures(); f ++) {
			Feature* feature = model->getFeatureFromIndex(f);
			variableNames[variableIndex] = var->getName() + "_" + feature->getName() + "_membrane";
			variableIndex ++;
		}
	}

	// Membrane ODE/PDE
	for (int i = 0; i < memVarSize; i ++) {
		Variable* var = memVarList[i];
		variableNames[variableIndex] = string(var->getName());
		variableIndex ++;
	}
		
	// Volume Region
	for (int i = 0; i < volRegionVarSize; i ++) {
		Variable* var = volRegionVarList[i];
		variableNames[variableIndex] = string(var->getName());
		variableIndex ++;

		for (int f = 0; f < model->getNumFeatures(); f ++) {
			Feature* feature = model->getFeatureFromIndex(f);
			variableNames[variableIndex] = var->getName() + "_" + feature->getName() + "_membrane";
			variableIndex ++;
		}
	} 		
		
	// Membrane Region
	for (int i = 0; i < memRegionVarSize; i ++) {
		Variable* var = memRegionVarList[i];
		variableNames[variableIndex] = string(var->getName());
		variableIndex ++;
	}

	assert(numSymbols == variableIndex);
	symbolTable = new SimpleSymbolTable(variableNames, variableIndex);
	delete[] variableNames;
}

int  SimulationExpression::getNumSymbols() {
	return numSymbols;
}

char** SimulationExpression::getOutputVarNames()
{
	if (outputVarCnt == 0)
	{
		outputVarCnt = 0;
		for (int i = 0; i < varList.size(); i ++)
		{
			++ outputVarCnt;
#ifndef CH_MPI
			Variable* var = varList[i];
			if (var->getExactErrorVariable() != NULL)
			{
				++ outputVarCnt;
				++ outputVarCnt;
			}
#endif
		}
		outputVarNames = new char*[outputVarCnt];
		outputVarTypes = new int[outputVarCnt];
		int varcnt = 0;
		for (int i = 0; i < varList.size(); i ++)
		{
			Variable* var = varList[i];
			outputVarTypes[varcnt] = var->getVarType();
			string name = var->getQualifiedName();
			outputVarNames[varcnt] = new char[name.size() + 1];
			sprintf(outputVarNames[varcnt], "%s", name.c_str());
			++ varcnt;

#ifndef CH_MPI
			Variable* errVar = var->getExactErrorVariable();
			if (errVar != NULL)
			{
				outputVarTypes[varcnt] = var->getVarType();
				name = errVar->getQualifiedName();
				outputVarNames[varcnt] = new char[name.size() + 1];
				sprintf(outputVarNames[varcnt], "%s", name.c_str());
				++ varcnt;

				errVar = var->getRelativeErrorVariable();
				outputVarTypes[varcnt] = var->getVarType();
				name = errVar->getQualifiedName();
				outputVarNames[varcnt] = new char[name.size() + 1];
				sprintf(outputVarNames[varcnt], "%s", name.c_str());
				++ varcnt;
			}
#endif
		}
	}
	return outputVarNames;
}

int SimulationExpression::getOutputVarCount()
{
	getOutputVarNames();
	return outputVarCnt;
}

int* SimulationExpression::getOutputVarTypes()
{
	getOutputVarNames();
	return outputVarTypes;
}

PostProcessingBlock* SimulationExpression::createPostProcessingBlock() {
	if (postProcessingBlock == NULL)
	{
		postProcessingBlock = new PostProcessingBlock(this);
	}
	return postProcessingBlock;
}

PostProcessingBlock* SimulationExpression::getPostProcessingBlock()
{
	return postProcessingBlock;
}
