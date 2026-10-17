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
*				tri1d.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	One dimensional specific grid generation functions.
*/

#if defined(ONED)
#include <tri/trilocaldecs.h>

	/* LOCAL Function Declarations */
LOCAL	int	add_crx_to_list1d(TRI_GRID*,int,FT_POINT*,int);
LOCAL	void	count_intfc_crossings_of_dual_lattice1d(TRI_GRID*);

/*
*			insert_grid_crossings1d():
*
*/


/*
*	LOCAL variables shared by routines below.
*/

LOCAL	int	xmax;

/*ARGSUSED*/
LIB_LOCAL	int insert_grid_crossings1d(
	TRI_GRID	*ntg)
{
	INTERFACE	*intfc = ntg->grid_intfc;
	RECT_GRID	*rgr = &ntg->rect_grid;
	FT_POINT		**p;
	int		ic[MAXD];
	int		status;
	int		n_crx;

	for (n_crx = 0, p = intfc->points; p && *p; ++p)
	{
	    if (!rect_in_which(Coords(*p),ic,rgr))
	    {
	    	double x = Coords(*p)[0];
	        double L = rgr->L[0], U = rgr->U[0], h = 0.5*rgr->h[0];
	        if (((L-h) < x) && (x < L))
		{
		    ic[0] = 0;
		}
		else if ((U < x) && (x < (U+h)))
		{
		    ic[0] = rgr->gmax[0]-1;
		}
		else
		{
	    	    screen("ERROR in insert_grid_crossings1d(), "
		           "rect_in_which() failed\n");
		    clean_up(ERROR);
		}
	    }
	    status = add_crx_to_list1d(ntg,n_crx++,*p,ic[0]);

	    if (status != GOOD_STEP)
		return status;
	}
	return GOOD_STEP;
}		/*end insert_grid_crossings1d*/

LOCAL	void count_intfc_crossings_of_dual_lattice1d(
	TRI_GRID	*ntg)
{
	INTERFACE	*intfc = ntg->grid_intfc;
	RECT_GRID	*rgr = &ntg->rect_grid;
	FT_POINT		**p;
	int		ic[MAXD];
	Table		*T = table_of_interface(intfc);

	for (p = intfc->points; p && *p; ++p)
	{
	    if (rect_in_which(Coords(*p),ic,rgr) == FUNCTION_FAILED)
	    {
	    	double x = Coords(*p)[0];
	        double L = rgr->L[0], U = rgr->U[0], h = 0.5*rgr->h[0];
	        if (((L-h) < x) && (x < L))
		{
		    ic[0] = 0;
		}
		else if ((U < x) && (x < (U+h)))
		{
		    ic[0] = rgr->gmax[0]-1;
		}
		else
		{
	    	    screen("ERROR in "
		           "count_intfc_crossings_of_dual_lattice1d(), "
	    	           "rect_in_which() failed\n");
		    clean_up(ERROR);
		}
	    }
	    ++T->seg_crx_count[ic[0]];
	}
}		/*end count_intfc_crossings_of_dual_lattice1d*/


LOCAL	int add_crx_to_list1d(
	TRI_GRID	*ntg,
	int		n_crx,
	FT_POINT		*p,
	int		ix)
{
	register CRXING *cross;
	Table		*T = table_of_interface(ntg->grid_intfc);
	CRXING		*crx_store = T->crx_store;

	int		nx, i, j, k;
	int		input, hold;
	int		*start;

	cross = &(crx_store[n_crx]);
	cross->hs = Hyper_surf(p);
	cross->pt = p;
	cross->crossing_direction = BELOW_TO_ABOVE;
	cross->crx_num = n_crx;

	nx = T->seg_crx_count[ix];
	if (nx == 0)
	{
	    (void) printf("WARNING in add_crx_to_list1d(), nx = 0");
	    return MODIFY_TIME_STEP;
	}

	start = T->seg_crx_lists[ix];

	for (j = 0;  j < nx;  ++j)
	{
	    if (*(start+j) == -1)
		break;
	}

	if (j == 0)
	    *start = n_crx;
	else
	{
	    for (i = 0;  i < j;  ++i)
	    {
	    	if (Coords((crx_store[*(start+i)]).pt)[0] >Coords(cross->pt)[0])
		    break;
	    }

	    if (i < j)
	    {
	    	input = n_crx;
	    	for (k = i;  k < j;  ++k)
	    	{
	    	    hold = *(start+k);
	    	    *(start+k) = input;
	    	    input = hold;
	    	}
	    	*(start+j) = input;
	    }
	    else
	    	*(start+j) = n_crx;
	}
	return GOOD_STEP;
}		/*end add_crx_to_list1d*/

/*
*			set_components1d():
*
*	Sets the component list associated with each grid node
*	of tri_grid->rect_grid.
*/

EXPORT	int set_components1d(
	TRI_GRID	*ntg,
	INTERFACE	*intfc)
{
	RECT_GRID    *gr = &ntg->rect_grid;
	double        *edges;
	int	     xmax = gr->gmax[0];
	int	     ix;
	FT_POINT	     **p, *pt;
	Table	     *T = table_of_interface(ntg->grid_intfc);
	COMPONENT    *comps = T->components;

	debug_print("set_components","Entered set_components()\n");

	if (intfc->points == NULL) /* Tracking turned off */
	    return set_untracked_components(ntg,intfc);

	ix = 0;
	edges = gr->edges[0];
	for (p = intfc->points; p && *p; ++p)
	{
	    for (; ix <= xmax; ++ix)
	    {
	        if (edges[ix] < Coords(*p)[0])
	            comps[ix] = negative_component(*p);
	        else
	            break;
	    }
	}
	pt = intfc->points[intfc->num_points-1];
	for (; ix <= xmax; ++ix)
	    comps[ix] = positive_component(pt);

	debug_print("set_components",
		"Leaving set_components(), status = GOOD_STEP\n");
	return GOOD_STEP;
}		/*end set_components1d*/

LIB_LOCAL	void set_crx_structure_storage1d(
	TRI_GRID	*ntg)
{
	RECT_GRID	*r_gr = &ntg->rect_grid;
	INTERFACE	*intfc = ntg->grid_intfc;
	int		n_crx, ii, n_segs;
	Table		*T = table_of_interface(intfc);

	xmax = r_gr->gmax[0];

	n_segs = xmax;
	uni_array(&T->seg_crx_count,n_segs,INT);
	for (ii = 0;  ii < n_segs;  ++ii)
		T->seg_crx_count[ii] = 0;

	ntg->n_crx = n_crx = intfc->num_points;
	count_intfc_crossings_of_dual_lattice1d(ntg);

	init_seg_crx_lists(intfc,n_crx,n_segs);

	ntg->guessed_num_lin = 0;
	ntg->lin_els = NULL;
	ntg->bilin_els = NULL;

	ntg->n_node_points += n_crx;
	ntg->n_fr_pts = intfc->num_points;
	alloc_node_points(ntg,ntg->n_node_points);

	ntg->n_tg_pts = 0;

	ntg->blk_els0 = NULL;
	ntg->blk_els1 = NULL;
}		/*end set_crx_structure_storage1d*/

/*ARGSUSED*/
LIB_LOCAL   int no_triangulated_mesh(
        TRI_GRID        *ntg)
{
        return GOOD_STEP;
}               /*end no_triangulated_mesh*/
#endif /* defined(ONED) */
