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
*				setb2d.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*/



#if defined(TWOD)
#define DEBUG_STRING    "setb2d"
#include <intfc/iloc.h>


	/* BPOINT is a structure characterizing Boundary Points: */

typedef struct  _BPOINT {
	struct _BPOINT *prev, *next;
	NODE           *node; /* NULL unless point is on an interior curve */
	FT_POINT          *posn; /* Location of the point */
	COMPONENT      comp1; /* Component below point - (below in arclength) */
	COMPONENT      comp2; /* Component above point - (above in arclength) */
	BDRY_SIDE      side;  /* Side containing the bpoint */
	double          dist;  /* Arc length from Reference Point XL,YL */
} BPOINT;


	/* LOCAL Function Declarations */
LOCAL	BDRY_SIDE	boundary_side_of_point(FT_POINT*,double*,double*,double);
LOCAL	BPOINT *node_in_bpoint_list(NODE*,BPOINT*);
LOCAL	BPOINT *set_bpoint(BPOINT*,NODE*,COMPONENT,COMPONENT,
	                   double*,double*,double,double*);
LOCAL	BPOINT *set_bpoint_at_corner(int,BPOINT*,INTERFACE*,
	                             double*,double*,double*,double);
LOCAL	BPOINT *set_bpoint_at_node(BPOINT*,NODE*,double*,double*,double*,double);
LOCAL	void	print_bpoint(BPOINT*);
LOCAL	void	show_bpoints(BPOINT*);
LOCAL	void	sort_bpoint(BPOINT*);

