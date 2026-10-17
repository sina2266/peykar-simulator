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
*				shift.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	Routines for shifting from one interface to another.
*/

#include <intfc/iloc.h>


	/* LOCAL Function Prototypes */

/*
*			remap_interface():
*
*	Creates an interface that is the image of the given
*	interface under the user defined remapping function
*	remap.
*/



EXPORT INTERFACE *remap_interface(
	INTERFACE	*intfc,
	void		(*remap)(FT_POINT*,BOND*,CURVE*,FT_POINT*,
				 BOND*,CURVE*,boolean,RECT_GRID*,POINTER),
	void		(*remap_rect_grid)(INTERFACE*,INTERFACE*,
					   void (*)(FT_POINT*,BOND*,CURVE*,
						    FT_POINT*,BOND*,CURVE*,boolean,
						    RECT_GRID*,POINTER),
					   POINTER),
	POINTER		params)
{
	HYPER_SURF	   *hs, *nhs;
	HYPER_SURF_ELEMENT *hse, *nhse;
	FT_POINT		   *p, *np;
	INTERFACE	   *new_intfc;
	RECT_GRID	   *rgr;
	boolean		   first = YES;
	double              eps;

	if (DEBUG)
	    (void) printf("Entering remap_interface(%llu)\n",
		          interface_number(intfc));
	if (remap == NULL || remap_rect_grid == NULL)
	    return NULL;
	if (exists_interface(intfc) != YES)
	    return NULL;
	if ((new_intfc = copy_interface(intfc)) == NULL)
	    return NULL;
	rgr = &topological_grid(new_intfc);

	/* TODO:  check for validity at hyper surface boundaries */
	(void) next_point(intfc,NULL,NULL,NULL);
	(void) next_point(new_intfc,NULL,NULL,NULL);
	while (next_point(intfc,&p,&hse,&hs) &&
	       next_point(new_intfc,&np,&nhse,&nhs))
	{
	    (*remap)(p,Bond_of_hse(hse),Curve_of_hs(hs),np,
	    	     Bond_of_hse(nhse),Curve_of_hs(nhs),first,rgr,params);
	    first = NO;
	}

		/* Remap the Underlying Grid: */

	(*remap_rect_grid)(intfc,new_intfc,remap,params);

		/* Reset boundary flags */

	eps = grid_tolerance(rgr);
#if defined(ONED)
	if (new_intfc->dim == 1)
	{
	    FT_POINT **pt;
	    for (pt = new_intfc->points; pt && *pt; pt++)
	    {
		if (boundary_side(Coords(*pt),rgr,eps) == NOT_A_BDRY)
		    set_not_bdry(*pt);
		else
		    set_is_bdry(*pt);
	    }
	}
#endif /* defined(ONED) */
#if defined(TWOD)
	if (new_intfc->dim == 2)
	{
	    CURVE **c;
	    NODE  **n;
	    for (n = new_intfc->nodes; n && *n; n++)
	    {
		if (boundary_side(Coords((*n)->posn),rgr,eps) == NOT_A_BDRY)
		    set_not_bdry(*n);
		else
		    set_is_bdry(*n);
	    }
	    for (c = new_intfc->curves; c && *c; c++)
	    {
	        BDRY_SIDE side_start, side_end, side;
		BOND      *b;
	        NODE      *ns, *ne;

		ns = (*c)->start;
		ne = (*c)->end;
		side_start = boundary_side(Coords(ns->posn),rgr,eps);
		side_end = boundary_side(Coords(ne->posn),rgr,eps);
		if ((side_start != side_end) || (side_start == NOT_A_BDRY))
		{
		    set_not_bdry(*c);
		    continue;
		}
		side = side_start;
		for (b = (*c)->first; b != (*c)->last; b = b->next)
		{
		    if (boundary_side(Coords(b->end),rgr,eps) != side)
		    {
			side = NOT_A_BDRY;
			break;
		    }
		}
		if (side == NOT_A_BDRY)
		    set_not_bdry(*c);
		else
		    set_is_bdry(*c);
	    }
	}
#endif /* defined(TWOD) */
#if defined(THREED)
	if (new_intfc->dim == 3)
	{
	    CURVE   **c;
	    NODE    **n;
	    SURFACE **s;
	    for (n = new_intfc->nodes; n && *n; n++)
	    {
		if (boundary_side(Coords((*n)->posn),rgr,eps) == NOT_A_BDRY)
		    set_not_bdry(*n);
		else
		    set_is_bdry(*n);
	    }
	    for (c = new_intfc->curves; c && *c; c++)
	    {
	        BDRY_SIDE side_start, side_end, side;
		BOND      *b;
	        NODE      *ns, *ne;

		ns = (*c)->start;
		ne = (*c)->end;
		side_start = boundary_side(Coords(ns->posn),rgr,eps);
		side_end = boundary_side(Coords(ne->posn),rgr,eps);
		if ((side_start != side_end) || (side_start == NOT_A_BDRY))
		{
		    set_not_bdry(*c);
		    continue;
		}
		side = side_start;
		for (b = (*c)->first; b != (*c)->last; b = b->next)
		{
		    if (boundary_side(Coords(b->end),rgr,eps) != side)
		    {
			side = NOT_A_BDRY;
			break;
		    }
		}
		if (side == NOT_A_BDRY)
		    set_not_bdry(*c);
		else
		    set_is_bdry(*c);
	    }
	    for (s = new_intfc->surfaces; s && *s; s++)
	    {
		TRI       *t;
		BDRY_SIDE side0, side1, side2, side;
		const double     *p0, *p1, *p2;
		t = first_tri(*s);
		p0 = Coords(Point_of_tri(t)[0]);
		p1 = Coords(Point_of_tri(t)[1]);
		p2 = Coords(Point_of_tri(t)[2]);
		side0 = boundary_side(p0,rgr,eps);
		side1 = boundary_side(p1,rgr,eps);
		if ((side0 != side1) || (side0 == NOT_A_BDRY))
		{
		    set_not_bdry(*s);
		    continue;
		}
		side = side0;
		side2 = boundary_side(p2,rgr,eps);
		if (side2 != side0)
		{
		    set_not_bdry(*s);
		    continue;
		}
		for (t = t->next; !at_end_of_tri_list(t,*s); t = t->next)
		{
		    p0 = Coords(Point_of_tri(t)[0]);
		    p1 = Coords(Point_of_tri(t)[1]);
		    p2 = Coords(Point_of_tri(t)[2]);
		    if ((boundary_side(p0,rgr,eps) != side) ||
		        (boundary_side(p1,rgr,eps) != side) ||
		        (boundary_side(p2,rgr,eps) != side))
		    {
			side = NOT_A_BDRY;
			break;
		    }
		}
		if (side == NOT_A_BDRY)
		    set_not_bdry(*s);
		else
		    set_is_bdry(*s);
	    }
	}
#endif /* defined(THREED) */


	if (DEBUG) (void) printf("Leaving remap_interface()\n\n");
	return new_intfc;
}		/*end remap_interface*/
