#include "VCellIDASolver.h"
#include "OdeResultSet.h"
#include <Expression.h>
#include <SimpleSymbolTable.h>
#include <Exception.h>
#include "StoppedByUserException.h"
#include <DivideByZeroException.h>
#include <FunctionDomainException.h>
#include <FunctionRangeException.h>
#include <sys/timeb.h>
#include <sstream>
#include <cassert>
#include <string>
#include <ida/ida.h>
#include <ida/ida_dense.h>
//#include <ida/ida_spgmr.h>
#include <nvector/nvector_serial.h>
#include <memory.h>
#include <format>
#ifdef USE_MESSAGING
#include <VCELL/SimulationMessaging.h>
#endif

/**
  * calling sequence
  ********************
initIDA
	reInit(start_time)
		IDACreate
		IDAMalloc
		IDASetRdata
		IDASetMaxStep
		IDADense
		IDASetMaxNumSteps
		IDASetId
		IDACalcIC
		IDAGetConsistentIC
	solveInitialDiscontinuities(start_time)
		updateTandVariableValues
		look for 0 in root function
		if (find root)			
			repeat fixInitialDiscontinuities(start_time) until no changes in discontinuity values
				IDARootInit // disable root finding
				save y
				IDASetStopTime // small time
				IDASolve
				updateTandVariableValues
				update Discontinuity values
				update values
				restore y, yp=0
				reInit(start_time);
					IDAReInit
					IDACalcIC
					IDAGetConsistentIC
				IDARootInit // enable root finding
		end if
	IDARootInit
idaSolve
	while (time) {
		IDASetStopTime
		IDASolve
		if (root return) {
			IDAGetRootInfo
			updateDiscontinuities on root return
				updateTandVariableValues
				invert Discontinuity values as needed
				update values
			reInit(time)
				IDAReInit
				IDACalcIC
				IDAGetConsistentIC
			solveInitialDiscontinuities(time)
				updateTandVariableValues
				look for 0 in root function
				if (find root)	
					repeat fixInitialDiscontinuities(time) until no changes in discontinuity values
						IDARootInit // disable root finding
						save y
						IDASetStopTime // small time
						IDASolve
						updateTandVariableValues
						update Discontinuity values
						update values
						restore y, yp=0
						reInit(time);
							IDAReInit
							IDACalcIC
							IDAGetConsistentIC
						IDARootInit // enable root finding
				endif

			for loop
				execute events
				updateDiscontinuities on event triggers
				reInit(time)
				solveInitialDiscontinuities(time)
			end for
		} else {
			for loop
				execute events
				updateDiscontinuities on event triggers
				reInit(time)
				solveInitialDiscontinuities(time)
			end for
			checkDiscontinuityConsistency
		}
	}
 ******************	
**/

