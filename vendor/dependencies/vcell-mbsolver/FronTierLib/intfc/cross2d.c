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
*				cross2d.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	This file contains routines to support and maintain the (2 dimensional)
*	CROSS structure, as the bonds and curves it points to are modified
*	during the modification of the interface (as occurs during a call
*	to untangle).  In particular, split_curve, join_curve, and addition
*	or deletion of points to a bond require a modification of the cross
*	list.
*/


#if defined(TWOD)
#include <intfc/iloc.h>

	/* LOCAL Function Declarations */
LOCAL	double	distance_between_points_on_curve(FT_POINT*,BOND*,FT_POINT*,BOND*,
						 CURVE*,ORIENTATION);
LOCAL	int	num_physical_sectors(CROSS*);
LOCAL	int	same_curves_cross(CROSS*,CROSS*);
LOCAL	void	reset_cross(FT_POINT*,BOND**,CURVE**,FT_POINT*,BOND*,
			    CURVE*,CURVE*,CURVE*);

/*
*			add_to_cross_list():
*
*	Add an element to a cross list ending in *cr.
*/

EXPORT	void add_to_cross_list(
	CROSS		**cross,
	CURVE		*c1,
	BOND		*b1,
	CURVE		*c2,
	BOND		*b2,
	FT_POINT		*p)
{
	CROSS		*cr = *cross;

	cr->next = (CROSS *)Store(sizeof(CROSS));
	cr->next->prev = cr;
	cr = cr->next;
	cr->next = NULL;

	cr->c1 = c1;
	cr->c2 = c2;
	cr->b1 = b1;
	cr->b2 = b2;
	cr->p  = p;
	*cross = cr;
}		/*end add_to_cross_list*/


EXPORT void insert_in_cross_list(
	CROSS		*cr,
	CROSS		*cr0)
{
	if (cr == NULL || cr0 == NULL)
	    return;
	cr0->prev = cr;
	cr0->next = cr->next;
	if (cr->next != NULL)
	    cr->next->prev = cr0;
	cr->next = cr0;
}		/*end insert_in_cross_list*/

/*
*			rcl_after_split():
*
*	Resets a cross list, starting at cr->next, due to split curves
*	in cross, where csplit is the pair of split curves.
*/

