#include <VCELL/FVSolver.h>

#include <cassert>
#include <sys/stat.h>
#include <cstring>
#include <fstream>
#include <sstream>
using std::ifstream;
using std::istringstream;
using std::stringstream;
using std::endl;


#include <Expression.h>
using VCell::Expression;

#include <VCELL/Element.h>
#include <VCELL/VolumeVariable.h>
#include <VCELL/VolumeParticleVariable.h>
#include <VCELL/VolumeRegionVariable.h>
#include <VCELL/MembraneVariable.h>
#include <VCELL/MembraneParticleVariable.h>
#include <VCELL/MembraneRegionVariable.h>
#include <VCELL/VCellModel.h>
#include <VCELL/Feature.h>
#include <VCELL/Membrane.h>
#include <VCELL/SimTool.h>
#include <VCELL/ODESolver.h>
#include <VCELL/EqnBuilderReactionForward.h>
#include <VCELL/MembraneEqnBuilderForward.h>
#include <VCELL/VolumeRegion.h>
#include <VCELL/MembraneRegion.h>
#include <VCELL/MembraneRegionEqnBuilder.h>
#include <VCELL/VolumeRegionEqnBuilder.h>
#include <VCELL/SparseMatrixEqnBuilder.h>
#include <VCELL/SparseVolumeEqnBuilder.h>
#include <VCELL/EllipticVolumeEqnBuilder.h>
#include <VCELL/MembraneEqnBuilderDiffusion.h>
#include <VCELL/SparseLinearSolver.h>
#include <VCELL/FastSystemExpression.h>
#include <VCELL/VolumeVarContextExpression.h>
#include <VCELL/VolumeRegionVarContextExpression.h>
#include <VCELL/MembraneVarContextExpression.h>
#include <VCELL/MembraneRegionVarContextExpression.h>
#include <VCELL/SimulationMessaging.h>
#include <VCELL/SimulationExpression.h>
#include <VCELL/CartesianMesh.h>
#include <VCELL/FieldData.h>
#include <VCELL/FVUtils.h>
#include <VCELL/RandomVariable.h>
#include <VCELL/SundialsSolverOptions.h>
#include <VCELL/PostProcessingBlock.h>
#include <VCELL/ProjectionDataGenerator.h>
#include <VCELL/GaussianConvolutionDataGenerator.h>
#include <VCELL/VariableStatisticsDataGenerator.h>
#include <VCELL/RoiDataGenerator.h>

FieldData *getPSFFieldData(const SimulationExpression* sim) {
	return sim->getPSFFieldData();
}

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
	char *broker = new char[256];
	char *smqusername = new char[256];
	char *password = new char[256];
	char *qname = new char[256];
	char *tname = new char[256];
	char *vcusername = new char[256];
	string nextToken, line;
	int simKey, jobIndex;

	while (!ifsInput.eof()) {
		getline(ifsInput, line);
		istringstream lineInput(line);

		nextToken = "";
		lineInput >> nextToken;
		if (nextToken.empty() || nextToken[0] == '#') {
			continue;
		}
		if (nextToken == "JMS_PARAM_END") {
			break;
		}

		if (nextToken == "JMS_BROKER") {
			memset(broker, 0, 256 * sizeof(char));
			lineInput >> broker;
		} else if (nextToken == "JMS_USER") {
			memset(smqusername, 0, 256 * sizeof(char));
			memset(password, 0, 256 * sizeof(char));
			lineInput >> smqusername >> password;
		} else if (nextToken == "JMS_QUEUE") {
			memset(qname, 0, 256 * sizeof(char));
			lineInput >> qname;
		} else if (nextToken == "JMS_TOPIC") {
			memset(tname, 0, 256 * sizeof(char));
			lineInput >> tname;
		} else if (nextToken == "VCELL_USER") {
			memset(vcusername, 0, 256 * sizeof(char));
			lineInput >> vcusername;
		} else if (nextToken == "SIMULATION_KEY") {
			lineInput >> simKey;
		} else if (nextToken == "JOB_INDEX") {
			lineInput >> jobIndex;
		}
	}

#ifdef USE_MESSAGING
	if (taskID >= 0) {
		SimulationMessaging::create(broker, smqusername, password, qname, tname, vcusername, simKey, jobIndex, taskID);
	} else {
		SimulationMessaging::create();
	}
