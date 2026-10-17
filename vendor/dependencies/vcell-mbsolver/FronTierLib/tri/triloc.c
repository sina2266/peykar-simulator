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
*			triloc.c
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*/


#include <tri/trilocaldecs.h>

	/* LOCAL Function Prototypes */
LOCAL	void	tri_grid_index_from_comp_grid_index(int*,int*,TRI_GRID*);
#if defined(TWOD) || defined(THREED)
LOCAL	boolean build_linear_element(POINT_COMP_ST*,int,COMPONENT,double*,
				     LINEAR_ELEMENT**,TRI_GRID*);
LOCAL	boolean collect_least_sqr_cluster(POINT_COMP_ST*,int,COMPONENT,double*,
				     LEAST_SQR_CLUSTER**,TRI_GRID*);
#endif /* defined(TWOD) || defined(THREED) */

#if defined(DEBUG_TRI_LOC)
LOCAL	int	debug_tri_loc = NO;		/* debugging switch */
EXPORT void set_debug_tri_loc(int y_or_n)
{
	debug_tri_loc = y_or_n;
}		/*end set_debug_tri_loc*/
#endif /* defined(DEBUG_TRI_LOC) */


/*
*			crossings_in_direction():
*
*	Finds the crossings in the given direction on the grid cell edge 
*	through the point icoords in the direction dir.
*	The index icoords is assumed to be offset by one from the origin
*	of the rectangular grid ntg->tg_grid.  That is the point
*	icoords[i] = 0, i = 0,..,dim corresponds to the point ic[i] = 1,
*	i = 0,...dim, on the lattice ntg->tg_grid.  The reason for this
*	offset is that in the typical application of this this function,
*	icoords is the index of a rectangular cell on a lattice that is
*	dual to the lattice ntg->tg_grid.  It is assumed that the origin
*	of this dual lattice aligns with the cells center of the 0th cell
*	in the lattice ntg->tg_grid.
*/

/*ARGSUSED*/
EXPORT	int crossings_in_direction(
	CRXING		**crx_list,
	int		*icoords,
	GRID_DIRECTION	dir,
	TRI_GRID	*ntg)
{
	int		dim = ntg->tg_grid.dim;
	int		ic[MAXD];
	int		ncross = 0;

	tri_grid_index_from_comp_grid_index(ic,icoords,ntg);
	ncross = GridSegCrossing(crx_list,ic,dir,ntg->grid_intfc);
	return ncross;
}		/*end crossings_in_direction*/

EXPORT	boolean nearest_crossing_state_with_comp(
	int *icoords,
	double *coords,
	COMPONENT comp,
	TRI_GRID *grid,
	Locstate *s)
{
	GRID_DIRECTION dirs[6] = {WEST,EAST,SOUTH,NORTH,LOWER,UPPER};
	CRXING *crx,*crx_tmp;
	int i,k,nc,*list;
	int ic[MAXD];
	double distance,d_tmp;
	int dim = dim = grid->tg_grid.dim;
	int *gmax = grid->rect_grid.gmax;
	Table *T = table_of_interface(grid->grid_intfc);

	if (dim < 3)	/* Need to implement */
	    return NO;
#if defined(THREED)
        tri_grid_index_from_comp_grid_index(ic,icoords,grid);
	crx = NULL;
	distance = HUGE;
	for (i = 0; i < 6; ++i)
	{
	    if ((ic[0] == 0 && dirs[i] == WEST) ||
	        (ic[0] == gmax[0] && dirs[i] == EAST))
		continue;
	    if ((ic[1] == 0 && dirs[i] == SOUTH) ||
	        (ic[1] == gmax[1] && dirs[i] == NORTH))
		continue;
	    if ((ic[2] == 0 && dirs[i] == LOWER) ||
	        (ic[2] == gmax[2] && dirs[i] == UPPER))
		continue;
	    k = seg_index3d(ic[0],ic[1],ic[2],dirs[i],gmax);
	    nc = T->seg_crx_count[k];
	    list = T->seg_crx_lists[k];
	    if (nc == 0) continue;
	    switch (dirs[i])
	    {
	    case WEST:
	    case SOUTH:
	    case LOWER:
	    	crx_tmp = T->crx_store + list[nc-1];

	    	break;
	    case EAST:
	    case NORTH:
	    case UPPER:
	    	crx_tmp = T->crx_store + list[0];
	    }
	    if (crx_tmp->lcomp != comp && crx_tmp->ucomp != comp)
	    	continue;
	    d_tmp = fabs(coords[i/2] - Coords(crx_tmp->pt)[i/2]);
	    if (d_tmp < distance)
	    {
	    	crx = crx_tmp;
		distance = d_tmp;
	    }
	}
	if (crx == NULL) return NO;
	*s = (comp == negative_component(crx->hs)) ? left_state(crx->pt) : 
			right_state(crx->pt);
	return YES;
#endif /* defined(THREED) */
}	/* end nearest_crossing_state */

