#ifndef FVSOLVER_H
#define FVSOLVER_H

#include <string>

#include "VCellModel.h"
using std::string;
using std::istream;

class SimTool;
class VCellModel;
namespace VCell {
	class Expression;
}
class VarContext;
class CartesianMesh;
class Variable;
class Feature;
class FastSystemExpression;
class SimulationExpression;
class Structure;
class Membrane;

class FVSolver {
public:
	FVSolver(const char* outdir=nullptr);
	virtual ~FVSolver();

	SimTool* createSimTool(istream& ifsInput, istream& vcgInput, int taskID, bool bSimZip=true);
	static void solve(SimTool* sim_tool, bool bLoadFinal=true, double* paramValues=nullptr);

	static void init(SimTool* sim_tool, double* paramValues=nullptr);
	static void step(SimTool* sim_tool, double* paramValues=nullptr);
	
	static double getCurrentTime(SimTool* sim_tool);
	static void setEndTime(SimTool* sim_tool, double endTime);

	static int getNumVariables(const SimTool* sim_tool);
	static string getVariableName(const SimTool* sim_tool, int index);
	static int getVariableLength(const SimTool* sim_tool, string& varName);
	static double* getValue(const SimTool* sim_tool, string& varName, int arrayID);  // arrayID=0 for "old" and 1 for "current"
	static void setInitialCondition(SimTool* sim_tool, string& varName, int dataLength, const double* data);

private:
	static void loadJMSInfo(istream& ifsInput, int taskID);
	static VCellModel* loadModel(istream& ifsInput);
	SimulationExpression* loadSimulation(SimTool* sim_tool, VCellModel* model, istream& ifsInput);
	static void loadSmoldyn(SimTool* sim_tool, istream& ifsInput);
	static VCell::Expression* readExpression(istream& ifsInput, string& var_name, const string& prefix="");
	static VarContext* loadEquation(istream& ifsInput, Structure* structure, Variable* var);
	static void loadJumpCondition(SimulationExpression* simulation, VCellModel* model, istream& ifsInput, Membrane*, string& var_name);
	static void loadPseudoConstants(istream& ifsInput, FastSystemExpression* fastSystem);
	static void loadFastRates(istream& ifsInput, FastSystemExpression* fastSystem);
	static void loadFastDependencies(istream& ifsInput, FastSystemExpression* fastSystem);
	static void loadJacobians(istream& ifsInput, FastSystemExpression* fastSystem);
	static void loadFastSystem(istream& ifsInput, FastSystemExpression* fastSystem);
	static void loadFeature(SimulationExpression* simulation, istream& ifsInput, Feature* feature);
	static void loadMembrane(SimulationExpression* simulation, VCellModel* model, istream& ifsInput, Membrane*);
	void loadSimulationParameters(SimTool* sim_tool, istream& ifsInput) const;
	void loadMeshFromVcg(VCellModel* model, istream& vcgInput);
	static void loadFieldData(SimulationExpression* simulation, istream& ifsInput);
	static void loadParameters(SimulationExpression* simulation, istream& ifsInput, int numParams);
	static void loadSerialScanParameters(SimulationExpression* simulation, istream& ifsInput, int numSerialScanParameters);
	static void loadSerialScanParameterValues(SimTool* simTool, SimulationExpression* simulation, istream& ifsInput, int numSerialScanParamValues);

	const char* outputPath;
	CartesianMesh *mesh;
	
	int loadSolveRegions(VCellModel* model, istream& instream, int*& solveRegions) const;

	static void loadPostProcessingBlock(SimTool* sim_tool, istream& ifsInput);
};

#endif
