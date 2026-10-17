/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/FastSystem.h>
#include <VCELL/Variable.h>
#include <iostream>
using std::cout;
using std::endl;

FastSystem::FastSystem(int dim, int numDepend) : AlgebraicSystem(dim)
{
	currIndex = 0;
	numDependents = numDepend;
	pVars = new Variable*[getDimension()];
	if(numDepend>0){
		pDependentVars = new Variable*[numDependents];
		dependencyMatrix = new double*[numDependents];
		for(int i=0; i<numDependents; i++){
			dependencyMatrix[i] = new double[getDimension()+1];
		}
	}else{
		pDependentVars = NULL;
		dependencyMatrix = NULL;
	}       
}

void FastSystem::updateVars()
{
	int dim = getDimension();
	for(int i=0; i<dim; i++){
		pVars[i]->setCurr(currIndex, getX(i));
	}
	updateDependentVars();
}

void FastSystem::showVars()
{
	int dim = getDimension();
	for(int i=0; i<dim; i++){
		cout << pVars[i]->getCurr(currIndex) << endl;
	}
	if(pDependentVars){
		int numDep = getNumDependents();
		for(int j=0; j<numDep; j++){
			cout << pDependentVars[j]->getCurr(currIndex) << endl;
		}
	}
}

void FastSystem::setDependencyMatrix(int i, int j, double value)
{
	if(pDependentVars){
		int dim = getDimension();
		int numDep = getNumDependents();
		ASSERTION((i>=0)&&(i<numDep)&&(j>=0)&&(j<dim+1));
		dependencyMatrix[i][j] = value;
	}     
}

void FastSystem::updateDependentVars()
{
	if(pDependentVars){
		int dim = getDimension();
		int numDep = getNumDependents();
		for(int i=0; i<numDep; i++){
			double value = dependencyMatrix[i][dim];
			for(int j=0; j<dim; j++){
				value += dependencyMatrix[i][j]*(pVars[j]->getCurr(currIndex));
			}
			pDependentVars[i]->setCurr(currIndex, value); 
		}
	}  
}