LOCAL	void tri_grid_index_from_comp_grid_index(
	int		*tgicrds,
	int		*icrds,
	TRI_GRID	*grid)
{
	RECT_GRID	*tg_grid = &grid->tg_grid;
	int		i, dim = tg_grid->dim;

	for (i = 0; i < dim; ++i)
		tgicrds[i] = icrds[i] + 1 + tg_grid->lbuf[i];
}		/*end tri_grid_index_from_comp_grid_index*/

#if defined(TWOD) || defined(THREED)
/*
*			tg_locate():
*
*	Finds the element lying in component comp in which the point
*	x,y lies.  If *qd != NULL, the point lies in the quadrangle
*	*qd; if *tri != NULL, the point lies in the triangle *tri;
*	otherwise the point does not lie in component comp.
*
*	Returns 1 if successful at finding the element, and 0 otherwise.
*
*	The number of cross products taken can be reduced by one if
*	all triangles are oriented the same way.
*/


LIB_LOCAL	boolean tg_locate(
	double			*coords,
	COMPONENT		comp,
	TRI_GRID		*grid,
	BILINEAR_ELEMENT	**bilin,
	LINEAR_ELEMENT		**lin,
	LEAST_SQR_CLUSTER	**lsq)
{
	BLK_EL0		*blk_el0;
	INTERFACE	*intfc = grid->grid_intfc;
	LINEAR_ELEMENT	*et;
	RECT_GRID	*gr = &grid->rect_grid;
	int		icoords[MAXD];
	int		dim = grid->rect_grid.dim;

	*bilin = NULL;	*lsq = NULL;		*lin = NULL;

	if (is_exterior_comp(comp,intfc) || (comp < min_component(intfc))
					 || (comp > max_component(intfc)))
	    return FUNCTION_FAILED;

	if (rect_in_which(coords,icoords,gr) == FUNCTION_FAILED)
	    return FUNCTION_FAILED;

	blk_el0 = &Blk_el0(icoords,grid);

	if (blk_el0_is_bilinear(blk_el0))
	{
	    *bilin = blk_el0_bilinear_el(blk_el0);
#if defined(DEBUG_TRI_LOC)
	    if (debug_tri_loc)
	    {
	    	(void) printf("x %g y %g comp %d in mesh block %d %d",
	    		      coords[0],coords[1],comp,icoords[0],icoords[1]);
	    	(void) printf("  quadrangle:\n");
	    	print_BILINEAR_ELEMENT(*bilin,grid);
	    }
#endif /* defined(DEBUG_TRI_LOC) */
	    if ((*bilin)->comp != comp)
	    {
	    	*bilin = NULL;
	    	return FUNCTION_FAILED;
	    }
	    return FUNCTION_SUCCEEDED;
	}
	else
	{
	    int	k, num_els = num_lin_els_in_blk(blk_el0);
	    switch (dim)
	    {
#if defined(TWOD)
	    case 2:
	        for (k = 0;  k < num_els;  ++k)
	        {
	            et = blk_el0_linear_els(blk_el0) + k;
		    if (point_in_triangle(coords,et,comp,grid))
		    {
			*lin = et;
#if defined(DEBUG_TRI_LOC)
			if (debug_tri_loc)
			{
			    (void) printf("x %g y %g comp %d ",
					  coords[0],coords[1],comp);
			    (void) printf("in mesh block %d %d",
					  icoords[0],icoords[1]);
			    (void) printf("  triangle:\n");
			    print_LINEAR_ELEMENT(et,grid);
		        }
#endif /* defined(DEBUG_TRI_LOC) */
			return FUNCTION_SUCCEEDED;
		    }
		}

		/* If reach here, lin not found */
		*lin = NULL;
		break;
#endif /* defined(TWOD) */
#if defined(THREED)
	    case 3:
		for (k = 0;  k < num_els;  ++k)
		{
		    et = blk_el0_linear_els(blk_el0) + k;
		    if (point_in_tetra(coords,et,comp,grid) == YES)
		    {
		        *lin = et;
#if defined(DEBUG_TRI_LOC)
		        if (debug_tri_loc)
		        {
		            (void) printf("x %g y %g z %g ",
					  coords[0],coords[1],coords[2]);
		            (void) printf("comp %d in block %d %d %d",comp,
					  icoords[0],icoords[1],icoords[2]);
		            (void) printf("  tetrahedral:\n");
		            print_LINEAR_ELEMENT(et,grid);
		        }
#endif /* defined(DEBUG_TRI_LOC) */
		        return FUNCTION_SUCCEEDED;
		    }
		}
		*lin = NULL;
		break;
#endif /* defined(THREED) */
	    }
	}
	if ((*lin == NULL) && (*bilin == NULL))
	    return FUNCTION_FAILED;
	return FUNCTION_SUCCEEDED;
}		/*end tg_locate*/