void VCellIDASolver::throwIDAErrorMessage(int returnCode) {
	switch (returnCode) {
		case IDA_SUCCESS: {
			throw "IDA_SUCCESS: IDASolve succeeded and no roots were found";
		}
		case IDA_ROOT_RETURN:  {
			throw "IDA_ROOT_RETURN: IDASolve succeeded, and found one or more roots. If nrtfn > 1, call IDAGetRootInfo to see which g_i were found to have a root at (*tret).";
		}
		case IDA_TSTOP_RETURN: {
			throw "IDA_TSTOP_RETURN: IDASolve returns computed results for the independent variable value tstop. That is, tstop was reached.";
		}
		case IDA_MEM_NULL: {
			throw "IDA_MEM_NULL: The IDA_mem argument was NULL";
		}
		case IDA_ILL_INPUT: {
			throw "IDA_ILL_INPUT: One of the inputs to IDASolve is illegal";
		}
		case IDA_NO_MALLOC: {
			throw "IDA_NO_MALLOC: The allocation function IDAMalloc has not been called.";
		}
		case IDA_TOO_MUCH_WORK: {
			throw "IDA_TOO_MUCH_WORK: The solver took mxstep internal steps but could not reach tout.\n\nTry reducing maximum time step.";
		}
		case IDA_TOO_MUCH_ACC: {
			throw "IDA_TOO_MUCH_ACC: The solver could not satisfy the accuracy demanded by the user for some internal step.";
		}
		case IDA_ERR_FAIL: {
			throw "IDA_ERR_FAIL: Error test failures occurred too many times (=MXETF = 10) during one internal step";
		}
		case IDA_CONV_FAIL: {
			throw "IDA_CONV_FAIL: Convergence test failures occurred too many times (= MXNCF = 10) during one internal step.";
		}
		case IDA_LINIT_FAIL: {
			throw "IDA_LINIT_FAIL: The linear solver's initialization function failed.";
		}
		case IDA_LSETUP_FAIL:{
			throw "IDA_LSETUP_FAIL: The linear solver's setup routine failed in an unrecoverable manner.";
		}
		case IDA_LSOLVE_FAIL:{
			throw "IDA_LSOLVE_FAIL: The linear solver's solve routine failed in an unrecoverable manner.";
		}
		case IDA_RES_FAIL:{
			throw "IDA_RES_FAIL: The user's residual function returned a nonrecoverable error flag.";
		}
		case IDA_CONSTR_FAIL:{
			throw "IDA_CONSTR_FAIL: The inequality constraints were violated, and the solver was unable to recover.";
		}
		case IDA_REP_RES_ERR:{
			std::stringstream ss;
			ss << "IDA_REP_RES_ERR: The user's residual function repeatedly returned a recoverable error flag, but the solver was unable to recover. " << recoverableErrMsg;
			throw ss.str();
		}
		case IDA_MEM_FAIL:{
			throw "IDA_MEM_FAIL: A memory allocation request has failed.";
		}
		case IDA_BAD_T:{
			throw "IDA_BAD_T: ";
		}
		case IDA_BAD_EWT:{
			throw "IDA_BAD_EWT: Some component of the error weight vector is zero (illegal).";
		}
		case IDA_FIRST_RES_FAIL:{
			std::stringstream ss;
			ss << "IDA_FIRST_RES_FAIL: The user's residual function returned a recoverable error flag on the first call, but IDA was unable to recover. " << recoverableErrMsg;
			throw ss.str();
		}
		case IDA_LINESEARCH_FAIL:{
			throw "IDA_LINESEARCH_FAIL: The linesearch algorithm failed to find a solution with a step larger than steptol in weighted RMS norm.";
		}
		case IDA_NO_RECOVERY:{
			throw "IDA_NO_RECOVERY: The user's residual function, or the linear solver's setup or solve function had a recoverable error, but IDA was unable to recover.";
		}
		case IDA_RTFUNC_FAIL:{
			throw "IDA_RTFUNC_FAIL: The rootfinding function failed.";
		}
		default:
			throw IDAGetReturnFlagName(returnCode);
	}
}

void VCellIDASolver::checkIDAFlag(int flag) {
	if (flag != IDA_SUCCESS){
		throwIDAErrorMessage(flag);
	}
}

VCellIDASolver::VCellIDASolver() : VCellSundialsSolver() {
	rhsExpressions = 0;

	numDifferential = 0;
	numAlgebraic = 0;
	transformMatrix = 0;
	inverseTransformMatrix = 0;
	yp = 0;
	id = 0;
}

VCellIDASolver::~VCellIDASolver() {
	IDAFree(&solver);

	N_VDestroy_Serial(yp);
	N_VDestroy_Serial(id);	

	for (int i = 0; i < NUM_EQUATIONS; i ++) {
		delete rhsExpressions[i];
		delete[] transformMatrix[i];
		delete[] inverseTransformMatrix[i];
	}
	delete[] rhsExpressions;
	delete[] transformMatrix;
	delete[] inverseTransformMatrix;	
}

//  Residual call back
int VCellIDASolver::Residual_callback(realtype t, N_Vector y, N_Vector yp, N_Vector residual, void *rdata) {
	VCellIDASolver* solver = (VCellIDASolver*)rdata;
	return solver->Residual(t, y, yp, residual);
}

//  Residual
//
// assume that rate rhsExpressions have been substituted so that they are only functions of state variables
// and that the massMatrix is constant coefficient.
//
// note: we should return 0 for success and 1 for recoverable failure (e.g. y or yp is Inf or NaN)
//
int VCellIDASolver::Residual(realtype t, N_Vector y, N_Vector yp, N_Vector residual) {
	
	try {
		updateTandVariableValues(t, y);
		for (int i = 0; i < numDifferential; i ++) {				
			NV_Ith_S(residual, i) = rhsExpressions[i]->evaluateVector(values) - NV_Ith_S(yp, i);
		}
		for (int i = numDifferential; i < numDifferential + numAlgebraic; i ++) {
			NV_Ith_S(residual, i) = rhsExpressions[i]->evaluateVector(values);
		}
		recoverableErrMsg = "";
		return 0;
	}catch (DivideByZeroException e){
		std::cout << "failed to evaluate residual: " << e.getMessage() << std::endl;
		recoverableErrMsg = e.getMessage();
		return 1;
	}catch (FunctionDomainException e){
		std::cout << "failed to evaluate residual: " << e.getMessage() << std::endl;
		recoverableErrMsg = e.getMessage();
		return 1;
	}catch (FunctionRangeException e){
		std::cout << "failed to evaluate residual: " << e.getMessage() << std::endl;
		recoverableErrMsg = e.getMessage();
		return 1;
	}
}