EXPORT void rcl_after_split(
	CROSS		*cr,
	FT_POINT		*p,
	BOND		*b,
	CURVE		*c,
	CURVE		**csplit)
{
	CROSS		*later_cr,*prev_cr;

	debug_print("rcl","Enter rcl_after_split()\n");
	if (cr == NULL || csplit == NULL) 
	{
	    debug_print("rcl","Leave rcl_after_split()\n");
	    return;
	}

	if (debugging("rcl"))
	{
	    (void) printf("Cross list in rcl_after_split()\n");
	    print_cross_list(cr);
	}

	for (later_cr = cr->next; later_cr; later_cr = later_cr->next) 
	{
	    if (later_cr->c1 == c)
	    {
	    	if (debugging("rcl"))
	    	{
	    	    (void) printf("call reset_cross() for\n");
	    	    (void) printf("cr - ");
	    	    print_cross(cr);
	    	    (void) printf("later_cr - ");
	    	    print_cross(later_cr);
	    	    (void) printf("csplit[0] %llu  [1] %llu\n",
	    			  curve_number(csplit[0]),
	    			  curve_number(csplit[1]));
	    	}
	    	reset_cross(later_cr->p,&later_cr->b1,&later_cr->c1,
	    		    p,b,c,csplit[0],csplit[1]);
	    }

	    if (later_cr->c2 == c)
	    {
	    	if (debugging("rcl"))
	    	{
	    	    (void) printf("call reset_cross() for\n");
	    	    (void) printf("cr - ");
	    	    print_cross(cr);
	    	    (void) printf("later_cr - ");
	    	    print_cross(later_cr);
	    	    (void) printf("csplit[0] %llu  [1] %llu\n",
	    			  curve_number(csplit[0]),
	    			  curve_number(csplit[1]));
	    	}
	    	reset_cross(later_cr->p,&later_cr->b2,&later_cr->c2,
	    		    p,b,c,csplit[0],csplit[1]);
	    }

	    if (debugging("rcl")) 
	    {
	    	(void) printf("Curves from later_cross:\n");
	    	print_curve(later_cr->c1);
	    	print_curve(later_cr->c2);
	    }
	}

	if (debugging("rcl"))
	{
	    (void) printf("rcl_after_split\n");
	    (void) printf("cross -\n");		print_cross(cr);
	    (void) printf("c -\n");			print_curve(c);
	    (void) printf("csplit[0] -\n");		print_curve(csplit[0]);
	    (void) printf("csplit[1] -\n");		print_curve(csplit[1]);
	}

	for (prev_cr = cr->prev; prev_cr; prev_cr = prev_cr->prev) 
	{
	    if (debugging("rcl"))
	    {
	    	(void) printf("prev_cr -\n");	print_cross(prev_cr);
	    }

	    if (prev_cr->c1 == c)
	    {
	    	if (debugging("rcl"))
	    	{
	    	    (void) printf("call reset_cross() for\n");
	    	    (void) printf("cr - ");		
	    	    print_cross(cr);
	    	    (void) printf("prev_cr - ");	
	    	    print_cross(prev_cr);
	    	    (void) printf("csplit[0] %llu  [1] %llu\n",
	    			  curve_number(csplit[0]),
	    			  curve_number(csplit[1]));
	    	}
	    	reset_cross(prev_cr->p,&prev_cr->b1,&prev_cr->c1,
	    		    p,b,c,csplit[0],csplit[1]);
	    }

	    if (prev_cr->c2 == c)
	    {
	    	if (debugging("rcl"))
	    	{
	    	    (void) printf("call reset_cross() for\n");
	    	    (void) printf("cr - ");		
	    	    print_cross(cr);
	    	    (void) printf("prev_cr - ");	
	    	    print_cross(prev_cr);
	    	    (void) printf("csplit[0] %llu  [1] %llu\n",
	    			  curve_number(csplit[0]),
	    			  curve_number(csplit[1]));
	    	}
	    	reset_cross(prev_cr->p,&prev_cr->b2,&prev_cr->c2,
	    		    p,b,c,csplit[0],csplit[1]);
	    }


	    if (debugging("rcl")) 
	    {
	    	(void) printf("Curves from prev_cross:\n");
	    	print_curve(prev_cr->c1);
	    	print_curve(prev_cr->c2);
	    }
	}

	if (cr->c1 == c) 
	{
	    if (debugging("rcl")) (void) printf("cr->c1 == c\n");

	    if (cr->b1 == b)
	    {
	    	cr->c1 = csplit[0];
	    	cr->b1 = csplit[0]->last;
	    }
	    else
		reset_cross(cr->p,&cr->b1,&cr->c1,p,b,c,csplit[0],csplit[1]);
	}
	
	if (cr->c2 == c)
	{
	    if (debugging("rcl")) (void) printf("cr->c2 == c\n");

	    if (cr->b2 == b) 
	    {
	    	cr->c2 = csplit[0];
	    	cr->b2 = csplit[0]->last;
	    }
	    else
		reset_cross(cr->p,&cr->b2,&cr->c2,p,b,c,csplit[0],csplit[1]);
	}
	debug_print("rcl","Leave rcl_after_split()\n");
}		/*end rcl_after_split*/

/*
*			rcl_after_join():
*
*	Resets a cross list, looking both ways from cr, due to joining of
*	curves c1 and c2 to form a new curve c.
*/

EXPORT void rcl_after_join(
	CROSS		*cr,
	CURVE		*c,
	CURVE		*c1,
	CURVE		*c2)
{
	CROSS		*later_cr,*prev_cr;

	for (later_cr = cr->next; later_cr; later_cr = later_cr->next)
	{
	    if (later_cr->c1 == c1 || later_cr->c1 == c2)
		later_cr->c1 = c;
	    if (later_cr->c2 == c1 || later_cr->c2 == c2)
		later_cr->c2 = c;
	}
	for (prev_cr = cr->prev; prev_cr; prev_cr = prev_cr->prev)
	{
	    if (prev_cr->c1 == c1 || prev_cr->c1 == c2)
		prev_cr->c1 = c;
	    if (prev_cr->c2 == c1 || prev_cr->c2 == c2)
		prev_cr->c2 = c;
	}
}		/*end rcl_after_join*/