LIB_LOCAL	boolean tg_build(
	double			*coords,
	COMPONENT		comp,
	TRI_GRID		*grid,
	BILINEAR_ELEMENT	**bilin,
	LINEAR_ELEMENT		**lin,
	LEAST_SQR_CLUSTER	**lsq)
{
	BLK_EL0		*blk_el0;
	INTERFACE	*intfc = grid->grid_intfc;
	RECT_GRID	*gr = &grid->rect_grid;
	int		icoords[MAXD];
	int		dim = grid->rect_grid.dim;
	int		nvertex = 1<<dim;
	int		*offset = grid->offset;
	int		k;
	static BILINEAR_ELEMENT *c_bilin = NULL;/*constructed bilinear element*/
	static LINEAR_ELEMENT   *c_lin = NULL; /* constructed linear element */
	static LEAST_SQR_CLUSTER  *c_lsq = NULL; /* least square cluster */

	if (c_lin == NULL) /*First call, initialize storage*/
	{
	    scalar(&c_bilin,sizeof(BILINEAR_ELEMENT));
	    scalar(&c_lin,sizeof(LINEAR_ELEMENT));
	    scalar(&c_lsq,sizeof(LEAST_SQR_CLUSTER));
	}

	*bilin = NULL;		*lin = NULL; 		*lsq = NULL;

	if (is_exterior_comp(comp,intfc) || (comp < min_component(intfc))
					 || (comp > max_component(intfc)))
	    return FUNCTION_FAILED;

	if (rect_in_which(coords,icoords,gr) == FUNCTION_FAILED)
		return FUNCTION_FAILED;

	/*debug with collect_pcs_in_mesh3d  pcs_cell */
	if(debugging("line_pj"))
	{
	    printf("#line_pj debug\n");
	    print_rectangular_grid(gr);
	    print_int_vector("icoords=", icoords, 3, "\n");
	}
	
	blk_el0 = &Blk_el0(icoords,grid);

	if (blk_el0_is_bilinear(blk_el0)) /* regular cell */
	{
	    if (comp != blk_el0_pcs_els(blk_el0)->comp[0])
	    	return FUNCTION_FAILED;
	    c_bilin->comp = comp;
	    for (k = 0; k < nvertex; ++k)
	    {
	    	c_bilin->p[k] = blk_el0_pcs_els(blk_el0)->p + offset[k];
	    }
	    *bilin = c_bilin;
	    return FUNCTION_SUCCEEDED;
	}
	else			/* irregular cell */
	{
	    int    num_els = num_pcs_els_in_blk(blk_el0);
	    if (use_least_square(grid))
	    {
	    	collect_least_sqr_cluster(blk_el0_pcs_els(blk_el0),
                             num_els,comp,coords,&c_lsq,grid);
	    	*lsq = c_lsq;
		return FUNCTION_SUCCEEDED;
	    }
	    else if (build_linear_element(blk_el0_pcs_els(blk_el0),
			     num_els,comp,coords,&c_lin,grid))
	    {
	        *lin = c_lin;
	        return FUNCTION_SUCCEEDED;
	    }
	    else
	    	return FUNCTION_FAILED;
	}

}	/* end tg_build */

