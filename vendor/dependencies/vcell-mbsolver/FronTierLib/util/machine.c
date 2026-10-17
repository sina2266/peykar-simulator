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
*			machine.c
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	This file contains simple routines for computing machine
*	dependent parameters. The parameters supported are:
*		
*	d1_mach(int)	- machine double precision constants
*	r1_mach(int)	- machine single precision constants
*	ft_endian_type(void) - enum defining machine endian
*	ft_endian_name(void) - character string defining machine endian 
*	reverse_string(char*,size_t) - reverses a character string
*	
*/

#include <cdecs.h>

/*
*  double-precision machine constants
*  d1mach( 1) = b**(emin-1), the smallest positive magnitude. 
*  d1mach( 2) = b**emax*(1 - b**(-t)), the largest magnitude.
*  d1mach( 3) = b**(-t), the smallest relative spacing.
*  d1mach( 4) = b**(1-t), the largest relative spacing.
*  d1mach( 5) = log10(b) 
*
*  b = 2 = base of number system
*/

EXPORT	double d1_mach(int i)
{
    static boolean    first = YES;
    static double dmach[5];

    if (first)
    {
	first = NO;
	dmach[0] = DBL_MIN;
	dmach[1] = DBL_MAX;
	dmach[2] = 0.5*DBL_EPSILON;
	dmach[3] = DBL_EPSILON;
	dmach[4] = log10(2.0);
    }

    if (i < 1 || i > 5)
    {
        (void) printf("d1mach - i = %d out of bounds\n",i);
        return 0.0;
    }
    return dmach[i - 1];
}		/*end d1_mach */

/*
*			r1_mach():
*
*  single-precision machine constants
*  r1_mach( 1) = b**(emin-1), the smallest positive magnitude. 
*  r1_mach( 2) = b**emax*(1 - b**(-t)), the largest magnitude.
*  r1_mach( 3) = b**(-t), the smallest relative spacing.
*  r1_mach( 4) = b**(1-t), the largest relative spacing.
*  r1_mach( 5) = log10(b) 
*
*  b = 2 = base of number system
*/

EXPORT TRUEfloat r1_mach(int i)
{
    static boolean   first = YES;
    static TRUEfloat rmach[5];

    if (first)
    {
	first = NO;
	rmach[0] = FLT_MIN;
	rmach[1] = FLT_MAX;
	rmach[2] = 0.5*FLT_EPSILON;
	rmach[3] = FLT_EPSILON;
	rmach[4] = log10(2.0);
    }


    if (i < 1 || i > 5)
    {
        (void) printf("r1mach - i = %d out of bounds\n",i);
        return 0.0;
    }
    return rmach[i - 1];
}	/* end FORTRAN_NAME(r1mach) */

EXPORT	FT_ENDIAN ft_endian_type(void)
{
	char    *c_one;
	size_t  i, one;
	boolean    big_endian, little_endian;
	static  FT_ENDIAN endian = FT_UNKNOWN_ENDIAN;
	static boolean first = YES;

	if (first)
	{
	    first = NO;
	    one = 1;
	    c_one = (char*)malloc(sizeof(size_t));
	    for (i = 0; i < sizeof(size_t); ++i)
	        c_one[i] = '\0';
	    c_one[sizeof(size_t)-1] = 1;
	    (void) memcpy((void*)&i,(const void*)c_one,sizeof(size_t));
	    big_endian = (i == one) ? YES : NO;
	    c_one[sizeof(size_t)-1] = '\0';
	    c_one[0] = 1;
	    (void) memcpy((void*)&i,(const void*)c_one,sizeof(size_t));
	    little_endian = (i == one) ? YES : NO;
	    if (big_endian && !little_endian)
	        endian = FT_BIG_ENDIAN;
	    else if (!big_endian && little_endian)
	        endian = FT_LITTLE_ENDIAN;
	    else
	        endian = FT_UNKNOWN_ENDIAN;
	    free(c_one);
	}
	return endian;
}		/*end ft_endian_type*/

EXPORT	const char *ft_endian_name(
	FT_ENDIAN endian)
{
	switch (endian)
	{
	case FT_BIG_ENDIAN:
	    return "FT_BIG_ENDIAN";
	case FT_LITTLE_ENDIAN:
	    return "FT_LITTLE_ENDIAN";
	case FT_UNKNOWN_ENDIAN:
	    return "FT_UNKNOWN_ENDIAN";
	default:
	    return "INVALID ENDIAN VALUE";
	}
}		/*end ft_endian_name*/

EXPORT	void reverse_string(char *s,size_t n)
{
	size_t i;
	char   c;

	for (i = 0; i < n/2; ++i)
	{
	    c = s[i];
	    s[i] = s[n-1-i];
	    s[n-1-i] = c;
	}
}

#if defined(_HPUX_SOURCE) || defined(cray)
EXPORT	double rint(double x)
{
	return ceil(x+0.5)-1.0;
}		/*end rint*/

EXPORT	double	copysign(double x, double y)
{
	return (y >= 0.0) ? fabs(x) : -fabs(x);
}		/*end copysign*/

EXPORT	double	log1p(double x)
{
	return	log(1.0 + x);
}		/*end log1p*/

EXPORT	double	expm1(double x)
{
	return	exp(x) - 1.0;
}		/*end expm1*/
#endif /* defined(_HPUX_SOURCE) || defined(cray) */

#if !defined(sun) || (defined(__SUNPRO_C) || defined(__SUNPRO_CC)) || defined(_HPUX_SOURCE) || defined(cray) || (defined(__GNUC__) && !defined(linux))
EXPORT	int irint(double x)
{
	return (int) rint(x);
}		/*end irint*/
#endif /* !defined(sun) || (defined(__SUNPRO_C) || defined(__SUNPRO_CC)) || defined(_HPUX_SOURCE)  || defined(cray) || (defined(__GNUC__) && !defined(linux)) */

EXPORT	char *get_basename(
	char		*name)
{
	char		*c;
	size_t		len;

	if (name == NULL)
	    return (char*)"";
	len = strlen(name);
	if (len == 0) return (char*)"";
	for (c = name + len; len != 0; c--, len--)
	    if (*(c-1) == '/') return c;
	return c;
}		/*end get_basename*/

EXPORT	char *get_dirname(
	char		*name)
{
	char		*c;
	size_t		len;

	if (name == NULL) return (char*)"";
	len = strlen(name);
	if (len == 0) return (char*)"";
	for (c = name + len; len != 0; c--, len--)
	{
	    if (*(c-1) == '/')
	    {
	    	*(c-1) = '\0';
	    	break;
	    }
	}
	if (len == 0)
	    return (char*)"";
	if (strcmp(name,".")==0)
	    return (char*)"";
	return name;
}		/*end get_dirname*/
