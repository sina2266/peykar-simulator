#include <VCELL/ChomboIF.h>
#include <Exception.h>
#include <Expression.h>

#include <sys/stat.h>
#include <VCELL/FVSolver.h>
#include <VCELL/VolumeVariable.h>
#include <VCELL/VolumeRegionVariable.h>
#include <VCELL/MembraneVariable.h>
#include <VCELL/MembraneRegionVariable.h>
#include <VCELL/VCellModel.h>
#include <VCELL/Feature.h>
#include <VCELL/Membrane.h>
#include <VCELL/SimTool.h>
#include <VCELL/FastSystemExpression.h>
#include <VCELL/VolumeVarContextExpression.h>
#include <VCELL/VolumeRegionVarContextExpression.h>
#include <VCELL/MembraneVarContextExpression.h>
#include <VCELL/MembraneRegionVarContextExpression.h>
#include <VCELL/SimulationMessaging.h>
#include <VCELL/SimulationExpression.h>
#include <VCELL/ChomboSemiImplicitScheduler.h>
#include <VCELL/ChomboSpec.h>
#include <VCELL/PostProcessingBlock.h>
#include <VCELL/VariableStatisticsDataGenerator.h>

#include <assert.h>
#include <sstream>
#include <fstream>
#include <string>
using std::ifstream;

#ifdef CH_MPI
extern bool bConsoleOutput;
#endif

/*
# JMS_Paramters
JMS_PARAM_BEGIN
JMS_BROKER tcp://code:2507
JMS_USER serverUser cbittech
JMS_QUEUE workerEventDev
JMS_TOPIC serviceControlDev
VCELL_USER fgao
SIMULATION_KEY 36230826
JOB_INDEX 0
JMS_PARAM_END
*/
void FVSolver::loadJMSInfo(istream& ifsInput, int taskID) {
	// JMS_USER, JMS_QUEUE and JMS_TOPIC are still parsed so the input file
	// format is unchanged, but vcell-messaging talks to the broker over the JMS
	// REST bridge now and derives the destinations itself, so they go unused.
	string broker, smqusername, password, qname, tname, vcusername;
	string nextToken, line;
	int simKey = 0, jobIndex = 0;

	while (!ifsInput.eof()) {
		getline(ifsInput, line);
		istringstream lineInput(line);

		nextToken = "";
		lineInput >> nextToken;
		if (nextToken.size() == 0 || nextToken[0] == '#') {
			continue;
		}
		if (nextToken == "JMS_PARAM_END") {
			break;
		}

		if (nextToken == "JMS_BROKER") {
			lineInput >> broker;
		} else if (nextToken == "JMS_USER") {
			lineInput >> smqusername >> password;
		} else if (nextToken == "JMS_QUEUE") {
			lineInput >> qname;
		} else if (nextToken == "JMS_TOPIC") {
			lineInput >> tname;
		} else if (nextToken == "VCELL_USER") {
			lineInput >> vcusername;
		} else if (nextToken == "SIMULATION_KEY") {
			lineInput >> simKey;
		} else if (nextToken == "JOB_INDEX") {
			lineInput >> jobIndex;
		}
	}

#ifdef USE_MESSAGING
	if (taskID >= 0) {
		SimulationMessaging::getInstVar()->initialize_curl_messaging(
				false, broker.c_str(), vcusername.c_str(), simKey, jobIndex, taskID);
	}
#endif
}

/*
# Model description: FEATURE name handle boundary_conditions
MODEL_BEGIN
FEATURE cyt 0 value value value value 
FEATURE ec 1 value value value value 
MEMBRANE cyt_ec_membrane cyt ec value value value value 
MODEL_END
*/
void FVSolver::loadModel(istream& ifsInput) {
	//cout << "loading model " << endl;
	string nextToken, line;
	string feature_name;
	int handle;
	int numFeatures = 0, numMembranes = 0;

	while (!ifsInput.eof()) {
		getline(ifsInput, line);
		istringstream lineInput(line);

		nextToken = "";
		lineInput >> nextToken;
		if (nextToken.size() == 0 || nextToken[0] == '#') {
			continue;
		} 
		if (nextToken == "MODEL_END") {
			break;
		} 
		
		Structure* structure = 0;
		if (nextToken == "FEATURE") {
			numFeatures ++;
			lineInput >> feature_name >> handle;
			Feature* feature = simTool->getModel()->addFeature(feature_name);
			structure = feature;
		} else if (nextToken == "MEMBRANE") {
			numMembranes ++;
			string membrane_name, feature1_name, feature2_name;
			lineInput >> membrane_name >> feature1_name >> feature2_name;
			Membrane* membrane = simTool->getModel()->addMembrane(membrane_name, feature1_name, feature2_name);
			structure = membrane;
		}

		string btstr;
		for (int i = 0; i < 6; i ++) {
			if (lineInput.eof()) {
				break;
			}
			lineInput >> btstr;
			if (btstr.length() == 0) {
				break;
			}
			BoundaryType bt = BOUNDARY_VALUE;
			if (btstr == "flux") {
				bt = BOUNDARY_FLUX;
			} else if (btstr == "value") {
				bt = BOUNDARY_VALUE;
			} else if (btstr == "periodic") {
				bt = BOUNDARY_PERIODIC;
			} else {
				stringstream ss1;
				ss1 << "loadModel(), wrong boundary type " << btstr;
				throw ss1.str();
			}
			switch (i) {
				case 0: // XM
					structure->setXmBoundaryType(bt);
					break;
				case 1: // XP
					structure->setXpBoundaryType(bt);
					break;
				case 2: // YM
					structure->setYmBoundaryType(bt);
					break;
				case 3: // YP
					structure->setYpBoundaryType(bt);
					break;
				case 4: // ZM
					structure->setZmBoundaryType(bt);
					break;
				case 5: // ZP
					structure->setZpBoundaryType(bt);
					break;
			}
		}
	}
}

static void trimString(string& str)
{
	string::size_type pos = str.find_last_not_of(" \r\n");
	if(pos != string::npos) {
		str.erase(pos + 1);
		pos = str.find_first_not_of(" \r\n");
		if(pos != string::npos) {
			str.erase(0, pos);
		}
	}
	else {
		str.erase(str.begin(), str.end());
	}
}

