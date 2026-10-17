/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef SUNDIALSPDESOLVER_H
#define SUNDIALSPDESOLVER_H

#include <VCELL/Scheduler.h>
#include <nvector/nvector_serial.h>
#include <sundials/sundials_types.h>
#include <VCELL/SundialsSolverOptions.h>

class Variable;
class CartesianMesh;
class SparseMatrixPCG;
struct MembraneElement;
struct VolumeElement;
class SimulationExpression;
class VarContext;
class Feature;

class SundialsPdeScheduler : public Scheduler
{
public:
	SundialsPdeScheduler(SimulationExpression *sim, const SundialsSolverOptions& sso, int numDisTimes, double* disTimes, bool bDefaultOutput);
	~SundialsPdeScheduler() override;

	void iterate(SimTool* sim_tool) override;
	double getCurrentTime() const { return currentTime; }
	void setSimStartTime(double st) {
		currentTime = st;
	}

private:
	SimulationExpression* simulation;
	void* sundialsSolverMemory;	// the memory for solver

	N_Vector y;
	static int RHS_callback(realtype t, N_Vector y, N_Vector r, void *fdata);
	int CVodeRHS(double t, double* yinput, double* rhs);

	static int pcSetup_callback(realtype t, N_Vector y, N_Vector fy, booleantype jok, booleantype *jcurPtr, realtype gamma,
                   void *P_data, N_Vector tmp1, N_Vector tmp2, N_Vector tmp3);
	int pcSetup(realtype t, N_Vector y, N_Vector fy, booleantype jok, booleantype *jcurPtr, realtype gamma);
	/*
	Definition typedef int (*CVSpilsPrecSetupFn)(realtype t, N Vector y, N Vector fy,
												booleantype jok, booleantype *jcurPtr,
												realtype gamma, void *p_data,
												N_Vector tmp1, N_Vector tmp2,
												N_Vector tmp3);
	Purpose
			This function evaluates and/or preprocesses Jacobian-related data needed by the pre-
			conditioner.
	Arguments
			The arguments of a CVSpilsPrecSetupFn are as follows:
			t 		is the current value of the independent variable.
			y 		is the current value of the dependent variable vector, namely the predicted
					value of y(t).
			fy		is the current value of the vector f(t, y).
			jok		is an input flag indicating whether Jacobian-related data needs to be recom-
					puted. The jok argument provides for the re-use of Jacobian data in the pre-
					conditioner solve function. jok == FALSE means that Jacobian-related data
					must be recomputed from scratch. jok == TRUE means that Jacobian data, if
					saved from the previous call to this function, can be reused (with the current
					value of gamma). A call with jok == TRUE can only occur after a call with jok
					== FALSE.
			jcurPtr	is a pointer to an output integer flag which is to be set to TRUE if Jacobian
					data was recomputed or to FALSE if Jacobian data was not recomputed, but
					saved data was reused.
			gamma	is the scalar appearing in the Newton matrix M = I - gammaJ.
			p_data	is a pointer to user data, the same as that passed to CVSpilsSetPreconditioner.
			tmp1
			tmp2
			tmp3	are pointers to memory allocated for variables of type N Vector which can be
					used by CVSpilsPrecSetupFn as temporary storage or work space.
	Return value
			The value to be returned by the preconditioner setup function is flag indicating
			whether it was successful. This value should be 0 if successful, positive for a recov-
			erable error (in which case the step will be retried), negative for an unrecoverable error
			(in which case the integration is halted).
	Notes
			The operations performed by this function might include forming a crude approximate
			Jacobian, and performing an LU factorization on the resulting approximation to M = I - gammaJ.

			Each call to the preconditioner setup function is preceded by a call to the CVRhsFn user
			function with the same (t,y) arguments. Thus the preconditioner setup function can
			use any auxiliary data that is computed and saved during the evaluation of the ODE
			right hand side.

			This function is not called in advance of every call to the preconditioner solve function,
			but rather is called only as often as needed to achieve convergence in the Newton
			iteration.

			If the user's CVSpilsPrecSetupFn function uses difference quotient approximations, it
			may need to access quantities not in the call list. These include the current stepsize, the
			error weights, etc. To obtain these, use the CVodeGet* functions described in $5.5.7.1.
			The unit roundoff can be accessed as UNIT ROUNDOFF defined in sundials_types.h.
	*/
	static int pcSolve_callback(realtype t, N_Vector y, N_Vector fy, N_Vector r, N_Vector z, realtype gamma, realtype delta,
                  int lr, void *P_data, N_Vector tmp);
	int pcSolve(realtype t, N_Vector y, N_Vector fy, N_Vector r, N_Vector z, realtype gamma, realtype delta, int lr);
	/*
	Definition typedef int (*CVSpilsPrecSolveFn)(realtype t, N_Vector y, N_Vector fy,
												N_Vector r, N_Vector z,
												realtype gamma, realtype delta,
												int lr, void *p data, N_Vector tmp);
	Purpose
			This function solves the preconditioning system Pz = r.
	Arguments
			t		is the current value of the independent variable.
			y		is the current value of the dependent variable vector.
			fy		is the current value of the vector f(t; y).
			r		is the right-hand side vector of the linear system.
			z		is the output vector computed.
			gamma	is the scalar � appearing in the Newton matrix M = I - gammaJ.
			delta	is an input tolerance to be used if an iterative method is employed in the solu-
					tion. In that case, the residual vector Res = r - Pz of the system should be
					made less than delta in weighted l2 norm, i.e., pPi(Resi � ewti)2 < delta.
					To obtain the N Vector ewt, call CVodeGetErrWeights (see x5.5.7.1).
			lr		is an input �ag indicating whether the preconditioner solve function is to use
					the left preconditioner (lr = 1) or the right preconditioner (lr = 2);
			p_data	is a pointer to user data | the same as the p data parameter passed to the
					function CVSp*SetPreconditioner.
			tmp		is a pointer to memory allocated for a variable of type N Vector which can be
					used for work space.
	Return value
			The value to be returned by the preconditioner solve function is a �ag indicating whether
			it was successful. This value should be 0 if successful, positive for a recoverable error
			(in which case the step will be retried), negative for an unrecoverable error (in which
			case the integration is halted).
	*/

