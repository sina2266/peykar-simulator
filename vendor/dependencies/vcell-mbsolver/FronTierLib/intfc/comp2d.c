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
*				comp2d.c:
*
*
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*		2D SPECIFIC CODE FOR COMPONENT COMPUTATIONS:
*
*			See comments in comp.c
*
*/


#if defined(DEBUG_COMP2D)
#define DEBUG_STRING "comp2d"
#endif /* defined(DEBUG_COMP2D) */

#include <intfc/iloc.h>

	/* LOCAL Function Declarations */
LOCAL	COMPONENT find_compon2d(int,int,INTERFACE*);
LOCAL	boolean	blocks_on_bond(BOND*,CURVE*,int**,COMPONENT**,RECT_GRID*);
LOCAL	boolean	comp_is_on_curve(CURVE*,const COMPONENT*,int);
LOCAL	boolean	make_bond_lists(INTERFACE*);
LOCAL	boolean	find_on_front_block_range2d(double,double,int,int,
				const COMPONENT*,int,INTERFACE*,USE_BOUNDARIES,
				HYPER_SURF*,int*,int*,int*,int*,int);
LOCAL	boolean	on_front_block2d(int,int,const COMPONENT*,int,INTERFACE*,
	                         USE_BOUNDARIES,HYPER_SURF*);
LOCAL	boolean	set_bond_and_curve_list_pointers(struct Table*,int,int);
LOCAL	double	i_sd2d(double,double,BOND*,FT_POINT**,double*,SIDE*);
LOCAL	SIDE	check_closest_point_is_node(double*,SIDE,HYPER_SURF*,COMPONENT,
					    COMPONENT,COMPONENT,USE_BOUNDARIES,
					    INTERFACE*);
LOCAL 	SIDE 	convex_side(FT_POINT*,BOND*,CURVE*,SIDE);
LOCAL	void	fill_bond_and_curve_lists(int**,BOND****,CURVE****,INTERFACE*);
LOCAL	void	mark_end_of_bond(void);
LOCAL	void	set_off_front_comp2d(COMPONENT**,INTERFACE*);
LOCAL	void	show_BOND_list(INTERFACE*);
LOCAL	void	store_bond(int,int,int**,COMPONENT**);

	/* LOCAL Variable Declarations */

/*
*	p_closest, b_closest, and c_closest record the nearest POINT,
*	BOND, and CURVE to the position last used as an argument to
*	the nearest_interface_point or component functions below.
*	The variable t_last records the fractional distance along
*	the bond b_closest to this position if called from the
*	nearest_interface_point functions.
*/
LOCAL	FT_POINT	*p_closest = NULL;
LOCAL	BOND	*b_closest = NULL;
LOCAL	CURVE	*c_closest = NULL;
LOCAL	double	t_last;	/* Parameter t on last call to shortest_distance2d */
static	int count,max_size;


LOCAL	boolean	comp_is_on_curve(
	CURVE *curve,
	const COMPONENT *comps,
	int   n_eq)
{
	int i;
	COMPONENT pcomp, ncomp;

	if (comps == NULL)
	    return YES;

	pcomp = positive_component(curve);
	ncomp = negative_component(curve);

	for (i = 0; i < n_eq; ++i)
	{
	    if (pcomp == comps[i])
		return YES;
	    else if (ncomp == comps[i])
		return YES;
	}
	return NO;
}		/*end comp_is_on_curve*/

#define	comps_are_on_curve(curve,l_comp,r_comp)				\
	comps_are_on_hyper_surf(Hyper_surf(curve),l_comp,r_comp)


/*
*				component2d():
*
*	Determines the topological COMPONENT relative to a given
*	INTERFACE which contains the point x,y.
*
*	If the INTERFACE has been modified since last call, then
*	the bond, curve, comp lists are recreated.   This imposes
*	a rectangular grid structure on the INTERFACE, by default
*	over the smallest rectangle that contains all of the
*	INTERFACE points.   The list of bonds and curves passing
*	through each grid block is then computed, bonds[iy][ix]
*	curves[iy][ix], and also the array of off-front component
*	values  compon2d[iy][ix].   For on-front blocks, the compon2d[][]
*	array is given the flag value ONFRONT.
*
*	component2d() determines the grid block containing the point
*	x,y and if it is off-front, returns the value compon2d[iy][ix].
*	If on-front, it locates the closest INTERFACE curve within
*	the grid block (and determines which side of the curve it
*	is on) by looping over the local bonds[iy][ix].  It then
*	returns the appropriate left or right COMPONENT value of
*	that CURVE.
*
*	Returns component value if successful, or ERROR on error.
*	This assumes that ERROR is not a possible COMPONENT value.
*	ERROR occurs if enough space cannot be found for the
*	allocation of bond_list arrays.
*	Note:  Distances below are actually squared distances.
*/

LIB_LOCAL COMPONENT component2d(
	double		*coords,
	INTERFACE	*intfc)
{
	FT_POINT		*p;
	BOND		**b;
	CURVE		**c;
	SIDE		side;	       /* Side of interface bond -
					* either NEGATIVE_SIDE(=left)
					* or POSITIVE_SIDE(=right)
					*/
	SIDE		side_closest;  /* Side of closest bond  */
	double		distance;      /* Distance from (x,y) to a Bond */
	double		min_distance;  /* Distance to Nearest Bond */
	double		norm_dist;     /* Normal Distance to a Bond */
	double		min_norm_dist; /* Normal Distance to the Nearest Bond*/
	int		ix,iy;	       /* Grid square containing coords */
	int             ixmin, ixmax, iymin, iymax;
	int		icoords[MAXD];
	int		k;
	struct Table	*T = intfc->table;

	/* Check for no curves in the interface (interior only) */
	if (intfc->curves == NULL)
	    return MIN_INTERIOR_COMP;

			/* In case closest_bond_curve() is called */
	min_norm_dist = HUGE_VAL; /* For lint */
	c_closest = NULL;	

	if (intfc->modified || T->new_grid)
	{
	    if (make_bond_comp_lists(intfc) == FUNCTION_FAILED)
	    {
	    	(void) printf("WARNING in component2d(), "
	    	              "make_bond_comp_lists failed\n");
	    	return ERROR;
	    }
	}

	if (rect_in_which(coords,icoords,&T->rect_grid) == FUNCTION_FAILED)
	    return exterior_component(intfc);	/* Point Outside */
	
	ix = icoords[0]; iy = icoords[1];
	if (T->compon2d[iy][ix] != ONFRONT)
	    return T->compon2d[iy][ix]; 		/* Off Front */

		/* On Front: */

			/* Find Closest Point on Front: */

	p_closest = NULL;
	min_distance = HUGE_VAL;

	ixmin = (ix > 0) ? ix-1 : ix;
	ixmax = ((ix+1) < T->rect_grid.gmax[0]) ? ix+1 : ix;
	iymin = (iy > 0) ? iy-1 : iy;
	iymax = ((iy+1) < T->rect_grid.gmax[1]) ? iy+1 : iy;

	for (iy = iymin; iy <= iymax; ++iy)
	{
	    for (ix = ixmin; ix <= ixmax; ++ix)
	    {
	        if (T->compon2d[iy][ix] != ONFRONT)
		    continue;

	        b = T->bonds[iy][ix];
	        c = T->curves[iy][ix];

	        for (k=0; k<T->num_of_bonds[iy][ix]; ++k,++b,++c)
	        {
	            distance = i_sd2d(coords[0],coords[1],*b,&p,
			              &norm_dist,&side);

	            if ((p != NULL) && (p == p_closest))
	            {

	            /*							    *
	            *							    *
	            *   Whenever p != NULL the point x,y is beyond the end  *
	            *   of the bond.   Thus this is the case where x,y is   *
	            *   beyond the end of two adjacent bonds, with their    *
	            *   intersection being the closest point to x,y.        *
	            *							    *
	            *   The side is in general ambiguous.		    *
	            *							    *
	            *   The correct bond wrt which the side should be com-  *
	            *   puted is that which has the larger normal distance  *
	            *   from x,y.					    *
	            *							    *
	            *   The following code assumes that both bonds belong   *
	            *   to the same curve except if the point x,y is	    *
	            *   essentially the intersection point.		    *
	            *							    */

	    	    	/* min_nor.. has value when used */
	    	        if (norm_dist >= min_norm_dist) 
	    	        {
	    	            side_closest = side;
	    	            b_closest = *b;
	    	            c_closest = *c;  
	    	        }
	            }
	            else if (distance < min_distance)
	            {
	    	        min_distance = distance;
	    	        min_norm_dist = norm_dist;
	    	        side_closest = side;
	    	        p_closest = p;
	    	        b_closest = *b;
	    	        c_closest = *c;
	            }
	        }
	    }
	}
	if (p_closest != NULL)
	{
	    side_closest = convex_side(p_closest,b_closest,c_closest,
	    		side_closest);
	}

	/*
	 * Possibly correct side_closest, b_closest, c_closest when p_closest
	 * is a POINT (and so might be the position of a NODE).
	 */
	side_closest = check_closest_point_is_node(coords,side_closest,NULL,
						   NO_COMP,NO_COMP,NO_COMP,
						   INCLUDE_BOUNDARIES,
						   intfc);
	return (side_closest == NEGATIVE_SIDE) ? negative_component(c_closest) :
	                                         positive_component(c_closest);
}		/*end component2d*/



/*
*			nearest_interface_point2d():
*
*	Given a point x,y and a COMPONENT comp, locates the closest
*	point px,py of the INTERFACE which borders COMPONENT comp.
*	Also returns the bond and curve containing p, and the parametric
*	location of the point on this bond.
*	Returns value 1 or 0 if successful or not in finding a
*	closest point.
*
*	This routine is O(1) if called on an ONFRONT gridblock,
*	but is otherwise O(N).
*
*	If hs is not NULL then component and boundary are ignored,
*	and instead the routine finds the closest point along Curve_of_hs(hs).
*/



