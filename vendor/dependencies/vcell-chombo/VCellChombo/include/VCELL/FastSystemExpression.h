#ifndef FASTSYSTEMEXPRESSION_H
#define FASTSYSTEMEXPRESSION_H

#include <VCELL/AlgebraicSystem.h>
#include <string>
using std::string;

class SimulationExpression;
class SimpleSymbolTable;
class Variable;
class Structure;
class VolIndex;
template <class> class LevelData;
class ChomboSemiImplicitScheduler;
namespace VCell {
	class Expression;
}

//-----------------------------------------------------------------
//
//  class FastSystem
//
//-----------------------------------------------------------------
class FastSystemExpression : public AlgebraicSystem
{
public:
    FastSystemExpression(Structure* structure, int dimension, int numDepend, SimulationExpression* sim);
	~FastSystemExpression();
	
	virtual void resolveReferences();

	void setPseudoConstants(string* symbols, VCell::Expression** expressions);
  void setFastRateExpressions(VCell::Expression** expressions);
	void setFastDependencyExpressions(string* symbols, VCell::Expression** expressions);
	void setJacobianExpressions(VCell::Expression** expressions);

	void setDependentVariables(string* vars); // must be called before other setters
	void setIndependentVariables(string* vars); // must be called before other setters
	void updateMatrix();

	void updateVars();
	inline int getNumDependents() {return numDependents;}
	void solve(ChomboSemiImplicitScheduler* scheduler, int iphase, int ivol);

private:
	Structure* structure;

	int numFastSymbols;
	string* pseudoSymbols;
	SimulationExpression* simulation;
	double* fastValues;	
	VCell::Expression** pseudoConstantExpressions;
	VCell::Expression** fastRateExpressions;
	VCell::Expression** fastDependencyExpressions;
	VCell::Expression** jacobianExpressions;
	Variable **pIndependentVars;
	int numDependents;
	Variable **pDependentVars;
	double **dependencyMatrix;

	int* dependentVarComponentIndexes;
	int* independentVarComponentIndexes;

	void bindAllExpressions();
	SimpleSymbolTable* createFastSymbolTable();
};

#endif
