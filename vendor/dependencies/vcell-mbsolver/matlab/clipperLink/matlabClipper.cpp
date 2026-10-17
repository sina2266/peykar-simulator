#include <fstream>
#include "matlabStruct.h"
#include <clipper.hpp>
#include "explore.h"
#include <MBridge/FronTierAdapt.h>
#include <MBridge/Figure.h>
#include <stdint.h>
#include <cassert>

#ifdef _WIN32
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#ifndef NOMINMAX
		#define NOMINMAX
	#e	ndif
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

typedef matlabLink::MData<int64_t> iArray;

void fillClipper(const iArray & xa, const iArray &ya, ClipperLib::Path &dest) {
	assert(xa.size == ya.size);
	for (int i = 0; i < xa.size; i++) {
		dest[i].X = xa.data[i];
		dest[i].Y = ya.data[i];
	}
}

void matlabClipper(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
	if (nrhs !=3) {
		mexPrintf("Description: calls vcell implementation of polygon clipping\n");
		mexPrintf("Usage: pass 3 arguments");
		mexPrintf("\tstruct with .x and .y files of type integer");
		mexPrintf("\tstruct with .x and .y files of type integer");
		mexPrintf("\tThe numeral '1'");
		mexPrintf("Returns: structure containing x and y arrays of resulting polygon\n");
		return;
	}
	matlabLink::MatlabStruct one(prhs[0]);
	matlabLink::MatlabStruct two(prhs[1]);
	iArray x1 = one.int64s("x");
	iArray y1 = one.int64s("y");
	if (x1.size != y1.size) {
		mexPrintf("x1 size %d and y1 size %d differ\n",x1.size, y1.size);
		mexErrMsgTxt("invalid input");
	}
	iArray x2 = two.int64s("x");
	iArray y2 = two.int64s("y");
	if (x2.size != y2.size) {
		mexPrintf("x2 size %d and y2 size %d differ\n",x2.size, y2.size);
		mexErrMsgTxt("invalid input");
	}
	//mexPrintf("%d %d %d %d arguments validated",x1.size,y1.size,x2.size,y2.size);
	typedef std::vector<spatial::TPoint<double,2> > PointVector;

	//this could be smarter as the cost of generality
	ClipperLib::Path clipperP1(x1.size);
	fillClipper(x1,y1,clipperP1);
	ClipperLib::Path clipperP2(x2.size);
	fillClipper(x2,y2,clipperP2);
	ClipperLib::Paths results;

	ClipperLib::Clipper c;
	c.AddPath(clipperP1,ClipperLib::ptSubject,true);
	c.AddPath(clipperP2,ClipperLib::ptClip,true);
	c.Execute(ClipperLib::ctIntersection,results,ClipperLib::pftEvenOdd,ClipperLib::pftEvenOdd);
	if (results.size( ) != 1) {
		mexPrintf("intersection has %d regions,", results.size( ));
		mexErrMsgTxt("Invalid intersection");
	}
	ClipperLib::Path result =  results.front( );
	const size_t size= result.size( );

	if (nlhs == 1) {
		const char *cartesian[] = {"x","y"}; //this sets field numbers used in mxSetFieldByNumber
		mwSize dims[] = {1,1};
		//dims[1] = size ;
		mxArray *output = mxCreateStructArray(2,dims,2,cartesian);
		plhs[0] = output;

			mxArray *xML = mxCreateDoubleMatrix(1,size,mxREAL);
			mxArray *yML = mxCreateDoubleMatrix(1,size,mxREAL);
			double *xout = mxGetPr(xML);
			double *yout = mxGetPr(yML);
			for (size_t i = 0; i < result.size( ); i++) {
				const ClipperLib::IntPoint & point = result[i];
				xout[i] = static_cast<double>(point.X); 
				yout[i] = static_cast<double>(point.Y); 
			}
			mxSetFieldByNumber(output,0,0,xML);
			mxSetFieldByNumber(output,0,1,yML);
		}
}