#if defined(ONED)

EXPORT	CRXING *nearest_crossing1d(
	int		*icoords,
	GRID_DIRECTION	dir,
	TRI_GRID	*ntg)
{
	CRXING		*crx;
	int		xmax;
	int		ic[MAXD];
	int		nix;
	int		k, nc;
	int		*list;
	Table		*T = table_of_interface(ntg->grid_intfc);

	tri_grid_index_from_comp_grid_index(ic,icoords,ntg);
	nix = ic[0];
	xmax = ntg->rect_grid.gmax[0];

	if ((nix < 0) || (nix > xmax))
	    crx = NULL;
	else if (dir == WEST)
	{
	    if (nix == 0)
		crx = NULL;
	    else
	    {
	    	k  = nix-1;
	    	nc = T->seg_crx_count[k];
	    					
	    	if (nc)
	    	{
	    	    list = T->seg_crx_lists[k];
	    	    crx  = T->crx_store + list[nc-1];
	    	}
	    	else
		    crx = NULL;
	    }
	}
	else if (dir == EAST)
	{
	    if (nix == xmax)
		crx = NULL;
	    else
	    {
	    	k  = nix;
	    	nc = T->seg_crx_count[k];
	    	if (nc)
	    	{
	    	    list = T->seg_crx_lists[k];
	    	    crx  = T->crx_store + list[0];
	    	}
	    	else
		    crx = NULL;
	    }
	}
	else
	    crx = NULL;

	return crx;
}		/*end nearest_crossing1d*/
#endif /* defined(ONED) */

#if defined(TWOD)

#define Cross_prod(x0,y0,x1,y1,xm,ym) 					\
	((x0 - xm)*(y1 - ym) - (x1 - xm)*(y0 - ym))

/*
*				tg_area():
*
*	Returns the area corresponding to component comp in grid square
*	icoords. The index icoords is assumed to be offset by one from the
*	origin of the rectangular grid ntg->tg_grid.  That is the point
*	icoords[i] = 0, i = 0,..,dim corresponds to the point ic[i] = 1,
*	i = 0,...dim, on the lattice ntg->tg_grid.  The reason for this
*	offset is that in the typical application of this this function,
*	icoords is the index of a rectangular cell on a lattice that is
*	dual to the lattice ntg->tg_grid.  It is assumed that the origin
*	of this dual lattice aligns with the cells center of the 0th cell
*	in the lattice ntg->tg_grid.
*/

LIB_LOCAL	double tg_area(
	int		*icoords,
	COMPONENT	comp,
	TRI_GRID	*grid)
{
	BLK_EL0		*blk_el0;
	INTERFACE	*intfc = grid->grid_intfc;
	LINEAR_ELEMENT  *et;
	RECT_GRID	*gr    = &grid->rect_grid;
	double		x0, y0, x1, y1, x2, y2, ans;

	if (is_exterior_comp(comp,intfc) || (comp < min_component(intfc)) ||
			(comp > max_component(intfc)))
	    return 0.0;

	blk_el0 = &Regular_blk_el0(icoords,grid);

	if (blk_el0_is_bilinear(blk_el0))
	{
	    ans = (blk_el0_bilinear_el(blk_el0)->comp != comp) ? 0.0 :
	    	  gr->h[0] * gr->h[1];
	}
	else
	{
	    int		k, num_els = num_lin_els_in_blk(blk_el0);
	    ans = 0.0;
	    for (k = 0;  k < num_els;  ++k)
	    {
	    	et = blk_el0_linear_els(blk_el0) + k;
	    	if (et->comp == comp)
	    	{
	    	    x0 = Coords(et->p[0])[0];
	    	    y0 = Coords(et->p[0])[1];
	    	    x1 = Coords(et->p[1])[0];
	       	    y1 = Coords(et->p[1])[1];
	    	    x2 = Coords(et->p[2])[0];
	    	    y2 = Coords(et->p[2])[1];
	    	    ans += fabs(Cross_prod(x2,y2,x0,y0,x1,y1));
	    	}
	    }
	    ans *= 0.5;
	}
	return ans;
}		/*end tg_area*/