/*
# Variables : type name time_dependent_flag advection_flag solve_whole_mesh_flag solve_regions
VARIABLE_BEGIN
MEMBRANE_ODE h_c
VOLUME_PDE K false false true
MEMBRANE_ODE m_o
MEMBRANE_ODE n_o
VOLUME_PDE Na false false true
MEMBRANE_REGION Voltage_membrane
VARIABLE_END
*/
void FVSolver::loadSimulation(istream& ifsInput) {
	//cout << "loading simTool->getSimulation()" << endl;
	// process geometry
	ChomboSemiImplicitScheduler* chomboScheduler = new ChomboSemiImplicitScheduler(simTool->getSimulation(), simTool->getChomboSpec());
	chomboScheduler->initializeGrids();
	simTool->getSimulation()->setScheduler(chomboScheduler);
	
	int numMembranePoints = chomboScheduler->getNumMembranePoints();
	string nextToken, line;
	IntVect viewLevelMeshSize = chomboScheduler->getViewLevelMeshSize();
	long sizeX = viewLevelMeshSize[0];
	long sizeY = viewLevelMeshSize[1];
	long sizeZ = chomboScheduler->getChomboGeometry()->getDimension() == 3 ? viewLevelMeshSize[2] : 1;
	string variable_name, variable_domain;

	while (!ifsInput.eof()) {
		getline(ifsInput, line);
		istringstream lineInput(line);

		nextToken = "";
		lineInput >> nextToken;
		if (nextToken.size() == 0 || nextToken[0] == '#') {
			continue;
		}
		if (nextToken == "VARIABLE_END") {
			break;
		}

		if (nextToken == "VOLUME_RANDOM")
		{
			throw "VolumeRandomVariable not supported yet";
		} 
		else if (nextToken == "MEMBRANE_RANDOM")
		{
			throw "MembraneRandomVariable not supported yet";
		}
		else if (nextToken == "VOLUME_PDE" || nextToken == "VOLUME_PDE_STEADY")
		{
			string advectionflag, time_dependent_diffusion_flag, grad_flag;
			lineInput >> variable_name >> variable_domain >> time_dependent_diffusion_flag >> advectionflag >> grad_flag;

			Feature* feature = simTool->getModel()->getFeatureFromName(variable_domain);
			if (feature == NULL) {
				stringstream ss;
				ss << "volume variable " << variable_name << " is not defined in any feature.";
				throw ss.str();
			}
			if (time_dependent_diffusion_flag == "true") {
				throw "time dependent diffusion not supported yet";
				simTool->getSimulation()->setHasTimeDependentDiffusionAdvection();
			}
			bool bConvection = false;
			if (advectionflag == "true" ) {
				throw "advection not supported yet";
				bConvection = true;
			}
			if (grad_flag == "true")
			{
				throw "gradient not supported yet";
			}
			VolumeVariable* volumeVar = new VolumeVariable(variable_name, feature, sizeX*sizeY*sizeZ, numMembranePoints);
			volumeVar->setDiffusing();
			if (nextToken == "VOLUME_PDE_STEADY")
			{
				volumeVar->setElliptic();
				simTool->getSimulation()->setHasElliptic();
			}
			else
			{
				simTool->getSimulation()->setHasParabolic();
			}
			feature->addDefinedVariable(volumeVar);
			simTool->getSimulation()->addVariable(volumeVar);
		} else if (nextToken == "VOLUME_ODE") {
			lineInput >> variable_name >> variable_domain;
			Feature* feature = simTool->getModel()->getFeatureFromName(variable_domain);
			if (feature == NULL) {
				stringstream ss;
				ss << "volume variable " << variable_name << " is not defined in any feature.";
				throw ss.str();
			}
			VolumeVariable* volumeVar = new VolumeVariable(variable_name, feature, sizeX*sizeY*sizeZ, numMembranePoints);
			feature->addDefinedVariable(volumeVar);
			simTool->getSimulation()->addVariable(volumeVar);
		} 
		else if (nextToken == "MEMBRANE_ODE")
		{
			lineInput >> variable_name >> variable_domain;
			Membrane* membrane = simTool->getModel()->getMembraneFromName(variable_domain);
			MembraneVariable* membraneVar = new MembraneVariable(variable_name, membrane, numMembranePoints);
			if (membrane == NULL) {
				stringstream ss;
				ss << "membrane variable " << variable_name << " is not defined on any membrane.";
				throw ss.str();
			}
			membrane->addDefinedVariable(membraneVar);
			simTool->getSimulation()->addVariable(membraneVar);
		} else if (nextToken == "MEMBRANE_PDE") {
			throw "Membrane diffusion not supported yet";
		} else if (nextToken == "VOLUME_REGION") {
			throw "VolumeRegionVariable not supported yet";
		} else if (nextToken == "MEMBRANE_REGION") {
			throw "MembraneRegionVariable not supported yet";
		}
	}
}

VCell::Expression* FVSolver::readExpression(istream& lineInput, string& var_name, string prefix) {
	string expStr;
	getline(lineInput, expStr);
	expStr = prefix + expStr;
	trimString(expStr);
	if (expStr[expStr.size()-1] != ';') {
		stringstream msg;
		msg << "Expression for [" << var_name << "] is not terminated by ';'";
		throw msg.str();
	}
	return new VCell::Expression(expStr);
}

/*
EQUATION_BEGIN U
INITIAL (sin((3.14159265358979 * x)) * sin((3.14159265358979 * y)) * exp( - (19.739208802178677 * t)));
RATE 0.0;
DIFFUSION 1.0;
VELOCITY_X 0.0;
VELOCITY_Y 0.0;
BOUNDARY_XM (sin((3.14159265358979 * x)) * sin((3.14159265358979 * y)) * exp( - (19.739208802178677 * t)));
BOUNDARY_XP (sin((3.14159265358979 * x)) * sin((3.14159265358979 * y)) * exp( - (19.739208802178677 * t)));
BOUNDARY_YM (sin((3.14159265358979 * x)) * sin((3.14159265358979 * y)) * exp( - (19.739208802178677 * t)));
BOUNDARY_YP (sin((3.14159265358979 * x)) * sin((3.14159265358979 * y)) * exp( - (19.739208802178677 * t)));
EQUATION_END
*/
void FVSolver::loadVarContext(istream& ifsInput, Structure* structure, Variable* var) {

	//cout << "loading volume var context " << var_name << endl;
	if (var->getStructure() != structure)
	{
		// now Variable only has one VarContext that makes sense. Others are dummy.
		return;
	}

	VarContext* varContext = NULL;
	if (var->getVarType() == VAR_VOLUME) {
		varContext = new VolumeVarContextExpression((Feature*)structure, (VolumeVariable*)var);
	} else if (var->getVarType() == VAR_VOLUME_REGION) {
		varContext = new VolumeRegionVarContextExpression((Feature*)structure, (VolumeRegionVariable*)var);
	} else if (var->getVarType() == VAR_MEMBRANE) {
		varContext = new MembraneVarContextExpression((Membrane*)structure, (MembraneVariable*)var);
	} else if (var->getVarType() == VAR_MEMBRANE_REGION) {
		varContext = new MembraneRegionVarContextExpression((Membrane*)structure, (MembraneRegionVariable*)var);
	} else {
		stringstream ss;
		ss << "loadEquation: variable type '" << var->getVarType() << "' is not supported yet for variable '" << var->getName() << "'";
		throw ss.str();
	}
	var->setVarContext(varContext);

	string nextToken, line;
	while (!ifsInput.eof()) {
		getline(ifsInput, line);
		istringstream lineInput(line);

		nextToken = "";
		lineInput >> nextToken;
		if (nextToken.size() == 0 || nextToken[0] == '#') {
			continue;
		}
		if (nextToken == "EQUATION_END") {
			break;
		}

		string var_name = var->getName();
		VCell::Expression* exp = readExpression(lineInput, var_name);
		ExpressionIndex expIndex;
		if (nextToken == "INITIAL") {
			expIndex = INITIAL_VALUE_EXP;
		} else if (nextToken == "EXACT") {
			expIndex = EXACT_EXP;
		} else if (nextToken == "DIFFUSION") {
			expIndex = DIFF_RATE_EXP;
		} else if (nextToken == "RATE") {
			expIndex = REACT_RATE_EXP;
		} else if (nextToken == "UNIFORMRATE") {
			expIndex = UNIFORM_RATE_EXP;
		} else if (nextToken == "BOUNDARY_XM") {
			expIndex = BOUNDARY_XM_EXP;
		} else if (nextToken == "BOUNDARY_XP") {
			expIndex = BOUNDARY_XP_EXP;
		} else if (nextToken == "BOUNDARY_YM") {
			expIndex = BOUNDARY_YM_EXP;
		} else if (nextToken == "BOUNDARY_YP") {
			expIndex = BOUNDARY_YP_EXP;
		} else if (nextToken == "BOUNDARY_ZM") {
			expIndex = BOUNDARY_ZM_EXP;
		} else if (nextToken == "BOUNDARY_ZP") {
			expIndex = BOUNDARY_ZP_EXP;
		} else if (nextToken == "VELOCITY_X") {
			expIndex = VELOCITY_X_EXP;
		} else if (nextToken == "VELOCITY_Y") {
			expIndex = VELOCITY_Y_EXP;
		} else if (nextToken == "VELOCITY_Z") {
			expIndex = VELOCITY_Z_EXP;
		} else {
			stringstream ss;
			ss << "FVSolver::loadEquation(), unexpected token " << nextToken;
			throw ss.str();
		}
		varContext->setExpression(exp, expIndex);
	}
}