LIB_LOCAL boolean nearest_interface_point2d(
	double		   *coords,
	COMPONENT	   comp,
	INTERFACE	   *intfc,
	USE_BOUNDARIES	   bdry,
	HYPER_SURF	   *hs,
	double		   *ans,
	double		   *t,
	HYPER_SURF_ELEMENT **phse,
	HYPER_SURF	   **phs)
{
    	const COMPONENT *eq_comps;
	int             n_eq;
	FT_POINT        *p;
	BOND         **b;
	CURVE        **c;
	RECT_GRID    *gr;
	SIDE	     side;	    /* Side of interface bond -
				     * either NEGATIVE_SIDE(=left)
				     * or POSITIVE_SIDE(=right)
				     */
	double        distance;      /* Distance from (x,y) to a Bond */
	double        min_distance;  /* Distance to Nearest Bond */
	double        norm_dist;     /* Normal Distance to a Bond */
	double        min_norm_dist; /* Normal Distance to the Nearest Bond */
	double        t_closest;     /* Parametric coord of closest point. */
	double        x, y;
	int          ix, iy;        /* Grid square containing x,y */
	int          icoords[MAXD];
	int          ix1, ix2, iy1, iy2;
	int          ix_max, iy_max;
	int          k;
	struct Table *T = intfc->table;

	if (hs && hs->interface != intfc)
	{
	    screen("ERROR in nearest_interface_point2d()"
		   "hs->interface != intfc\n");
	    clean_up(ERROR);
	}

	min_norm_dist = HUGE_VAL; /* For lint */
	if (intfc->modified || T->new_grid)
	{
	    if (make_bond_comp_lists(intfc) == FUNCTION_FAILED)
	    {
	        static int first = YES;

	        (void) printf("WARNING in nearest_interface_point2d(), "
	                      "make_bond_comp_lists() failed\n");
	        (void) printf("coords = (%g, %g), ",coords[0],coords[1]);
	        (void) printf("comp = %d, bdry = %d, ",comp,bdry);
	        (void) printf("hs = %llu\n",hypersurface_number(hs));
	        (void) printf("Topological grid of interface\n");
	        print_rectangular_grid(&topological_grid(intfc));
	        if (first == YES)
	        {
	            (void) printf("Interface into ");
	            (void) printf("nearest_interface_point2d()\n");
	            print_interface(intfc);
	        }
	        first = NO;
	        return NO;
	    }
	}
	gr = &topological_grid(intfc);
    	eq_comps = equivalent_components_list(comp,&n_eq,intfc);

	 x = coords[0];    y = coords[1];
	p_closest = NULL;    c_closest = NULL;       b_closest = NULL;
	min_distance = HUGE_VAL;
	t_last = 0.0;

	if (!rect_in_which(coords,icoords,gr))
	{
	    return long_nearest_interface_point2d(coords,comp,intfc,
	                                              bdry,hs,ans,t,phse,phs);
	}
	else if (T->compon2d[icoords[1]][icoords[0]] != ONFRONT) /* Off Front */
	{
	    ix = icoords[0];  iy = icoords[1];
	    if (!find_on_front_block_range2d(x,y,ix,iy,eq_comps,n_eq,intfc,
			                     bdry,hs,&ix1,&ix2,&iy1,&iy2,-1))
	        return NO;
	}
	else	/* On Front: */
	{
	    ix = icoords[0];  iy = icoords[1];

	    /* Find Closest Point on Front: */

	    /*   setting of returned argument values to NULL is a good  */
	    /* idea even though a success status value is also returned */

	    /* Check center block first for existence of allowed closest point */
	    b = T->bonds[iy][ix];  c = T->curves[iy][ix];
	    for (k = 0; k < T->num_of_bonds[iy][ix]; ++k, ++b, ++c)
	    {
		if (( hs && (Curve_of_hs(hs) == *c)) ||
		    (!hs && comp_is_on_curve(*c,eq_comps,n_eq) &&
		       !skip_boundary_hs(Hyper_surf(*c),bdry)))
		{
	            distance = i_sd2d(x,y,*b,&p,&norm_dist,&side);

	            if ((p != NULL) && (p == p_closest))
	            {

	            /*                                                      *
	            *                                                       *
	            *   Whenever p != NULL the point x,y is beyond the end  *
	            *   of the bond.   Thus this is the case where x,y is   *
	            *   beyond the end of two adjacent bonds, with their    *
	            *   intersection being the closest point to x,y.        *
	            *                                                       *
	            *   The side is in general ambiguous.                   *
	            *                                                       *
	            *   The correct bond wrt which the side should be com-  *
	            *   puted is that which has the larger normal distance  *
	            *   from x,y.                                           *
	            *                                                       *
	            *   The following code assumes that both bonds belong   *
	            *   to the same curve.                                  *
	            *                                                       */

	                /* min_nor.. has value when used */
	                if (norm_dist >= min_norm_dist)
	                {
	                    t_closest = t_last;
	                    b_closest = *b;
	                    c_closest = *c;  
	                }
	            }
	            else if (distance < min_distance)
	            {
	                min_distance = distance;
	                min_norm_dist = norm_dist;
	                t_closest = t_last;
	                p_closest = p;
	                b_closest = *b;
	                c_closest = *c;
	            }
		}
	    } 
	    if (b_closest == NULL)
	    {
	        if (!find_on_front_block_range2d(x,y,ix,iy,eq_comps,n_eq,
			                         intfc,bdry,hs,&ix1,&ix2,
						 &iy1,&iy2,-1))
	            return NO;
	    }
	    else
	    {
	        ix_max = gr->gmax[0]-1;
	        distance = sqrt(min_distance);
	        ix1 = cell_index(x - distance,0,gr);
	        ix1 = max(0,ix1);
	        ix1 = min(ix,ix1);
	        ix2 = cell_index(x + distance,0,gr);
	        ix2 = min(ix_max,ix2);
	        ix2 = max(ix,ix2);

	        iy_max = gr->gmax[1]-1;
	        iy1 = cell_index(y - distance,1,gr);
	        iy1 = max(0,iy1);
	        iy1 = min(iy,iy1);
	        iy2 = cell_index(y + distance,1,gr);
	        iy2 = min(iy_max,iy2);
	        iy2 = max(iy,iy2);
	    }
	}

	for (ix = ix1; ix <= ix2; ++ix)
	for (iy = iy1; iy <= iy2; ++iy)
	{
	    if (((ix == icoords[0]) && (iy == icoords[1])) ||
	        (T->compon2d[iy][ix] != ONFRONT))
	        continue;
	    b = T->bonds[iy][ix];  c = T->curves[iy][ix];
	    for (k = 0; k < T->num_of_bonds[iy][ix]; ++k, ++b, ++c)
	    {
		if (( hs && (Curve_of_hs(hs) == *c)) ||
		    (!hs && comp_is_on_curve(*c,eq_comps,n_eq) &&
		       !skip_boundary_hs(Hyper_surf(*c),bdry)))
		{
	            distance = i_sd2d(x,y,*b,&p,&norm_dist, &side);

	            if ((p != NULL) && (p == p_closest))
	            {

	            /*                                                      *
	            *                                                       *
	            *   Whenever p != NULL the point x,y is beyond the end  *
	            *   of the bond.   Thus this is the case where x,y is   *
	            *   beyond the end of two adjacent bonds, with their    *
	            *   intersection being the closest point to x,y.        *
	            *                                                       *
	            *   The side is in general ambiguous.                   *
	            *                                                       *
	            *   The correct bond wrt which the side should be com-  *
	            *   puted is that which has the larger normal distance  *
	            *   from x,y.                                           *
	            *                                                       *
	            *   The following code assumes that both bonds belong   *
	            *   to the same curve.                                  *
	            *                                                       */

	                /* min_nor.. has value when used */
	                if (norm_dist >= min_norm_dist)
	                {
	                    t_closest = t_last;
	                    b_closest = *b;
	                    c_closest = *c;  
	                }
	            }

	            else if (distance < min_distance)
	            {
	                min_distance = distance;
	                min_norm_dist = norm_dist;
	                t_closest = t_last;
	                p_closest = p;
	                b_closest = *b;
	                c_closest = *c;
	            }
		}
	     } 
	}

	if (b_closest == NULL)
	   return NO;

	/*
	 * Possibly correct side, b_closest, c_closest when
	 * p_closest might be a NODE.
	 * DO NOT correct if hs is non-NULL since
	 * c_closest may be set != Curve_of_hs(hs) by this call.
	 */
	side = check_closest_point_is_node(coords,side,hs,comp,
					   NO_COMP,NO_COMP,bdry,intfc);
	*phse = Hyper_surf_element(b_closest);
	*phs = Hyper_surf(c_closest);
	ans[0] = (1.0 - t_closest)*Coords(b_closest->start)[0] +
	                t_closest *Coords(b_closest->end)[0];
	ans[1] = (1.0 - t_closest)*Coords(b_closest->start)[1] +
	                t_closest *Coords(b_closest->end)[1];
	*t = t_closest;
	return YES;
}	    /*end nearest_interface_point2d*/

LOCAL	boolean find_on_front_block_range2d(
	double           x,
	double           y,
	int             ix,
	int             iy,
	const COMPONENT *eq_comps,
	int             n_eq,
	INTERFACE       *intfc,
	USE_BOUNDARIES  bdry,
	HYPER_SURF      *hs,
	int             *ix1,
	int             *ix2,
	int             *iy1,
	int             *iy2,
	int		range)
{
	RECT_GRID *gr;
	boolean      block_found;
	double     dx, dy;
	double     dx0, dx1, dy0, dy1;
	int       i, j, k, kmax;
	int       imin, imax, jmin, jmax;
	int       ix_max, iy_max;

	gr = &topological_grid(intfc);
	ix_max = gr->gmax[0]-1;
	iy_max = gr->gmax[1]-1;
	kmax = max(ix_max-ix,ix);
	kmax = max(kmax,iy);
	kmax = max(kmax,iy_max-iy);
	if (range != -1)
	{
	    kmax = min(range,kmax);
	}

	block_found = NO;
	for (k = 1; k < kmax && !block_found; ++k)
	{
	    if (k <= ix)
	    {
		i = ix-k;
		jmin = max(0,iy-k);
		jmax = min(iy_max,iy+k);
		for (j = jmin; j <= jmax; ++j)
		{
		    if (on_front_block2d(i,j,eq_comps,n_eq,intfc,bdry,hs))
		    {
			block_found = YES;
			break;
		    }
		}
		if (block_found)
		    break;
	    }
	    if (k <= (ix_max-ix))
	    {
		i = ix+k;
		jmin = max(0,iy-k);
		jmax = min(iy_max,iy+k);
		for (j = jmin; j <= jmax; ++j)
		{
		    if (on_front_block2d(i,j,eq_comps,n_eq,intfc,bdry,hs))
		    {
			block_found = YES;
			break;
		    }
		}
		if (block_found)
		    break;
	    }
	    if (k <= iy)
	    {
		j = iy-k;
		imin = max(0,ix-k);
		imax = min(ix_max,ix+k);
		for (i = imin; i <= imax; ++i)
		{
		    if (on_front_block2d(i,j,eq_comps,n_eq,intfc,bdry,hs))
		    {
			block_found = YES;
			break;
		    }
		}
		if (block_found)
		    break;
	    }
	    if (k <= (iy_max-iy))
	    {
		j = iy+k;
		imin = max(0,ix-k);
		imax = min(ix_max,ix+k);
		for (i = imin; i <= imax; ++i)
		{
		    if (on_front_block2d(i,j,eq_comps,n_eq,intfc,bdry,hs))
		    {
			block_found = YES;
			break;
		    }
		}
		if (block_found)
		    break;
	    }
	}
	if (!block_found)
	    return NO;

	if ((i < ix) && (j < iy))
	{
	    dx = x - cell_edge(i,0,gr);
	    dy = y - cell_edge(j,1,gr);
	}
	else if ((i < ix) && (iy < j))
	{
	    dx = x - cell_edge(i,0,gr);
	    dy = cell_edge(j+1,1,gr) - y;
	}
	else if ((ix < i) && (iy < j))
	{
	    dx = cell_edge(i+1,0,gr) - x;
	    dy = y - cell_edge(j,1,gr);
	}
	else if ((ix < i) && (j < iy))
	{
	    dx = cell_edge(i+1,0,gr) - x;
	    dy = cell_edge(j+1,1,gr) - y;
	}
	else if ((i == ix) && (j < iy))
	{
	    dx0 = x - cell_edge(i,0,gr);
	    dx1 = cell_edge(i+1,0,gr) - x;
	    dx = max(dx0,dx1);
	    dy = y - cell_edge(j,1,gr);
	}
	else if ((i == ix) && (iy < j))
	{
	    dx0 = x - cell_edge(i,0,gr);
	    dx1 = cell_edge(i+1,0,gr) - x;
	    dx = max(dx0,dx1);
	    dy = cell_edge(j+1,1,gr) - y;
	}
	else if ((i < ix) && (j == iy))
	{
	    dx = x - cell_edge(i,0,gr);
	    dy0 = y - cell_edge(j,1,gr);
	    dy1 = cell_edge(j+1,1,gr) - y;
	    dy = max(dy0,dy1);
	}
	else if ((ix < i) && (j == iy))
	{
	    dx = cell_edge(i+1,0,gr) - x;
	    dy0 = y - cell_edge(j,1,gr);
	    dy1 = cell_edge(j+1,1,gr) - y;
	    dy = max(dy0,dy1);
	}
	else
	{
	    *ix1 = max(0,ix-1);
	    *ix2 = min(ix_max,ix+1);
	    *iy1 = max(0,iy-1);
	    *iy2 = min(iy_max,iy+1);
	    return YES;
	}
	k = (int) ceil(hypot(dx/gr->h[0],dy/gr->h[1]));
	*ix1 = max(0,ix-k);
	*ix2 = min(ix_max,ix+k);
	*iy1 = max(0,iy-k);
	*iy2 = min(iy_max,iy+k);
	return YES;
}		/*end find_on_front_block_range2d*/

LOCAL	boolean on_front_block2d(
	int             i,
	int             j,
	const COMPONENT *eq_comps,
	int             n_eq,
	INTERFACE       *intfc,
	USE_BOUNDARIES  bdry,
	HYPER_SURF      *hs)
{
    	BOND  **b;
	CURVE **c;
    	int k;
	struct Table *T = intfc->table;

	if (T->compon2d[j][i] != ONFRONT)
	    return NO;
	b = T->bonds[j][i];  c = T->curves[j][i];
	for (k = 0; k < T->num_of_bonds[j][i]; ++k, ++b, ++c)
	{
	    if (( hs && (Curve_of_hs(hs) == *c)) ||
		(!hs && comp_is_on_curve(*c,eq_comps,n_eq) &&
		 !skip_boundary_hs(Hyper_surf(*c),bdry)))
	        return YES;
	}
	return NO;
}		/*end on_front_block2d*/ 

/*
*			nearest_similar_interface_point2d():
*
*	Given a point x,y and  COMPONENTs l_comp,r_comp locates the closest
*	point px,py of the INTERFACE on a curve with same COMPONENTs.
*	(and same sense)
*	Also returns the bond and curve containing p, and the parametric
*	location of the point on this bond.
*	Returns value 1 or 0 if succesful or not in finding a
*	closest point.
*
*	This routine is O(1) if called on an ONFRONT gridblock,
*	but is otherwise O(N).
*
*	If hs is not NULL then component and boundary are ignored,
*	and instead the routine finds the closest point along Curve_of_hs(hs).
*/

