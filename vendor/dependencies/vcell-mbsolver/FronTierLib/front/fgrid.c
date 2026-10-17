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
 * 			fgrid.c:
 *
 */

#include <front/fdecs.h>

LOCAL	int set_grid_intfc_components1d(INTERFACE*,INTERFACE*);
LOCAL	int set_grid_intfc_components2d(INTERFACE*,INTERFACE*);
LOCAL	COMPONENT set_comp2d(COMPONENT,COMPONENT,COMPONENT,COMPONENT,COMPONENT);
LOCAL	int inconsistent_cross2d(int,int,int,int,COMPONENT,COMPONENT,
		COMPONENT,COMPONENT,COMPONENT,int,int,int,int,int*,int*,
		int*,int*,INTERFACE*,INTERFACE*);
LOCAL	void count_crossings(INTERFACE*,double,double,double,double,int,int,int,
			int,double,double,double,double,CURVE*,int*,RECT_GRID*);
LOCAL	int  insert_grid_intfc_crossings1d(INTERFACE*);
LOCAL	int  insert_grid_intfc_crossings2d(INTERFACE*);
LOCAL	int  count_grid_intfc_crossings1d(INTERFACE*);
LOCAL	int  count_grid_intfc_crossings2d(INTERFACE*);
LOCAL	int  set_grid_intfc_components3d(INTERFACE*,INTERFACE*);
LOCAL	int  set_untracked_components(INTERFACE*,INTERFACE*);
LOCAL   int  add_crx_to_list1d(INTERFACE*,int,FT_POINT*,int);
LOCAL	void show_grid_components2d(int*,int*,int,INTERFACE*);
LOCAL	void show_grid_components3d(int*,int*,int,INTERFACE*);
LOCAL	void print_side_crx_list(INTERFACE*,char,int,int*);

EXPORT	int set_grid_intfc_components(
	INTERFACE	*grid_intfc,
	INTERFACE	*intfc)
{
	int dim = intfc->dim;
	switch(dim)
	{
	case 1:
	    return set_grid_intfc_components1d(grid_intfc,intfc);
	case 2:
	    return set_grid_intfc_components2d(grid_intfc,intfc);
	case 3:
	    return set_grid_intfc_components3d(grid_intfc,intfc);
	}
}	/* end set_grid_intfc_components */

/*
*			set_grid_intfc_components1d():
*
*	Sets the component list associated with each grid node
*	of topological_grid(grid_intfc).
*/

LOCAL	int set_grid_intfc_components1d(
	INTERFACE	*grid_intfc,
	INTERFACE	*intfc)
{
	RECT_GRID    *gr = &topological_grid(grid_intfc);
	double        *edges;
	int	     xmax = gr->gmax[0];
	int	     ix;
	FT_POINT	     **p, *pt;
	Table	     *T = table_of_interface(grid_intfc);
	COMPONENT    *comps = T->components;

	debug_print("set_components","Entered set_components()\n");

	if (intfc->points == NULL) /* Tracking turned off */
	    return set_untracked_components(grid_intfc,intfc);

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
}		/*end set_grid_intfc_components1d*/

/*
*			set_grid_intfc_components2d():
*
*	Sets the component list associated with each grid node
*	of tri_grid->rect_grid.
*/

