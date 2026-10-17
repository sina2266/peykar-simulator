#include "mex.h"
#include <FronTier.h>
#include <vector>
using std::vector;

static void retrievePoints_2d(Front* front, vector<FT_POINT*>& points)
{
	CURVE** curves = front->interf->curves;
	if (curves == NULL) 
	{
		return;
	}
	int numCurves = 0;
	for (int i = 0; curves[i] != NULL; ++ i)
	{
		mexPrintf("curve %d\n", i);
		CURVE* curve = curves[i];
		if (is_bdry(curve))
		{
			continue;
		}
		for (BOND* bond = curve->first; bond != NULL; bond = bond->next)
		{
			FT_POINT* p = bond->start;
			if (p != NULL)
			{
				points.push_back(p);
			}
		}

		if (!curve->last && !curve->last->end)
		{
			FT_POINT* p = curve->last->end;
			points.push_back(p);
		}
		++ numCurves;
	}
	mexPrintf("# interior curves = %d\n", numCurves);
}

void retrievePoints_3d(Front* front, vector<FT_POINT*>& points)
{
	if (front->interf->surfaces == NULL)
	{
		return;
	}
	int numSurfaces = (int)size_of_pointers((POINTER *)front->interf->surfaces);
	mexPrintf("# surfaces = %d\n", numSurfaces);
	for (int i = 0; i < numSurfaces; i ++)
	{
		SURFACE* s = front->interf->surfaces[i];
		FT_POINT		*p;
		for (TRI* tri = first_tri(s); !at_end_of_tri_list(tri,s); tri = tri->next)
		{
			Index_of_point(Point_of_tri(tri)[0]) =
			Index_of_point(Point_of_tri(tri)[1]) =
			Index_of_point(Point_of_tri(tri)[2]) = ERROR;
		}
		int num_points = 0;
		for (TRI* tri = first_tri(s); !at_end_of_tri_list(tri,s); tri = tri->next)
		{
			for (int i = 0; i < 3; ++i)
			{
				p = Point_of_tri(tri)[i];
				if (Index_of_point(p) == ERROR) 
				{
					points.push_back(p);
					Index_of_point(p) = num_points ++;
				}
			}
		}
	}
}

int retrievePoints(Front* front, double*& matrix)
{
	vector<FT_POINT*> points;
	int dim = front->rect_grid->dim;
	if (dim == 2)
	{
		retrievePoints_2d(front, points);
	}
	else if (dim == 3)
	{
		retrievePoints_3d(front, points);
	}
	matrix = new double[points.size() * dim];
	int cnt = 0;
	for (vector<FT_POINT*>::iterator iter = points.begin(); iter < points.end(); ++ iter)
	{
		FT_POINT* p = *iter;
		double* coords = Coords(p);
		memcpy(matrix + cnt * dim, Coords(p), dim * sizeof(double));
		++ cnt;
	}
	return cnt;
}

double level_func(POINTER func_params, double *coords)
{
	mxArray *rhs[1], *lhs[1];
	// always allocation 3 doubles even for 2d
	rhs[0] = mxCreateDoubleMatrix(1, 3, mxREAL);
	double* ptr = mxGetPr(rhs[0]);
	memcpy(ptr, coords, 3 * sizeof(double));

	mexCallMATLAB(1, lhs, 1, rhs, "FT_level_func");
	ptr = mxGetPr(lhs[0]);
	double dist = ptr[0];
	mxDestroyArray(rhs[0]);
    mxDestroyArray(lhs[0]);
	return dist;
}

int velo_func(POINTER params, Front *front, FT_POINT *p, HYPER_SURF_ELEMENT *hse,
	HYPER_SURF *hs, double *vel)
{	
	double* coords = Coords(p);
	mxArray *rhs[2], *lhs[1];
	// always allocation 3 doubles even for 2d
	rhs[0] = mxCreateDoubleScalar(front->time);
	rhs[1] = mxCreateDoubleMatrix(1, 3, mxREAL);
	double* ptr = mxGetPr(rhs[1]);
	memcpy(ptr, coords, 3 * sizeof(double));

	mexCallMATLAB(1, lhs, 2, rhs, "FT_velo_func");
	ptr = mxGetPr(lhs[0]);
	int dim = front->rect_grid->dim;
	memcpy(vel, ptr, dim * sizeof(double));
	mxDestroyArray(rhs[0]);
	mxDestroyArray(rhs[1]);
    mxDestroyArray(lhs[0]);
	return 0;
}

