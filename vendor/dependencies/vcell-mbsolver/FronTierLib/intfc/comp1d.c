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
*				comp1d.c:
*
*
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*		CODE FOR COMPONENT AND POINTS LIST COMPUTATIONS:
*
*	The points list in the interface is assumed linearly ordered from 
*	left to right.
*
*	The components at any point can be obtained from the left/right
*	component informations of the ordered point list.
*
*	The main user routines for COMPONENT computations are:
*	
*		component(x,intfc)
*		double x;
*		INTERFACE *intfc;
*
*	Returns the COMPONENT of x on intfc.
*
*		COMPONENT max_component(intfc)
*		INTERFACE *intfc;
*
*	Returns the maximum COMPONENT value for intfc.
*
*		COMPONENT min_component(intfc)
*		INTERFACE *intfc;
*
*	Returns the lowest COMPONENT value for intfc.
*
*		COMPONENT exterior_component(intfc)
*		INTERFACE *intfc;
*
*	Returns the COMPONENT value of the exterior of intfc.
*
*/

#if defined(ONED)
#include <intfc/iloc.h>

	/* LOCAL Function Declarations */
LOCAL	boolean	comp_is_on_point(FT_POINT*,const COMPONENT*,int);
LOCAL	void	show_Comp_list(INTERFACE*);
LOCAL	void	show_point_list(INTERFACE*);
LOCAL	void	sort_point_list(FT_POINT**,int);
#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */
LOCAL	int	pt_compr(const void *,const void *);
#if defined(__cplusplus)
}
#endif /* defined(__cplusplus) */

LOCAL	boolean	comp_is_on_point(
	FT_POINT           *pt,
	const COMPONENT *comps,
	int             n_eq)
{
	int i;
	COMPONENT pcomp, ncomp;

	if (comps == NULL)
	    return YES;

	pcomp = positive_component(pt);
	ncomp = negative_component(pt);

	for (i = 0; i < n_eq; ++i)
	{
	    if (pcomp == comps[i])
		return YES;
	    else if (ncomp == comps[i])
		return YES;
	}
	return NO;
}		/*end comp_is_on_curve*/



EXPORT boolean make_point_comp_lists(
	INTERFACE	*intfc)
{
	int		ix, ix0, ix1;
	int		zp;
	FT_POINT		**p;
	struct Table	*T;
	COMPONENT	icomp;
        RECT_GRID       *grid;

	if (DEBUG)
	    (void) printf("Entered make_point_comp_lists()\n");

	if ((T = table_of_interface(intfc)) == NULL) 
	{
	    (void) printf("WARNING in make_point_comp_lists(), "
	                  "table_of_interface = NULL\n");
	    return FUNCTION_FAILED;
	}

	if (no_topology_lists(intfc) == YES)
	{
	    screen("ERROR in make_point_comp_lists(), "
		   "illegal attempt to construct interface topology\n"
		   "no_topology_lists(intfc) == YES\n");
	    clean_up(ERROR);
	}

	grid = &T->rect_grid;

			/* Free old storage if necessary */

	if (T->num_of_points != NULL)
	    free(T->num_of_points);
	if (T->pts_in_zone != NULL)
	    free(T->pts_in_zone);
	if (T->compon1d != NULL)
	    free(T->compon1d);

			/* Create a Grid if Needed: */

	if (!T->fixed_grid)
	    set_topological_grid(intfc,(RECT_GRID *)NULL);


			/* Allocate new storage if necessary */

	uni_array(&T->num_of_points,grid->gmax[0],INT);
	if (T->num_of_points == NULL)
	{
	    (void) printf("WARNING in make_point_complist(), "
	                  "can't allocate T->num_of_points\n");
	    return FUNCTION_FAILED;
	}
	if (DEBUG)
	    (void) printf("T->num_of_points allocated\n");

	/* NOTE: vector returns integer values initalized to zero */
	uni_array(&T->compon1d,grid->gmax[0],sizeof(COMPONENT));
	if (T->compon1d == NULL)
	{
	    (void) printf("WARNING in make_point_complist(), "
	                  "can't allocate T->compon1d\n");
	    return FUNCTION_FAILED;
	}
	if (DEBUG)
	    (void) printf("T->compon1d allocated\n");

	for (ix = 0; ix < grid->gmax[0]; ++ix)
	    T->compon1d[ix] = NO_COMP;
	uni_array(&T->pts_in_zone,grid->gmax[0],sizeof(FT_POINT **));
	/* NOTE: vector returns pointer values initalized to NULL */
	if (T->pts_in_zone == NULL)
	{
	    (void) printf("WARNING in make_point_complist(), "
	                  "can't allocate T->pts_in_zone\n");
	    return FUNCTION_FAILED;
	}

	start_clock("make_point_comp_lists");
	if ((intfc->modified)  && (intfc->num_points > 0))
	    sort_point_list(intfc->points,intfc->num_points);

		/* Default setting */
	ix0 = 0;
	if (intfc->num_points != 0)
	{
	    p = intfc->points;
	    icomp = negative_component(*p);
	    ix1 = -1;
	    for (p = intfc->points; p && *p; ++p)
	    {
	    	if (rect_in_which(Coords(*p),&zp,&T->rect_grid) ==
							FUNCTION_FAILED)
	    	    continue;
	    	++T->num_of_points[zp];
	    	if ((p == intfc->points) || (zp != ix1))
	    	{
	    	    T->pts_in_zone[zp] = p;
	    	}

	    	T->compon1d[zp] = ONFRONT;
	    	ix1 = zp;
	    	for (ix = ix0; ix < ix1; ++ix)
	    	    T->compon1d[ix] = icomp;
	    	icomp = positive_component(*p);
	    	ix0 = zp + 1;
	    }
	    for (ix = ix0; ix < grid->gmax[0]; ++ix)
	    	T->compon1d[ix] = icomp;
	}
	stop_clock("make_point_comp_lists");

	intfc->modified = NO;
	intfc->table->new_grid = NO;

	if (DEBUG) (void) printf("Leaving make_point_comp_lists()\n\n");
	return FUNCTION_SUCCEEDED;
}		/*end make_point_comp_lists*/



