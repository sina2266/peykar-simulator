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
*				cleanup.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	Contains functions for the setting of error handlers, the
*	clean up after the occurance of an error and  for printing 
*	advisory messages.
*
*	This file is to be independent of all other libs except
*	util.
*
*/

#include <cdecs.h>
#include <sys/types.h>
#include <time.h>

#if defined(__MPI__) && defined(_MIPS_ISA_MIPS4)
#  include <arraysvcs.h>
#endif /* defined(__MPI__) && defined(_MIPS_ISA_MIPS4) */

#include <signal.h>

#if defined(sparc) && !(defined(__SUNPRO_C) || defined(__SUNPRO_CC)) && !defined(__GNUC__)
#  include <floatingpoint.h>
#endif /* defined(sparc) && !(defined(__SUNPRO_C) || defined(__SUNPRO_CC)) && !defined(__GNUC__)*/

#if defined(linux) && defined(HAS_FENV)
#  include <fenv.h>
#endif /* defined(linux) && defined(HAS_FENV) */


LOCAL	boolean	dump_core = NO;

#if !defined(_HPUX_SOURCE) && !(defined(__SUNPRO_C) || defined(__SUNPRO_CC)) && !defined(__GNUC__)
#   if defined(__cplusplus)
        extern "C"    int     sigsetmask(int);
#   else /* defined(__cplusplus) */
        extern	int	sigsetmask(int);
#   endif /* defined(__cplusplus) */
#endif /*!defined(_HPUX_SOURCE) && !(defined(__SUNPRO_C) || defined(__SUNPRO_CC)) && !defined(__GNUC__)*/

#if defined(mips) && !defined(_MIPS_ISA_MIPS4)

#include <siginfo.h>
#include <ucontext.h>

#if defined(__cplusplus)
extern "C" {
#   endif /* defined(__cplusplus) */

typedef void (Error_handler)(int,siginfo_t*,ucontext_t*);
LOCAL	Error_handler	mips_clean_up;
LOCAL	Error_handler	mips_second_clean_up;
LOCAL	Error_handler	mips_exit;

#if defined(__cplusplus)
}
#   endif /* defined(__cplusplus) */


#define	CLEAN_UP	mips_clean_up
#define	SECOND_CLEAN_UP	mips_second_clean_up
#define	EXIT		mips_exit
#define SA_FLAGS	SA_SIGINFO

#else /* defined(mips) && !defined(_MIPS_ISA_MIPS4) */

#if defined(__cplusplus)
extern "C" {
#   endif /* defined(__cplusplus) */

typedef void (Error_handler)(int);
LOCAL   Error_handler   clean_up_handler;
LOCAL	Error_handler	second_clean_up_handler;
LOCAL	Error_handler	terminate_run_handler;

#if defined(__cplusplus)
}
#   endif /* defined(__cplusplus) */


#define	CLEAN_UP	clean_up_handler
#define	SECOND_CLEAN_UP	second_clean_up_handler
#define	EXIT		terminate_run_handler
#define SA_FLAGS	0

#endif /* defined(mips) && !defined(_MIPS_ISA_MIPS4) */

#if defined(mips)
LOCAL	void		mips_fpe_handler(unsigned int [5],int [2]);
LOCAL	void		set_mips_floating_traps(void);
#endif /* defined(mips) */
#if defined(linux) && defined(HAS_FENV)
LOCAL	void		set_linux_floating_traps(void);
#endif /* defined(linux) && defined(HAS_FENV) */


	/* LOCAL Function Prototypes */
LOCAL	const char *error_name(int);
LOCAL	void	clear_pp_trap(int);
LOCAL	void	print_final_storage_and_error(int);
LOCAL	void	second_clean_up(int);
LOCAL	void	set_error_handler(Error_handler*,boolean);
LOCAL	void	terminate_run(int);

	/* Function pointer for clean up printout */
LOCAL	void	(*clean_up_printout)(int) = NULL;
LOCAL	void	(*user_clean_up)(void) = NULL;

