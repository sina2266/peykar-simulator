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
*			navdecs.h
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	Navigator code written by John D. Pinezich, 1999.
*/

#if !defined(_NAVDECS_H)
#define _NAVDECS_H
#include <cdecs.h>

#if defined(NAVIGATOR)

#define MAX_CHARS 50 /* Max Number of characters used per name */

#undef DEBUG_ENTER
#undef DEBUG_LEAVE
#undef NAV_ENTER
#undef NAV_LEAVE
#undef pindent

/* commands for nav_update() */

#define pINIT 		0
#define pENTER		1
#define pLEAVE		2
#define pSTATIC		3
#define pSPACE		4
#define pLIMIT          5
#define pPRINT_STACK  	6

/* commands for nav_trace(), function_monitor(), depth_monitor() */

#define pLOOK		-1
#define pSTOP		0
#define pSTART		1

/* commands for nav_switches() */

#define pBRIEFLY		0
#define pSUMMARY		1
#define pDETAILS		2
#define pEXPLAIN		3
#define pNAVIGATE		4
#define pDEPTH_LIMITING 	5
#define pFORCE			6
#define pADD_TO_DEBUG		7
#define pREMOVE_FROM_DEBUG	8
#define pSUPPRESS		9

/* commands for nav_integers() */

#define pBRIEFLY_CNT		0
#define pSUMMARY_CNT		1
#define pDETAILS_CNT		2
#define pEXPLAIN_CNT		3
#define pNAVIGATE_CNT		4
#define pFORCE_CNT		5
#define pADD_TO_DEBUG_CNT	6
#define pREMOVE_FROM_DEBUG_CNT	7
#define pC_COUNT		8
#define pLIMIT_VALUE		9
#define pLIMIT_CNT		10
#define pSUPPRESS_CNT		11
#define pINT0			12
#define pINT1			13

/* commands for nav_integer_pointers() */

#define pINTEGER0	0
#define pINTEGER1	1

/* commands for nav_float_pointers() */

#define pFLOAT0		0
#define pFLOAT1		1
#define pFLOAT2		2

/* commands for nav_pointers() */

#define pPOINTER	0

/* commands for nav_strings() */

#define pADD_TO_DEBUG_STRING		0
#define pREMOVE_FROM_DEBUG_STRING	1
#define pSTRING_BUG			2

/* nav macros */

/* macros for nav_integers */

#define BRIEFLY_CNT           (nav_integers(pBRIEFLY_CNT,fname_index))
#define SUMMARY_CNT           (nav_integers(pSUMMARY_CNT,fname_index))
#define DETAILS_CNT           (nav_integers(pDETAILS_CNT,fname_index))
#define EXPLAIN_CNT           (nav_integers(pEXPLAIN_CNT,fname_index))
#define NAVIGATE_CNT	      (nav_integers(pNAVIGATE_CNT,fname_index))
#define FORCE_CNT	      (nav_integers(pFORCE_CNT,fname_index))
#define ADD_TO_DEBUG_CNT      (nav_integers(pADD_TO_DEBUG_CNT,fname_index))
#define REMOVE_FROM_DEBUG_CNT (nav_integers(pREMOVE_FROM_DEBUG_CNT,fname_index))
#define C_COUNT	              (nav_integers(pC_COUNT,fname_index))
#define LIMIT_VALUE	      (nav_integers(pLIMIT_VALUE,fname_index))
#define LIMIT_CNT	      (nav_integers(pLIMIT_CNT,fname_index))
#define SUPPRESS_CNT	      (nav_integers(pSUPPRESS_CNT,fname_index))
#define INT0		      (nav_integers(pINT0,fname_index))
#define INT1		      (nav_integers(pINT1,fname_index))

/* macros for nav_integer_pointers */

#define INTEGER0              (nav_integer_pointers(pINTEGER0,fname_index))
#define INTEGER1              (nav_integer_pointers(pINTEGER1,fname_index))

/* macros for nav_float_pointers() */

#define FLOAT0		      (nav_float_pointers(pFLOAT0,fname_index))
#define FLOAT1		      (nav_float_pointers(pFLOAT1,fname_index))
#define FLOAT2		      (nav_float_pointers(pFLOAT2,fname_index))

/* macros for nav_pointers() */

#define POINTR		(nav_pointers(pPOINTER,fname_index))

/* macros for nav_strings() */

#define ADD_TO_DEBUG_STRING	   \
                  nav_strings(pADD_TO_DEBUG_STRING,fname_index)

#define REMOVE_FROM_DEBUG_STRING   \
                  nav_strings(pREMOVE_FROM_DEBUG_STRING,fname_index)

#define STRING_BUG		   \
                  nav_strings(pSTRING_BUG,fname_index)

/* macros for nav_trace() */

