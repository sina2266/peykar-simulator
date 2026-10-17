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
*			C  DEBUGGING  PACKAGE:
*
*		Provides a general debugging package:
*	Compile any program along with the file debug.c.
*	Include a call to the function  init_debug() somewhere
*	in the program.   Then the function  debug_print() becomes available.
*
*	Usage:      debug_print("function_name", printf arguments);
*
*		The arguments are a standard set of arguments to printf.
*	The printing will be done only if debugging of the function
*	function_name was requested when init_debug() was called, and
*	provided the variable db_mode is nonzero.
*
*		Init_debug()  first asks if debugging is requested.
*	Typing the word  debug  or an abbreviation  will initiate
*	debugging.   The word debug may optionally be followed by
*	a filename, in which case the information about which functions
*	to debug will be sought in that file rather than at the terminal,
*	and corresponding prompting omitted.
*
*		There is a choice between having the debugging printing 
*	done on stderr or stdout - initdebug_print() requests either screen or
*	file.
*
*		Init_debug requests the names of the functions to be 
*	debugged.  These should be typed one to a line (not essential).
*	Alternately, you can type   all   followed by a list of names
*	NOT to be debugged (which may be an empty list).   In either
*	case the lists should be terminated by typing the word  end.
*
*		Init_debug remembers the names it was given the last 
*	time it was used.   These names are stored in a file called:
*				dnames  
*	Thus to rerun a  previous debugging session, just type
*			debug   dnames
*	when requesting debugging (or possibly edit dnames first).
*
*		Only the first 8 characters of a function name are used
*	by the debug package,  which therefore allows abbreviation in both
*	giving names to init_debug() and in typing  debug_print()  statements.
*
*		The variable db_mode is available via the INIT_DATA structure
*	to signal that debugging was requested (if nonzero).   All other
*	variables in the debugging package are hidden from the user.
*
*		The value of db_mode may be modified at will
*	by the user and debug_print() statements will be ignored for a requested
*	function if db_mode is zero at the time of the call.
*	This provides a form of dynamic debugging.   The value of db_mode is
*	initialized to  1 or 0  by init_debug() according as debugging is
*	requested or not.
*
*		There is also a function called   debugging()  which
*	determines if debugging was requested for a given function, in
*	which case it returns 1, otherwise 0.  This package
*	may be used for purposes other than debugging - the storing and
*	comparing of names will work in general.
*
*		Finally, there is a function called  debug_trace()  which
*	provides a traceback at the time it is called over the last
*	MAX_TRACE function calls to the debug routines whether or not
*	those functions were being debugged.   However the traceback is
*	available only if db_mode != NONE.
*
*	NOTE:	There is a limit of MAX_BYTES on the total number of characters
*	required by all the function names requested, except that when 
*	using  all  the limit is on the function names not to be debugged.
*	There is no limit on the number of characters per name.   However
*	only the first 8 characters are actually used anywhere.
*	   	The total number of requested functions should not be more
*	than MAX_NAMES.
*
*	Author:		Oliver A. McBryan,
*			Courant Institute, New York.
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*/

/* LINTLIBRARY */

#include <cdecs.h>

#if !defined(MAX_NAMES)
enum { MAX_NAMES = 50 };	/* Max Number of debugging names */
#endif /*!defined(MAX_NAMES)*/

#if !defined(MAX_BYTES)
enum { MAX_BYTES = 500 };	/* Max Number of characters in names (total) */
#endif /*!defined(MAX_BYTES)*/

#if !defined(MAX_CHARS)
enum { MAX_CHARS = 8 };	/* Max Number of characters used per name */
#endif /*!defined(MAX_CHARS)*/

#if !defined(MAX_TRACE)
enum { MAX_TRACE = 50 };	/* Length of traceback */
#endif /*!defined(MAX_TRACE)*/

struct entry {			/* Circular list to store the traceback. */
	char fname[MAX_CHARS+1];
	struct entry *prev;
	struct entry *next;
};