/*
*			rcl_after_insert_point():
*
*	Resets a cross list, starting at cr->next, due to insert_point_in_bond,
*	where the point p has been inserted into the bond b.
*/

EXPORT void rcl_after_insert_point(
	CROSS		*cr,
	FT_POINT		*p,
	BOND		*b)
{
	CROSS		*later_cr,*prev_cr;
	int		dim = cr->c1->interface->dim;

	for (later_cr = cr->next; later_cr; later_cr = later_cr->next)
	{
	    if ((b == later_cr->b1) &&
		(points_in_strict_order(p,b,later_cr->p,b,dim) == YES))
	    {
	    	later_cr->b1 = b->next;
	    }
	    if ((b == later_cr->b2) &&
		(points_in_strict_order(p,b,later_cr->p,b,dim) == YES))
	    {
	    	later_cr->b2 = b->next;
	    }
	}
	for (prev_cr = cr->prev; prev_cr; prev_cr = prev_cr->prev)
	{
	    if ((b == prev_cr->b1) &&
		(points_in_strict_order(p,b,prev_cr->p,b,dim) == YES))
	    {
	    	prev_cr->b1 = b->next;
	    }
	    if ((b == prev_cr->b2) &&
		(points_in_strict_order(p,b,prev_cr->p,b,dim) == YES))
	    {
	    	prev_cr->b2 = b->next;
	    }
	}
}		/*end rcl_after_insert_point*/


/*
*		rcl_after_delete_bond_fragment_at_node():
*
*	The bond fragment from p to the node is to be deleted. All crosses
*	which occur in this fragment will be deleted from the list.
*/


EXPORT void rcl_after_delete_bond_fragment_at_node(
	CROSS		*cr,
	FT_POINT		*p,
	CURVE		*c,
	ORIENTATION	orient)
{
	CROSS		*later_cr,*prev_cr;
	FT_POINT		*node_pos;
	BOND		*cr_b;

	cr_b = Bond_at_node(c,orient);
	node_pos = Node_of(c,orient)->posn;
	
	for (later_cr = cr->next; later_cr; later_cr = later_cr->next)
	{
	    if ((later_cr->b1 == cr_b) &&
	    	(separation(node_pos,later_cr->p,c->interface->dim)
	    		<= separation(node_pos,p,c->interface->dim)))
	    	delete_from_cross_list(later_cr);
	    if ((later_cr->b2 == cr_b) && 
	    	(separation(node_pos,later_cr->p,c->interface->dim)
	    		<= separation(node_pos,p,c->interface->dim)))
	    	delete_from_cross_list(later_cr);
	}
	for (prev_cr = cr->prev; prev_cr; prev_cr = prev_cr->prev)
	{
	    if ((prev_cr->b1 == cr_b) &&
	    	(separation(node_pos,prev_cr->p,c->interface->dim)
	    		<= separation(node_pos,p,c->interface->dim)))
	    	delete_from_cross_list(prev_cr);
	    if ((prev_cr->b2 == cr_b) &&
	    	(separation(node_pos,prev_cr->p,c->interface->dim)
	    		<= separation(node_pos,p,c->interface->dim)))
	    	delete_from_cross_list(prev_cr);
	}
}		/*end rcl_after_delete_bond_fragment_at_node*/

/*
*			reset_cross():
*/


