#ifndef FASTSYSTEMEXPRESSION_H
#define FASTSYSTEMEXPRESSION_H

#include <VCELL/FastSystem.h>

using std::string;

class SimulationExpression;
class SimpleSymbolTable;
class SimTool;
namespace VCell {
	class Expression;
}

//-----------------------------------------------------------------
//
//  class FastSystem
//
//-----------------------------------------------------------------
class FastSystemExpression : public FastSystem {
public:
    FastSystemExpression(int dimension, int numDepend, SimulationExpression* sim);
	~FastSystemExpression() override;

    void resolveReferences(SimulationExpression *sim) override;
	void updateDependentVars() override;

	void setPseudoConstants(string* symbols, VCell::Expression** expressions);
    void setFastRateExpressions(VCell::Expression** expressions);
	void setFastDependencyExpressions(string* symbols, VCell::Expression** expressions);
	void setJacobianExpressions(VCell::Expression** expressions);
	void setCoordinates(double time_sec, WorldCoord& wc) override;

    void initVars(SimTool *sim_tool) override;
	void setDependentVariables(string* vars) const; // must be called before other setters
	void setIndependentVariables(string* vars) const; // must be called before other setters
	void updateIndepValues() const;
	void updateMatrix() override;

private:
	string* pseudoSymbols;
	SimulationExpression* simulation;
	double* pseudoConstants;
	SimpleSymbolTable* fastSymbolTable; // include independent, pseudo, field data, random variables;
	double* fastValues;	
	VCell::Expression** pseudoConstantExpressions;
	VCell::Expression** fastRateExpressions;
	VCell::Expression** fastDependencyExpressions;
	VCell::Expression** jacobianExpressions;

	void bindAllExpressions();
	SimpleSymbolTable* getFastSymbolTable();
};

#endif