int VCellIDASolver::RootFn_callback(realtype t, N_Vector y, N_Vector yp, realtype *gout, void *g_data) {
	VCellIDASolver* solver = (VCellIDASolver*)g_data;
	return solver->RootFn(t, y, /*yp, */gout);
}

/**----------------------------------------------------
Input format: 
	STARTING_TIME 0.0
	ENDING_TIME 0.1
	RELATIVE_TOLERANCE 1.0E-9
	ABSOLUTE_TOLERANCE 1.0E-9
	MAX_TIME_STEP 1.0
	KEEP_EVERY 1
	DISCONTINUITIES 1
	D_B0 (t > 0.0432); (-0.0432 + t);
	NUM_EQUATIONS 2
	VAR x_i INIT (0.0);
	VAR x_o INIT (0.8);
	TRANSFORM
	3.322259136212625E-4 0.0 
	0.0 1.0 
	INVERSETRANSFORM
	3010.0 0.0 
	0.0 1.0 
	RHS DIFFERENTIAL 1 ALGEBRAIC 1
	(3.322259136212625E-4 * ((20.0 * x_o * D_B0) - (50.0 * x_i)));
	((1505000.0 * (3.3222591362126253E-4 - (3.322259136212625E-4 * x_i) - (3.322259136212625E-4 * x_o))) - (100.0 * x_o));
--------------------------------------------------------------*/

void VCellIDASolver::readEquations(VCellSolverInputBreakdown& inputBreakdown) {
	try {
		for (int i = 0; i < NUM_EQUATIONS; i ++) {
			variableNames[i] = inputBreakdown.modelSettings.VARIABLE_NAMES[i];
			try {
				initialConditionExpressions[i] = new Expression(inputBreakdown.modelSettings.INITIAL_CONDITION_EXPRESSIONS[i]);
			} catch (VCell::Exception& ex) {
				throw VCell::Exception(std::string("Initial condition expression for [") + variableNames[i] + "] " + ex.getMessage());
			}
		}
		transformMatrix = new double*[NUM_EQUATIONS];
		for (int i = 0; i < NUM_EQUATIONS; i ++) {
			transformMatrix[i] = new double[NUM_EQUATIONS];
			for (int j = 0; j < NUM_EQUATIONS; j ++) {
				transformMatrix[i][j] = std::stod(inputBreakdown.modelSettings.FLAT_TRANSFORM_MATRIX[i * NUM_EQUATIONS + j]);
			}
		}

		inverseTransformMatrix = new double*[NUM_EQUATIONS];
		for (int i = 0; i < NUM_EQUATIONS; i ++) {
			inverseTransformMatrix[i] = new double[NUM_EQUATIONS];
			for (int j = 0; j < NUM_EQUATIONS; j ++) {
				inverseTransformMatrix[i][j] = std::stod(inputBreakdown.modelSettings.FLAT_INVERSE_TRANSFORM_MATRIX[i * NUM_EQUATIONS + j]);
			}
		}
		numDifferential = inputBreakdown.modelSettings.NUM_DIFFERENTIAL;
		numAlgebraic = inputBreakdown.modelSettings.NUM_ALGEBRAIC;
		rhsExpressions = new Expression*[NUM_EQUATIONS];
		for (int i = 0; i < NUM_EQUATIONS; i ++) {
			try {
				rhsExpressions[i] = new Expression(inputBreakdown.modelSettings.RATE_EXPRESSIONS[i]);
			} catch (VCell::Exception& ex) {
				std::stringstream ss;
				ss << "RHS[" << i << "] " << ex.getMessage();
				throw VCell::Exception(ss.str());
			}
		}
	} catch (const char* ex) {
		throw VCell::Exception(std::string("VCellIDASolver::readInput() : ") + ex);
	} catch (VCell::Exception& ex) {
		throw VCell::Exception(std::string("VCellIDASolver::readInput() : ") + ex.getMessage());
	} catch (...) {
		throw VCell::Exception("VCellIDASolver::readInput() caught unknown exception");
	}
}