LOCAL	int set_grid_intfc_components2d(
	INTERFACE	*grid_intfc,
	INTERFACE	*intfc)
{
	COMPONENT	cS, cE, cN, cW;
	COMPONENT	*comp;
	COMPONENT	ext_comp = exterior_component(grid_intfc);
	Table		*T = table_of_interface(grid_intfc);
	CRXING	 	*cr;
	CRXING		*crx_store = T->crx_store;
	RECT_GRID	*gr = &topological_grid(grid_intfc);
	double		coords[MAXD];
	int		xmax = gr->gmax[0], ymax = gr->gmax[1];
	int		xmaxx = xmax+1;
	int		ix, iy, ix0;
	int		k_s, k_w, k_n, k_e, ncs, ncw, ncn, nce;
	int		*s_list, *e_list, *n_list, *w_list;
	int		*seg_crx_count = T->seg_crx_count;
	int		**seg_crx_lists = T->seg_crx_lists;

	debug_print("set_components","Entered set_components2d()\n");
	if (debugging("set_comp_intfc"))
	{
	    (void) printf("\tinput interface into set_components2d()\n");
	    print_interface(intfc);
	    (void) printf("\tgrid interface into set_components2d()\n");
	    print_interface(grid_intfc);
	}

	if (intfc->curves == NULL) /* Tracking turned off */
	    return set_untracked_components(grid_intfc,intfc);

	comp = T->components;

	/* Set components on lower boundary */

	/*
	*  Try to find at least one crossing near
	*  the lower boundary to get started
	*/

	for (ix0 = 0; ix0 <= xmax; ++ix0)
	{
	    k_n = ix0 + xmax;
	    ncn    = seg_crx_count[k_n];

	    if (ix0 < xmax)
	    {
	    	k_e = ix0;
	    	nce    = seg_crx_count[k_e];
	    }
	    else
	    {
	    	k_e = -1;	nce = 0;
	    }
	    if (ncn > 0 || nce > 0)
		break;
	}

	if (ix0 > xmax)
	{
	    /*
	     *  There are no crossings near the lower boundary
	     *  get nearby component from original interface
	     */

	    coords[0] = grid_center_coord(0,gr);
	    coords[1] = gr->L[1];
	    cN = component(coords,intfc);
	    for (ix = 0; ix <= xmax; ++ix)
		comp[ix] = cN;
	}
	else
	{
	    /* Set components up to starting point found above */

	    cS = cE = cN = cW = ext_comp;
	    k_s = -1;	ncs = 0;	s_list = NULL;
	    if (ncn > 0)
	    {
	    	n_list = seg_crx_lists[k_n];
	    	cr = crx_store + *n_list;
	    	cN = (cr->crossing_direction == LEFT_TO_RIGHT) ?
				positive_component(cr->hs) : 
				negative_component(cr->hs);
	    }

	    if (nce > 0)
	    {
	    	e_list = seg_crx_lists[k_e];
	    	cr = crx_store + *e_list;
	    	cE = (cr->crossing_direction == BELOW_TO_ABOVE) ?
				negative_component(cr->hs) :
				positive_component(cr->hs);
	    }

	    comp[0] = set_comp2d(cS,cE,cN,cW,ext_comp);
	    if (comp[0] == NO_COMP)
	    {
	        return inconsistent_cross2d(ix0,0,xmax,ymax,cS,cE,cN,cW,
			            ext_comp,0,nce,ncn,0,NULL,
				    e_list,n_list,NULL,grid_intfc,intfc);
	    }

	    for (ix = 1; ix <= ix0; ++ix)
		comp[ix] = comp[0];

	    /* Set remaining components on lower boundary */

	    k_s = -1;	ncs = 0;	s_list = NULL;
	    for (ix = ix0+1; ix <= xmax; ++ix)
	    {
	    	k_w    = ix - 1;
	    	ncw    = seg_crx_count[k_w];
	    	w_list = seg_crx_lists[k_w];

	    	k_n    = ix + xmax;
	    	ncn    = seg_crx_count[k_n];
	    	n_list = seg_crx_lists[k_n];

	    	if (ix == xmax)
	    	{
	    	    k_e = -1;	nce = 0;	e_list = NULL;
	    	}
	    	else
	    	{
	    	    k_e    = ix;
	    	    nce    = seg_crx_count[k_e];
	    	    e_list = seg_crx_lists[k_e];
	    	}

		if (ncw)
		{
		    cr = crx_store + *(w_list+ncw-1);
		    cW = (cr->crossing_direction == BELOW_TO_ABOVE) ?
				positive_component(cr->hs) :
				negative_component(cr->hs);
		}
		else
		    cW = comp[ix - 1];

		if (ncn)
		{
		    cr = crx_store + *n_list;
		    cN = (cr->crossing_direction == LEFT_TO_RIGHT) ?
				positive_component(cr->hs) :
				negative_component(cr->hs);
		}
		else
		    cN = cW;

		if (nce)
		{
		    cr = crx_store + *e_list;
		    cE = (cr->crossing_direction == BELOW_TO_ABOVE) ?
				negative_component(cr->hs) :
				positive_component(cr->hs);
		}
		else
		    cE = cW;

		comp[ix] = set_comp2d(cS,cE,cN,cW,ext_comp);
		if (comp[ix] == NO_COMP)
		{
		    return inconsistent_cross2d(ix,0,xmax,ymax,cS,cE,cN,cW,
			        ext_comp,ncs,nce,ncn,ncw,s_list,
			        e_list,n_list,w_list,grid_intfc,intfc);
		}
	    }
	}

	/* Set components on upper boundary */

	for (ix0 = 0; ix0 <= xmax; ++ix0)
	{
	    k_s = 2*ymax*xmax + ymax + ix0 - xmax - 1;
	    ncs = seg_crx_count[k_s];

	    if (ix0 < xmax)
	    {
	    	k_e = k_s + xmax + 1;
	    	nce    = seg_crx_count[k_e];
	    }
	    else
	    {
	    	k_e = -1;	nce = 0;
	    }
	    if (ncs > 0 || nce > 0)
		break;
	}

	if (ix0 > xmax)
	{
	    /*
	     *  There are no crossings near the upper boundary
	     *  get nearby component from original interface
	     */

	    coords[0] = grid_center_coord(0,gr);
	    coords[1] = gr->U[1];
	    cS = component(coords,intfc);
	    for (ix = 0; ix <= xmax; ++ix)
	    {
	    	comp[ymax*xmaxx + ix] = cS;
	    }
	}
	else
	{
	    /* Set components up to starting point found above */

	    cS = cE = cN = cW = ext_comp;
	    k_n = -1;	ncn = 0;	n_list = 0;
	    if (ncs > 0)
	    {
	    	s_list = seg_crx_lists[k_s];
	    	cr = crx_store + *(s_list+ncs-1);
	    	cS = (cr->crossing_direction == LEFT_TO_RIGHT) ?
	    		negative_component(cr->hs) :
	    		positive_component(cr->hs);
	    }

	    if (nce > 0)
	    {
	    	e_list = seg_crx_lists[k_e];
	    	cr = crx_store + *e_list;
	    	cE = (cr->crossing_direction == BELOW_TO_ABOVE) ?
	    		negative_component(cr->hs) :
	    		positive_component(cr->hs);
	    }

	    comp[ymax*xmaxx] = set_comp2d(cS,cE,cN,cW,ext_comp);
	    if (comp[ymax*xmaxx] == NO_COMP)
	    {
	        return inconsistent_cross2d(ix0,ymax,xmax,ymax,cS,cE,cN,cW,
					    ext_comp,0,nce,ncn,0,NULL,e_list,
					    n_list,NULL,grid_intfc,intfc);
	    }

	    for (ix = 1; ix <= ix0; ++ix)
	    	comp[ymax*xmaxx+ix] = comp[ymax*xmaxx];

	    /* Set remaining components on upper boundary */

	    k_n = -1;	ncn = 0;	n_list = NULL;
	    for (ix = 1+ix0; ix <= xmax; ++ix)
	    {
	    	k_w    = 2*ymax*xmax + ymax + ix - 1;
	    	ncw    = seg_crx_count[k_w];
	    	w_list = seg_crx_lists[k_w];
	
	    	k_s    = k_w - xmax;
	    	ncs    = seg_crx_count[k_s];
	    	s_list = seg_crx_lists[k_s];

	    	if (ix == xmax)
	    	{
	    	    k_e = -1;	nce = 0;	e_list = NULL;
	    	}
	    	else
	    	{
	    	    k_e    = k_w + 1;
	    	    nce    = seg_crx_count[k_e];
	    	    e_list = seg_crx_lists[k_e];
	    	}

		if (ncw)
		{
		    cr = crx_store + *(w_list+ncw-1);
		    cW = (cr->crossing_direction == BELOW_TO_ABOVE) ?
				positive_component(cr->hs) :
				negative_component(cr->hs);
		}
		else
		    cW = comp[ymax*xmaxx + ix - 1];

		if (ncs)
		{
		    cr = crx_store + *(s_list+ncs-1);
		    cS = (cr->crossing_direction == LEFT_TO_RIGHT) ?
				negative_component(cr->hs) :
				positive_component(cr->hs);
		}
		else
		    cS = cW;

		if (nce)
		{
		    cr = crx_store + *e_list;
		    cE = (cr->crossing_direction == BELOW_TO_ABOVE) ?
				negative_component(cr->hs) :
				positive_component(cr->hs);
		}
		else
		    cE = cW;

		comp[ymax*xmaxx+ix] = set_comp2d(cS,cE,cN,cW,ext_comp);
		if (comp[ymax*xmaxx+ix] == NO_COMP)
		{
		    return inconsistent_cross2d(ix,ymax,xmax,ymax,cS,cE,cN,cW,
			        ext_comp,ncs,nce,ncn,ncw,s_list,
			        e_list,n_list,w_list,grid_intfc,intfc);
		}
	    }
	}


	/* Set components on left boundary */

	cW = ext_comp;
	k_w = -1;	ncw = 0;	w_list = NULL;
	for (iy = 1; iy < ymax; ++iy)
	{
	    k_e    = 2*iy*xmax + iy;
	    nce    = seg_crx_count[k_e];
	    e_list = seg_crx_lists[k_e];

	    k_s    = k_e - xmax - 1;
	    ncs    = seg_crx_count[k_s];
	    s_list = seg_crx_lists[k_s];

	    k_n    = k_e + xmax;
	    ncn    = seg_crx_count[k_n];
	    n_list = seg_crx_lists[k_n];


	    if (ncs)
	    {
	    	cr = crx_store + *(s_list+ncs-1);
	    	cS = (cr->crossing_direction == LEFT_TO_RIGHT) ?
	    		negative_component(cr->hs) :
	    		positive_component(cr->hs);
	    }
	    else
	    	cS = comp[(iy-1)*xmaxx];

	    if (nce)
	    {
	    	cr = crx_store + *e_list;
	    	cE = (cr->crossing_direction == BELOW_TO_ABOVE) ?
	    		negative_component(cr->hs) :
	    		positive_component(cr->hs);
	    }
	    else
	    	cE = cS;

	    if (ncn)
	    {
	    	cr = crx_store + *n_list;
	    	cN = (cr->crossing_direction == LEFT_TO_RIGHT) ?
	    		positive_component(cr->hs) :
	    		negative_component(cr->hs);
	    }
	    else
	    	cN = cS;

	    comp[xmaxx*iy] = set_comp2d(cS,cE,cN,cW,ext_comp);
	    if (comp[xmaxx*iy] == NO_COMP)
	    {
	    	return inconsistent_cross2d(0,iy,xmax,ymax,cS,cE,cN,cW,ext_comp,
					    ncs,nce,ncn,ncw,s_list,e_list,
					    n_list,w_list,grid_intfc,intfc);
	    }
	}


	/* Set components on right boundary */

	cE = ext_comp;
	k_e = -1;	nce = 0;	e_list = NULL;
	for (iy = 1; iy < ymax; ++iy)
	{
	    k_s    = 2*iy*xmax + iy - 1;
	    ncs    = seg_crx_count[k_s];
	    s_list = seg_crx_lists[k_s];

	    k_w    = k_s + xmax;
	    ncw    = seg_crx_count[k_w];
	    w_list = seg_crx_lists[k_w];

	    k_n    = k_w + 1 + xmax;
	    ncn    = seg_crx_count[k_n];
	    n_list = seg_crx_lists[k_n];

	    if (ncs)
	    {
	    	cr = crx_store+ *(s_list+ncs-1);
	    	cS = (cr->crossing_direction == LEFT_TO_RIGHT) ?
	    		negative_component(cr->hs) :
	    		positive_component(cr->hs);
	    }
	    else
	    	cS = comp[(iy-1)*xmaxx + xmax];

	    if (ncw)
	    {
	    	cr = crx_store + *(w_list+ncw-1);
	    	cW = (cr->crossing_direction == BELOW_TO_ABOVE) ?
	    		positive_component(cr->hs) :
	    		negative_component(cr->hs);
	    }
	    else
	    	cW = cS;

	    if (ncn)
	    {
	    	cr = crx_store + *n_list;
	    	cN = (cr->crossing_direction == LEFT_TO_RIGHT) ?
	    		positive_component(cr->hs) :
	    		negative_component(cr->hs);
	    }
	    else
	    	cN = cS;

	    comp[iy*xmaxx+xmax] = set_comp2d(cS,cE,cN,cW,ext_comp);
	    if (comp[iy*xmaxx+xmax] == NO_COMP)
	    {
	    	return inconsistent_cross2d(xmax,iy,xmax,ymax,cS,cE,cN,cW,
			    ext_comp,ncs,nce,ncn,ncw,s_list,
			    e_list,n_list,w_list,grid_intfc,intfc);
	    }
	}


        /*   components inside computational region   */

	for (iy = 1;  iy < ymax;  ++iy)
	{
	    for (ix = 1;  ix < xmax;  ++ix)
	    {
	    	k_e    = 2*iy*xmax + iy + ix;
	    	nce    = seg_crx_count[k_e];
	    	e_list = seg_crx_lists[k_e];

	    	k_w    = k_e - 1;
	    	ncw    = seg_crx_count[k_w];
	    	w_list = seg_crx_lists[k_w];

	    	k_s    = k_w - xmax;
	    	ncs    = seg_crx_count[k_s];
	    	s_list = seg_crx_lists[k_s];

	    	k_n    = k_e + xmax;
	    	ncn    = seg_crx_count[k_n];
	    	n_list = seg_crx_lists[k_n];

	    	cS = cE = cN = cW = ext_comp;
	    	if (ncw)
	    	{
	    	    cr = crx_store + *(w_list+ncw-1);
	    	    cW = (cr->crossing_direction == BELOW_TO_ABOVE) ?
	    			positive_component(cr->hs) :
	    			negative_component(cr->hs);
	    	}
	    	else
	    	    cW = comp[iy*xmaxx+ix-1];

	    	if (ncs)
	    	{
	    	    cr = crx_store + *(s_list+ncs-1);
	    	    cS = (cr->crossing_direction == LEFT_TO_RIGHT) ?
	    			negative_component(cr->hs) :
	    			positive_component(cr->hs);
	    	}
	    	else
	    	    cS = comp[(iy-1)*xmaxx+ix];

	    	if (nce)
	    	{
	    	    cr = crx_store + *e_list;
	    	    cE = (cr->crossing_direction == BELOW_TO_ABOVE) ?
	    			negative_component(cr->hs) :
	    			positive_component(cr->hs);
	    	}

	    	if (ncn)
	    	{
	    	    cr = crx_store + *n_list;
	    	    cN = (cr->crossing_direction == LEFT_TO_RIGHT) ?
	    			positive_component(cr->hs) :
	    			negative_component(cr->hs);
	    	}

	    	comp[iy*xmaxx+ix] = set_comp2d(cS,cE,cN,cW,ext_comp);
	    	if (comp[iy*xmaxx+ix] == NO_COMP)
		{
	    	    return inconsistent_cross2d(ix,iy,xmax,ymax,cS,cE,cN,cW,
			        ext_comp,ncs,nce,ncn,ncw,s_list,
			        e_list,n_list,w_list,grid_intfc,intfc);
		}
	    }

	}

	debug_print("set_components",
		"Leaving set_components2d(), status = GOOD_STEP\n");
	return GOOD_STEP;
}		/*end set_grid_intfc_components2d*/