/*ARGSUSED*/
EXPORT	boolean	point_in_triangle(
	double		*coords,
	LINEAR_ELEMENT	*et,
	COMPONENT	comp,
	TRI_GRID	*ntg)
{
	double	x0, y0, x1, y1, x2, y2, cp;
	double	*h = ntg->rect_grid.h;

	if (et->comp != comp)
	    return NO;

	x0 = Coords(et->p[0])[0];	y0 = Coords(et->p[0])[1];
	x1 = Coords(et->p[1])[0];	y1 = Coords(et->p[1])[1];
	x2 = Coords(et->p[2])[0];	y2 = Coords(et->p[2])[1];

	cp = Cross_prod(x2,y2,x0,y0,x1,y1);

	if (cp == 0.0) 
	    return NO;                  /* degenerated triangle */
	else if (cp > 0.0)		/* counterclockwise triangle */
	{
	    cp = Cross_prod(x1,y1,coords[0],coords[1],x0,y0);
	    if (cp < 0.0)
	    	return NO;
	    cp = Cross_prod(x2,y2,coords[0],coords[1],x1,y1);
	    if (cp < 0.0)
	    	return NO;
	    cp = Cross_prod(x0,y0,coords[0],coords[1],x2,y2);
	    if (cp < 0.0)
	    	return NO;
	}
	else				      /* clockwise triangle */
	{
	    cp = Cross_prod(x1,y1,coords[0],coords[1],x0,y0);
	    if (cp > 0.0)
	    	return NO;
	    cp = Cross_prod(x2,y2,coords[0],coords[1],x1,y1);
	    if (cp > 0.0)
	    	return NO;
	    cp = Cross_prod(x0,y0,coords[0],coords[1],x2,y2);
	    if (cp > 0.0)
	    	return NO;
	}
	return YES;
}	/*end point_in_triangle*/



/*
*			nearest_crossing2d():
*
*	Given a dual lattice node nix, niy  = icoords[0] + 1, icoords[1] + 1,
*	and a direction dir this routine returns the nearest cross on the
*	adjacent grid line in that direction. If no cross exits, the routine
*	returns NULL.
*
*	The index icoords is assumed to be offset by one from the origin
*	of the rectangular grid ntg->tg_grid.  That is the point
*	icoords[i] = 0, i = 0,..,dim corresponds to the point ic[i] = 1,
*	i = 0,...dim, on the lattice ntg->tg_grid.  The reason for this
*	offset is that in the typical application of this this function,
*	icoords is the index of a rectangular cell on a lattice that is
*	dual to the lattice ntg->tg_grid.  It is assumed that the origin
*	of this dual lattice aligns with the cells center of the 0th cell
*	in the lattice ntg->tg_grid.
*/