void VCellIDASolver::initialize() {
	VCellSundialsSolver::initialize();

	for (int i = 0; i < NUM_EQUATIONS; i ++) {
		rhsExpressions[i]->bindExpression(defaultSymbolTable);
	}

	yp = N_VNew_Serial(NUM_EQUATIONS);
	id = N_VNew_Serial(NUM_EQUATIONS);

	if (yp == 0 || id == 0) {
		throw "Out of Memory";
	}		
}

void VCellIDASolver::solve(double* paramValues, bool bPrintProgress, FILE* outputFile, void (*checkStopRequested)(double, long)) {

	if (checkStopRequested != 0) {
		checkStopRequested(STARTING_TIME, 0);
	}

	writeFileHeader(outputFile);

	// clear data in result set before solving
	odeResultSet->clearData();

	// copy parameter values to the end of values, these will stay the same during solving
	// values[0] is time, y values will be copied to 1~NEQ of values in residual function
	memset(values, 0, (NUM_EQUATIONS + 1 + NUM_PARAMETERS) * sizeof(double));
	memcpy(values + NUM_EQUATIONS + 1, paramValues, NUM_PARAMETERS * sizeof(double));
	memset(values + 1 + NUM_EQUATIONS + NUM_PARAMETERS, 0, numDiscontinuities * sizeof(double));

	initIDA(paramValues);
	idaSolve(bPrintProgress, outputFile, checkStopRequested);	
}

#define DEBUG_PRINT \
	for (int i = 0; i < NEQ; i ++) {\
		cout << "y[" << i << "] = " << NV_Ith_S(y,i) << ",  yp[" << i << "] = " << NV_Ith_S(yp,i) << endl; \
	}

void VCellIDASolver::initIDA(double* paramValues) {
	// compute initial condition
	double* initCond = new double[NUM_EQUATIONS];
	for (int i = 0; i < NUM_EQUATIONS; i ++) {
		initCond[i] = initialConditionExpressions[i]->evaluateVector(paramValues);
	}

	// must initialize y and yp before call IDAMalloc
	// Initialize y, yp and id. transform initial condition to y
	for (int i = 0; i < NUM_EQUATIONS; i ++) {
		NV_Ith_S(id, i) = i < numDifferential ? RCONST(1) : RCONST(0);
		NV_Ith_S(yp, i) = 0; // Initialize yp  to be 0, they will be reinitialized later.
		NV_Ith_S(y, i) = 0;		
		for (int j = 0; j < NUM_EQUATIONS; j ++) {
			NV_Ith_S(y, i) += transformMatrix[i][j] * initCond[j];
		}
	}
	delete[] initCond;

	if (numDiscontinuities > 0) {
		initDiscontinuities();
	}

	reInit(STARTING_TIME);

	if (numDiscontinuities > 0) {
		solveInitialDiscontinuities(STARTING_TIME);
		int flag = IDARootInit(solver, 2*numDiscontinuities, RootFn_callback, this);
		checkIDAFlag(flag);
	}
	executeEvents(STARTING_TIME);
}

void VCellIDASolver::reInit(double t) {
	int flag = 0;
	int ToleranceType = IDA_SS;
	if (solver == 0) {
		solver = IDACreate();
		if (solver == 0) {
			throw "Out of memory";
		}
		flag = IDAMalloc(solver, Residual_callback, t, y, yp, ToleranceType, RelativeTolerance, &AbsoluteTolerance);
		checkIDAFlag(flag);

		// set non-default solver options (in this case the "this" pointer to include in callbacks)
		//IDASetErrHandlerFn(ida_mem, IDAErrHandlerFn ehfun, eh_data);
		flag = IDASetRdata(solver, this);
		checkIDAFlag(flag);
		flag = IDASetMaxStep(solver, maxTimeStep);
		checkIDAFlag(flag);

		// choose the linear solver (Dense "direct" matrix LU decomposition solver).
		flag = IDADense(solver, NUM_EQUATIONS);
		//flag = IDASpgmr(solver, 0);
		checkIDAFlag(flag);

		IDASetMaxNumSteps(solver, 5000);

		flag = IDASetId(solver, id); // used with IDACalcIC( IDA_YA_YDP_INIT ).
	}  else {
		flag = IDAReInit(solver, Residual_callback, t, y, yp, ToleranceType, RelativeTolerance, &AbsoluteTolerance);
	}
	checkIDAFlag(flag);

	realtype tout1 = std::min(ENDING_TIME, t + 2 * maxTimeStep + (1e-15));
	
	flag = IDACalcIC(solver, IDA_YA_YDP_INIT, tout1);
	checkIDAFlag(flag);
	flag = IDAGetConsistentIC(solver, y, yp);
	checkIDAFlag(flag);
}