void initFront(Front* front, int dim, double* xlohi, int N, double tmax)
{
	static F_BASIC_DATA f_basic;
	static LEVEL_FUNC_PACK level_func_pack;
	static VELO_FUNC_PACK velo_func_pack;

	f_basic.dim = dim;	
	FT_Init(0, 0, &f_basic);

	f_basic.L[0] = xlohi[0];	
	f_basic.U[0] = xlohi[1];
	f_basic.L[1] = xlohi[2];
	f_basic.U[1] = xlohi[3];
	if (dim == 3)
	{
		f_basic.L[2] = xlohi[4];
		f_basic.U[2] = xlohi[5];
	}
	f_basic.gmax[0] = f_basic.gmax[1] = N;
	if (dim == 3)
	{
		f_basic.gmax[2] = N;
	}
	f_basic.boundary[0][0] = f_basic.boundary[0][1] = PERIODIC_BOUNDARY;
	f_basic.boundary[1][0] = f_basic.boundary[1][1] = PERIODIC_BOUNDARY;
	if (dim == 3)
	{
		f_basic.boundary[2][0] = f_basic.boundary[2][1] = PERIODIC_BOUNDARY;
	}
	f_basic.size_of_intfc_state = 0;

	FT_StartUp(front, &f_basic);

	level_func_pack.neg_component = 1;
	level_func_pack.pos_component = 2;
	level_func_pack.func_params = NULL;
	level_func_pack.func = level_func;
	level_func_pack.wave_type = FIRST_PHYSICS_WAVE_TYPE;
	FT_InitIntfc(front,&level_func_pack);

	if (dim == 2)
	{
		FT_ClipIntfcToSubdomain(front);
	}

	velo_func_pack.func_params = NULL;
	velo_func_pack.func = velo_func;
	if (dim == 2)
	{
		velo_func_pack.point_propagate = fourth_order_point_propagate;
	}

	FT_InitVeloFunc(front,&velo_func_pack);

	if (dim == 3)
	{
		front->_point_propagate = first_order_point_propagate;
	}
   
	front->max_time = tmax;
	front->max_step = 1000000;
    front->print_time_interval = 1000000;
	front->movie_frame_interval = 1000000;

	if (dim == 3)
	{
		Time_step_factor(front) = 0.2;
		Frequency_of_redistribution(front,GENERAL_WAVE) = 1000;
	}
	FT_RedistMesh(front);
	FT_ResetTime(front);
	// This is a virtual propagation to get maximum front 
	// speed to determine the first time step.
    FT_Propagate(front);
    FT_SetTimeStep(front);
    FT_SetOutputCounter(front);
}

#define USAGE_INIT_2D "[front time points] = mexFT_Step(dim, [xlo xhi], [ylo yhi], N, tmax)"
#define USAGE_INIT_3D "[front time points] = mexFT_Step(dim, [xlo xhi], [ylo yhi], [zlo zhi], N, tmax)"
#define USAGE_STEP "[time points] = mexFT_Step(front, tstop)" 

