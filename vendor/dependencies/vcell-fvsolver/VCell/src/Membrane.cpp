#include <VCELL/Membrane.h>
#include <VCELL/MembraneVariable.h>
#include <VCELL/MembraneRegionVariable.h>
#include <VCELL/SimulationExpression.h>
#include <VCELL/Feature.h>
#include <VCELL/MembraneVarContextExpression.h>
#include <VCELL/MembraneRegionVarContextExpression.h>
#include <VCELL/FastSystem.h>
#include <VCELL/Element.h>
#include <VCELL/JumpCondition.h>
#include <SimpleSymbolTable.h>
#include <sstream>
using std::stringstream;

Membrane::Membrane(string& name, Feature* f1, Feature* f2) : Structure(name)
{
	feature1 = f1;
	feature2 = f2;
	fastSystem = 0;
}

Membrane::~Membrane(void)
{
	delete fastSystem;
	for (int i = 0; i < (int)membraneVarContextList.size(); i ++) {
		delete membraneVarContextList[i];
	}
	membraneVarContextList.clear();
	for (int i = 0; i < (int)membraneRegionVarContextList.size(); i ++) {
		delete membraneRegionVarContextList[i];
	}
	membraneRegionVarContextList.clear();
}

void Membrane::initMembraneValues(MembraneElement *membraneElement)
{
	for (int i = 0; i < (int)membraneVarContextList.size(); i ++) {
		MembraneVarContextExpression *membraneVarContext = membraneVarContextList[i];
		double value = membraneVarContext->getInitialValue(membraneElement);
		MembraneVariable* var = (MembraneVariable *)membraneVarContext->getVar();
		var->setOld(membraneElement->index, value);
		var->setCurr(membraneElement->index, value);
	}
}

void Membrane::initMembraneRegionValues(int membraneRegionIndex)
{
	for (int i = 0; i < (int)membraneRegionVarContextList.size(); i ++) {
		MembraneRegionVarContextExpression *membraneRegionVarContext = membraneRegionVarContextList[i];
		double value = membraneRegionVarContext->getInitialValue(membraneRegionIndex);
		MembraneRegionVariable* var = (MembraneRegionVariable *)membraneRegionVarContext->getVar();
		var->setOld(membraneRegionIndex, value);
		var->setCurr(membraneRegionIndex, value);
	}
}

void Membrane::addMembraneVarContext(MembraneVarContextExpression *mvc)
{
	membraneVarContextList.push_back(mvc);
}

void Membrane::addMembraneRegionVarContext(MembraneRegionVarContextExpression *mrvc)
{
	membraneRegionVarContextList.push_back(mrvc);
}

MembraneVarContextExpression* Membrane::getMembraneVarContext(MembraneVariable *memVar)
{
	for (int i = 0; i < (int)membraneVarContextList.size(); i ++) {
		if (membraneVarContextList[i]->getVar() == memVar) {
			return membraneVarContextList[i];
		}
	}
	return 0;
}

MembraneRegionVarContextExpression* Membrane::getMembraneRegionVarContext(MembraneRegionVariable *memRegionVar)
{
	for (int i = 0; i < (int)membraneRegionVarContextList.size(); i ++) {
		if (membraneRegionVarContextList[i]->getVar() == memRegionVar) {
			return membraneRegionVarContextList[i];
		}
	}
	return 0;
}

bool Membrane::inBetween(Feature* f1, Feature* f2) {
	return (f1 == feature1 && f2 == feature2 || f1 == feature2 && f2 == feature1);
}

void Membrane::resolveReferences(SimulationExpression *sim)
{
	for (int i = 0; i < (int)membraneVarContextList.size(); i ++) {
		MembraneVarContextExpression *membraneVarContext = membraneVarContextList[i];
		membraneVarContext->resolveReferences(sim);
	}
	
	for (int i = 0; i < (int)membraneRegionVarContextList.size(); i ++) {
		MembraneRegionVarContextExpression* membraneRegionVarContext = membraneRegionVarContextList[i];
		membraneRegionVarContext->resolveReferences(sim);
	}    

	if(fastSystem!=NULL){
		fastSystem->resolveReferences(sim);
	}
}

void Membrane::reinitConstantValues(SimulationExpression* sim) {
	for (int i = 0; i < (int)membraneVarContextList.size(); i ++) {
		membraneVarContextList[i]->reinitConstantValues(sim);
	}
	for (int i = 0; i < (int)membraneRegionVarContextList.size(); i ++) {
		membraneRegionVarContextList[i]->reinitConstantValues(sim);
	}
}