LOCAL	int set_grid_intfc_components3d(
	INTERFACE	*grid_intfc,
	INTERFACE	*intfc)
{
	Table		*T = table_of_interface(grid_intfc);
	RECT_GRID	*gr = &topological_grid(grid_intfc);
	COMPONENT	*comp = T->components;
	COMPONENT	***coz, **cozy, *cozyx, c;
	boolean		status;
	int             xmax, ymax, zmax;
	int		*gmax = gr->gmax;
	int		ix, iy, iz, i, n_reg_node;
	int		ixx,iyy,izz;
	int		smin[3], smax[3];
	DEBUG_ENTER(set_grid_intfc_components3d)

	n_reg_node = (gmax[0]+1)*(gmax[1]+1)*(gmax[2]+1);

	for (i = 0; i < n_reg_node; ++i)
	    comp[i] = NO_COMP;

	xmax = gmax[0];   /*                   */
	ymax = gmax[1];   /* global to tri3d.c */
	zmax = gmax[2];   /*                   */

	for (i = 0; i < 3; ++i)
	{
	    smin[i] = 0;
	    smax[i] = gmax[i];
	}
	status = track_comp_through_crxings3d(smin,smax,gmax,
			grid_intfc,MULTIPLE);
	
	if (status == FUNCTION_FAILED)
	{
	    screen("ERROR in set_grid_intfc_components3d(), "
	           "track_comp_through_crxings3d() failed!\n");
	    print_interface(intfc);
	    clean_up(ERROR);
	}
	DEBUG_LEAVE(set_grid_intfc_components3d)
	return GOOD_STEP;
}		/*end set_grid_intfc_components3d*/

LOCAL	int	set_untracked_components(
	INTERFACE	*grid_intfc,
	INTERFACE	*intfc)
{
	Table		*T = table_of_interface(grid_intfc);
	RECT_GRID	*gr = &topological_grid(grid_intfc);
	RECT_GRID	*c_gr;
	COMPONENT	ext_comp = exterior_component(grid_intfc);
	COMPONENT	icomp;
	COMPONENT	*comp;
	COMPONENT	*comps = T->components;
	double		coords[MAXD];
	int		xmax = 0, ymax = 0, zmax = 0;
	int		xmaxx = 0, ymaxx = 0;
	int		i, j, k, dim = gr->dim;
	int		n_reg_nodes;
	int		*gmax = gr->gmax;

	n_reg_nodes = gmax[0] + 1;
	for (i = 1; i < dim; ++i)
	     n_reg_nodes *= (gmax[i] + 1);
	c_gr = computational_grid(intfc);
	for (i = 0; i < dim; ++i)
	    coords[i] = grid_center_coord(i,c_gr);
	icomp = component(coords,intfc);
	comp = comps;
	for (i = 0; i < n_reg_nodes; ++i)
	    *comp++ = icomp;

	switch (dim)
	{
	case 1:
	    xmax = gr->gmax[0];
	    break;
	case 2:
	    ymax = gr->gmax[1];
	    xmax = gr->gmax[0];	xmaxx = xmax+1;
	    break;
	case 3:
	    zmax = gr->gmax[2];
	    ymax = gr->gmax[1];	ymaxx = ymax+1;
	    xmax = gr->gmax[0];	xmaxx = xmax+1;
	    break;
	}

	if (!buffered_boundary_type(rect_boundary_type(intfc,0,0)))
	{
	    for (k = 0; k <= zmax; ++k)
	    {
	    	comp = comps + k*xmaxx*ymaxx;
	    	for (j = 0; j <= ymax; ++j, comp += xmaxx)
	    	    *comp = ext_comp;
	    }
	}
	if (!buffered_boundary_type(rect_boundary_type(intfc,0,1)))
	{
	    for (k = 0; k <= zmax; ++k)
	    {
	    	comp = comps + xmax + k*xmaxx*ymaxx;
	    	for (j = 0; j <= ymax; ++j, comp += xmaxx)
		    *comp = ext_comp;
	    }
	}

	if (dim == 1)
	    return GOOD_STEP;

	if (!buffered_boundary_type(rect_boundary_type(intfc,1,0)))
	{
	    for (k = 0; k <= zmax; ++k)
	    {
	    	comp = comps + k*xmaxx*ymaxx;
	    	for (i = 0; i <= xmax; ++i, comp += 1)
	    	    *comp = ext_comp;
	    }
	}
	if (!buffered_boundary_type(rect_boundary_type(intfc,1,1)))
	{
	    for (k = 0; k <= zmax; ++k)
	    {
	    	comp = comps + ymax*xmaxx + k*xmaxx*ymaxx;
	    	for (i = 0; i <= xmax; ++i, comp += 1)
	    	    *comp = ext_comp;
	    }
	}

	if (dim == 2)
	    return GOOD_STEP;

	if (!buffered_boundary_type(rect_boundary_type(intfc,2,0)))
	{
	    comp = comps;
	    for (j = 0; j <= ymax; ++j)
	    	for (i = 0; i <= xmax; ++i, comp += 1)
	    	    *comp = ext_comp;
	}
	if (!buffered_boundary_type(rect_boundary_type(intfc,2,1)))
	{
	    comp = comps + zmax*xmaxx*ymaxx;
	    for (j = 0; j <= ymax; ++j)
	    	for (i = 0; i <= xmax; ++i, comp += 1)
	    	    *comp = ext_comp;
	}

	return GOOD_STEP;
}		/*end set_untrack_components*/

LOCAL	COMPONENT set_comp2d(
	COMPONENT	cS,
	COMPONENT	cE,
	COMPONENT	cN,
	COMPONENT	cW,
	COMPONENT	ext_comp)
{
	register COMPONENT c;

	c = ext_comp;
	if (cS != ext_comp)
	    c = cS;
	if (cE != ext_comp)
	{
	    if (c == ext_comp)
		c = cE;
	    else if (c != cE)
		return NO_COMP;
	}
	if (cN != ext_comp)
	{
	    if (c == ext_comp)
		c = cN;
	    else if (c != cN)
		return NO_COMP;
	}
	if (cW != ext_comp)
	{
	    if (c == ext_comp)
		c = cW;
	    else if (c != cW)
		return NO_COMP;
	}

	return c;
}		/*end set_comp2d*/


LOCAL	int inconsistent_cross2d(
	int		ix,
	int		iy,
	int		xmax,
	int		ymax,
	COMPONENT	cS,
	COMPONENT	cE,
	COMPONENT	cN,
	COMPONENT	cW,
	COMPONENT	ext_comp,
	int		ncs,
	int		nce,
	int		ncn,
	int		ncw,
	int		*s_list,
	int		*e_list,
	int		*n_list,
	int		*w_list,
	INTERFACE	*grid_intfc,
	INTERFACE	*intfc)
{
	RECT_GRID	*gr = &topological_grid(grid_intfc);
	(void) printf("WARNING in inconsistent_cross2d(), "
	              "inconsistent crossings, reducing time step\n");
	(void) printf("node: (%d %d), posn (%g, %g)\n",
			  ix,iy,cell_edge(ix,0,gr),cell_edge(iy,1,gr));
	(void) printf("xmax %d ymax %d\n",xmax,ymax);
	(void) printf("cS %d cE %d cN %d cW %d\n",cS,cE,cN,cW);
	(void) printf("ext_comp %d\n",ext_comp);

	print_side_crx_list(grid_intfc,'S',ncs,s_list);
	print_side_crx_list(grid_intfc,'E',nce,e_list);
	print_side_crx_list(grid_intfc,'N',ncn,n_list);
	print_side_crx_list(grid_intfc,'W',ncw,w_list);

	if (debugging("xg_inconsist"))
	{
	    char open_name[100];
	    static int count = 0;
	    double center[3];
	    sprintf(open_name,"inconsist-%d-%d",pp_mynode(),count++);
	    center[0] = cell_edge(ix,0,gr);
	    center[1] = cell_edge(iy,1,gr);
	    (void) printf("Inconsistent grid: %f %f\n",center[0],center[1]);
	    xgraph_2d_intfc_within_range(open_name,intfc,center,5*gr->h[0],YES);
	    printf("%f %f\n",center[0]-gr->h[0],center[1]);
	    printf("%f %f\n",center[0],center[1]);
	    printf("%f %f\n",center[0]+gr->h[0],center[1]);
	    printf("\n");
	    printf("%f %f\n",center[0],center[1]-gr->h[1]);
	    printf("%f %f\n",center[0],center[1]);
	    printf("%f %f\n",center[0],center[1]+gr->h[1]);
	}
	if (debugging("set_components"))
	{
	    (void) printf("\tinput interface\n");
	    print_interface(intfc);
	    plot_interface(intfc,"Input_interface",NULL,NULL,"Input interface");
	    (void) printf("\tgrid interface\n");
	    print_interface(grid_intfc);
	    plot_interface(grid_intfc,"Grid_interface",NULL,NULL,
			   "Grid interface");
	}
	debug_print("set_components",
	      "Left set_components(), status = MODIFY_TIME_STEP\n");
	return MODIFY_TIME_STEP;
}		/*end inconsistent_cross2d*/

EXPORT	int insert_grid_intfc_crossings(
	INTERFACE	*grid_intfc)
{
	int dim = computational_grid(grid_intfc)->dim;
	switch (dim)
	{
        case 1:
            return insert_grid_intfc_crossings1d(grid_intfc);
	case 2:
	    return insert_grid_intfc_crossings2d(grid_intfc);
	case 3:
	    return insert_grid_intfc_crossings3d(grid_intfc);
	}
}	/* end insert_grid_intfc_crossings */

EXPORT	int count_grid_intfc_crossings(
	INTERFACE	*grid_intfc)
{
	int dim = Dimension(grid_intfc);
	switch (dim)
	{
        case 1:
            return count_grid_intfc_crossings1d(grid_intfc);
	case 2:
	    return count_grid_intfc_crossings2d(grid_intfc);
	case 3:
	    return count_grid_intfc_crossings3d(grid_intfc);
	}
}	/* end insert_grid_intfc_crossings */

