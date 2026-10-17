#include "mex.h"

void flink(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]); 

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
	flink(nlhs, plhs, nrhs, prhs);
}