#if defined(__MPI__) && defined(_MIPS_ISA_MIPS4)
#define SIG_PP_TRAP		SIGUSR2
#endif /* defined(__MPI__) && defined(_MIPS_ISA_MIPS4) */

#if defined(DEBUG_CLEAN_UP)
#define debug_enter(fname) (void) printf("Entered %s()\n",#fname);
#define debug_leave(fname) (void) printf("Left %s()\n",#fname);
#define debug_mesg_and_int(mesg,sig) (void) printf("%s %d\n",mesg,sig);
#else /* defined(DEBUG_CLEAN_UP) */
#define debug_enter(fname)
#define debug_leave(fname)
#define debug_mesg_and_int(mesg,sig)
#endif /* defined(DEBUG_CLEAN_UP) */


EXPORT	void init_clean_up(
	void (*_user_clean_up)(void),
	void (*_clean_up_printout)(int))
{
	debug_enter(init_clean_up)

	user_clean_up = _user_clean_up;
	clean_up_printout = _clean_up_printout;

	set_error_handler(CLEAN_UP,YES); /* to call clean_up		  */

	debug_leave(init_clean_up)
}		/*end inti_clean_up*/




/*
*			clean_upp():
*
*		Clean Up - Print Execution Times, Flush Buffers:
*
*	Produces an execution trace if the argument is positive.
*	Also prints an error message.  Positive values of
*	the argument are reserved for software and hardware failures.
*/

EXPORT void clean_upp(
	int		error)
{
	static boolean	first = YES;
	int		call_printout;
	time_t		tvec;
	const char      *errstring;

	errstring = strerror(errno);
	debug_enter(clean_up)

	clear_pp_trap(error);

	if (first == YES)
	{
	    first = NO;
	    if (error == 0)
	       	call_printout = NO;
	    else
	        call_printout = (clean_up_printout) ? YES : NO;
	}
	else
	{
	    call_printout = NO;
	    second_clean_up(error);
	}
#if !defined(SIG_PP_TRAP)
	if (pp_numnodes() > 1)
	    call_printout = NO;
#endif /* !defined(SIG_PP_TRAP) */

	(void) printf("\n\t\tCLEAN_UP, error = %d",error);
	if (error != 0)
	{
	    const char *ename = error_name(error);
	    if (ename[0] != '\0')
		(void) printf(", %s, errno = %s\n",ename,errstring);
	}
	else
	    (void) printf("\n");

	    /* Ensure any error produced by clean_up() causes termination */

	set_error_handler(SECOND_CLEAN_UP,NO);

	    /* print search string to access cleanup */
	    /*  information easily in a binary file  */


	print_errors();

	if (error)
	{
	    print_call_stack(" FROM CLEAN UP");
	    debug_trace();
	    print_final_storage_and_error(error);
	}

	    /*    Attempt printout on cleanup only after all other   */
	    /* run/debugging information in cleanup has been printed */
	    /*   so that none is lost if statistics/printout fails   */
	    /*     Ensures that if printout during cleanup fails,    */
	    /*       the original error status will not be lost      */

	if (call_printout)
	    (*clean_up_printout)(error);
	
	print_final_storage_and_error(error);
	(void) time(&tvec);
	(void) printf("\n\t\tTERMINATION TIME OF RUN:   %s\n",ctime(&tvec));
	trace_output();

	debug_leave(clean_up)

	terminate_run(error);
}		/*end clean_upp*/

LOCAL void second_clean_up(
	int	error)
{

	debug_enter(second_clean_up)

	screen("ERROR clean_up() called twice\n");
	(void) printf("\n\t\tSECOND CLEAN_UP, error = %d",error);
	if (error != 0)
	{
	    const char *ename = error_name(error);
	    if (ename[0] != '\0')
		(void) printf(", %s\n",ename);
	}
	else
	    (void) printf("\n");

	set_error_handler(EXIT,NO);

	print_errors();

	debug_trace();
	print_final_storage_and_error(error);

	debug_leave(second_clean_up)

	user_clean_up = NULL;
	terminate_run(error);
}		/*end second_clean_up*/