/*
JUMP_CONDITION_BEGIN Ca
FLUX Nucleus (150.0 * (Ca_Cytosol_membrane - Ca_Nucleus_membrane));
FLUX Cytosol  - (150.0 * (Ca_Cytosol_membrane - Ca_Nucleus_membrane));
JUMP_CONDITION_END
*/
void FVSolver::loadJumpCondition(istream& ifsInput, Membrane* membrane, string& var_name) {
	//cout << "loading jump condition " << var_name << endl;
	string nextToken, line;

	Variable* var = simTool->getSimulation()->getVariableFromName(var_name);
	while (!ifsInput.eof()) {
		getline(ifsInput, line);
		istringstream lineInput(line);

		nextToken = "";
		lineInput >> nextToken;
		if (nextToken.size() == 0 || nextToken[0] == '#') {
			continue;
		}  
		if (nextToken == "JUMP_CONDITION_END") {
			break;
		}
		if (nextToken == "FLUX" || nextToken == "VALUE") {
			string featurename;
			lineInput >> featurename;
			assert(!featurename.empty());
			Feature* f = simTool->getModel()->getFeatureFromName(featurename);
			f->setEbBcType(membrane, nextToken == "FLUX" ? BOUNDARY_FLUX : BOUNDARY_VALUE);
			string var_name = var->getName();
			VCell::Expression* exp = readExpression(lineInput, var_name);
			VarContext* varContext = var->getVarContext();
			if (var->getVarType() == VAR_VOLUME || var->getVarType() == VAR_VOLUME_REGION)
			{
				varContext->addJumpCondition(membrane, exp);
			} 
			else
			{
				throw "Only volume variables and volume region variables have jump conditions";
			}
		} 
		else
		{
			throw "Expecting FLUX or VALUE in JumpCondition.";
		}
	}
}

/*
PSEUDO_CONSTANT_BEGIN
__C0 (CaBPB + CaBP);
__C1 ( - CaBP + CaB + Ca);
PSEUDO_CONSTANT_END
*/
void FVSolver::loadPseudoConstants(istream& ifsInput, FastSystemExpression* fastSystem) {
	//cout << "loading pseudo constants for fast system" << endl;
	string nextToken, line;
	int count = 0;
	int numDep = fastSystem->getNumDependents();
	string* vars = new string[numDep];
	VCell::Expression **expressions = new VCell::Expression*[numDep];

	while (!ifsInput.eof()) {
		getline(ifsInput, line);
		istringstream lineInput(line);

		nextToken = "";
		lineInput >> nextToken;
		if (nextToken.size() == 0 || nextToken[0] == '#') {
			continue;
		}
		if (nextToken == "PSEUDO_CONSTANT_END") {
			break;
		}
		vars[count] = nextToken;
		expressions[count] = readExpression(lineInput, vars[count]);
		count ++;
	}

	fastSystem->setPseudoConstants(vars, expressions);
	if (count != numDep) {
		throw "In the fast system the number of pseudo constants should be the same as that of dependent variables";
	}
}

/*
FAST_RATE_BEGIN
( - ((0.1 * (400.0 - (__C1 + __C0 - Ca - CaBPB)) * Ca) - (__C1 + __C0 - Ca - CaBPB)) - ((20.0 * Ca * ( - CaBPB + __C0)) - (8.6 * CaBPB)));
 - ((20.0 * Ca * ( - CaBPB + __C0)) - (8.6 * CaBPB));
FAST_RATE_END
*/
void FVSolver::loadFastRates(istream& ifsInput, FastSystemExpression* fastSystem) {
	//cout << "loading fast rates for fast system" << endl;
	string nextToken, line;
	int count = 0;
	int numIndep = fastSystem->getDimension();
	VCell::Expression **expressions = new VCell::Expression*[numIndep];

	while (!ifsInput.eof()) {
		getline(ifsInput, line);
		istringstream lineInput(line);

		nextToken = "";
		lineInput >> nextToken;
		if (nextToken.size() == 0 || nextToken[0] == '#') {
			continue;
		}
		if (nextToken == "FAST_RATE_END") {
			break;
		}

		string varname("fastRate");
		expressions[count] = readExpression(lineInput, varname, nextToken);
		count ++;
	}
	if (count != numIndep) {
		throw "In the fast system the number of fast rates should be the same as that of independent variables";
	}
	fastSystem->setFastRateExpressions(expressions);
}

