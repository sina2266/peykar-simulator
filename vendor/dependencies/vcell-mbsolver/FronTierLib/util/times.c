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
*			times.c
*
*			Routines for timing purposes.
*
*	For the most part we define the cpu time of a process as the
*	sum of its user and system times and of those of all its terminated
*	subprocesses.
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*/

#include <cdecs.h>

#include <sys/types.h>
#include <time.h>
#ifndef WIN32
#include <sys/times.h>
#endif
#if !defined(CLK_TCK)
#define CLK_TCK sysconf(_SC_CLK_TCK)
#endif /* !defined(CLK_TCK) */

/* LINTLIBRARY */

	/* LOCAL Function prototypes*/
LOCAL	void	print_rss_usage(void);

LOCAL	int  MAX_TIMES = 0;	/* Length of cputime stack */
LOCAL	double *cputime = NULL;	/* Stack for  Storing Recursive Times */
LOCAL int  top = 0;			/* Pointer To top of Stack */



EXPORT void print_execution_times(void)
{
	size_t	   usert_min, usert_sec, usert_day, usert_hr;
	size_t	   syst_min, syst_sec, syst_day, syst_hr;
	size_t     clk_tck;
	clk_tck = CLK_TCK;
#ifdef WIN32
	usert_sec = 0;
	usert_day = (usert_sec/86400);
	usert_hr = (usert_sec/3600) % 24;
	usert_min = (usert_sec/60) % 60;
	usert_sec = usert_sec%60;
	syst_sec = 0;
	syst_day = (syst_sec/86400);
	syst_hr = (syst_sec/3600) % 24;
	syst_min = (syst_sec/60) % 60;
	syst_sec = syst_sec%60;
#else
	struct tms Tm;	
	(void) times(&Tm);

	usert_sec = (Tm.tms_utime+Tm.tms_cutime)/clk_tck;
	usert_day = (usert_sec/86400);
	usert_hr = (usert_sec/3600) % 24;
	usert_min = (usert_sec/60) % 60;
	usert_sec = usert_sec%60;
	syst_sec = (Tm.tms_stime+Tm.tms_cstime)/clk_tck;
	syst_day = (syst_sec/86400);
	syst_hr = (syst_sec/3600) % 24;
	syst_min = (syst_sec/60) % 60;
	syst_sec = syst_sec%60;
#endif

	screen("\n\n\t\tEXECUTION TIME DATA:\n\n\t\tCPU TIME: ");
	if (usert_day) screen("%d Day%s ",usert_day,(usert_day==1)?"":"s");
	if (usert_hr) screen("%d Hour%s ",usert_hr,(usert_hr==1)?"":"s");
	if (usert_min) screen("%d Minute%s ",usert_min,(usert_min==1)?"":"s");
	screen("%d Second%s",usert_sec,(usert_sec==1)?"":"s");
	screen("\n\t\tSYS TIME: ");
	if (syst_day) screen("%d Day%s ",syst_day,(syst_day==1)?"":"s");
	if (syst_hr) screen("%d Hour%s ",syst_hr,(syst_hr==1)?"":"s");
	if (syst_min) screen("%d Minute%s ",syst_min,(syst_min==1)?"":"s");
	screen("%d Second%s\n",syst_sec,(syst_sec==1)?"":"s");
	print_rss_usage();
}		/*end print_execution_times*/





EXPORT void start_clock(
	const char	*s)
{
	int	i;

	if (!debugging("CLOCK"))
	    return;

	if (cputime == NULL)
	{
	    MAX_TIMES = 40;
	    cputime = (double*)malloc(MAX_TIMES*sizeof(double));
	    zero_scalar(cputime,MAX_TIMES*sizeof(double));
	}

	if  (top >= MAX_TIMES)
	{
	    double	*new_cputime;
	    int	NEW_MAX_TIMES = 2*MAX_TIMES;

	    new_cputime = (double*)malloc(NEW_MAX_TIMES*sizeof(double));
	    zero_scalar(new_cputime,NEW_MAX_TIMES*sizeof(double));
	    for (i = 0; i < MAX_TIMES; i++)
	    	new_cputime[i] = cputime[i];
	    MAX_TIMES = NEW_MAX_TIMES;
	}

	for (i = 0; i < top; i++)
	    (void) printf("  ");
	(void) printf("CLOCK           (%s)\n",s);
	cputime[top] = cpu_seconds();
	top++;
}		/*end start_clock*/