EXPORT	CRXING *nearest_crossing2d(
	int		*icoords,
	GRID_DIRECTION	dir,
	TRI_GRID	*ntg)
{
	CRXING		*crx;
	int		xmax, ymax;
	int		ic[MAXD];
	int		nix, niy;
	int		k, nc;
	int		*list;
	Table		*T = table_of_interface(ntg->grid_intfc);

	tri_grid_index_from_comp_grid_index(ic,icoords,ntg);
	nix = ic[0], niy = ic[1];
	xmax = ntg->rect_grid.gmax[0];
	ymax = ntg->rect_grid.gmax[1];

	if ((nix < 0) || (niy < 0) || (nix > xmax) || (niy > ymax))
	    crx = NULL;
	else if (dir == SOUTH)
	{
	    if ((niy == 0) || (nix == xmax))
		crx = NULL;
	    else
	    {
	    	k  = 2*niy*xmax + niy - 1 + nix - xmax;
	    	nc = T->seg_crx_count[k];
	    	if (nc)
	    	{
	    	    list = T->seg_crx_lists[k];
	    	    crx  = T->crx_store + list[nc-1];
	    	}
	    	else
		    crx = NULL;
	    }
	}
	else if (dir == WEST)
	{
	    if ((nix == 0) || (niy == ymax))
		crx = NULL;
	    else
	    {
	    	k  = 2*niy*xmax + niy + nix-1;
	    	nc = T->seg_crx_count[k];
	    					
	    	if (nc)
	    	{
	    	    list = T->seg_crx_lists[k];
	    	    crx  = T->crx_store + list[nc-1];
	    	}
	    	else
		    crx = NULL;
	    }
	}
	else if (dir == NORTH)
	{
	    if ((niy == ymax) || (nix == xmax))
		crx = NULL;
	    else
	    {
	    	k  = 2*niy*xmax + niy + xmax + nix;
	    	nc = T->seg_crx_count[k];
	    	if (nc)
	    	{
	    	    list = T->seg_crx_lists[k];
	    	    crx  = T->crx_store + list[0];
	    	}
	    	else
		    crx = NULL;
	    }
	}
	else if (dir == EAST)
	{
	    if ((niy == ymax) || (nix == xmax))
		crx = NULL;
	    else
	    {
	    	k  = 2*niy*xmax + niy + nix;
	    	nc = T->seg_crx_count[k];
	    	if (nc)
	    	{
	    	    list = T->seg_crx_lists[k];
	    	    crx  = T->crx_store + list[0];
	    	}
	    	else
		    crx = NULL;
	    }
	}
	else
	    crx = NULL;

	return crx;
}		/*end nearest_crossing2d*/

#endif /* defined(TWOD) */

#if defined(THREED)
EXPORT CRXING *nearest_crossing3d(
	int		*icoords,
	GRID_DIRECTION	dir,
	TRI_GRID	*ntg)
{
	int		ic[MAXD],*gmax;
	int		dim = ntg->grid_intfc->dim;
	CRXING		*crx;
	int		i,k,nc,*list;
	Table		*T = table_of_interface(ntg->grid_intfc);

	tri_grid_index_from_comp_grid_index(ic,icoords,ntg);
	gmax = ntg->rect_grid.gmax;
	for (i = 0; i < dim; ++i)
	{
	    if (ic[i] <= 0 || ic[i] >= gmax[i])
	    	return NULL;
	}
	k = seg_index3d(ic[0],ic[1],ic[2],dir,gmax);
	nc = T->seg_crx_count[k];
	list = T->seg_crx_lists[k];
	if (nc == 0) return NULL;
	switch (dir)
	{
	case WEST:
	case SOUTH:
	case LOWER:
	    crx = T->crx_store + list[nc-1];
	    break;
	case EAST:
	case NORTH:
	case UPPER:
	    crx = T->crx_store + list[0];
	    break;
	}
	return crx;
}		/*end nearest_crossing3d*/
#endif /* defined(THREED) */