/*
FAST_DEPENDENCY_BEGIN
CaB (__C1 + __C0 - Ca - CaBPB);
CaBP ( - CaBPB + __C0);
FAST_DEPENDENCY_END
*/
void FVSolver::loadFastDependencies(istream& ifsInput, FastSystemExpression* fastSystem) {
	//cout << "loading fast dependencies for fast system" << endl;
	string nextToken, line;
	int count = 0;
	int numDep = fastSystem->getNumDependents();
	string* vars = new string[numDep];
	VCell::Expression **expressions = new VCell::Expression*[numDep];

	while (!ifsInput.eof()) {
		getline(ifsInput, line);
		istringstream lineInput(line);

		nextToken = "";
		lineInput >> nextToken;
		if (nextToken.size() == 0 || nextToken[0] == '#') {
			continue;
		}
		if (nextToken == "FAST_DEPENDENCY_END") {
			break;
		}

		vars[count] = nextToken;
		expressions[count] = readExpression(lineInput, vars[count]);
		count ++;
	}
	fastSystem->setFastDependencyExpressions(vars, expressions);
	delete[] vars;
	if (count != numDep) {
		throw "In the fast system the number of fast dependencies should be the same as that of dependent variables";
	}
}

/*
JACOBIAN_BEGIN
( - (1.0 + (0.1 * Ca) + (0.1 * (400.0 - (__C1 + __C0 - Ca - CaBPB)))) - (20.0 * ( - CaBPB + __C0)));
( - (1.0 + (0.1 * Ca)) - (-8.6 - (20.0 * Ca)));
 - (20.0 * ( - CaBPB + __C0));
 - (-8.6 - (20.0 * Ca));
JACOBIAN_END
*/
void FVSolver::loadJacobians(istream& ifsInput, FastSystemExpression* fastSystem) {
	//cout << "loading jacobians for fast system" << endl;
	string nextToken, line;
	int count = 0;
	int numIndep = fastSystem->getDimension();
	VCell::Expression **expressions = new VCell::Expression*[numIndep * numIndep];

	while (!ifsInput.eof()) {
		getline(ifsInput, line);
		istringstream lineInput(line);

		nextToken = "";
		lineInput >> nextToken;
		if (nextToken.size() == 0 || nextToken[0] == '#') {
			continue;
		}
		if (nextToken == "JACOBIAN_END") {
			break;
		}

		string varname("jacobian");
		expressions[count] = readExpression(lineInput, varname, nextToken);
		count ++;
	}
	if (count != numIndep * numIndep) {
		throw "In the fast system the number of Jacobian should dim*dim";
	}
	fastSystem->setJacobianExpressions(expressions);
}

/*
# fast system dimension num_dependents
FAST_SYSTEM_BEGIN 2 2
INDEPENDENT_VARIALBES Ca CaBPB
DEPENDENT_VARIALBES CaB CaBP

PSEUDO_CONSTANT_BEGIN
__C0 (CaBPB + CaBP);
__C1 ( - CaBP + CaB + Ca);
PSEUDO_CONSTANT_END

FAST_RATE_BEGIN
( - ((0.1 * (400.0 - (__C1 + __C0 - Ca - CaBPB)) * Ca) - (__C1 + __C0 - Ca - CaBPB)) - ((20.0 * Ca * ( - CaBPB + __C0)) - (8.6 * CaBPB)));
 - ((20.0 * Ca * ( - CaBPB + __C0)) - (8.6 * CaBPB));
FAST_RATE_END

FAST_DEPENDENCY_BEGIN
CaB (__C1 + __C0 - Ca - CaBPB);
CaBP ( - CaBPB + __C0);
FAST_DEPENDENCY_END

JACOBIAN_BEGIN
( - (1.0 + (0.1 * Ca) + (0.1 * (400.0 - (__C1 + __C0 - Ca - CaBPB)))) - (20.0 * ( - CaBPB + __C0)));
( - (1.0 + (0.1 * Ca)) - (-8.6 - (20.0 * Ca)));
 - (20.0 * ( - CaBPB + __C0));
 - (-8.6 - (20.0 * Ca));
JACOBIAN_END

FAST_SYSTEM_END
*/
void FVSolver::loadFastSystem(istream& ifsInput, FastSystemExpression* fastSystem) {
	//cout << "loading fast system for " << feature->getName() << endl;
	string nextToken, line;
	int numIndep = fastSystem->getDimension();
	int numDep = fastSystem->getNumDependents();

	while (!ifsInput.eof()) {
		getline(ifsInput, line);
		istringstream lineInput(line);

		nextToken = "";
		lineInput >> nextToken;
		if (nextToken.size() == 0 || nextToken[0] == '#') {
			continue;
		}
		if (nextToken == "FAST_SYSTEM_END") {
			break;
		}

		if (nextToken == "DEPENDENT_VARIALBES") {
			string* vars = new string[numDep];
			for (int i = 0; i < numDep; i ++) {
				lineInput >> vars[i];
			}
			fastSystem->setDependentVariables(vars);
			delete[] vars;
		} else if (nextToken == "INDEPENDENT_VARIALBES") {
			string* vars = new string[numIndep];
			for (int i = 0; i < numIndep; i ++) {
				lineInput >> vars[i];
			}
			fastSystem->setIndependentVariables(vars);
			delete[] vars;
		} else if (nextToken == "PSEUDO_CONSTANT_BEGIN") {
			loadPseudoConstants(ifsInput, fastSystem);
		} else if (nextToken == "FAST_RATE_BEGIN") {
			loadFastRates(ifsInput, fastSystem);
		} else if (nextToken == "FAST_DEPENDENCY_BEGIN") {
			loadFastDependencies(ifsInput, fastSystem);
		} else if (nextToken == "JACOBIAN_BEGIN") {
			loadJacobians(ifsInput, fastSystem);
		}
	}
}