LIB_LOCAL boolean nearest_similar_interface_point2d(
	double		*coords,
	COMPONENT	l_comp,
	COMPONENT	r_comp,
	INTERFACE	*intfc,
	USE_BOUNDARIES	bdry,
	HYPER_SURF	*hs,
	double		*ans,
	double		*t,
	HYPER_SURF_ELEMENT **phse,
	HYPER_SURF	**phs)
{
	FT_POINT    *p;
	BOND     **b;
	CURVE    **c;
	SIDE	 side;	       /* Side of interface bond -
				* either NEGATIVE_SIDE(=left)
				* or POSITIVE_SIDE(=right)
				*/
	double    distance;      /* Distance from (x,y) to a Bond */
	double    min_distance;  /* Distance to Nearest Bond */
	double    norm_dist;     /* Normal Distance to a Bond */
	double    min_norm_dist; /* Normal Distance to the Nearest Bond*/
	double    t_closest;     /* Parametric coord of closest point. */
	int      ix,iy;           /* Grid square containing x,y */
	int      icoords[MAXD];
	int      ix1,ix2, iy1,iy2;
	int       k;
	struct Table    *T = intfc->table;

	min_norm_dist = HUGE_VAL; /* For lint */

	if (intfc->modified || T->new_grid)
	{
	    if (make_bond_comp_lists(intfc) == FUNCTION_FAILED)
	    {
	        (void) printf("WARNING in nearest_similar_interface_point2d(), "
	                      "make_bond_comp_lists failed\n");
	        return NO;
	    }
	}

	            /* Test for Off Front */
	if ((rect_in_which(coords,icoords,&T->rect_grid) == FUNCTION_FAILED) ||
	    (T->compon2d[icoords[1]][icoords[0]] != ONFRONT))
	{
	    return long_nearest_similar_interface_point2d(coords,l_comp,r_comp,
							  intfc,bdry,hs,ans,t,
							  phse,phs);
	}
	ix = icoords[0];    iy = icoords[1];

	            /* On Front */

	        /* Find Closest Point on Front */

	p_closest = NULL;               b_closest = NULL;
	min_distance = HUGE_VAL;

	ix1 = ix-1;    if (ix1 < 0)                ix1 = ix;
	ix2 = ix+1;    if (ix2 == T->rect_grid.gmax[0])    ix2 = ix;
	iy1 = iy-1;    if (iy1 < 0)                iy1 = iy;
	iy2 = iy+1;    if (iy2 == T->rect_grid.gmax[1])    iy2 = iy;

	for (ix = ix1;  ix <= ix2;  ++ix)
	{
	    for (iy = iy1;  iy <= iy2;  ++iy)
	    {
	        if (T->compon2d[iy][ix] != ONFRONT)
	            continue;

	        b = T->bonds[iy][ix];    c = T->curves[iy][ix];

	        for (k = 0;  k < T->num_of_bonds[iy][ix];  ++k,++b,++c)
	        {
	            if (hs)
	            {
	                if (Curve_of_hs(hs) != *c)
	                    continue;
	            }
	            else
	            { 
			if (!comps_are_on_curve(*c,l_comp,r_comp))
	                    continue;
		        if (skip_boundary_hs(Hyper_surf(*c),bdry))
	                    continue;
	            }
	            distance = i_sd2d(coords[0],coords[1],*b,&p,&norm_dist,
				      &side);

	            if ((p != NULL) && (p == p_closest))
	            {

	                /*
	                 * Whenever p != NULL the point x,y is beyond the end
	                 * of the bond.   Thus this is the case where x,y is
	                 * beyond the end of two adjacent bonds, with their
	                 * intersection being the closest point to x,y.
	                 *
	                 * The side is in general ambiguous.
	                 *
	                 * The correct bond wrt which the side should be com-
	                 * puted is that which has the larger normal distance
	                 * from x,y.
	                 *
	                 * The following code assumes that both bonds belong
	                 * to the same curve.
	                 */


	                /* min_nor_dist has value when used */
	                if (norm_dist >= min_norm_dist) 
	                {
	                    t_closest = t_last;
	                    b_closest = *b;
	                    c_closest = *c;  
	                }
	            }
	            else if (distance < min_distance)
	            {
	                min_distance = distance;
	                min_norm_dist = norm_dist;
	                t_closest = t_last;
	                p_closest = p;
	                b_closest = *b;
	                c_closest = *c;
	            }
	        } 
	    }
	}

	if (b_closest == NULL)
	{
	    if (debugging("siti"))
	    {
	       (void) printf("nearest_similar_interface_point2d "
	                     "b_closest == NULL\ncalling "
			     "long_nearest_similar_interface_point2d()\n");
	    }
	    return long_nearest_similar_interface_point2d(coords,l_comp,r_comp,
	                                                  intfc,bdry,hs,ans,t,
							  phse,phs);
	}

	/*
	 * Possibly correct side, b_closest, c_closest when
	 * p_closest might be a NODE.
	 * DO NOT correct if hs is non-NULL since
	 * c_closest may be set != Curve_of_hs(hs) by this call.
	 */
	side = check_closest_point_is_node(coords,side,hs,NO_COMP,
					   l_comp,r_comp,bdry,intfc);
	*phse  = Hyper_surf_element(b_closest);
	*phs  = Hyper_surf(c_closest);
	ans[0] = (1. - t_closest)*Coords(b_closest->start)[0] + 
	           t_closest *Coords(b_closest->end)[0];
	ans[1] = (1. - t_closest)*Coords(b_closest->start)[1] + 
	           t_closest *Coords(b_closest->end)[1];
	*t  = t_closest;

	return YES;
}	    /*end nearest_similar_interface_point2d*/





/*
*			long_component2d():
*
*	Determines the topological COMPONENT relative to a given
*	INTERFACE containing a point x,y.
*
*	This version is much less efficient for many calls on
*	the same unmodified INTERFACE than is function component().
*	For a small number of calls it is much more efficient.
*
*
*	Differs from  component()  in that the local bond/curve lists
*	are not constructed.   A loop over all BONDS of INTERFACE
*	is performed to determine the closest one and appropriate
*	side.   The corresponding CURVE COMPONENT is then returned.
*	Thus the cost of the function is proportional to the total
*	number of BONDS on the INTERFACE.
*
*	Note:  Distances below are actually squared distances.
*/

LIB_LOCAL COMPONENT long_component2d(
	double		*coords,
	INTERFACE	*intfc)
{
	FT_POINT    *p;
	BOND     *b;
	CURVE    **c;
	SIDE	side;	       /* Side of interface bond -
				* either NEGATIVE_SIDE(=left)
				* or POSITIVE_SIDE(=right)
				*/
	SIDE     side_closest;  /* Side of closest bond  */
	double    distance;      /* Distance from (x,y) to a Bond */
	double    min_distance;  /* Distance to Nearest Bond */
	double    norm_dist;     /* Normal Distance to a Bond */
	double    min_norm_dist; /* Normal Distance to the Nearest Bond*/
	double    x, y;
	double    dsx, dsy, dex, dey, dbx, dby, ds2, de2;
	double    l, sx, sy, ex, ey, spe, sps, vp;

	        /* Find Closest Point on Front: */
	p_closest = NULL;
	min_distance = HUGE_VAL;
	min_norm_dist = HUGE_VAL; /* For lint */
	x = coords[0];
	y = coords[1];
	for (c = intfc->curves; c && *c; ++c)
	{
	    ex = Coords((*c)->first->start)[0];
	    dex = ex - x;
	    ey = Coords((*c)->first->start)[1];
	    dey = ey - y;
	    de2 = dex*dex + dey*dey;
	    for (b = (*c)->first; b ; b = b->next)
	    {
		if ((l = bond_length(b)) == 0)
		    continue;

		sx = ex;
		dsx = dex;
		sy = ey;
		dsy = dey;
		ds2 = de2;

	        ex = Coords(b->end)[0];
		dex = ex - x;
	        ey = Coords(b->end)[1];
		dey = ey - y;
	        de2 = dex*dex + dey*dey;

		dbx = ex - sx;
		dby = ey - sy;

		sps = dbx*dsx + dby*dsy;
		spe = dbx*dex + dby*dey;

		if ((ds2 > min_distance) && (de2 > min_distance))
		{
		    if ((sps > 0.0) || (0.0 > spe))
			continue;
		    vp = dsx*dey-dsy*dex; 
		    if ((norm_dist = vp*vp/(l*l)) > min_distance)
			continue;
	            side = (vp >= 0.0) ? NEGATIVE_SIDE : POSITIVE_SIDE;
		    p = NULL;
		    distance = norm_dist;
		}
		else
		{     
		    vp = dsx*dey-dsy*dex; 
	            side = (vp >= 0.0) ? NEGATIVE_SIDE : POSITIVE_SIDE;
		    norm_dist = vp*vp/(l*l);
		    if (sps >= 0.0)
		    {
			p = b->start;
			distance = ds2;
		    }
		    else if (spe <= 0.0)
		    {
			p = b->end;
			distance = de2;
		    }
		    else
		    {
		        p = NULL;
	                distance = norm_dist;
		    }
		}
	        if ((p != NULL) && (p == p_closest))
	        {
	
	             /*                                                      *
	             *                                                       *
	             *   Whenever p != NULL the point x,y is beyond the end  *
	             *   of the bond.   Thus this is the case where x,y is   *
	             *   beyond the end of two adjacent bonds, with their    *
	             *   intersection being the closest point to x,y.        *
	             *                                                       *
	             *   The side is in general ambiguous.                   *
	             *                                                       *
	             *   The correct bond wrt which the side should be com-  *
	             *   puted is that which has the larger normal distance  *
	             *   from x,y.                                           *
	             *                                                       *
	             *   The following code assumes that both bonds belong   *
	             *   to the same curve.                                  *
	             *                                                       */

	
	                /* min_nor.. has value when used */
	            if (norm_dist >= min_norm_dist) 
	            {
	                side_closest = side;
	                b_closest = b;
	                c_closest = *c;
	            }
	        }

	        else if (distance < min_distance)
	        {
	            min_distance = distance;
	            min_norm_dist = norm_dist;
	            side_closest = side;
	            p_closest = p;
	            b_closest = b;
	            c_closest = *c;
	        }
	    }
	}
	if (p_closest != NULL)
	{
	    side_closest = convex_side(p_closest,b_closest,c_closest,
	    		side_closest);
	}

	/*
	 * Possibly correct side_closest, b_closest, c_closest
	 * when p_closest is a POINT (and so might be the position of a NODE).
	 */
	side_closest = check_closest_point_is_node(coords,side_closest,NULL,
					 	   NO_COMP,NO_COMP,NO_COMP,
						   INCLUDE_BOUNDARIES,intfc);
	return (side_closest == NEGATIVE_SIDE) ? negative_component(c_closest) :
	                                         positive_component(c_closest);
}	    /*end long_component2d*/


/*
*			long_nearest_interface_point2d():
*
*	Given a point x,y and a COMPONENT comp, locates the closest
*	point px,py of the INTERFACE which borders COMPONENT comp.
*	Also returns the bond and curve containing p, and the paramatric
*	location of the point on this bond.
*	Returns value 1 or 0 if succesful or not in finding a
*	closest point.
*
*	If hs is not NULL then component and boundary are ignored,
*	and instead the routine finds the closest point along Curve_of_hs(hs.
*/