struct _INTERNAL_DEBUG_PARAMS {
	DEBUG_PARAMS	_debug_params;
	FILE		*_debug_output;/* Output Destination for Debugging */
        FILE		*_debug_input; /* Input source for Debugging Info  */
        FILE		*_remember;    /* Stores debug names from run here */
	int		_num_of_debug_names; /* # names in debug_names[i]  */
	struct entry	*_head;
	struct entry	_ftrace[MAX_TRACE];
	char 		_debug_names[MAX_NAMES][MAX_CHARS+1];
	                        /* Stores names of Routines to be debugged */
};
typedef struct _INTERNAL_DEBUG_PARAMS INTERNAL_DEBUG_PARAMS;

LOCAL	INTERNAL_DEBUG_PARAMS	DbParams = {
	                                        {
	                                            NONE/*debug_mode*/
	                                        },
	                                        NULL, /*debug_output*/
	                                        NULL, /*debug_input*/
	                                        NULL, /*remember*/
	                                        0,    /*num_of_debug_names*/
	                                        NULL  /*head*/
	                                   };
#define	db_mode			DbParams._debug_params._debug_mode
#define	num_of_debug_names	DbParams._num_of_debug_names
#define	debug_names		DbParams._debug_names
#define debug_output		DbParams._debug_output
#define debug_input		DbParams._debug_input
#define remember		DbParams._remember
#define head			DbParams._head
#define ftrace			DbParams._ftrace

#if defined(DEB_FILE)
static const char *REMEMBER_FILENAME = "dnames";     /* Name of remember file */
#endif /* defined(DEB_FILE) */

static const char *NOTHING = "";

	/* LOCAL Function Prototypes */

EXPORT	DEBUG_PARAMS *default_debug(
	const DEBUG_MODE	dbmode)
{
	int i;

	if (debug_output == NULL)
	    debug_output = stdout;

	num_of_debug_names = 0;
	debug_input = stdin;

	for(i=0; i<MAX_TRACE; ++i)		/* Create Circular List: */
	{
	    (void) strcpy(ftrace[i].fname,NOTHING);
	    ftrace[i].prev  = ftrace+i-1;
	    ftrace[i].next  = ftrace+i+1;
	}
	head = ftrace[0].prev = ftrace+MAX_TRACE-1;
	ftrace[MAX_TRACE-1].next = ftrace;
	(void) strcpy(head->fname,NOTHING);
        db_mode = SOME;
	remember = NULL;
	debug_output = stdout;
	num_of_debug_names = 0;
	return debug_params(&DbParams);
}		/*end default_debug*/

EXPORT	DEBUG_PARAMS *init_debug(
	const DEBUG_MODE	dbmode)
{
	int i;
	char s[Gets_BUF_SIZE];
#if defined(DEB_FILE)
	char filename[15];
#endif /* defined(DEB_FILE) */

	if (debug_output == NULL)
	    debug_output = stdout;

	if (dbmode != PROMPT_FOR_DEBUG)
	{
	    db_mode = dbmode;
	    return debug_params(&DbParams);
	}
	db_mode = NONE;
	num_of_debug_names = 0;
	debug_input = stdin;

	for(i=0; i<MAX_TRACE; ++i)		/* Create Circular List: */
	{
	    (void) strcpy(ftrace[i].fname,NOTHING);
	    ftrace[i].prev  = ftrace+i-1;
	    ftrace[i].next  = ftrace+i+1;
	}
	head = ftrace[0].prev = ftrace+MAX_TRACE-1;
	ftrace[MAX_TRACE-1].next = ftrace;
	(void) strcpy(head->fname,NOTHING);


	screen("Type debug to Turn On Debugging: ");
	(void) Gets(s);
	if (s[0] == 'd')
	{
	    db_mode = SOME;

	    remember = NULL;

#if defined(DEB_FILE)
	                                      /* Look for a Filename */
	    screen("Enter Optional Debug Input Filename: ");
	    (void) Gets(s);
	    if (sscanf(s,"%s",filename) == 1)
	    {  
	        if (strcmp(filename,REMEMBER_FILENAME) != 0) 
	            remember = fopen(REMEMBER_FILENAME,"w");
	        if ((debug_input = fopen(filename,"r")) == NULL)
	        {
	            screen("NO DEBUG FILE CALLED: %s\n",filename);
	            debug_input = stdin;
	            if (remember == NULL)
	                remember = fopen(REMEMBER_FILENAME,"w");
	        }
	    }
	    if (debug_input == stdin)
	        remember = fopen(REMEMBER_FILENAME,"w");
#endif /* defined(DEB_FILE) */

	    screen("Specify Debugging Destination, screen or file: ");
	    (void) Gets(s);
	    debug_output = (s[0] == 's') ? stderr : stdout;

	    screen("List Functions to Debug - or all -  end  ends list\n"
	           "Names following  all  will not be debugged - end ends\n");

get_names:
	    for(i=0; ;++i)
	    {
	        screen("\t: ");
	        (void) fscanf(debug_input,"%s",s);
	        (void) printf("%s\n",s);
	        (void) strncpy(debug_names[i],s,MAX_CHARS);/* Read next Name */
	        if (remember != NULL)			/* Put in Remember */
	            (void) fprintf(remember,"%s\n",debug_names[i]);  

	        if (strcmp(debug_names[i],"all")==0)
	        {
	            db_mode = ALL;
	            goto get_names;			/* Restart Loop */
	        }

	        if (strcmp(debug_names[i],"end")==0)
	            break;

	        if (((int) strlen(debug_names[i])) >= MAX_CHARS)
	        {
	            debug_names[i][MAX_CHARS] = '\0';
	        }
	    }

	    num_of_debug_names = i;
	    /* Remove any residual whitespace */
	    (void) fgets(s,Gets_BUF_SIZE-2,debug_input);
	    if (remember != NULL)
	        (void) fclose(remember);
	}
	else
	{
	    screen("Type 't' to obtain traceback of debug lines upon ");
	    screen("error termination: ");
	    if (db_mode == NONE)
	    {
	        (void) Gets(s);
	        if (s[0] == 't')
	            db_mode = TRACE_ONLY;
	    }
	    else
	        screen("\n");
	}
	return debug_params(&DbParams);
}		/*end init_debug*/

