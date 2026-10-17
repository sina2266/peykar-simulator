/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef FVUTILS_H
#define FVUTILS_H

#include <VCELL/SimTypes.h>

#include <string>
using std::string;


extern "C"
{
#if defined(FORTRAN_BARE)
	#define PCGWRAPPER pcgwrapper
	#define IDAMAX idamax
	#define DSCAL dscal
	#define DAXPY daxpy
	#if defined(FORTRAN_CAPITAL) || defined(FORTRAN_UNDERSCORE)
		#error "Only one fortran convention should be defined"
	#endif

#elif defined(FORTRAN_CAPITAL)
	//no operation required
	#if defined(FORTRAN_BARE) || defined(FORTRAN_UNDERSCORE)
		#error "Only one fortran convention should be defined"
	#endif
#elif defined(FORTRAN_UNDERSCORE)

	#define PCGWRAPPER pcgwrapper_
	#define IDAMAX idamax_
	#define DSCAL dscal_
	#define DAXPY daxpy_
	//extern void PCGWRAPPER(...);
	//extern void DSCAL(...);
	//extern void PCILU(...);
	//extern void DAXPY(...);
	#if defined(FORTRAN_BARE) || defined(FORTRAN_CAPITAL)
		#error "Only one fortran convention should be defined"
	#endif
#else
	#error "Fortran linking convention not specified"
#endif
	int IDAMAX(long *, double*, int*);
	void PCGWRAPPER(long *, long *, int *, int32 *, double *, double *, double *, double *, int *, double *, double *, double *, double*);	
	void DSCAL(int *, double *, double *, int *); 
	void DAXPY(long*, double*, double*, int*, double*, int*);
}

double computeRHSscale(long length, double* rhs, string& varname);
void throwPCGExceptions(int errorCode, int additional);
bool isNAN(double number);
bool isInfinity(double number);
void validateNumber(string& variableName, int index, char* coeffName, double coeffValue);
void sortColumns(int numCols, int* columnIndices, double* columnValues);
void trimString(string& str);

#ifndef SUNDIALS_PDE_SOLVER
#define SUNDIALS_PDE_SOLVER "SUNDIALS_PDE_SOLVER"
#endif

#ifndef FV_SOLVER
#define FV_SOLVER "FV_SOLVER"
#endif

#endif