LIB_LOCAL boolean long_nearest_interface_point2d(
	double		*coords,
	COMPONENT	comp,
	INTERFACE	*intfc,
	USE_BOUNDARIES	bdry,
	HYPER_SURF	*hs,
	double		*ans,
	double		*t,
	HYPER_SURF_ELEMENT **phse,
	HYPER_SURF	**phs)
{
    	const COMPONENT *eq_comps;
	int             n_eq;
	FT_POINT    *p;
	BOND     *b;
	CURVE    **c;
	SIDE	 side;	       /* Side of interface bond -
				* either NEGATIVE_SIDE(=left)
				* or POSITIVE_SIDE(=right)
				*/
	double    distance;      /* Distance from (x,y) to a Bond */
	double    min_distance;  /* Distance to Nearest Bond */
	double    norm_dist;     /* Normal Distance to a Bond */
	double    min_norm_dist; /* Normal Distance to the Nearest Bond*/
	double    t_closest;
	double    x, y;
	double    dsx, dsy, dex, dey, dbx, dby, ds2, de2;
	double    l, sx, sy, ex, ey, spe, sps, vp;

	        /* Find Closest Point on Front: */
	(void) printf("Warning: calling long_nearest_interface_point2d()\n");
	p_closest = NULL;       c_closest = NULL;       b_closest = NULL;
	min_distance = HUGE_VAL;
	t_last = 0.0;
	min_norm_dist = HUGE_VAL; /* For lint */
	b_closest = NULL;
	x = coords[0];	y = coords[1];
    	eq_comps = equivalent_components_list(comp,&n_eq,intfc);
	for (c = intfc->curves; c && *c; ++c)
	{
	    if (hs)
	    {
	        if (Curve_of_hs(hs) != *c) continue;
	        /* Note: In this case for statement is redundant
	        *  but it does provide a check that 
	        *  curve cc is on interface intfc
	        */
	    }
	    else
	    {
		if (!comp_is_on_curve(*c,eq_comps,n_eq))
	            continue;
		if (skip_boundary_hs(Hyper_surf(*c),bdry))
	            continue;
	    }
	    ex = Coords((*c)->first->start)[0];
	    dex = ex - x;
	    ey = Coords((*c)->first->start)[1];
	    dey = ey - y;
	    de2 = dex*dex + dey*dey;
	    for (b = (*c)->first; b ; b = b->next)
	    {
		if ((l = bond_length(b)) == 0)
		    continue;

		sx = ex;
		dsx = dex;
		sy = ey;
		dsy = dey;
		ds2 = de2;

	        ex = Coords(b->end)[0];
		dex = ex - x;
	        ey = Coords(b->end)[1];
		dey = ey - y;
	        de2 = dex*dex + dey*dey;

		dbx = ex - sx;
		dby = ey - sy;

		sps = dbx*dsx + dby*dsy;
		spe = dbx*dex + dby*dey;

		if ((ds2 > min_distance) && (de2 > min_distance))
		{
		    if ((sps > 0.0) || (0.0 > spe))
			continue;
		    vp = dsx*dey-dsy*dex; 
		    if ((norm_dist = vp*vp/(l*l)) > min_distance)
			continue;
	            side = (vp >= 0.0) ? NEGATIVE_SIDE : POSITIVE_SIDE;
		    p = NULL;
		    distance = norm_dist;
		    t_last = -sps/(l*l);
		}
		else
		{     
		    vp = dsx*dey-dsy*dex; 
	            side = (vp >= 0.0) ? NEGATIVE_SIDE : POSITIVE_SIDE;
		    norm_dist = vp*vp/(l*l);
		    if (sps >= 0.0)
		    {
			p = b->start;
			distance = ds2;
	                t_last = 0.0;
		    }
		    else if (spe <= 0.0)
		    {
			p = b->end;
			distance = de2;
	                t_last = 1.0;
		    }
		    else
		    {
		        p = NULL;
	                distance = norm_dist;
		        t_last = -sps/(l*l);
		    }
		}

	        if ((p != NULL) && (p == p_closest))
	        {
	
	            /*                                                    *
	            *                                                     *
	            * Whenever p != NULL coords is beyond the end of      *
	            * the bond.   Thus this is the case where x,y is      *
	            * beyond the end of two adjacent bonds, with their    *
	            * intersection being the closest point to coords.     *
	            *                                                     *
	            * The side is in general ambiguous.                   *
	            *                                                     *
	            * The correct bond wrt which the side should be com-  *
	            * puted is that which has the larger normal distance  *
	            * from coords.                                        *
	            *                                                     *
	            * The following code assumes that both bonds belong   *
	            * to the same curve.                                  *
	            *                                                     */

	            /* min_nor.. has value when used */
	            if (norm_dist >= min_norm_dist)
	            {
	                t_closest = t_last;
	                b_closest = b;
	                c_closest = *c;
	            }
	        }

	        else if (distance < min_distance || c_closest == NULL)
	        {
	            min_distance = distance;
	            min_norm_dist = norm_dist;
	            t_closest = t_last;
	            p_closest = p;
	            b_closest = b;
	            c_closest = *c;
	        }
	    }
	}

	if (b_closest == NULL)
	{
	    static int first = YES;

	    (void) printf("WARNING in long_nearest_interface_point2d(), "
	                  "b_closest == NULL\n"
	                  "coords = (%g, %g), ",coords[0],coords[1]);
	    (void) printf("comp = %d, bdry = %d, ",comp,bdry);
	    (void) printf("hs = %llu\n",hypersurface_number(hs));
	    (void) printf("Topological grid of interface\n");
	    print_rectangular_grid(&topological_grid(intfc));
	    if (first == YES)
	    {
	        (void) printf("Interface into "
	                      "long_nearest_interface_point2d()\n");
	        print_interface(intfc);
	    }
	    first = NO;
	    return NO;
	}

	/*
	 * Possibly correct side, b_closest, c_closest when
	 * p_closest might be a NODE.
	 * DO NOT correct if hs is non-NULL since
	 * c_closest may be set != Curve_of_hs(hs) by this call.
	 */
	side = check_closest_point_is_node(coords,side,hs,comp,
					   NO_COMP,NO_COMP,bdry,intfc);
	*phse = Hyper_surf_element(b_closest);
	*phs = Hyper_surf(c_closest);

	ans[0] = (1. - t_closest)*Coords(b_closest->start)[0] +
	           t_closest *Coords(b_closest->end)[0];
	ans[1] = (1. - t_closest)*Coords(b_closest->start)[1] +
	           t_closest *Coords(b_closest->end)[1];
	*t = t_closest;
	return YES;
}	    /*end long_nearest_interface_point2d*/

/*
*			long_nearest_similar_interface_point2d():
*
*	Given a point x,y and COMPONENTs  l_comp,r_comp locates the closest
*	point px,py of the INTERFACE  and curve with same COMPONENTs .
*       (and same sense)
*	Also returns the bond and curve containing p, and the paramatric
*	location of the point on this bond.
*	Returns value 1 or 0 if succesful or not in finding a
*	closest point.
*
*	If hs is not NULL then component and boundary are ignored,
*	and instead the routine finds the closest point along Curve_of_hs(hs).
*/

LIB_LOCAL boolean long_nearest_similar_interface_point2d(
	double		*coords,
	COMPONENT	l_comp,
	COMPONENT	r_comp,
	INTERFACE	*intfc,
	USE_BOUNDARIES	bdry,
	HYPER_SURF	*hs,
	double		*ans,
	double		*t,
	HYPER_SURF_ELEMENT **phse,
	HYPER_SURF	**phs)
{
	FT_POINT    *p;
	BOND     *b;
	CURVE    **c;
	SIDE	side;	       /* Side of interface bond -
				* either NEGATIVE_SIDE(=left)
				* or POSITIVE_SIDE(=right)
				*/
	double    distance;      /* Distance from (x,y) to a Bond */
	double    min_distance;  /* Distance to Nearest Bond */
	double    norm_dist;     /* Normal Distance to a Bond */
	double    min_norm_dist; /* Normal Distance to the Nearest Bond*/
	double    t_closest;

	        /* Find Closest Point on Front: */
	p_closest = NULL;
	min_distance = HUGE_VAL;
	min_norm_dist = HUGE_VAL; /* For lint */
	b_closest = NULL;
	c_closest = NULL;
	for (c = intfc->curves; c && *c; ++c)
	{
	    if (hs)
	    {
	        if (Curve_of_hs(hs) != *c) continue;
	        /* Note: In this case for statement is redundant
	        *    but it does provide a check that 
	        *    curve cc is on interface intfc
	        */
	    }
	    else
	    {
		if (!comps_are_on_curve(*c,l_comp,r_comp))
	            continue;
		if (skip_boundary_hs(Hyper_surf(*c),bdry))
	            continue;
	    }
	    for (b = (*c)->first; b ; b = b->next)
	    {
	        distance = i_sd2d(coords[0],coords[1],b,&p,&norm_dist,&side);

	        if ((p != NULL) && (p == p_closest))
	        {
	
	         /*
	          * Whenever p != NULL the point x,y is beyond the end
	          * of the bond.   Thus this is the case where x,y is
	          * beyond the end of two adjacent bonds, with their
	          * intersection being the closest point to x,y.
	          *
	          * The side is in general ambiguous.
	          *
	          * The correct bond wrt which the side should be com-
	          * puted is that which has the larger normal distance
	          * from x,y.
	          *
	          * The following code assumes that both bonds belong
	          * to the same curve.
	          */

	                /* min_nor.. has value when used */
	            if (norm_dist >= min_norm_dist)    
	            {
	                t_closest = t_last;
	                b_closest = b;
	                c_closest = *c;
	            }
	        }

	        else if (distance < min_distance)
	        {
	            min_distance = distance;
	            min_norm_dist = norm_dist;
	            t_closest = t_last;
	            p_closest = p;
	            b_closest = b;
	            c_closest = *c;
	        }
	    }
	}

	if (b_closest == NULL)
	    return NO;

	/*
	 * Possibly correct side, b_closest, c_closest if p_closest is a NODE.
	 */
	side = check_closest_point_is_node(coords,side,hs,NO_COMP,
					   l_comp,r_comp,bdry,intfc);
	*phse = Hyper_surf_element(b_closest);
	*phs = Hyper_surf(c_closest);

	ans[0] = (1. - t_closest)*Coords(b_closest->start)[0] + 
	           t_closest *Coords(b_closest->end)[0];
	ans[1] = (1. - t_closest)*Coords(b_closest->start)[1] + 
	           t_closest *Coords(b_closest->end)[1];
	*t = t_closest;
	return YES;
}	    /*end long_nearest_similar_interface_point2d*/


/*
*			i_sd2d():
*			shortest_distance2d():
*
*	Computes the Shortest Distance Squared From point (x,y) to the inter-
*	face BOND b.    
*
*	Determines on which side of BOND b the point (x,y) lies.  
*
*	Also computes the normal distance squared, the distance to the 
*	infinite line through the bond.
*
*	In case the BOND has zero length, distance is set equal to HUGE_VAL.
*
*	The paramater  t  below is used to find the parametric 
*	equation of the interface BOND starting at POINT b->start. 
*
*	i_sd2d() is the local interface to this function,  shortest_distance2d()
*	is the global interface.  The allows for possible inlining of the
*	local version i_sd2d().
*/

LOCAL	double	i_sd2d(
	double	x,
	double  y,
	BOND	*b,
	FT_POINT	**p,
	double	*norm_dist,
	SIDE	*side)
{
    	FT_POINT		*ps = b->start, *pe = b->end;
	double           *crds_s = Coords(ps); 
	double           *crds_e = Coords(pe); 
	double		x1 = crds_s[0];
	double		y1 = crds_s[1];
	double		x2 = crds_e[0];
	double		y2 = crds_e[1];
	double		y21 = y2 - y1;
	double		x21 = x2 - x1;
	double		x_1 = x - x1;
	double		y_1 = y - y1;
	double		scalar_prod, vector_prod;
	double		l;	/* squared length of bond */
	double		distance;

		/* Compute Vector Product to get Side: */
	
	vector_prod = y_1*x21 - x_1*y21;
	scalar_prod = x21*x_1 + y21*y_1;
	l = x21*x21 + y21*y21;
	*side =  (vector_prod >= 0.0) ? NEGATIVE_SIDE : POSITIVE_SIDE;
	if (l == 0.0)
	{
	    *p = NULL;
	    t_last = HUGE_VAL;
	    *norm_dist = HUGE_VAL;
	    distance = HUGE_VAL;
	}
	else
	{
	    distance = sqr(vector_prod)/l;
	    *norm_dist = (double)distance;

	    if (scalar_prod >= l)
	    {
	        *p = pe;
	        t_last = 1.0;
	        distance = sqr(x - x2) + sqr(y - y2);
	    }
	    else if (scalar_prod <= 0.0)
	    {
	        *p = ps;
	        t_last = 0.0;
	        distance = sqr(x_1) + sqr(y_1);
	    }
	    else
	    {
	        *p = NULL;
	        t_last = scalar_prod/l;
	    }
	}
	return distance;
}		/*end i_sd2d*/

EXPORT double shortest_distance2d(
	double		*coords,
	BOND		*b,
	FT_POINT		**p,
	double		*norm_dist,
	SIDE		*side)
{
    	return i_sd2d(coords[0],coords[1],b,p,norm_dist,side);
}		/*end shortest_distance2d*/