/*
*				component1d():
*
*	Determines the topological COMPONENT relative to a given
*	INTERFACE which contains the position coords.
*
*	Returns component value if successful, or ERROR on error.
*	This assumes that ERROR is not a possible COMPONENT value.
*	ERROR occurs if enough space cannot be found for the
*	allocation of bond_list arrays.
*/


LIB_LOCAL COMPONENT component1d(
	double		*coords,
	INTERFACE	*intfc)
{
	double		x = coords[0];
	FT_POINT		**p;
	int		ix;	
	int		k;
	struct Table	*T = intfc->table;

	/* Check for no points in the interface (interior only) */
	if (intfc->points == NULL)
	    return MIN_INTERIOR_COMP;

	if (intfc->modified || T->new_grid)
	{
	    if (make_point_comp_lists(intfc) == FUNCTION_FAILED)
	    {
	    	(void) printf("WARNING in component1d(), "
	    	              "make_point_comp_lists failed\n");
	    	return NO_COMP;
	    }
	}

	if (rect_in_which(coords,&ix,&T->rect_grid) == FUNCTION_FAILED)
	    return exterior_component(intfc);	/* Point Outside */


	if (T->compon1d[ix] != ONFRONT) 		/* Off Front */
	    return T->compon1d[ix];
	else						/* On Front: */
	{
	    for (k=0, p=T->pts_in_zone[ix]; k < T->num_of_points[ix]; ++k, ++p)
	    {
	    	if (x <= Coords(*p)[0]) 
	    	    return negative_component(*p);
	    }
	    p--;
	    return positive_component(*p);
	}
}		/*end component1d*/


LIB_LOCAL void show_point_comp_lists(
	INTERFACE	*intfc)
{
	show_Comp_list(intfc);
	show_point_list(intfc);
}		/*end show_point_comp_lists*/

LOCAL void show_Comp_list(
	INTERFACE	*intfc)
{
	int		xmax = intfc->table->rect_grid.gmax[0];
	int		ix;

	(void) printf("\n\n\tHere is the component list of interface %llu:\n\n",
		      interface_number(intfc));
	for (ix = 0; ix < xmax; ++ix)
	    (void) printf("Comp[%d] = %d\n",ix,intfc->table->compon1d[ix]);
}		/*end show_Comp_list*/

