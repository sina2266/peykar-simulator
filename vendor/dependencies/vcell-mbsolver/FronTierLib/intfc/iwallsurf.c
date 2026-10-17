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
*				iwallsurf.c:
*
*       Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	Containing function of rebuilding interface within a mesh block.
*
*/

#define DEBUG_STRING "i_wall_surf"

#include <intfc/int.h>

LOCAL  int compare_surf(const void *, const void *);

EXPORT  void  set_is_not_wall_surface(SURFACE *s)
{
	s->number = INSIDE_WALL;
}
EXPORT  void  set_is_wall_surface(SURFACE *s)
{
	s->number = ON_WALL;
}

EXPORT  boolean  is_wall_surface(SURFACE *s)
{
	if(s->number == ON_WALL)
	    return YES;
	return NO;
}

/*ASSUME fluid comp is 2 and 3 */
EXPORT void  get_default_fluid_comp(
	int	   *comp1, 
	int	   *comp2,
	INTERFACE  *intfc)
{
	*comp1 = 2;
	*comp2 = 3;
}

EXPORT void  reset_wall_flag_for_surface(INTERFACE  *intfc)
{
SURFACE    **s;

	for(s=intfc->surfaces; s && *s; s++)
	    (*s)->number = UNKNOWN_DOMAIN;
}

EXPORT void  set_wall_flag_for_surface(INTERFACE *intfc)
{
SURFACE   **s;
int	  comp1, comp2;

	get_default_fluid_comp(&comp1,&comp2,intfc);

	for(s=intfc->surfaces; s && *s; s++)
	    if(positive_component(*s) == comp1 && negative_component(*s) == comp2 || 
	       positive_component(*s) == comp2 && negative_component(*s) == comp1)
	    {
		set_is_not_wall_surface(*s);
	    }
	    else
	    {
		set_is_wall_surface(*s);
	    }
}


LOCAL  int compare_surf(const void *a, const void *b)
{
SURFACE **c1=(SURFACE**)a, **c2=(SURFACE**)b;

	if(positive_component(*c1) < positive_component(*c2))
	    return -1;
	else  if(positive_component(*c1) > positive_component(*c2))
	    return 1;
	else  if(negative_component(*c1) < negative_component(*c2))
	    return -1;
	else  if(negative_component(*c1) > negative_component(*c2))
	    return 1;
	
	return 0;
}

/*
 * determine the 3 comp curve orientation:
 *
 * 	  p | n
 *	    ^
 *	n   |       n
 * ----<----x---<-----
 *      p           p
 */

EXPORT  int  add_to_o_surfaces(
	O_SURFACE    **sarr,
	int	     *n,
	SURFACE      *sp[4])
{
int	  i, j, pc, nc;
SURFACE   *s_tmp;

	/*arrange the surfaces and assume the sp[2] is physical */
	qsort((POINTER)sp, 3, sizeof(SURFACE*), compare_surf);
	
	j = -1;
	for(i=0; i<3; i++)
	{
	    if(!is_wall_surface(sp[i]))
	        j = i;
	}
	
	/*sp[2] is the physical surface */
	if(j == 1 || j == 0)
	{
	    s_tmp = sp[2];
	    sp[2] = sp[j];
	    sp[j] = s_tmp;
	}
			
	for(j=0; j<*n; j++)
	    if(sp[0] == sarr[j][0].surface && 
	       sp[1] == sarr[j][1].surface &&
	       sp[2] == sarr[j][2].surface )
	       return j;

	for(j=0; j<3; j++)
	    sarr[*n][j].surface = sp[j];

	/*the physical surface has the same orient as the curve. */
	sarr[*n][2].orient = POSITIVE_ORIENTATION;
	pc = positive_component(sp[2]);
	nc = negative_component(sp[2]);

	for(j=0; j<2; j++)
	{
	    if(positive_component(sp[j]) == pc)
	        sarr[*n][j].orient = NEGATIVE_ORIENTATION;
	    else if(positive_component(sp[j]) == nc)
	        sarr[*n][j].orient = POSITIVE_ORIENTATION;
	    else if(negative_component(sp[j]) == pc)
	        sarr[*n][j].orient = POSITIVE_ORIENTATION;
	    else if(negative_component(sp[j]) == nc)
	        sarr[*n][j].orient = NEGATIVE_ORIENTATION;
	    else
	    {
	        printf("ERROR  add_to_o_surface sp[%d] impossible components\n", j);
	        clean_up(ERROR);
	    }
	}

	(*n)++;
	return -1;
}





