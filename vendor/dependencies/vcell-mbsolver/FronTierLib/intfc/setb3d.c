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
*				setb3d.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	The main purpose of this file is to add to a given interface a
*	boundary, in the form of the surface of a rectangular solid
*	with specified coordinates.
*/

#define DEBUG_STRING    "setb3d"
#include <intfc/int.h>
#include <intfc/iprotos.h>

	/* LOCAL Function Declarations */
LOCAL	int	set_boundary_surface_components(INTERFACE*,INTERFACE*,
						COMPONENT);

EXPORT boolean i_set_boundary3d(
	INTERFACE	*intfc,
	RECT_GRID	*gr,
	COMPONENT	default_comp,
	double		eps)
{
	INTERFACE	*old_intfc;
	int		status;
	SURFACE         **s;

	DEBUG_ENTER(i_set_boundary3d)

	if (!exists_interface(intfc))
	{
	    DEBUG_LEAVE(i_set_boundary3d)
	    return NO;
	}

	set_current_interface(intfc);

	old_intfc = (!intfc->surfaces || !*(intfc->surfaces)) ?
			NULL : copy_interface(intfc);
	set_current_interface(intfc);

	make_bdry_surfaces(intfc,gr);

	/* Set components of boundary surfaces */
 
	status = set_boundary_surface_components(intfc,old_intfc,default_comp);

	if (old_intfc != NULL)
	    (void) delete_interface(old_intfc);
	
	if (! status)
	{
	    DEBUG_LEAVE(i_set_boundary3d)
	    return NO;
	}

	DEBUG_LEAVE(i_set_boundary3d)
	return YES;
}		/*end i_set_boundary3d*/


LOCAL int set_boundary_surface_components(
	INTERFACE	*intfc,
	INTERFACE	*old_intfc,
	COMPONENT	default_comp)
{
	SURFACE		**s;
	double		coords[MAXD];
	int		i, dim = intfc->dim;

	DEBUG_ENTER(set_boundary_surface_components)

		/* Set the pos bdry comps to exterior; neg to NO_COMP */

	for (s = intfc->surfaces; s && *s; s++)
	{
	    if (! Boundary(*s))
		continue;
	    if ((positive_component(*s) == NO_COMP) && (old_intfc == NULL))
	        positive_component(*s) = intfc->default_comp;
	    if (negative_component(*s) == NO_COMP)
	        negative_component(*s) = exterior_component(intfc);
	}

	if (old_intfc != NULL)
	{
	    for (s = intfc->surfaces; s && *s; s++)
	    {
	        TRI *tri;

	        if (! Boundary(*s))
		    continue;
	        if (positive_component(*s) != NO_COMP)
		    continue;
	        for (tri = first_tri(*s); !at_end_of_tri_list(tri,*s);
	    	     tri = tri->next)
	        {
	            if (!Boundary_tri(tri))
			break;
	        }
	        if (at_end_of_tri_list(tri,*s))
		    tri = first_tri(*s);
	        for (i = 0; i < dim; i++)
	        {
	            coords[i] = (Coords(Point_of_tri(tri)[0])[i]+
	    			 Coords(Point_of_tri(tri)[1])[i]+
	    			 Coords(Point_of_tri(tri)[2])[i])/3.0;
	        }
	        positive_component(*s) = long_component(coords,old_intfc);
	    }
	}

	DEBUG_LEAVE(set_boundary_surface_components)
	return 1;
}		/*end set_boundary_surface_components*/