LOCAL void show_point_list(
	INTERFACE	*intfc)
{
	struct Table	*T = intfc->table;
	int		xmax = intfc->table->rect_grid.gmax[0];
	int		ix,i;

	(void) printf("\n\n\tHere is the point list of interface %llu\n\n",
		      interface_number(intfc));
	for (ix = 0; ix < xmax; ++ix)
	{
		if (T->num_of_points[ix] == 0) 
		{
		  (void) printf ("ix=%d:\tnum_of_points=0,\tpts_in_zone=NULL\n",
				 ix);
		}
		else
		{
		    (void) printf("ix=%d:\tnum_of_points=%d,\tpts_in_zone=",ix,
				  T->num_of_points[ix]);
		    for(i = 0; i < T->num_of_points[ix]; ++i)
			    (void) printf("  %llu",
					  point_number(T->pts_in_zone[ix][i]));
		    (void) printf("\n");
		}
	}
}		/*end show_point_list*/

/*
*			nearest_interface_point1d():
*
*	Given a point with coordinates coords and a COMPONENT comp,
*	this routine locates the closest point of the INTERFACE which
*	borders COMPONENT comp. The coordinates of this point are ans.
*	Returns value 1 or 0 if successful or not in finding a
*	closest point.
*
*	This routine is O(1) if called on an ONFRONT gridblock,
*	but is otherwise O(N).
*/

LIB_LOCAL boolean nearest_interface_point1d(
        double              *coords, 
        COMPONENT          comp,
        INTERFACE          *intfc, 
        USE_BOUNDARIES     bdry, 
        HYPER_SURF         *hs, 
        double              *ans,
        double              *t,
	HYPER_SURF_ELEMENT **phse, 
        HYPER_SURF         **phs)
{
        const COMPONENT *eq_comps;
	int             n_eq;
	FT_POINT		**p;
	FT_POINT		*p_closest;
	int		ix;		/* Grid square containing x */
	int		icoords[MAXD];
	int		ix1,ix2;
	int		k;
	struct Table	*T = intfc->table;

	if (intfc->modified || T->new_grid)
	{
	    if (make_point_comp_lists(intfc) == FUNCTION_FAILED)
	    {
	        static boolean first = YES;

	        (void) printf("WARNING in nearest_interface_point1d(), "
	                      "make_point_comp_lists() failed\n"
	                      "coords = (%g), comp = %d, bdry = %d, hs = %p\n",
			      coords[0],comp,bdry,hs);
	        (void) printf("Topological grid of interface\n");
	        print_rectangular_grid(&topological_grid(intfc));
	        if (first == YES)
	        {
	    	    (void) printf("Interface into ");
	    	    (void) printf("nearest_interface_point1d()\n");
	    	    print_interface(intfc);
	        }
	        first = NO;
	        return NO;
	    }
	}

	if ((!rect_in_which(coords,icoords,&T->rect_grid)) ||
	    (T->compon1d[icoords[0]] != ONFRONT)) /* Off Front: */
	    return long_nearest_interface_point1d(coords,comp,intfc,
						  bdry,hs,ans,t,phse,phs);

		/* On Front: */

			/* Find Closest Point on Front: */

	eq_comps = equivalent_components_list(comp,&n_eq,intfc);
	if (hs)
	{
	    p_closest = Point_of_hs(hs);
	}
	else
	{
	    double	distance;	/* Distance from (x) to a Point */
	    double	min_distance;	/* Distance to the nearest Point */

	    ix = icoords[0];
	    min_distance = HUGE_VAL;
	    p_closest = NULL;
	    p = T->pts_in_zone[ix];

    	    /* Check center block first for existence of allowed */
    	    /*  closest point				     */

	    for (k = 0; k < T->num_of_points[ix]; ++k,++p)
	    {
	    	if (!comp_is_on_point(*p,eq_comps,n_eq))
		    continue;
		if (skip_boundary_hs(Hyper_surf(*p),bdry))
		    continue;
		distance = sqr(coords[0] - Coords(*p)[0]);

		if (distance < min_distance)
		{
		    min_distance = distance;
		    p_closest = *p;
		}
	    } 
	    if (p_closest == NULL)
	    	return long_nearest_interface_point1d(coords,comp,intfc,bdry,
						      hs,ans,t,phse,phs);

	    ix1 = ix-1;	if (ix1 < 0)			 ix1=ix;
	    ix2 = ix+1;	if (ix2 == T->rect_grid.gmax[0]) ix2 = ix;
	    for(ix = ix1; ix <= ix2; ++ix)
	    {
	    	if ((ix == icoords[0]) || (T->compon1d[ix] != ONFRONT))
		    continue;

	    	p = T->pts_in_zone[ix];
	    	for (k = 0; k < T->num_of_points[ix]; ++k,++p)
	    	{
	    	    if (!comp_is_on_point(*p,eq_comps,n_eq))
	    	    	continue;
		    if (skip_boundary_hs(Hyper_surf(*p),bdry))
			continue;
	    	    distance = sqr(coords[0] - Coords(*p)[0]);

	    	    if (distance < min_distance)
	    	    {
	    	    	min_distance = distance;
	    	    	p_closest = *p;
	    	    }
	    	} 
	    }

	    if (p_closest == NULL)
	    	return long_nearest_interface_point1d(coords,comp,
					intfc,bdry,hs,ans,t,phse,phs);
	}

	*phs = Hyper_surf(p_closest);
	*phse = Hyper_surf_element(p_closest);
	ans[0] = Coords(p_closest)[0];
	t[0] = 0.0;
	return YES;
}		/*end nearest_interface_point1d*/