EXPORT	void	set_debug_output(
	FILE *file)
{
	debug_output = file;
}	/*end set_debug_output()*/

EXPORT	void	add_to_debug(
	const char *name)
{
	int i;

	if (db_mode == NONE)
	    return;
	for (i = 0; i < num_of_debug_names; ++i)
	{
	    if (strncmp(debug_names[i],name,MAX_CHARS) == 0)
	        return;
	}
	(void) strncpy(debug_names[i],name,MAX_CHARS);
	if (((int) strlen(name)) >= MAX_CHARS)
	    debug_names[i][MAX_CHARS] = '\0';
	++num_of_debug_names;
}		/*end add_to_debug*/

EXPORT	void	remove_from_debug(
	const char *name)
{
	int i, j;

	if (db_mode == NONE)
	    return;
	for (i = 0; i < num_of_debug_names; ++i)
	{
	    if (strncmp(debug_names[i],name,MAX_CHARS) != 0)
	        continue;
	    for (j = i+1; j < num_of_debug_names; ++j)
	        (void) strncpy(debug_names[j-1],debug_names[j],MAX_CHARS);
	    num_of_debug_names--;
	    i--;
	}
}		/*end remove_from_debug*/


/*
*				debug_print():
*	Prints debugging data on the output file.   Prints only when
*	debugging has been explicitly requested.
*/

enum {
	MAX_DEBUG_LINES = 2*MAX_TRACE,
	MAX_LINE_LEN    = 77
};
static char debug_lines[MAX_DEBUG_LINES][MAX_LINE_LEN+1];
static int  curr_line = 0;
static int  looped = 0;


#include <stdarg.h>


#if defined(FAST_DEBUG)

EXPORT	void debug_print(
	const char *funcname,
	const char *fmt,
	...)
{
}		/*end debug*/

#else /* defined(FAST_DEBUG) */

/* VARARGS */
EXPORT	void debug_print(
	const char *funcname,
	const char *fmt,
	...)
{
	va_list ap;
	int i;
	char buf[1000];
	char *debug_l;

	if (db_mode == NONE)
	    return;


	if (debugging(funcname))
	{
	    va_start(ap, fmt);
	    (void) vfprintf(debug_output,fmt,ap);
	    va_end(ap);
	}
	va_start(ap, fmt);
	(void) vsprintf(buf,fmt,ap);
	va_end(ap);
	debug_l = debug_lines[curr_line];
	(void) sprintf(debug_l,"%-10s",funcname);
	(void) strcpy(debug_l+10,"| ");
	for(i=0; i<(MAX_LINE_LEN-12) && (debug_l[i+12] = buf[i]); ++i) ;
	debug_l[i+12] = '\0';
	++curr_line;
	if (curr_line == MAX_DEBUG_LINES)
	{
	    curr_line=0;
	    looped = 1;
	}
}		/*end debug*/
#endif /* defined(FAST_DEBUG) */


