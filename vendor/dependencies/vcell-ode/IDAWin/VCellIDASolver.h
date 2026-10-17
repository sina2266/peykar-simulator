#ifndef VCELLIDASOLVER_H
#define VCELLIDASOLVER_H

#include "VCellSundialsSolver.h"
#include <string>

class VCellIDASolver : public VCellSundialsSolver {
public:
	VCellIDASolver();	
	~VCellIDASolver() override;

	void solve(double* paramValues=0, bool bPrintProgress=false, FILE* outputFile=0, void (*checkStopRequested)(double, long)=0) override;

protected:
	void updateTempRowData(double currTime) override;
	void readEquations(VCellSolverInputBreakdown& inputBreakdown) override;
	void initialize() override;
	std::string getSolverName() override { return "IDA"; }
	VCellSolverTypes getSolverType() override { return VCellSolverTypes::IDA; }

private:
	VCell::Expression** rhsExpressions;  // can be rate expression in ODE case or RHS expression in DAE case
	double **transformMatrix;
	double **inverseTransformMatrix;
	int numDifferential;
	int numAlgebraic;

	N_Vector yp;
	N_Vector id;  // 1 for differential variable, 0 for algebraic variable (used in IDACalcIC()).

	int Residual(realtype t, N_Vector y, N_Vector yp, N_Vector residual);	
	static int Residual_callback(realtype t, N_Vector y, N_Vector yp, N_Vector residual, void *rdata);
	/*
	Arguments	
		tt			is the current value of the independent variable.
		yy			is the current value of the dependent variable vector, y(t).
		yp			is the current value of y0(t).
		rr			is the output residual vector F(t; y; y0).
		res_data	is a pointer to user data | the same as the res data parameter passed to IDASetRdata.
	Return value 
		An IDAResFn function type should return a value of 0 if successful, a positive value if a recoverable 
		error occured (e.g. yy has an illegal value), or a negative value if a nonrecoverable error occured.
		In the latter case, the integrator halts. If a recoverable error occured, the integrator will attempt to correct and retry.
	*/
	//int RootFn(realtype t, N_Vector y, N_Vector yp, realtype *gout);
	static int RootFn_callback(realtype t, N_Vector y, N_Vector yp, realtype *gout, void *g_data);
	/*
	Arguments 
		t		is the current value of the independent variable.
		y		is the current value of the dependent variable vector, y(t).
		yp		is the current value of y0(t), the t�derivative of y.
		gout	is the output array, of length nrtfn, with components gi(t; y; y0).
		g data	is a pointer to user data | the same as the g data parameter passed to IDARootInit.
	Return value 
		An IDARootFn should return 0 if successful or a non-zero value if an error occured (in which case 
		the integration is halted and IDASolve returns IDA RTFUNC FAIL).
	*/
	void initIDA(double* paramValues);
	void idaSolve(bool bPrintProgress, FILE* outputFile, void (*checkStopRequested)(double, long));
	void throwIDAErrorMessage(int returnCode);
	void checkIDAFlag(int flag);

	void reInit(realtype t);
	bool fixInitialDiscontinuities(double t) override;
	void updateTandVariableValues(realtype t, N_Vector y) override;

	void onIDAReturn(realtype Time, int returnCode);
};

#endif
