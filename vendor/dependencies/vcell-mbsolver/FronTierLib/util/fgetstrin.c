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
*				fgetstring(), sgetstring():
*
*	Reads a file until 'string' is found.   Returns 1 if successful,
*	or zero on end-of-file.  Reading commences at the current file
*	pointer position in the file.
*
*	sgetstring() performs the same operation on a string, returning
*	pointer to first char following 'string', or NULL if not present.
*
*	WARNING:	THIS ROUTINE IS NOT FAILPROOF.
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*/

#include <cdecs.h>

boolean fgetstring_debug = NO;

EXPORT	boolean fgetstring(
	FILE		*file,
	const char	*strng)
{
	const char	*s;
	int		ch;
	long		current;

	current = ftell(file); /*Mark current location in file*/
	if (fgetstring_debug)
	    (void) fprintf(stderr,"fgetstring(%s) ",strng);
	s = strng;
	while((ch = getc(file)) != EOF)
	{
	    if (ch != *s)
	        s = strng;
	    else if (!*++s)
	        break;
	}
	if (ch == EOF)
	{
	    /* strng not found,  restore position in file */
	    (void) fseek(file,current,SEEK_SET);
	    if (fgetstring_debug)
	        (void) fprintf(stderr,"FAIL\n");
	    return FUNCTION_FAILED;
	}
	else
	{	
	    if (fgetstring_debug)
	        (void) fprintf(stderr,"SUCCESS\n");
	    return FUNCTION_SUCCEEDED;
	}
}		/*end fgetstring*/

EXPORT	void CursorAfterString(
	FILE		*file,
	const char	*strng)
{
	if (fgetstring(file,strng) == FUNCTION_FAILED)
	{
	    screen("Cannot find string %s in file!\n",strng);
	    clean_up(ERROR);
	}
	(void) printf("%s ",strng);
}	/* end CursorAfterString */

EXPORT	boolean CursorAfterStringOpt(
	FILE		*file,
	const char	*strng)
{
	long cur_pos;

	cur_pos = ftell(file);
	(void) printf("%s ",strng);
	if (fgetstring(file,strng) == FUNCTION_FAILED)
	{
	    fseek(file,cur_pos,SEEK_SET);
	    screen("Cannot find the string, using default\n");
	    return NO;
	}
	return YES;
}	/* end CursorAfterStringOpt */

/*
*		Searches for string in a string S.   Returns pointer
*	to the character in S following string, or NULL on error.
*/


EXPORT	const char *sgetstring(
	const char *S,
	const char *strng)
{
	const char	*s;
	int		ch;

	if (fgetstring_debug)
	    (void) fprintf(stderr,"sgetstring(%s) ",strng);
	s = strng;
	if (*s == '\0')
	    return S;
	while ((ch = *S++) != '\0')
	{
	    if (ch != *s)
		s = strng;
	    else if (!*++s)
		break;
	}
	if (!ch)
	{
	    if (fgetstring_debug)
	    	(void) fprintf(stderr,"FAIL\n");
	    return NULL;
	}
	else
	{	
	    if (fgetstring_debug)
	    	(void) fprintf(stderr,"SUCCESS\n");
	    return S;
	}
}		/*end sgetstring*/





/*
*				copy_until():
*
*	Reads a file, copying its output to another file, until a
*	specified string is detected.   Returns the string if found,
*	or NULL.   An empty string or a NULL string is matched immediatly.
*	The copy argument specifies that all characters read are to
*	be copied if TRUE, otherwise no characters are copied.
*/

EXPORT	char *copy_until(
	char *st,
	FILE *ifile,
	FILE *ofile,
	int  copy)
{

	int		n = (int)strlen(st);
	int		i, j, c;
	boolean		KeepReading = YES;

	if (st == NULL || *st=='\0')
	    return st;
	while (KeepReading == YES)
	{
	    while ((c = getc(ifile)) != *st && c != EOF)
	    	if (copy)
		    (void) putc(c,ofile);
	    if (c == EOF)
		break;

	    for (i = 1; i < n; i++)
	    	if ((c = getc(ifile)) != st[i])
		    break;
	    if (c == EOF)
		break;
	    if (i != n)
	    {
	    	for (j = 0; j < i; j++)
	    	    if (copy)
	    	    	(void) putc((int)st[j],ofile);
	    	if (copy)
	    	    (void) putc(c,ofile);
	    	continue;
	    }
	    else
	    {
	    	for (j = 0; j < n; j++)
	    	    if (copy)
	    	    	(void) putc((int)st[j],ofile);
	    	return st;
	    }
	}
	return NULL;
}		/*end copy_until*/
