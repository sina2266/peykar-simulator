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
*				setb1d.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*/



#if defined(ONED)
#define DEBUG_STRING    "setb1d"
#include <intfc/iloc.h>

/*
*				i_set_boundary1d():
*
*	Sets the boundary for an interface defined in a rectangle.
*	The interface may be arbitrarily complicated.   It is assumed
*	that all interface points lie within or on the rectangle.
*	If the interface already contains boundary curves, these will
*	be discarded.   The routine considers the given interior
*	curves and constructs a set of boundary curves such that the
*	rectangle is divided into connected components each of exactly
*	one component number.   The component number of the exterior
*	is arbitrarily ft_assigned as one greater that that of any interior
*	region.   The new boundary curves are then added to the
*	interface.   Corresponding NODES have their boundary field set.
*
*	This routine sets the current interface to intfc.
*
*	Returns 1 if successful or zero on error.
*	The routine checks the validity of the component ft_assignments of the
*	given interface and returns 0 if these are not consistent.   A zero
*	return may also be caused by an invalid interface pointer.
*/

EXPORT boolean i_set_boundary1d(
	INTERFACE	*intfc,
	RECT_GRID	*gr,
	COMPONENT	i_comp,
	double		eps)	/* boundary distance tolerance */
{
	int		outside;	/* Component Number of Exterior */
	double		*L = gr->VL;
	double		*U = gr->VU;
	FT_POINT		*pt, *pointl, *pointr;

	DEBUG_ENTER(i_set_boundary1d)

		/* Check Validity of Interface and Set Storage Allocation: */

#if defined(DEBUG_SETB1D)
	if (DEBUG)
	{
	    print_general_vector("L = ",L,intfc->dim,NO);
	    print_general_vector(", U = ",U,intfc->dim,NO);
	    (void) printf(", eps = %g, i_comp = %d\n",eps,i_comp);
	    print_interface(intfc);
	}
#endif /* defined(DEBUG_SETB1D) */

	if (exists_interface(intfc) != YES)
	{
	    DEBUG_LEAVE(i_set_boundary1d)
	    return FUNCTION_FAILED;
	}
	set_current_interface(intfc);

		/* Get rid of Old Boundary Points if Any: */

	outside = exterior_component(intfc);

#if defined(DEBUG_SETB1D)
	if (DEBUG)  (void) printf("\nOutside Component = %d\n\n",outside);
#endif /* defined(DEBUG_SETB1D) */

	if (intfc->num_points > 0)
	{
	    pt = intfc->points[0];
	    if (!is_bdry(pt) || (Coords(pt)[0] > (L[0]+eps)))
	    {
	        pointl = make_point(L,outside,negative_component(pt));
	        set_is_bdry(pointl);
		if (is_bdry(pt) && (Coords(pt)[0] <= (L[0]+eps)))
		    delete_point(pt);
	    }
	    else
	    {
	        pointl = pt;
	        Coords(pointl)[0] = L[0];
	    }
	    pt = intfc->points[intfc->num_points-1];
	    if (!is_bdry(pt) || (Coords(pt)[0] < (U[0]-eps)))
	    {
	        pointr = make_point(U,positive_component(pt),outside);
	        set_is_bdry(pointr);
		if (is_bdry(pt) && (Coords(pt)[0] >= (U[0]-eps)))
		    delete_point(pt);
	    }
	    else
	    {
	        pointr = pt;
	        Coords(pointr)[0] = U[0];
	    }
	}
	else
	{
	    pointl = make_point(L,outside,i_comp);
	    set_is_bdry(pointl);
	    pointr = make_point(U,i_comp,outside);
	    set_is_bdry(pointr);
	}
	if (intfc->table->new_grid || intfc->modified)
	    if (!make_point_comp_lists(intfc))
		return FUNCTION_FAILED;

#if defined(DEBUG_SETB1D)
	if (DEBUG)
	{
	    print_interface(intfc);
	    (void) printf("Leaving set_boundary1d()\n\n");
	}
#endif /* defined(DEBUG_SETB1D) */
	DEBUG_LEAVE(i_set_boundary1d)
	return FUNCTION_SUCCEEDED;
}		/*end i_set_boundary1d*/


#endif /* defined(ONED) */
