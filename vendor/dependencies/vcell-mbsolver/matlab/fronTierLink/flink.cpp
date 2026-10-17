#include <memory>
#include "mex.h"
#include <VFrontier.h>
#include <vector>
#include <matlabStruct.h>
#include <VCellException.h>
#include <VCDictionary.h>
#include <matlabAssert.h>
#include <mexstream.h>
#include <vcGccCompat.h>
//#include <VCellFrontierUtil.h>
using std::vector;
namespace Frontier {
	template <class T>
	double *Coords(const T & obj) {
		return obj->_coords;
	}
}
namespace {

	//slot in
	std::vector<Front *> fronts;
	using vcell_util::Dictionary;

	struct BoundaryTypeMap : public Dictionary<int> {
		BoundaryTypeMap( )
			:Dictionary<int>("boundaryType") {
				BoundaryTypeMap & self = *this;
#define INSERT(x) self[#x] = x; 
				INSERT(PERIODIC_BOUNDARY );
				INSERT(REFLECTION_BOUNDARY);
				INSERT(MIXED_TYPE_BOUNDARY);
				INSERT(OPEN_BOUNDARY);
				INSERT(FIRST_USER_BOUNDARY_TYPE);
				//#undef INSERT
		}
	} boundaryTypeMap; 
	typedef void (*PropFunc)(struct _Front*,POINTER,FT_POINT*,FT_POINT*, HYPER_SURF_ELEMENT*,HYPER_SURF*,double,double*);
	struct PropagateTypeMap : public Dictionary<PropFunc> {
		PropagateTypeMap( )  
			:Dictionary<PropFunc>("propagate") {
				PropagateTypeMap & self = *this;
				INSERT(first_order_point_propagate);
				INSERT(second_order_point_propagate);
				//INSERT(third_order_point_propagate);
				INSERT(fourth_order_point_propagate);
		}
	} propagateTypeMap;

	struct RedistTypeMap : public Dictionary<int> {
		RedistTypeMap( ) 
			:Dictionary<int>("redistribution") {
				RedistTypeMap & self =  *this;
				INSERT(NO_REDIST);
				INSERT(EXPANSION_REDIST);
				INSERT(FULL_REDIST);
		}
	} redistTypeMap;

	struct RedistVersionMap : public Dictionary<int> {
		RedistVersionMap( ) 
			:Dictionary<int>("fullRedistributionVersion") {
				RedistVersionMap & self = *this;
				INSERT(ORDINARY_REDISTRIBUTE);
				INSERT(EQUI_BOND_REDISTRIBUTE);
		}
	} redistVersionMap;

	/**
	* replace Frontier macro
	*/
	template <class T>
	void zeroStruct(T &t) {
		memset(&t,0,sizeof(T));
	}
}

