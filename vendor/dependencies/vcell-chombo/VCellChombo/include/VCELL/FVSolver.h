#ifndef FVSOLVER_H
#define FVSOLVER_H

#include <string>
using std::string;
using std::istream;

class SimTool;
class VCellModel;
namespace VCell {
	class Expression;
}
class VarContext;
class ChomboSpec;
class ChomboScheduler;
class Variable;
class Feature;
class FastSystemExpression;
class SimulationExpression;
class Structure;
class Membrane;
class ChomboIF;

class FVSolver {
public:
	FVSolver(istream& fvinput, int taskID=-1);
	void createSimTool(istream& ifsInput, int taskID);
	void solve(bool convertChomboData);

	double getCurrentTime();
	void setEndTime(double endTime);

	int getNumVariables();
	string getVariableName(int index);
	int getVariableLength(string& varName);
	double* getValue(string& varName, int arrayID);  // arrayID=0 for "old" and 1 for "current"
	void setInitialCondition(string& varName, int dataLength, const double* data);

private:
	void loadJMSInfo(istream& ifsInput, int taskID);
	void loadModel(istream& ifsInput);
	void loadSimulation(istream& ifsInput);
	VCell::Expression* readExpression(istream& ifsInput, string& var_name, string prefix="");
	void loadVarContext(istream& ifsInput, Structure* structure, Variable* var);
	void loadJumpCondition(istream& ifsInput, Membrane*, string& var_name);
	void loadPseudoConstants(istream& ifsInput, FastSystemExpression* fastSystem);
	void loadFastRates(istream& ifsInput, FastSystemExpression* fastSystem);
	void loadFastDependencies(istream& ifsInput, FastSystemExpression* fastSystem);
	void loadJacobians(istream& ifsInput, FastSystemExpression* fastSystem);
	void loadFastSystem(istream& ifsInput, FastSystemExpression* fastSystem);
	void loadFeature(istream& ifsInput, Feature* feature);
	void loadMembrane(istream& ifsInput, Membrane*);
	void loadSimulationParameters(istream& ifsInput);
	void loadFieldData(istream& ifsInput);
	void loadParameters(istream& ifsInput, int numParams);
	void loadSerialScanParameters(istream& ifsInput, int numSerialScanParameters);
	void loadSerialScanParameterValues(istream& ifsInput, int numSerialScanParamValues);
	void loadChomboSpec(istream& ifsInput);

	void loadPostProcessingBlock(istream& ifsInput);

	SimTool* simTool;
};

#endif
