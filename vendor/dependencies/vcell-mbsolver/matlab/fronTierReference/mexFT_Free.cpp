#include "mex.h"
#include <FronTier.h>

/* gateway function */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] )
{
	if (nrhs != 1)
	{
        mexErrMsgIdAndTxt("MATLAB:mexget:invalidInput", "mexFT_Free(front)");
    }
	Front* front = *reinterpret_cast<Front**>(mxGetPr(prhs[0]));
	delete_interface(front->interf);
	delete front;
	mexPrintf("front freed\n");
}