#else
	SimulationMessaging::create();
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
VCellModel* FVSolver::loadModel(istream& ifsInput) {
	//cout << "loading model " << endl;
	auto* model = new VCellModel();
	string nextToken, line;
	string feature_name;
	int handle;
	int numFeatures = 0, numMembranes = 0;

	while (!ifsInput.eof()) {
		getline(ifsInput, line);
		istringstream lineInput(line);

		nextToken = "";
		lineInput >> nextToken;
		if (nextToken.empty() || nextToken[0] == '#') {
			continue;
		} 
		if (nextToken == "MODEL_END") {
			break;
		} 
		
		Structure* structure = nullptr;
		if (nextToken == "FEATURE") {
			numFeatures ++;
			lineInput >> feature_name >> handle;
			Feature* feature = model->addFeature(feature_name, handle);
			structure = feature;
		} else if (nextToken == "MEMBRANE") {
			numMembranes ++;
			string membrane_name, feature1_name, feature2_name;
			lineInput >> membrane_name >> feature1_name >> feature2_name;
			Membrane* membrane = model->addMembrane(membrane_name, feature1_name, feature2_name);
			structure = membrane;
		}

		string btstr;
		for (int i = 0; i < 6; i ++) {
			if (lineInput.eof()) {
				break;
			}
			lineInput >> btstr;
			if (btstr.empty()) {
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
				throw runtime_error(ss1.str());
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
	return model;
}

int FVSolver::loadSolveRegions(VCellModel* model, istream& lineInput, int*& solveRegions) const
{
	int numVolumeRegions = mesh->getNumVolumeRegions();
	int regionCount = 0;
	while (true) {
		string feature_name;
		lineInput >> feature_name;
		if (feature_name.empty()) {
			break;
		}
		for (int i = 0; i < numVolumeRegions; i++){
			VolumeRegion *volRegion = mesh->getVolumeRegion(i);
			Feature* feature = model->getFeatureFromName(feature_name);
			if (feature == nullptr) {
				stringstream ss;
				ss << "Feature '" << feature_name << "' doesn't exist!";
				throw runtime_error(ss.str());
			}
			if (solveRegions == nullptr) {
				solveRegions = new int[numVolumeRegions];
			}
			if (volRegion->getFeature()->getHandle() == feature->getHandle()){
				solveRegions[regionCount ++] = volRegion->getIndex();
			}
		}
	}
	return regionCount;
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
SimulationExpression* FVSolver::loadSimulation(SimTool* simTool, VCellModel *model, istream& ifsInput) {
	//cout << "loading simulation" << endl;
	auto* simulation = new SimulationExpression(mesh);
	string nextToken, line;
	long sizeX = mesh->getNumVolumeX();
	long sizeY = mesh->getNumVolumeY();
	long sizeZ = mesh->getNumVolumeZ();
	int numVolumeRegions = mesh->getNumVolumeRegions();
	string variable_name, structure_name;

	while (!ifsInput.eof()) {
		getline(ifsInput, line);
		istringstream lineInput(line);

		nextToken = "";
		lineInput >> nextToken;
		if (nextToken.empty() || nextToken[0] == '#') {
			continue;
		}
		if (nextToken == "VARIABLE_END") {
			break;
		}

		if (nextToken == "VOLUME_RANDOM") {
			string name;
			ifsInput >> name;
			auto* rv = new RandomVariable(name, VAR_VOLUME, mesh->getNumVolumeElements());
			simulation->addRandomVariable(rv);
		} else if (nextToken == "MEMBRANE_RANDOM") {
			string name;
			ifsInput >> name;
			auto* rv = new RandomVariable(name, VAR_MEMBRANE, mesh->getNumMembraneElements());
			simulation->addRandomVariable(rv);
		} else if (nextToken == "VOLUME_PDE" || nextToken == "VOLUME_PDE_STEADY") {
			bool bSteady = false;
			if (nextToken == "VOLUME_PDE_STEADY") {
				bSteady = true;
			}

			bool bNoConvection = true;
			bool bTimeDependent = false;
			string advectionflag, time_dependent_diffusion_flag, solve_whole_mesh_flag;
			string gradient_flag;
			lineInput >> variable_name >> structure_name >> time_dependent_diffusion_flag >> advectionflag >> gradient_flag >> solve_whole_mesh_flag;

			assert(solve_whole_mesh_flag == "true" ||  solve_whole_mesh_flag == "false");

			bool bSolveVariable = true;
			int numSolveRegions = 0;  // flag specifying to solve for all regions
			int *solveRegions = nullptr;

			bool* vrmap = new bool[numVolumeRegions];
			for (int i = 0; i < numVolumeRegions; i ++) {
				vrmap[i] = true;
			} // by default, solve everywhere

			if (solve_whole_mesh_flag == "false") {
				memset(vrmap, 0, numVolumeRegions * sizeof(bool)); // if not solve everywhere;
				numSolveRegions = loadSolveRegions(model, lineInput, solveRegions);

				if (numSolveRegions == 0) {
					bSolveVariable = false;
				} else {
					for (int r = 0; r < numSolveRegions; r ++) {
						vrmap[solveRegions[r]] = true;
					}
				}
			}

			if (time_dependent_diffusion_flag == "true") {
				bTimeDependent = true;
				simulation->setHasTimeDependentDiffusionAdvection();
			}
			if (advectionflag == "true" ) {
				bNoConvection = false;
			}

			bool bGradient = gradient_flag == "true";
			Feature* feature = model->getFeatureFromName(structure_name);
			auto* volumeVar = new VolumeVariable(variable_name, feature, sizeX, sizeY, sizeZ, true, !bNoConvection, bGradient);
			if (bSolveVariable && !simTool->isSundialsPdeSolver()) {
				SparseMatrixEqnBuilder* builder = nullptr;
				if (bSteady) {
					builder = new EllipticVolumeEqnBuilder(volumeVar,mesh, numSolveRegions, solveRegions);
				} else {
					builder = new SparseVolumeEqnBuilder(volumeVar,mesh, bNoConvection, numSolveRegions, solveRegions);
				}
				PDESolver* pdeSolver = new SparseLinearSolver(volumeVar,builder,simTool->getPCGRelativeErrorTolerance(),bTimeDependent);
				simulation->addSolver(pdeSolver);
			}
			simulation->addVolumeVariable(volumeVar, vrmap);
		} else if (nextToken == "VOLUME_ODE") {
			string solve_whole_mesh_flag;
			lineInput >> variable_name >> structure_name >> solve_whole_mesh_flag;

			assert(solve_whole_mesh_flag == "true" ||  solve_whole_mesh_flag == "false");

			int numSolveRegions = 0;  // flag specifying to solve for all regions
			int *solveRegions = nullptr;

			bool* vrmap = new bool[numVolumeRegions];
			for (int i = 0; i < numVolumeRegions; i ++) {
				vrmap[i] = true;
			} // by default, solve everywhere

			bool bSolveVariable = true;
			if (solve_whole_mesh_flag == "false") {
				memset(vrmap, 0, numVolumeRegions * sizeof(bool)); // if not solve everywhere;
				numSolveRegions = loadSolveRegions(model, lineInput, solveRegions);

				if (numSolveRegions == 0) {
					bSolveVariable = false;
				} else {
					for (int r = 0; r < numSolveRegions; r ++) {
						vrmap[solveRegions[r]] = true;
					}
				}
			}

			Feature* feature = model->getFeatureFromName(structure_name);
			auto* volumeVar = new VolumeVariable(variable_name, feature, sizeX, sizeY, sizeZ, false);
			if (bSolveVariable && !simTool->isSundialsPdeSolver()) {
				auto* odeSolver = new ODESolver(volumeVar,mesh,numSolveRegions,solveRegions);
				EqnBuilder* builder = new EqnBuilderReactionForward(volumeVar,mesh,odeSolver);
				odeSolver->setEqnBuilder(builder);
				simulation->addSolver(odeSolver);
			}
			simulation->addVolumeVariable(volumeVar, vrmap);
		} else if (nextToken == "VOLUME_PARTICLE") {
			lineInput >> variable_name >> structure_name;
			Feature* feature = model->getFeatureFromName(structure_name);
			auto* volumeParticleVar = new VolumeParticleVariable(variable_name, feature, sizeX, sizeY, sizeZ);
			simulation->addVolumeParticleVariable(volumeParticleVar);
		} else if (nextToken == "MEMBRANE_PARTICLE") {
			lineInput >> variable_name >> structure_name;
			Membrane* membrane = model->getMembraneFromName(structure_name);
			auto* membraneParticleVar = new MembraneParticleVariable(variable_name, membrane, mesh->getNumMembraneElements());
			simulation->addMembraneParticleVariable(membraneParticleVar);
		} else if (nextToken == "MEMBRANE_ODE") {
			lineInput >> variable_name >> structure_name;
			int numSolveRegions = 0;  // flag specifying to solve for all regions
			int *solveRegions = nullptr;

			Membrane* membrane = model->getMembraneFromName(structure_name);
			MembraneVariable* membraneVar = new MembraneVariable(variable_name, membrane, mesh->getNumMembraneElements(), false);
			if (!simTool->isSundialsPdeSolver()) {
				auto* odeSolver = new ODESolver(membraneVar,mesh,numSolveRegions,solveRegions);
				EqnBuilder* builder = new MembraneEqnBuilderForward(membraneVar,mesh,odeSolver);
				odeSolver->setEqnBuilder(builder);
				simulation->addSolver(odeSolver);
			}
			simulation->addMembraneVariable(membraneVar);
		} else if (nextToken == "MEMBRANE_PDE") {
			bool bNoConvection = true;    // define symmflg = 0 (general) or 1 (symmetric)
			bool bTimeDependent = false;
			string time_dependent_diffusion_flag;
			lineInput >> variable_name >> structure_name >> time_dependent_diffusion_flag;
			if (time_dependent_diffusion_flag == "true") {
				bTimeDependent = true;
				simulation->setHasTimeDependentDiffusionAdvection();
			}

			Membrane* membrane = model->getMembraneFromName(structure_name);
			auto* membraneVar = new MembraneVariable(variable_name, membrane, mesh->getNumMembraneElements(), true);
			if (!simTool->isSundialsPdeSolver()) {
				SparseMatrixEqnBuilder* smbuilder = new MembraneEqnBuilderDiffusion(membraneVar,mesh);
				auto* slSolver = new SparseLinearSolver(membraneVar,smbuilder,simTool->getPCGRelativeErrorTolerance(),bTimeDependent);
				simulation->addSolver(slSolver);
			}
			simulation->addMembraneVariable(membraneVar);
		} else if (nextToken == "VOLUME_REGION") {
			lineInput >> variable_name >> structure_name;
			int numSolveRegions = 0;  // flag specifying to solve for all regions
			int *solveRegions = nullptr;

			Feature* feature = model->getFeatureFromName(structure_name);
			auto* volumeRegionVar = new VolumeRegionVariable(variable_name, feature, mesh->getNumVolumeRegions());
			if (!simTool->isSundialsPdeSolver()) {
				auto* odeSolver = new ODESolver(volumeRegionVar,mesh,numSolveRegions,solveRegions);
				EqnBuilder* builder = new VolumeRegionEqnBuilder(volumeRegionVar,mesh,odeSolver);
				odeSolver->setEqnBuilder(builder);
				simulation->addSolver(odeSolver);
			}
			simulation->addVolumeRegionVariable(volumeRegionVar);
		} else if (nextToken == "MEMBRANE_REGION") {
			lineInput >> variable_name >> structure_name;
			int numSolveRegions = 0;  // flag specifying to solve for all regions
			int *solveRegions = nullptr;

			Membrane* membrane = model->getMembraneFromName(structure_name);
			auto* memRegionVariable = new MembraneRegionVariable(variable_name, membrane, mesh->getNumMembraneRegions());
			if (!simTool->isSundialsPdeSolver()) {
				auto* odeSolver = new ODESolver(memRegionVariable,mesh,numSolveRegions,solveRegions);
				EqnBuilder* builder = new MembraneRegionEqnBuilder(memRegionVariable,mesh,odeSolver);
				odeSolver->setEqnBuilder(builder);
				simulation->addSolver(odeSolver);
			}
			simulation->addMembraneRegionVariable(memRegionVariable);
		}
	}
	return simulation;
}

Expression* FVSolver::readExpression(istream& lineInput, string& var_name, const string& prefix) {
	string expStr;
	getline(lineInput, expStr);
	expStr = prefix + expStr;
	trimString(expStr);
	if (expStr[expStr.size()-1] != ';') {
		stringstream msg;
		msg << "Expression for [" << var_name << "] is not terminated by ';'";
		throw runtime_error(msg.str());
	}
	return new Expression(expStr);
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
VarContext* FVSolver::loadEquation(istream& ifsInput, Structure* structure, Variable* var) {

	//cout << "loading volume var context " << var_name << endl;

	VarContext* varContext = nullptr;
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
		ss << "loadEquation: variable type not supported yet: " << var->getName();
		throw runtime_error(ss.str());
	}

	string nextToken, line;

	while (!ifsInput.eof()) {
		getline(ifsInput, line);
		istringstream lineInput(line);

		nextToken = "";
		lineInput >> nextToken;
		if (nextToken.empty() || nextToken[0] == '#') {
			continue;
		}
		if (nextToken == "EQUATION_END") {
			break;
		}

		string var_name = var->getName();
		Expression* exp = readExpression(lineInput, var_name);
		int expIndex = 0;
		if (nextToken == "INITIAL") {
			expIndex = INITIAL_VALUE_EXP;
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
		} else if (nextToken == "GRADIENT_X") {
			expIndex = GRADIENT_X_EXP;
		} else if (nextToken == "GRADIENT_Y") {
			expIndex = GRADIENT_Y_EXP;
		} else if (nextToken == "GRADIENT_Z") {
			expIndex = GRADIENT_Z_EXP;
		} else {
			stringstream ss;
			ss << "FVSolver::loadEquation(), unexpected token " << nextToken;
			throw ss.str();
		}
		varContext->setExpression(exp, expIndex);
	}
	return varContext;
}

/*
JUMP_CONDITION_BEGIN Ca
FLUX Nucleus (150.0 * (Ca_Cytosol_membrane - Ca_Nucleus_membrane));
FLUX Cytosol  - (150.0 * (Ca_Cytosol_membrane - Ca_Nucleus_membrane));
JUMP_CONDITION_END
*/
void FVSolver::loadJumpCondition(SimulationExpression* simulation, VCellModel* model, istream& ifsInput, Membrane* membrane, string& var_name) {
	//cout << "loading jump condition " << var_name << endl;
	string nextToken, line;

	Variable* var = simulation->getVariableFromName(var_name);	
	while (!ifsInput.eof()) {
		getline(ifsInput, line);
		istringstream lineInput(line);

		nextToken = "";
		lineInput >> nextToken;
		if (nextToken.empty() || nextToken[0] == '#') {
			continue;
		}  
		if (nextToken == "JUMP_CONDITION_END") {
			break;
		}
		if (nextToken == "FLUX") {
			string featurename;
			lineInput >> featurename;
			assert(!featurename.empty());
			Feature* f = model->getFeatureFromName(featurename);
			string var_name = var->getName();
			Expression* exp = readExpression(lineInput, var_name);
			VarContext *varContext = nullptr;
			if (var->getVarType() == VAR_VOLUME) {
				varContext = f->getVolumeVarContext((VolumeVariable*)var);
			} else if (var->getVarType() == VAR_VOLUME_REGION) {
				varContext = f->getVolumeRegionVarContext((VolumeRegionVariable*)var);
			} else {
				throw runtime_error("Only volume variables and volume region variables have jump conditions");
			}
			if (varContext == nullptr) {
				stringstream ss;
				ss << "variable '" << var->getName() << "' is not defined in " << featurename;
				throw runtime_error(ss.str());
			}
			varContext->addJumpCondition(membrane, exp);
		} else {
			throw runtime_error("Expecting FLUX in JumpCondition.");
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
	auto* vars = new string[numDep];
	auto **expressions = new Expression*[numDep];

	while (!ifsInput.eof()) {
		getline(ifsInput, line);
		istringstream lineInput(line);

		nextToken = "";
		lineInput >> nextToken;
		if (nextToken.empty() || nextToken[0] == '#') {
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
	auto **expressions = new Expression*[numIndep];

	while (!ifsInput.eof()) {
		getline(ifsInput, line);
		istringstream lineInput(line);

		nextToken = "";
		lineInput >> nextToken;
		if (nextToken.empty() || nextToken[0] == '#') {
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
		throw runtime_error("In the fast system the number of fast rates should be the same as that of independent variables");
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
	auto* vars = new string[numDep];
	auto **expressions = new Expression*[numDep];

	while (!ifsInput.eof()) {
		getline(ifsInput, line);
		istringstream lineInput(line);

		nextToken = "";
		lineInput >> nextToken;
		if (nextToken.empty() || nextToken[0] == '#') {
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
		throw runtime_error("In the fast system the number of fast dependencies should be the same as that of dependent variables");
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
	auto **expressions = new Expression*[numIndep * numIndep];

	while (!ifsInput.eof()) {
		getline(ifsInput, line);
		istringstream lineInput(line);

		nextToken = "";
		lineInput >> nextToken;
		if (nextToken.empty() || nextToken[0] == '#') {
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
		throw runtime_error("In the fast system the number of Jacobian should dim*dim");
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
		if (nextToken.empty() || nextToken[0] == '#') {
			continue;
		}  
		if (nextToken == "FAST_SYSTEM_END") {
			break;
		} 
		
		if (nextToken == "DEPENDENT_VARIALBES") {
			auto* vars = new string[numDep];
			for (int i = 0; i < numDep; i ++) {
				lineInput >> vars[i];
			}
			fastSystem->setDependentVariables(vars);
			delete[] vars;
		} else if (nextToken == "INDEPENDENT_VARIALBES") {
			auto* vars = new string[numIndep];
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
void FVSolver::loadFeature(SimulationExpression* simulation, istream& ifsInput, Feature* feature) {
	//cout << "loading feature " << feature->getName() << endl;
	string nextToken, line;

	while (!ifsInput.eof()) {
		getline(ifsInput, line);
		istringstream lineInput(line);

		nextToken = "";
		lineInput >> nextToken;
		if (nextToken.empty() || nextToken[0] == '#') {
			continue;
		}
		if (nextToken == "COMPARTMENT_END") {
			break;
		}

		if (nextToken == "EQUATION_BEGIN") {
			char var_name[256];
			lineInput >> var_name;
			Variable* var = simulation->getVariableFromName(var_name);
			VarContext *varContext = loadEquation(ifsInput, feature, var);
			if (var->getVarType() == VAR_VOLUME) {
				feature->addVolumeVarContext((VolumeVarContextExpression*)varContext);
			} else {
				feature->addVolumeRegionVarContext((VolumeRegionVarContextExpression*)varContext);
			}
		} else if (nextToken == "FAST_SYSTEM_BEGIN") {
			int dimension, num_of_dependents;
			lineInput >> dimension >> num_of_dependents;
			auto* fastSystem = new FastSystemExpression(dimension, num_of_dependents, simulation);
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
void FVSolver::loadMembrane(SimulationExpression* simulation, VCellModel* model, istream& ifsInput, Membrane* membrane) {
	//cout << "loading membrane " << var_name << endl;
	string nextToken, line;

	while (!ifsInput.eof()) {
		getline(ifsInput, line);
		istringstream lineInput(line);

		nextToken = "";
		lineInput >> nextToken;
		if (nextToken.empty() || nextToken[0] == '#') {
			continue;
		}
		if (nextToken == "MEMBRANE_END") {
			break;
		}
		
		if (nextToken == "EQUATION_BEGIN") {
			string var_name;
			lineInput >> var_name;
			Variable* var = simulation->getVariableFromName(var_name);
			VarContext *varContext = loadEquation(ifsInput, membrane, var);
			if (var->getVarType() == VAR_MEMBRANE) {
				membrane->addMembraneVarContext((MembraneVarContextExpression*)varContext);
			} else {
				membrane->addMembraneRegionVarContext((MembraneRegionVarContextExpression*)varContext);
			}
		}  else if (nextToken == "FAST_SYSTEM_BEGIN") {
			int dimension, num_of_dependents;
			lineInput >> dimension >> num_of_dependents;
			auto* fastSystem = new FastSystemExpression(dimension, num_of_dependents, simulation);
			loadFastSystem(ifsInput, fastSystem);
			membrane->setFastSystem(fastSystem);
		} else if (nextToken == "JUMP_CONDITION_BEGIN") {
			string var_name;
			lineInput >> var_name;
			loadJumpCondition(simulation, model, ifsInput, membrane, var_name);
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
void FVSolver::loadSimulationParameters(SimTool* simTool, istream& ifsInput) const
{
	string nextToken, line;

	while (!ifsInput.eof()) {
		getline(ifsInput, line);
		istringstream lineInput(line);

		nextToken = "";
		lineInput >> nextToken;
		if (nextToken.empty() || nextToken[0] == '#') {
			continue;
		} 
		if (nextToken == "SIMULATION_PARAM_END") {
			break;
		}
		
		if (nextToken == "SOLVER") {
			string solver;
			lineInput >> solver;
			simTool->setSolver(solver);
			if (solver == FV_SOLVER) {
				double pcgRelTol = 1e-8;
				lineInput >> pcgRelTol;
				simTool->setPCGRelativeErrorTolerance(pcgRelTol);
			} else {
				std::vector<std::string> result;
				for (std::string s; lineInput >> s;) {
					result.push_back(s);
				}
				SundialsSolverOptions sso;
				sso.relTol = std::atof(result[0].data());
				sso.absTol = std::atof(result[1].data());
				sso.maxStep= std::atof(result[2].data());
				if(result.size() == 4) {
					if (result[3].compare("true") == 0 || result[3].compare("false") == 0) {
						bool b;
						std::istringstream is(result[3]);
						is >> std::boolalpha >> b;
						sso.borderExtrapolationDisable = b;
					} else {
						sso.maxOrderAdvection = std::atoi(result[3].data());
					}
				}else if(result.size() == 5){
					sso.maxOrderAdvection = std::atoi(result[3].data());
					bool b;
					std::istringstream is(result[4]);
					is >> std::boolalpha >> b;
					sso.borderExtrapolationDisable = b;
				}
//				lineInput >> sso.relTol >> sso.absTol >> sso.maxStep >> sso.maxOrderAdvection >> sso.borderExtrapolationDisable;
				simTool->setSundialsSolverOptions(sso);
			}
		} else if (nextToken == "DISCONTINUITY_TIMES") {
			int numDisTimes = 0;
			lineInput >> numDisTimes;
			if (numDisTimes > 0) {
				double* discontinuityTimes = nullptr;
				discontinuityTimes = new double[numDisTimes];
				for (int i = 0; i < numDisTimes; i ++) {
					lineInput >> discontinuityTimes[i];
				}
				simTool->setDiscontinuityTimes(numDisTimes, discontinuityTimes);
			}
		} else if (nextToken == "BASE_FILE_NAME") {
			string basefilename;
			getline(lineInput, basefilename);
			trimString(basefilename);
			if (outputPath == nullptr) {
				simTool->setBaseFilename((char*)basefilename.c_str());
			} else {
				const char* baseSimName = strrchr(basefilename.c_str(), DIRECTORY_SEPARATOR);
				if (baseSimName == nullptr) {
					baseSimName = basefilename.c_str();
				} else {
					baseSimName += 1;
				}
				char newBaseName[512];
				sprintf(newBaseName, "%s%c%s", outputPath, DIRECTORY_SEPARATOR, baseSimName);
				simTool->setBaseFilename(newBaseName);
			}	
		} else if (nextToken == "ENDING_TIME") {
			double end_time;
			lineInput >> end_time;
			simTool->setEndTimeSec(end_time);
		} else if (nextToken == "TIME_STEP") {
			double time_step;
			lineInput >> time_step;
			simTool->setTimeStep(time_step);
		} else if (nextToken == "SMOLDYN_STEP_MULTIPLIER") {
			int smoldynStepMultiplier;
			lineInput >> smoldynStepMultiplier;
			simTool->setSmoldynStepMultiplier(smoldynStepMultiplier);
		} else if (nextToken == "CHECK_SPATIALLY_UNIFORM") {
			double spatiallyUniformAbsTol = 1e-6;
			double spatiallyUniformRelTol = 1e-3;
			lineInput >> spatiallyUniformAbsTol >> spatiallyUniformRelTol;
			simTool->setCheckSpatiallyUniform();
			simTool->setSpatiallyUniformErrorTolerance(spatiallyUniformAbsTol, spatiallyUniformRelTol);
		} else if (nextToken == "KEEP_EVERY") {
			int keep_every = 1;
			string one_step, keep_every_str;
			lineInput >> one_step >> keep_every_str;
			if (one_step == "ONE_STEP") {
				simTool->setSundialsOneStepOutput();
			} else {
				keep_every_str = one_step;	
			}	
			keep_every = atoi(keep_every_str.c_str());
			
			simTool->setKeepEvery(keep_every);
		} else if (nextToken == "KEEP_AT_MOST") {
			int keep_at_most;
			lineInput >> keep_at_most;
			simTool->setKeepAtMost(keep_at_most);
		} else if (nextToken == "STORE_ENABLE") {
			int bStoreEnable=1;
			lineInput >> bStoreEnable;
			simTool->setStoreEnable(bStoreEnable!=0);
		} else {
			throw runtime_error("loadSimulationParameters(), encountered unknown token " + nextToken);
		}
	}
	
	//SimTool::getInstance()->setFileCompress(false);
}

/*
# Mesh file
MESH_BEGIN
VCG_FILE \\cfs01.vcell.uchc.edu\raid\Vcell\users\fgao\SimID_36230826_0_.vcg
MESH_END
*/
void FVSolver::loadMeshFromVcg(VCellModel* model, istream& vcgInput) {
	if (model == nullptr) {
		throw std::runtime_error("Model has to be initialized before mesh initialization");
	}

	SimulationMessaging::getInstVar()->setWorkerEvent(new WorkerEvent(JOB_STARTING, "initializing mesh"));
	mesh = new CartesianMesh();
	mesh->initialize(model, vcgInput);
	SimulationMessaging::getInstVar()->setWorkerEvent(new WorkerEvent(JOB_STARTING, "mesh initialized"));
}

/*
# Field Data
FIELD_DATA_BEGIN
#id, name, varname, time filename
0 _VCell_FieldData_0 FRAP_binding_ALPHA rfB 0.1 \\\\SAN2\\raid\\Vcell\\users\\fgao\\SimID_22489731_0_FRAP_binding_ALPHA_rfB_0_1.fdat
FIELD_DATA_END
*/
void FVSolver::loadFieldData(SimulationExpression* simulation, istream& ifsInput) {
	if (simulation == nullptr) {
		throw runtime_error("Simulation has to be initialized before loading field data");
	}

	string nextToken, line;
	int fdIndex;
	string fdVarType, fdID, fdName, fdVarName, fdFile;
	double fdTime;

	while (!ifsInput.eof()) {
		getline(ifsInput, line);
		istringstream lineInput(line);

		nextToken = "";
		lineInput >> nextToken;
		if (nextToken.empty() || nextToken[0] == '#') {
			continue;
		}
		if (nextToken == "FIELD_DATA_END") {
			break;
		}
		
		if (nextToken == "PSF_FIELD_DATA_INDEX") {
			lineInput >> fdIndex;
			simulation->setPSFFieldDataIndex(fdIndex);
		} else {
			fdIndex = -1;
			stringstream ss(nextToken);
			ss >> fdIndex;
			fdTime = -1;
			lineInput >> fdVarType >> fdID >> fdName >> fdVarName >> fdTime;
			getline(lineInput,  fdFile);
			trimString(fdFile);
			if (fdVarType.empty() || fdID.empty() || fdName.empty() || fdVarName.empty() || fdFile.empty() || fdIndex < 0 || fdTime < 0) {
				throw runtime_error("loadFieldData(), wrong input");
			}
			VariableType varType = VAR_UNKNOWN;
			if (fdVarType == "Membrane") {
				varType = VAR_MEMBRANE;
			} else if (fdVarType == "Volume") {
				varType = VAR_VOLUME;
			} else {
				throw runtime_error("field data is only supported for volume and membrane variables");
			}
			simulation->addFieldData(new FieldData(fdIndex, varType, fdID, fdName, fdVarName, fdTime, fdFile));
		}
	}
}

/*
# Parameters
PARAMETER_BEGIN 3
D
U0
U1
PARAMETER_END
*/
void FVSolver::loadParameters(SimulationExpression* simulation, istream& ifsInput, int numParameters) {
	if (simulation == nullptr) {
		throw runtime_error("Simulation has to be initialized before loading field data");
	}

	string nextToken, line;
	int nread = 0;
	while (!ifsInput.eof()) {
		getline(ifsInput, line);
		istringstream lineInput(line);

		lineInput >> nextToken;
		if (nextToken.empty() || nextToken[0] == '#') {
			continue;
		}
		if (nextToken == "PARAMETER_END") {
			break;
		}
		
		nread ++;
		simulation->addParameter(nextToken);
	}
	assert(nread == numParameters);
}

void FVSolver::loadSerialScanParameters(SimulationExpression* simulation, istream& ifsInput, int numSerialScanParameters) {
	if (simulation == nullptr) {
		throw runtime_error("Simulation has to be initialized before loading serial scan parameters");
	}

	string nextToken, line;
	int nread = 0;
	while (!ifsInput.eof()) {
		getline(ifsInput, line);
		istringstream lineInput(line);

		lineInput >> nextToken;
		if (nextToken.empty() || nextToken[0] == '#') {
			continue;
		}
		if (nextToken == "SERIAL_SCAN_PARAMETER_END") {
			break;
		}
		
		nread ++;
		simulation->addParameter(nextToken);
	}
	assert(nread == numSerialScanParameters);
}

/*
# Parameter Scan Values
PARAMETER_SCAN_BEGIN 2
1.0 
2.0 
PARAMETER_SCAN_END
*/
void FVSolver::loadSerialScanParameterValues(SimTool* simTool, SimulationExpression* simulation, istream& ifsInput, int numSerialScanParameterValues) {
	if (simulation == nullptr) {
		throw runtime_error("Simulation has to be initialized before loading serial scan parameter values");
	}
	string nextToken, line;

	int numSerialScanParameters = simulation->getNumParameters();
	auto** serialScanParameterValues = new double*[numSerialScanParameterValues];

	for (int i = 0; i < numSerialScanParameterValues; i ++) {
		serialScanParameterValues[i] = new double[numSerialScanParameters];
	
		getline(ifsInput, line);
		istringstream lineInput(line);

		for (int j = 0; j < numSerialScanParameters; j ++) {
			lineInput >> serialScanParameterValues[i][j];
		}
	}

	while (!ifsInput.eof()) {
		getline(ifsInput, line);
		istringstream lineInput(line);

		lineInput >> nextToken;

		if (nextToken.empty() || nextToken[0] == '#') {
			continue;
		}
		if (nextToken == "SERIAL_SCAN_PARAMETER_VALUE_END") {
			break;
		}
	}
	simTool->setSerialParameterScans(numSerialScanParameterValues, serialScanParameterValues);
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
SimTool* FVSolver::createSimTool(istream& ifsInput, istream& vcgInput, int taskID, bool bSimZip)
{
	SimTool* simTool = new SimTool();

	if (taskID < 0) { // no messaging
		SimulationMessaging::create();
	}

	string meshfile;
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
			loadJMSInfo(ifsInput, taskID);
#ifdef USE_MESSAGING
			SimulationMessaging::getInstVar()->start(); // start the thread
#endif
			SimulationMessaging::getInstVar()->setWorkerEvent(new WorkerEvent(JOB_STARTING, "preprocessing started"));

		} else if (nextToken == "SIMULATION_PARAM_BEGIN") {
			loadSimulationParameters(simTool, ifsInput);
			simTool->checkTaskIdLockFile();//check if taskid compatible with taskid lockfile
		} else if (nextToken == "POST_PROCESSING_BLOCK_BEGIN") {
			loadPostProcessingBlock(simTool, ifsInput);
		} else if (nextToken == "MODEL_BEGIN") {
			VCellModel* model = loadModel(ifsInput);
			if (model == nullptr) {
				throw runtime_error("Model has 0 features");
			}
			simTool->setModel(model);
		} else if (nextToken == "MESH_BEGIN") {
			// consume lines until MESH_END
			while (!ifsInput.eof())
			{
				getline(ifsInput, line);
				istringstream lineInput(line);

				nextToken = "";
				lineInput >> nextToken;
				if (nextToken.size() == 0 || nextToken[0] == '#') {
					continue;
				}
				if (nextToken == "MESH_END") {
					break;
				}
			}
			loadMeshFromVcg(simTool->getModel(), vcgInput);
		} else if (nextToken == "VARIABLE_BEGIN") {
			SimulationExpression* simulation = loadSimulation(simTool, simTool->getModel(), ifsInput);
			simTool->setSimulation(simulation);
		} else if (nextToken == "SMOLDYN_BEGIN") {
			loadSmoldyn(simTool, ifsInput);
		} else if (nextToken == "PARAMETER_BEGIN") {
			int numParams = 0;
			lineInput >> numParams;
			loadParameters(simTool->getSimulation(), ifsInput, numParams);
		} else if (nextToken == "SERIAL_SCAN_PARAMETER_BEGIN") {
			int numSerialScanParams = 0;
			lineInput >> numSerialScanParams;
			loadSerialScanParameters(simTool->getSimulation(), ifsInput, numSerialScanParams);
		} else if (nextToken == "SERIAL_SCAN_PARAMETER_VALUE_BEGIN") {
			int numSerialScanParamValues = 0;
			lineInput >> numSerialScanParamValues;
			loadSerialScanParameterValues(simTool, simTool->getSimulation(), ifsInput, numSerialScanParamValues);
		} else if (nextToken == "FIELD_DATA_BEGIN") {
			loadFieldData(simTool->getSimulation(), ifsInput);
		} else if (nextToken == "COMPARTMENT_BEGIN") {
			string feature_name;
			lineInput >> feature_name;
			Feature* feature = simTool->getModel()->getFeatureFromName(feature_name);
			if (feature != nullptr) {
				loadFeature(simTool->getSimulation(), ifsInput, feature);
			} else {
				throw runtime_error("createSimTool(), Invalid compartment when loading feature!");
			}
		} else if (nextToken == "MEMBRANE_BEGIN") {
			string mem_name, feature1_name, feature2_name;
			lineInput >> mem_name >> feature1_name >> feature2_name;
			Membrane* membrane = simTool->getModel()->getMembraneFromName(mem_name);
			if (membrane != nullptr) {
				loadMembrane(simTool->getSimulation(), simTool->getModel(), ifsInput, membrane);
			} else {
				throw runtime_error("createSimTool(), Invalid compartment when loading membrane!");
			}
		} else {
			throw runtime_error("createSimTool(), encountered unknown token " + nextToken);
		}
	}
	if (!bSimZip) {
		simTool->requestNoZip();
	}
	return simTool;
}

void FVSolver::loadSmoldyn(SimTool* sim_tool, istream& ifsInput) {
	if (sim_tool->getSimulation() == nullptr) {
		throw runtime_error("Simulation has to be initialized before loading smoldyn");
	}

	string nextToken, line;
	int nread = 0;
	while (!ifsInput.eof()) {
		getline(ifsInput, line);
		istringstream lineInput(line);

		lineInput >> nextToken;
		if (nextToken.empty() || nextToken[0] == '#') {
			continue;
		}
		if (nextToken == "SMOLDYN_END") {
			break;
		}
		if (nextToken == "SMOLDYN_INPUT_FILE") {
			string inputfile;
			getline(lineInput, inputfile);
			trimString(inputfile);
			sim_tool->setSmoldynInputFile(inputfile);
		}
	}
}

FVSolver::FVSolver(const char* outdir) {
	// simTool = nullptr;
	// simulation = nullptr;
	// model = nullptr;
	mesh = nullptr;
	outputPath = outdir;
}

FVSolver::~FVSolver() {
	delete SimulationMessaging::getInstVar();
	// delete simulation;
	// delete model;
	delete mesh;
	// delete simTool;
}

void FVSolver::solve(SimTool* simTool, bool bLoadFinal, double* paramValues)
{
	if (paramValues != nullptr) {
		simTool->getSimulation()->setParameterValues(simTool, paramValues);
	}
	simTool->setLoadFinal(bLoadFinal);

	SimulationMessaging::getInstVar()->setWorkerEvent(new WorkerEvent(JOB_STARTING, "preprocessing finished"));

	simTool->start();
}

void FVSolver::init(SimTool* sim_tool, double* paramValues){
	// setting initial conditions.
	sim_tool->getSimulation()->setParameterValues(sim_tool, paramValues);
	sim_tool->getSimulation()->initSimulation(sim_tool);
}

void FVSolver::step(SimTool* sim_tool, double* paramValues)
{
	//simulation->setParameterValues(paramValues);
	sim_tool->getSimulation()->iterate(sim_tool);
	sim_tool->getSimulation()->update(sim_tool);
}

string FVSolver::getVariableName(const SimTool* sim_tool, int index){
	return sim_tool->getSimulation()->getVariable(index)->getName();
}

int FVSolver::getNumVariables(const SimTool* sim_tool){
	return sim_tool->getSimulation()->getNumVariables();
}

double* FVSolver::getValue(const SimTool* sim_tool, string& var, int arrayID) {
	if (arrayID==0){
		return sim_tool->getSimulation()->getVariableFromName(var)->getOld();
	} else if (arrayID==1){
		return sim_tool->getSimulation()->getVariableFromName(var)->getCurr();
	} else {
		throw runtime_error("arrayID out of bounds");
	}
}

int FVSolver::getVariableLength(const SimTool* sim_tool, string& var) {
	return sim_tool->getSimulation()->getVariableFromName(var)->getSize();
}

void FVSolver::setInitialCondition(SimTool* sim_tool, string& varName, int dataLength, const double* data) {
	Variable* var = sim_tool->getSimulation()->getVariableFromName(varName);
	if (var == nullptr) {
		throw runtime_error("FVSolver::setInitialCondition() : variable " + varName + " doesn't exist");
	}
	if (var->getSize() != dataLength) {
		throw runtime_error("FVSolver::setInitialCondition() : variable " + varName + " doesn't match in size, " + to_string(var->getSize()) + ", " + to_string(dataLength));
	}
	memcpy(var->getCurr(), data, dataLength * sizeof(double));
	var->update();
}

double FVSolver::getCurrentTime(SimTool* sim_tool){
	return sim_tool->getSimulation()->getTime_sec(sim_tool);
}

void FVSolver::setEndTime(SimTool* sim_tool, double endTime){
	sim_tool->setEndTimeSec(endTime);
}

/**
# Post Processing Block
POST_PROCESSING_BLOCK_BEGIN
PROJECTION_DATA_GENERATOR postDex cell x sum (5.0 * Dex_cell);
POST_PROCESSING_BLOCK_END
*/
void FVSolver::loadPostProcessingBlock(SimTool* sim_tool, istream& ifsInput){
	if (sim_tool->getSimulation() == nullptr) {
		throw runtime_error("Simulation has to be initialized before loading field data");
	}

	// create post processing block;
	sim_tool->getSimulation()->createPostProcessingBlock();
	PostProcessingBlock* postProcessingBlock = sim_tool->getSimulation()->getPostProcessingBlock();
	// add var statistics data generator always
	postProcessingBlock->addDataGenerator(new VariableStatisticsDataGenerator());
	
	string nextToken, line, storeEnabledStr, fieldName, strNotUsed;
	while (!ifsInput.eof()) {
		getline(ifsInput, line);
		istringstream lineInput(line);

		nextToken = "";
		lineInput >> nextToken;
		if (nextToken.empty() || nextToken[0] == '#') {
			continue;
		}
		if (nextToken == "POST_PROCESSING_BLOCK_END") {
			break;
		}

		if (nextToken == "PROJECTION_DATA_GENERATOR") {
			string name, domain_name, op, axis;
			lineInput >> name >> domain_name >> axis >> op; 
			Feature* feature = sim_tool->getModel()->getFeatureFromName(domain_name);
			Expression* function = readExpression(lineInput, name);
			auto* dataGenerator = new ProjectionDataGenerator(name, feature, axis, op, function);
			postProcessingBlock->addDataGenerator(dataGenerator);
		} else if (nextToken == "GAUSSIAN_CONVOLUTION_DATA_GENERATOR") {
			string name, domain_name;
			double sigmaXY, sigmaZ;
			lineInput >> name >> domain_name >> sigmaXY >> sigmaZ;
			Feature* feature = sim_tool->getModel()->getFeatureFromName(domain_name);
			string volFunctionKeyword = "GAUSSIAN_CONVOLUTION_VOL_FUNCTION";
			string memFunctionKeyword = "GAUSSIAN_CONVOLUTION_MEM_FUNCTION";
			size_t indexOfVolumeFunctionKeyword = line.find(volFunctionKeyword);
			size_t indexOfMembraneFunctionKeyword = line.find(memFunctionKeyword);

			size_t indexOfVolumeExp = indexOfVolumeFunctionKeyword+volFunctionKeyword.size();
			string volumeFunctionString = line.substr(indexOfVolumeExp, indexOfMembraneFunctionKeyword-indexOfVolumeExp);
			trimString(volumeFunctionString);
			auto* volumeFunction = new Expression(volumeFunctionString);

			size_t indexOfMembraneExp = indexOfMembraneFunctionKeyword+memFunctionKeyword.size();
			string membraneFunctionString = line.substr(indexOfMembraneExp, string::npos);
			trimString(membraneFunctionString);
			auto* membraneFunction = new Expression(membraneFunctionString);
			auto* dataGenerator = new GaussianConvolutionDataGenerator(name, feature, sigmaXY, sigmaZ, volumeFunction, membraneFunction);
			postProcessingBlock->addDataGenerator(dataGenerator);

			string garbage;
			getline(lineInput, garbage);

		} else if (nextToken == "ROI_DATA_GENERATOR_BEGIN") {
			int* volumePoints = nullptr;
			int* membranePoints = nullptr;
			int numVolumePoints = 0;
			int numMembranePoints = 0;
			FieldData* sampleImage;
			int numImageRegions = 0;
			int zSlice = 0;
			string roiDGName;
			
			lineInput >> roiDGName; //read ROI data generator name
			//read other info. for ROI data generator
			while (!ifsInput.eof()) {
				getline(ifsInput, line);
				istringstream lineInput_roi(line);

				nextToken = "";
				lineInput_roi >> nextToken;
				if (nextToken.empty() || nextToken[0] == '#') {
					continue;
				}
				if (nextToken == "ROI_DATA_GENERATOR_END") {
					break;
				}
				if (nextToken == "VolumePoints") {
					lineInput_roi >> numVolumePoints;
					volumePoints = new int[numVolumePoints];
					memset(volumePoints, 0, numVolumePoints * sizeof(int));
					getline(ifsInput, line);
					istringstream vpStream(line);
					for (int i = 0; i < numVolumePoints; i ++) {
						vpStream >> volumePoints[i];
					}
				} else if (nextToken == "MembranePoints") {
					lineInput_roi >> numMembranePoints;
					membranePoints = new int[numMembranePoints];
					memset(membranePoints, 0, numMembranePoints * sizeof(int));
					for (int i = 0; i < numMembranePoints; i ++) {
						lineInput_roi >> membranePoints[i];
					}
				} else if (nextToken == "StoreEnabled") {
					lineInput_roi >> storeEnabledStr;
					if (storeEnabledStr == "false") {
						sim_tool->setStoreEnable(false);
					}
				} else if (nextToken == "SampleImage") {
					lineInput_roi >> numImageRegions >> zSlice;
					lineInput_roi >> strNotUsed;
					getline(lineInput_roi, fieldName);
					//loadSampleImage(simTool, vcdataID, varName, time);			
				} else if (nextToken == "SampleImageFile") {			
					string varName;
					double time;
					lineInput_roi >> varName >> time;
					string fieldfilename;
					getline(lineInput_roi, fieldfilename);
					trimString(fieldfilename);
					sampleImage = new FieldData(0, VAR_VOLUME, "", fieldName, varName, time, fieldfilename);
				}
			}// end while loop for roi data generator
			auto* dataGenerator = new RoiDataGenerator(roiDGName,  volumePoints, numVolumePoints, membranePoints, numMembranePoints, sampleImage, numImageRegions, zSlice);
			postProcessingBlock->addDataGenerator(dataGenerator);
		} else {
			throw runtime_error("loadPostProcessingBlock(), encountered unknown token " + nextToken);
		}
	}
}

//static void trimString(string& str)
//{
//	string::size_type pos = str.find_last_not_of(" \r\n");
//	if(pos != string::npos) {
//		str.erase(pos + 1);
//		pos = str.find_first_not_of(" \r\n");
//		if(pos != string::npos) {
//			str.erase(0, pos);
//		}
//	}
//	else {
//		str.erase(str.begin(), str.end());
//	}
//}
