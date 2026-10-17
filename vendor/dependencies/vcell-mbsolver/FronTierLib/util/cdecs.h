/************************************************************************************
FronTier is a set of libraries that implements differnt types of Front Traking algorithms.
Front Tracking is a numerical method for the solution of partial differential equations
whose solutions have discontinuities.


Copyright (C) 1999 by The University at Stony Brook.


This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

******************************************************************************/


/************************************************************************************
FronTier is a set of libraries that implements differnt types of Front Traking algorithms.
Front Tracking is a numerical method for the solution of partial differential equations
whose solutions have discontinuities.


Copyright (C) 1999 by The University at Stony Brook.


This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

******************************************************************************/


/*
*		Useful Extensions to C language:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*/

#if !defined(_CDECS_H)
#define _CDECS_H

/* On Windows, when this header is pulled into a C++ translation unit (e.g. the
 * moving-boundary solver sources, which also include Boost/HDF5 and thus the
 * Windows SDK), include <windows.h> up front so the SDK's own FLOAT/CHAR/INT/
 * DOUBLE/FALSE/TRUE definitions are established BEFORE FronTier redefines those
 * identifiers below.  The SDK header's include guard turns any later inclusion
 * (via Boost, HDF5, ...) into a no-op, so FronTier's macros can no longer
 * corrupt the SDK headers.  FronTier's own C sources compile as LANGUAGE C and
 * are unaffected by this block.  NOGDI/NOMINMAX/WIN32_LEAN_AND_MEAN keep the
 * SDK surface minimal (NOGDI drops <wingdi.h>, whose `typedef ... FIXED`
 * clashes with FronTier's FIXED enum). */
#if defined(WIN32) && defined(__cplusplus)
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  ifndef NOGDI
#    define NOGDI
#  endif
#  include <windows.h>
   /* FronTier defines FALSE/TRUE/NO/YES as an enum (_boolean) further down;
    * drop the SDK's object-like macros so that enum is well-formed. */
#  undef FALSE
#  undef TRUE
#  undef NO
#  undef YES
#endif /* defined(WIN32) && defined(__cplusplus) */

#if defined(USE_OVERTURE)
#else
#if defined(linux)
#  if !defined(_GNU_SOURCE)
#    define _GNU_SOURCE
#  endif /*!defined(_GNU_SOURCE)*/
#endif /* defined(linux) */
#endif /* if defined(USE_OVERTURE) */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <ctype.h>
#include <math.h>
#if defined(__INTEL_COMPILER) || defined(__bg__)
#include <stdint.h>
#endif /* defined(__INTEL_COMPILER) || defined(__bg__)*/
#include <limits.h>
#include <float.h>
#include <errno.h>
#if defined(__MPI__)
#   include <mpi.h>
#endif /* defined(__MPI__) */
#if defined(__GD__)
#include <gd.h>
#include <gdfonts.h>
#include <gdfontl.h>
#include <gdfontt.h>
#endif /* defined(__GD__) */

#define ptr2ull(p) u_ptr2ull((void*)(p))

#if defined(mips) || defined(__GNUC__) || defined(linux) || __SUNPRO_CC>=0x500 || __SUNPRO_C>=0x500

#define HasGen 1

#else /* #define HasGen defined(mips) || defined(__GNUC__) || defined(linux) || __SUNPRO_CC>=0x500 || __SUNPRO_C>=0x500 */

#define HasGen 0

#endif /* #define HasGen defined(mips) || defined(__GNUC__) || defined(linux) || __SUNPRO_CC>=0x500 || __SUNPRO_C>=0x500 */

#if HasGen
#   include <libgen.h>
#endif /* HasGen */
#include <limits.h>

#if defined(USE_OVERTURE)
/* Including header files of overture into FronTier */
#include "FT_overture.h"
#endif /* defined(USE_OVERTURE) */

#undef HUGE
#define HUGE 1.e+18

		/* Machine Dependent Quantities: */