LOCAL void reset_cross(
	FT_POINT		*later_pt,
	BOND		**later_bond,
	CURVE		**later_curve,
	FT_POINT		*old_pt,
	BOND		*old_bond,
	CURVE		*old_curve,
	CURVE		*new_c0,
	CURVE		*new_c1)
{
	BOND		*oldb,*newb;
	int		dim = new_c0->interface->dim;

	debug_print("rcl","Enter reset_cross()\n");
	if (debugging("rcl"))
	{
	    (void) printf("later: pt %g %g\n",
	    	      Coords(later_pt)[0],Coords(later_pt)[1]);
	    (void) printf("       bond - ");      print_bond(*later_bond);
	    (void) printf("       curve - ");     print_curve(*later_curve);
	    (void) printf("\n");
	    (void) printf("old: pt %g %g\n",
	    	      Coords(old_pt)[0],Coords(old_pt)[1]);
	    (void) printf("       bond - ");      print_bond(old_bond);
	    (void) printf("       curve - ");     print_curve(old_curve);
	    (void) printf("\n");
	    (void) printf("new:   c0 - ");	      print_curve(new_c0);
	    (void) printf("new:   c1 - ");	      print_curve(new_c1);
	    (void) printf("\n");
	}
	if (points_in_strict_order(later_pt,*later_bond,old_pt,
				   old_bond,dim) == YES) 
	{
	    oldb = old_curve->first;
	    newb = new_c0->first;
	    while (oldb != *later_bond) 
	    {
	    	oldb = oldb->next;
	    	newb = newb->next;
	    }
	    *later_bond = newb;
	    *later_curve = new_c0;
	}
	else 
	{
	    oldb = old_curve->last;
	    newb = new_c1->last;
	    if (debugging("rcl"))
	    {
	    	(void) printf("oldb - ");	print_bond(oldb);
	    	(void) printf("newb - ");	print_bond(newb);
	    	(void) printf("later - ");	print_bond(*later_bond);
	    }
	    while (oldb != *later_bond && oldb != NULL) 
	    {
	    	oldb = oldb->prev;
	    	newb = newb->prev;
	    }
	    if (newb != NULL && oldb != NULL)
	    {
	    	*later_bond = newb;
	    	*later_curve = new_c1;
	    	debug_print("rcl","Leave reset_cross()\n");
	    	return;
	    }

	    /*
	     *  If *later_bond and old_bond are equal and later_point
	     *  and old_point have the same position the correct
	     *  *later_curve is new_c0 and the above code will
	     *  produce a NULL *later_bond.  This situation can
	     *  arise in some applications.
	     */

	    oldb = old_curve->first;
	    newb = new_c0->first;
	    while (oldb != *later_bond) 
	    {
	    	oldb = oldb->next;
	    	newb = newb->next;
	    }
	    *later_bond = newb;
	    *later_curve = new_c0;
	}
	debug_print("rcl","Leave reset_cross()\n");
}		/*end reset_cross*/

/*
*			print_cross():
*
*	This routine prints a cross structure.
*/

EXPORT 	void print_cross(
	CROSS		*cr)
{
	BOND		*bb;
	int		is_on;

	if (cr == NULL)
	{
	    (void) printf("Cross NULL\n");
	    return;
	}
	(void) printf("Cross %p    next %p prev %p\n",
		      (POINTER)cr,(POINTER)cr->next,(POINTER)cr->prev);

	bb = cr->b1;
	(void) printf("\tc1 %llu\n\tb1 ",curve_number(cr->c1));
	print_bond(cr->b1);

	if (cr->c1 != NULL)
	{
	    is_on = NO;
	    for (bb = cr->c1->first;  bb;  bb = bb->next)
	    {
	    	if (bb == cr->b1)
		    is_on = YES;
	    }
	    (void) printf("\tb1 %s on c1\n",is_on ? "is" : "is NOT");
	}


	bb = cr->b2;
	(void) printf("\tc2 %llu\n\tb2 ",curve_number(cr->c2));
	print_bond(cr->b2);

	if (cr->c2 != NULL)
	{
	    is_on = NO;
	    for (bb = cr->c2->first;  bb;  bb = bb->next)
	    {
	    	if (bb == cr->b2)
		    is_on = YES;
	    }
	    (void) printf("\tb2 %s on c2\n",is_on ? "is" : "is NOT");
	}

	if (cr->p == NULL)
	    (void) printf("p NULL\n");
	else
	    (void) printf("p %llu (%g, %g)\n",point_number(cr->p),
	    	          Coords(cr->p)[0],Coords(cr->p)[1]);

	(void) printf("\n");
}		/*end print_cross*/

EXPORT 	void print_cross_list(
	CROSS		*cross)
{
	CROSS		*cr;

	if (cross == NULL)
	{
	    (void) printf("Cross NULL\n");
	    return;
	}
	for (cr = cross;  cr;  cr = cr->next)
	    print_cross(cr);
}		/*end print_cross_list*/

LOCAL int same_curves_cross(
	CROSS		*cr1,
	CROSS		*cr2)
{
	if (cr1->c1 == cr2->c1 && cr1->c2 == cr2->c2)
	    return YES;
	if (cr1->c1 == cr2->c2 && cr1->c2 == cr2->c1)
	    return YES;
	return NO;
}		/*end same_curves_cross*/

