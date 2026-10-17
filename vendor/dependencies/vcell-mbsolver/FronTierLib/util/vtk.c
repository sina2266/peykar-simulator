/************************************************************************************
FronTier is a set of libraries that implements differnt types of Front Traking algorithms.
Front Tracking is a numerical method for the solution of partial differential equations 
whose solutions have discontinuities.  


Copyright (C) 1999 by The University at Stony Brook. 
 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

******************************************************************************/


/*
*                       vtk.c
*
*   This file provides functions for determining endianness and 
*   endian swapping during vtk print routines.
*
*/
#include <cdecs.h>

float endian_float_swap(float f)
{
        int j;
        const int size = sizeof(float);
        union
        {
            float f;
            unsigned char b[16];
        } dat1, dat2;

        dat1.f = f;
        for(j = 0; j < size; ++j)
   	    dat2.b[j] = dat1.b[size-1-j];

        return dat2.f;
}       /* end endian_float_swap */

double endian_double_swap(double f)
{
        int j;
        const int size = sizeof(double);
        union
        {
            double f;
            unsigned char b[16];
        } dat1, dat2;

        dat1.f = f;
        for(j = 0; j < size; ++j)
   	    dat2.b[j] = dat1.b[size-1-j];

        return dat2.f;
}       /* end endian_float_swap */

int endian_int_swap(int i)
{
        int j;
        int const size = sizeof(int);
        union
	{
	    int i; 
            unsigned char b[16];
        } dat1, dat2;
        
	dat1.i = i;
        for(j = 0; j < size; ++j)
	    dat2.b[j] = dat1.b[size-1-j];

	return dat2.i;
}	/* end endian_int_swap */

boolean hardware_is_little_endian()
{
    	short int word = 0x0001;
    	char *byte = (char *) &word;
    	if(byte[0] == 1) 
	    return YES;
    	else
            return NO;
}       /* end hardware_is_little_endian() */

int count_digits(int num)
{
        int digits = 1;

        while(num > 0)
        {
            if(num/10 > 0)
            {
                digits++;
                num = num/10;
            }
            else
                return digits;
        }
        return digits;
}  	/* end count_digits */
