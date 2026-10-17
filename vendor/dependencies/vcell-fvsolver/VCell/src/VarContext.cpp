/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/VarContext.h>

#include <sstream>
using std::stringstream;

#include <Expression.h>
using VCell::Expression;

#include <VCELL/Element.h>
#include <VCELL/Variable.h>
#include <VCELL/SimulationExpression.h>
#include <VCELL/CartesianMesh.h>
#include <VCELL/Membrane.h>
#include <VCELL/JumpCondition.h>
#include <VCELL/SimTool.h>
#include <cstring>

VarContext::VarContext(Structure *s, Variable* var)
: expressions(TOTAL_NUM_EXPRESSIONS, nullptr),
  constantValues(TOTAL_NUM_EXPRESSIONS, nullptr),
  dependencyMask(TOTAL_NUM_EXPRESSIONS, DEPENDENCY_MASK_UNDEFINED),
  jumpConditionList(0)
{
	structure = s;
	ASSERTION(structure != nullptr);

	species = var;
	sim = nullptr;
}

void VarContext::resolveReferences(SimulationExpression *Asim)
{
	sim = Asim;
	if (sim == nullptr) {
		throw std::runtime_error("VarContext::resolveReference(), simulation can't be null");
	}
}

double VarContext::getInitialValue(long index)
{
	throw std::runtime_error("Application Error: neither initialValue nor getInitialValue() specified for VarContext");
}

VarContext::~VarContext()
{
	for (int i = 0; i < TOTAL_NUM_EXPRESSIONS; i ++) {
		delete expressions[i];		
		delete constantValues[i];
	}
	expressions.clear();
	constantValues.clear();
	dependencyMask.clear();

	for (auto & i : jumpConditionList) {
		delete i;
	}
	jumpConditionList.clear();	
}

void VarContext::setExpression(Expression* newexp, int expIndex) {
	if (expressions[expIndex] != nullptr) {
		stringstream ss;
		ss << "Expression " << String_Expression_Index[expIndex] << " for variable " << species->getName() << " in Structure " 
			<< structure->getName() << " has been set already";
		throw std::runtime_error(ss.str());
	}
	expressions[expIndex] = newexp;
}

void VarContext::bindAll(SimulationExpression* sim) {
	SymbolTable* symbolTable = sim->getSymbolTable();

	for (int i = 0; i < TOTAL_NUM_EXPRESSIONS; i ++) {
		if (expressions[i] == nullptr) {
			if (isNullExpressionOK(i)) {
				continue;
			} else {
				stringstream ss;
				ss << "VarContext::bindAll(), expression " << String_Expression_Index[i] << " for variable " << species->getName() << " not defined";
				throw std::runtime_error(ss.str());
			}
		}
		try {
			//cout << expressions[i]->infix() << std::endl;
			double d = expressions[i]->evaluateConstant();
			constantValues[i] = new double[1];
			constantValues[i][0] = d;
		} catch (...) {
			expressions[i]->bindExpression(symbolTable);
		}
		computeDependencyMask(sim, i);
	}

	for (auto & i : jumpConditionList) {
		i->bindExpression(symbolTable);
	}
}

double VarContext::evaluateExpression(MembraneElement* element, long expIndex) const
{
	if (expressions[expIndex] == nullptr) { // not defined
		throw std::runtime_error("VarContext::evaluateExpression(MembaneElement), expression not defined");
	}
	if (constantValues[expIndex] != nullptr) {
		return constantValues[expIndex][0];
	}
	//cout << expressions[expIndex]->infix() << endl;
	if (dependencyMask[expIndex] & DEPENDENCY_MASK_XYZ) {
		WorldCoord wc = sim->getMesh()->getMembraneWorldCoord(element);
		sim->setCurrentCoordinate(wc);
	}
	int* indices = sim->getIndices();
	indices[VAR_VOLUME_INDEX] = -1;
	indices[VAR_VOLUME_REGION_INDEX] = -1;	
	indices[VAR_MEMBRANE_INDEX] = element->index;
	indices[VAR_MEMBRANE_REGION_INDEX] = element->getRegionIndex();
	return expressions[expIndex]->evaluateProxy();	
}

double VarContext::evaluateConstantExpression(long expIndex) const
{
	// pure constant
	if (constantValues[expIndex] != nullptr) {
		return constantValues[expIndex][0];
	}

	stringstream ss;
	ss << "VarContext::evaluateConstantExpression(), for variable " << species->getName() << " expression " << String_Expression_Index[expIndex] << " not defined OR not a constant expression";
	throw std::runtime_error(ss.str());
}

double VarContext::evaluateExpression(long volIndex, long expIndex) const
{
	if (expressions[expIndex] == nullptr) { // not defined
		stringstream ss;
		ss << "VarContext::evaluateExpression(VolIndex), for variable " << species->getName() << " expression " << String_Expression_Index[expIndex] << " not defined";
		throw std::runtime_error(ss.str());
	}
	if (constantValues[expIndex] != nullptr) {
		return constantValues[expIndex][0];
	}
	if (dependencyMask[expIndex] & DEPENDENCY_MASK_XYZ) {
		WorldCoord wc = ((CartesianMesh*)sim->getMesh())->getVolumeWorldCoord(volIndex);
		sim->setCurrentCoordinate(wc);
	}
	int* indices = sim->getIndices();
	indices[VAR_MEMBRANE_INDEX] = -1;
	indices[VAR_MEMBRANE_REGION_INDEX] = -1;
	indices[VAR_VOLUME_INDEX] = volIndex;
	indices[VAR_VOLUME_REGION_INDEX] = sim->getMesh()->getVolumeElements()[volIndex].getRegionIndex();
	return expressions[expIndex]->evaluateProxy();	
}