/*
COMPARTMENT_BEGIN cyt

BOUNDARY_CONDITIONS value value value value 

EQUATION_BEGIN U
INITIAL (sin((3.14159265358979 * x)) * sin((3.14159265358979 * y)) * exp( - (19.739208802178677 * t)));
RATE 0.0;
DIFFUSION 1.0;
VELOCITY_X 0.0;
VELOCITY_Y 0.0;
BOUNDARY_XM (sin((3.14159265358979 * x)) * sin((3.14159265358979 * y)) * exp( - (19.739208802178677 * t)));
BOUNDARY_XP (sin((3.14159265358979 * x)) * sin((3.14159265358979 * y)) * exp( - (19.739208802178677 * t)));
BOUNDARY_YM (sin((3.14159265358979 * x)) * sin((3.14159265358979 * y)) * exp( - (19.739208802178677 * t)));
BOUNDARY_YP (sin((3.14159265358979 * x)) * sin((3.14159265358979 * y)) * exp( - (19.739208802178677 * t)));
EQUATION_END

COMPARTMENT_END

COMPARTMENT_BEGIN ec

BOUNDARY_CONDITIONS value value value value 

EQUATION_BEGIN U
INITIAL (sin((3.14159265358979 * x)) * sin((3.14159265358979 * y)) * exp( - (19.739208802178677 * t)));
RATE 0.0;
DIFFUSION 0.0;
VELOCITY_X 0.0;
VELOCITY_Y 0.0;
BOUNDARY_XM (sin((3.14159265358979 * x)) * sin((3.14159265358979 * y)) * exp( - (19.739208802178677 * t)));
BOUNDARY_XP (sin((3.14159265358979 * x)) * sin((3.14159265358979 * y)) * exp( - (19.739208802178677 * t)));
BOUNDARY_YM (sin((3.14159265358979 * x)) * sin((3.14159265358979 * y)) * exp( - (19.739208802178677 * t)));
BOUNDARY_YP (sin((3.14159265358979 * x)) * sin((3.14159265358979 * y)) * exp( - (19.739208802178677 * t)));
EQUATION_END

COMPARTMENT_END
*/
void FVSolver::loadFeature(istream& ifsInput, Feature* feature) {
	//cout << "loading feature " << feature->getName() << endl;
	string nextToken, line;

	while (!ifsInput.eof()) {
		getline(ifsInput, line);
		istringstream lineInput(line);

		nextToken = "";
		lineInput >> nextToken;
		if (nextToken.size() == 0 || nextToken[0] == '#') {
			continue;
		}
		if (nextToken == "COMPARTMENT_END") {
			break;
		}

		if (nextToken == "EQUATION_BEGIN") {
			char var_name[256];
			lineInput >> var_name;
			Variable* var = simTool->getSimulation()->getVariableFromName(var_name);
			if (var->getStructure() == feature)
			{
				loadVarContext(ifsInput, feature, var);
			}
		} else if (nextToken == "FAST_SYSTEM_BEGIN") {
			simTool->getSimulation()->setHasFastSystem();
			int dimension, num_of_dependents;
			lineInput >> dimension >> num_of_dependents;
			FastSystemExpression* fastSystem = new FastSystemExpression(feature, dimension, num_of_dependents, simTool->getSimulation());
			loadFastSystem(ifsInput, fastSystem);
			feature->setFastSystem(fastSystem);
		}
	}
}

/*
MEMBRANE_BEGIN subVolume0_subVolume1_membrane subVolume0 subVolume1

BOUNDARY_CONDITIONS flux value flux value 

EQUATION_BEGIN n_o
INITIAL 0.304015731;
RATE (1000.0 * ((0.01 * (1.0 - n_o) * (10.0 - (62.0 + Voltage_membrane)) / (-1.0 + exp((0.1 * (10.0 - (62.0 + Voltage_membrane)))))) - (0.125 * n_o * exp( - (0.0125 * (62.0 + Voltage_membrane))))));
EQUATION_END

EQUATION_BEGIN m_o
INITIAL 0.04759071;
RATE (1000.0 * ((0.1 * (1.0 - m_o) * (25.0 - (62.0 + Voltage_membrane)) / (-1.0 + exp((0.1 * (25.0 - (62.0 + Voltage_membrane)))))) - (4.0 * m_o * exp( - (0.05555555555555555 * (62.0 + Voltage_membrane))))));
EQUATION_END

EQUATION_BEGIN h_c
INITIAL 0.372877409;
RATE  - (1000.0 * ((0.07 * h_c * exp( - (0.05 * (62.0 + Voltage_membrane)))) - ((1.0 - h_c) / (1.0 + exp((0.1 * (30.0 - (62.0 + Voltage_membrane))))))));
EQUATION_END

EQUATION_BEGIN Voltage_membrane
INITIAL -62.897633102;
RATE (100000.0 * ((0.1 * (t < 0.05)) - ((0.0030 * (51.4 + Voltage_membrane)) - (1.2 * ((25.851990049751244 * log((Na_subVolume1_membrane / Na_subVolume0_membrane))) - Voltage_membrane) * pow(m_o,3.0) * (1.0 - h_c)) - (0.36 * ((25.851990049751244 * log((K_subVolume1_membrane / K_subVolume0_membrane))) - Voltage_membrane) * pow(n_o,4.0)))));
UNIFORMRATE 0.0;
EQUATION_END

JUMP_CONDITION_BEGIN K
FLUX subVolume0 (3731.3432835820895 * ((25.851990049751244 * log((K_subVolume1_membrane / K_subVolume0_membrane))) - Voltage_membrane) * pow(n_o,4.0));
FLUX subVolume1  - (3731.3432835820895 * ((25.851990049751244 * log((K_subVolume1_membrane / K_subVolume0_membrane))) - Voltage_membrane) * pow(n_o,4.0));
JUMP_CONDITION_END

JUMP_CONDITION_BEGIN Na
FLUX subVolume0 (12437.810945273632 * ((25.851990049751244 * log((Na_subVolume1_membrane / Na_subVolume0_membrane))) - Voltage_membrane) * pow(m_o,3.0) * (1.0 - h_c));
FLUX subVolume1  - (12437.810945273632 * ((25.851990049751244 * log((Na_subVolume1_membrane / Na_subVolume0_membrane))) - Voltage_membrane) * pow(m_o,3.0) * (1.0 - h_c));
JUMP_CONDITION_END

MEMBRANE_END
*/
void FVSolver::loadMembrane(istream& ifsInput, Membrane* membrane) {
	//cout << "loading membrane " << var_name << endl;
	string nextToken, line;

	while (!ifsInput.eof()) {
		getline(ifsInput, line);
		istringstream lineInput(line);

		nextToken = "";
		lineInput >> nextToken;
		if (nextToken.size() == 0 || nextToken[0] == '#') {
			continue;
		}
		if (nextToken == "MEMBRANE_END") {
			break;
		}
		
		if (nextToken == "EQUATION_BEGIN") {
			string var_name;
			lineInput >> var_name;
			Variable* var = simTool->getSimulation()->getVariableFromName(var_name);
			loadVarContext(ifsInput, membrane, var);
		} else if (nextToken == "FAST_SYSTEM_BEGIN") {
			throw "Fast system not supported";
		} else if (nextToken == "JUMP_CONDITION_BEGIN") {
			string var_name;
			lineInput >> var_name;
			loadJumpCondition(ifsInput, membrane, var_name);
		}
	}
}