LOCAL	boolean build_linear_element(
	POINT_COMP_ST	*pcs,
	int		num_els,
	COMPONENT	comp,
	double		*coords,
	LINEAR_ELEMENT	**lin,
	TRI_GRID	*ntg)
{
	POINT_COMP_ST	*pcs_tmp;
	double		d2_tmp;
	int		npcs;
	int		i,j,k,l;
	int		dim = ntg->rect_grid.dim;
	int		number_of_bilinear_element_vertices = (1 << dim);
	int		number_of_linear_element_vertices = dim + 1;
	static	POINT_COMP_ST	**pcs_list = NULL;
	static	double		*d2 = NULL;
	static	int		max_npcs = 0;

	if (pcs_list == NULL)
	{
	    max_npcs = number_of_bilinear_element_vertices + num_els;
	    uni_array(&pcs_list,max_npcs,sizeof(POINT_COMP_ST*));
	    uni_array(&d2,max_npcs,FLOAT);
	}
	else if (max_npcs < (number_of_bilinear_element_vertices + num_els))
	{
	    free_these(2,pcs_list,d2);
	    max_npcs = number_of_bilinear_element_vertices + num_els;
	    uni_array(&pcs_list,max_npcs,sizeof(POINT_COMP_ST*));;
	    uni_array(&d2,max_npcs,FLOAT);
	}

	for (npcs = 0, i = 0; i < number_of_bilinear_element_vertices; ++i)
	{
	    if (pcs[i].comp[0] == comp)
	    {
	        pcs_list[npcs] = pcs+i;
		d2[npcs] = sqr(coords[0] - Coords(pcs_list[npcs]->p)[0]);
		for (j = 1; j < dim; ++j)
		    d2[npcs] += sqr(coords[j] - Coords(pcs_list[npcs]->p)[j]);
	    	++npcs;
	    }
	}
	for (i = number_of_bilinear_element_vertices; i < num_els; ++i)
	{
	    if (pcs[i].comp[0]==comp || pcs[i].comp[1]==comp)
	    {
	    	pcs_list[npcs] = pcs+i;
		d2[npcs] = sqr(coords[0] - Coords(pcs_list[npcs]->p)[0]);
		for (j = 1; j < dim; ++j)
		    d2[npcs] += sqr(coords[j] - Coords(pcs_list[npcs]->p)[j]);
	    	++npcs;
	    }
	}
	if (npcs < number_of_linear_element_vertices)
		return FUNCTION_FAILED;
	
	for (i = 0; i < npcs-1; ++i)
	{
	    for (j = i+1; j < npcs; ++j)
	    {
		if (d2[i] > d2[j])
		{
		    pcs_tmp = pcs_list[j];
		    pcs_list[j] = pcs_list[i];
		    pcs_list[i] = pcs_tmp;
		    d2_tmp = d2[j];
		    d2[j] = d2[i];
		    d2[i] = d2_tmp;
		}
	    }
	}
	(*lin)->comp = comp;
	
	if(debugging("line_pj"))
	{
	    printf("#npcs=%d comp=%d\n", npcs, (*lin)->comp);
	    print_general_vector("coords=", coords, 3, "\n");
	    for(i=0; i<npcs; i++)
	    {
	        printf("comp = %d  %d  d2=%24.16e\n",pcs_list[i]->comp[0], 
		    pcs_list[i]->comp[1], d2[i]);
		print_general_vector("p=", Coords(pcs_list[i]->p), 3, "\n");
	    }
	}

	switch(dim)
	{
#if defined(TWOD)
	case 2:
	    for (i = 0; i < npcs; i++)
	    {
	    	(*lin)->p[0] = pcs_list[i]->p;
	    	(*lin)->s[0] = (pcs_list[i]->comp[0] == comp) ? 
				pcs_list[i]->s[0] : 
				pcs_list[i]->s[1];
	    	for (j = i+1; j < npcs; j++)
		{
	    	    (*lin)->p[1] = pcs_list[j]->p;
	    	    (*lin)->s[1] = (pcs_list[j]->comp[0] == comp) ? 
				    pcs_list[j]->s[0] : 
				    pcs_list[j]->s[1];
		    for (k = j+1; k < npcs; k++)
		    {
	    	    	(*lin)->p[2] = pcs_list[k]->p;
	    	    	(*lin)->s[2] = (pcs_list[k]->comp[0] == comp) ? 
				    	pcs_list[k]->s[0] : 
					pcs_list[k]->s[1];
			if (point_in_linear_element(coords,*lin,comp,ntg) 
						== YES)
	    		    return FUNCTION_SUCCEEDED;
		    }
		}
	    }
	    break;
#endif /* defined(TWOD) */
#if defined(THREED)
	case 3:
	    for (i = 0; i < npcs; i++)
	    {
	    	(*lin)->p[0] = pcs_list[i]->p;
	    	(*lin)->s[0] = (pcs_list[i]->comp[0] == comp) ? 
				pcs_list[i]->s[0] : 
				pcs_list[i]->s[1];
	    	for (j = i+1; j < npcs; j++)
		{
	    	    (*lin)->p[1] = pcs_list[j]->p;
	    	    (*lin)->s[1] = (pcs_list[j]->comp[0] == comp) ? 
				    pcs_list[j]->s[0] : 
				    pcs_list[j]->s[1];
		    for (k = j+1; k < npcs; k++)
		    {
	    	    	(*lin)->p[2] = pcs_list[k]->p;
	    	    	(*lin)->s[2] = (pcs_list[k]->comp[0] == comp) ? 
				    	pcs_list[k]->s[0] : 
					pcs_list[k]->s[1];
			for (l = k+1; l < npcs; l++)
			{
	    	    	    (*lin)->p[3] = pcs_list[l]->p;
	    	    	    (*lin)->s[3] = (pcs_list[l]->comp[0] == comp) ? 
				    	    pcs_list[l]->s[0] : 
					    pcs_list[l]->s[1];
			    if (point_in_linear_element(coords,*lin,comp,ntg) 
			    			== YES)
	    		        return FUNCTION_SUCCEEDED;
			}
		    }
		}
	    }
#endif /* defined(THREED) */
	}
	return FUNCTION_FAILED;
}		/*end build_linear_element*/


