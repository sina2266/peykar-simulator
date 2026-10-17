#include "VCellCVodeSolver.h"
#include "OdeResultSet.h"
#include <Expression.h>
#include <Exception.h>
#include <cassert>
#include <DivideByZeroException.h>
#include <FunctionDomainException.h>
#include <FunctionRangeException.h>
#include <sys/timeb.h>
#include <sstream>
#include <string>
#include <memory.h>
#include <cvode/cvode.h>             /* prototypes for CVODE fcts. and consts. */
#include <nvector/nvector_serial.h>  /* serial N_Vector types, fcts., and macros */
#include <cvode/cvode_dense.h>       /* prototype for CVDense */
//#include <cvode/cvode_spgmr.h>       /* prototype for CVSPGMR */
#include <sundials/sundials_dense.h> /* definitions DenseMat DENSE_ELEM */
#include <sundials/sundials_types.h> /* definition of type realtype */
#ifdef USE_MESSAGING
#include <VCELL/SimulationMessaging.h>
#endif
#define ToleranceType CV_SS

/**
  * calling sequence
  ********************
initCVode
	reInit(start_time)
		CVodeCreate
		CVodeMalloc
		CVodeSetFdata
		CVDense
		CVodeSetMaxNumSteps
	solveInitialDiscontinuities(start_time)
		updateTandVariableValues
		look for 0 in root function
		if (find root)			
			repeat fixInitialDiscontinuities(start_time) until no changes
				save y
				CVodeSetStopTime // small time
				CVode
				updateTandVariableValues
				update Discontinuity values
				update values
				restore y
				reInit(start_time);
					CVodeReInit
		end if
	CVodeRootInit
cvodeSolve
	while (time) {
		CVodeSetStopTime
		CVode
		if (root return) {
			CVodeGetRootInfo
			updateDiscontinuities on root return
				updateTandVariableValues
				invert Discontinuity values as needed
				update values
			for loop
				executeEvents
				updateDiscontinuities on event triggers
			end for
			reInit(time)
				CVodeReInit			
		} else {
			for loop
				executeEvents
				updateDiscontinuities on triggers
			end for
			if events executed
				reInit(time)
			end if
			checkDiscontinuityConsistency
		}
	}
 *****************
**/
void VCellCVodeSolver::throwCVodeErrorMessage(int returnCode) {
	switch (returnCode){
		case CV_SUCCESS: {
			throw "CV_SUCCESS: CVode succeeded and no roots were found.";
		}						 
		case CV_ROOT_RETURN: {
			throw "CV_ROOT_RETURN: CVode succeeded, and found one or more roots. If nrtfn > 1, call CVodeGetRootInfo to see which g_i were found to have a root at (*tret).";
		}
		case CV_TSTOP_RETURN: {
			throw "CV_TSTOP_RETURN: CVode succeeded and returned at tstop.";
		}
		case CV_MEM_NULL:{
			throw "CV_MEM_NULL: mem argument was null";
		}
		case CV_ILL_INPUT:{
			throw "CV_ILL_INPUT: one of the inputs to CVode is illegal";
		}
		case CV_TOO_MUCH_WORK:{
			throw "CV_TOO_MUCH_WORK: took mxstep internal steps but could not reach tout.\n\nTry reducing maximum time step.";
		}
		case CV_TOO_MUCH_ACC:{
			throw "CV_TOO_MUCH_ACC: could not satisfy the accuracy demanded by the user for some internal step";
		}
		case CV_ERR_FAILURE:{
			throw "CV_ERR_FAILURE: error test failures occurred too many times during one internal step";
		}
		case CV_CONV_FAILURE:{
			throw "CV_CONV_FAILURE: convergence test failures occurred too many times during one internal step";
		}
		case CV_LINIT_FAIL:{
			throw "CV_LINIT_FAIL: the linear solver's initialization function failed.";
		}
		case CV_LSETUP_FAIL:{
			throw "CV_LSETUP_FAIL: the linear solver's setup routine failed in an unrecoverable manner.";
		}
		case CV_LSOLVE_FAIL:{
			throw "CV_LSOLVE_FAIL: the linear solver's solve routine failed in an unrecoverable manner";
		}
		case CV_REPTD_RHSFUNC_ERR: {
			std::stringstream ss;
			ss << "CV_REPTD_RHSFUNC_ERR: repeated recoverable right-hand side function errors : " << recoverableErrMsg;
			throw ss.str();
		}
		case CV_UNREC_RHSFUNC_ERR:{
			std::stringstream ss;
			ss << "CV_UNREC_RHSFUNC_ERR: the right-hand side failed in a recoverable manner, but no recovery is possible : " <<  recoverableErrMsg;
			throw ss.str();
		}
		case CV_FIRST_RHSFUNC_ERR: {
			std::stringstream ss;
			ss << "CV_FIRST_RHSFUNC_ERR: The right-hand side routine failed at the first call : " <<  recoverableErrMsg;
			throw ss.str();
		}
		default:
			throw CVodeGetReturnFlagName(returnCode);
	}	
}