/*
*			check_closest_point_is_node():
*
*	Check for and deal with the possibility that p_closest (a FILE local
*	variable set by component2d(), nearest_interface_point2d(), ...)  is
*	actually the position of a NODE.  In this case, we assume p_closest
*	has been set correctly by this fcn's caller.  In particular, we check
*	that p_closest is a NODE of c_closest as set by this fcn's caller
*	(c_closest and b_closest are also FILE local).
*
*	If (l_comp != NO_COMP) or (r_comp is != NO_COMP) or
*       (bdry != NO_BOUNDARIES) then various CURVES at the NODE will be
*	ignored in a manner consistent with this fcn's caller.
*
*	If, after rejecting CURVES based on these conditions there are less
*	than 3 CURVES remaining, this fcn makes exits and returns sideClosest,
*	c_closest b_closest unmodified.
*
*	OTHERWISE, reset the closest CURVE and BOND based on minimum
*	ANGULAR distance to the vector from NODE to the arg coords and
*	return sideClosest.
*
*	NOTE1: this fcn should be called only AFTER p_closest has been
*	determined to be a POINT following a BOND-by-BOND loop over the
*	entire INTERFACE.
*
*	NOTE2: this fcn MUST NOT BE CALLED from other fcns in this file which
*	have a HYPER_SURF* parameter with non-NULL value.
*	REASON: this fcn will likely modify c_closest which should NOT happen
*	in this case.
*/

LOCAL	SIDE	check_closest_point_is_node(
	double		*coords,
	SIDE		sideClosest,
	HYPER_SURF	*hs,
	COMPONENT	comp,
	COMPONENT	l_comp,
	COMPONENT	r_comp,
	USE_BOUNDARIES	bdry,
	INTERFACE       *intfc)
{
    	const COMPONENT *eq_comps;
	int             n_eq;
	NODE		*node;
	O_NODE		*oNode;
	double		ang;
	int		num_c;
	int     	i, j;
#if defined(DEBUG_COMP2D)
	boolean		reset = NO;
#endif /* defined(DEBUG_COMP2D) */


	if (
	    (hs != NULL) ||
	    (p_closest == NULL) ||
	    is_closed_curve(c_closest) ||
	    (  (p_closest!=c_closest->start->posn) &&
	       (p_closest!=c_closest->end->posn)  )
	)
	    return sideClosest;
    	eq_comps = equivalent_components_list(comp,&n_eq,intfc);

#if defined(DEBUG_COMP2D)
	debug_print("ccpn","Entered check_closest_point_is_node()\n");
#endif /* defined(DEBUG_COMP2D) */

	/*
	 *    If p_closest is the position of a node then resolution of
	 *    the component requires that we identify the angular sector
	 *    that (x,y) lies in with respect to the curves at the node.
         */

	node = (p_closest==c_closest->start->posn) ?
			    c_closest->start : c_closest->end;

	/*
         * Make the O_NODE corresponding to node.
         * make_onode() allocates to INTERFACE Table
         * so no need to free() in this fcn.
         */
	oNode = make_onode(node);
	ang = angle(coords[0] - Coords(p_closest)[0],
		    coords[1] - Coords(p_closest)[1]);

#if defined(DEBUG_COMP2D)
	if (debugging("ccpn"))
	{
	    (void) printf("Closest point to position (%g %g) is a node\n",
			  coords[0],coords[1]);
	    (void) printf("node\n");		print_node(node);
	    (void) printf("b_closest\n");	print_bond(b_closest);
	    (void) printf("c_closest\n");	print_curve(c_closest);
	    if (comp == NO_COMP)  (void) printf("comp = NO_COMP, ");
	    else                  (void) printf("comp = %d, ",comp);
	    if (l_comp == NO_COMP) (void) printf("l_comp = NO_COMP, ");
	    else                  (void) printf("l_comp = %d, ",l_comp);
	    if (r_comp == NO_COMP) (void) printf("r_comp = NO_COMP\n");
	    else                  (void) printf("r_comp = %d\n",r_comp);
	    (void) printf("bdry = %d\n",bdry);
	    print_angle("ang = ",ang,"\n");
	    print_onode(oNode);
	}
#endif /* defined(DEBUG_COMP2D) */

	num_c = oNode->num_c;
	for (i = 0; i < num_c; ++i)
	{
	    if ((!comp_is_on_curve(oNode->nc[i],eq_comps,n_eq)) ||
	        (!comps_are_on_curve(oNode->nc[i],l_comp,r_comp)) ||
	        (skip_boundary_hs(Hyper_surf(oNode->nc[i]),bdry)))
		continue;
	    if (ang <= oNode->ang[i])
	        break;
	}
	if (i == num_c)
	{
	    for (i = 0; i < num_c; ++i)
	    {
		if ((comp_is_on_curve(oNode->nc[i],eq_comps,n_eq)) &&
		    (comps_are_on_curve(oNode->nc[i],l_comp,r_comp)) &&
		    (!skip_boundary_hs(Hyper_surf(oNode->nc[i]),bdry)))
		    break;
	    }
	    if (i == num_c) /*This shouldn't happen since c_closest
			      satisfies the component and boundary
			      compatibility relations.*/
	    {
#if defined(DEBUG_COMP2D)
	        if (debugging("ccpn"))
	        {
		    (void) printf("check_closest_point_is_node took no action "
		                  "because\n\t");
		    if (!comp_is_on_curve(oNode->nc[i],eq_comps,n_eq))
		        (void) printf("comp %d is not on curve %llu\n",
				      comp,curve_number(oNode->nc[i]));
		    if (!comps_are_on_curve(oNode->nc[i],l_comp,r_comp))
		        (void) printf("l_comp %d and r_comp %d "
				      "are not on curve %llu\n",
				      l_comp,r_comp,curve_number(oNode->nc[i]));
		    if (skip_boundary_hs(Hyper_surf(oNode->nc[i]),bdry))
		        (void) printf("it skipped a boundary curve\n");
	        }
	        debug_print("ccpn","Left check_closest_point_is_node()\n");
#endif /* defined(DEBUG_COMP2D) */
	        return sideClosest;
	    }
	}
	for (j = (i - 1 + num_c)%num_c; j != i; j = (j - 1 + num_c)%num_c)
	{
	    if (comp_is_on_curve(oNode->nc[j],eq_comps,n_eq) &&
	        comps_are_on_curve(oNode->nc[j],l_comp,r_comp) &&
	        (!skip_boundary_hs(Hyper_surf(oNode->nc[j]),bdry)))
	        break;
	}
	if (normalized_angle(oNode->ang[i] - ang) <
				normalized_angle(ang - oNode->ang[j]))
	{
	    if (c_closest != oNode->nc[i])
	    {
	        c_closest = oNode->nc[i];
	        if (oNode->orient[i] == POSITIVE_ORIENTATION)
	        {
	            b_closest = oNode->nc[i]->first;
	            sideClosest = POSITIVE_SIDE;
	        }
	        else
	        {
	            b_closest = oNode->nc[i]->last;
	            sideClosest = NEGATIVE_SIDE;
	        }
#if defined(DEBUG_COMP2D)
		reset = YES;
#endif /* defined(DEBUG_COMP2D) */
	    }
	}
	else
	{
	    if (c_closest != oNode->nc[j])
	    {
	        c_closest = oNode->nc[j];
	        if (oNode->orient[j] == POSITIVE_ORIENTATION)
	        {
	            b_closest = oNode->nc[j]->first;
		    sideClosest = NEGATIVE_SIDE;
	        }
	        else
	        {
		    b_closest = oNode->nc[j]->last;
		    sideClosest = POSITIVE_SIDE;
	        }
#if defined(DEBUG_COMP2D)
		reset = YES;
#endif /* defined(DEBUG_COMP2D) */
	    }
	}
#if defined(DEBUG_COMP2D)
	if (debugging("ccpn"))
	{
	    if (reset == YES)
	    {
	        (void) printf("Closest point to position (%g %g) was reset\n",
			      coords[0],coords[1]);
	        (void) printf("b_closest\n");	print_bond(b_closest);
	        (void) printf("c_closest\n");	print_curve(c_closest);
	    }
	    else
	    {
		(void) printf("check_closest_point_is_node took no action "
			      "because\n\t");
		(void) printf("input c_closeset was already correct\n");
	    }
	}
	debug_print("ccpn","Left check_closest_point_is_node()\n");
#endif /* defined(DEBUG_COMP2D) */
	return sideClosest;
}		/*end check_closest_point_is_node*/




/*
*			make_bond_comp_lists():
*
*	This function determines the local component and bond list arrays
*	compon2d[][], num_of_bonds[][] and bonds[][] for an INTERFACE
*	relative to a grid. 
*
*	Returns 1 if successful or 0 on error (insufficient space).
*/


EXPORT boolean make_bond_comp_lists(
	INTERFACE	*intfc)
{

	if (DEBUG) (void) printf("\n\nEntered make_bond_comp_lists()\n");

	if (no_topology_lists(intfc) == YES)
	{
	    screen("ERROR in make_bond_comp_lists(), "
		   "illegal attempt to construct interface topology\n"
		   "no_topology_lists(intfc) == YES\n");
	    clean_up(ERROR);
	}

	start_clock("make_bond_comp_lists");
	if (make_bond_lists(intfc) == FUNCTION_FAILED)
	{
	    (void) printf("WARNING in make_bond_comp_lists(), "
	                  "make_bond_lists() failed\n");
	    return FUNCTION_FAILED;
	}

	stop_clock("make_bond_comp_lists");

	intfc->modified = NO;
	intfc->table->new_grid = NO;

	if (DEBUG) (void) printf("Leaving make_bond_comp_lists()\n\n");
	return FUNCTION_SUCCEEDED;
}		/*end make_bond_comp_lists*/










/*                            make_bond_lists():
*
*	Computes the number of BONDS num_of_bonds[iy][ix] of INTERFACE
*	traversing each grid block of RECT_GRID, and constructs the
*	lists bonds[iy][ix] and curves[iy][ix] of traversing BONDS 
*	and CURVES for each grid block:
*	
*	Initializes  compon2d[iy][ix]  to ONFRONT for blocks traversed
*	by INTERFACE and to the component value elsewhere.
*
*
*	Returns 1 if successful or 0 on error (if insufficient
*	storage is available, or there is a problem in rect_in_which()).
*
*
*				Details:
*
*	This is a three-pass construction over the INTERFACE with an
*	intermediate pass over the RECT_GRID.   
*
*	On the first pass, the array bond_blocks is filled in along with 
*	the number of bonds in each mesh-square num_of_bonds[][].  This
*	is accomplished by calling the routine blocks_on_bond()
*	consecutively on each BOND of INTERFACE.
*
*	bond_blocks will contain for each BOND of INTERFACE the list 
*	of (ix,iy) integer pairs for all grid blocks of grid which are
*	traversed by BOND.   The end of the integer-pair list for a
*	BOND is marked with the integer  END_BOND.
*
*	The second pass, set_bond_list_pointers(), initializes the
*	pointers bonds[iy][ix] to point to the correct offsets in
*	the array bondstore[] using the previously constructed
*	num_of_bonds[][] array.   Similarly for  curves[iy][ix].
*
*	The third pass , fill_bond_and_curve_lists(), is a loop over the
*	BONDS of INTERFACE.   For each BOND, the grid square information
*	previously stored in bond_blocks is transferred to the bonds[][]
*	array by adding BOND to bonds[iy][ix] for each (ix,iy) pair
*	in the bond_block list for that BOND.   Similarly for curves[][].
*	
*	Storage Requirements:
*
*	Three arrays of unknown size must be allocated here: bondstore,
*	curvestore and bond_blocks.
*
*		Lemma 1:   size(bond_blocks) <= 4*L/h + 3*num_points
*
*	where L is length of INTERFACE, num_points is number of POINTS
*	on INTERFACE and  h = min(grid->h[0],grid->h[1]).
*
*		Lemma 2:   size(bondstore) = sum of num_of_bonds[iy][ix]
*
*	summed over all grid blocks iy,ix.
*
*	Proofs:
*		A straight-line BOND of length l cannot traverse more
*	than  2*l/h + 2  grid blocks since it cannot cross more than l/h+ 1
*	grid-lines either horizontally or vertically.   Summing over all
*	BONDS we arrive at 2*L/h + num_of_bonds  as an upper bound on the
*	number of grid blocks traversed by INTERFACE.   Since each
*	block requires storage of two integers and the end of each
*	BOND requires storage of one integer (END_BOND) to mark it,
*	an upper bound on required integers is:
*
*		size(bond_blocks) <= 2(2*L/h + num_of_bonds) + num_of_bonds;
*
*	Finally we note that  num_of_bonds <= num_points.
*
*		Lemma 2 is obvious.   Fortunately num_of_bonds[][] are computed
*	in pass 1, while bondstore is not needed until pass 3 at which point
*	we know how large it should be.   It is difficult to give a good
*	a priori estimate for bondstore.
*
*	Remark:
*		For interfaces with grid spacings of different orders of
*	magnitude, the estimate for size(bond_blocks) above is too high.
*	A corollary to the proof of Lemma 1, shows that a better upper
*	bound is given by
*
*		Lemma 1':	size(bond_blocks) <=
*			2*(sum over bonds (lbx/hx + lby/hy)) + 7*num_points
*
*	where lbx and lby are the x and y components of the direction
*	uni_array corresponding to bond b.
*/