EXPORT void stop_clock(
	const char	*s)
{
	if (!debugging("CLOCK"))
	    return;
	if (cputime == NULL)
	    return;
	top--;
	if (top < 0)
	{
	    (void) printf("ERROR: stop_clock(%s): CLOCK STACK EMPTY\n",s);
	    top = 0;
	}
	else if (top >= MAX_TIMES) 
	    (void) printf("ERROR: stop_clock(%s): CLOCK STACK FULL\n",s);
	else
	{
	    int i;
	    for (i = 0; i < top; i++)
		(void) printf("  ");
	    (void) printf("CLOCK  %6.2f   (%s)\n",
	    		  (cpu_seconds() - cputime[top]),s);
	}
}		/*end stop_clock*/







EXPORT void cpu_time(
	const char	*s)
{
	(void) printf("TIME: %7.2f  at %s\n", cpu_seconds(), s);
}		/*end cpu_time*/




/*
*				cpu_seconds():
*
*	Returns the number of CPU seconds used by the program to this point
*	Includes both the user and system time for both the program and
*	all its terminated subprocesses.
*/


EXPORT double cpu_seconds(void)
{	
	double		cpu_time;
	double		clk_tck;
	clk_tck = (double)CLK_TCK;

#ifndef WIN32
	struct tms	Tm;
	(void) times(&Tm);
	cpu_time = Tm.tms_utime + Tm.tms_cutime;
#endif
	return cpu_time/clk_tck;
}		/*end cpu_seconds*/




/*
*				real_time():
*
*	Returns the current real time in seconds measured from
*	some constant date.
*/

EXPORT double real_time(void)
{
	time_t		tvec;
	static time_t	stvec;
	static int	first = 1;

	if (first)
	{
		first = 0;
		(void) time(&stvec);
	}

	(void) time(&tvec);
	return (double)(tvec-stvec);
}		/*end real_time*/



EXPORT char *date_string(void)
{
	time_t	tvec;

	time (&tvec);
	return ctime(&tvec);
}		/*end date_string*/

/*  #bjet2 */
#define    ADD_MAX_LEN   20
LOCAL int  add_top = 0;
LOCAL double add_time[ADD_MAX_LEN], add_time_st[ADD_MAX_LEN];


EXPORT void add_time_start(int ind)
{
	add_time_st[ind] = cpu_seconds();
}

EXPORT void add_time_end(int ind)
{
	add_time[ind] += cpu_seconds() - add_time_st[ind];
}

EXPORT void add_time_clear(int ind)
{
	add_time[ind] = 0.0;
}

EXPORT double add_time_get(int ind)
{
	return  add_time[ind];
}


#if !defined(WIN32) && !(defined(__SUNPRO_C) || defined(__SUNPRO_CC)) && !defined(_HPUX_SOURCE) && !defined(cray) && !defined(tflops)
#include <sys/time.h>
#include <sys/resource.h>

LOCAL	void	print_rss_usage(void)
{
	struct rusage	Rusg, Crusg;
	int		pagesz = 1;

	(void) getrusage(RUSAGE_SELF,&Rusg);
	(void) getrusage(RUSAGE_CHILDREN,&Crusg);

#if defined(sparc)
	pagesz = getpagesize()/1024;
#endif /* defined(sparc) */
	screen("\t\tMAX RSS = %d Kilobytes\n",
		(Rusg.ru_maxrss+Crusg.ru_maxrss)*pagesz);
}		/*end print_rss_usage*/
#else /* !(defined(__SUNPRO_C) || defined(__SUNPRO_CC)) && !defined(_HPUX_SOURCE) && !defined(cray) && !defined(tflops) */

/*ARGSUSED*/
LOCAL	void	print_rss_usage(void)
{
}		/*end print_rss_usage*/

#endif /* !(defined(__SUNPRO_C) || defined(__SUNPRO_CC)) && !defined(_HPUX_SOURCE) && !defined(cray) && !defined(tflops) */