void VCellCVodeSolver::checkCVodeFlag(int flag) {
	if (flag != CV_SUCCESS){
		throwCVodeErrorMessage(flag);
	}
}

VCellCVodeSolver::VCellCVodeSolver() : VCellSundialsSolver() {
	rateExpressions = 0;
}

VCellCVodeSolver::~VCellCVodeSolver() {
	CVodeFree(&solver);

	for (int i = 0; i < NUM_EQUATIONS; i ++) {
		delete rateExpressions[i];
	}
	delete[] rateExpressions;
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
	ODE x_i INIT 0.0;
		 RATE ((20.0 * x_o * D_B0) - (50.0 * x_i));
	ODE x_o INIT 0.0;
		 RATE ( - ((20.0 * x_o * D_B0) - (50.0 * x_i)) + (1505000.0 * (3.322259136212625E-4 - (3.322259136212625E-4 * x_o) - (3.322259136212625E-4 * x_i))) - (100.0 * x_o));
--------------------------------------------------------------*/

void VCellCVodeSolver::readEquations(VCellSolverInputBreakdown& inputBreakdown) {
	try {
		for (int i = 0; i < NUM_EQUATIONS; i ++) {
			variableNames[i] = inputBreakdown.modelSettings.VARIABLE_NAMES[i];
			try {
				initialConditionExpressions[i] = new Expression(inputBreakdown.modelSettings.INITIAL_CONDITION_EXPRESSIONS[i]);
			} catch (VCell::Exception& ex) {
				throw VCell::Exception(std::string("Initial condition expression for [") + variableNames[i] + "] " + ex.getMessage());
			}
		}

		rateExpressions = new Expression*[inputBreakdown.modelSettings.RATE_EXPRESSIONS.size()];
		for (int i = 0; i < inputBreakdown.modelSettings.RATE_EXPRESSIONS.size(); i++) {
			rateExpressions[i] = new Expression(inputBreakdown.modelSettings.RATE_EXPRESSIONS[i]);
		}
	} catch (char* ex) {
		throw VCell::Exception(std::string("VCellCVodeSolver::readInput() : ") + ex);
	} catch (VCell::Exception& ex) {
		throw VCell::Exception(std::string("VCellCVodeSolver::readInput() : ") + ex.getMessage());
	} catch (...) {
		throw "VCellCVodeSolver::readInput() : caught unknown exception";
	}
}

void VCellCVodeSolver::initialize() {
	VCellSundialsSolver::initialize();

	// rate can be function of variables, parameters and discontinuities.
	for (int i = 0; i < NUM_EQUATIONS; i ++) {
		rateExpressions[i]->bindExpression(defaultSymbolTable);
	}
}

int VCellCVodeSolver::RHS (realtype t, N_Vector y, N_Vector r) {	
	try {
		updateTandVariableValues(t, y);
		double* r_data = NV_DATA_S(r);
		for (int i = 0; i < NUM_EQUATIONS; i ++) {
			r_data[i] = rateExpressions[i]->evaluateVector(values);
		}
		recoverableErrMsg = "";
		return 0;
	}catch (DivideByZeroException& e){
		std::cout << "failed to evaluate residual: " << e.getMessage() << std::endl;
		recoverableErrMsg = e.getMessage();
		return 1;
	}catch (FunctionDomainException& e){
		std::cout << "failed to evaluate residual: " << e.getMessage() << std::endl;
		recoverableErrMsg = e.getMessage();
		return 1;
	}catch (FunctionRangeException& e){
		std::cout << "failed to evaluate residual: " << e.getMessage() << std::endl;
		recoverableErrMsg = e.getMessage();
		return 1;
	}
}

double VCellCVodeSolver::RHS (double* allValues, int equationIndex) {	
	return rateExpressions[equationIndex]->evaluateVector(allValues);		
}

int VCellCVodeSolver::RHS_callback(realtype t, N_Vector y, N_Vector r, void *fdata) {
	VCellCVodeSolver* solver = (VCellCVodeSolver*)fdata;
	return solver->RHS(t, y, r);
}

int VCellCVodeSolver::RootFn_callback(realtype t, N_Vector y, realtype *gout, void *g_data) {
	VCellCVodeSolver* solver = (VCellCVodeSolver*)g_data;
	return solver->RootFn(t, y, gout);
}

void VCellCVodeSolver::solve(double* paramValues, bool bPrintProgress, FILE* outputFile, void (*checkStopRequested)(double, long)) {
	if (checkStopRequested != nullptr) checkStopRequested(STARTING_TIME, 0);
	writeFileHeader(outputFile);

	// clear data in result set before solving
	this->odeResultSet->clearData();

	// copy parameter values to the end of values, these will stay the same during solving
	memset(values, 0, (NUM_EQUATIONS + 1) * sizeof(double));
	memcpy(values + 1 + NUM_EQUATIONS, paramValues, NUM_PARAMETERS * sizeof(double));
	memset(values + 1 + NUM_EQUATIONS + NUM_PARAMETERS, 0, numDiscontinuities * sizeof(double));

	initCVode(paramValues);
	cvodeSolve(bPrintProgress, outputFile, checkStopRequested);
}

void VCellCVodeSolver::initCVode(double* paramValues) {
	//Initialize y, variable portion of values
	for (int i = 0; i < NUM_EQUATIONS; i ++) {
		NV_Ith_S(y, i) = initialConditionExpressions[i]->evaluateVector(paramValues);
	}

	if (numDiscontinuities > 0) {
		initDiscontinuities();
	}

	reInit(STARTING_TIME);

	if (numDiscontinuities > 0) {
		solveInitialDiscontinuities(STARTING_TIME);
		int flag = CVodeRootInit(solver, 2 * numDiscontinuities, RootFn_callback, this);
		checkCVodeFlag(flag);
	}

	executeEvents(STARTING_TIME);
}

void VCellCVodeSolver::reInit(double t) {
	int flag = 0;
	if (solver == nullptr) {
		solver = CVodeCreate(CV_BDF, CV_NEWTON);
		if (solver == nullptr) {
			throw "VCellCVodeSolver:: Out of memory";
		}
		flag = CVodeMalloc(solver, RHS_callback, t, y, ToleranceType, RelativeTolerance, &AbsoluteTolerance);
		checkCVodeFlag(flag);

		flag = CVodeSetFdata(solver, this);
		checkCVodeFlag(flag);
		flag = CVDense(solver, NUM_EQUATIONS);
		//flag = CVSpgmr(solver, PREC_NONE, 0);
		checkCVodeFlag(flag);

		flag = CVodeSetMaxNumSteps(solver, 5000);
	} else {
		flag = CVodeReInit(solver, RHS_callback, t, y, ToleranceType, RelativeTolerance, &AbsoluteTolerance);
	}
	checkCVodeFlag(flag);
}

bool VCellCVodeSolver::fixInitialDiscontinuities(double t) {
	double* oldy = new double[NUM_EQUATIONS];
	memcpy(oldy, NV_DATA_S(y), NUM_EQUATIONS * sizeof(realtype));

	double epsilon = std::max(1e-15, ENDING_TIME * 1e-10);
	double currentTime = t;	
	double tout = currentTime + epsilon;
	CVodeSetStopTime(solver, tout);
	int returnCode = CVode(solver, tout, y, &currentTime, CV_ONE_STEP_TSTOP);
	if (returnCode != CV_TSTOP_RETURN && returnCode != CV_SUCCESS) {
		throwCVodeErrorMessage(returnCode);
	}

	updateTandVariableValues(currentTime, y);
	bool bInitChanged = false;
	for (int i = 0; i < numDiscontinuities; i ++) {
		// evaluate discontinuities at t+epsilon
		double v = odeDiscontinuities[i]->discontinuityExpression->evaluateVector(values);
		if (v != discontinuityValues[i]) {
			std::cout << "fixInitialDiscontinuities() : update discontinuities at time " << t << " : " << odeDiscontinuities[i]->discontinuityExpression->infix() << " " << discontinuityValues[i] << " " << v << std::endl;
			discontinuityValues[i] = v;			
			bInitChanged = true;
		}
	}	
	if (bInitChanged) {
		memcpy(values + 1 + NUM_EQUATIONS + NUM_PARAMETERS, discontinuityValues, numDiscontinuities * sizeof(double));
	}

	//revert y
	memcpy(NV_DATA_S(y), oldy, NUM_EQUATIONS * sizeof(realtype));
	reInit(t);

	delete[] oldy;

	return bInitChanged;
}

void VCellCVodeSolver::onCVodeReturn(realtype Time, int returnCode) {
	if (returnCode == CV_ROOT_RETURN) {
		// flip discontinuities
		int flag = CVodeGetRootInfo(solver, rootsFound);
		checkCVodeFlag(flag);
		std::cout << std::endl << "cvodeSolve() : roots found at time " << Time << std::endl;
#ifdef SUNDIALS_DEBUG
		printVariableValues(Time);
#endif
		updateDiscontinuities(Time, true);

		int updateCount = 0;
		for (updateCount = 0;  updateCount < MAX_NUM_EVENTS_DISCONTINUITIES_EVAL; updateCount ++) {
			bool bExecuted = executeEvents(Time);
			if (!bExecuted) {
				break;
			}
			updateDiscontinuities(Time, false);
		}
		reInit(Time);
	} else {
		int updateCount = 0;
		for (updateCount = 0;  updateCount < MAX_NUM_EVENTS_DISCONTINUITIES_EVAL; updateCount ++) {			
			bool bExecuted = executeEvents(Time);
			if (!bExecuted) {
				break;
			}
			updateDiscontinuities(Time, false);
		}
		if (updateCount > 0) {
			reInit(Time);
		}
		checkDiscontinuityConsistency();
	}
}

void VCellCVodeSolver::cvodeSolve(bool bPrintProgress, FILE* outputFile, void (*checkStopRequested)(double, long)) {
	if (checkStopRequested != 0) {
		checkStopRequested(STARTING_TIME, 0);
	}

	realtype Time = STARTING_TIME;
	double lastPercentile=0.00;
	clock_t lastTime = clock(); // to control the output of progress, send progress every 2 seconds
	double increment = 0.01;
	long iterationCount=0;
	long outputCount=0;

	// write intial conditions
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
			
			CVodeSetStopTime(solver, tstop);
			int returnCode = CVode(solver, ENDING_TIME, y, &Time, CV_ONE_STEP_TSTOP);
			iterationCount++;

			// save data if return CV_TSTOP_RETURN (meaning reached end of time or max time step 
			// before one normal step) or CV_SUCCESS (meaning one normal step)
			if (returnCode == CV_TSTOP_RETURN || returnCode == CV_SUCCESS || returnCode == CV_ROOT_RETURN) {
				onCVodeReturn(Time, returnCode);

				if (returnCode == CV_ROOT_RETURN || iterationCount % keepEvery == 0 || Time >= ENDING_TIME){
					outputCount++;
					if (outputCount * (NUM_EQUATIONS + 1) * bytesPerSample > MaxFileSizeBytes){
						/* if more than one gigabyte, then fail */
						const std::string msg{"output exceeded maximum " + std::to_string(MaxFileSizeBytes) + " bytes"};
						throw VCell::Exception(msg.c_str());
					}
					writeData(Time, outputFile);
					if (bPrintProgress) {
						printProgress(Time, lastPercentile, lastTime, increment, outputFile);
					}
				}
			} else {
				throwCVodeErrorMessage(returnCode);
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

				CVodeSetStopTime(solver, tstop);
				int returnCode = CVode(solver, sampleTime, y, &Time, CV_NORMAL_TSTOP);
				iterationCount++;

				// if return CV_SUCCESS, this is an intermediate result, continue without saving data.
				if (returnCode == CV_TSTOP_RETURN || returnCode == CV_SUCCESS || returnCode == CV_ROOT_RETURN) {
					onCVodeReturn(Time, returnCode);

					if (Time == sampleTime) {
						writeData(Time, outputFile);
						if (bPrintProgress) {
							printProgress(Time, lastPercentile, lastTime, increment, outputFile);
						}
						outputCount ++;
						break;
					}
				} else {
					throwCVodeErrorMessage(returnCode);
				}
			}
		}
	}
#ifdef USE_MESSAGING
	SimulationMessaging::getInstVar()->setWorkerEvent(JobEvent::JOB_COMPLETED, 1, Time);
#endif
}

void VCellCVodeSolver::updateTandVariableValues(realtype t, N_Vector y) {
	values[0] = t;
	memcpy(values + 1, NV_DATA_S(y), NUM_EQUATIONS * sizeof(realtype));
}