enum { END_BOND = -1 }; /* Any integer that cannot be a grid block index */

LOCAL int total_num_of_bonds;	/* Computes sum of num_of_bonds[iy][ix] */

LOCAL int *Bond_blocks,		/* Temporary Array */	
	  *bond_blocks;		/* Points to temporary array */

LOCAL boolean make_bond_lists(
	INTERFACE	*intfc)
{
	boolean		status;
	int		ix,iy;
	CURVE		*c;
	BOND		*b;
	struct Table	*T;
	RECT_GRID	*grid;
#if defined(OLD_BOND_BLOCKS)
	double		Length;
#else /* defined(OLD_BOND_BLOCKS) */
	double		hx, hy, nlx, nly;
#endif /* defined(OLD_BOND_BLOCKS) */

	if (DEBUG) (void) printf("Entered make_bond_lists()\n");
	if ((T = table_of_interface(intfc)) == NULL)
	{
	    (void) printf("WARNING in make_bond_lists(), "
	                  "table_of_interface = NULL\n");
	    return FUNCTION_FAILED;
	}
	grid = &T->rect_grid;

			/* Free old storage */

	if (T->num_of_bonds != NULL)
	    free(T->num_of_bonds);
	if (T->bondstore != NULL)
	    free(T->bondstore);
	if (T->curvestore != NULL)
	    free(T->curvestore);
	if (T->compon2d != NULL)
	    free(T->compon2d);
	if (T->bonds != NULL)
	    free(T->bonds);
	if (T->curves != NULL)
	   free(T->curves);

			/* Create a Grid if Needed: */

	if (!T->fixed_grid)
	    set_topological_grid(intfc,(RECT_GRID *)NULL);


		/* Allocate New num_of_bonds[][], compon2d[][]: */
	
	bi_array(&T->num_of_bonds,grid->gmax[1],grid->gmax[0],INT);
	if (T->num_of_bonds == NULL)
	{
	    (void) printf("WARNING in make_bond_lists(), "
	                  "can not allocate T->num_of_bonds\n");
	    return FUNCTION_FAILED;
	}
	/* NOTE:
	 * matrix returns data initialized to 0, so 
	 * T->num_of_bonds[j][i] = 0 for all i, j initially
	 */
	bi_array(&T->compon2d,grid->gmax[1],grid->gmax[0],sizeof(COMPONENT));
	if (T->compon2d == NULL)
	{
	    (void) printf("WARNING in make_bond_lists(), matrix failed\n"
	                  "can not allocate T->compon2d\n");
	    return FUNCTION_FAILED;
	}
	if (DEBUG)
	    (void) printf("T->num_of_bonds, T->compon2d allocated\n");
	for (iy = 0; iy < grid->gmax[1]; ++iy)
	for (ix = 0; ix < grid->gmax[0]; ++ix)
	    T->compon2d[iy][ix] = NO_COMP;


		/* Find Length of and Allocate Bond_blocks array: */

#if defined(OLD_BOND_BLOCKS)
						/* Find intfc length: */
	Length = 0.0;
	(void) next_bond(intfc,NULL,NULL);
	while (next_bond(intfc,&b,&c))
	    Length += bond_length(b);

	max_size = 3*intfc->num_points +
			(int)(4*Length/min(grid->h[0],grid->h[1]));
#else /* defined(OLD_BOND_BLOCKS) */
	hx = grid->h[0];	hy = grid->h[1];
	max_size = 0;
	(void) next_bond(intfc,NULL,NULL);
	while (next_bond(intfc,&b,&c))
	{
	    nlx = fabs(Coords(b->end)[0] - Coords(b->start)[0])/hx;
	    nly = fabs(Coords(b->end)[1] - Coords(b->start)[1])/hy;
	    max_size += (int) (nlx + nly);
	}
	max_size = 7*intfc->num_points + 2*max_size;
#endif /* defined(OLD_BOND_BLOCKS) */

							/* See Lemma 1 */
	uni_array(&Bond_blocks,max_size,INT); 
	if (Bond_blocks == NULL)  
	{
	    (void) printf("WARNING in make_bond_lists(), "
	                  "can not allocate Bond_blocks\n");
	    return FUNCTION_FAILED;
	}
	bond_blocks = Bond_blocks;

		/* Fill the num_of_bonds[][] and Bond_square arrays: */

	total_num_of_bonds = 0;
	count = 0;
	(void) next_bond(intfc,NULL,NULL);
	while (next_bond(intfc,&b,&c))
	{
	    if (blocks_on_bond(b,c,T->num_of_bonds,T->compon2d,grid) ==
		FUNCTION_FAILED)
	    {
		(void) printf("WARNING in make_bond_lists(), "
		              "blocks_on_bond() failed\n");
		return FUNCTION_FAILED;
	    }
        }

			/* Assign bonds[][] and curves[][]: */
	status=set_bond_and_curve_list_pointers(T,grid->gmax[0],grid->gmax[1]);
	if (status == FUNCTION_SUCCEEDED)
	{

			/* Copy in the grid blocks from Bond_blocks: */
	    fill_bond_and_curve_lists(T->num_of_bonds,T->bonds,T->curves,intfc);

				/* Set the compon2d[][] array: */

	    set_off_front_comp2d(T->compon2d,intfc);
	}

	free(Bond_blocks);

	if (DEBUG)
	{
	    show_COMP_2d(stdout,intfc);
	    show_BOND_list(intfc);
	    (void) printf("Leaving make_bond_lists()\n\n");
	}
	return status;
}		/*end make_bond_lists*/









/*
*			set_bond_and_curve_list_pointers():
*
*	Does what its name implies! Read documentation above for the
*	function make_bond_list().
*	Returns 1 if successful, or 0 if unable to allocate enough
*	space.
*/

LOCAL boolean set_bond_and_curve_list_pointers(
	struct Table	*T,
	int		xmax,
	int		ymax)
{
	int		ix,iy;
	BOND		**lastb;
	CURVE		**lastc;

	if (DEBUG)
	    (void) printf("\n\nEntered set_bond_and_curve_list_pointers()\n");

			/* Allocate bonds and curves arrays: */

	bi_array(&T->bonds,ymax,xmax,sizeof(BOND **));
	if (T->bonds == NULL)
	{
	    (void) printf("WARNING in set_bond_and_curve_list_pointers(), ");
	    (void) printf("can not allocate T->bonds\n");
	    return FUNCTION_FAILED;
	}
					
	bi_array(&T->curves,ymax,xmax,sizeof(CURVE **));
	if (T->curves == NULL)
	{
	    (void) printf("WARNING in set_bond_and_curve_list_pointers(), ");
	    (void) printf("can not allocate T->curves\n");
	    return FUNCTION_FAILED;
	}

			/* Allocate bondstore, curvestore arrays: */

	uni_array(&T->bondstore,total_num_of_bonds, sizeof(BOND *));
	if (T->bondstore == NULL)
	{
	    (void) printf("WARNING in set_bond_and_curve_list_pointers(), ");
	    (void) printf("can not allocate T->bondstore\n");
	    return FUNCTION_FAILED;
	}


	uni_array(&T->curvestore,total_num_of_bonds, sizeof(CURVE *));
	if (T->curvestore == NULL)
	{
	    (void) printf("WARNING in set_bond_and_curve_list_pointers(), ");
	    (void) printf("can not allocate T->curvestore\n");
	    return FUNCTION_FAILED;
	}

	lastb = T->bondstore;
	lastc = T->curvestore;
	for (iy = 0; iy < ymax; ++iy)
		for (ix = 0; ix < xmax; ++ix)
		{
			T->bonds[iy][ix] = lastb;
			T->curves[iy][ix] = lastc;
			lastb += T->num_of_bonds[iy][ix];
			lastc += T->num_of_bonds[iy][ix];
			T->num_of_bonds[iy][ix] = 0;
		}

	if (DEBUG)
		(void) printf("Leaving set_bond_and_curve_list_pointers()\n\n");
	return FUNCTION_SUCCEEDED;
}		/*end set_bond_and_curve_list_pointers*/









/*                       
*				fill_bond_and_curve_lists():
*
*	Does what its name implies!  Read the documentation above 
*	for function  make_bond_lists().
*/

LOCAL void fill_bond_and_curve_lists(
	int		**num_of_bonds,
	BOND		****bonds,
	CURVE		****curves,
	INTERFACE	*intfc)
{
	int		ix,iy;
	BOND		*b;
	CURVE		*c;

	if (DEBUG) (void) printf("\n\nEntered fill_bond_and_curve_lists()\n");
	bond_blocks = Bond_blocks;

	(void) 	next_bond(intfc,NULL,NULL);
	while (next_bond(intfc,&b,&c))
	{
	    while (*bond_blocks != END_BOND)
	    {
	        ix = *(bond_blocks++);
	        iy = *(bond_blocks++);
	        bonds[iy][ix][num_of_bonds[iy][ix]] = b;
	        curves[iy][ix][num_of_bonds[iy][ix]++] = c;
	    }
	    ++bond_blocks;	/* Skip END_BOND */
	}
	if (DEBUG) (void) printf("Leaving fill_bond_and_curve_lists()\n\n");
}		/*end fill_bond_and_curve_lists*/










/*                         blocks_on_bond():
*
*	Locates all blocks on BOND b of the interface and updates the array
*	num_of_bonds.  The component comp[][] is updated with respect to
*	ONFRONT values only. A bond in block [iy][ix] is noted in bond_blocks.
*
*	Returns 0 on error (problems in rect_in_which()) and 1 otherwise.
*/
		