LOCAL	void set_error_handler(
	Error_handler	*func,
	boolean		first)
{
#ifdef WIN32
#else
	int		i;
	struct sigaction Sigact;
	sigset_t	sa_mask;
static int SIGS[] = {
#if !defined(sparc) && !(defined(__SUNPRO_C) || defined(__SUNPRO_CC))
	                SIGFPE,
#endif /* !defined(sparc) && !(defined(__SUNPRO_C) || defined(__SUNPRO_CC)) */
	                SIGINT,
	                SIGBUS,
	                SIGSEGV,
	                SIGILL,
#if defined(SIGTERM)
	                SIGTERM,
#endif /* defined(SIGTERM) */
	                -1
	};

	debug_enter(set_error_handler)

#if !(defined(__SUNPRO_C) || defined(__SUNPRO_CC)) && !defined(__GNUC__)
	sigsetmask(0); /* Clear current signal mask */
#endif /* !(defined(__SUNPRO_C) || defined(__SUNPRO_CC)) && !defined(__GNUC__) */

	(void) sigemptyset(&sa_mask);
	Sigact.sa_handler = func;
	Sigact.sa_mask = sa_mask;
	Sigact.sa_flags = SA_FLAGS;

	if ((first == NO) &&  (dump_core == YES))
	    Sigact.sa_handler = SIG_DFL;

#if defined(sparc) && !(defined(__SUNPRO_C) || defined(__SUNPRO_CC)) && !defined(__GNUC__)

	if (ieee_handler("set","common",(sigfpe_handler_type) func) != 0)
	    (void) printf("ieee_handler can't set FPE handler \n");

#endif /* defined(sparc) && !(defined(__SUNPRO_C) || defined(__SUNPRO_CC)) && !defined(__GNUC__) */

	for (i = 0; SIGS[i] > 0; ++i)
	{
	    debug_mesg_and_int("Setting trap for signal ",SIGS[i])
	    (void) sigaction(SIGS[i],&Sigact,(struct sigaction *)NULL);
	}

#if defined(SIG_PP_TRAP)
	if (first == YES)
	    (void) sigaction(SIG_PP_TRAP,&Sigact,(struct sigaction *)NULL);
#endif /* defined(SIG_PP_TRAP) */

#if defined(mips)
	set_mips_floating_traps();
#endif /* defined(mips) */

#if defined(linux) && defined(HAS_FENV)
	set_linux_floating_traps();
#endif /* defined(linux) && defined(HAS_FENV) */

	debug_leave(set_error_handler)
#endif
}		/*end set_error_handler*/

#if defined(SIG_PP_TRAP)
LOCAL	void clear_pp_trap(
	int		error)
{
	IMPORT	boolean	allow_pp_comm;
	struct sigaction Sigact;
	sigset_t	sa_mask;
	static boolean	first = YES;

	if (error == 0 || first == NO) return;
	debug_enter(clear_pp_trap)
#if !(defined(__SUNPRO_C) || defined(__SUNPRO_CC)) && !defined(__GNUC__)
	sigsetmask(0); /* Clear current signal mask */
#endif /* !(defined(__SUNPRO_C) || defined(__SUNPRO_CC)) && !defined(__GNUC__) */
	(void) sigemptyset(&sa_mask);
	Sigact.sa_handler = SIG_IGN;
	Sigact.sa_mask = sa_mask;
	Sigact.sa_flags = SA_FLAGS;
	(void) sigaction(SIG_PP_TRAP,&Sigact,(struct sigaction *)NULL);

#if defined(_MIPS_ISA_MIPS4)
	if ((pp_numnodes() > 1) && (error != SIG_PP_TRAP))
	{
	    ash_t ASH = getash();
	    askillash_array(NULL,NULL,ASH,SIG_PP_TRAP);
	}
#endif /* defined(_MIPS_ISA_MIPS4) */

	allow_pp_comm = NO;/* Turn off parallel communication */
	first = NO;
	debug_leave(clear_pp_trap)
}		/*end clear_pp_trap*/
#else /* defined(SIG_PP_TRAP) */
/*ARGSUSED*/
LOCAL	void clear_pp_trap(
	int		error)
{
}		/*end clear_pp_trap*/
#endif /* defined(SIG_PP_TRAP) */