bool VCellIDASolver::fixInitialDiscontinuities(double t) {

	// disable root finding to fix discontinuities after roots found
	int flag = IDARootInit(solver, 0, RootFn_callback, this);
	checkIDAFlag(flag);

	double* oldy = new double[NUM_EQUATIONS];
	memcpy(oldy, NV_DATA_S(y), NUM_EQUATIONS * sizeof(realtype));

	double epsilon = std::max(1e-15, ENDING_TIME * 1e-8);
	double currentTime = t;
	realtype tout = currentTime + epsilon;
	IDASetStopTime(solver, tout);
	int returnCode = IDASolve(solver, tout, &currentTime, y, yp, IDA_ONE_STEP_TSTOP);
	if (returnCode != IDA_TSTOP_RETURN && returnCode != IDA_SUCCESS) {
		throwIDAErrorMessage(returnCode);
	}

	updateTandVariableValues(currentTime, y);
	bool bInitChanged = false;
	for (int i = 0; i < numDiscontinuities; i ++) {
		double v = odeDiscontinuities[i]->discontinuityExpression->evaluateVector(values);
		if (v != discontinuityValues[i]) {
			std::cout << "update discontinuities at time " << t << " : " << odeDiscontinuities[i]->discontinuityExpression->infix() << " " << discontinuityValues[i] << " " << v << std::endl;
			discontinuityValues[i] = v;			
			bInitChanged = true;
		}
	}
	if (bInitChanged) {
		memcpy(values + 1 + NUM_EQUATIONS + NUM_PARAMETERS, discontinuityValues, numDiscontinuities * sizeof(double));
	}

	//revert y and yp
	memcpy(NV_DATA_S(y), oldy, NUM_EQUATIONS * sizeof(realtype));
	memset(NV_DATA_S(yp), 0, NUM_EQUATIONS * sizeof(realtype));
	reInit(t);

	flag = IDARootInit(solver, 2*numDiscontinuities, RootFn_callback, this);
	checkIDAFlag(flag);

	delete[] oldy;

	return bInitChanged;
}

void VCellIDASolver::onIDAReturn(realtype Time, int returnCode) {
	if (returnCode == IDA_ROOT_RETURN) {
		// flip discontinuities				
		int flag = IDAGetRootInfo(solver, rootsFound);
		checkIDAFlag(flag);
		std::cout << std::endl << "idaSolve() : roots found at time " << Time << std::endl;
#ifdef SUNDIALS_DEBUG
		printVariableValues(Time);
#endif
		updateDiscontinuities(Time, true);
		reInit(Time); // recalculate y
		solveInitialDiscontinuities(Time); // recalculate booleans for discontinuities after discontinuity state change
		
		int updateCount = 0;
		for (updateCount = 0;  updateCount < MAX_NUM_EVENTS_DISCONTINUITIES_EVAL; updateCount ++) {
			bool bExecuted = executeEvents(Time);
			if (!bExecuted) {
				break;
			}
			updateDiscontinuities(Time, false);
			reInit(Time); // recalculate y
			solveInitialDiscontinuities(Time); // recalculate booleans for discontinuities after discontinuity state change
		}
	} else {
		int updateCount = 0;
		for (updateCount = 0;  updateCount < MAX_NUM_EVENTS_DISCONTINUITIES_EVAL; updateCount ++) {			
			bool bExecuted = executeEvents(Time);
			if (!bExecuted) {
				break;
			}
			updateDiscontinuities(Time, false);
			reInit(Time); // recalculate y
			solveInitialDiscontinuities(Time); // recalculate booleans for discontinuities after discontinuity state change
		}
		checkDiscontinuityConsistency();
	}
}