LOCAL boolean blocks_on_bond(
	BOND		*b,
	CURVE		*c,
	int		**num_of_bonds,
	COMPONENT	**comp,
	RECT_GRID	*grid)
{
	int		ix1,iy1,ix,iy;
	int		icoords1[MAXD],icoords[MAXD];
	double		n[MAXD];
	double		t, tmp1, tmp2;
	double		x1, y1, coords1[MAXD];
	double		x2 = Coords(b->end)[0], y2 = Coords(b->end)[1];
	double		x,y;		/* a generic point on the bond */
	double		coords[MAXD];
	double		yintercept, slope;
	double		length,maxlength;
	int		*gmax = grid->gmax;

	if (DEBUG)
	    (void) printf("Entered blocks_on_bond()\n");

	coords1[0] = Coords(b->start)[0];     coords1[1] = Coords(b->start)[1];
	if (rect_in_which(coords1,icoords1,grid) == FUNCTION_FAILED)
	{
	    n[0] = Coords(b->end)[0] - Coords(b->start)[0];
	    n[1] = Coords(b->end)[1] - Coords(b->start)[1];

	    if (!intersect_ray_with_boundary(Coords(b->start),n,
				                grid->L,grid->U,coords1,2))
	    {
	    	mark_end_of_bond();
	    	return FUNCTION_SUCCEEDED;
	    }
	    if (rect_in_which(coords1,icoords1,grid) == FUNCTION_FAILED)
	    {
	    	(void) printf("WARNING in blocks_on_bond(),  "
	    	              "coords not on grid\n");
	    	(void) printf("b %llu c %llu comp %p\n",
	    	              bond_number(b,c->interface),curve_number(c),comp);
	    	(void) printf("b->start <%g, %g> b->end <%g, %g>\n",
	    		      coords1[0],coords1[1],x2,y2);
	    	if (debugging("blocks_on_bond"))
	    	    print_interface(c->interface);
	    	return FUNCTION_FAILED;
	    }
	}
	x1  = coords1[0];	y1  = coords1[1];
	ix1 = icoords1[0];	iy1 = icoords1[1];

	if (ix1 >= 0 && ix1 < gmax[0] && iy1 >= 0 && iy1 < gmax[1])
	    store_bond(ix1,iy1,num_of_bonds,comp);
	tmp1 = fabs((x2-x1)/(grid->h[0]));
	tmp2 = fabs((y2-y1)/(grid->h[1]));
	length = max(tmp1,tmp2);
	maxlength = 0.999;

	begin:

	if (length > maxlength)
	{
	    t = maxlength/length;
	    x = x1 + t*(x2-x1);	y = y1 + t*(y2-y1);
	}
	else
	{
	    x = x2;			y = y2;
	}

	coords[0] = x; coords[1] = y;
	if (rect_in_which(coords,icoords,grid) == FUNCTION_FAILED)
	{
	    n[0] = Coords(b->end)[0] - Coords(b->start)[0];
	    n[1] = Coords(b->end)[1] - Coords(b->start)[1];

	    coords1[0] = x1;	coords1[1] = y1;
	    if (!intersect_ray_with_boundary(coords1,n,grid->L,grid->U,
						coords,2))
	    {
	    	mark_end_of_bond();
	    	return FUNCTION_SUCCEEDED;
	    }
	    if (rect_in_which(coords,icoords,grid) == FUNCTION_FAILED)
	    {
	    	(void) printf("WARNING in blocks_on_bond(),  "
	    	              "coords not on grid\n");
	    	(void) printf("b %llu c %llu comp %p x %g y %g\n",
			       bond_number(b,c->interface),
			       curve_number(c),comp,x,y);
	    	(void) printf("x1 %g y1 %g x2 %g y2 %g\n",x1,y1,x2,y2);
	    	if (debugging("blocks_on_bond"))
	    	    print_interface(c->interface);
		return FUNCTION_FAILED;
	    }
		x = x2 = coords[0];	y = y2 = coords[1];
		tmp1 = fabs((x2-x1)/(grid->h[0]));
		tmp2 = fabs((y2-y1)/(grid->h[1]));
		length = max(tmp1,tmp2);
	}
	ix = icoords[0];	iy = icoords[1];
	if (DEBUG)
	    (void) printf("rect_in_which(%g,%g,%d,%d)\n",x,y,ix,iy);

					/* x,y in Another Square: */
	if (((ix1 != ix) || (iy1 != iy)) &&
	    ix >= 0 && ix < gmax[0] && iy >= 0 && iy < gmax[1])
	    store_bond(ix,iy,num_of_bonds,comp);
	

					/* Diagonally Adjacent Squares: */
	if ((ix1 != ix) && (iy1 != iy))
	{
	    slope = (y-y1)/(x-x1);	/* Slope line x1,y1 -> x,y */
	    yintercept = y1 + slope*(cell_edge(max(ix1,ix),0,grid) - x1);

	/*								*
	*	yintercept is the y coordinate at which the line from	*
	*	x1,y1 to x,y crosses a side of element containing	*
	*	x,y.							*
	*							       */

	    if (iy > iy1) 		/* Element  Above Element 1: */
	    {
	    	if (yintercept > cell_edge(iy,1,grid) &&
	    	    ix1 >= 0 && ix1 < gmax[0] && iy >= 0 && iy < gmax[1])
	    	    store_bond(ix1,iy,num_of_bonds,comp);
	    	else if (ix >= 0 && ix < gmax[0] && iy1 >= 0 && iy1 < gmax[1])
	    	    store_bond(ix,iy1,num_of_bonds,comp);
	    }

	    else 				/* Element  Below Element 1: */
	    {
	    	if (yintercept < cell_edge(iy1,1,grid) &&
	    	    ix1 >= 0 && ix1 < gmax[0] && iy >= 0 && iy < gmax[1])
	    	    store_bond(ix1,iy,num_of_bonds,comp);
	    	else if (ix >= 0 && ix < gmax[0] && iy1 >= 0 && iy1 < gmax[1])
	    	    store_bond(ix,iy1,num_of_bonds,comp);
	    }
	}
	if (length > maxlength)
	{
	    ix1 = ix;
	    x1  = x;
	    iy1 = iy;
	    y1  = y;
	    length -= maxlength;
	    goto begin;
	}
	mark_end_of_bond();

	return FUNCTION_SUCCEEDED;
}		/*end blocks_on_bond*/


 






/*
*				store_bond():
*
*	Adds a new ix,iy block entry to the bond_blocks array.
*	Also increments num_of_bonds[iy][ix] and sets compon[iy][ix]
*	to ONFRONT.
*/


LOCAL void store_bond(
	int		ix,
	int		iy,
	int		**num_of_bonds,
	COMPONENT	**compon)
{
	if (count >= max_size-2)
	{
	    int i,*Tmp_bond_blocks;
	    uni_array(&Tmp_bond_blocks,max_size*2,INT); 
	    for (i = 0; i < max_size; ++i)
		Tmp_bond_blocks[i] = Bond_blocks[i];
	    max_size *= 2;
	    free(Bond_blocks);
	    Bond_blocks = Tmp_bond_blocks;
	    bond_blocks = Bond_blocks + count;
	}
	*(bond_blocks++) = ix;
	*(bond_blocks++) = iy;
	count += 2;
	total_num_of_bonds++;
	num_of_bonds[iy][ix]++;
	compon[iy][ix] = ONFRONT;
}		/*end store_bond*/









/*
*				mark_end_of_bond():
*
*	Marks the end of the grid blocks for a BOND in the 
*	bond_blocks array with a marker (the negative integer
*	END_BOND).   This allows the bond_blocks array to be
*	reprocessed later BOND by BOND.
*/

LOCAL void mark_end_of_bond(void)
{
	if (count >= max_size-1)
	{
	    int i,*Tmp_bond_blocks;
	    uni_array(&Tmp_bond_blocks,max_size*2,INT); 
	    for (i = 0; i < max_size; ++i)
		Tmp_bond_blocks[i] = Bond_blocks[i];
	    max_size *= 2;
	    free(Bond_blocks);
	    Bond_blocks = Tmp_bond_blocks;
	    bond_blocks = Bond_blocks + count;
	}
	*(bond_blocks++) = END_BOND;
	count++;
}		/*end mark_end_of_bond*/








LOCAL void set_off_front_comp2d(
	COMPONENT	**compon,
	INTERFACE	*intfc)
{
	int          ix,iy;
	register int xmax, ymax;
	RECT_GRID    *grid = &topological_grid(intfc);
	COMPONENT    c;

	if (compon == NULL)
	    return;

	if (DEBUG)
	    (void) printf("Entered set_off_front_comp2d()\n");
	xmax = grid->gmax[0];
	ymax = grid->gmax[1];


	--xmax; --ymax;
	for (iy = 0; iy <= ymax; ++iy)
	{
	    for (ix = 0; ix <= xmax; ++ix)
	    {
	    	if (compon[iy][ix] == ONFRONT)
	    	    continue;
	    	else if (compon[iy][ix] == NO_COMP)
	    	{
	    	    compon[iy][ix] = find_compon2d(ix,iy,intfc);
	    	}
	    	c = compon[iy][ix];
	    	if ((iy < ymax) && (compon[iy+1][ix] == NO_COMP))
	    	    compon[iy+1][ix] = c;
	    	if ((iy == 0) && (ix < xmax) && (compon[iy][ix+1] == NO_COMP))
	    	    compon[iy][ix+1] = c;
	    	if ((ix<xmax) && (iy<ymax) && (compon[iy+1][ix+1]==NO_COMP))
	    	    compon[iy+1][ix+1] = c;
	    }
	}

	if (DEBUG) (void) printf("Leaving set_off_front_comp2d()\n");
}		/*end set_off_front_comp2d*/

LOCAL	COMPONENT	find_compon2d(
	int       ix,
	int       iy,
	INTERFACE *intfc)
{
    	BOND         **b;
	CURVE        **c;
	FT_POINT	     *p;
	SIDE	     side;	    /* Side of interface bond - 
				     * either NEGATIVE_SIDE(=left)
				     * or POSITIVE_SIDE(=right)
				     */
	SIDE	     side_closest;  /* Side of closest bond  */
	RECT_GRID    *grid = &topological_grid(intfc);
	double	     norm_dist;     /* Normal Distance to a Bond */
	double	     min_norm_dist; /* Normal Distance to the Nearest Bond*/
	double	     min_distance;  /* Distance to Nearest Bond */
	double	     distance;      /* Distance from (x,y) to a Bond */
	struct Table *T = intfc->table;
	double        x, y, coords[3];
	int          i, j, k, ix1, ix2, iy1, iy2;

	coords[0] = x = cell_center(ix,0,grid);
	coords[1] = y = cell_center(iy,1,grid);
	if (!find_on_front_block_range2d(x,y,ix,iy,NULL,0,intfc,
		                         INCLUDE_BOUNDARIES,NULL,
			                 &ix1,&ix2,&iy1,&iy2,-1))
	    return long_component2d(coords,intfc);
			/* Find Closest Point on Front: */

	p_closest = NULL;
	b_closest = NULL;
	c_closest = NULL;
	min_distance = HUGE_VAL;

	for (j = iy1; j <= iy2; ++j)
	{
	    for (i = ix1; i <= ix2; ++i)
	    {
	        if (T->compon2d[j][i] != ONFRONT)
		    continue;

	        b = T->bonds[j][i];
	        c = T->curves[j][i];

	        for (k=0; k<T->num_of_bonds[j][i]; ++k,++b,++c)
	        {
	            distance = i_sd2d(x,y,*b,&p,&norm_dist,&side);

	            if ((p != NULL) && (p == p_closest))
	            {

	            /*							    *
	            *							    *
	            *   Whenever p != NULL the point x,y is beyond the end  *
	            *   of the bond.   Thus this is the case where x,y is   *
	            *   beyond the end of two adjacent bonds, with their    *
	            *   intersection being the closest point to x,y.        *
	            *							    *
	            *   The side is in general ambiguous.		    *
	            *							    *
	            *   The correct bond wrt which the side should be com-  *
	            *   puted is that which has the larger normal distance  *
	            *   from x,y.					    *
	            *							    *
	            *   The following code assumes that both bonds belong   *
	            *   to the same curve except if the point x,y is	    *
	            *   essentially the intersection point.		    *
	            *							    */

	    	    	/* min_nor.. has value when used */
	    	        if (norm_dist >= min_norm_dist) 
	    	        {
	    	            side_closest = side;
	    	            b_closest = *b;
	    	            c_closest = *c;  
	    	        }
	            }
	            else if (distance < min_distance)
	            {
	    	        min_distance = distance;
	    	        min_norm_dist = norm_dist;
	    	        side_closest = side;
	    	        p_closest = p;
	    	        b_closest = *b;
	    	        c_closest = *c;
	            }
	        }
	    }
	}
	if (p_closest != NULL)
	{
	    side_closest = convex_side(p_closest,b_closest,c_closest,
	    		side_closest);
	}

	/*
	 * Possibly correct side_closest, b_closest, c_closest when p_closest
	 * is a FT_POINT (and so might be the position of a NODE).
	 */
	side_closest = check_closest_point_is_node(coords,side_closest,NULL,
						   NO_COMP,NO_COMP,NO_COMP,
						   INCLUDE_BOUNDARIES,intfc);
	return (side_closest == NEGATIVE_SIDE) ? negative_component(c_closest) :
	                                         positive_component(c_closest);
}		/*end find_compon2d*/


/*
*				show_COMP_2d():
*
*	Shows the COMPONENT values for Grid block of an INTERFACE.
*	The ONFRONT blocks are indicated by the word ON.
*/



LIB_LOCAL void show_COMP_2d(
	FILE		*file,
	INTERFACE	*intfc)
{
	int		ix,iy;
	int		iymax, ixmax;

	ixmax = topological_grid(intfc).gmax[0];
	iymax = topological_grid(intfc).gmax[1];
	(void) fprintf(file,"\n\nCOMPONENTS by Grid Block for INTERFACE %llu:\n",
		       interface_number(intfc));
	for (iy = iymax - 1; iy >= 0; --iy)
	{
	    for (ix = 0; ix < ixmax; ++ix)
	    {
	    	if (intfc->table->compon2d[iy][ix] == ONFRONT)
	    	    (void) fprintf(file,"ON ");
	    	else
	    	    (void) fprintf(file,"%2d ",intfc->table->compon2d[iy][ix]);
	    }
	    (void) fprintf(file,"\n");
	}
	(void) fprintf(file,"\n\n");
}		/*end show_COMP_2d*/


/*                    
*				show_BOND_list():
*
*		Displays the bondlists of an INTERFACE:
*/

