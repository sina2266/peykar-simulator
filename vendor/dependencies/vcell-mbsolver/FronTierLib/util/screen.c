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
*			screen():
*
*		A version of fprintf which echoes its output onto
*	the standard output as well as the standard error. 	The
*	printing on standard error (screen) is suppressed if the 
*	boolean variable  suppress_prompts  is YES.
*		While not fully portable, this program should be easy
*	to modify to run under other operating systems or machines.
*		The present version uses the fact that chars are converted
*	to ints and floats to doubles on function calls (This IS portable -
*	being part of the definition of the C language).   It also assumes 
*	ints, and doubles are aligned on word boundaries (not portable);
*	
*	Author:		Oliver A. McBryan.
*			Courant Institute, New York.
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*/

/* LINTLIBRARY */

#include <cdecs.h>
#include <stdarg.h>


	/* LOCAL function Prototypes */
LOCAL	int	search_for(char);
LOCAL	void	deblank(void);


EXPORT	boolean suppress_prompts = NO;

/* VARARGS */

EXPORT	void screen(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	(void) vprintf(fmt,ap);
	va_end(ap);
	if (suppress_prompts == NO)
	{
	    va_start(ap, fmt);
	    (void) vfprintf(stderr,fmt,ap);
	    va_end(ap);
	}
}		/*end screen*/

			




			
/*
*				Scanf():
*	This program provides a version of the function scanf()
*	that echoes everything scanned from the standard input onto
*	the standard output.
*
*	There are some important differences from the Library version.
*	First of all, octal and hexadecimal numbers are not supported.
*
*	Individual format fields begin with a % and are otherwise as
*	in scanf() except that ft_assignment suppression is not available.
*
*	Blanks in the format string are ignored as in scanf().
*	All other characters, including NEWLINE are treated as follows:
*	the input is searched until either that character is seen or
*	end of file is found (an error).
*	Thus:
*			Scanf(" %d  %d \n", &m, &n);
*	will try to read two integers m,n  and will continue reading
*	to the end of the last line.
*
*	Scanf returns EOF on end of file and ERROR_SCAN = EOF-1 on errors.
*	Otherwise it returns the number of %-requested objects read.
*
*/


/* VARARGS */
EXPORT	int	Scanf(const char *fmt, ...)
{
	char Fmt[10],*t;
	int count;
	int *ppi;
	long *ppl;
	char *ppc,*pps;
	double *ppd;
	static const int ERROR_SCAN = EOF-1;
	va_list ap;

	va_start(ap, fmt);

	Fmt[0] = '%';
	count = 0;
	while (*fmt)
	{
	    if (*fmt != '%')
	    { 
	        if (*fmt != ' ') 
	    	    if (!search_for(*fmt)) return ERROR_SCAN;
	        ++fmt;
	        continue;
	    }
	    ++count;
					/* Form Format String Fmt: */
	    ++fmt;
	    t = Fmt+1;
	    while ((*t = *fmt, ! isalpha(*fmt)))  { ++fmt;  ++t; }
	    *++t = '\0';

	    deblank();		/* Print Blanks from Input: */
					/* Switch on Format Type: */
	    switch (*fmt)
	    {
	    case 'c':		/* Character: */
	    	ppc = va_arg(ap,char *);
	    	(void) scanf(Fmt,ppc);
	    	(void) printf(Fmt,*ppc);
	    	break;
	    case 'd':		/* Integer: */
	    	ppi = va_arg(ap,int *);
	    	(void) scanf(Fmt,ppi);
	    	(void) printf(Fmt,*ppi);
		    break;

	    case 'l':		/* Long or Double: */
	        *t = *++fmt;
	        *++t = '\0';
	        if (*fmt == 'd') 		/* Long: */
	        {
	            ppl = va_arg(ap,long *);
	            (void) scanf(Fmt,ppl);
	            (void) printf(Fmt,*ppl);
	        }
	        else				/* Double: */
		{
		    ppd = va_arg(ap,double *);
		    (void) scanf(Fmt,ppd);
		    --t;
		    *--t = *fmt;
		    *++t = '\0';
		    (void) printf(Fmt,*ppd);
		}
		break;

	    case 's':		/* String: */
	        pps = va_arg(ap,char *);
	        (void) scanf(Fmt,pps);
	        (void) printf(Fmt,pps);
	        break;

	    case 'f':		/* Float: */
	        {
	            ppd = va_arg(ap,double *);
	            (void) scanf("%lf",ppd);
	            (void) printf("%g",*ppd);
	        }
	        break;

	    default:		/* Error: */
	        return ERROR_SCAN;

	    }
	    ++fmt;
	}
	va_end(ap);
	(void) fflush(stdout);
	return count;
}		/*end Scanf*/

			

			

LOCAL	void	deblank(void)
{
	int c;

	while ((c=getchar()), isspace(c)) (void) putchar(c);
	(void) ungetc(c,stdin);
}		/*end deblank*/
			




LOCAL int search_for(char ch)
{
	int c;

	while ((c=getchar()) != EOF)
	{
		(void) putchar(c);
		if (c == ch) return 1;
	}
	return 0;
}		/*end search_for*/




EXPORT	char *Gets(char *s)
{
	char *t;
	int n, i;
	int len;

	t = fgets(s,Gets_BUF_SIZE-2,stdin);
	if (t != NULL)
	{
	    fputs(s,stdout);
	    (void) fflush(stdout);
	    len = (int)strlen(s) - 1;
	    s[len] = '\0';
	    /* Remove leading blanks */
	    for (n = 0; n < len; ++n)
	    	if (!isspace(s[n])) break;
	    if (n > 0)
	    {
	    	for (i = n; i <= len; ++i)
	   	    s[i-n] = s[i];
	    }
	}
	return t;
}		/*end Gets*/

EXPORT	void	screen_print_long_string(
	const char	*s)
{
	char	*c;
	int	len;
	int	maxlen = 79;
	int	newlen;
	int	tablen = 8;
	boolean start_of_line;
	static	char	*line = NULL;
	static	size_t	allocated_length = 0;

	if (strlen(s) == 0)
	    return;

	if (strlen(s) >= allocated_length)
	{
	    if (line != NULL)
	    	free(line);
	    allocated_length = strlen(s) + 1024;
	    line = (char*) malloc(allocated_length*sizeof(char));
	}
	strcpy(line,s);

	start_of_line = YES;
	for (len = 0, c = strtok(line," "); c != NULL; c = strtok(NULL," "))
	{
	    newlen = len+(int)strlen(c);
	    if ((newlen+1) >  maxlen)
	    {
	    	screen("\n\t");
		start_of_line = YES;
	    	len = tablen;
	    	newlen = len+(int)strlen(c);
	    }
	    else if (start_of_line == NO)
	    {
	    	screen(" ");
	    	++newlen;
	    	++len;
	    }
	    screen("%s",c);
	    start_of_line = NO;
	    if (newlen == maxlen)
	    {
	    	screen("\n\t");
		start_of_line = YES;
	    	len = tablen;
	    }
	    else
	    {
	    	if (c[strlen(c)-1] == '.')
	    	{
	   	    screen(" ");
		    ++len;
		}
		len += (int)strlen(c);
	    }
	}
}		/*end screen_print_long_string*/