LOCAL	void print_final_storage_and_error(
	int	error)
{
	debug_enter(print_final_storage_and_error)

	if (error != 0 || debugging("FINAL_STORAGE")) 
	{
	    (void) printf("\n\nAllocated Storage at end of run\n\n");
	    long_alloc_view(stdout);
	}
	(void) printf("\nStorage used %d bytes\n",get_vmalloc_storage_use());

	print_execution_times();

	if (error != 0)
	{
	    const char *ename = error_name(error);
	    screen("\t\t\tERROR = %d\n",error);
	    if (ename[0] != '\0')
		screen("\t\t\t%s\n",ename);
	}

	debug_leave(print_final_storage_and_error)
}		/*end print_final_storage_and_error*/

LOCAL	const char *error_name(
	int error)
{
#ifdef WIN32
	return ""; //gcw 2015-02-19
#else
	switch (error) {
	case SIGHUP:
	    return "Hang up";
	case SIGINT:
	    return "Interrupt";
	case SIGQUIT:
	    return "Quit";
	case SIGILL:
	    return "Illegal Instruction";
	case SIGTRAP:
	    return "Trace Trap";
#if defined(SIGABRT)
	case SIGABRT:
	    return "ABORT";
#endif /* defined(SIGABRT) */
#if defined(SIGEMT)
	case SIGEMT:
	    return "Emulator Trap";
#endif /* defined(SIGEMT) */
	case SIGFPE:
	    return "Floating Point Exception";
	case SIGKILL:
	    return "Kill";
	case SIGBUS:
	    return "Bus Error";
	case SIGSEGV:
	    return "Segmentation Violation";
#if defined(SIGSYS)
	case SIGSYS:
	    return "SIGSYS";
#endif /* defined(SIGSYS) */
#if defined(SIGTERM)
	case SIGTERM:
	    return "SIGTERM";
#endif /* defined(SIGTERM) */
	case SIGPIPE:
	    return "";
	case SIGALRM:
	    return "";
#if defined(SIG_PP_TRAP)
	case SIG_PP_TRAP:
	    return "Parallel Trap";
#endif /* defined(SIG_PP_TRAP) */
	case -1:
	    return "Source Code Generated Error";
	case 0:
	    return "";
	default:
	    return "UNKNOWN SYSTEM ERROR";
	}
#endif
}		/*end error_name*/

EXPORT	void print_storage(
	const char *s,
	const char *s1)
{
	debug_enter(print_storage)

	if (debugging(s1) || debugging("ALL_storage"))
	{
	    (void) printf("Storage current %d bytes: %s\n",
	    get_vmalloc_storage_use(),s);
	    if (debugging("BIG_STORAGE")) alloc_view(stdout);
	}

	debug_leave(print_storage)
}		/*end print_storage*/

EXPORT	void set_dump_core(
	boolean		dodump)
{
	debug_enter(set_dump_core)

	dump_core = dodump;

	debug_leave(set_dump_core)
}		/*end set_dump_core*/

LOCAL	void	terminate_run(
	int	error)
{
	boolean call_user_clean_up;

	call_user_clean_up = (user_clean_up != NULL) ? YES : NO;
#if !defined(SIG_PP_TRAP)
	if ((call_user_clean_up == YES)  && (pp_numnodes() > 1))
	    call_user_clean_up = NO;
#endif /* !defined(SIG_PP_TRAP) */

	if (call_user_clean_up == YES)
	    (*user_clean_up)();

#if !defined(SIG_PP_TRAP)
	if ((pp_numnodes() > 1) && error)
	    pp_abort(error,dump_core);
#endif /* !defined(SIG_PP_TRAP) */

	(void) pp_finalize();

#if defined(_CRAYMPP)
	(void) fflush(stdout);
	if (error != 0)
	    globalexit(error);
#endif /* defined(_CRAYMPP) */
	if (dump_core == YES && error)
	    abort();
	exit(error);
}	/*end terminate_run*/