void VCellIDASolver::idaSolve(bool bPrintProgress, FILE* outputFile, void (*checkStopRequested)(double, long)) {
	if (checkStopRequested != 0) {
		checkStopRequested(STARTING_TIME, 0);
	}

	realtype Time = STARTING_TIME;
	double lastPercentile=0.00;
	clock_t lastTime = clock(); // to control the output of progress, send progress every 2 seconds
	double increment = 0.01;
	long iterationCount=0;
	long outputCount = 0;

	// write initial conditions
	writeData(Time, outputFile);
	if (bPrintProgress) {
		printProgress(Time, lastPercentile, lastTime, increment, outputFile);
	}

	if (outputTimes.size() == 0) {
		while (Time < ENDING_TIME) {
			if (checkStopRequested != 0) {
				checkStopRequested(Time, iterationCount);
			}

			double tstop = std::min(ENDING_TIME, Time + 2 * maxTimeStep + (1e-15));
			tstop = std::min(tstop, getNextEventTime());

			IDASetStopTime(solver, tstop);
			int returnCode = IDASolve(solver, ENDING_TIME, &Time, y, yp, IDA_ONE_STEP_TSTOP);
			iterationCount++;

			// save data if return IDA_TSTOP_RETURN (meaning reached end of time or max time step 
			// before one normal step) or IDA_SUCCESS (meaning one normal step)
			if (returnCode == IDA_TSTOP_RETURN || returnCode == IDA_SUCCESS || returnCode == IDA_ROOT_RETURN) {
				onIDAReturn(Time, returnCode);

				if (returnCode == IDA_ROOT_RETURN || iterationCount % keepEvery == 0 || Time >= ENDING_TIME){
					outputCount ++;
					if (outputCount *(NUM_EQUATIONS + 1) * bytesPerSample > MaxFileSizeBytes){
						/* if more than one gigabyte, then fail */
						std::string problem = std::format("output exceeded {} bytes\n", MaxFileSizeBytes);
						throw VCell::Exception(problem);
					}
					writeData(Time, outputFile);
					if (bPrintProgress) {
						printProgress(Time, lastPercentile, lastTime, increment, outputFile);
					}
				}
			} else {
				throwIDAErrorMessage(returnCode);
			}
		} 
	} else {
		double sampleTime = 0.0;
		assert(outputTimes[0] > STARTING_TIME);
		while (Time < ENDING_TIME && outputCount < (int)outputTimes.size()) {
			if (checkStopRequested != 0) {
				checkStopRequested(Time, iterationCount);
			}

			sampleTime = outputTimes[outputCount];	
			while (Time < sampleTime) {
				if (checkStopRequested != 0) {
					checkStopRequested(Time, iterationCount);
				}

				double tstop = std::min(sampleTime, Time + 2 * maxTimeStep + (1e-15));
				tstop = std::min(tstop, getNextEventTime());

				IDASetStopTime(solver, tstop);
				int returnCode = IDASolve(solver, sampleTime, &Time, y, yp, IDA_NORMAL_TSTOP);
				iterationCount++;

				// if return IDA_SUCCESS, this is an intermediate result, continue without saving data.
				if (returnCode == IDA_TSTOP_RETURN || returnCode == IDA_SUCCESS || returnCode == IDA_ROOT_RETURN) {
					onIDAReturn(Time, returnCode);

					if (Time == sampleTime) {
						writeData(Time, outputFile);
						if (bPrintProgress) {
							printProgress(Time, lastPercentile, lastTime, increment, outputFile);
						}
						outputCount ++;
						break;
					}
				} else {
					throwIDAErrorMessage(returnCode);
				}
			}
		} 
	}
#ifdef USE_MESSAGING
	SimulationMessaging::getInstVar()->setWorkerEvent(JobEvent::JOB_COMPLETED, 1, Time);
#endif
}


// override updateTempRowData, since y values need to be transformed to the original variables.
void VCellIDASolver::updateTempRowData(double currTime) {
	tempRowData[0] = currTime; 
	for (int i = 0; i < NUM_EQUATIONS; i ++) {
		tempRowData[i + 1] = 0;
		for (int j = 0; j < NUM_EQUATIONS; j ++) {
			tempRowData[i + 1] += inverseTransformMatrix[i][j] * NV_Ith_S(y, j);
		}
	}
}

void VCellIDASolver::updateTandVariableValues(realtype t, N_Vector y) {
	values[0] = t;
	for (int i = 0; i < NUM_EQUATIONS; i ++) {
		values[i + 1] = 0;
		for (int j = 0; j < NUM_EQUATIONS; j ++) {
			values[i + 1] += inverseTransformMatrix[i][j] * NV_Ith_S(y, j);
		}
	}
}