/*
*				debugging():
*	Returns 1 if debugging has been requested for function funcname
*	and returns 0 otherwise.
*/



#if defined(FAST_DEBUG)

EXPORT	boolean debugging(
	const char *funcname)
{
	return NO;
}		/*end debugging*/

#else /* defined(FAST_DEBUG) */

EXPORT	boolean debugging(
	const char *funcname)
{
	int i;
	size_t len;

	if (db_mode == NONE) return NO;

	len = strlen(funcname);
	if ((head != NULL) && (strncmp(funcname,head->fname,MAX_CHARS) != 0))
	{
	    head = head->next;
	    (void) strncpy(head->fname,funcname,MAX_CHARS);
	    if (len >= MAX_CHARS)
	        head->fname[MAX_CHARS] = '\0';
	}

	                                /* Skip Names Given, Debug Others: */
	if (db_mode == ALL)
	{		
	    for(i=0; i < num_of_debug_names; ++i)
	        if (strncmp(funcname,debug_names[i],MAX_CHARS) == 0)
	            return NO;
	    return YES;
	}
	                                /* Debug functions given: */
	else if (db_mode == SOME)
	{
	    for(i=0; i < num_of_debug_names; ++i) 
	        if (strncmp(funcname,debug_names[i],MAX_CHARS) == 0)
	            return YES;
	    return NO;
	}
	return NO;
}		/*end debugging*/

#endif /* defined(FAST_DEBUG) */

#if defined(FAST_DEBUG)
EXPORT	char **debugging_names(
	int *ndbnames)
{
	*ndbnames = 0;
	return (char **)NULL;
}		/*end debugging_names*/
#else /* defined(FAST_DEBUG) */

EXPORT	char **debugging_names(
	int *ndbnames)
{
	static char buf[MAX_NAMES][MAX_CHARS+1];
	static char *db[MAX_NAMES];
	int i;

	if ((db_mode == NONE) || (num_of_debug_names == 0))
	{
	    *ndbnames = 0;
	    return (char **)NULL;
	}

	for (i = 0; i < num_of_debug_names; ++i)
	{
	    db[i] = buf[i];
	    (void) strcpy(db[i],debug_names[i]);
	}
	for (i = num_of_debug_names; i < MAX_NAMES; ++i)
	    db[i] = NULL;
	*ndbnames = num_of_debug_names;
	return db;
}		/*end debugging_names*/
#endif /* defined(FAST_DEBUG) */



/*
*			debug_trace():
*
*	Provides a traceback of recently called debug routines provided
*	that db_mode != NONE.
*/



EXPORT void debug_trace(void)
{
	struct entry *entr = head;
	int i;

	if ((db_mode == NONE) || (head == NULL))
	    return;
	(void) fprintf(debug_output,"\n\nBacktrace of recent debug calls:\n");
	for (entr=head; strcmp(entr->fname,NOTHING); entr=entr->prev)
	{
	    (void) fprintf(debug_output,"%s\n",entr->fname);
	    if ((entr == head->next))
	        break;
	}
	(void) fprintf(debug_output,"End of trace\n\n");

	(void) fprintf(debug_output,"Recent Debug Lines:\n");
	curr_line--;
	for(i=curr_line; i>=0; i--)
	    (void) fprintf(debug_output,"%s\n",debug_lines[i]);
	if (looped) 
	    for(i=MAX_DEBUG_LINES-1; i>curr_line; i--)
	        (void) fprintf(debug_output,"%s\n",debug_lines[i]);
	(void) fprintf(debug_output,"End recent debug output\n\n");
}		/*end debug_trace*/

EXPORT	void unset_debug()
{
	int i;
	db_mode = NONE;
	num_of_debug_names = 0;
}	/* end unset_debug */