/*
*				i_set_boundary2d():
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



EXPORT boolean i_set_boundary2d(
	INTERFACE	*intfc,
	RECT_GRID	*gr,
	COMPONENT	i_comp,
	double		eps)	/* boundary distance tolerance */
{
	BPOINT		Bpoint, *bpoint;
	CURVE		**c,*curve;
	INTERFACE	*cur_intfc = current_interface();
	NODE		**nds;
	double		*L = gr->VL;
	double		*U = gr->VU;
	double		offset[5];    /* Arclength of corners from L */
	double		D[2];        /* Length of sides of Rectangle */
	COMPONENT	outside;    /* Component Number of Exterior */
	int		i;

	DEBUG_ENTER(i_set_boundary2d)

	    /* Check Validity of Interface and Set Storage Allocation: */

	if (DEBUG)
	{
	    print_general_vector("L = ",L,intfc->dim,"");
	    print_general_vector(", U = ",U,intfc->dim,"");
	    (void) printf(", eps = %g, i_comp = %d\n",eps,i_comp);
	    (void) printf("Input interface into i_set_boundary2d()\n");
	    print_interface(intfc);
	}

	if (exists_interface(intfc) != YES)
	{
	    (void) printf("WARNING in i_set_boundary2d(), "
	                  "interface does not exist\n");
	    DEBUG_LEAVE(i_set_boundary2d)
	    return FUNCTION_FAILED;
	}
	set_current_interface(intfc);

	    /* Get rid of Old Boundary Curves if Any: */

	outside = exterior_component(intfc);

	if (DEBUG)  (void) printf("\nOutside Component = %d\n\n",outside);


	    /* Compute Sides and Arclength Offsets of Corners: */

	for (i = 0; i < 2; i++)
	    D[i] = U[i] - L[i];
	for (offset[0] = 0.0, i = 1; i <= 4; i++)
	    offset[i] = offset[i-1] + D[i%2];

	Bpoint.next = Bpoint.prev = NULL;

	    /* Initialize BPOINT list with the 4 Corners of Domain: */
	bpoint = &Bpoint;
	bpoint = set_bpoint_at_corner(0,bpoint,intfc,L,U,offset,eps);
	bpoint->prev = NULL;
	bpoint = set_bpoint_at_corner(1,bpoint,intfc,L,U,offset,eps);
	bpoint = set_bpoint_at_corner(2,bpoint,intfc,L,U,offset,eps);
	bpoint = set_bpoint_at_corner(3,bpoint,intfc,L,U,offset,eps);

	for (nds = intfc->nodes; nds && *nds; nds++)
	    bpoint = set_bpoint_at_node(bpoint,*nds,L,U,offset,eps);

	if (DEBUG)
	{
	    (void) printf("Bpoint array after adding boundary nodes:\n");
	    show_bpoints(Bpoint.next);
	}

	sort_bpoint(Bpoint.next);

	for (bpoint=Bpoint.next; bpoint != NULL; bpoint=bpoint->next)
	{
	    if (bpoint->node == NULL)
	        bpoint->node = make_node(bpoint->posn);
	    set_is_bdry(bpoint->node);
	}


	    /* Find a BPOINT with components set */
	for (bpoint=Bpoint.next; bpoint != NULL; bpoint=bpoint->next)
	    if (bpoint->comp1 != NO_COMP) break;

	    /* Case where No Curves Touch Bdry */
	if (bpoint == NULL)
	{
	    COMPONENT bdry_nbhr;
	    if( intfc->curves && *intfc->curves )
	    {
	        double bdry_nbhr_coords[MAXD];
	        bdry_nbhr_coords[0] = L[0] + eps;
	        bdry_nbhr_coords[1] = L[1] + eps;
	        bdry_nbhr = long_component(bdry_nbhr_coords,intfc);
	    }
	    else
	        bdry_nbhr = i_comp;

	    for (bpoint=Bpoint.next; bpoint != NULL; bpoint=bpoint->next)
	        bpoint->comp1 = bpoint->comp2 = bdry_nbhr;
	}
	else
	{
	    BPOINT *bp;

	    for (bp = bpoint->prev; bp != NULL; bp = bp->prev)
	    {
	        if (bp->comp2 == NO_COMP)
	        {
	            bp->comp2 = bp->next->comp1;
	            bp->comp1 = bp->comp2;
	        }
	    }
	    for (bp = bpoint->next; bp != NULL; bp = bp->next)
	    {
	        if (bp->comp2 == NO_COMP)
	        {
	            bp->comp1 = bp->prev->comp2;
	            bp->comp2 = bp->comp1;
	        }
	    }

	}

	for (bpoint=Bpoint.next; bpoint != NULL; bpoint=bpoint->next)
	{
	    curve = NULL;
	    for (c = bpoint->node->out_curves; c && *c; c++)
	    {
	        if (is_bdry(*c))
	        {
	            curve = *c;
	            break;
	        }
	    }

	    if (curve == NULL)
	    {
	        if (bpoint->next != NULL)
	        {
	            if (bpoint->comp2 != bpoint->next->comp1)
	            {
	                /* The interface is inconsistent somehow, so that the
			 * interior component for the new curve is different at
			 * the the start and end nodes. */

	                (void) printf("WARNING in i_set_boundary2d(), "
	                              "bpoint->comp2 = %d != "
				      "bpoint->next->comp1 = %d\n",
				      bpoint->comp2,bpoint->next->comp1);
			(void) printf("eps = %g, i_comp = %d\n",eps,i_comp);
			print_rectangular_grid(gr);
			print_interface(intfc);
			print_bpoint(bpoint);
			show_bpoints(Bpoint.next);
	                DEBUG_LEAVE(i_set_boundary2d)
	                return FUNCTION_FAILED;
	            }
	            curve = make_curve(outside,bpoint->comp2,bpoint->node,
	                               bpoint->next->node);
	        }
	        else
	        {
	            if (bpoint->comp2 != Bpoint.next->comp1)
	            {
	                /* See component test above. */
	                (void) printf("WARNING in i_set_boundary2d(), "
	                              "bpoint->comp2 = %d != "
				      "Bpoint.next->comp1 = %d\n",
				      bpoint->comp2,Bpoint.next->comp1);
			(void) printf("eps = %g, i_comp = %d\n",eps,i_comp);
			print_rectangular_grid(gr);
			print_interface(intfc);
			print_bpoint(bpoint);
			show_bpoints(Bpoint.next);
	                DEBUG_LEAVE(i_set_boundary2d)
	                return FUNCTION_FAILED;
	            }
	            curve = make_curve(outside,bpoint->comp2,bpoint->node,
	                               Bpoint.next->node);
	        }
	    }
	    set_is_bdry(curve);
	}

	if (DEBUG)
	{
	    (void) printf("Output interface formt i_set_boundary2d()\n");
	    print_interface(intfc);
	}
	set_current_interface(cur_intfc);
	DEBUG_LEAVE(i_set_boundary2d)
	return FUNCTION_SUCCEEDED;
}	    /*end i_set_boundary2d*/


