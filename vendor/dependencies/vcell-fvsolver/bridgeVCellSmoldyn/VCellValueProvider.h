#ifndef VCELL_VALUE_PROVIDER_H
#define VCELL_VALUE_PROVIDER_H

#include <smoldyn.h>
#include <Expression.h>
#include <VCELL/SimTool.h>

class VCellValueProvider : public ValueProvider {
public:
	VCellValueProvider(simptr sim, SimTool* simTool,string& rateExp);
	double getConstantValue();
	double getValue(double t, double x, double y, double z, rxnptr rxn);
	double getValue(double t, double x, double y, double z, rxnptr rxn, char* panelName);
	double getValue(double t, double x, double y, double z, surfactionptr actiondetails, char* panelName);
	void bindExpression(SymbolTable* symbolTable);
private:
	VCell::Expression* rateExp;
	SimTool* simTool;
	simptr sim;
};

class VCellValueProviderFactory : public ValueProviderFactory{
private:
	SimTool* simTool;
	simptr sim;
public:
	VCellValueProviderFactory(SimTool* simTool);
	ValueProvider* createValueProvider(string& rateExpStr);
};
#endif