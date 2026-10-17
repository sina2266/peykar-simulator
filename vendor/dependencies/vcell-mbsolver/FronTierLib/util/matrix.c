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
*				bi_array.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	Contains simple routines for the manipulation of matrices.
*
*		rotate_matrix()
*		rotate_vector()
*/

#include <cdecs.h>
#include <vmalloc.h>

/*
*	               rotate_vector():
*	               rotate_matrix():
*
*	Multiply the given vector by the matrix M.
*	Multiply two matricies.  Both functions assume the
*	dimension of these objects is at most 3.  The function
*	MatrixTimesMatix also assumes the matricies are square.
*/

EXPORT void rotate_vector(
	double		*rv,
	double		**M,
	double		*v,
	int		dim)
{
	int		i, j;
	double		vtemp[3];
	double		*v1;

	if (v == rv)
	{
		for (i = 0; i < dim; i++) vtemp[i] = v[i];
		v1 = vtemp;
	}
	else
		v1 = v;

	for (i = 0; i < dim; i++)
	{
		rv[i] = 0.0;
		for (j = 0; j < dim; j++)
			rv[i] += M[i][j]*v1[j];
	}
}		/*end rotate_vector*/

EXPORT void rotate_matrix(
	double		**M,
	double		**M1,
	double		**M2,
	int		dim)
{
	int		i, j, k;
	double		**m1, **m2;
	static boolean	first = YES;
	static double	**mtmp1 = NULL, **mtmp2 = NULL;

	if (first)
	{
	    first = NO;
	    bi_array(&mtmp1,3,3,FLOAT);
	    bi_array(&mtmp2,3,3,FLOAT);
	}

	if (M == M1)
	{
	    for (i = 0; i < dim; i++)
	    	for (j = 0; j < dim; j++)
	    	    mtmp1[i][j] = M1[i][j];
	    m1 = mtmp1;
	}
	else
	    m1 = M1;

	if (M == M2)
	{
	    for (i = 0; i < dim; i++)
	    	for (j = 0; j < dim; j++)
	    	    mtmp2[i][j] = M2[i][j];
	    m2 = mtmp2;
	}
	else
	    m2 = M2;

	for (i = 0; i < dim; i++)
	{
	    for (j = 0; j < dim; j++)
	    {
	    	M[i][j] = 0.0;
	    	for (k = 0; k < dim; k++)
	    	    M[i][j] += m1[i][k]*m2[k][j];
	    }
	}
}		/*end rotate_matrix*/