LOCAL int num_physical_sectors(
	CROSS		*cr)
{
	int		num = 0;

	if (negative_component(cr->c1) == negative_component(cr->c2)) num++;
	if (negative_component(cr->c1) == positive_component(cr->c2)) num++;
	if (positive_component(cr->c1) == negative_component(cr->c2)) num++;
	if (positive_component(cr->c1) == positive_component(cr->c2)) num++;
	return num;
}		/*end num_physical_sectors*/


/*
*
*			find_companion_cross():
*
*	Given a cross this funcion attempts to find the companion
*	cross corresponding to the given cross.  Generally this occurs
*	when two curves cross each other in their interiors.  In this
*	case, crosses should occur in pairs where the curves first cross
*	into each other and then out of each other. Thus the companion cross
*	is defined as one of the two adjacent crosses with the same curves as
*	the given cross. Here the order of crosses with the same curves is
*	defined by the order of crossing points on cross->c1.
*	The choice between the adjacent crosses is determined by inconsistent
*	components of the crossing curves, or if this fail, by the shortest
*	distance.
*
*	The orient variables give the orientation of the two
*	crosses, thus for the given cross c1_orient and c2_orient
*	give the orientations of the two curves at the cross
*	into the physical sector, similarly for c11_orient and
*	c12_orient.  In other words, the orientations give directions
*	pointing away from the companion cross.
*
*	NO is returned if the companion cross is not found.
*/

