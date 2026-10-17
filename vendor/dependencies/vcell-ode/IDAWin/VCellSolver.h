//
// Created by Logan Drescher on 6/26/25.
//

#ifndef VCELLSOLVER_H
#define VCELLSOLVER_H
#include "VCellSolverInput.h"

/**
 * Interfacing class for all solvers; we generally only care about configuring and running...
 */
class VCellSolver {
	public:
		virtual ~VCellSolver() = default;

		virtual void configureFromInput(VCellSolverInputBreakdown& inputBreakdown) = 0;

		virtual void solve(double *paramValues = nullptr, bool bPrintProgress = false, FILE *outputFile = nullptr,
					   void (*checkStopRequested)(double, long) = nullptr) = 0;

	protected:
		VCellSolver() = default;
};

#endif //VCELLSOLVER_H