LOCAL	BPOINT	*set_bpoint_at_corner(
	int		corner_number,
	BPOINT		*bpoint,
	INTERFACE	*intfc,
	double		*L,
	double		*U,
	double		*offset,
	double		eps)
{
	NODE		**n, *nc;
	double		sep, dist, min_dist = HUGE_VAL;
	double		corner[MAXD];
	int		i, dim = intfc->dim;

	DEBUG_ENTER(set_bpoint_at_corner)

	switch (corner_number)
	{
	case 0:
	    corner[0] = L[0];
	    corner[1] = L[1];
	    break;
	case 1:
	    corner[0] = L[0];
	    corner[1] = U[1];
	    break;
	case 2:
	    corner[0] = U[0];
	    corner[1] = U[1];
	    break;
	case 3:
	    corner[0] = U[0];
	    corner[1] = L[1];
	    break;
	default:
	    DEBUG_LEAVE(set_bpoint_at_corner)
	    return NULL;
	}

	nc = NULL;
	for (n = intfc->nodes; n && *n; n++)
	{
	    if (!is_bdry(*n))
	        continue;
	    dist = 0.0;
	    for (i = 0; i < dim; i++)
	    {
	        sep = fabs(corner[i] - Coords((*n)->posn)[i]);
	        if (sep > eps)
	            break;
	        dist += sep;
	    }
	    if (i < dim)
	        continue;
	    if ((nc == NULL) || (dist < min_dist))
	    {
	        nc = *n;
	        min_dist = dist;
	    }
	}
	if (nc != NULL)
	{
	    bpoint = set_bpoint_at_node(bpoint,nc,L,U,offset,eps);
	}
	else
	{
	    bpoint->next = (BPOINT*) store(sizeof(BPOINT));
	    bpoint->next->prev = bpoint;
	    bpoint = bpoint->next;
	    bpoint->next = NULL;
	    bpoint->node = NULL;
	    bpoint->dist = offset[corner_number];
	    bpoint->comp1 = NO_COMP;
	    bpoint->comp2 = NO_COMP;
	    bpoint->posn = Point(corner);
	    bpoint->side = NOT_A_BDRY;
	}

	DEBUG_LEAVE(set_bpoint_at_corner)
	return bpoint;
}		/*end set_bpoint_at_corner*/



/*
*				set_bpoint():
*
*	Adds a NODE node to the BPOINT array provided that node is really
*	on the boundary, and increments the number of BPOINTS.
*/


LOCAL BPOINT	*set_bpoint(
	BPOINT		*bpoint,
	NODE		*node,
	COMPONENT	c1,
	COMPONENT	c2,
	double		*L,
	double		*U,
	double		eps,
	double		*offset)
{
	BPOINT		*bp;
	BDRY_SIDE	side;
	FT_POINT		*p = node->posn;

	DEBUG_ENTER(set_bpoint)

	side = boundary_side_of_point(p,L,U,eps);
	if (side == NOT_A_BDRY)
	{
	    set_not_bdry(node);
	    DEBUG_LEAVE(set_bpoint)
	    return bpoint;
	}
	bp = node_in_bpoint_list(node,bpoint);
	if (bp != NULL)
	{
	    bp->comp1 = c1;
	    bp->comp2 = c2;
	    DEBUG_LEAVE(set_bpoint)
	    return bpoint;
	}

	bpoint->next = (BPOINT*) store(sizeof(BPOINT));
	bpoint->next->prev = bpoint;
	bpoint = bpoint->next;
	bpoint->next = NULL;
	bpoint->node = node;
	bpoint->posn = p;
	bpoint->comp1 = c1;
	bpoint->comp2 = c2;
	bpoint->side = side;

	set_is_bdry(node);

	switch (side)
	{
	case LEFT_BDRY:
	    bpoint->dist  = Coords(p)[1] - L[1] + offset[0];
	    break;
	case UPPER_BDRY:
	    bpoint->dist  = Coords(p)[0] - L[0] + offset[1];
	    break;
	case RIGHT_BDRY:
	    bpoint->dist  = U[1] - Coords(p)[1] + offset[2];
	    break;
	case LOWER_BDRY:
	    bpoint->dist  = U[0] - Coords(p)[0] + offset[3];
	    break;
	}
	DEBUG_LEAVE(set_bpoint)
	return bpoint;
}		/*end set_bpoint*/
	

LOCAL	BPOINT*	node_in_bpoint_list(
	NODE		*node,
	BPOINT		*bpoint)
{
	BPOINT		*bp;

	DEBUG_ENTER(node_in_bpoint_list)

	for (bp = bpoint; bp != NULL; bp = bp->next)
	{
	    if (bp->node == node)
	    {
	        DEBUG_LEAVE(node_in_bpoint_list)
	        return bp;
	    }
	}
	for (bp = bpoint->prev; bp != NULL; bp = bp->prev)
	{
	    if (bp->node == node)
	    {
	        DEBUG_LEAVE(node_in_bpoint_list)
	        return bp;
	    }
	}
	DEBUG_LEAVE(node_in_bpoint_list)
	return NULL;
}		/*end node_in_bpoint_list*/