EXPORT	void free_grid_intfc(
	INTERFACE	*grid_intfc)
{
	int dim = Dimension(grid_intfc);
	free_crx_storage(grid_intfc);
	if (dim <= 2)
	    free_grid_lines(&topological_grid(grid_intfc));
	delete_interface(grid_intfc);
}	/* end free_grid_intfc */

EXPORT	INTERFACE *make_grid_intfc(
	INTERFACE	*intfc,
	GRID_TYPE	gr_type,
	VOLUME_FRAC	*volume_frac)
{
	int 	  i, smin[3], smax[3];
	RECT_GRID Dual_grid, *comp_grid = computational_grid(intfc);
	INTERFACE *grid_intfc;
	int dim = comp_grid->dim;

	if (debugging("make_grid_intfc"))
	    (void) printf("Entering make_grid_intfc()\n");

	start_clock("make_grid_intfc");
	set_size_of_intfc_state(size_of_state(intfc));
	set_copy_intfc_states(YES);
	interpolate_intfc_states(intfc) = YES;
	grid_intfc = pp_copy_interface(intfc);
	table_of_interface(grid_intfc)->fixed_grid = YES;

	switch(gr_type)
	{
	case COMP_GRID:
	    set_topological_grid(grid_intfc,comp_grid);
	    break;
	case DUAL_GRID:
	    set_dual_grid(&Dual_grid,comp_grid);
	    set_topological_grid(grid_intfc,&Dual_grid);
	    break;
	case EXPANDED_DUAL_GRID:
	    set_dual_grid(&Dual_grid,comp_grid);
	    set_expanded_grid(&Dual_grid,&topological_grid(grid_intfc));
	    break;
	default:
	    screen("In make_grid_intfc(), unknown grid type\n");
	    stop_clock("make_grid_intfc");
	    return NULL;
	}
	/* 
	   add this function to ensure interface topology 
	   does not break down due to tolerance 
	*/
	/*adjust_grid_intfc_points(grid_intfc); */

	if (debugging("make_grid_intfc"))
	{
	    (void) printf("Grid to be based:\n");
	    (void) print_rectangular_grid(&topological_grid(grid_intfc));
	}
	make_interface_topology_lists(grid_intfc);
	set_crx_storage_for_reconstruction(grid_intfc,volume_frac);

	for (i = 0; i < 3; ++i)
	{
	    smin[i] = 0;
            smax[i] = topological_grid(grid_intfc).gmax[i];
	}
	
	start_clock("insert_grid_intfc_crossings");
	interpolate_intfc_states(grid_intfc) = YES;
	insert_grid_intfc_crossings(grid_intfc);
	stop_clock("insert_grid_intfc_crossings");
	start_clock("set_grid_intfc_components");
	set_grid_intfc_components(grid_intfc,intfc);
	stop_clock("set_grid_intfc_components");

	if (dim == 3 && volume_frac != NULL)
	{
	    if (debugging("make_grid_intfc"))
	    {
	    	(void) printf("Doing volume fraction calculation\n");
	    }
	    start_clock("reconstruct_intfc3d_in_box");
	    strip_subdomain_bdry_curves(grid_intfc);
	    if (!reconstruct_intfc3d_in_box(grid_intfc,smin,smax,YES,
	    			volume_frac))
	    {
	    	DEBUG_LEAVE(make_grid_intfc)
	        stop_clock("make_grid_intfc");
	    	return NULL;
	    }
	    install_subdomain_bdry_curves(grid_intfc);
	    stop_clock("reconstruct_intfc3d_in_box");
	}

	reset_intfc_num_points(grid_intfc);
	if (debugging("make_grid_intfc"))
	    (void) printf("Leaving make_grid_intfc()\n");
        stop_clock("make_grid_intfc");
	return grid_intfc;
}	/*end make_grid_intfc*/

LOCAL	void print_side_crx_list(
	INTERFACE	*grid_intfc,
	char		side,
	int		n,
	int		*n_list)
{
	int		i;

	Table *T = table_of_interface(grid_intfc);
	(void) printf("%c crx list: %d crossings\n",side,n);
	for (i = 0;  i < n;  ++i)
	    print_crxings(T->crx_store + n_list[i],NO);
}		/*end print_side_crx_list*/


	/* LOCAL Function Declarations */
LOCAL	int	add_crx_to_list2d(INTERFACE*,int*,int,CROSSING_DIRECTION,
				  FT_POINT*,CURVE*,int*);
LOCAL	int	find_index_of_node(NODE*,INTERFACE*);
LOCAL	int	grid_crossings_on_segment(INTERFACE*,double,double,double,double,
					  int,int,int,int,double,double,double,
					  double,CURVE*,int*,int*,RECT_GRID*,
					  int*,FT_POINT**);
LOCAL	int	shift_from_cell_edge(double*,int,RECT_GRID*,double);
LOCAL	void	count_crossings(INTERFACE*,double,double,double,double,int,int,int,
			 int,double,double,double,double,CURVE*,int*,RECT_GRID*);

/*ARGSUSED*/
LOCAL   int insert_grid_intfc_crossings1d(
        INTERFACE       *grid_intfc)
{
        RECT_GRID       *rgr = &topological_grid(grid_intfc);
        register FT_POINT  **p;
        int             ic[MAXD];
        int             status;
        int             n_crx;

        for (n_crx = 0, p = grid_intfc->points; p && *p; ++p)
        {
            if (!rect_in_which(Coords(*p),ic,rgr))
            {
                double x = Coords(*p)[0];
                double L = rgr->L[0], U = rgr->U[0], h = rgr->h[0];
                if (((L-h) <= x) && (x < L))
                {
                    ic[0] = 0;
                }
                else if ((U < x) && (x <= (U+h)))
                {
                    ic[0] = rgr->gmax[0]-1;
                }
                else
                {
                    screen("ERROR in insert_grid_intfc_crossings1d(), "
                           "rect_in_which() failed\n");
                    clean_up(ERROR);
                }
            }
            status = add_crx_to_list1d(grid_intfc,n_crx++,*p,ic[0]);

            if (status != GOOD_STEP)
                return status;
        }
        return GOOD_STEP;
}               /*end insert_grid_intfc_crossings1d*/

LOCAL	int insert_grid_intfc_crossings2d(
	INTERFACE	*grid_intfc)
{
	register CURVE	**c;
	register BOND	*b;
	Table		*T = table_of_interface(grid_intfc);
	RECT_GRID	*rgr = &topological_grid(grid_intfc);
	NODE		**node;
	int		n_crx;
	int		crx_num;
	int		i;
	int		First_cross_on_curve;
	int		status;
	double		XL = rgr->L[0];
	double		XU = rgr->U[0];
	double		hx = rgr->h[0];
	double		YL = rgr->L[1];
	double		YU = rgr->U[1];
	double		hy = rgr->h[1];
	int		xmax = rgr->gmax[0];
	int		ymax = rgr->gmax[1];
	int		ix1,ix2,iy1,iy2;
	double		max_lenx,max_leny;
	int   		n_new_intfc_points = 0;
	FT_POINT		*new_intfc_points[50];
	int		*gmax = rgr->gmax;
				/*POTENTIAL BUG, static array size*/

	max_lenx = IG_ONEMTOL*hx;		max_leny = IG_ONEMTOL*hy;
	n_crx = T->n_crx;

			/* record crossings */
	crx_num = -1;
	for (c = grid_intfc->curves; c && *c;  ++c)
	{
	    First_cross_on_curve = YES;
	    b = (*c)->first;
	    ix2 = cell_index(Coords(b->start)[0],0,rgr);
	    iy2 = cell_index(Coords(b->start)[1],1,rgr);
	    while (b != NULL)
	    {
	    	n_new_intfc_points = 0;
	    	ix1 = ix2;		iy1 = iy2;
	    	ix2 = cell_index(Coords(b->end)[0],0,rgr);
	    	iy2 = cell_index(Coords(b->end)[1],1,rgr);

	    	status = grid_crossings_on_segment(grid_intfc,
				Coords(b->start)[0],Coords(b->start)[1],
				Coords(b->end)[0],Coords(b->end)[1],
				ix1,iy1,ix2,iy2,
				fabs(Coords(b->end)[0] - Coords(b->start)[0]),
				fabs(Coords(b->end)[1] - Coords(b->start)[1]),
				max_lenx,max_leny,
				*c,&crx_num,&First_cross_on_curve,rgr,
				&n_new_intfc_points,new_intfc_points);

	    	if (status != GOOD_STEP) 
	    	{
	    	    return status;
	    	}

	    	for (i = 0;  i < n_new_intfc_points;  ++i)
	    	{
	    	    if (insert_point_in_bond(new_intfc_points[i],b,*c) !=
			FUNCTION_SUCCEEDED)
	            {
	                screen("ERROR in insert_grid_crossings2d(), "
		               "insert_point_in_bond() failed\n");
	                clean_up(ERROR);
	            }
	    	    b = b->next;
	    	}
	    	b = b->next;
	    }
	}

	if (crx_num > 0)
	    T->crx_store[crx_num].end = YES;

		/* identify mesh squares having required interior nodes */

	i = (xmax+1)*(ymax+1) + n_crx;

	if (crx_num != n_crx-1)
	{
	    (void) printf("WARNING in insert_grid_crossings2d(), "
		          "inconsistent cross count, crx_num %d, n_crx %d\n",
			  crx_num,n_crx);
	    return MODIFY_TIME_STEP;
	}

	return GOOD_STEP;
}		/*end insert_grid_crossings2d*/

#define north_cross_on_grid(iy,ymax) (0 <= (iy) && (iy) <  (ymax))
#define south_cross_on_grid(iy,ymax) (0 <  (iy) && (iy) <= (ymax))
#define east_cross_on_grid(ix,xmax)  (0 <= (ix) && (ix) <  (xmax))
#define west_cross_on_grid(ix,xmax)  (0 <  (ix) && (ix) <= (xmax))

/*
*	Counts total grid crossings found. For use in initializing storage.
*/