LOCAL	boolean collect_least_sqr_cluster(
	POINT_COMP_ST	*pcs,
	int		num_els,
	COMPONENT	comp,
	double		*coords,
	LEAST_SQR_CLUSTER **lsq,
	TRI_GRID	*ntg)
{
	int		i,j,npcs;
	int		dim = ntg->rect_grid.dim;
	int             number_of_bilinear_element_vertices = (1 << dim);
	TG_PT		*p_tmp;
	Locstate	s_tmp;
	double		d2_tmp;
	static		TG_PT **p;
	static		Locstate *s;
	static		double *d2;

	if (p == NULL)
	{
	    stat_vector(&p,MAX_LSQ_PTS,sizeof(TG_PT*));
	    stat_vector(&s,MAX_LSQ_PTS,sizeof(Locstate));
	    stat_vector(&d2,MAX_LSQ_PTS,FLOAT);
	}

	(*lsq)->comp = comp;
	(*lsq)->dim = dim;
	for (npcs = 0, i = 0; i < number_of_bilinear_element_vertices; ++i)
	{
	    if (pcs[i].comp[0] == comp)
	    {
	    	p[npcs] = pcs[i].p;
	    	s[npcs] = pcs[i].s[0];
		for (j = 1; j < dim; ++j)
		    d2[npcs] += sqr(coords[j] - Coords(pcs[i].p)[j]);
	    	++npcs;
	    }
	}
	for (i = number_of_bilinear_element_vertices; i < num_els; ++i)
	{
	    if (pcs[i].comp[0] == comp)
	    {
	    	p[npcs] = pcs[i].p;
	    	s[npcs] = pcs[i].s[0];
		for (j = 1; j < dim; ++j)
		    d2[npcs] += sqr(coords[j] - Coords(pcs[i].p)[j]);
	    	++npcs;
	    }
	    else if (pcs[i].comp[1] == comp)
	    {
	    	p[npcs] = pcs[i].p;
	    	s[npcs] = pcs[i].s[1];
		for (j = 1; j < dim; ++j)
		    d2[npcs] += sqr(coords[j] - Coords(pcs[i].p)[j]);
	    	++npcs;
	    }
	}
	for (i = 0; i < npcs-1; ++i)
	{
	    for (j = i+1; j < npcs; ++j)
	    {
	    	if (d2[i] > d2[j])
		{
		    p_tmp = p[j];
		    p[j] = p[i];
		    p[i] = p_tmp;
		    s_tmp = s[j];
		    s[j] = s[i];
		    s[i] = s_tmp;
		    d2_tmp = d2[j];
                    d2[j] = d2[i];
                    d2[i] = d2_tmp;
		}
	    }
	}
	(*lsq)->p = p;
	(*lsq)->s = s;
	(*lsq)->nr = npcs;
	return FUNCTION_SUCCEEDED;
}		/*end collect_least_sqr_cluster */
#endif /* defined(TWOD) || defined(THREED) */