/* gateway function */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] )
{
	Front* front = 0;
	int outParamIndex = 0;
	if (nrhs >= 5 && nlhs == 3)
	{
		int rhsIndex = 0;
        int dim = (int)mxGetScalar(prhs[rhsIndex ++]);
		if (dim != 2 && dim != 3)
		{
			 mexErrMsgIdAndTxt("MATLAB:mexget:invalidInput", "Only 2D and 3D are supported.");
			 return;
		}
		if (dim == 2 && nrhs != 5)
		{
			 mexErrMsgIdAndTxt("MATLAB:mexget:invalidInput", USAGE_INIT_2D " OR " USAGE_STEP);
			 return;
		}
		else if (dim == 3 && nrhs != 6)
		{
			 mexErrMsgIdAndTxt("MATLAB:mexget:invalidInput", USAGE_INIT_3D " OR " USAGE_STEP);
			 return;
		}

		double xlohi[6];
		for (int i = 0; i < dim; ++ i)
		{
			double* ptr = mxGetPr(prhs[rhsIndex ++]);
			//int m = mxGetM(prhs[1]);
			//int n = mxGetN(prhs[1]);
			xlohi[i * dim] = ptr[0];
			xlohi[i * dim + 1] = ptr[1];
		}
		int N = (int)mxGetScalar(prhs[rhsIndex ++]);
		double tmax = mxGetScalar(prhs[rhsIndex ++]);

		mexPrintf("dim=%d\n", dim);
		mexPrintf("[xlo xhi]=[%f %f]\n", xlohi[0], xlohi[1]);
		mexPrintf("[ylo yhi]=[%f %f]\n", xlohi[2], xlohi[3]);
		if (dim == 3)
		{
			mexPrintf("[zlo zhi]=[%f %f]\n", xlohi[4], xlohi[5]);
		}
		mexPrintf("N=%d\n", N);
		mexPrintf("tmax=%f\n", tmax);

		front = new Front();
		initFront(front, dim, xlohi, N, tmax);
		mexPrintf("front initialized.\n");

		plhs[0]  = mxCreateNumericMatrix(1, 1, mxUINT32_CLASS, mxREAL);
		*reinterpret_cast<Front**>(mxGetPr(plhs[0])) = front;
		mexPrintf("front saved.\n");
		outParamIndex = 1;
    }
	else if (nrhs == 2 && nlhs == 2)
	{
		front = *reinterpret_cast<Front**>(mxGetPr(prhs[0]));
		if (front == 0)
		{
			mexErrMsgIdAndTxt("MATLAB:mexget:invalidInput", "front is invalid, it either has been not initialized or has been freed");
			return;
		}

		double tstop = mxGetScalar(prhs[1]);
		front->max_time = tstop;
	}
	else
	{
		mexErrMsgIdAndTxt("MATLAB:mexget:invalidParameters", USAGE_INIT_2D " OR " USAGE_INIT_3D " OR " USAGE_STEP);
		return;
	}

	if (front->max_time > front->time)
	{
		FT_TimeControlFilter(front);
		mexPrintf("time = %f\tstep = %5d\tnext dt = %f\n", front->time, front->step, front->dt);
		for (;;)
		{	
			FT_Propagate(front);
			FT_AddTimeStepToCounter(front);
			FT_SetTimeStep(front);
			mexPrintf("time = %f\tstep = %5d\tnext dt = %f\n", front->time, front->step, front->dt);
			if (FT_TimeLimitReached(front))
			{
				mexPrintf("tstop reached\n");
				break;
			}
			FT_TimeControlFilter(front);
		}
	}
	else
	{
		mexPrintf("time = %f\tstep = %5d\tnext dt = %f\n", front->time, front->step, front->dt);
	}
	int dim = front->rect_grid->dim;
	plhs[outParamIndex] = mxCreateDoubleScalar(front->time);
	mexPrintf("time saved, %f.\n", front->time);

	++ outParamIndex;
	double* matrix;
	int M = retrievePoints(front, matrix);
	int N = dim;
	mexPrintf("number of points=%d\n", M); 
	plhs[outParamIndex] = mxCreateDoubleMatrix((mwSize)M, (mwSize)N, mxREAL);
	double* ptr = mxGetPr(plhs[outParamIndex]);
	for(int i = 0; i < N; ++ i)
    {
        for(int j = 0; j < M; ++ j)
        {
            ptr[i * M + j] = matrix[j * N + i];
        }
    }
	mexPrintf("points saved.\n");
	delete matrix;
}