LOCAL	void count_crossings(
	INTERFACE	*grid_intfc,
	double		x1,
	double		y1,
	double		x2,
	double		y2,
	int		ix1,
	int		iy1,
	int		ix2,
	int		iy2,
	double		lenx,
	double		leny,
	double		max_lenx,
	double		max_leny,
	CURVE		*curve,
	int		*n_crx,
	RECT_GRID	*rgr)
{
	register int	ix, iy;

	double		p3[MAXD], x_h, y_h, x_v, y_v;
	double		tolx, toly, wtolx, wtoly;
	int		msk, signf, signx;
	Table		*T = table_of_interface(grid_intfc);
	double		hx = rgr->h[0];
	double		hy = rgr->h[1];
	int		xmax = rgr->gmax[0];
	int		ymax = rgr->gmax[1];
	double		*xx_grid,*yy_grid;
	int 		*gmax = rgr->gmax;

	xx_grid = rgr->edges[0];	yy_grid = rgr->edges[1];

		/* if bond is too long, recursively cut off */
		/* hunks with lenx > max_lenx or leny > max_leny */

	tolx = IG_TOL * hx;	toly = IG_TOL * hy;
	if ((lenx > max_lenx) || (leny > max_leny))
	{
	    int	save_ix2, save_iy2;
	    double	s0, s1;

	    s0 = 1.0;
	    if (lenx > max_lenx)
		s0 = max_lenx/lenx;
	    if (leny > max_leny)
		s0 = min(s0,max_leny/leny);
	    s1 = 1.0 - s0;

	    p3[0] = x2 - s0 * (x2-x1);	p3[1] = y2 - s0 * (y2-y1);
	    save_ix2 = ix2;			save_iy2 = iy2;

	    ix2 = shift_from_cell_edge(p3,0,rgr,tolx);
	    iy2 = shift_from_cell_edge(p3,1,rgr,toly);

	    count_crossings(grid_intfc,x1,y1,p3[0],p3[1],ix1,iy1,ix2,iy2,
	    		s1*lenx,s1*leny,max_lenx,max_leny,curve,n_crx,rgr);
	    ix1 = ix2; 		iy1 = iy2;
	    ix2 = save_ix2; 	iy2 = save_iy2;
	    x1  = p3[0];		y1  = p3[1];
	}


	if ((iy1 != iy2) && (ix1 != ix2))    /* crossings of both vertical */
	{				      /* and horizontal grid bonds */

	    iy = max(iy1,iy2);	ix = max(ix1,ix2);
	    y_h = yy_grid[iy];
	    x_h = x1 + (x2 - x1) * (y_h - y1) / (y2 - y1);
	    x_v = xx_grid[ix];
	    y_v = y1 + (y2 - y1) * (x_v - x1) / (x2 - x1);

	    	/* test if too close to grid crossings */

	    wtolx = x_h - x_v;		wtoly = y_v - y_h;

	    signf = ((ix1-ix2)*(iy1-iy2) > 0) ? 1 : -1;
	    signx = 0;
	    if (fabs(wtolx) < tolx)
	    {
	    	if (wtolx >= 0)
	    	{
	    	    x_h += tolx;
	    	    signx =  1;
	    	}
	    	else
	    	{
	    	    x_h -= tolx;
	    	    signx = -1;
	    	}
	    }
	    if (fabs(wtoly) < toly)
	    {
	    	if (signx != 0)
	    	    y_v -= signx * signf * toly;
		else
		{
		    if (wtoly >= 0)
			y_v += toly;
		    else
			y_v -= toly;
		}	
	    }

	    if (x_h <= x_v)
	    {
	    	if (west_cross_on_grid(ix,xmax))
	    	{
		    msk = seg_index2d(ix,iy,WEST,gmax);
		    if (msk >= 0)
		    {
	    	    	++(*n_crx);
	    	    	++T->seg_crx_count[msk];
		    }
	    	}
	    	if ((ix2 - ix1)*(iy2 - iy1) > 0)
	    	{
	    	    if (north_cross_on_grid(iy,ymax))
	    	    {
			msk = seg_index2d(ix,iy,NORTH,gmax);
			if (msk >= 0)
			{
	    	    	    ++(*n_crx);
	    	    	    ++T->seg_crx_count[msk];
			}
	    	    }
	    	}
	    	else
	    	{
	    	    if (south_cross_on_grid(iy,ymax))
	    	    {
			msk = seg_index2d(ix,iy,SOUTH,gmax);
			if (msk >= 0)
			{
	    	    	    ++(*n_crx);
	    	    	    ++T->seg_crx_count[msk];
			}
	    	    }
	    	}
	    }
	    else
	    {
	    	if (east_cross_on_grid(ix,xmax))
	    	{
		    msk = seg_index2d(ix,iy,EAST,gmax);
		    if (msk >= 0)
		    {
	    	        ++(*n_crx);
	    	    	++T->seg_crx_count[msk];
		    }
	    	}
	    	if ((ix2 - ix1)*(iy2 - iy1) > 0)
	    	{
	    	    if (south_cross_on_grid(iy,ymax))
	    	    {
		    	msk = seg_index2d(ix,iy,SOUTH,gmax);
			if (msk >= 0)
			{
	    	    	    ++(*n_crx);
	    	    	    ++T->seg_crx_count[msk];
			}
	    	    }
	    	}
	    	else
	    	{
	    	    if (north_cross_on_grid(iy,ymax))
	    	    {
			msk = seg_index2d(ix,iy,NORTH,gmax);
			if (msk >= 0)
			{
	    	    	    ++(*n_crx);
	    	    	    ++T->seg_crx_count[msk];
			}
	    	    }
	    	}
	    }
	}
	else if (iy1 != iy2 && east_cross_on_grid(ix1,xmax))
	{
		/* crossing of horizontal grid bond */

	    iy = max(iy1,iy2);
	    msk = seg_index2d(ix1,iy,EAST,gmax);
	    if (msk >= 0)
	    {
	    	++(*n_crx);
	    	++T->seg_crx_count[msk];
	    }

	}
	else if (ix1 != ix2 && north_cross_on_grid(iy1,ymax))
	{
	    /* crossing of vertical grid bond */

	    ix = max(ix1,ix2);
	    msk = seg_index2d(ix,iy1,NORTH,gmax);
	    if (msk >= 0)
	    {
	    	++(*n_crx);
	    	++T->seg_crx_count[msk];
	    }
	}
}		/*end count_crossings*/



/*
*	Recursively inserts the grid crossings for each bond of the intfc
*/

