/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <sstream>
using std::stringstream;

#include <VCELL/VarContext.h>
#include <VCELL/Variable.h>
#include <VCELL/SimulationExpression.h>
#include <VCELL/Membrane.h>
#include <VCELL/JumpCondition.h>
#include <VCELL/SimTool.h>
#include <Expression.h>

#include <memory.h>

VarContext::VarContext(Structure *s, Variable* var)
{
	structure = s;
	variable = var;
	sim = NULL;	

	expressions = new VCell::Expression*[TOTAL_NUM_EXPRESSIONS];
	constantValues = new double*[TOTAL_NUM_EXPRESSIONS];
	needsXYZ = new bool[TOTAL_NUM_EXPRESSIONS];
	memset(expressions, 0, TOTAL_NUM_EXPRESSIONS * sizeof(Expression*));
	memset(constantValues, 0, TOTAL_NUM_EXPRESSIONS * sizeof(double*));
	memset(needsXYZ, 0, TOTAL_NUM_EXPRESSIONS * sizeof(bool));
}

void VarContext::resolveReferences(SimulationExpression *Asim)
{
	sim = Asim;
#ifndef CH_MPI
	if (expressions[EXACT_EXP] != NULL)
	{
		variable->createErrorVariables();
	}
#endif
}

VarContext::~VarContext()
{
	for (int i = 0; i < TOTAL_NUM_EXPRESSIONS; i ++) {
		delete expressions[i];		
		delete constantValues[i];
	}	
	delete[] expressions;
	delete[] constantValues;
	delete[] needsXYZ;

	for (int i = 0; i < (int)jumpConditionList.size(); i ++) {
		delete jumpConditionList[i];
	}
	jumpConditionList.clear();	
}

void VarContext::setExpression(VCell::Expression* newexp, ExpressionIndex expIndex) {
	if (expressions[expIndex] != 0) {
		stringstream ss;
		ss << "Expression " << String_Expression_Index[expIndex] << " for variable " << variable->getName() << " in Structure "
			<< structure->getName() << " has been set already";
		throw ss.str();
	}
	expressions[expIndex] = newexp;
}

void VarContext::bindAll(SimulationExpression* simulation) {
	SymbolTable* symbolTable = simulation->getSymbolTable();

	for (int i = 0; i < TOTAL_NUM_EXPRESSIONS; i ++) {
		if (expressions[i] == 0) {
			if (isNullExpressionOK((ExpressionIndex)i)) {
				continue;
			} else {
				stringstream ss;
				ss << "VarContext::bindAll(), expression " << String_Expression_Index[i] << " for variable " << variable->getName() << " not defined";
				throw ss.str();
			}
		}
		try {
			//cout << expressions[i]->infix() << endl;
			double d = expressions[i]->evaluateConstant();
			constantValues[i] = new double[1];
			constantValues[i][0] = d;
		} catch (...) {		
			expressions[i]->bindExpression(symbolTable);
			if (expressions[i]->getSymbolBinding("x") != NULL ||
					expressions[i]->getSymbolBinding("y") != NULL ||
					expressions[i]->getSymbolBinding("z") != NULL) {
				needsXYZ[i] = true;
			}
		}
	}

	for (int i = 0; i < (int)jumpConditionList.size(); i ++) {
		jumpConditionList[i]->bindExpression(symbolTable);
	}
}

double VarContext::evaluateConstantExpression(ExpressionIndex expIndex) {
	// pure constant
	if (constantValues[expIndex] != 0) {
		return constantValues[expIndex][0];
	}

	stringstream ss;
	ss << "VarContext::evaluateConstantExpression(), for variable " << variable->getName() << " expression " << String_Expression_Index[expIndex] << " not defined OR not a constant expression";
	throw ss.str();
}

double VarContext::evaluateExpression(ExpressionIndex expIndex, double* values) {
	if (expressions[expIndex] == 0) { // not defined
		stringstream ss;
		ss << "VarContext::evaluateExpression(), for variable " << variable->getName() << " expression " << String_Expression_Index[expIndex] << " not defined";
		throw ss.str();
	}
	if (constantValues[expIndex] != NULL) {
		return constantValues[expIndex][0];
	}
	return expressions[expIndex]->evaluateVector(values);	
}

void VarContext::addJumpCondition(Membrane* membrane, VCell::Expression* exp) {
	JumpCondition* jc = new JumpCondition(membrane, exp);
	jumpConditionList.push_back(jc);
}

double VarContext::evaluateJumpCondition(Membrane* membrane, double* values)
{
	for (int i = 0; i < (int)jumpConditionList.size(); i ++) {
		if (jumpConditionList[i]->getMembrane() == membrane) {
			return jumpConditionList[i]->evaluateExpression(values);
		}
	}
	stringstream ss;
	ss << "Jump Condition for variable " << variable->getName() << " in Feature " << structure->getName()
		<< " not found for Membrane " << membrane->getName();
	throw ss.str();
}

bool VarContext::isConstantExpression(ExpressionIndex expIndex) {
		// pure constant
	if (constantValues[expIndex] != 0) {
		return true;
	}

	// not defined
	if (expressions[expIndex] == 0) {
		stringstream ss;
		ss << "VarContext::isConstantExpression(), for variable " << variable->getName() << " expression " << String_Expression_Index[expIndex] << " not defined";
		throw ss.str();
	}
	
	return true;
}