double VarContext::evaluateExpression(long expIndex, double* values) const
{
	if (expressions[expIndex] == nullptr) { // not defined
		stringstream ss;
		ss << "VarContext::evaluateExpression(), for variable " << species->getName() << " expression " << String_Expression_Index[expIndex] << " not defined";
		throw std::runtime_error(ss.str());
	}
	if (constantValues[expIndex] != nullptr) {
		return constantValues[expIndex][0];
	}
	return expressions[expIndex]->evaluateVector(values);	
}

void VarContext::addJumpCondition(Membrane* membrane, Expression* exp) {
	auto* jc = new JumpCondition(membrane, exp);
	jumpConditionList.push_back(jc);
}

double VarContext::evaluateJumpCondition(MembraneElement* element) const
{
	for (auto & i : jumpConditionList) {
		if (i->getMembrane() == element->getMembrane()) {
			return i->evaluateExpression(sim, element);
		}
	}
	stringstream ss;
	ss << "Jump Condition for variable " << species->getName() << " in Feature " << structure->getName() 
		<< " not found for Membrane " << element->getMembrane()->getName();
	throw std::runtime_error(ss.str());
}

double VarContext::evaluateJumpCondition(MembraneElement* element, double* values) const
{
	for (auto & i : jumpConditionList) {
		if (i->getMembrane() == element->getMembrane()) {
			return i->evaluateExpression(values);
		}
	}
	stringstream ss;
	ss << "Jump Condition for variable " << species->getName() << " in Feature " << structure->getName() 
		<< " not found for Membrane " << element->getMembrane()->getName();
	throw std::runtime_error(ss.str());
}

bool VarContext::isConstantExpression(long expIndex) const
{
	if (dependencyMask[expIndex] == DEPENDENCY_MASK_CONSTANT) {
		return true;
	}

	// not defined
	if (expressions[expIndex] == nullptr) {
		stringstream ss;
		ss << "VarContext::isConstantExpression(), for variable " << species->getName() << " expression " << String_Expression_Index[expIndex] << " not defined";
		throw std::runtime_error(ss.str());
	}

	return false;
}

bool VarContext::isXYZOnlyExpression(long expIndex) const
{
	if (dependencyMask[expIndex] == DEPENDENCY_MASK_XYZ) {
		return true;
	}

	// not defined
	if (expressions[expIndex] == nullptr) {
		stringstream ss;
		ss << "VarContext::isConstantExpression(), for variable " << species->getName() << " expression " << String_Expression_Index[expIndex] << " not defined";
		throw std::runtime_error(ss.str());
	}

	return false;
}

void VarContext::computeDependencyMask(SimulationExpression* sim, int expIndex) {
	// not defined
	if (constantValues[expIndex] != nullptr) { // constant 0
		dependencyMask[expIndex] = DEPENDENCY_MASK_CONSTANT;
		return;
	}
	if (expressions[expIndex] == nullptr) { // expression is not defined
		return;
	}

	bool bHasXYZ = false;
	bool bHasVariable = false;
	bool bHasTime = false;
	vector<string> symbols;
	expressions[expIndex]->getSymbols(symbols);
	for (auto & symbol : symbols) {
		if (symbol == "t") {
			bHasTime = true;
		} else if (sim->isVariable(symbol)) {
			bHasVariable = true;
		} else if (!sim->isParameter(symbol)) {
			bHasXYZ = true;
		}
	}	
	
	if (!bHasXYZ && !bHasTime && !bHasVariable) {
		dependencyMask[expIndex] = DEPENDENCY_MASK_CONSTANT;
	} else {
		if (bHasXYZ) {
			dependencyMask[expIndex] |= DEPENDENCY_MASK_XYZ;
		}
		if (bHasTime) {
			dependencyMask[expIndex] |= DEPENDENCY_MASK_TIME;
		}
		if (bHasVariable) {
			dependencyMask[expIndex] |= DEPENDENCY_MASK_VARIABLE;
		}
	}
}

double VarContext::evaluateMembraneRegionExpression(long memRegionIndex, long expIndex) const
{
	int* indices = sim->getIndices();
	indices[VAR_VOLUME_INDEX] = -1;
	indices[VAR_VOLUME_REGION_INDEX] = -1;
	indices[VAR_MEMBRANE_INDEX] = -1;	
	indices[VAR_MEMBRANE_REGION_INDEX] = memRegionIndex;
	return expressions[expIndex]->evaluateProxy();	
}

double VarContext::evaluateVolumeRegionExpression(long volRegionIndex, long expIndex) const
{
	int* indices = sim->getIndices();
	indices[VAR_VOLUME_INDEX] = -1;
	indices[VAR_MEMBRANE_INDEX] = -1;
	indices[VAR_MEMBRANE_REGION_INDEX] = -1;	
	indices[VAR_VOLUME_REGION_INDEX] = volRegionIndex;
	return expressions[expIndex]->evaluateProxy();
}

void VarContext::reinitConstantValues(SimulationExpression* sim) {
	for (int i = 0; i < TOTAL_NUM_EXPRESSIONS; i ++) {
		if (expressions[i] == nullptr || !isConstantExpression(i)) {
			continue;
		}
		double d = expressions[i]->evaluateProxy();
		if (constantValues[i] == nullptr) {
			constantValues[i] = new double[1];
		}
		constantValues[i][0] = d;
	}

	for (auto & i : jumpConditionList) {
		i->reinitConstantValues(sim);
	}
}