/*
*			long_nearest_interface_point1d():
*
*	Given a coordinates (coords) and a COMPONENT comp, locates the closest
*	point ans of the INTERFACE which borders COMPONENT comp.
*	Returns value 1 or 0 if succesful or not in finding a
*	closest point.
*/

/*ARGSUSED*/
LIB_LOCAL boolean long_nearest_interface_point1d(
	double		   *coords,
	COMPONENT 	   comp,
	INTERFACE 	   *intfc,
	USE_BOUNDARIES 	   bdry,
	HYPER_SURF 	   *hs,
	double 		   *ans,
	double 		   *t,
	HYPER_SURF_ELEMENT **phse,
	HYPER_SURF 	   **phs)
{
        const COMPONENT *eq_comps;
	int             n_eq;
	FT_POINT **p;
	FT_POINT *p_closest;

			/* Find Closest Point on Front: */

	if (hs)
	{
	    p_closest = Point_of_hs(hs);
	}
	else
	{
	    double   distance;	/* Distance from (x) to a point */
	    double	min_distance;	/* Distance to Nearest point */

	    eq_comps = equivalent_components_list(comp,&n_eq,intfc);
	    min_distance = HUGE_VAL;
	    p_closest = NULL;
	    for (p = intfc->points; p && (*p); ++p)
	    {
	    	if (!comp_is_on_point(*p,eq_comps,n_eq))
		    continue;
		if (skip_boundary_hs(Hyper_surf(*p),bdry))
		    continue;
		distance = sqr(coords[0] - Coords(*p)[0]);

		if (distance < min_distance)
		{
		    min_distance = distance;
		    p_closest = *p;
		}
	    }

	    if (p_closest == NULL)
	    {
		static int first = YES;

		(void) printf("WARNING in long_nearest_interface_point1d(), "
		              "p_closest == NULL\n"
		              "coords = (%g), comp = %d, bdry = %d, "
		              "hs = %p\n",coords[0],comp,bdry,hs);
		(void) printf("Topological grid of interface\n");
		print_rectangular_grid(&topological_grid(intfc));
		if (first == YES)
		{
		    (void) printf("Interface into "
		                  "long_nearest_interface_point1d()\n");
		    print_interface(intfc);
		}
		first = NO;
		return NO;
	    }
	}
	*phs = Hyper_surf(p_closest);
	*phse = Hyper_surf_element(p_closest);

	ans[0] = Coords(p_closest)[0];
	t[0] = 0.0;
	return YES;
}		/*end long_nearest_interface_point1d*/

LIB_LOCAL void show_COMP_1d(
	FILE		*file,
	INTERFACE	*intfc)
{
	int		ix;
	int		ixmax;

	ixmax = topological_grid(intfc).gmax[0];
	(void) fprintf(file,"\n\nCOMPONENTS by Grid Block for INTERFACE %llu:\n",
		       interface_number(intfc));
	for (ix = 0; ix < ixmax; ++ix)
	{
	    if (intfc->table->compon1d[ix] == ONFRONT)
	    	(void) fprintf(file,"ON ");
	    else
	    	(void) fprintf(file,"%2d ",intfc->table->compon1d[ix]);
	    (void) fprintf(file,"\n");
	}
	(void) fprintf(file,"\n\n");
}		/*end show_COMP_1d*/