LOCAL	int grid_crossings_on_segment(
	INTERFACE	*grid_intfc,
	double		x1,
	double		y1,
	double		x2,
	double		y2,
	int		ix1,
	int		iy1,
	int		ix2,
	int		iy2,
	double		lenx,
	double		leny,
	double		max_lenx,
	double		max_leny,
	CURVE		*curve,
	int		*crx_num,
	int		*fcoc,
	RECT_GRID	*rgr,
	int		*n_new_intfc_points,
	FT_POINT		**new_intfc_points)
{
	register int	ix, iy;
	register FT_POINT	*p_h, *p_v;

	double	           p3[MAXD], x_h, y_h, x_v, y_v;
	double	           coords_h[MAXD], coords_v[MAXD];
	double	           tolx, toly, wtolx, wtoly;
	boolean            weight;
	CROSSING_DIRECTION dir_v, dir_h;
	int 	           signf, signx, msk, status;
	double		hx = rgr->h[0];
	double		hy = rgr->h[1];
	int		xmax = rgr->gmax[0];
	int		ymax = rgr->gmax[1];
	double		*xx_grid,*yy_grid;
	int		*gmax = rgr->gmax;

	xx_grid = rgr->edges[0];	yy_grid = rgr->edges[1];

		/* if bond is too long, recursively cut off */
		/* hunks with lenx > max_lenx or leny > max_leny */

	tolx = IG_TOL * hx;	toly = IG_TOL * hy;
	if ((lenx > max_lenx) || (leny > max_leny))
	{
	    int	save_ix2, save_iy2;
	    double	s0, s1;

	    s0 = 1.0;
	    if (lenx > max_lenx)
		s0 = max_lenx/lenx;
	    if (leny > max_leny)
		s0 = min(s0,max_leny/leny);
	    s1 = 1.0 - s0;

	    p3[0] = x2 - s0 * (x2-x1);	p3[1] = y2 - s0 * (y2-y1);
	    save_ix2 = ix2;			save_iy2 = iy2;

	    ix2 = shift_from_cell_edge(p3,0,rgr,tolx);
	    iy2 = shift_from_cell_edge(p3,1,rgr,toly);

	    status = grid_crossings_on_segment(grid_intfc,x1,y1,p3[0],p3[1],ix1,
	    				iy1,ix2,iy2,s1*lenx,s1*leny,max_lenx,
					max_leny,curve,crx_num,fcoc,rgr,
					n_new_intfc_points,new_intfc_points);

	    if (status != GOOD_STEP)
		return status;

	    ix1 = ix2;		iy1 = iy2;
	    ix2 = save_ix2;	iy2 = save_iy2;
	    x1  = p3[0];	y1  = p3[1];
	}


	if ((iy1 != iy2) && (ix1 != ix2))    /* crossings of both vertical */
	{				       /* and horizontal grid bonds */
	    iy = max(iy1,iy2);
	    y_h = yy_grid[iy];
	    x_h = x1 + (x2 - x1) * (y_h - y1) / (y2 - y1);

	    dir_h = (iy1 < iy2) ? BELOW_TO_ABOVE : ABOVE_TO_BELOW;

	    ix = max(ix1,ix2);
	    x_v = xx_grid[ix];
	    y_v = y1 + (y2 - y1) * (x_v - x1) / (x2 - x1);

	    dir_v = (ix1 < ix2) ? LEFT_TO_RIGHT : RIGHT_TO_LEFT;

	    	/* test if too close to grid crossings */

	    wtolx = x_h - x_v;		wtoly = y_v - y_h;

	    signf = ((ix1-ix2)*(iy1-iy2) > 0) ? 1 : -1;
	    signx =  0;

	    if (fabs(wtolx) < tolx)
	    {
	    	if (wtolx >= 0)
	    	{
	    	    x_h += tolx;
	    	    signx =  1;
	    	}
	    	else
	    	{
	    	    x_h -= tolx;
	    	    signx = -1;
	    	}
	    }
	    if (fabs(wtoly) < toly)
	    {
	    	if (signx != 0)
	    	    y_v -= signx * signf * toly;
	    	else
	    	{
	    	    if (wtoly >= 0)
			y_v += toly;
	    	    else
			y_v -= toly;
	    	}	
	    }

		/* NOTE: weight is a boolean    	   */
		/* NOTE: for the rest of this else{} loop  */
		/*	 x_h,y_h and x_v,y_v must be equiv */
		/*	 expressions to p_h and p_v	   */

	    weight = ((sqr(x_h-x1)+sqr(y_h-y1)) > (sqr(x_v-x1)+sqr(y_v-y1))) ?
		     YES : NO;
	    coords_h[0] = x_h;	coords_h[1] = y_h;
	    coords_v[0] = x_v;	coords_v[1] = y_v;

	    msk = 2*iy*xmax + iy + ix;
	    if (x_h <= x_v)
	    {
	        if ((ix2 - ix1)*(iy2 - iy1) > 0)
	        {
	            if (weight == YES)
	            {
	    	        if (north_cross_on_grid(iy,ymax))
	    	        {
	    		    p_v = Point(coords_v);
	    		    new_intfc_points[(*n_new_intfc_points)++] = p_v;
			    msk = seg_index2d(ix,iy,NORTH,gmax);
	    		    status = add_crx_to_list2d(grid_intfc,crx_num,
			    		msk,dir_v,p_v,curve,fcoc);

	    		    if (status != GOOD_STEP)
				return status;
	    	        }

	    	        if (west_cross_on_grid(ix,xmax))
	    	        {
	    		    p_h = Point(coords_h);
	    		    new_intfc_points[(*n_new_intfc_points)++] = p_h;
			    msk = seg_index2d(ix,iy,WEST,gmax);
	    		    status = add_crx_to_list2d(grid_intfc,crx_num,
			    		msk,dir_h,p_h,curve,fcoc);

	    		    if (status != GOOD_STEP)
				return status;
	    	        }
		    }
		    else
		    {
		        if (west_cross_on_grid(ix,xmax))
		        {
		    	    p_h = Point(coords_h);
		    	    new_intfc_points[(*n_new_intfc_points)++] = p_h;
			    msk = seg_index2d(ix,iy,WEST,gmax);
		    	    status = add_crx_to_list2d(grid_intfc,crx_num,
			    		msk,dir_h,p_h,curve,fcoc);

		            if (status != GOOD_STEP)
				return status;
		        }

		        if (north_cross_on_grid(iy,ymax))
			{
			    p_v = Point(coords_v);
			    new_intfc_points[(*n_new_intfc_points)++] = p_v;
			    msk = seg_index2d(ix,iy,NORTH,gmax);
			    status = add_crx_to_list2d(grid_intfc,crx_num,
			    		msk,dir_v,p_v,curve,fcoc);

			    if (status != GOOD_STEP) return status;
			}
		    }
		}
		else
	        {
		    if (weight == YES)
		    {
		        if (south_cross_on_grid(iy,ymax))
		        {
		    	    p_v = Point(coords_v);
		    	    new_intfc_points[(*n_new_intfc_points)++] = p_v;
			    msk = seg_index2d(ix,iy,SOUTH,gmax);
		    	    status = add_crx_to_list2d(grid_intfc,crx_num,
			    		msk,dir_v,p_v,curve,fcoc);

		    	    if (status != GOOD_STEP)
				return status;
		        }

			if (west_cross_on_grid(ix,xmax))
			{
			    p_h = Point(coords_h);
			    new_intfc_points[(*n_new_intfc_points)++] = p_h;
			    msk = seg_index2d(ix,iy,WEST,gmax);
			    status = add_crx_to_list2d(grid_intfc,crx_num,
			    		msk,dir_h,p_h,curve,fcoc);

			    if (status != GOOD_STEP) return status;
			}
		    }
		    else
		    {
		        if (west_cross_on_grid(ix,xmax))
		        {
			    p_h = Point(coords_h);
			    new_intfc_points[(*n_new_intfc_points)++] = p_h;
			    msk = seg_index2d(ix,iy,WEST,gmax);
			    status = add_crx_to_list2d(grid_intfc,crx_num,
			    		msk,dir_h,p_h,curve,fcoc);

			    if (status != GOOD_STEP)
				return status;
			}

			if (south_cross_on_grid(iy,ymax))
			{
			    p_v = Point(coords_v);
			    new_intfc_points[(*n_new_intfc_points)++] = p_v;
			    msk = seg_index2d(ix,iy,SOUTH,gmax);
			    status = add_crx_to_list2d(grid_intfc,crx_num,
			    		msk,dir_v,p_v,curve,fcoc);

			    if (status != GOOD_STEP)
				return status;
			}
		    }
		}
	    }
	    else
	    {
	        if ((ix2 - ix1)*(iy2 - iy1) > 0)
	        {
	    	    if (weight == YES)
	    	    {
	    	        if (south_cross_on_grid(iy,ymax))
	    	        {
	    		    p_v = Point(coords_v);
	    		    new_intfc_points[(*n_new_intfc_points)++] = p_v;
			    msk = seg_index2d(ix,iy,SOUTH,gmax);
	    		    status = add_crx_to_list2d(grid_intfc,crx_num,
			    		msk,dir_v,p_v,curve,fcoc);

	    		    if (status != GOOD_STEP)
				return status;
	    	        }

	    	        if (east_cross_on_grid(ix,xmax))
	    	        {
	    		    p_h = Point(coords_h);
	    		    new_intfc_points[(*n_new_intfc_points)++] = p_h;
			    msk = seg_index2d(ix,iy,EAST,gmax);
	    		    status = add_crx_to_list2d(grid_intfc,crx_num,msk,
			    		dir_h,p_h,curve,fcoc);

			    if (status != GOOD_STEP)
				return status;
			}
		    }
		    else
		    {
		        if (east_cross_on_grid(ix,xmax))
		        {
			    p_h = Point(coords_h);
			    new_intfc_points[(*n_new_intfc_points)++] = p_h;
			    msk = seg_index2d(ix,iy,EAST,gmax);
			    status = add_crx_to_list2d(grid_intfc,crx_num,msk,
			    		dir_h,p_h,curve,fcoc);

			    if (status != GOOD_STEP)
				return status;
			}

			if (south_cross_on_grid(iy,ymax))
			{
			    p_v = Point(coords_v);
			    new_intfc_points[(*n_new_intfc_points)++] = p_v;
			    msk = seg_index2d(ix,iy,SOUTH,gmax);
			    status = add_crx_to_list2d(grid_intfc,crx_num,
			    		msk,dir_v,p_v,curve,fcoc);

			    if (status != GOOD_STEP)
				return status;
			}
		    }
		}
		else
		{
		    if (weight == YES)
		    {
		        if (north_cross_on_grid(iy,ymax))
		        {
			    p_v = Point(coords_v);
			    new_intfc_points[(*n_new_intfc_points)++] = p_v;
			    msk = seg_index2d(ix,iy,NORTH,gmax);
			    status = add_crx_to_list2d(grid_intfc,crx_num,
			    		msk,dir_v,p_v,curve,fcoc);

			    if (status != GOOD_STEP)
				return status;
		        }

			if (east_cross_on_grid(ix,xmax))
			{
			    p_h = Point(coords_h);
			    new_intfc_points[(*n_new_intfc_points)++] = p_h;
			    msk = seg_index2d(ix,iy,EAST,gmax);
			    status = add_crx_to_list2d(grid_intfc,crx_num,msk,
			    		dir_h,p_h,curve,fcoc);

			    if (status != GOOD_STEP)
				return status;
			}
		    }
		    else
		    {
		        if (east_cross_on_grid(ix,xmax))
		        {
		    	    p_h = Point(coords_h);
			    new_intfc_points[(*n_new_intfc_points)++] = p_h;
			    msk = seg_index2d(ix,iy,EAST,gmax);
			    status = add_crx_to_list2d(grid_intfc,crx_num,msk,
			    		dir_h,p_h,curve,fcoc);

			    if (status != GOOD_STEP)
				return status;
			}

			if (north_cross_on_grid(iy,ymax))
			{
			    p_v = Point(coords_v);
			    new_intfc_points[(*n_new_intfc_points)++] = p_v;
			    msk = seg_index2d(ix,iy,NORTH,gmax);
			    status = add_crx_to_list2d(grid_intfc,crx_num,
			    		msk,dir_v,p_v,curve,fcoc);

			    if (status != GOOD_STEP)
				return status;
			}
		    }
	        }
	    }
	}
	else if (iy1 != iy2) 
	{
	    /* crossing of horizontal grid bond */

	    iy = max(iy1,iy2);
	    y_h = yy_grid[iy];	x_v = xx_grid[ix1];
	    x_h = x1 + (x2 - x1) * (y_h - y1) / (y2 - y1);

	    wtolx = x_h - x_v;
	    if (fabs(wtolx) < tolx)
	    {
	    	if (wtolx > 0.0)
		    x_h += tolx;
	    	else
		    x_h -= tolx;
	    }

	    if (east_cross_on_grid(ix1,xmax))
	    {
	    	dir_h = (iy1 < iy2) ? BELOW_TO_ABOVE : ABOVE_TO_BELOW;
	    	coords_h[0] = x_h;
		coords_h[1] = y_h;
	    	p_h = Point(coords_h);
	    	new_intfc_points[(*n_new_intfc_points)++] = p_h;
		msk = seg_index2d(ix1,iy,EAST,gmax);
	    	status = add_crx_to_list2d(grid_intfc,crx_num,msk,
				dir_h,p_h,curve,fcoc);

	    	if (status != GOOD_STEP)
		    return status;
	    }
	}
	else if (ix1 != ix2)
	{
	    /* crossing of vertical bond grid */

	    ix = max(ix1,ix2);
	    x_v = xx_grid[ix];	y_h = yy_grid[iy1];
	    y_v = y1 + (y2 - y1) * (x_v - x1) / (x2 - x1);

	    wtoly = y_v - y_h;
	    if (fabs(wtoly) < toly)
	    {
	    	if (wtoly < 0.0)
		    y_v -= toly;
	    	else
		    y_v += toly;
	    }

	    if (north_cross_on_grid(iy1,ymax))
	    {
	    	dir_v = (ix1 < ix2) ? LEFT_TO_RIGHT : RIGHT_TO_LEFT;

	    	coords_v[0] = x_v;	coords_v[1] = y_v;
	    	p_v = Point(coords_v);
	    	new_intfc_points[(*n_new_intfc_points)++] = p_v;
		msk = seg_index2d(ix,iy1,NORTH,gmax);
	    	status = add_crx_to_list2d(grid_intfc,crx_num,
				msk,dir_v,p_v,curve,fcoc);

	    	if (status != GOOD_STEP)
		    return status;
	    }
	}
	return GOOD_STEP;
}		/*end grid_crossings_on_segment*/


