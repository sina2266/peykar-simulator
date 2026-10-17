/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef SUNDIALSSOLVEROPTIONS_H
#define SUNDIALSSOLVEROPTIONS_H

struct SundialsSolverOptions {
	double relTol, absTol;
	double maxStep;
	int maxOrderAdvection;
	bool borderExtrapolationDisable;

	SundialsSolverOptions() {
		relTol = 1e-7;
		absTol = 1e-9;
		maxStep = 0.1;
		maxOrderAdvection = 2;
		borderExtrapolationDisable = false;
	}
};
#endif
