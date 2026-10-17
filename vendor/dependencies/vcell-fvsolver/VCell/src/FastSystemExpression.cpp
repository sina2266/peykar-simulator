/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/FastSystemExpression.h>

#include <VCELL/DoubleVector3.h>
#include <VCELL/SimulationExpression.h>
#include <VCELL/Variable.h>
#include <VCELL/CartesianMesh.h>
#include <VCELL/SimTypes.h>
#include <VCELL/RandomVariable.h>

#include <SimpleSymbolTable.h>
#include <Expression.h>
using VCell::Expression;

#include <sstream>
using std::stringstream;

#include <string.h>

FastSystemExpression::FastSystemExpression(int dimension, int numDepend, SimulationExpression* sim) 
: FastSystem(dimension, numDepend)
{	
	simulation = sim;

	if (simulation->getNumParameters() > 0) {
		throw "Fast System doesn't support parameter scans yet";
	}

	pseudoConstants = new double[numDependents];
	memset(pseudoConstants, 0, numDependents * sizeof(double));	
	for (int i = 0; i < dimension; i ++) {
		pVars[i] = NULL;
	}
	for (int i = 0; i < numDependents; i ++) {
		pDependentVars[i] = NULL;
	}
	// which includes t,x,y,z,field data, random variables, independent variables and pseudo constants
	fastSymbolTable = NULL;
	int numFastSymbols = 4 + simulation->getNumFields() + simulation->getNumRandomVariables() + dimension + numDependents;
	fastValues = new double[numFastSymbols];
	memset(fastValues, 0, sizeof(double) * numFastSymbols);
	pseudoConstantExpressions = NULL;
	fastRateExpressions = NULL;
	fastDependencyExpressions = NULL;
	jacobianExpressions = NULL;	
	pseudoSymbols = 0;

	setTolerance(1e-7);
}

FastSystemExpression::~FastSystemExpression()
{
	delete[] pseudoConstants;
	delete[] fastSymbolTable;
	delete[] fastValues;
	for (int i = 0; i < dimension; i ++) {
		delete fastRateExpressions[i];
		for (int j = 0; j < dimension; j ++) {
			delete jacobianExpressions[i];
		}
	}
	for (int i = 0; i < numDependents; i ++) {
		delete pseudoConstantExpressions[i];
		delete fastDependencyExpressions[i];
	}
	delete[] pseudoConstantExpressions;
	delete[] fastRateExpressions;
	delete[] fastDependencyExpressions;
	delete[] jacobianExpressions;
}

void FastSystemExpression::setPseudoConstants(string* symbols, Expression** expressions) {
	pseudoSymbols = symbols;
	pseudoConstantExpressions = expressions;	
}

void FastSystemExpression::setFastRateExpressions(Expression** expressions) {
	fastRateExpressions = expressions;
}

void FastSystemExpression::bindAllExpressions() {
	if (dimension < 1 || pVars[0] == 0) {		
		throw "No independent variables defined in Fast System";
	}
	if (numDependents > 0 && pDependentVars[0] == 0) {
		throw "No dependent variables defined in Fast System";
	}

	//bind pseudo constants;
	int numSymbols = 4 + dimension + numDependents;
	string* indepAndDepSymbols = new string[numSymbols];
	indepAndDepSymbols[0] = "t";
	indepAndDepSymbols[1] = "x";
	indepAndDepSymbols[2] = "y";
	indepAndDepSymbols[3] = "z";
	for (int i = 0; i < dimension; i ++) {
		indepAndDepSymbols[i + 4] = pVars[i]->getName();
	}
	for (int i = 0; i < numDependents; i ++) {		
		indepAndDepSymbols[i + 4 + dimension] = pDependentVars[i]->getName();
	}
	SimpleSymbolTable* simpleSymbolTable = new SimpleSymbolTable(indepAndDepSymbols, numSymbols);	
	for (int i = 0; i < numDependents; i ++) {
		pseudoConstantExpressions[i]->bindExpression(simpleSymbolTable);
	}
	delete[] indepAndDepSymbols;

	// bind fast rate
	for (int i = 0; i < dimension; i ++) {		
		fastRateExpressions[i]->bindExpression(getFastSymbolTable());
	}

	// bind fast dependency
	for (int i = 0; i < numDependents; i ++) {		
		fastDependencyExpressions[i]->bindExpression(getFastSymbolTable());
	}
	// bind jacobian
	for (int i = 0; i < dimension * dimension; i ++) {		
		jacobianExpressions[i]->bindExpression(getFastSymbolTable());		
	}
}

void FastSystemExpression::setFastDependencyExpressions(string* symbols, Expression** expressions){
	for (int i = 0; i < numDependents; i ++) {
		if (symbols[i] != pDependentVars[i]->getName()) {
			throw "Fast dependency is out of order";
		}
	}
	fastDependencyExpressions = expressions;
}

void FastSystemExpression::setJacobianExpressions(Expression** expressions) {
	jacobianExpressions = expressions;
}