	VolumeElement* pVolumeElement;
	MembraneElement* pMembraneElement;
	void checkCVodeReturnCode(int returnCode);
	void printCVodeStats();

	int numSymbolsPerVolVar; // U, U_Feature1_membrane, U_Feature2_membrane, ...
	long numUnknowns;
	long numVolUnknowns;

	int* global2Local;
	int* local2Global;
	int* regionOffsets;
	int* regionSizes;
	int* regionDefinedVolVariableSizes; // the number of variables defined in each region
	int** regionDefinedVolVariableIndexes;
	bool* bRegionHasTimeDepdentVariables; // used to see if we have to update neighbor state values 
	bool* bRegionHasConstantCoefficients; // call variable operator if false.

	int* volVectorOffsets;
	int memVectorOffset;
	int volRegionVectorOffset;
	int memRegionVectorOffset;

	int volSymbolOffset;
	int memSymbolOffset;
	int volRegionSymbolOffset;
	int memRegionSymbolOffset;
	int regionSizeVariableSymbolOffset;
	int fieldDataSymbolOffset;
	int randomVariableSymbolOffset;
	int parameterSymbolOffset;

	CartesianMesh* mesh;
	int dimension;
	double VOLUME;
	int Nx, Nxy;

	double oneOverH[3];

	void setupOrderMaps();
	void applyVolumeOperatorOld(double t, double* yinput, double* rhs);
	void applyVolumeOperator(double t, double* yinput, double* rhs);
	void regionApplyVolumeOperatorConstant(int regionID, double t, double* yinput, double* rhs);
	void regionApplyVolumeOperatorVariable(int regionID, double t, double* yinput, double* rhs);
	void applyMembraneDiffusionReactionOperator(double t, double* yinput, double* rhs);
	void applyMembraneFluxOperator(double t, double* yinput, double* rhs);
	void applyVolumeRegionReactionOperator(double t, double* yinput, double* rhs);
	void applyMembraneRegionReactionOperator(double t, double* yinput, double* rhs);
	void initSundialsSolver(VCellModel* model);
	void solve(SimTool* sim_tool);

	double *statePointValues, **neighborStatePointValues;
	void updateVolumeStatePointValues(int volIndex, double t, double* yinput, double* values);
	void updateMembraneStatePointValues(MembraneElement& me, double t, double* yinput, double* values);
	void updateRegionStatePointValues(int regionID, double t, double* yinput, bool bVolumeRegion, double* values);

	void updateSolutions();

	double* discontinuityTimes;
	int numDiscontinuityTimes;
	int currDiscontinuityTimeCount;

	SundialsSolverOptions sundialsSolverOptions;
	SparseMatrixPCG* M; //
	double* pcg_workspace;
	long nsp; // size of ISP and RSP
	bool bPcReinit;

	void preallocateM();
	void buildM_Volume(double t, double* yinput, double gamma);
	void buildM_Membrane(double t, double* yinput, double gamma);

	int getVolumeElementVectorOffset(int volIndex, int regionID);
	int getMembraneElementVectorOffset(int meindex);
	int getVolumeRegionVectorOffset(int regionID);
	int getMembraneRegionVectorOffset(int regionID);

	bool bLUcomputed;
	double oldGamma;
	double currentTime;
	bool bSundialsOneStepOutput;

	bool bHasVariableDiffusionAdvection, bHasAdvection;

	double txyzValues[4];
	void dirichletPointSetup(int volIndex, Feature* feature, VarContext* varContext, int mask, int* volumeNeighbors, double& ypoint);
	double computeNeumannCondition(Feature* feature, VarContext* varContext, int mask, double* scaleSs);

	double* diffCoeffs;
	void precomputeDiffusionCoefficients();

	double* rhsGradients;
	bool bHasGradient;
};

#endif