#if defined(mips) && !defined(_MIPS_ISA_MIPS4)

/*ARGSUSED*/
LOCAL	void	mips_clean_up(
	int		error,
	siginfo_t	*sip,
	ucontext_t	*up)
{
	clean_up(error);
}		/*end mips_clean_up*/

/*ARGSUSED*/
LOCAL	void	mips_second_clean_up(
	int		error,
	siginfo_t	*sip,
	ucontext_t	*up)
{
	second_clean_up(error);
}		/*end mips_second_clean_up*/

/*ARGSUSED*/
LOCAL	void	mips_exit(
	int		error,
	siginfo_t	*sip,
	ucontext_t	*up)
{
	terminate_run(error);
}		/*end mips_exit*/

#else /* defined(mips) && !defined(_MIPS_ISA_MIPS4) */

#if defined(__cplusplus)
extern "C" {
#   endif /* defined(__cplusplus) */

/*ARGSUSED*/
LOCAL	void	clean_up_handler(
	int		error)
{
	clean_up(error);
}		/*end clean_up_handler*/

/*ARGSUSED*/
LOCAL	void	second_clean_up_handler(
	int		error)
{
	second_clean_up(error);
}		/*end second_clean_up_handler*/

/*ARGSUSED*/
LOCAL	void	terminate_run_handler(
	int		error)
{
	terminate_run(error);
}		/*end mips_exit*/

#if defined(__cplusplus)
}
#   endif /* defined(__cplusplus) */

#endif /* defined(mips) && !defined(_MIPS_ISA_MIPS4) */

#if defined(mips) && defined(_MIPS_ISA_MIPS4)
#include <sigfpe.h>