LOCAL	int add_crx_to_list2d(
	INTERFACE	   *grid_intfc,
	int		   *n_crx,
	int		   msk,
	CROSSING_DIRECTION dir,
	FT_POINT		   *p,
	CURVE		   *cur,
	int		   *First_cross_on_curve)
{
	register CRXING *cross;
	Table		*T = table_of_interface(grid_intfc);
	CRXING		*crx_store = T->crx_store;

	int		nx, i, j, k, hold, input;
	int		*start;
	RECT_GRID	*gr = &topological_grid(grid_intfc);
	int		xmax = gr->gmax[0];
	int		ymax = gr->gmax[1];

	if (msk < 0)
	    return GOOD_STEP;
	++(*n_crx);

	cross = &(crx_store[*n_crx]);
	cross->hs = Hyper_surf(cur);
	cross->pt = p;
	cross->crossing_direction = dir;
	cross->crx_num = *n_crx;

	if ((First_cross_on_curve != NULL) && (*First_cross_on_curve == YES))
	{
	    cross->end = YES;
	    if (*n_crx > 0)
	    {
	    	crx_store[*n_crx - 1].end = YES;
	    }
	    *First_cross_on_curve = NO;
	}
	else
	    cross->end = NO;

	nx = T->seg_crx_count[msk];
	if (nx == 0)
	{
	    (void) printf("WARNING in add_crx_to_list2d(), nx = 0  msk = %d",
	    					msk);
	    return MODIFY_TIME_STEP;
	}

	start = T->seg_crx_lists[msk];

	switch (dir)
	{
	case BELOW_TO_ABOVE:
	case ABOVE_TO_BELOW:
	    for (j = 0;  j < nx;  ++j)
	    {
	    	if (*(start+j) == -1) break;
	    }

	    if (j == 0)
	    	*start = *n_crx;
	    else
	    {
	    	for (i = 0;  i < j;  ++i)
	    	{
	    	    if (Coords((crx_store[*(start+i)]).pt)[0] >
						 Coords(cross->pt)[0])
			break;
		}

		if (i < j)
		{
		    input = *n_crx;
		    for (k = i;  k < j;  ++k)
		    {
		    	hold = *(start+k);
		    	*(start+k) = input;
		    	input = hold;
		    }
		    *(start+j) = input;
		}
		else
		    *(start+j) = *n_crx;
	    }
	    break;
	case LEFT_TO_RIGHT:
	case RIGHT_TO_LEFT:
	    for (j = 0;  j < nx;  ++j)
	    {
	    	if (*(start+j) == -1) break;
	    }

	    if (j == 0)
	    	*start = *n_crx;
	    else
	    {
	    	for (i = 0;  i < j;  ++i)
	    	{
	    	    if (Coords((crx_store[*(start+i)]).pt)[1] >
	    				 Coords(cross->pt)[1])
	    		    break;
	    	}

	    	if (i < j)
	    	{
	    	    input = *n_crx;
	    	    for (k = i;  k < j;  ++k)
	    	    {
	    	    	hold = *(start+k);
	    	    	*(start+k) = input;
	    	    	input = hold;
	    	    }
	    	    *(start+j) = input;
	    	}
	    	else
	    	    *(start+j) = *n_crx;
	    }
	    break;
	}
	return GOOD_STEP;
}		/*end add_crx_to_list2d*/


LOCAL	int	shift_from_cell_edge(
	double		*p,
	int		idir,
	RECT_GRID	*rgr,
	double		tol)
{
	int		ic;
	double		sfrac;

	ic = cell_index(p[idir],idir,rgr);
	sfrac = (p[idir] - rgr->edges[idir][ic]) / cell_width(ic,idir,rgr);
	if (ic == rgr->gmax[idir] && sfrac < IG_TOL)
	{
	    p[idir] -= tol;
	    ic = cell_index(p[idir],idir,rgr);
	}
	else if (sfrac < IG_TOL)     	p[idir] += tol;
	else if (sfrac > IG_ONEMTOL)	p[idir] -= tol;
	ic = cell_index(p[idir],idir,rgr);
	return ic;
}		/*end shift_from_cell_edge*/


LOCAL	int count_grid_intfc_crossings2d(
	INTERFACE	*grid_intfc)
{
	RECT_GRID	*rgr = &topological_grid(grid_intfc);
	register CURVE	**c;
	register BOND	*b;
	double		tolx, toly;
	int		n_index;
	int		n_crx = 0;
	double		hx = rgr->h[0];
	double		hy = rgr->h[1];
	int		ix1,ix2,iy1,iy2;
	double		max_lenx,max_leny;
	double		*xx_grid,*yy_grid;
	int		*gmax = rgr->gmax;
	Table           *T = table_of_interface(grid_intfc);

	max_lenx = IG_ONEMTOL*hx;		max_leny = IG_ONEMTOL*hy;
	xx_grid = rgr->edges[0];	yy_grid = rgr->edges[1];
	tolx = IG_TOL * hx;		toly = IG_TOL * hy;

			/* count intfc crossings of dual lattice */

	for (c = grid_intfc->curves; c && *c;  ++c)
	{
	    b = (*c)->first;
	    ix2 = shift_from_cell_edge(Coords(b->start),0,rgr,tolx);
	    iy2 = shift_from_cell_edge(Coords(b->start),1,rgr,toly);
	    while (b != NULL)
	    {
	    	ix1 = ix2;		iy1 = iy2;

	    	ix2 = shift_from_cell_edge(Coords(b->end),0,rgr,tolx);
		iy2 = shift_from_cell_edge(Coords(b->end),1,rgr,toly);

		count_crossings(grid_intfc,Coords(b->start)[0],
				Coords(b->start)[1],Coords(b->end)[0],
				Coords(b->end)[1],ix1,iy1,ix2,iy2,
				fabs(Coords(b->end)[0] - Coords(b->start)[0]),
				fabs(Coords(b->end)[1] - Coords(b->start)[1]),
				max_lenx,max_leny,*c,&n_crx,rgr);
		b = b->next;
	    }
	}
	T->n_crx = n_crx;
	return n_crx;
}		/*end count_grid_intfc_crossings2d*/

EXPORT	void show_grid_components(
	int *smin,
	int *smax,
	int idir,
	INTERFACE *intfc)
{
	int dim = Dimension(intfc);
	switch (dim)
	{
	case 2:
	    show_grid_components2d(smin,smax,idir,intfc);
	    return;
	case 3:
	    show_grid_components3d(smin,smax,idir,intfc);
	    return;
	}
}	/* end show_grid_components */


EXPORT	void show_line_components3d(
	int *smin,
	int *smax,
	int idir,
	INTERFACE *intfc)
{
	int 		ix,iy,iz,k,nc;
	Table		*T = table_of_interface(intfc);
	RECT_GRID	gr = topological_grid(intfc);
	COMPONENT       *comp = T->components;
	int 		*gmax = gr.gmax;

	if(pp_mynode() == 9)
	{
	    ix = 12;
	    iy = 7;
	}
	else
	{
	    ix = 12;
	    iy = 27;
	}

	for (iz = smin[2]; iz <= smax[2]; ++iz)
	{
	    printf("( %5d  ", iz);
	    if (iz != smax[2])
	    {
		k = seg_index3d(ix,iy,iz,UPPER,gmax);
		nc = T->seg_crx_count[k];
		if (nc == 0)
		    printf("%2d ",comp[d_index3d(ix,iy,iz,gmax)]);
		else if (nc == 1)
		    printf("%2d|",comp[d_index3d(ix,iy,iz,gmax)]);
		else
		    printf("%2d*",comp[d_index3d(ix,iy,iz,gmax)]);
	    }
	    else
		printf("%2d ",comp[d_index3d(ix,iy,iz,gmax)]);
	}
	printf("\n");
}

EXPORT	void show_the_grid_comp(
	const char *msg, 
	INTERFACE  *intfc)
{
	int  i, tmin[3], tmax[3];
	int  cen[3] = {5,8,3};

	for(i=0; i<3; i++)
	{
	    tmin[i] = cen[i] - 2;
	    tmax[i] = cen[i] + 2;
	}
	if(pp_mynode() == 9)
	{
	    tmin[2] += 40;
	    tmax[2] += 40;
	}

	printf("#show_the_grid_comp %s\n", msg);
	
	show_grid_components(tmin,tmax,0,intfc);
	show_grid_components(tmin,tmax,1,intfc);
	show_grid_components(tmin,tmax,2,intfc);
}

