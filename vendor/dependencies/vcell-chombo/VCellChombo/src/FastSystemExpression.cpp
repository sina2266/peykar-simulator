/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <RealVect.H>
#include <EBArith.H>

#include <VCELL/FastSystemExpression.h>

#include <VCELL/SimulationExpression.h>
#include <VCELL/ChomboSemiImplicitScheduler.h>
#include <VCELL/Variable.h>
#include <VCELL/Structure.h>

#include <SimpleSymbolTable.h>
#include <Expression.h>
using VCell::Expression;

#include <sstream>
using std::stringstream;

#include <string.h>

/*
 *  pseudo constant expressions only have independent and dependent variables.
 *  all other expressions have independent variables and pseudo constants.
 *  so there are 2 symbol tables.
 */
FastSystemExpression::FastSystemExpression(Structure* structure, int dim, int numDepend, SimulationExpression* sim)
: AlgebraicSystem(dim)
{
	this->structure = structure;
	numDependents = numDepend;
	pIndependentVars = new Variable*[dimension];  // dimension is #indep
	independentVarComponentIndexes = new int[dimension];  // dimension is #indep
	if (numDepend > 0)
	{
		pDependentVars = new Variable*[numDependents];
		dependentVarComponentIndexes = new int[numDependents];
		dependencyMatrix = new double*[numDependents];
		for(int i = 0; i < numDependents; i ++)
		{
			dependencyMatrix[i] = new double[dimension + 1];
		}
	}
	else
	{
		pDependentVars = NULL;
		dependencyMatrix = NULL;
	}

	simulation = sim;

	for (int i = 0; i < dimension; i ++) {
		pIndependentVars[i] = NULL;
	}
	for (int i = 0; i < numDependents; i ++) {
		pDependentVars[i] = NULL;
	}
	// which includes t,x,y,z,independent variables and pseudo constants
	numFastSymbols = 4 + dimension + numDependents; // dimension=#indep #dep=#pseudo constants
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
	delete[] independentVarComponentIndexes;
	delete[] dependentVarComponentIndexes;

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
	if (dimension < 1 || pIndependentVars[0] == 0) {
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
		indepAndDepSymbols[i + 4] = pIndependentVars[i]->getName();
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
	SimpleSymbolTable* fastSymbolTable = createFastSymbolTable();
	for (int i = 0; i < dimension; i ++) {		
		fastRateExpressions[i]->bindExpression(fastSymbolTable);
	}

	// bind fast dependency
	for (int i = 0; i < numDependents; i ++) {		
		fastDependencyExpressions[i]->bindExpression(fastSymbolTable);
	}
	// bind jacobian
	for (int i = 0; i < dimension * dimension; i ++) {		
		jacobianExpressions[i]->bindExpression(fastSymbolTable);
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

void FastSystemExpression::setDependentVariables(string* vars)
{
	for (int i = 0; i < numDependents; i ++)
	{
		int numDefinedVars = structure->getNumDefinedVariables();
		for (int ivar = 0; ivar < numDefinedVars; ++ ivar)
		{
			Variable* var = structure->getDefinedVariable(ivar);
			if (var->getName() == vars[i])
			{
				dependentVarComponentIndexes[i] = ivar;
				pDependentVars[i] = var;
				break;
			}
		}
		if (pDependentVars[i] == NULL)
		{
			stringstream ss;
			ss << "could not resolve variable " << vars[i];
			throw ss.str();
		}
	}
}

void FastSystemExpression::setIndependentVariables(string* vars)
{
	for (int i = 0; i < dimension; i ++)
	{
		int numDefinedVars = structure->getNumDefinedVariables();
		for (int ivar = 0; ivar < numDefinedVars; ++ ivar)
		{
			Variable* var = structure->getDefinedVariable(ivar);
			if (var->getName() == vars[i])
			{
				independentVarComponentIndexes[i] = ivar;
				pIndependentVars[i] = var;
				break;
			}
		}
		if (pIndependentVars[i] == NULL)
		{
			stringstream ss;
			ss << "could not resolve variable " << vars[i];
			throw ss.str();
		}		
	}
}

SimpleSymbolTable* FastSystemExpression::createFastSymbolTable() {
	for (int i = 0; i < dimension; i ++) {
		if (pIndependentVars[i] == NULL) {
			throw "No independent variables defined";
		}
	}

	int numSymbols = 4 + dimension + numDependents;
	string* fastSymbols = new string[numSymbols];

	int symbolCount = 0;

	// t, x, y, x
	fastSymbols[symbolCount ++] = "t";
	fastSymbols[symbolCount ++] = "x";
	fastSymbols[symbolCount ++] = "y";
	fastSymbols[symbolCount ++] = "z";

	// independent variables
	for (int i = 0; i < dimension; i ++) {
		fastSymbols[symbolCount ++] = string(pIndependentVars[i]->getName());
	}

	// pseudo constant symbols
	if (pseudoSymbols != NULL) {
		for (int i = 0; i < numDependents; i ++) {
			fastSymbols[symbolCount ++] = pseudoSymbols[i];
		}
	}
	SimpleSymbolTable* fastSymbolTable = new SimpleSymbolTable(fastSymbols, symbolCount);
	delete[] fastSymbols;
	return fastSymbolTable;
}

void FastSystemExpression::resolveReferences() {
	bindAllExpressions();
}

void FastSystemExpression::updateMatrix()
{
	// update independent portion of fastValues from X
	int indepOffset = 4;
	for (int i = 0; i < dimension; i ++) {
		fastValues[indepOffset + i] = getX(i);
	}

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

void FastSystemExpression::solve(ChomboSemiImplicitScheduler* scheduler, int iphase, int ivol)
{
	int numDefinedVolVars = structure->getNumDefinedVariables();
	// values are used to evaluate pseudo constants
	double* values = new double[numFastSymbols];
	values[0] = simulation->getTime_sec();
	// fast values are used to evaluate all the other expressions
	fastValues[0] = simulation->getTime_sec();

	for(int ilev = 0; ilev < scheduler->numLevels; ilev ++)
	{
		DisjointBoxLayout& currGrids = scheduler->vectGrids[ilev];

		for(DataIterator dit = currGrids.dataIterator(); dit.ok(); ++dit)
		{
			const EBISBox& currEBISBox = scheduler->vectEbis[iphase][ivol][ilev][dit()];
			const Box& currBox = scheduler->vectGrids[ilev][dit()];

			if (numDefinedVolVars == 0)
			{
				continue;
			}

			EBCellFAB& solnEBCellFAB = (*scheduler->volSoln[iphase][ivol][ilev])[dit()];
			FArrayBox& solnFab = solnEBCellFAB.getFArrayBox();
			IntVect solnSize = solnFab.size();
			const int *solnLo = solnFab.loVect();
			Real* solnDataPtr = solnFab.dataPtr();

#if CH_SPACEDIM==3
			for (int k = scheduler->numGhostSoln[2]; k < solnSize[2] - scheduler->numGhostSoln[2]; k ++)
			{ // phi has ghost point
#endif
				for (int j = scheduler->numGhostSoln[1]; j < solnSize[1] - scheduler->numGhostSoln[1]; j ++)
				{ // phi has ghost point
					for (int i = scheduler->numGhostSoln[0]; i < solnSize[0] - scheduler->numGhostSoln[0]; i ++)
					{
						IntVect gridIndex(D_DECL(i + solnLo[0], j + solnLo[1], k + solnLo[2]));
						IntVect localDataIndex(D_DECL(i, j, k));
						if (currEBISBox.isCovered(gridIndex) || scheduler->isInNextFinerLevel(ilev, gridIndex))
						{
							continue;
						}
						RealVect coord = EBArith::getIVLocation(gridIndex, scheduler->vectDxes[ilev], scheduler->chomboGeometry->getDomainOrigin());

						// ============================
            // initVars
						values[1] = coord[0];
						values[2] = coord[1];
#if CH_SPACEDIM == 3
						values[3] = coord[2];
#endif

						for (int n = 0; n < dimension; n ++)
						{
							int solnLocalIndex = scheduler->getChomboBoxLocalIndex(solnSize, independentVarComponentIndexes[n], localDataIndex);
							double sol = solnDataPtr[solnLocalIndex];
							values[4 + n] = sol;
							setX(n, sol);
						}

						// dependent variables
						for (int n = 0; n < numDependents; n ++)
						{
							int solnLocalIndex = scheduler->getChomboBoxLocalIndex(solnSize, dependentVarComponentIndexes[n], localDataIndex);
							double sol = solnDataPtr[solnLocalIndex];
							values[4 + dimension + n] = sol;
						}

						// t, x, y, x
						fastValues[1] = coord[0];
						fastValues[2] = coord[1];
#if CH_SPACEDIM == 3
						fastValues[3] = coord[2];
#endif

						// set pseudo constants portion of fastValues, this doesn't change during solve
						// independent portion is updated in updateMatrix() which gets from X
						int pseudoConstantOffset = 4 + dimension;
						for (int n = 0; n < numDependents; n ++) {
							double pseudoConstant = pseudoConstantExpressions[n]->evaluateVector(values);
							fastValues[pseudoConstantOffset + n] = pseudoConstant;
						}
						// ============================

						solveSystem();

						//=====================
						// updateVars();
						// update independent solutions and independent portion of fastValues used to get new dependent solutions
						int indepOffset = 4;
						for(int n = 0; n < dimension; n ++)
						{
							int solnLocalIndex = scheduler->getChomboBoxLocalIndex(solnSize, independentVarComponentIndexes[n], localDataIndex);
							double newsol = getX(n);
							solnDataPtr[solnLocalIndex] = newsol;
							fastValues[indepOffset + n] = newsol;
						}

						// update dependent solutions
						for (int n = 0; n < numDependents; n ++)
						{
							int solnLocalIndex = scheduler->getChomboBoxLocalIndex(solnSize, dependentVarComponentIndexes[n], localDataIndex);
							double newsol = fastDependencyExpressions[n]->evaluateVector(fastValues);
							solnDataPtr[solnLocalIndex] = newsol;
						}
					}
				}
#if CH_SPACEDIM==3
			}
#endif
		}
	}
	delete[] values;
}