/*ARGSUSED*/
LOCAL	void	mips_fpe_handler(
	unsigned int	exception_parameters[5],
	int		value[2])
{
	double    *vd;
	TRUEfloat *vf;
	long      *vl;
	int       *vi;
	screen("Mips floating point trap sprung\n"
	       "Exception type = %d, ",exception_parameters[0]);
	switch (exception_parameters[0])
	{
	case _UNDERFL:
	    screen("_UNDERFL\n");
	    break;
	case _OVERFL:
	    screen("_OVERFL\n");
	    break;
	case _DIVZERO:
	    screen("_DIVZERO\n");
	    break;
	case _INVALID:
	    screen("_INVALID\n");
	    break;
	case _INT_OVERFL:
	    screen("_INT_OVERFL\n");
	    break;
	case _NO_EXCEPTION_TYPE:
	    screen("_NO_EXCEPTION_TYPE\n");
	    break;
	case _ALL_EXCEPTION_TYPES:
	    screen("_ALL_EXCEPTION_TYPES\n");
	    break;
	default:
	    screen("UNKNOWN EXCEPTION TYPE\n");
	    break;
	}
	screen("Invalid action = %d, ",exception_parameters[1]);
	switch (exception_parameters[1])
	{
	case _SET_RESULT:
	    screen("_SET_RESULT\n");
	    break;
	case _REPL_RS:
	    screen("_REPL_OPERAND/_REPL_RS\n");
	    break;
	case _REPL_RT:
	    screen("_REPL_RT\n");
	    break;
	default:
	    screen("UNKNOWN INVALID ACTION\n");
	    break;
	}
	screen("Invalid type = %d, ",exception_parameters[2]);
	switch (exception_parameters[2])
	{
	case 0:
	    screen("ignored\n");
	    break;
	case _SQRT_NEG_X:
	    screen("_SQRT_NEG_X\n");
	    break;
	case _CVT_OVERFL:
	    screen("_CVT_OVERFL\n");
	    break;
	case _CVTW_OVERFL:
	    screen("_CVTW_OVERFL\n");
	    break;
	case _CVTW_NAN:
	    screen("_CVTW_NAN\n");
	    break;
	case _CVTW_INF:
	    screen("_CVTW_INF\n");
	    break;
	case _UNORDERED_CMP:
	    screen("_UNORDERED_CMP\n");
	    break;
	case _SNAN_OP:
	    screen("_SNAN_OP\n");
	    break;
	default:
	    screen("UNKNOWN INVALID TYPE\n");
	    break;
	}
	screen("Value type = %d, ",exception_parameters[3]);
	switch (exception_parameters[3])
	{
	case _SINGLE:
	    screen("_SINGLE\n");
	    break;
	case _DOUBLE:
	    screen("_DOUBLE\n");
	    break;
	case _WORD:
	    screen("_WORD\n");
	    break;
	case _LONGWORD:
	    screen("_LONGWORD\n");
	    break;
	default:
	    screen("UNKNOWN VALUE TYPE\n");
	    break;
	}
	screen("Value sign = %d, ",exception_parameters[4]);
	switch (exception_parameters[4])
	{
	case _POSITIVE:
	    screen("_POSITIVE\n");
	    break;
	case _NEGATIVE:
	    screen("_NEGATIVE\n");
	    break;
	default:
	    screen("UNKNOWN VALUE SIGN\n");
	    break;
	}

	screen("value = 0x%x%x\n",value[0],value[1]);
	switch (exception_parameters[3])
	{
	case _SINGLE:
	    vf = (TRUEfloat*)value;
	    screen("value = %g\n",*vf);
	    break;
	case _DOUBLE:
	    vd = (double*)value;
	    screen("value = %g\n",*vd);
	    break;
	case _WORD:
	    vi = (int*)value;
	    screen("value = %d\n",*vi);
	    break;
	case _LONGWORD:
	    vl = (long*)value;
	    screen("value = %l\n",*vl);
	    break;
	default:
	    break;
	}
	clean_up(SIGFPE);
}		/*end mips_fpe_handler*/

LOCAL	void	set_mips_floating_traps(void)
{
	if (!debugging("trap_floats")) return;

	sigfpe_[_UNDERFL].repls = _FLUSH_ZERO;
	sigfpe_[_OVERFL].repls = _USER_DETERMINED;
	sigfpe_[_DIVZERO].repls = _USER_DETERMINED;
	sigfpe_[_INVALID].repls = _USER_DETERMINED;
	handle_sigfpes(_ON,_EN_UNDERFL|_EN_OVERFL|_EN_DIVZERO|_EN_INVALID,
		       mips_fpe_handler,_ABORT_ON_ERROR,0);
}		/*end set_mips_floating_traps*/

#endif /* defined(mips) && defined(_MIPS_ISA_MIPS4) */

#if defined(mips) && defined(__TRACE_BACK__)
#include <libexc.h>
#endif /* defined(mips) && defined(__TRACE_BACK__) */

/*ARGSUSED*/
EXPORT	void	print_call_stack(const char *mesg)
{
#if defined(mips) && defined(__TRACE_BACK__)
	debug_enter(print_call_stack)
	(void) printf("\nFUNCTION CALL STACK%s\n",mesg);
	(void) trace_back_stack_and_print();
	(void) printf("END FUNCTION CALL STACK%s\n\n",mesg);
	debug_leave(print_call_stack)
#endif /* defined(mips) && defined(__TRACE_BACK__) */
}		/*end print_call_stack*/


EXPORT	int is_floating_point(double x)
{
	return isnan(x);
}


#if defined(linux) && defined(HAS_FENV)
LOCAL	void	set_linux_floating_traps(void)
{
    if (debugging("trapfpe"))
    {
	feenableexcept(FE_DIVBYZERO);
	feenableexcept(FE_INVALID);
	feenableexcept(FE_OVERFLOW);
    }
}		/*end set_linux_floating_traps*/
#endif /* defined(linux) && defined(HAS_FENV) */