#define NAVIGATING	(nav_trace(pLOOK,"",0))
#define START_TRACE     nav_trace(pSTART,__FNAME__,fname_index);
#define STOP_TRACE      nav_trace(pSTOP,__FNAME__,fname_index);

#define LOOK          	(depth_monitor(pLOOK) && function_monitor(pLOOK))

/* macros for nav_switches() */

#define NAVIGATE	  (nav_switches(pNAVIGATE,fname_index)          \
			  && (NAVIGATE_CNT == -1 || NAVIGATE_CNT == C_COUNT))

#define BRIEFLY           (LOOK && nav_switches(pBRIEFLY,fname_index)  \
                          && (BRIEFLY_CNT == -1 || BRIEFLY_CNT == C_COUNT))

#define SUMMARY           (LOOK && nav_switches(pSUMMARY,fname_index)  \
                          && (SUMMARY_CNT == -1 || SUMMARY_CNT == C_COUNT))

#define DETAILS           (LOOK && nav_switches(pDETAILS,fname_index)  \
                          && (DETAILS_CNT == -1 || DETAILS_CNT == C_COUNT))

#define EXPLAIN           (LOOK && nav_switches(pEXPLAIN,fname_index)  \
                          && (EXPLAIN_CNT == -1 || EXPLAIN_CNT == C_COUNT))

#define FORCE	          (nav_switches(pFORCE,fname_index)             \
                          && (FORCE_CNT == -1 || FORCE_CNT == C_COUNT))

#define DEPTH_LIMITING	  (nav_switches(pDEPTH_LIMITING,fname_index)    \
                          && (LIMIT_CNT == -1 || LIMIT_CNT == C_COUNT))

#define SUPPRESS	  (nav_switches(pSUPPRESS,fname_index)          \
                          && (SUPPRESS_CNT == -1          ||           \
                               SUPPRESS_CNT != C_COUNT))

#define ADD_TO_DEBUG	  (nav_switches(pADD_TO_DEBUG,fname_index)      \
                          && (ADD_TO_DEBUG_CNT == -1      ||           \
                               ADD_TO_DEBUG_CNT == C_COUNT))

#define REMOVE_FROM_DEBUG (nav_switches(pREMOVE_FROM_DEBUG,fname_index) \
                          && (REMOVE_FROM_DEBUG_CNT == -1 ||           \
                               REMOVE_FROM_DEBUG_CNT == C_COUNT))

#define pindent  	if (NAVIGATING) (void) nav_update(pSPACE,"",0,0);
#define nprintf         pindent; (void) printf

#define BEGIN_BRIEFLY   if (BRIEFLY || FORCE) {
#define END_BRIEFLY     }

#define BEGIN_SUMMARY   if (SUMMARY || FORCE) {
#define END_SUMMARY     }

#define BEGIN_DETAILS   if (DETAILS) {
#define END_DETAILS     }

#define BEGIN_EXPLAIN   if (EXPLAIN) {
#define END_EXPLAIN     }

#define BEGIN_FORCE   	if (FORCE) {
#define END_FORCE     	}

#define DEBUG_VARIABLES(fname)                      \
        const char *__FNAME__ = #fname;             \
	static int fname_index = -2;                \
        int        reset_limit;                     \
	if (fname_index == -2)                      \
        {                                           \
	    fname_index = nav_index(__FNAME__);     \
	}                                                         

#define CHECK_START_TRACE 	if (NAVIGATE) {START_TRACE}
#define CHECK_STOP_TRACE 	if (NAVIGATE) {STOP_TRACE}

#define CHECK_ADD_TO_DEBUG	if (ADD_TO_DEBUG)      \
                                  {add_to_debug(ADD_TO_DEBUG_STRING);}
#define CHECK_REMOVE_FROM_DEBUG	if (REMOVE_FROM_DEBUG) \
                                  {remove_from_debug(REMOVE_FROM_DEBUG_STRING);}

#define FUNCTION_ENTRY  (void) nav_update(pENTER,__FNAME__,0,fname_index);
#define FUNCTION_EXIT   (void) nav_update(pLEAVE,__FNAME__,0,fname_index);

#define LIMIT_TRACE	if(LOOK && DEPTH_LIMITING)                         \
                            reset_limit = nav_update(pLIMIT,__FNAME__,      \
                                              LIMIT_VALUE,fname_index);

#define UNLIMIT_TRACE	if(LOOK && DEPTH_LIMITING)                         \
                            (void) nav_update(pLIMIT,__FNAME__,             \
                                              reset_limit,fname_index);

#define NAV_ENTER(fname)                                     \
                        DEBUG_VARIABLES(fname)               \
			FUNCTION_ENTRY                       \
			CHECK_START_TRACE		     \
			CHECK_ADD_TO_DEBUG                   \
			LIMIT_TRACE