EXPORT int find_companion_cross(
	CROSS		*cross,
	CROSS		**cross1,
	ORIENTATION	*c1_orient,
	ORIENTATION	*c2_orient,
	ORIENTATION	*c11_orient,
	ORIENTATION	*c12_orient)
{
	CROSS		*cr;
	CROSS		*cr1f;	/* cross immediately following given cross */
	CROSS		*cr1b;	/* cross immediately before given cross */
	BOND		*bb;
	double		cp, cp1;
	double		l1f;	/* distance on cross->c1 to following cross */
	double		l1b;	/* distance on cross->c1 to preceeding cross */
	double		l2f;	/* forward distance on cross->c2 to *cross1 */
	double		l2b;	/* backward distance on cross->c2 to *cross1 */
	int		dim = cross->c1->interface->dim;

	/* Determine neighboring crosses */

	*cross1 = cr1f = cr1b = NULL;
	if (cross == NULL)
	    return NO;



	/* Check for companion in following crosses */

	for (cr = cross->next; cr != NULL; cr = cr->next)
	{
	    if (!same_curves_cross(cross,cr)) continue;
	    
	    /* Set cr curves consistently with cross curves */
	    
	    if (cr->c1 != cross->c1)
	    {
	    	cr->c2 = cr->c1;
	    	cr->c1 = cross->c1;
	    	bb = cr->b1;
	    	cr->b1 = cr->b2;
	    	cr->b2 = bb;
	    }
	    if (points_in_strict_order(cross->p,cross->b1,cr->p,
				       cr->b1,dim) == YES)
	    {
	    	if ((!cr1f) ||
		    (points_in_strict_order(cr->p,cr->b1,cr1f->p,
					    cr1f->b1,dim) == YES))
	    	{
	    	    cr1f = cr;
	    	}
	    }
	    else
	    {
	    	if ((!cr1b) ||
		    (points_in_strict_order(cr1b->p,cr1b->b1,cr->p,
					    cr->b1,dim) == YES))
		{
		    cr1b = cr;
	        }
	    }
	}

	/* Check for companion in previous crosses */

	for (cr = cross->prev; cr != NULL; cr = cr->prev)
	{
	    if (!same_curves_cross(cross,cr))
		continue;

	    /* Set cr curves consistently with cross curves */
	    
	    if (cr->c1 != cross->c1)
	    {
	    	cr->c2 = cr->c1;
	    	cr->c1 = cross->c1;
	    	bb = cr->b1;
	    	cr->b1 = cr->b2;
	    	cr->b2 = bb;
	    }
	    if (points_in_strict_order(cross->p,cross->b1,cr->p,
				       cr->b1,dim) == YES)
	    {
	    	if ((!cr1f) ||
		    (points_in_strict_order(cr->p,cr->b1,cr1f->p,
					    cr1f->b1,dim) == YES))
	       	{
			cr1f = cr;
		}
	    }
	    else
	    {
		if ((!cr1b) ||
		    (points_in_strict_order(cr1b->p,cr1b->b1,cr->p,
					    cr->b1,dim) == YES))
		{
		    cr1b = cr;
		}
	    }
	}

	if (is_closed_curve(cross->c1) && (cr1b == NULL || cr1f == NULL))
	{
	    if (cr1b != NULL)
	    {
	    	for (cr = cross->next; cr != NULL; cr = cr->next)
	    	{
	    	    if (!same_curves_cross(cross,cr))
			continue;
	    
	    	    if ((!cr1f) ||
			(points_in_strict_order(cr->p,cr->b1,cr1f->p,
						cr1f->b1,dim) == YES))
	    	    {
	    	    	cr1f = cr;
	    	    }
	    	}
	    	for (cr = cross->prev; cr != NULL; cr = cr->prev)
	    	{
	    	    if (!same_curves_cross(cross,cr))
			continue;
	    
	    	    if ((!cr1f) ||
			(points_in_strict_order(cr->p,cr->b1,cr1f->p,
						cr1f->b1,dim) == YES))
	    	    {
	    	    	cr1f = cr;
	    	    }
	    	}
	    }
	    else if (cr1f != NULL)
	    {
	    	for (cr = cross->next; cr != NULL; cr = cr->next)
	    	{
	    	    if (!same_curves_cross(cross,cr))
			continue;
	    
	    	    if ((!cr1b) ||
			(points_in_strict_order(cr1b->p,cr1b->b1,cr->p,
						cr->b1,dim) == YES))
	    	    {
	    	    	cr1b = cr;
	    	    }
	    	}
	    	for (cr = cross->prev; cr != NULL; cr = cr->prev)
	    	{
	    	    if (!same_curves_cross(cross,cr))
			continue;
	    
	    	    if ((!cr1b) ||
			(points_in_strict_order(cr1b->p,cr1b->b1,cr->p,
						cr->b1,dim) == YES))
	    	    {
	    	    	cr1b = cr;
	    	    }
	    	}
	    }
	}

	vector_product_on_bonds(cross->b1,cross->b2,2,&cp);
	if (num_physical_sectors(cross) == 1)
	{
	    COMPONENT lcomp1 = negative_component(cross->c1);
	    COMPONENT rcomp1 = positive_component(cross->c1);
	    COMPONENT lcomp2 = negative_component(cross->c2);
	    COMPONENT rcomp2 = positive_component(cross->c2);

	    /* Determine companion cross from components */

	    if (cp > 0)
	    {
	    	if (rcomp1 == lcomp2)
	    	{
	    	    *cross1 = cr1f;
	    	    *c1_orient = NEGATIVE_ORIENTATION;
	    	    *c2_orient = NEGATIVE_ORIENTATION;
	    	}
	    	else if (lcomp1 == rcomp2)
	    	{
	    	    *cross1 = cr1b;
	    	    *c1_orient = POSITIVE_ORIENTATION;
	    	    *c2_orient = POSITIVE_ORIENTATION;
	    	}
	    	else if (lcomp1 == lcomp2)
	    	{
	    	    *cross1 = cr1f;
	    	    *c1_orient = NEGATIVE_ORIENTATION;
	    	    *c2_orient = POSITIVE_ORIENTATION;
	    	}
	    	else if (rcomp1 == rcomp2)
	    	{
	    	    *cross1 = cr1b;
	    	    *c1_orient = POSITIVE_ORIENTATION;
	    	    *c2_orient = NEGATIVE_ORIENTATION;
	    	}
	    }
	    else
	    {
	    	if (lcomp1 == rcomp2)
	    	{
	    	    *cross1 = cr1f;
	    	    *c1_orient = NEGATIVE_ORIENTATION;
	    	    *c2_orient = NEGATIVE_ORIENTATION;
	    	}
	    	else if (rcomp1 == lcomp2)
	    	{
	    	    *cross1 = cr1b;
	    	    *c1_orient = POSITIVE_ORIENTATION;
	    	    *c2_orient = POSITIVE_ORIENTATION;
	    	}
	    	else if (lcomp1 == lcomp2)
	    	{
	    	    *cross1 = cr1b;
	    	    *c1_orient = POSITIVE_ORIENTATION;
	    	    *c2_orient = NEGATIVE_ORIENTATION;
	    	}
	    	else if (rcomp1 == rcomp2)
	    	{
	    	    *cross1 = cr1f;
	    	    *c1_orient = NEGATIVE_ORIENTATION;
	    	    *c2_orient = POSITIVE_ORIENTATION;
	    	}
	    }
	    if (!*cross1) return NO;


	    *c11_orient = Opposite_orient(*c1_orient);

	    if ((*cross1)->c2 == cross->c2)
	    	*c12_orient = Opposite_orient(*c2_orient);
	    else
	    {
	    	vector_product_on_bonds((*cross1)->b1,(*cross1)->b2,2,&cp1);
	    	*c12_orient = (cp*cp1 > 0.0) ? *c2_orient :
					       Opposite_orient(*c2_orient);
	    		
	    }

	    return YES;
	}

	/* Determine companion cross from lengths */

	l1f = (cr1f != NULL) ?
	    distance_between_points_on_curve(cross->p,cross->b1,cr1f->p,
					     cr1f->b1,cross->c1,
					     POSITIVE_ORIENTATION) :
	    HUGE_VAL;

	l1b = (cr1b != NULL) ?
	    distance_between_points_on_curve(cross->p,cross->b1,cr1b->p,
						 cr1b->b1,cross->c1,
						 NEGATIVE_ORIENTATION) :
	    HUGE_VAL;
	
	if (l1b < l1f) 
	{
	    *cross1 = cr1b; *c1_orient = POSITIVE_ORIENTATION;
	}
	else
	{
	    *cross1 = cr1f; *c1_orient = NEGATIVE_ORIENTATION;
	}


	if (!*cross1)
	    return NO;



	if (is_closed_curve(cross->c2))
	{
	    l2f = distance_between_points_on_curve(cross->p,cross->b2,
	    	                                   (*cross1)->p,(*cross1)->b2,
						   (*cross1)->c2,
	    	                                   POSITIVE_ORIENTATION);
	    l2b = distance_between_points_on_curve(cross->p,cross->b2,
	    	                                   (*cross1)->p,(*cross1)->b2,
						   (*cross1)->c2,
	    	                                   NEGATIVE_ORIENTATION);
	    *c2_orient = (l2f < l2b) ? NEGATIVE_ORIENTATION :
				       POSITIVE_ORIENTATION;
	}
	else
	{
	    *c2_orient = (points_in_strict_order(cross->p,cross->b2,
				                 (*cross1)->p,(*cross1)->b2,
						 dim) == YES) ?
			     NEGATIVE_ORIENTATION : POSITIVE_ORIENTATION;
	}

	*c11_orient = Opposite_orient(*c1_orient);
	if ((*cross1)->c2 == cross->c2)
	    *c12_orient = Opposite_orient(*c2_orient);
	else
	{
	    vector_product_on_bonds((*cross1)->b1,(*cross1)->b2,2,&cp1);
	    *c12_orient = (cp*cp1 > 0.0) ? *c2_orient :
					   Opposite_orient(*c2_orient);
	}

	return YES;
}		/*end find_companion_cross*/

LOCAL	double distance_between_points_on_curve(
	FT_POINT		*ps,
	BOND		*bs,
	FT_POINT		*pe,
	BOND		*be,
	CURVE		*c,
	ORIENTATION	orient)
{
	FT_POINT		*p0, *p1;
	BOND		*bb;
	double		dist = HUGE_VAL;

	if (bs == be)
	    return separation(ps,pe,c->interface->dim);

	p0 = Point_of_bond(bs,Opposite_orient(orient));
	p1 = Point_of_bond(be,orient);
	dist = separation(ps,p0,c->interface->dim) +
	       separation(p1,pe,c->interface->dim);
	for (bb = Following_bond(bs,orient); bb != NULL && bb != be;
	     bb = Following_bond(bb,orient))
	{
	    dist += bond_length(bb);
	}
	if (bb == NULL && is_closed_curve(c))
	{
	    for (bb = Bond_at_node(c,orient); bb != NULL && bb != be;
		 bb = Following_bond(bb,orient))
	    {
	    	dist += bond_length(bb);
	    }
	}
	return dist;
}		/*end distance_between_points_on_curve*/

#endif /* defined(TWOD) */