LOCAL void show_BOND_list(
	INTERFACE	*intfc)
{
	int		ix,iy,i;
	int		ixmax, iymax;
	BOND		*b;

	ixmax = topological_grid(intfc).gmax[0];
	iymax = topological_grid(intfc).gmax[1];
	(void) printf("\n\nBond Numbers for INTERFACE %llu:\n",
	       interface_number(intfc));
	for (iy = iymax - 1; iy >= 0; --iy)
	{
		for (ix = 0; ix < ixmax; ++ix)
			(void) printf("%d ",intfc->table->num_of_bonds[iy][ix]);
		(void) printf("\n");
	}
	(void) printf("\n\n");

	(void) printf("\n\nBondlists for INTERFACE %llu\n",interface_number(intfc));
	for (iy = iymax - 1; iy>=0;  --iy) 
	{
		for (ix = 0; ix < ixmax; ++ix)
		{
			if (intfc->table->num_of_bonds[iy][ix] == 0) continue;
			(void) printf("Block %2d %2d, num_of_bonds = %2d:",
			       iy,ix,intfc->table->num_of_bonds[iy][ix]);
			for (i=0; i<intfc->table->num_of_bonds[iy][ix]; ++i)
			{
				(void) printf("%s",(i%2)?"  ":"\n");
				b = intfc->table->bonds[iy][ix][i];
				(void) printf("%g %g <-> %g %g,",
				   Coords(b->start)[0],Coords(b->start)[1],
				   Coords(b->end)[0],Coords(b->end)[1]);
			}
				     
			(void) printf("\n\n");
		}
	}
	(void) printf("End Bondlists\n\n");
}		/*end show_BOND_list*/


LOCAL SIDE convex_side(
	FT_POINT *p_closest,
	BOND *b_closest,
	CURVE *c_cosest,
	SIDE side)
{
	BOND *b1,*b2;
	double p1[MAXD],p2[MAXD];
	double dpt,cpt;

	if (p_closest == b_closest->end)
	{
	    b1 = b_closest;
	    b2 = b_closest->next;
	    if (b2 == NULL)
	    {
		if (is_closed_curve(c_closest))
		    b2 = c_closest->first;
	    	else /* cannot do any better, return as it */
		    return side;
	    }
	}
	else if (p_closest == b_closest->start)
	{
	    b1 = b_closest->prev;
	    b2 = b_closest;
	    if (b1 == NULL)
	    {
		if (is_closed_curve(c_closest))
		    b1 = c_closest->last;
	    	else /* cannot do any better, return as it */
		    return side;
	    }
	}
	difference(Coords(b1->start),Coords(b1->end),p1,2);
	difference(Coords(b2->end),Coords(b2->start),p2,2);
	dpt = scalar_product(p1,p2,2);
	if(dpt <= 0) return side;

	Cross2d(p1,p2,cpt);
	if (fabs(cpt) == 0.0)
	    return side;
	side = (cpt < 0.0) ? POSITIVE_SIDE : NEGATIVE_SIDE;
	return side;
}	/* end convex_side */

EXPORT	void show_box_comp_crx(
	int *smin,
	int *smax,
	int *gmax,
	COMPONENT *comp,
	int *seg_crx_count)
{
	int ix,iy,l,nc;
	COMPONENT c;

        for (iy = smax[1]; iy >= smin[1]; iy--)
        {
            for (ix = smin[0]; ix <= smax[0]; ix++)
            {
                c = comp[d_index2d(ix,iy,gmax)];
		if (c < 0) c = 0;
		if (ix == smax[0])
		{
		    printf("%d ",c);
		    continue;
		}
		l = seg_index2d(ix,iy,EAST,gmax);
		nc = seg_crx_count[l];
		if (nc > 1)
		    printf("%dm",c);
		else if (nc == 1)
		    printf("%d|",c);
		else
		    printf("%d ",c);
	    }
	    printf("\n");
	    if (iy == smin[1]) continue;
            for (ix = smin[0]; ix <= smax[0]; ix++)
            {
		l = seg_index2d(ix,iy,SOUTH,gmax);
		nc = seg_crx_count[l];
		if (nc > 1)
		    printf("%dm",c);
		else if (nc == 1)
		    printf("%d|",c);
		else
		    printf("%d",c);
	    }
	    printf("\n");
	}
}	/* end show_box_comp_crx */

LIB_LOCAL boolean nearest_interface_point_within_range2d(
	double		   *coords,
	COMPONENT	   comp,
	INTERFACE	   *intfc,
	USE_BOUNDARIES	   bdry,
	HYPER_SURF	   *hs,
	double		   *ans,
	double		   *t,
	HYPER_SURF_ELEMENT **phse,
	HYPER_SURF	   **phs,
	int		   range)
{
    	const COMPONENT *eq_comps;
	int             n_eq;
	FT_POINT        *p;
	BOND         **b;
	CURVE        **c;
	RECT_GRID    *gr;
	SIDE	     side;	    /* Side of interface bond -
				     * either NEGATIVE_SIDE(=left)
				     * or POSITIVE_SIDE(=right)
				     */
	double        distance;      /* Distance from (x,y) to a Bond */
	double        min_distance;  /* Distance to Nearest Bond */
	double        norm_dist;     /* Normal Distance to a Bond */
	double        min_norm_dist; /* Normal Distance to the Nearest Bond */
	double        t_closest;     /* Parametric coord of closest point. */
	double        x, y;
	int          ix, iy;        /* Grid square containing x,y */
	int          icoords[MAXD];
	int          ix1, ix2, iy1, iy2;
	int          ix_max, iy_max;
	int          k;
	struct Table *T = intfc->table;

	if (hs && hs->interface != intfc)
	{
	    screen("ERROR in nearest_interface_point2d()"
		   "hs->interface != intfc\n");
	    clean_up(ERROR);
	}

	min_norm_dist = HUGE_VAL; /* For lint */
	if (intfc->modified || T->new_grid)
	{
	    if (make_bond_comp_lists(intfc) == FUNCTION_FAILED)
	    {
	        static int first = YES;

	        (void) printf("WARNING in nearest_interface_point2d(), "
	                      "make_bond_comp_lists() failed\n");
	        (void) printf("coords = (%g, %g), ",coords[0],coords[1]);
	        (void) printf("comp = %d, bdry = %d, ",comp,bdry);
	        (void) printf("hs = %llu\n",hypersurface_number(hs));
	        (void) printf("Topological grid of interface\n");
	        print_rectangular_grid(&topological_grid(intfc));
	        if (first == YES)
	        {
	            (void) printf("Interface into ");
	            (void) printf("nearest_interface_point2d()\n");
	            print_interface(intfc);
	        }
	        first = NO;
	        return NO;
	    }
	}
	gr = &topological_grid(intfc);
    	eq_comps = equivalent_components_list(comp,&n_eq,intfc);

	 x = coords[0];    y = coords[1];
	p_closest = NULL;    c_closest = NULL;       b_closest = NULL;
	min_distance = HUGE_VAL;
	t_last = 0.0;

	if (!rect_in_which(coords,icoords,gr))
	        return NO;
	else if (T->compon2d[icoords[1]][icoords[0]] != ONFRONT) /* Off Front */
	{
	    ix = icoords[0];  iy = icoords[1];
	    if (!find_on_front_block_range2d(x,y,ix,iy,eq_comps,n_eq,intfc,
			             bdry,hs,&ix1,&ix2,&iy1,&iy2,range))
	        return NO;
	}
	else	/* On Front: */
	{
	    ix = icoords[0];  iy = icoords[1];

	    /* Find Closest Point on Front: */

	    /*   setting of returned argument values to NULL is a good  */
	    /* idea even though a success status value is also returned */

	    /* Check center block first for existence of allowed closest point */
	    b = T->bonds[iy][ix];  c = T->curves[iy][ix];
	    for (k = 0; k < T->num_of_bonds[iy][ix]; ++k, ++b, ++c)
	    {
		if (( hs && (Curve_of_hs(hs) == *c)) ||
		    (!hs && comp_is_on_curve(*c,eq_comps,n_eq) &&
		       !skip_boundary_hs(Hyper_surf(*c),bdry)))
		{
	            distance = i_sd2d(x,y,*b,&p,&norm_dist,&side);

	            if ((p != NULL) && (p == p_closest))
	            {

	            /*                                                      *
	            *                                                       *
	            *   Whenever p != NULL the point x,y is beyond the end  *
	            *   of the bond.   Thus this is the case where x,y is   *
	            *   beyond the end of two adjacent bonds, with their    *
	            *   intersection being the closest point to x,y.        *
	            *                                                       *
	            *   The side is in general ambiguous.                   *
	            *                                                       *
	            *   The correct bond wrt which the side should be com-  *
	            *   puted is that which has the larger normal distance  *
	            *   from x,y.                                           *
	            *                                                       *
	            *   The following code assumes that both bonds belong   *
	            *   to the same curve.                                  *
	            *                                                       */

	                /* min_nor.. has value when used */
	                if (norm_dist >= min_norm_dist)
	                {
	                    t_closest = t_last;
	                    b_closest = *b;
	                    c_closest = *c;  
	                }
	            }
	            else if (distance < min_distance)
	            {
	                min_distance = distance;
	                min_norm_dist = norm_dist;
	                t_closest = t_last;
	                p_closest = p;
	                b_closest = *b;
	                c_closest = *c;
	            }
		}
	    } 
	    if (b_closest == NULL)
	    {
	        if (!find_on_front_block_range2d(x,y,ix,iy,eq_comps,n_eq,
			                         intfc,bdry,hs,&ix1,&ix2,
						 &iy1,&iy2,-1))
	            return NO;
	    }
	    else
	    {
	        ix_max = gr->gmax[0]-1;
	        distance = sqrt(min_distance);
	        ix1 = cell_index(x - distance,0,gr);
	        ix1 = max(0,ix1);
	        ix1 = min(ix,ix1);
	        ix2 = cell_index(x + distance,0,gr);
	        ix2 = min(ix_max,ix2);
	        ix2 = max(ix,ix2);

	        iy_max = gr->gmax[1]-1;
	        iy1 = cell_index(y - distance,1,gr);
	        iy1 = max(0,iy1);
	        iy1 = min(iy,iy1);
	        iy2 = cell_index(y + distance,1,gr);
	        iy2 = min(iy_max,iy2);
	        iy2 = max(iy,iy2);
	    }
	}

	for (ix = ix1; ix <= ix2; ++ix)
	for (iy = iy1; iy <= iy2; ++iy)
	{
	    if (((ix == icoords[0]) && (iy == icoords[1])) ||
	        (T->compon2d[iy][ix] != ONFRONT))
	        continue;
	    b = T->bonds[iy][ix];  c = T->curves[iy][ix];
	    for (k = 0; k < T->num_of_bonds[iy][ix]; ++k, ++b, ++c)
	    {
		if (( hs && (Curve_of_hs(hs) == *c)) ||
		    (!hs && comp_is_on_curve(*c,eq_comps,n_eq) &&
		       !skip_boundary_hs(Hyper_surf(*c),bdry)))
		{
	            distance = i_sd2d(x,y,*b,&p,&norm_dist, &side);

	            if ((p != NULL) && (p == p_closest))
	            {

	            /*                                                      *
	            *                                                       *
	            *   Whenever p != NULL the point x,y is beyond the end  *
	            *   of the bond.   Thus this is the case where x,y is   *
	            *   beyond the end of two adjacent bonds, with their    *
	            *   intersection being the closest point to x,y.        *
	            *                                                       *
	            *   The side is in general ambiguous.                   *
	            *                                                       *
	            *   The correct bond wrt which the side should be com-  *
	            *   puted is that which has the larger normal distance  *
	            *   from x,y.                                           *
	            *                                                       *
	            *   The following code assumes that both bonds belong   *
	            *   to the same curve.                                  *
	            *                                                       */

	                /* min_nor.. has value when used */
	                if (norm_dist >= min_norm_dist)
	                {
	                    t_closest = t_last;
	                    b_closest = *b;
	                    c_closest = *c;  
	                }
	            }

	            else if (distance < min_distance)
	            {
	                min_distance = distance;
	                min_norm_dist = norm_dist;
	                t_closest = t_last;
	                p_closest = p;
	                b_closest = *b;
	                c_closest = *c;
	            }
		}
	     } 
	}

	if (b_closest == NULL)
	   return NO;

	/*
	 * Possibly correct side, b_closest, c_closest when
	 * p_closest might be a NODE.
	 * DO NOT correct if hs is non-NULL since
	 * c_closest may be set != Curve_of_hs(hs) by this call.
	 */
	side = check_closest_point_is_node(coords,side,hs,comp,
					   NO_COMP,NO_COMP,bdry,intfc);
	*phse = Hyper_surf_element(b_closest);
	*phs = Hyper_surf(c_closest);
	ans[0] = (1.0 - t_closest)*Coords(b_closest->start)[0] +
	                t_closest *Coords(b_closest->end)[0];
	ans[1] = (1.0 - t_closest)*Coords(b_closest->start)[1] +
	                t_closest *Coords(b_closest->end)[1];
	*t = t_closest;
	return YES;
}	    /*end nearest_interface_point_within_range2d*/

