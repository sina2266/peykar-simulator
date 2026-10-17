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
*				fbdry2.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	This file contains routines to determine, impose or untangle
*	front boundaries and boundary conditions. Specifically:
*
*			physical_side_of_bdry_curve()
*			is_curve_in_cross_list()
*			curve_exits()
*/

#if defined(TWOD)

#include <front/fdecs.h>


	/* LOCAL Function Declarations */



EXPORT SIDE physical_side_of_bdry_curve(
	CURVE	*c)
{
	if (wave_type(c) >= FIRST_PHYSICS_WAVE_TYPE) 
	{
	    screen("ERROR in physical_side_of_bdry_curve(), non bdry curve\n");
	    print_curve(c);
	    clean_up(ERROR);
	}
	if (is_bdry(c)) 
	{
	    return is_exterior_comp(negative_component(c),c->interface) ?
			POSITIVE_SIDE : NEGATIVE_SIDE;
	}
	else if (is_excluded_comp(negative_component(c),c->interface))
	    return POSITIVE_SIDE;
	else if (is_excluded_comp(positive_component(c),c->interface))
	    return NEGATIVE_SIDE;
	else 
	{
	    screen("ERROR in physical_side_of_bdry_curve(), "
		   "interior boundary not bounded by an excluded component\n");
	    print_curve(c);
	    clean_up(ERROR);
	}
	return UNKNOWN_SIDE; /* for lint */
}		/*end physical_side_of_bdry_curve*/

EXPORT int is_curve_in_cross_list(
	CROSS		*cross,
	CURVE		*c)
{
	CROSS		*cr = cross;

	for ( ; cr; cr = cr->next)
	    if (cr->c1 == c || cr->c2 == c)
		return YES;
	return NO;
}		/*end is_curve_in_cross_list*/

#endif /* defined(TWOD) */