LOCAL	void show_grid_components3d(
	int *smin,
	int *smax,
	int idir,
	INTERFACE *intfc)
{
	int 		ix,iy,iz,k,nc;
	Table		*T = table_of_interface(intfc);
	RECT_GRID	gr = topological_grid(intfc);
	COMPONENT       *comp = T->components;
	int 		*gmax = gr.gmax;


	printf("#show_grid_components3d\n");
	print_general_vector("L", gr.L, 3, "\n");
	print_general_vector("h", gr.h, 3, "\n");
	print_int_vector("smin", smin, 3, "\n");
	print_int_vector("smax", smax, 3, "\n");
	
	if (idir == 0)
	{
	    for (ix = smin[0]; ix <= smax[0]; ++ix)
	    {
		printf("\n\t\tix = %d\n\n",ix);
	    	for (iy = smin[1]; iy <= smax[1]; ++iy)
		{
		    for (iz = smin[2]; iz <= smax[2]; ++iz)
		    {
			if (iz != smax[2])
			{
			    k = seg_index3d(ix,iy,iz,UPPER,gmax);
			    nc = T->seg_crx_count[k];
			    if (nc == 0)
		    	        printf("%2d ",comp[d_index3d(ix,iy,iz,gmax)]);
		    	    else if (nc == 1)
		    	        printf("%2d|",comp[d_index3d(ix,iy,iz,gmax)]);
			    else
		    	        printf("%2d*",comp[d_index3d(ix,iy,iz,gmax)]);
			}
			else
			    printf("%2d ",comp[d_index3d(ix,iy,iz,gmax)]);
		    }
		    printf("\n");
		    if(iy == smax[1])    
		    {
		        printf("\n");
		        continue;
		    }
		    for (iz = smin[2]; iz <= smax[2]; ++iz)
		    {
			k = seg_index3d(ix,iy,iz,NORTH,gmax);
			nc = T->seg_crx_count[k];
			if (nc == 0)
		    	    printf("   ");
		    	else if (nc == 1)
		    	    printf(" - ");
		    	else 
		    	    printf(" * ");
		    }
		    printf("\n");
		}
	    }
	}
	else if (idir == 1)
	{
	    for (iy = smin[1]; iy <= smax[1]; ++iy)
	    {
		printf("\n\t\tiy = %d\n\n",iy);
		for (iz = smin[2]; iz <= smax[2]; ++iz)
		{
	    	    for (ix = smin[0]; ix <= smax[0]; ++ix)
		    {
			if (ix != smax[0])
			{
			    k = seg_index3d(ix,iy,iz,EAST,gmax);
			    nc = T->seg_crx_count[k];
			    if (nc == 0)
		    	        printf("%2d ",comp[d_index3d(ix,iy,iz,gmax)]);
		    	    else if (nc == 1)
		    	        printf("%2d|",comp[d_index3d(ix,iy,iz,gmax)]);
		    	    else
		    	        printf("%2d*",comp[d_index3d(ix,iy,iz,gmax)]);
			}
			else
			    printf("%2d ",comp[d_index3d(ix,iy,iz,gmax)]);
		    }
		    printf("\n");
		    if(iz == smax[2])    
		    {
		        printf("\n");
		        continue;
		    }
	    	    for (ix = smin[0]; ix <= smax[0]; ++ix)
		    {
			k = seg_index3d(ix,iy,iz,UPPER,gmax);
			nc = T->seg_crx_count[k];
			if (nc == 0)
		    	    printf("   ");
		    	else if (nc == 1)
		    	    printf(" - ");
		    	else
		    	    printf(" * ");
		    }
		    printf("\n");
		}
	    }
	}
	else
	{
	    for (iz = smin[2]; iz <= smax[2]; ++iz)
	    {
		printf("\n\t\tiz = %d\n\n",iz);
	    	for (ix = smin[0]; ix <= smax[0]; ++ix)
		{
	    	    for (iy = smin[1]; iy <= smax[1]; ++iy)
		    {
			if (iy != smax[1])
			{
			    k = seg_index3d(ix,iy,iz,NORTH,gmax);
			    nc = T->seg_crx_count[k];
			    if (nc == 0)
		    	        printf("%2d ",comp[d_index3d(ix,iy,iz,gmax)]);
		    	    else if (nc == 1)
		    	        printf("%2d|",comp[d_index3d(ix,iy,iz,gmax)]);
		    	    else
		    	        printf("%2d*",comp[d_index3d(ix,iy,iz,gmax)]);
			}
			else
			    printf("%2d ",comp[d_index3d(ix,iy,iz,gmax)]);
		    }
		    printf("\n");
		    if(ix == smax[0])    
		    {
		        printf("\n");
		        continue;
		    }
	    	    for (iy = smin[1]; iy <= smax[1]; ++iy)
		    {
			k = seg_index3d(ix,iy,iz,EAST,gmax);
			nc = T->seg_crx_count[k];
			if (nc == 0)
		    	    printf("   ");
		    	else if (nc == 1)
		    	    printf(" - ");
			else
		    	    printf(" * ");
		    }
		    printf("\n");
		}
	    }
	}
}	/* end show_grid_components3d */


LOCAL	void show_grid_components2d(
	int *smin,
	int *smax,
	int idir,
	INTERFACE *intfc)
{
	int 		ix,iy,k,nc;
	Table		*T = table_of_interface(intfc);
	RECT_GRID	gr = topological_grid(intfc);
	COMPONENT       *comp = T->components;
	int 		*gmax = gr.gmax;

	for (iy = smax[1]; iy >= smin[1]; --iy)
	{
	    for (ix = smin[0]; ix <= smax[0]; ++ix)
	    {
		if (ix != smax[0])
		{
		    k = seg_index2d(ix,iy,EAST,gmax);
		    nc = T->seg_crx_count[k];
		    if (nc == 0)
	    	        printf("%1d ",comp[d_index2d(ix,iy,gmax)]);
	    	    else if (nc == 1)
	    	        printf("%1d|",comp[d_index2d(ix,iy,gmax)]);
	    	    else
	    	        printf("%1d*",comp[d_index2d(ix,iy,gmax)]);
		}
		else
		    printf("%1d ",comp[d_index2d(ix,iy,gmax)]);
	    }
	    printf("\n");
	    if (iy == smin[1]) continue;
	    for (ix = smin[0]; ix <= smax[0]; ++ix)
	    {
		k = seg_index2d(ix,iy,SOUTH,gmax);
		nc = T->seg_crx_count[k];
		if (nc == 0)
	    	    printf("  ");
	    	else if (nc == 1)
	    	    printf("- ");
		else
	    	    printf("* ");
	    }
	    printf("\n");
	}
}	/* end show_grid_components3d */

LOCAL   int count_grid_intfc_crossings1d(
        INTERFACE       *grid_intfc)
{
        RECT_GRID       *rgr = &topological_grid(grid_intfc);
        register FT_POINT  **p;
        int             ic[MAXD];
        Table           *T = table_of_interface(grid_intfc);
        int             n_crx = 0;

        for (p = grid_intfc->points; p && *p; ++p)
        {
            if (rect_in_which(Coords(*p),ic,rgr) == FUNCTION_FAILED)
            {
                double x = Coords(*p)[0];
                double L = rgr->L[0], U = rgr->U[0], h = rgr->h[0];
                if (((L-h) <= x) && (x < L))
                {
                    ic[0] = 0;
                }
                else if ((U < x) && (x <= (U+h)))
                {
                    ic[0] = rgr->gmax[0]-1;
                }
                else
                {
                    screen("ERROR in "
                           "count_grid_intfc_crossings1d(), "
                           "rect_in_which() failed\n");
                    clean_up(ERROR);
                }
            }
            n_crx++;
            ++T->seg_crx_count[ic[0]];
        }
        T->n_crx = n_crx;
        return n_crx;
}               /*end count_grid_intfc_crossings1d*/

LOCAL   int add_crx_to_list1d(
        INTERFACE       *grid_intfc,
        int             n_crx,
        FT_POINT           *p,
        int             ix)
{
        register CRXING *cross;
        Table           *T = table_of_interface(grid_intfc);
        CRXING          *crx_store = T->crx_store;

        int             nx, i, j, k;
        int             input, hold;
        int             *start;

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
                if (Coords((crx_store[*(start+i)]).pt)[0] >Coords(cross->pt)[0])                    break;
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
}               /*end add_crx_to_list1d*/

EXPORT void adjust_grid_intfc_points(INTERFACE *intfc)
{
	RECT_GRID *gr = &topological_grid(intfc);
	double *L = gr->L;
	double *h = gr->h;
	FT_POINT *p;
        HYPER_SURF *hs;
        HYPER_SURF_ELEMENT *hse;
	int icoords[MAXD];
	int i,sign,max_i,dim = gr->dim;
	double dist[MAXD],max_dist;
	boolean to_adjust;

	next_point(intfc,NULL,NULL,NULL);
        while (next_point(intfc,&p,&hse,&hs))
        {
	    if (wave_type(hs) < FIRST_PHYSICS_WAVE_TYPE)
		continue;
	    if (!rect_in_which(Coords(p),icoords,gr)) continue;
	    max_dist = 0.0;	max_i = -1;
	    to_adjust = YES;
	    for (i = 0; i < dim; ++i)
	    {
	    	if (fabs(Coords(p)[i] - L[i] - icoords[i]*h[i]) >
		    fabs(Coords(p)[i] - L[i] - (icoords[i]+1)*h[i]))
		    icoords[i]++;
		dist[i] = Coords(p)[i] - L[i] - icoords[i]*h[i];
		if (fabs(dist[i]) > 0.004*h[i])
		{
		    to_adjust = NO;
		    break;
		}
		if (max_dist < fabs(dist[i]))
		{
		    max_dist = fabs(dist[i]);
		    max_i = i;
		    sign = (dist[i] > 0.0) ? 1 : -1;
		}
	    }
	    if (to_adjust == NO) continue;
	    Coords(p)[max_i] = L[max_i] + icoords[max_i]*h[max_i] +
			sign*0.004*h[max_i];
	    intfc->modified = YES;
        }
}	/* end adjust_grid_intfc_points */
