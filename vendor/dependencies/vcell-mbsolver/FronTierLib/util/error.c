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
*			General Error Handling Facility:
*
*	The routine  Error(number,message)  adds an error to
*	the current error list.   Here  number  is an (not
*	necessarily unique) integer characterizing the error.
*
*	The error messages may be retrieved later by calling
*	print_errors(), which prints them, or report_errors()
*	which returns the number and sets a pointer to the error 
*	list.   In addition, error messages are printed at the
*	time of call in FILE file, if the routine
*			set_error_immediate(file)
*	has been called.   A call with file==NULL turns off such
*	printing, which is also the default.
*
*	Generally it is expected that calls to Error() and
*	print_errors() will suffice.
*
*	Errors are maintained as a linked list of _Error 
*       data structures - see cdecs.h for the definition.
*
*	Error messages are only saved as string pointers.   Thus
*	they should not be automatic variables or allocated variables
*	and if (static) variables they should not be reft_assigned too.
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*/

#include <cdecs.h>
#include <stdlib.h>
#ifndef __clang__
# include <malloc.h>
#endif


LOCAL struct _Error error_list;
LOCAL struct _Error *last_error = NULL;
LOCAL int num_errors = 0;


EXPORT void print_errors(void)
{
	struct _Error *err;

	if (last_error==NULL) return;
	(void) printf("ERROR:  The following %d errors have been detected:\n",
		      num_errors);
	(void) printf("%15s %4s:  %5s   %s\n",
		      "Filename","Line","Number","Message");
	for (err = &error_list; err; err = err->next)
		(void) printf("%15s %4d:  %5d   %s\n",err->filename,
			      err->line_number,err->number,err->message);
}		/*end print_errors*/





LOCAL FILE *error_immediate = NULL;

EXPORT void set_error_immediate(
	FILE	*file)
{
	error_immediate = file;
}		/*end set_error_immediate*/





EXPORT void log_error(
	const char	*filename,
	int		line_number,
	int		number,
	const char	*message)
{
	struct	_Error	*new_error;

	if (last_error==NULL)
	    new_error = last_error = &error_list;
	else
	{
	    new_error = (struct _Error *)malloc(sizeof(struct _Error));
	    if (new_error==NULL)
		return;
	    last_error->next = new_error;
	    last_error = new_error;
	}
	num_errors++;

	new_error->filename = filename;
	new_error->line_number = line_number;
	new_error->number = number;
	new_error->message = message;
	new_error->next = NULL;
	if (error_immediate==NULL)
	    return;
	(void) fprintf(error_immediate,"%15s %4d:  %5d   %s\n",
		       filename,line_number,number,message);
}		/*end log_error*/