/*
# Simulation Parameters
SIMULATION_PARAM_BEGIN
SOLVER SUNDIALS_PDE_SOLVER 1.0E-7 1.0E-9 1.0
BASE_FILE_NAME \\cfs01.vcell.uchc.edu\raid\Vcell\users\fgao\SimID_36269803_0_
ENDING_TIME 0.1
TIME_STEP 0.1
KEEP_EVERY 1
SIMULATION_PARAM_END
--------OR--------------
# Simulation Parameters
SIMULATION_PARAM_BEGIN
SOLVER FV_SOLVER 1.0E-8
BASE_FILE_NAME \\cfs01.vcell.uchc.edu\raid\Vcell\users\fgao\SimID_36230826_0_
ENDING_TIME 1.0
TIME_STEP 0.01
KEEP_EVERY 10
SIMULATION_PARAM_END
*/
void FVSolver::loadSimulationParameters(istream& ifsInput) {
	string nextToken, line;

	while (!ifsInput.eof()) {
		getline(ifsInput, line);
		istringstream lineInput(line);

		nextToken = "";
		lineInput >> nextToken;
		if (nextToken.size() == 0 || nextToken[0] == '#') {
			continue;
		} 
		if (nextToken == "SIMULATION_PARAM_END") {
			break;
		}
		
		if (nextToken == "SOLVER") {
			string solver="";
			lineInput >> solver;
			simTool->setSolver(solver);
			if (solver == CHOMBO_SEMIIMPLICIT_SOLVER)
			{
			}
//			else if (solver == CHOMBO_SUNDIALS_SOLVER) {
//				double sundialsRelTol = 1e-7;
//				double sundialsAbsTol = 1e-9;
//				double maxStep = 0.1;
//				lineInput >> sundialsRelTol >> sundialsAbsTol >> maxStep;
//				simTool->setSundialsErrorTolerances(sundialsRelTol, sundialsAbsTol);
//				simTool->setSundialsMaxStep(maxStep);
//			}
		} else if (nextToken == "DISCONTINUITY_TIMES") {
			throw "discontinuity times not supported.";
//			int numDisTimes = 0;
//			lineInput >> numDisTimes;
//			if (numDisTimes > 0) {
//				double* discontinuityTimes = 0;
//				discontinuityTimes = new double[numDisTimes];
//				for (int i = 0; i < numDisTimes; i ++) {
//					lineInput >> discontinuityTimes[i];
//				}
//				simTool->setDiscontinuityTimes(numDisTimes, discontinuityTimes);
//			}
		} else if (nextToken == "BASE_FILE_NAME") {
			string basefilename;
			getline(lineInput, basefilename);
			trimString(basefilename);
			simTool->setBaseFilename(basefilename);
		} else if (nextToken == "PRIMARY_DATA_DIR") {
			string pd;
			getline(lineInput, pd);
			trimString(pd);
			simTool->setPrimaryDataDir(pd);
		} else if (nextToken == "ENDING_TIME") {
			double end_time;
			lineInput >> end_time;
			simTool->setEndTimeSec(end_time);
		}
		else if (nextToken == "TIME_INTERVALS") {
			int numts;
			lineInput >> numts;
			double p_tstop = 0;
			for (int i = 0; i < numts; ++ i)
			{
				getline(ifsInput, line);
				istringstream lineInput0(line);
				double dt, tstop;
				int keepEvery;
				lineInput0 >> tstop >> dt >> keepEvery;
				TimeInterval ts(p_tstop, tstop, dt, keepEvery);
				simTool->getSimulation()->addTimeInterval(ts);
				p_tstop = tstop;
			}
			simTool->setEndTimeSec(p_tstop);
		} else if (nextToken == "CHECK_SPATIALLY_UNIFORM") {
			throw "check spatially uniform not supported.";
		} else if (nextToken == "KEEP_EVERY") {
		} else if (nextToken == "KEEP_AT_MOST") {
			throw "keep at most not supported.";
//			int keep_at_most;
//			lineInput >> keep_at_most;
//			simTool->setKeepAtMost(keep_at_most);
		} else if (nextToken == "STORE_ENABLE") {
			int bStoreEnable=1;
			lineInput >> bStoreEnable;
			simTool->setStoreEnable(bStoreEnable!=0);
		} else {
			stringstream ss;
			ss << "loadSimulationParameters(), encountered unknown token " << nextToken << endl;
			throw ss.str();
		}
	}
}

/*
# Field Data
FIELD_DATA_BEGIN
#id, name, varname, time filename
0 _VCell_FieldData_0 FRAP_binding_ALPHA rfB 0.1 \\\\SAN2\\raid\\Vcell\\users\\fgao\\SimID_22489731_0_FRAP_binding_ALPHA_rfB_0_1.fdat
FIELD_DATA_END
*/
void FVSolver::loadFieldData(istream& ifsInput) {
	if (simTool->getSimulation() == 0) {
		throw "Simulation has to be initialized before loading field data";
	}
	throw "field data not supported";	
}

/*
# Parameters
PARAMETER_BEGIN 3
D
U0
U1
PARAMETER_END
*/
void FVSolver::loadParameters(istream& ifsInput, int numParameters) {
	if (simTool->getSimulation() == 0) {
		throw "Simulation has to be initialized before loading field data";
	}
	throw "parameters not supported";
}

void FVSolver::loadSerialScanParameters(istream& ifsInput, int numSerialScanParameters) {
	if (simTool->getSimulation() == 0) {
		throw "Simulation has to be initialized before loading serial scan parameters";
	}
	throw "serial scan parameters not supported";	
}

/*
# Parameter Scan Values
PARAMETER_SCAN_BEGIN 2
1.0 
2.0 
PARAMETER_SCAN_END
*/
void FVSolver::loadSerialScanParameterValues(istream& ifsInput, int numSerialScanParameterValues) {
	if (simTool->getSimulation() == 0) {
		throw "Simulation has to be initialized before loading serial scan parameter values";
	}
	throw "serial scan parameter not supported";
}