void FastSystemExpression::setCoordinates(double time_sec, WorldCoord& wc) {
	// t, x, y, x
	fastValues[0] = time_sec;
	fastValues[1] = wc.x;
	fastValues[2] = wc.y;
	fastValues[3] = wc.z;
	// field data
	simulation->populateFieldValues(fastValues + 4, currIndex);
	// random variables
	simulation->populateRandomValues(fastValues + 4 + simulation->getNumFields(), currIndex);
}


void FastSystemExpression::initVars(SimTool* sim_tool)
{
	// independent variables
	int numSymbols = 4 + dimension + numDependents;
	double* values = new double[numSymbols];

	WorldCoord wc = simulation->getMesh()->getVolumeWorldCoord(currIndex);
	values[0] = simulation->getTime_sec(sim_tool);
	values[1] = wc.x;
	values[2] = wc.y;
	values[3] = wc.z;

	for (int i = 0; i < dimension; i ++) {		
		values[4 + i] = pVars[i]->getCurr(currIndex);
		setX(i, values[4 + i]);		
	}

	// dependent variables
	for (int i = 0; i < numDependents; i ++) {
		values[4 + i + dimension] = pDependentVars[i]->getCurr(currIndex);
	}

	// set values of pseudo constants by using initial values of independent and dependent variables.
	int pseudoConstantOffset = 4 + simulation->getNumFields() + simulation->getNumRandomVariables() + dimension;
	for (int i = 0; i < numDependents; i ++) {
		pseudoConstants[i] = pseudoConstantExpressions[i]->evaluateVector(values);
		fastValues[pseudoConstantOffset + i] = pseudoConstants[i];
	}

	delete[] values;
}

void FastSystemExpression::updateDependentVars() {	
	updateIndepValues();
	for (int i = 0; i < numDependents; i ++) {
		pDependentVars[i]->setCurr(currIndex, fastDependencyExpressions[i]->evaluateVector(fastValues));
	}	
}

void FastSystemExpression::setDependentVariables(string* vars) const
{
	for (int i = 0; i < numDependents; i ++) {		
		pDependentVars[i] = simulation->getVariableFromName(vars[i]);
		if (pDependentVars[i] == NULL){
			stringstream ss;
			ss << "could not resolve variable " << vars[i];
			throw ss.str();
		}
	}
}

void FastSystemExpression::setIndependentVariables(string* vars) const
{
	for (int i = 0; i < dimension; i ++) {		
		pVars[i] = simulation->getVariableFromName(vars[i]);
		if (pVars[i] == NULL){
			stringstream ss;
			ss << "could not resolve variable " << vars[i];
			throw ss.str();
		}		
	}
}

SimpleSymbolTable* FastSystemExpression::getFastSymbolTable() {
	if (fastSymbolTable == nullptr) {
		for (int i = 0; i < dimension; i ++) {
			if (pVars[i] == nullptr) {
				throw std::runtime_error("No independent variables defined");
			}
		}

		int numFields = simulation->getNumFields();
		int numRandomVariables = simulation->getNumRandomVariables();
		int numSymbols = 4 + numFields + numRandomVariables + dimension + numDependents;
		string* fastSymbols = new string[numSymbols];

		int symbolCount = 0;

		// t, x, y, x
		fastSymbols[symbolCount ++] = "t";
		fastSymbols[symbolCount ++] = "x";
		fastSymbols[symbolCount ++] = "y";
		fastSymbols[symbolCount ++] = "z";

		// field data
		string* fieldSymbols = simulation->getFieldSymbols();
		for (int i = 0; i < numFields; i ++) {
			fastSymbols[symbolCount ++] = fieldSymbols[i];
		}
		delete[] fieldSymbols;

		// random variables
		for (int i = 0; i < numRandomVariables; i ++) {
			fastSymbols[symbolCount ++] = simulation->getRandomVariable(i)->getName();
		}

		// independent variables
		for (int i = 0; i < dimension; i ++) {		
			fastSymbols[symbolCount ++] = string(pVars[i]->getName());
		}

		// pseudo constant symbols
		if (pseudoSymbols != NULL) {
			for (int i = 0; i < numDependents; i ++) {
				fastSymbols[symbolCount ++] = pseudoSymbols[i];
			}
		}
		fastSymbolTable = new SimpleSymbolTable(fastSymbols, symbolCount);	
		delete[] fastSymbols;
	}
	return fastSymbolTable;
}

void FastSystemExpression::updateIndepValues() const
{
	int indepOffset = 4 + simulation->getNumFields() + simulation->getNumRandomVariables();
	for (int i = 0; i < dimension; i ++) {		
		fastValues[indepOffset + i] = getX(i);
	}	
}

void FastSystemExpression::resolveReferences(SimulationExpression *sim) {
	bindAllExpressions();
}

void FastSystemExpression::updateMatrix()
{
	updateIndepValues();
	double mvalue = 0;
	int i, j;
	for (i = 0; i < dimension; i ++) {
		for (j = 0; j < dimension; j ++) {			
			mvalue = jacobianExpressions[i * dimension + j]->evaluateVector(fastValues);
			setMatrix(i, j, mvalue);		
		}
		mvalue = -fastRateExpressions[i]->evaluateVector(fastValues);
		setMatrix(i, j, mvalue);
	}	 
}
