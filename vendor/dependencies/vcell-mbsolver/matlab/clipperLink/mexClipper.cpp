#include <fstream>
#include "matlabStruct.h"
#include <intersection.h>
#include "explore.h"
#include <MBridge/FronTierAdapt.h>
#include <MBridge/Figure.h>
#include <stdint.h>

#ifdef _WIN32
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#ifndef NOMINMAX
		#define NOMINMAX
	#endif
	#ifndef NOGDI
		#define NOGDI
	#endif
	#include <windows.h>
	#ifdef POINT
		#undef POINT
	#endif
	#ifdef FIXED
		#undef FIXED
	#endif
#endif

using spatial::Point2D;
void mexFunctionClipper(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
	if (nrhs == 0) {
		mexPrintf("Description: calls vcell implementation of polygon clipping\n");
		mexPrintf("Usage: pass in structure with required fields\n");
		mexPrintf("\tx1, y1: 1 x n arrays of first polygon points\n");
		mexPrintf("\tx2, y2: 1 x n arrays of second polygon points\n");
		mexPrintf("Optional arguments:\n");
		mexPrintf("\tfilename: result file name (will be overwritten if exists.\n");
		mexPrintf("\t\toutput file options (no effect if filename not provided)\n");
		mexPrintf("\t\t\tdrawinput: plot input polygons (if non-empty string)\n");
		mexPrintf("\t\t\tfigurename: MATLAB figure name\n");
		mexPrintf("\t\t\tdrawoptions: 'plot' options for result polygon(s)\n");
		mexPrintf("\t\t\tinputoptions: 'plot' options for input polygons\n"); 
		mexPrintf("\tversion: (logical) display library version number\n");
		mexPrintf("Returns: structure containing x and y arrays of resulting polygon\n");
		return;
	}
	if (nrhs != 1) {
		mexErrMsgTxt("Single argument of structure type required");
	}
	//	analyze_structure(prhs[0]);
	matlabLink::MatlabStruct mls(prhs[0]);
	{
		const bool showVer = mls.boolean("version", false);
		if (showVer) {
			mexPrintf("Moving Boundary version %s\n","unknown - integrate with github version - TODO");
		}
	}
	typedef matlabLink::MData<double> DS;
	DS x1 = mls.doubles("x1");
	DS y1 = mls.doubles("y1");
	if (x1.size != y1.size) {
		mexPrintf("x1 size %d and y1 size %d differ\n",x1.size, y1.size);
		mexErrMsgTxt("invalid input");
	}
	DS x2 = mls.doubles("x2");
	DS y2 = mls.doubles("y2");
	if (x2.size != y2.size) {
		mexPrintf("x2 size %d and y2 size %d differ\n",x2.size, y2.size);
		mexErrMsgTxt("invalid input");
	}
	std::vector<Point2D> a;
	for (unsigned int i = 0; i < x1.size; i++) {
		a.push_back(Point2D(x1.data[i],y1.data[i]));
	}
	std::vector<Point2D> b;
	for (unsigned int i = 0; i < x1.size; i++) {
		b.push_back(Point2D(x2.data[i],y2.data[i]));
	}
#ifdef WIN32
	LARGE_INTEGER start = {0,0};
	BOOL r = QueryPerformanceCounter(&start);
#endif

	spatial::Volume<double,double,2>  result;
	spatial::intersections(result,a,b);

#ifdef WIN32
	LARGE_INTEGER end = {0,0};
	r *= QueryPerformanceCounter(&end);
	LARGE_INTEGER freq = {0,1};
	r *= QueryPerformanceFrequency(&freq);
	if (r == 0) {
		mexPrintf("Windows API timing failure"); 
	}
	else {
		const long double delta = static_cast<long double>(end.QuadPart - start.QuadPart);
		const long double time = delta/freq.QuadPart;
		mexPrintf("spatial::intersection call took %e seconds\n",time);
	}
#endif

	typedef std::vector<spatial::TPoint<double,2> > PointVector;
	std::vector< PointVector > vecs = result.points( );
	if (vecs.size( ) == 0) {
		mexPrintf("No intersection found\n");
		return;
	}
	std::string filename = mls.str("filename", false);
	if (filename.length( ) > 0) {
		std::string figureName = mls.str("figurename", false);
		std::string options = mls.str("drawoptions","k+-");
		std::string dinput = mls.str("drawinput",false);
		std::string inputoptions = mls.str("inputoptions","g+-");
		bool drawInput = false;
		if (dinput.length( ) > 0) {
			drawInput = true;
		}

		std::ofstream file(filename);
		if (figureName.length( ) != 0) {
			file << matlabBridge::FigureName(figureName.c_str( ));
		}
		if (drawInput) {
			matlabBridge::Polygon pgonA(inputoptions.c_str( ));
			matlabBridge::Polygon pgonB(inputoptions.c_str( ));
			frontTierAdapt::copyVectorInto(pgonA,a);
			frontTierAdapt::copyVectorInto(pgonB,b);
			file << pgonA << pgonB;
		}
		for (std::vector< PointVector >::const_iterator iter = vecs.begin( ); iter != vecs.end( ); ++iter) {
			const PointVector & pv = *iter;
			matlabBridge::Polygon pgon(options.c_str( ));
			frontTierAdapt::copyVectorInto(pgon,pv);
			file << pgon;
		}
	}
	if (nlhs == 1) {
		const char *cartesian[] = {"x","y"}; //this sets field numbers used in mxSetFieldByNumber
		mwSize dims[] = {1,0};
		dims[1] = vecs.size( );
		mxArray *output = mxCreateStructArray(2,dims,2,cartesian);
		plhs[0] = output;

		for (size_t field = 0; field < vecs.size( ); ++ field) {
			const PointVector & pv = vecs[field];
			mxArray *xML = mxCreateDoubleMatrix(1,pv.size( ),mxREAL);
			mxArray *yML = mxCreateDoubleMatrix(1,pv.size( ),mxREAL);
			double *xout = mxGetPr(xML);
			double *yout = mxGetPr(yML);
			for (size_t i = 0; i < pv.size( ); i++) {
				xout[i] = pv[i](spatial::cX);
				yout[i] = pv[i](spatial::cY);
			}
			mxSetFieldByNumber(output,field,0,xML);
			mxSetFieldByNumber(output,field,1,yML);
		}
	}
}