static void retrievePoints_2d(Front * front, vector<FT_POINT*>& points, std::ostream & log)
{
	CURVE** curves = front->interf->curves;
	if (curves == NULL) 
	{
		return;
	}
	int numCurves = 0;
	for (int i = 0; curves[i] != NULL; ++ i)
	{
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
	log << "# interior curves = " <<  numCurves << std::endl;
}

void retrievePoints_3d(Front *front, vector<FT_POINT*>& points, std::ostream &log)
{
	if (front->interf->surfaces == NULL)
	{
		return;
	}
	int numSurfaces = (int)size_of_pointers((POINTER *)front->interf->surfaces);
	log << "# surfaces = " << numSurfaces<< std::endl;
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

int retrievePoints(Front * front,std::unique_ptr<double> & matrix, std::ostream &log)
{
	vector<FT_POINT*> points;
	int dim = front->rect_grid->dim;
	if (dim == 2)
	{
		retrievePoints_2d(front,points,log);
	}
	else if (dim == 3)
	{
		retrievePoints_3d(front,points,log);
	}
	matrix.reset( new double[points.size() * dim] );
	int cnt = 0;
	for (vector<FT_POINT*>::iterator iter = points.begin(); iter < points.end(); ++ iter)
	{
		FT_POINT* p = *iter;
		double* coords = Frontier::Coords(p);
		memcpy(matrix.get( ) + cnt * dim, Frontier::Coords(p), dim * sizeof(double));
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
	double* coords = Frontier::Coords(p);
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

void setupRedistribution(Front *front,const matlabLink::MatlabStruct & mlStruct, std::ostream &log) {
	std::string redistType = mlStruct.str(redistTypeMap.name( ),false);
	if (!redistType.empty( )) {
		log << "redistribution type " << redistType;
		const int option = redistTypeMap.get(redistType);
		CURVE_CLEANERS Sav_cleaners = Curve_redistribution_info(front).Cleaners;
		switch (option) {
		case NO_REDIST:
			zeroStruct(Curve_redistribution_info(front));
			Curve_redistribution_info(front).Cleaners = Sav_cleaners;
			break;
		case EXPANSION_REDIST:
			zeroStruct(Curve_redistribution_info(front));
			Curve_redistribution_function(front) = expansion_redistribute;
			Curve_redistribution_info(front).Cleaners = Sav_cleaners;
			break;
		case FULL_REDIST:
			std::string versionType = mlStruct.str(redistVersionMap.name( ), true);
			log << " version " << versionType;
			Curve_redistribution_function(front) = full_redistribute;
			const int subOption = redistVersionMap.get(versionType);
			switch (subOption) {
			case ORDINARY_REDISTRIBUTE:
				Forward_curve_redistribute_function(front) =
					full_inc_redist_cur;
				Backward_curve_redistribute_function(front) =
					full_dec_redist_cur;
				break;
			case EQUI_BOND_REDISTRIBUTE:
				Forward_curve_redistribute_function(front) =
					equi_curve_redistribute;
				Backward_curve_redistribute_function(front) =
					backward_equi_curve_redistribute;
				break;
			}
		}
		log << std::endl;
	}

	auto rd = mlStruct.doubles("redistributionFrequency",false);
	if (rd.size > 0) {
		int rFre = static_cast<int>(rd[0]);
		Frequency_of_redistribution(front,GENERAL_WAVE) = rFre; 
		Frequency_of_redistribution(front,VECTOR_WAVE) = rFre; 
		Frequency_of_redistribution(front,GENERAL_NODE) = rFre; 
	}
}

void showHelp( ) {
	using std::endl;
	const char tab = '\t';
	const char * const req = "(required)";
	const char * const opt = "(optional)";
	std::ostringstream help;
	help << "Usage: fronTierCmd(s) " 
		<< tab << "s must be structure with fields" << endl
		<<  "command (init,step,help, free) "<< req << endl;

	help <<  "init parameters: " << endl 
		<< tab << "dim " << req << endl
		<< tab << "limits, 4 or 6 values (2D/3D) " << req << endl
		<< tab << "maxTime " << req << endl
		<< tab << "maxStep  " << opt << endl
		<< tab << boundaryTypeMap.options(true) << opt << endl 
		<< tab << propagateTypeMap.options(true) << opt << endl 
		<< tab << redistTypeMap.options(true) << opt << endl 
		<< tab << redistVersionMap.options(true) << req << " (if full distribution type)" << endl 
		<< tab << "redistributionFrequency  " << opt << endl
		<< tab << "redistOrder (1 or 2)  " << opt << endl
		<< tab << "debug (print debug messages) " << opt << endl 
		<< "returns handle to front " << endl << endl;

	help <<  "step parameters: " << endl 
		<< tab << "front (returned from prior 'init' call) " << req << endl 
		<< tab << "timeStop " << req << endl
		<< tab << "debug (print debug messages) " << opt << endl 
		<< "returns [time front] " << endl << endl;

	help <<  "free parameters: " << endl 
		<< tab << "front (returned from prior 'init' call) " << req << endl ;
#ifdef NDEBUG
	const char * const variant = "release";
#else
	const char * const variant = "debug";
#endif
		
	help << "link library " << variant << endl;
    help << std::ends;

	std::string helpString = help.str( ); // create string on stack to keep c_str pointer valid until end of function
	char * c_ptr = const_cast<char *>(helpString.c_str( ));
	mexPrintf("%s",c_ptr);
}

void initFront(Front *front,const matlabLink::MatlabStruct & mlStruct, std::ostream & config) 
{
	ML_ASSERT(front != nullptr);

	using matlabLink::MData;
	MData<double> dims = mlStruct.doubles("dim");
	int dim = static_cast<int>(dims[0]);
	MData<double> limits = mlStruct.doubles("limits");
	int rsize = 0;
	switch (dim) {
	case 2:
		rsize = 4;
		break;
	case 3:
		rsize = 6;
		break;
	default:
		VCELL_EXCEPTION(domain_error, "number dimensions " << dim << " must be 2 or 3");
	}
	if (limits.size != rsize) {
		VCELL_EXCEPTION(domain_error,dim << " front must have " << rsize << " limit values");
	}
	config << dim  << " dimensions " << std::endl;

	static F_BASIC_DATA f_basic;
	static LEVEL_FUNC_PACK level_func_pack;
	static VELO_FUNC_PACK velo_func_pack;

	f_basic.dim = dim;	
	FT_Init(0, 0, &f_basic);

	f_basic.L[0] = limits[0]; 
	f_basic.U[0] = limits[1];
	f_basic.L[1] = limits[2];
	f_basic.U[1] = limits[3];
	if (dim == 3)
	{
		f_basic.L[2] = limits[4];
		f_basic.U[2] = limits[5];
	}
	MData<double> gmax = mlStruct.doubles("gmax");
	config << " gmax " << "[" << gmax[0] << " " << gmax[1];

	f_basic.gmax[0] = gmax[0];
	f_basic.gmax[1] = gmax[1];
	if (dim == 3)
	{
		f_basic.gmax[2] = gmax[2];
		config << " " << gmax[2];
	}
	config << "]" << std::endl;
	std::string boundaryType = mlStruct.str(boundaryTypeMap.name( ),"PERIODIC_BOUNDARY");
	int bndType = boundaryTypeMap.get(boundaryType);
	config << "boundaryType " << boundaryType << std::endl;

	f_basic.boundary[0][0] = f_basic.boundary[0][1] =  bndType;
	f_basic.boundary[1][0] = f_basic.boundary[1][1] =  bndType;
	if (dim == 3)
	{
		f_basic.boundary[2][0] = f_basic.boundary[2][1] = bndType; 
	}
	f_basic.size_of_intfc_state = 0;

	FT_StartUp(front, &f_basic);

	level_func_pack.neg_component = 1;
	level_func_pack.pos_component = 2;
	level_func_pack.func_params = NULL;
	level_func_pack.func = level_func;
	level_func_pack.wave_type = FIRST_PHYSICS_WAVE_TYPE;
	FT_InitIntfc(front,&level_func_pack);
//	MData<double> redistOrder = mlStruct.doubles("redistOrder",false);
//	if (redistOrder.size > 0) {
//		int ro = redistOrder[0];
//		spatial::setRedistDefault(front,ro);
//		config << "set default redist_order to " << ro << std::endl;
//	}

	if (dim == 2)
	{
		FT_ClipIntfcToSubdomain(front);
	}

	std::string propagateType = mlStruct.str(propagateTypeMap.name( ),"fourth_order_point_propagate");
	PropFunc pf = propagateTypeMap.get(propagateType);

	velo_func_pack.func_params = NULL;
	velo_func_pack.func = velo_func;
	if (dim == 2)
	{
		velo_func_pack.point_propagate = pf; 
	}

	FT_InitVeloFunc(front,&velo_func_pack);

	if (dim == 3)
	{
		front->_point_propagate = pf; 
	}

	config << "propagateType " << propagateType << std::endl;
	front->max_time = mlStruct.doubles("maxTime")[0]; 
	front->max_step = 1000000;
	auto maxStep = mlStruct.doubles("maxStep",false);
	if (maxStep.size  > 0) {
		front->max_step = static_cast<int>(maxStep[0]);
	}
	front->print_time_interval = front->max_step; 
	front->movie_frame_interval = front->max_step; 
	config << "max time " << front->max_time << ", max step " << front->max_step  << std::endl;

	if (dim == 3)
	{
		Time_step_factor(front) = 0.2;
		Frequency_of_redistribution(front,GENERAL_WAVE) = 1000;
	}
	setupRedistribution(front,mlStruct, config);
	FT_RedistMesh(front);
	FT_ResetTime(front);
	// This is a virtual propagation to get maximum front 
	// speed to determine the first time step.
	FT_Propagate(front);
	FT_SetTimeStep(front);
	FT_SetOutputCounter(front);
}

void step(Front * front, int nlhs, mxArray *plhs[], const matlabLink::MatlabStruct & mlStruct, std::ostream & log) {
	if (nlhs != 2) {
		mexErrMsgTxt("step requires two output arguments");
	}
	using std::endl;
	const char tab = '\t';

	double tstop =  mlStruct.doubles("timeStop")[0]; 
	front->max_time = tstop;

	if (front->max_time > front->time)
	{
		FT_TimeControlFilter(front);
		log << "time = " << front->time << tab << "step = " << front->step << tab << "next dt = " << front->dt << endl; 
		for (;;)
		{	
			FT_Propagate(front);
			FT_AddTimeStepToCounter(front);
			FT_SetTimeStep(front);
			log << "time = " << front->time << tab << "step = " << front->step << tab << "next dt = " << front->dt << endl; 
			if (FT_TimeLimitReached(front))
			{
				log << tab << "stop reached" << endl;
				break;
			}
			FT_TimeControlFilter(front);
		}
	}
	else
	{
		log << "time = " << front->time << tab << "step = " << front->step << tab << "next dt = " << front->dt << endl; 
	}
	int dim = front->rect_grid->dim;
	plhs[0] = mxCreateDoubleScalar(front->time);
	log << "time saved, " << front->time << endl;

	std::unique_ptr<double> matrix;
	int M = retrievePoints(front, matrix,log);
	int N = dim;
	log << "number of points = " << M << endl; 

	mxArray *mPtr =  mxCreateDoubleMatrix((mwSize)M, (mwSize)N, mxREAL);
	plhs[1] = mPtr; 
	double* ptr = mxGetPr(mPtr);
	for(int i = 0; i < N; ++ i)
	{
		for(int j = 0; j < M; ++ j)
		{
			ptr[i * M + j] = matrix.get( )[j * N + i];
		}
	}
	log << "points saved " << endl;
}
#define USAGE_INIT_2D "[front time points] = mexFT_Step(dim, [xlo xhi], [ylo yhi], N, tmax)"
#define USAGE_INIT_3D "[front time points] = mexFT_Step(dim, [xlo xhi], [ylo yhi], [zlo zhi], N, tmax)"
#define USAGE_STEP "[time points] = mexFT_Step(front, tstop)" 

/* gateway function */
void flink(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] )
{
	if (nrhs == 0) {
		showHelp( );
		return;
	}
	try {
		matlabLink::MatlabStruct mlStruct(prhs[0]);
		std::string command = mlStruct.str("command"); 
		bool debug = mlStruct.boolean("debug",false);
		matlabLink::mexStream log;
		if (!debug) {
			log.setstate(std::ios_base::badbit);
		}

		if (command == "init") {
			if (nlhs == 1) {
				Front * front = new Front( );
				initFront(front, mlStruct,log);
				plhs[0]  = mxCreateNumericMatrix(1, 1, mxINT64_CLASS, mxREAL);
				fronts.push_back(front);
				*static_cast<int64_t *>(mxGetData(plhs[0])) = fronts.size( ) - 1; 
			}
			else {
				mexErrMsgTxt("init requires single left hand argument");
				return;
			}
		}
		else if (command == "step") {
			int64_t index = mlStruct.int64s("front")[0];
			if (static_cast<size_t>(index) >= fronts.size( ) )  {
				VCELL_EXCEPTION_NOLOCATION(domain_error,"invalid front index " << index);
			}
			Front * front = fronts[index];
			if (front == nullptr) {
				VCELL_EXCEPTION_NOLOCATION(domain_error,"front index " << index << " no longer valid (prior call to 'free'?)");
			}
			step(front,nlhs,plhs,mlStruct,log);
		}
		else if (command == "free") {
			int64_t index = mlStruct.int64s("front")[0];
			if (static_cast<size_t>(index) >= fronts.size( ) )  {
				VCELL_EXCEPTION_NOLOCATION(domain_error,"invalid front index " << index);
			}
			Front * front = fronts[index];
			fronts[index] = nullptr;
			delete front;
		}
		else if (command == "help") {
			showHelp( );
		}
	} catch (std::exception &e) {
		mexErrMsgIdAndTxt("fronTierLink:exception",e.what( ));
	}
	/*
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
	*/
}