/*
# JMS_Paramters
JMS_PARAM_BEGIN
JMS_BROKER tcp://code:2507
JMS_USER serverUser cbittech
JMS_QUEUE workerEventDev
JMS_TOPIC serviceControlDev
VCELL_USER fgao
SIMULATION_KEY 36269803
JOB_INDEX 0
JMS_PARAM_END

# Simulation Parameters
SIMULATION_PARAM_BEGIN
SOLVER SUNDIALS_PDE_SOLVER 1.0E-7 1.0E-9 1.0
BASE_FILE_NAME \\cfs01.vcell.uchc.edu\raid\Vcell\users\fgao\SimID_36269803_0_
ENDING_TIME 0.1
TIME_STEP 0.1
KEEP_EVERY 1
SIMULATION_PARAM_END

# Model description: FEATURE name handle boundary_conditions
MODEL_BEGIN
FEATURE cyt 0 value value value value 
FEATURE ec 1 value value value value 
MEMBRANE cyt_ec_membrane cyt ec value value value value 
MODEL_END

# Mesh file
MESH_BEGIN
VCG_FILE \\cfs01.vcell.uchc.edu\raid\Vcell\users\fgao\SimID_36269803_0_.vcg
MESH_END

# Variables : type name time_dependent_flag advection_flag solve_whole_mesh_flag solve_regions
VARIABLE_BEGIN
VOLUME_PDE U false false false cyt
VARIABLE_END

COMPARTMENT_BEGIN cyt

BOUNDARY_CONDITIONS value value value value 

EQUATION_BEGIN U
INITIAL (sin((3.14159265358979 * x)) * sin((3.14159265358979 * y)) * exp( - (19.739208802178677 * t)));
RATE 0.0;
DIFFUSION 1.0;
VELOCITY_X 0.0;
VELOCITY_Y 0.0;
BOUNDARY_XM (sin((3.14159265358979 * x)) * sin((3.14159265358979 * y)) * exp( - (19.739208802178677 * t)));
BOUNDARY_XP (sin((3.14159265358979 * x)) * sin((3.14159265358979 * y)) * exp( - (19.739208802178677 * t)));
BOUNDARY_YM (sin((3.14159265358979 * x)) * sin((3.14159265358979 * y)) * exp( - (19.739208802178677 * t)));
BOUNDARY_YP (sin((3.14159265358979 * x)) * sin((3.14159265358979 * y)) * exp( - (19.739208802178677 * t)));
EQUATION_END

COMPARTMENT_END

COMPARTMENT_BEGIN ec

BOUNDARY_CONDITIONS value value value value 

EQUATION_BEGIN U
INITIAL (sin((3.14159265358979 * x)) * sin((3.14159265358979 * y)) * exp( - (19.739208802178677 * t)));
RATE 0.0;
DIFFUSION 0.0;
VELOCITY_X 0.0;
VELOCITY_Y 0.0;
BOUNDARY_XM (sin((3.14159265358979 * x)) * sin((3.14159265358979 * y)) * exp( - (19.739208802178677 * t)));
BOUNDARY_XP (sin((3.14159265358979 * x)) * sin((3.14159265358979 * y)) * exp( - (19.739208802178677 * t)));
BOUNDARY_YM (sin((3.14159265358979 * x)) * sin((3.14159265358979 * y)) * exp( - (19.739208802178677 * t)));
BOUNDARY_YP (sin((3.14159265358979 * x)) * sin((3.14159265358979 * y)) * exp( - (19.739208802178677 * t)));
EQUATION_END

COMPARTMENT_END


MEMBRANE_BEGIN cyt_ec_membrane cyt ec

BOUNDARY_CONDITIONS value value value value 

JUMP_CONDITION_BEGIN U
FLUX cyt (6.28318530717958 * exp( - (19.739208802178677 * t)) * ((cos((3.14159265358979 * x)) * sin((3.14159265358979 * y)) * x) + (sin((3.14159265358979 * x)) * cos((3.14159265358979 * y)) * y)));
FLUX ec 0.0;
JUMP_CONDITION_END

MEMBRANE_END
*/
void FVSolver::createSimTool(istream& ifsInput, int taskID)
{
	SimTool::create();
	simTool = SimTool::getInstance();

	string nextToken, line;

	while (!ifsInput.eof()) {
		getline(ifsInput, line);
		istringstream lineInput(line);

		nextToken = "";
		lineInput >> nextToken;
		if (nextToken.size() == 0 || nextToken[0] == '#') {
			continue;
		}

		if (nextToken == "JMS_PARAM_BEGIN") {
			// MessageEventManager owns the sending thread and starts it with the
			// singleton, so there is no separate start() step any more.
			loadJMSInfo(ifsInput, taskID);

#ifdef CH_MPI
			if (bConsoleOutput || SimTool::getInstance()->isRootRank())
#endif
			{
				SimulationMessaging::getInstVar()->setWorkerEvent(JobEvent::JOB_STARTING, "preprocessing started");
			}
		} else if (nextToken == "SIMULATION_PARAM_BEGIN") {
			loadSimulationParameters(ifsInput);
		} else if (nextToken == "MODEL_BEGIN") {
			loadModel(ifsInput);
		} else if (nextToken == "MESH_BEGIN") {
			throw "MESH_BEGIN is deprecated.";
		} else if (nextToken == "CHOMBO_SPEC_BEGIN") {
			loadChomboSpec(ifsInput);
		} else if (nextToken == "VARIABLE_BEGIN") {
			loadSimulation(ifsInput);
		} else if (nextToken == "PARAMETER_BEGIN") {
			int numParams = 0;
			lineInput >> numParams;
			loadParameters(ifsInput, numParams);
		} else if (nextToken == "SERIAL_SCAN_PARAMETER_BEGIN") {
			int numSerialScanParams = 0;
			lineInput >> numSerialScanParams;
			loadSerialScanParameters(ifsInput, numSerialScanParams);
		} else if (nextToken == "SERIAL_SCAN_PARAMETER_VALUE_BEGIN") {
			int numSerialScanParamValues = 0;
			lineInput >> numSerialScanParamValues;
			loadSerialScanParameterValues(ifsInput, numSerialScanParamValues);
		} else if (nextToken == "FIELD_DATA_BEGIN") {
			loadFieldData(ifsInput);
		} else if (nextToken == "POST_PROCESSING_BLOCK_BEGIN") {
			loadPostProcessingBlock(ifsInput);
		} else if (nextToken == "COMPARTMENT_BEGIN") {
			string feature_name;
			lineInput >> feature_name;
			Feature* feature = simTool->getModel()->getFeatureFromName(feature_name);
			if (feature != NULL) {
				loadFeature(ifsInput, feature);
			} else {
				throw "createSimTool(), Invalid compartment when loading feature!";
			}
		} else if (nextToken == "MEMBRANE_BEGIN") {
			string mem_name, feature1_name, feature2_name;
			lineInput >> mem_name >> feature1_name >> feature2_name;
			Membrane* membrane = simTool->getModel()->getMembraneFromName(mem_name);
			if (membrane != 0) {
				loadMembrane(ifsInput, membrane);
			} else {
				throw "createSimTool(), Invalid compartment when loading membrane!";
			}
		} else {
			stringstream ss;
			ss << "createSimTool(), encountered unknown token " << nextToken << endl;
			throw ss.str();
		}
	}
}

FVSolver::FVSolver(istream& fvinput, int taskID) {
	simTool = 0;
	createSimTool(fvinput, taskID);
}

void FVSolver::solve(bool convertChomboData)
{
#ifdef CH_MPI
	if (bConsoleOutput || SimTool::getInstance()->isRootRank())
#endif
	{
		SimulationMessaging::getInstVar()->setWorkerEvent(JobEvent::JOB_STARTING, "preprocessing finished");
	}
	simTool->start(convertChomboData);
}

