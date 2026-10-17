#ifndef SPARSELINEARSOLVER_H
#define SPARSELINEARSOLVER_H

#include <VCELL/PDESolver.h>

class SparseMatrixEqnBuilder;
class Variable;

class SparseLinearSolver : public PDESolver
{
public:
    SparseLinearSolver(Variable *Var,  SparseMatrixEqnBuilder* eqnbuilder, double rtol, bool AbTimeDependent);
    ~SparseLinearSolver() override;

    void solveEqn(SimTool* sim_tool, double deltaTime, int volumeIndexStart, int volumeIndexSize, int membraneIndexStart, int membraneIndexSize, bool bFirstTime) override;

private:
	bool enableRetry;
	void initPCGWorkspace(long additional=0);
	double pcgRelErr;

protected:
	int* PCGSolve(SimTool* sim_tool, bool bRecomputeIncompleteFactorization);
	SparseMatrixEqnBuilder* smEqnBuilder;    
	double* pcg_workspace;	
	long nWork;
	
};
#endif
