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
*			other.c
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*/

#include <cdecs.h>
#include <stdarg.h>

enum { MAX_FLOATS = 255 };

/*
*			print_line_of_floats():
*
*	Prints a collection of floats in binary or non-binary format
*	See the comments for print_vector_of_floats().
*
*	usage:  print_line_of_floats(n,f1,f2,f3,...,fn)
*
*	Useful for printing columnar data
*/



/* VARARGS */

EXPORT void print_line_of_floats(
	int	number,
	...)
{
	va_list ap;
	int	num,i;
	double	next;
	double	f[MAX_FLOATS];
	       /* MAX_FLOATS = maximum number of floats in line */



	if (is_binary_output() == YES)
	{
	    while (number)
	    {
	    	num = (number>MAX_FLOATS) ? MAX_FLOATS : number;
	    	(void) printf("\f%c",num);
	    	for (i = 0; i < num; ++i)
	    	{
		    va_start(ap, number);
	    	    next = va_arg(ap,double);
		    va_end(ap);
	    	    f[i] = (double)next;
	    	}
	    	(void) fwrite((const void *)f,sizeof(double),num,stdout);
	    	number -= num;
	    }
	}
	else
	{
	    for (i = 0; i < number; ++i)
	    {
		va_start(ap, number);
	    	next = va_arg(ap,double);
		va_end(ap);
	    	(void) printf("%-"FFMT" ",next);
	    }
	    (void) printf("\n");
	}
}		/*end print_line_of_floats*/

/*
*			fprint_line_of_floats():
*
*	Prints a collection of floats in binary or non-binary format
*	See the comments for print_vector_of_floats().
*
*	usage:  fprint_line_of_floats(file,n,f1,f2,f3,...,fn)
*
*	Useful for printing columnar data
*/



/* VARARGS */

EXPORT void fprint_line_of_floats(
	FILE	*file,
	int	number,
	 ...)
{
	va_list		ap;
	int		num,i;
	double		next;
	double		f[MAX_FLOATS];
		       /* MAX_FLOATS = maximum number of floats in line */

	va_start(ap, number);


	if (is_binary_output() == YES)
	{
	    while (number)
	    {
	    	num = (number>MAX_FLOATS) ? MAX_FLOATS : number;
		(void) fprintf(file,"\f%c",num);
		for (i = 0; i < num; ++i)
		{
		    next = va_arg(ap,double);
		    f[i] = (double)next;
		}
		(void) fwrite((const void *)f,sizeof(double),num,file);
		number -= num;
	    }
	}
	else
	{
	    for (i = 0; i < number; ++i)
	    {
	    	next = va_arg(ap,double);
	    	(void) fprintf(file,"%-"FFMT" ",next);
	    }
	    (void) fprintf(file,"\n");
	}
	va_end(ap);
}		/*end fprint_line_of_floats*/
/*since it returns a local static var, it can be called only once in one printf */
EXPORT	const char *right_flush(
	int	   n,
	int	   ndigits)
{
	static	char	s[20];
	int		i;

	if (n == 0)
		ndigits--;
	for (i = n; i > 0; i /= 10) ndigits--;
	
	s[0] = '\0';
	for (;ndigits > 0; ndigits--)
		(void) sprintf(s,"%s0",s);
	(void) sprintf(s,"%s%d",s,n);
	return s;
}		/*end right_flush*/

EXPORT	const	char	*y_or_n(
	boolean	arg)
{
	switch (arg)
	{
	case YES:
	    return "yes";
	case NO:
	default:
	    return "no";
	}
}		/*end y_or_n*/

EXPORT	const char *ordinal_suffix(
	int i)
{
	i = (i >= 0) ? i%20 : (-i)%20;
	switch (i)
	{
	case 1:
	    return "st";
	case 2:
	    return "nd";
	case 3:
	    return "rd";
	default:
	    return "th";
	}
}		/*end ordinal_suffix*/

/*
*			base_and_dir_name():
*
*	Extracts the base and directory part of a path name.  The
*	strings returned point to substrings of an internal static
*	character strings,  so that the values of these strings are
*	only valid until the next call to this function.
*/

EXPORT	void	base_and_dir_name(
	const char	*name,
	char		**dname,
	char		**bname)
{
	static	char	*buf = NULL;
	static  char    nostring[] = "";
	static	size_t	len = 0;

	if (name == NULL)
	{
	    if (dname != NULL)
	    	*dname = nostring;
	    if (bname != NULL)
	    	*bname = nostring;
	    return;
	}
	if (len == 0)
	{
	    len  = 256 + strlen(name);
	    buf = (char*)malloc(len*sizeof(char));
	}
	else if (strlen(name) >= len)
	{
	    free(buf);
	    len  = 256 + strlen(name);
	    buf = (char*)malloc(len*sizeof(char));
	}
	(void) strcpy(buf,name);
	if (bname != NULL)
	    *bname = get_basename(buf);
	if (dname != NULL)
	    *dname = get_dirname(buf);
}		/*end base_and_dir_name*/


EXPORT	void	print_boolean(
	const char *mesg1,
	boolean	   b,
	const char *mesg2)
{
	if (mesg1 != NULL)
	    (void) printf("%s",mesg1);
	switch (b)
	{
	case YES:
	    (void) printf("YES, FUNCTION_SUCCEEDED");
	    break;
	case NO:
	default:
	    (void) printf("NO, FUNCTION_FAILED");
	    break;
	}
	if (mesg2 != NULL)
	    (void) printf("%s",mesg2);
}		/*end print_boolean*/