void FVSolver::loadChomboSpec(istream& ifsInput) {

	string nextToken, line;

	ChomboSpec* chomboSpec = simTool->getChomboSpec();
	ChomboGeometry* chomboGeometry = chomboSpec->getChomboGeometry();

	while (!ifsInput.eof()) {
		nextToken = "";
		getline(ifsInput, line);
		istringstream lineInput(line);

		lineInput >> nextToken;
		if (nextToken.size() == 0 || nextToken[0] == '#') {
			continue;
		}
		if (nextToken == "CHOMBO_SPEC_END") {
			break;
		}

		if (nextToken == "DIMENSION") {
			int geoDim;
			lineInput >> geoDim;
			assert(geoDim == SpaceDim);
			chomboGeometry->setDimension(geoDim);
		} else if (nextToken == "MESH_SIZE") {
			IntVect meshSize;
			for (int i = 0; i < SpaceDim; i ++) {
				lineInput >> meshSize[i];
			}
			chomboGeometry->setMeshSize(meshSize);
		} else if (nextToken == "DOMAIN_SIZE") {
			RealVect domainSize;
			for (int idir = 0; idir < SpaceDim; idir ++) {
				lineInput >> domainSize[idir];
			}
			chomboGeometry->setDomainSize(domainSize);
		} else if (nextToken == "DOMAIN_ORIGIN") {
			RealVect domainOrigin;
			for (int idir = 0; idir < SpaceDim; idir ++) {
				lineInput >> domainOrigin[idir];
			}
			chomboGeometry->setDomainOrigin(domainOrigin);
		} else if (nextToken == "SUBDOMAINS") {
			int numSubdomains;
			string distanceMap;
			int phaseIndex;
			lineInput >> numSubdomains >> distanceMap;

			for (int i = 0; i < numSubdomains; i ++)
			{
				getline(ifsInput, line);
				istringstream lineInput(line);

				lineInput >> nextToken >> phaseIndex;
				Feature* feature = simTool->getModel()->getFeatureFromName(nextToken);

				if (distanceMap.empty())
				{
					// IF (R) expression
					getline(ifsInput, line);
					istringstream lineInput0(line);
					string whichExp;
					lineInput0 >> whichExp;
					string fullName = nextToken + ":" + whichExp;
					VCell::Expression* ifExp = readExpression(lineInput0, fullName);

					// user expression
					getline(ifsInput, line);
					istringstream lineInput1(line);
					lineInput1 >> whichExp;
					fullName = nextToken + ":" + whichExp;
					VCell::Expression* userExp = readExpression(lineInput1, fullName);
					chomboGeometry->addSubdomain(feature, phaseIndex, ifExp, userExp);
				} 
				else
				{
					string distanceMapFile;
					getline(lineInput, distanceMapFile);
					trimString(distanceMapFile);
					chomboGeometry->addSubdomain(feature, phaseIndex, distanceMapFile);
				}
			}
		} else if (nextToken == "REFINEMENTS") {
			int numLevels;
			lineInput >> numLevels;
			chomboSpec->setNumLevels(numLevels);
			int* refineratios = new int[numLevels];
			for (int i = 0; i < numLevels; i ++) {
				lineInput >> refineratios[i];
			}
			chomboSpec->setRefRatios(refineratios);
		} else if (nextToken == "MAX_BOX_SIZE") {
			int maxBoxSize = 64;
			lineInput >> maxBoxSize;
			chomboSpec->setMaxBoxSize(maxBoxSize);
		} else if (nextToken == "REFINEMENT_ROIS") {
			string roiType;
			int numRois;
			lineInput >> roiType >> numRois;
			for (int i = 0; i < numRois; ++ i)
			{
				int level;
				string roi;
				getline(ifsInput, line);
				istringstream lineInput0(line);
				lineInput0 >> level;
				getline(lineInput0, roi);
				roi.erase(roi.find_last_not_of(" \n\r\t") + 1);
				ChomboRefinementRoi* refinementRoi = new ChomboRefinementRoi(chomboGeometry, level, roi);
				if (roiType == "Membrane")
				{
					chomboSpec->addMembraneRefinementRoi(refinementRoi);
				}
				else if (roiType == "Volume")
				{
					chomboSpec->addVolumeRefinementRoi(refinementRoi);
				}
			}
		} else if (nextToken == "RELATIVE_TOLERANCE") {
			double rel_tol;
			lineInput >> rel_tol;
			chomboSpec->setRelTol(rel_tol);
		} else if (nextToken == "VIEW_LEVEL") {
			int viewLevel;
			lineInput >> viewLevel;
			chomboSpec->setViewLevel(viewLevel);
		} else if (nextToken == "SAVE_VCELL_OUTPUT") {
			lineInput >> nextToken;
			bool bSaveVCellOutput = nextToken == "true";
			chomboSpec->setSaveVCellOutput(bSaveVCellOutput);
		} else if (nextToken == "SAVE_CHOMBO_OUTPUT") {
			lineInput >> nextToken;
			bool bSaveChomboOutput = nextToken == "true";
			chomboSpec->setSaveChomboOutput(bSaveChomboOutput);
		} else if (nextToken == "FILL_RATIO") {
			double fillRatio = 0.9;
			lineInput >> fillRatio;
			chomboSpec->setFillRatio(fillRatio);
		} else if (nextToken == "ACTIVATE_FEATURE_UNDER_DEVELOPMENT") {
			lineInput >> nextToken;
			bool b = nextToken == "true";
			chomboSpec->setActivateFeatureUnderDevelopment(b);
		} else if (nextToken == "SMALL_VOLFRAC_THRESHOLD") {
			double d = 0;
			lineInput >> d;
			chomboSpec->setSmallVolfracThreshold(d);
		} else if (nextToken == "BLOCK_FACTOR") {
			int d = 0;
			lineInput >> d;
			chomboSpec->setBlockFactor(d);
		} else if (nextToken == "TAGS_GROW") {
			int d = 0;
			lineInput >> d;
			chomboSpec->setTagsGrow(d);
		}
	}

	chomboSpec->printSummary();
}

/**
# Post Processing Block
POST_PROCESSING_BLOCK_BEGIN
PROJECTION_DATA_GENERATOR postDex cell x sum (5.0 * Dex_cell);
POST_PROCESSING_BLOCK_END
*/
void FVSolver::loadPostProcessingBlock(istream& ifsInput){
	// create post processing block;
	PostProcessingBlock* postProcessingBlock = simTool->getSimulation()->createPostProcessingBlock();
	// add var statistics data generator always
	postProcessingBlock->addDataGenerator(new VariableStatisticsDataGenerator());

	string line, nextToken;
	while (!ifsInput.eof()) {
		getline(ifsInput, line);
		istringstream lineInput(line);

		nextToken = "";
		lineInput >> nextToken;
		if (nextToken.size() == 0 || nextToken[0] == '#') {
			continue;
		}
		if (nextToken == "POST_PROCESSING_BLOCK_END") {
			break;
		}

		if (nextToken == "PROJECTION_DATA_GENERATOR")
		{
			throw "Projection data generator is not supported yet";
		} 
		else if (nextToken == "GAUSSIAN_CONVOLUTION_DATA_GENERATOR")
		{
			throw "Guassian convolution data generator is not supported yet";
		} 
		else if (nextToken == "ROI_DATA_GENERATOR_BEGIN")
		{
			throw "ROI data generator is not supported yet";
		} 
		else
		{
			stringstream ss;
			ss << "loadPostProcessingBlock(), encountered unknown token " << nextToken << endl;
			throw ss.str();
		}
	}
}