#if defined(WIN32)
#include <windows_math_func.h>
#elif defined(cray)
#   define isnan(x)     (NO)
#   define isnanf(x)    (NO)
#   define isnand(x)    (NO)
#elif defined(sun) && !(defined(__SUNPRO_C) || defined(__SUNPRO_CC))
#   define isnanf(x)    isnan(x)
#   define isnand(x)    isnan(x)
#elif defined(__GNUC__) || defined(__PGI__) || defined(__INTEL_COMPILER) || defined(__bg__)
#   if !defined(isnan)
#       define isnan(x) isnand(x)
#   endif /*!defined(isnan)*/
#elif !defined(__alpha) && !defined(__hpux) && !defined(linux) && !defined(_AIX)
#   include <ieeefp.h>
#   define isnan(x)    isnand(x)
#endif /* defined(cray) */

#define MACH_EPS DBL_EPSILON

#if defined(__hpux)
#if defined(isfinite)
#define finite(x) isfinite(x)
#endif /* defined(isfinite) */
#endif /* defined(__hpux) */

typedef float   TRUEfloat;
#define REAL double

#   define SFMT "22s"
#   define FFMT "24.18g"

typedef	double 	ALIGN;	/* Strictest alignment type of all data types */
#define	num_aligns(size)	(((size) + sizeof(ALIGN)-1)/sizeof(ALIGN))
#include <stdint.h> //gcw
#if defined(c_plusplus) || defined(__cplusplus)
extern "C" {
#endif

#if defined(__GNUC__)
//typedef u_int64_t uint64_t;
#elif defined(__alpha)
typedef long long int int64_t;
typedef unsigned long long uint64_t;
#elif defined(WIN32)
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#endif /*defined(linux) || defined(__alpha)*/

enum {
	Gets_BUF_SIZE = 513
};

		/* Macros for REAL variables and Vectors: */

typedef unsigned char byte;	/* Generic object of size 1, sizeof(byte) = 1 */
/*
#if defined(__cplusplus)
#define NO                 FALSE
#define FUNCTION_FAILED    FALSE
#define YES		   TRUE
#define FUNCTION_SUCCEEDED TRUE
#else  defined(__cplusplus) */
#ifdef WIN32
//#include <Windows.h>
#define sleep(x) FT_Sleep(x)
void __stdcall FT_Sleep(unsigned long);
#endif
enum _boolean { FALSE              = 0,
		NO                 = FALSE,
		FUNCTION_FAILED    = FALSE,
		TRUE               = 1,
		YES                = TRUE,
		FUNCTION_SUCCEEDED = TRUE};
typedef enum _boolean boolean;
/* #endif  defined(__cplusplus) */

#if defined(ERROR)
#  undef ERROR
#endif /* defined(ERROR) */
enum {ERROR=-1};

typedef  void	    *POINTER;	/* Pointer to Unknown Data Types. */
typedef  const void *CPOINTER;	/* Constant Pointer to Unknown Data Types. */

#define  ERROR_FLOAT  -HUGE_VAL	/* Returned by Float Functions on Errors */


		/* stuff for parallel computers */

enum {
	IO_NODE_ID = 0,    /* processor node used as IO node*/
		/* identifier tags for timed global op  */
	ALL_GATHER_TAG,
	GLOBAL_IOR_TAG,
	GLOBAL_ISUM_TAG,
	GLOBAL_SUM_TAG,
	GLOBAL_IMAX_TAG,
	GLOBAL_MAX_TAG,
	GLOBAL_IMIN_TAG,
	GLOBAL_MIN_TAG,
	SYNC_TAG,
	GLOBAL_STATUS_TAG,
	USER_MIN_MESG_ID = GLOBAL_STATUS_TAG + 10
};

#define is_io_node(id)  ((id) == IO_NODE_ID)

/*      Re-direct standard I/O to files:
*       This structure is used on both host and node: normally two
*       entirely different machies.
*/

typedef struct {
	char    stderr_file[80];
	char    stdin_data[80];
	char    stdin_argv[80];
	char    stdout_file[80];
} REDIRECT_FILES;

#define	pp_isend(tag,buf,len,node,request)				\
	u_pp_isend(tag,buf,len,node,request,__FILE__,__LINE__)

#define	pp_send(tag,buf,len,node)					\
	u_pp_send(tag,buf,len,node,__FILE__,__LINE__)

#define	pp_send_all(tag,buf,len)					\
	u_pp_send_all(tag,buf,len,__FILE__,__LINE__)

#define	pp_irecv(tag,source,buf,len,request)				\
	u_pp_irecv(tag,source,buf,len,request,__FILE__,__LINE__)

#define	pp_recv(tag,source,buf,len)					\
	u_pp_recv(tag,source,buf,len,__FILE__,__LINE__)

#define	pp_recv_any(tag,buf,len)					\
	u_pp_recv_any(tag,buf,len,__FILE__,__LINE__)

#define	pp_all_gather(sendbuf,sendcount,recvbuf,recvcount)		\
	u_pp_all_gather(sendbuf,sendcount,recvbuf,recvcount,__FILE__,__LINE__)

#define pp_bcast(root,buf,len)						\
	u_pp_bcast(root,buf,len,__FILE__,__LINE__)

	/* Enclosed Code Compiled iff Nonzero */
#define  DONT_COMPILE  0

		/*  Inline Functions - Be Careful: */

#if defined(__cplusplus) && !defined(__hpux)

#else /* defined(__cplusplus) && !defined(__hpux) */

#if !defined(max)
#  define     max(a,b)     (((a) > (b)) ? (a) : (b))
#endif /* !defined(max) */

#if !defined(min)
#define	    min(a,b)     (((a) < (b)) ? (a) : (b))
#endif /* !defined(min) */

#endif /* defined(__cplusplus) && !defined(__hpux) */

#define     sqr(x)       ((x)*(x))

/* Structure copying; num is the number of chars to be copied */

#define ft_assign(l,r,num)	(void) memcpy((void*)(l),(const void*)(r),num)

#define clean_up(num)   \
	screen("\n%s:%i: error: calling clean_up\n",__FILE__,__LINE__), \
	clean_upp( num);


/* Structure zeroing; num is the number of chars to be zeroed */

#define zero_scalar(s,num)	(void) memset((POINTER)(s),0,num)

enum {
	READ	  = 0,
	WRITE,
	READ_WRITE
};

struct _COMPLEX
{
        double real;
        double imag;
};
typedef struct _COMPLEX	COMPLEX;

#define   IMPORT        extern
#define   LOCAL         static
#define   EXPORT
#define   LIB_LOCAL

/****
*******************Proper use of IMPORT,  LOCAL,  and EXPORT ******************

The explanation of the LOCAL vs.  static is due to the split nature of
the construction in C.  The static declaration really does two things,
first it declares a variable to be static,  that is its value is retained
from one function call to another,  the second effect is that the variable
becomes local,  that is its access is restricted to the enclosing block.
Actually in C all variables declared within a block (brackets) are local,
so the second case only applies to external variables in a file.  The
default is that externals are global (ie known to all files),  so that to
restrict the scope of an external variable the static is declaration is
used.  Note that all external variable are by default static and global.
The defines EXPORT,  IMPORT,  and LOCAL were designed to explicitly
declare the scope of a given procedure or variable,  thus leaving the
declaration static to perform its first (primary) function.  Thus the
declarations should be used as follows

EXPORT - any variable or procedure with global scope,  this includes
all functions to be called outside of their file of definition,  as well
as any transfile global variables (these are strongly discouraged).

IMPORT - any variable or procedure which is defined elsewhere,  IMPORT
is really just another name for extern and serves exactly the same function.
IMPORTED variable are just that,  variables or procedures that are imported
from somewhere else.

LOCAL - any variable or procedure whose scope is to be limited to its
file of definition.

static - any variable (presumably within a block) whose value is static and
will not change between function calls to that block.

Even though in C LOCAL and static are the same,  we see that the division
of this single variable into two types provides a useful documentation
function by separating the static declaration from the access restriction.

*****/


#if defined(cray)
#   include <fortran.h>
#   if defined(__cplusplus)
#       define FORTRAN IMPORT "C"
#   else /* defined(__cplusplus) */
#       define	FORTRAN	fortran
#   endif /* defined(__cplusplus) */
#else /* defined(cray) */
#   if defined(__cplusplus)
#       if defined(FORTRAN)
#           undef FORTRAN
#       endif /* defined(FORTRAN) */
#       define FORTRAN IMPORT "C"
#   else /* defined(__cplusplus) */
#       define	FORTRAN	IMPORT
#   endif /* defined(__cplusplus) */
#endif /* defined(cray) */

#if defined(cray) || defined(_AIX) || defined(__bg__)
#   define   FORTRAN_NAME(a)    a
#else /* defined(cray) || defined(_AIX) */
#   define   FORTRAN_NAME(a)    a ## _
#endif /* defined(cray) || defined(_AIX) */
#if defined(__GNUC__) && !defined(LAHEY_FORTRAN)
#   define   SFORTRAN_NAME(a)   a ## _  /* GCC has a different naming */
					 /* * convention for FORTRAN routines */
					 /* * with character strings as */
					 /* * arguments. */

#else /* defined(__GNUC__) */
#   define   SFORTRAN_NAME(a)   FORTRAN_NAME(a)
#endif /* defined(__GNUC__) */

#if defined(LAHEY_FORTRAN)
#   define C_MAIN_PROGRAM MAIN__
int MAIN__(int,char**);
#else /* defined(LAHEY_FORTRAN) */
#   define C_MAIN_PROGRAM main
#endif /* defined(LAHEY_FORTRAN) */

typedef void (LSODE_FUNC)(int*,double*,double*,double*);
typedef void (LSODE_JAC)(int*,double*,double*,int*,int*,double*,int*);

#define	Null(x)		(!(x))
#if !defined(PI)
#   define	PI		3.14159265358979
#endif /* !defined(PI) */
#define	degrees(ang)	((ang)*180.0/PI)
#define radians(ang)    ((ang)*PI/180.0)

struct _Error {
	const char *filename;
	int        line_number;
	int        number;
	const char *message;
	struct _Error *next;
};

#define Error(__num,__mess)  log_error(__FILE__,__LINE__,__num,__mess)


/* Opaque holder for storing location in file for output */

typedef void OUTPUT;

#define	fclose(file)	Fclose(file)

	/* Simpleio Macros */

#define	print_double_matrix(title, rows, cols, mtrx, fmt) \
	fprint_double_matrix(stdout,title, rows, cols, mtrx, fmt)

#define	print_double_vector( title, n, vec, fmt) \
	fprint_double_vector(stdout, title, n, vec, fmt)

#define	print_double_vector_as_matrix( title, length, cols, vec, fmt) \
	fprint_double_vector_as_matrix(stdout, title, length, cols, vec, fmt)

#define	print_float_matrix( title, rows, cols, mtrx, fmt) \
	fprint_float_matrix(stdout, title, rows, cols, mtrx, fmt)

#define	print_float_vector( title, n, vec, fmt) \
	fprint_float_vector(stdout, title, n, vec, fmt)

#define	print_float_vector_as_matrix( title, length, cols, vec, fmt) \
	fprint_float_vector_as_matrix(stdout, title, length, cols, vec, fmt)

#define	print_matrix( title, rows, cols, mtrx, fmt) \
	fprint_matrix(stdout, title, rows, cols, mtrx, fmt)

#define	print_int_matrix( title, rows, cols, mtrx, fmt) \
	fprint_int_matrix(stdout, title, rows, cols, mtrx, fmt)

#define	print_int_vector_as_matrix( title, length, cols, vec, fmt) \
	fprint_int_vector_as_matrix(stdout, title, length, cols, vec, fmt)

#define	print_vector( title, n, vec, fmt) \
	fprint_vector(stdout, title, n, vec, fmt)

#define	print_vector_as_matrix( title,length, cols, vec, fmt) \
	fprint_vector_as_matrix(stdout, title,length, cols, vec, fmt)

#define	print_vector_of_floats( num, f) \
	fprint_vector_of_floats(stdout, num, f)


 /* True iff  x  lies between  y  and  z  */
#define  Between(x,y,z)   ( ((x)>=(y) && (x)<=(z)) || ((x)<=(y) && (x)>=(z)) )

#if DONT_COMPILE
 /* Alternate definition of Between, may be faster for floating comparisons but
  * is subject to inaccurate answers due to floating point degeneracies. May
  * also give inaccurate results for integer comparisons. */
#define  Between(x,y,z)   ( ((x)-(y)) * ((z)-(x)) >= 0. )
#endif /* DONT_COMPILE */

struct _Prompt_type {
	const char *prompt;	/* Full prompt name */
	const char *select;  	/* Abbreviated name for input */
	int	   ncmp;	/* # of chars to uniquely identify select */
	union {int itype; const char *ctype;} type;
};
typedef struct _Prompt_type Prompt_type;


	/* Possible values of variable  debug_mode: */

enum _DEBUG_MODE {
	PROMPT_FOR_DEBUG=-1,	/*Initiate debug prompting*/
	NONE=             0,	/* Indicates no debugging requested */
	SOME=             1,	/* Indicates debugging requested not with all */
	ALL=              2,	/* Indicates all was the first request */
	TRACE_ONLY=	  3	/* Trace debug calls but don't print messages*/
};
typedef enum _DEBUG_MODE DEBUG_MODE;

struct _DEBUG_PARAMS {
	DEBUG_MODE	_debug_mode;
};
typedef struct _DEBUG_PARAMS DEBUG_PARAMS;
#define	debug_params(prms)	((DEBUG_PARAMS*)(prms))
#define debug_mode(prms)	(debug_params(prms))->_debug_mode

/*
* Basic initialization structure.
*/

/*
* Machine Endian type
*/

enum _FT_ENDIAN {
	FT_LITTLE_ENDIAN  = -1, /* Little endian byte ordering */
	FT_UNKNOWN_ENDIAN =  0, /* Undetermined byte ordering */
	FT_BIG_ENDIAN     =  1 /* Big endian byte ordering */
};
typedef enum _FT_ENDIAN FT_ENDIAN;

/*
* Return status from quadrature programs
*/

enum _QUADRATURE_STATUS {
        ACCURATE_INTEGRAL    = 0,
	INACCURATE_INTEGRAL  = 1,
	INVALID_EPSILON      = 6
};
typedef enum _QUADRATURE_STATUS QUADRATURE_STATUS;

/* Reverse bytes of argument to convert endian sex */
#define byte_reverse(_x_)	reverse_string((char*)&_x_,sizeof(_x_))

/*
* IO conversion specifications
*/

struct _IO_TYPE {
	FILE        *file;
	size_t      read_float_size;
	size_t      cpu_float_size;
	FT_ENDIAN   read_endian;
	FT_ENDIAN   cpu_endian;
	boolean        reverse_endian;
};
typedef struct _IO_TYPE IO_TYPE;

struct _INIT_DATA {
	const char   *_title;
	boolean	     _interactive_prompting;
	DEBUG_PARAMS *_dbparams;	/*PROMPTED*/
};
typedef struct _INIT_DATA INIT_DATA;
#define init_data(init) ((INIT_DATA *)(init))
#define	interactive_prompting(init) init_data(init)->_interactive_prompting
#define	dbparams(init) init_data(init)->_dbparams
#define title(init) init_data(init)->_title

#if defined(c_plusplus) || defined(__cplusplus)
}
#endif

#include <fnamedebug.h>
#include <uprotos.h>

#endif /* !defined(_CDECS_H) */