#define NAV_LEAVE(fname)                                     \
			UNLIMIT_TRACE	                     \
			CHECK_REMOVE_FROM_DEBUG              \
			CHECK_STOP_TRACE                     \
			FUNCTION_EXIT                        

#define DEBUG_ENTER(fname)	NAV_ENTER(fname)
#define DEBUG_LEAVE(fname)	NAV_LEAVE(fname)

/* Prototypes for navigate.c */

#if defined(c_plusplus) || defined(__cplusplus)
extern "C" {
#endif


IMPORT  void  	print_nav_data(const char*);
IMPORT  int   	nav_update(int,const char*,int,int);
IMPORT  int   	nav_index(const char*);
IMPORT  int   	nav_switches(int,int);
IMPORT  int   	*nav_integer_pointers(int,int);
IMPORT  int   	nav_integers(int,int);
IMPORT  double 	*nav_float_pointers(int,int);
IMPORT  POINTER *nav_pointers(int,int);
IMPORT  char	*nav_strings(int,int);
IMPORT	int   	nav_trace(int,const char*,int);
IMPORT 	void 	nprint_long_string(const char*);
IMPORT  int   	depth_monitor(int);
IMPORT  int   	function_monitor(int);
IMPORT  void  	read_navigator_input(const char*);

#if DONT_COMPILE /*SUGGESTED REVISION FOR NAVIGATOR 19990423*/

#define	BRIEFLY(fname)	NAVIGATOR_BRIEFLY(#fname)
#define	EXPLAIN(fname)	NAVIGATOR_EXPLAIN(#fname)
#define	DETAILS(fname)	NAVIGATOR_DETAILS(#fname)
#define	SUMMARY(fname)	NAVIGATOR_SUMMARY(#fname)

/* navigator macros */

#define	INT0(fname)	NAVIGATOR_INT0(#fname)
#define	INT1(fname)	NAVIGATOR_INT1(#fname)

#define	INTEGER0(fname)	NAVIGATOR_INTEGER0(#fname)
#define	INTEGER1(fname)	NAVIGATOR_INTEGER1(#fname)

#define FLOAT0(fname)	NAVIGATOR_FLOAT0(#fname)
#define FLOAT1(fname)	NAVIGATOR_FLOAT1(#fname)
#define FLOAT2(fname)	NAVIGATOR_FLOAT2(#fname)

#define POINTR(fname)	NAVIGATOR_POINTR(#fname)

#define STRING_BUG(fname)	NAVIGATOR_STRING_BUG(#fname)

#if defined(DEBUG_ENTER)
#undef DEBUG_ENTER
#endif /* defined(DEBUG_ENTER) */
#define DEBUG_ENTER(fname)	NAVIGATOR_DEBUG_ENTER(#fname);

#if defined(DEBUG_LEAVE)
#undef DEBUG_LEAVE
#endif /* defined(DEBUG_LEAVE) */
#define DEBUG_LEAVE(fname)	NAVIGATOR_DEBUG_LEAVE(#fname);

/* Prototypes for navigate.c.c */

IMPORT	POINTER    *NAVIGATOR_POINTR(const char*);
IMPORT	boolean    NAVIGATOR_BRIEFLY(const char*);
IMPORT	boolean    NAVIGATOR_DETAILS(const char*);
IMPORT	boolean    NAVIGATOR_EXPLAIN(const char*);
IMPORT	boolean    NAVIGATOR_SUMMARY(const char*);
IMPORT	char       *NAVIGATOR_STRING_BUG(const char*);
IMPORT	double	   *NAVIGATOR_FLOAT0(const char*);
IMPORT	double      *NAVIGATOR_FLOAT1(const char*);
IMPORT	double      *NAVIGATOR_FLOAT2(const char*);
IMPORT	int        *NAVIGATOR_INTEGER0(const char*);
IMPORT	int        *NAVIGATOR_INTEGER1(const char*);
IMPORT	int	   NAVIGATOR_INT0(const char*);
IMPORT	int	   NAVIGATOR_INT1(const char*);
IMPORT	void	   NAVIGATOR_DEBUG_ENTER(const char*);
IMPORT	void	   NAVIGATOR_DEBUG_LEAVE(const char*);
IMPORT 	void 	   nprint_long_string(char*);
IMPORT  void       print_navigator_data(const char*);
IMPORT  void       read_navigator_input(const char*);
IMPORT	void       nprintf(const char*,...);

#if defined(c_plusplus) || defined(__cplusplus)
}
#endif


#endif /*DONT_COMPILE*/ /*SUGGESTED REVISION FOR NAVIGATOR*/

#endif /* defined(NAVIGATOR) */

#endif /* !defined(_NAVDECS_H) */