LOCAL	BDRY_SIDE boundary_side_of_point(
	FT_POINT *p,
	double *L,
	double *U,
	double eps)
{
	if (fabs(Coords(p)[0] - L[0]) < eps)
	    return LEFT_BDRY;
	else if (fabs(Coords(p)[1] - U[1]) < eps)
	    return UPPER_BDRY;
	else if (fabs(Coords(p)[0] - U[0]) < eps)
	    return RIGHT_BDRY;
	else if (fabs(Coords(p)[1] - L[1]) < eps)
	    return LOWER_BDRY;
	return NOT_A_BDRY;
}		/*end boundary_side_of_point*/

LOCAL	BPOINT*	set_bpoint_at_node(
	BPOINT		*bpoint,
	NODE		*n,
	double		*L,
	double		*U,
	double		*offset,
	double		eps)
{
	COMPONENT	comp1 = NO_COMP, comp2 = NO_COMP;
	CURVE		*curve;
	CURVE		**c;
	CURVE		*ca, *ctmp;
	CURVE		*bc_in, *bc_out;
	ORIENTATION	ca_orient, ctmp_orient;
	int		num_in, num_out;
	int		i;
	BDRY_SIDE       side;

	DEBUG_ENTER(set_bpoint_at_node)
	if ((side = boundary_side_of_point(n->posn,L,U,eps)) == NOT_A_BDRY)
	{
	    DEBUG_LEAVE(set_bpoint_at_node)
	    return bpoint;
	}
	for (num_in=0, bc_in=NULL, c=n->in_curves; c && *c; num_in++, c++)
	    if (is_bdry(*c))
	        bc_in = *c;
	for (num_out=0, bc_out=NULL, c=n->out_curves; c && *c; num_out++, c++)
	    if (is_bdry(*c))
	        bc_out = *c;

	if (bc_in != NULL && bc_out != NULL)
	{
	    comp1 = positive_component(bc_in);
	    comp2 = positive_component(bc_out);
	}
	else if (bc_out != NULL)
	{
	    if (num_in == 0 && num_out == 1)
	    {
	        comp1 = positive_component(bc_out);
	        comp2 = positive_component(bc_out);
	    }
	    else
	    {
	        ctmp = bc_out;
	        ctmp_orient = POSITIVE_ORIENTATION;
	        do
	        {
	            ca = ctmp;
	            ca_orient = ctmp_orient;
	            ctmp = adjacent_curve(ca,ca_orient,CLOCKWISE,&ctmp_orient);
	        } while (ctmp != NULL && ctmp != bc_out);
	        comp1 = (ca_orient == POSITIVE_ORIENTATION) ?
	            positive_component(ca) : negative_component(ca);
	        comp2 = positive_component(bc_out);
	    }
	}
	else if (bc_in != NULL)
	{
	    if (num_in == 1 && num_out == 0)
	    {
	        comp1 = positive_component(bc_in);
	        comp2 = positive_component(bc_in);
	    }
	    else
	    {
	        ctmp = bc_in;
	        ctmp_orient = NEGATIVE_ORIENTATION;
	        do
	        {
	            ca = ctmp;
	            ca_orient = ctmp_orient;
	            ctmp = adjacent_curve(ca,ca_orient,
	                                  COUNTER_CLOCK,&ctmp_orient);
	        } while (ctmp != NULL && ctmp != bc_in);
	        comp1 = positive_component(bc_in);
	        comp2 = (ca_orient == POSITIVE_ORIENTATION) ?
	            negative_component(ca) : positive_component(ca);
	    }
	}
	else if (num_in == 0 && num_out == 1)
	{
	    curve = n->out_curves[0];
	    comp1 = positive_component(curve);
	    comp2 = negative_component(curve);
	}
	else if (num_in == 1 && num_out == 0)
	{
	    curve = n->in_curves[0];
	    comp1 = negative_component(curve);
	    comp2 = positive_component(curve);
	}
	else if ((num_in + num_out) > 0)
	{
	    O_NODE *on = make_onode(n);
	    CURVE *c1 = NULL, *c2 = NULL;
	    ORIENTATION c1_orient, c2_orient;

	    if ((side != LEFT_BDRY) ||
		((on->ang[on->num_c-1] - on->ang[0]) <= 0.5*PI))
	    {
	        c1 = on->nc[0];
	        c1_orient = on->orient[0];
	        c2 = on->nc[on->num_c-1];
	        c2_orient = on->orient[on->num_c-1];
	    }
	    else 
	    {
	        for (i = on->num_c-2; i >= 0; i--)
	        {
	            if (on->ang[i] <= 0.5*PI)
	            {
	                i++;
	                break;
	            }
	        }
	        c1 = on->nc[i];
	        c1_orient = on->orient[i];

	        for (i = 1; i < on->num_c; i++)
	        {
	            if (on->ang[i] >= 1.5*PI)
	            {
	                i--;
	                break;
	            }
	        }
	        c2 = on->nc[i];
	        c2_orient = on->orient[i];
	    }
	    comp1 = (c1_orient == POSITIVE_ORIENTATION) ?
	        positive_component(c1) : negative_component(c1);
	    comp2 = (c2_orient == NEGATIVE_ORIENTATION) ?
	        positive_component(c2) : negative_component(c2);

	}
	DEBUG_LEAVE(set_bpoint_at_node)
	return set_bpoint(bpoint,n,comp1,comp2,L,U,eps,offset);
}		/*end set_bpoint_at_node*/