EXPORT	boolean i_intersections1d(
	INTERFACE	*intfc,
	CROSS		**cross,
	const boolean	bdry)
{
	FT_POINT   **points, **p;
	FT_POINT   **usp;
	CROSS   *cr, Cr;
	int     i, j, k, np;

	points = intfc->points;
	if (points == NULL)
	    return FUNCTION_SUCCEEDED;
	np = intfc->num_points;
	p   = (FT_POINT **)store(np*sizeof(FT_POINT*));
	usp = (FT_POINT **)store(np*sizeof(FT_POINT*));
	for (i = 0; i < np; ++i)
	    usp[i] = p[i] = points[i];

	sort_point_list(p,np);

	Cr.prev = NULL;
	Cr.next = NULL;
	cr = &Cr;
	for (i = 0; i < np; ++i)
	{
	    if (is_bdry(p[i]) && !bdry)
		continue;
	    if (p[i] != usp[i])
	    {
		for (j = i+1; j < np; ++j)
		    if (p[j] == usp[j])
			break;
		cr->next = (CROSS *)store(sizeof(CROSS));
		cr->next->prev = cr;
		cr = cr->next;
		cr->next = NULL;
		cr->pt = (FT_POINT **)store((j-i)*sizeof(FT_POINT*));
		for (cr->npt = 0, k = i; k < j; ++k)
		{
	            if (bdry || !is_bdry(usp[k]))
			cr->pt[cr->npt++] = usp[k];
		}
		i = j-1;
	    }
	}
	*cross = Cr.next;
	if (*cross)
	    (*cross)->prev = NULL;
	return FUNCTION_SUCCEEDED;
}		/*end i_intersections1d*/

EXPORT boolean consistent_components1d(
	INTERFACE *intfc)
{
	FT_POINT **p;
	int   i, np;

	p = intfc->points;
	if (p == NULL)
	    return YES;
	np = intfc->num_points;
	for (i = 1; i < np; ++i)
	{
	    if (positive_component(p[i-1]) != negative_component(p[i]))
		return NO;
	}
	return YES;
}		/*end consistent_components1d */

EXPORT void reset_intfc_components(
	INTERFACE *intfc)
{
	FT_POINT **p;
	int   i, np;

	p = intfc->points;
	if (p == NULL)
	    return;
	np = intfc->num_points;
	for (i = 1; i < np; ++i)
	{
	    if (positive_component(p[i-1]) != negative_component(p[i]))
	    {
                COMPONENT new_comp, ncomp, pcomp;
                ncomp = positive_component(p[i-1]);
                pcomp = negative_component(p[i]);
                negative_component(p[i]) = ncomp;
                set_equivalent_comps(ncomp,pcomp,intfc);
	    }
	}
}		/*end reset_intfc_components */

LIB_LOCAL void	i_print_intersections1d(
	CROSS	  *cross,
	INTERFACE *intfc)
{
	int		i, num;
	CROSS		*cr;


	if (cross == NULL)
	{
	    (void) printf("NO INTERSECTIONS \n");
	    return;
	}

	(void) output();
	(void) printf("INTERSECTIONS:\n\n");
	print_interface(intfc);

	(void) printf("Here are the intersections:\n\n");
	for (num = 1, cr = cross; cr; ++num, cr = cr->next)
	{
	    (void) printf("Intersection %d, ",num);
	    (void) printf("Cross %p    next %p prev %p\n",
			  (POINTER)cr,(POINTER)cr->next,(POINTER)cr->prev);
	    (void) printf("Crossing points\n");
	    (void) printf("cr->npt = %d\n",cr->npt);
	    for (i = 0; i < cr->npt; ++i)
		print_point(cr->pt[i]);
	}
}		/*end i_print_intersections1d*/

LOCAL	void sort_point_list(
	FT_POINT	**points,
	int     num_points)
{
	qsort((POINTER)points,(size_t)num_points,sizeof(FT_POINT*),pt_compr);
}		/*end sort_point_list*/

#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */

LOCAL	int	pt_compr(
	const void *pp1,
	const void *pp2)
{
	FT_POINT	**p1 = (FT_POINT**)pp1;
	FT_POINT	**p2 = (FT_POINT**)pp2;
	if (Coords(*p1)[0] < Coords(*p2)[0])
	    return -1;
	if (Coords(*p1)[0] > Coords(*p2)[0])
	    return 1;
	return 0;
}		/*end pt_compr*/

#if defined(__cplusplus)
}
#endif /* defined(__cplusplus) */
#endif /* defined(ONED) */