/*
*				sort_bpoint():
*
*	Sorts a given bpoint array into the order of increasing  dist field
*/


LOCAL void sort_bpoint(
	BPOINT		*bpoint)
{
	BPOINT		*bphead;
	BPOINT		*bp1, *bp2;
	BPOINT		Tmp1, Tmp2;

	DEBUG_ENTER(sort_bpoint)

	if (bpoint == NULL)
	{
	    DEBUG_LEAVE(sort_bpoint)
	    return;
	}

	for (bphead = bpoint; bphead->prev; bphead = bphead->prev);
	for (bp1 = bphead; bp1 != NULL; bp1 = bp1->next)
	{
	    for (bp2 = bp1->next; bp2 != NULL; bp2 = bp2->next)
	    {
	        if (bp2->dist < bp1->dist)
	        {
	            Tmp1 = *bp1;
	            Tmp2 = *bp2;
	            *bp1 = Tmp2;
	            *bp2 = Tmp1;
	            bp1->next = Tmp1.next;
	            bp1->prev = Tmp1.prev;
	            bp2->next = Tmp2.next;
	            bp2->prev = Tmp2.prev;
	        }
	    }
	}
	DEBUG_LEAVE(sort_bpoint)
}		/*end sort_bpoint*/

/*
*			show_bpoints():
*
*	Prints a BPOINT array in a reasonable format for reading.
*/


LOCAL void show_bpoints(
	BPOINT		*bpoints)
{
	int		i = 0;

	DEBUG_ENTER(show_bpoints)

	(void) printf("%3s %15s %15s %15s %9s %2s %2s %10s\n",
		      "i","x","y","dist","node","c1","c2","side");
	for (;bpoints != NULL; bpoints = bpoints->next, i++)
	{
	    (void) printf("%3d %15.12g %15.12g %15.12g %9llu %2d %2d %10s\n",i,
	                  Coords(bpoints->posn)[0],Coords(bpoints->posn)[1],
	                  bpoints->dist,node_number(bpoints->node),
	                  bpoints->comp1,bpoints->comp2,
			  bdry_side_as_string(bpoints->side));
	}
	(void) printf("\n\n");

	DEBUG_LEAVE(show_bpoints)
}		/*end show_bpoints*/

LOCAL	void	print_bpoint(
	BPOINT *bpoint)
{
	(void) printf("BPOINT 0x%p, prev 0x%p next 0x%p\n",
		      bpoint,bpoint->prev,bpoint->next);
	(void) printf("node %llu\n",node_number(bpoint->node));
	print_node(bpoint->node);
	(void) printf("posn 0x%p, at (%15.12g, %15.12g)\n",bpoint->posn,
		      Coords(bpoint->posn)[0],Coords(bpoint->posn)[1]);
	(void) printf("comp1 = %d, comp2 = %d\n",bpoint->comp1,
		      bpoint->comp2);
	(void) printf("dist = %15.12g\n",bpoint->dist);
	print_bdry_side("side = ",bpoint->side,"\n");
}		/*end print_bpoint*/
#endif /* defined(TWOD) */
