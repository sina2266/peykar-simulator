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
*				fbdry4.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	Contains code to delete all curves which leave the computational
*	domain in such a manner that intersections() does not see them.
*	This includes forms of the following topologies:
*
*			Case A
*		 _______________________
*	       /                         \
*	      /        exterior           \
*	-----x-----------------------------x-----
*		       interior
*
*
*			Case B
*		 _______________________
*	       /                         \
*	      /        exterior           \
*	     /				   \
*	----x-------x-----------------------x----
*		    ^  interior
*		    |
*		fixed node
*
*			Case B'
*	     ______________________________
*	    /				   \
*          |		exterior	    \
*	   |	x----------------------------x--------
*	   |	|
*	    \	|	interior
*	     ---x
*		|
*
*	(Clearly B and B' are of the same topology.)
*
*	This includes those curves that lie solely within a tolerance
*	limit of the interior of the boundary.
*/

#if defined(TWOD)

#include <front/fdecs.h>


	/* LOCAL Function Declarations */
LOCAL	boolean	is_loop_of_bdry_like_curves(CURVE*,ORIENTATION,
					    ANGLE_DIRECTION,double*,
					    boolean*,boolean*);
LOCAL	boolean	is_pt_near_bdry_like_curve(INTERFACE*,double,double,double,double);
LOCAL	boolean	nodes_of_c_on_same_bdry_like_path(CURVE*);
LOCAL	void	nearest_bdry_like_point(double,double,INTERFACE*,double*,double*,
					double*,BOND**,CURVE**);
LOCAL	void	short_dist(double,double,BOND*,FT_POINT**,double*,double*,SIDE*);
LOCAL	void	transfer_states_cp_to_cb(CURVE*,SIDE,CURVE*,ORIENTATION,
					 SIDE,Front*);

EXPORT	void f_delete_exterior_curves(
	Front		*fr,
	INTERFACE	*old_intfc)
{
	INTERFACE	*intfc = fr->interf;
	RECT_GRID	*rgr = fr->rect_grid;
	CURVE		**cc, *c;
	NODE		*ns, *ne;
	double		*h = rgr->h;

	for (cc = intfc->curves;  cc && *cc;  cc++)
	{
	    c = *cc;
	    ns = c->start;
	    ne = c->end;
	    if ((is_bdry_like_curve(c)) ||
	        (wave_type(c) >= FIRST_VECTOR_PHYSICS_WAVE_TYPE) ||
	        (! is_bdry_like_node(ns)) ||
		is_subdomain_node(ns) ||
	        (! is_bdry_like_node(ne)) ||
		is_subdomain_node(ne))
		continue;

	    if ((all_pts_on_c_are_ext_to_rect(c,rgr) == YES) ||
	        (all_pts_of_c_are_exterior_like(c,old_intfc)) ||
	        (c_parallel_to_bdry_like_curves(c,h[0],h[1]) == YES))
	    {
	        if (ns == ne)	/* bubble attached to boundary flows out */
	        {
		    (void) delete_curve(c);
		    (void) delete_redundant_node(ns,NULL,NULL,fr);
	        }
	        else
	        {
	    	    CURVE		*bc;
	    	    ORIENTATION	c_or, bc_or;
	    	    boolean		is_b1, is_b2;
	    	    boolean		is_b1_sub, is_b2_sub;
	    	    double		b1_len, b2_len;

	    	    c_or = POSITIVE_ORIENTATION;
	    	    if (!shortest_connecting_bdry_like_path(c,c_or,&bc,&bc_or,
						       &is_b1,&is_b1_sub,
						       &b1_len,
						       &is_b2,&is_b2_sub,
						       &b2_len))
		    {
		        (void) printf("WARNING in f_delete_exterior_curves(), "
				      "shortest_connecting_bdry_like_path "
				      "return error\n");
		        (void) printf("for curve\n");
		        print_curve(c);
		        continue;
		    }

		    if (bc == NULL)
		       continue;
		    if (((is_b1==YES && is_b1_sub==YES) && (b1_len < b2_len))
			    			    ||
		        ((is_b2==YES && is_b2_sub==YES) && (b2_len < b1_len)))
			    continue;

		    shift_c_states_to_bdry_curves(c,bc,fr);
		    (void) delete_curve(c);
		    (void) delete_redundant_node(ns,NULL,NULL,fr);
		    (void) delete_redundant_node(ne,NULL,NULL,fr);
	        }
		cc = intfc->curves - 1;/*Restart loop*/
	    }
	}
}		/*end f_delete_exterior_curves*/

/*
*			f_delete_phys_remn_on_bdry():
*
*	Delete interior curves of scalar_wave_type that are remnants
*	(defined as 1 or 2 bond curves), and are attached at both
*	ends to NEUMANN_BOUNDARY curves.
*/

EXPORT	boolean f_delete_phys_remn_on_bdry(
	Front		*fr)
{
	CURVE		**c, *tc, *bc;
	INTERFACE	*intfc = fr->interf;
	NODE		*ns, *ne;
	double		hx, hy, b1_len, b2_len;
	double		tx, ty, tl, tclen, tol;
	ORIENTATION	tc_or, bc_or;
	boolean		is_b1, is_b2;
	boolean		is_b1_sub, is_b2_sub;

	hx = fr->rect_grid->h[0];
	hy = fr->rect_grid->h[1];
	for (c = intfc->curves;  c && *c;  c++)
	{
	    tc = *c;
	    if (wave_type(tc) < FIRST_PHYSICS_WAVE_TYPE)
		continue;
	    if (wave_type(tc) > FIRST_VECTOR_PHYSICS_WAVE_TYPE)
		continue;

	    ns = tc->start;
	    ne = tc->end;

	    if (node_type(ns) != node_type(ne))
		continue;
	    if (node_type(ns) != NEUMANN_NODE)
		continue;
	    if (ns == ne)
		continue;

	    tclen = curve_length(tc);
	    tx = fabs(Coords(ne->posn)[0] - Coords(ns->posn)[0]);
	    ty = fabs(Coords(ne->posn)[1] - Coords(ns->posn)[1]);
	    tl = hypot(tx,ty);
	    tx = tx/tl;
	    ty = ty/tl;
	    tol = (tl < min(hx,hy)) ? max(hx,hy) : tx*hx + ty*hy;/*TOLERANCE*/

	    if (tclen < tol)
	    {
		tc_or = POSITIVE_ORIENTATION;

		if (!shortest_connecting_bdry_like_path(tc,tc_or,&bc,&bc_or,
			                           &is_b1,&is_b1_sub,&b1_len,
						   &is_b2,&is_b2_sub,&b2_len))
		    return NO;

		if (bc == NULL)
		    continue;

		if (((b1_len <= b2_len) && (b1_len > 2.0 * tclen)) ||
		    ((b2_len <= b1_len) && (b2_len > 2.0 * tclen)))
		    continue;

		if (((is_b1==YES && is_b1_sub==YES) && (b1_len < b2_len))
		    			||
		    ((is_b2==YES && is_b2_sub==YES) && (b2_len < b1_len)))
		    continue;

		/** delete_interior_points_of_curve(tc); **/

		shift_c_states_to_bdry_curves(tc,bc,fr);
		(void) delete_curve(tc);

		(void) delete_redundant_node(ns,NULL,NULL,fr);
		(void) delete_redundant_node(ne,NULL,NULL,fr);

		c = intfc->curves - 1;/*Restart loop*/
	    }
	}
	return YES;
}		/*end f_delete_phys_remn_on_bdry*/


EXPORT	boolean all_pts_on_c_are_ext_to_rect(
	CURVE	 	*c,
	RECT_GRID 	*rgr)
{
	INTERFACE	   *intfc = c->interface;
	BOND		   *b;
	FT_POINT		   *p, *ps, *pe;
	double		   *h = rgr->h, *L = rgr->VL, *U = rgr->VU;
	double		   xl, yl, xu, yu;
	double		   x, y,  xd, yd, xh, yh;
	double		   xltol, yltol, xutol, yutol;
	static const double tol = 0.0001;/*TOLERANCE*/

	xl = L[0];	xu = U[0];
	yl = L[1];	yu = U[1];
	xltol = (!buffered_boundary_type(rect_boundary_type(intfc,0,0))) ?
		tol*h[0] : 0.0;/*TOLERANCE*/
	xutol = (!buffered_boundary_type(rect_boundary_type(intfc,0,1))) ?
		tol*h[0] : 0.0;/*TOLERANCE*/
	yltol = (!buffered_boundary_type(rect_boundary_type(intfc,1,0))) ?
		tol*h[1] : 0.0;/*TOLERANCE*/
	yutol = (!buffered_boundary_type(rect_boundary_type(intfc,1,1))) ?
		tol*h[1] : 0.0;/*TOLERANCE*/

	xd = xl + xltol;			yd = yl + yltol;
	xh = xu - xutol;			yh = yu - yutol;

	for (b = c->first;  b != c->last;  b = b->next)
	{
	    p = b->end;
	    x = Coords(p)[0];
	    y = Coords(p)[1];
	    if ((xl < x) && (x < xu) && (yl < y) && (y < yu))
	        return NO;
	}

	    /* Special case is single bond curves. Both nodes must */
	    /* lie completely outside, or lie on the same boundary */
	    /*		 for it to be declared exterior		   */

	if (c->first == c->last)
	{
	    ps = c->start->posn;	pe = c->end->posn;
	    if (   (xl - xltol < Coords(ps)[0]) && (Coords(ps)[0] < xd)
	        && (xl - xltol < Coords(pe)[0]) && (Coords(pe)[0] < xd))
		return YES;

	    if (   (xh < Coords(ps)[0]) && (Coords(ps)[0] < xu + xutol)
	        && (xh < Coords(pe)[0]) && (Coords(pe)[0] < xu + xutol))
		return YES;

	    if (   (yl - yltol < Coords(ps)[1]) && (Coords(ps)[1] < yd)
	        && (yl - yltol < Coords(pe)[1]) && (Coords(pe)[1] < yd))
		return YES;

	    if (   (yh < Coords(ps)[1]) && (Coords(ps)[1] < yu + yutol)
	        && (yh < Coords(pe)[1]) && (Coords(pe)[1] < yu + yutol))
		return YES;
	    return NO;
	}
	return YES;
}		/*end all_pts_on_c_are_ext_to_rect*/


/*
*	Tests if all points on a curve lie in an exterior component
*	Assumes that c is a propagated curve, corresponding to an
*	unpropagated curve in an old, unmodified intfc which has
*	existing bond_comp_lists. If either there is no corresponding
*	curve or the old_intfc is modified, the status ERROR is returned.
*/

EXPORT	boolean all_pts_of_c_are_exterior_like(
	CURVE		*c,
	INTERFACE	*old_intfc)
{
	BOND 		*b;
	FT_POINT		*p;
	COMPONENT	comp;

	if (old_intfc->modified)
	    return NO;

	for (b = c->first;  b != c->last;  b = b->next)
	{
	    p = b->end;
	    comp = component(Coords(p),old_intfc);
	    if (! is_excluded_comp(comp,old_intfc))
	    	return NO;
	}
	return YES;
}		/*end all_pts_of_c_are_exterior_like*/


/*
*			c_parallel_to_bdry_like_curves():
*
*	Checks to see that a curve is essentially parallel to a
*	boundary like curve. Each point on c is checked to see
*	if it lies within a tolerance of a boundary like curve.
*	The curve must also pass the additional test that the start
*	and end points of the curve belong to the same boundary,
*	by which it is meant that one can travel from the start node
*	of the curve to the end node of the curve along a path
*	solely consisting of boundary curves.
*/

EXPORT	boolean c_parallel_to_bdry_like_curves(
	CURVE		*c,
	double		hx,
	double		hy)

{
	BOND		*b;
	FT_POINT		*ps, *pe;
	double		x, y, xtol, ytol;
	boolean		is_nr;
	static const double tol = 0.001;/*TOLERANCE*/

	xtol = tol*hx;/*TOLERANCE*/
	ytol = tol*hy;/*TOLERANCE*/

	b = c->first;
	if (b == c->last)
	{
	    ps = b->start;			pe = b->end;
	    x = 0.5 * (Coords(ps)[0] + Coords(pe)[0]);
	    y = 0.5 * (Coords(ps)[1] + Coords(pe)[1]);
	    is_nr = is_pt_near_bdry_like_curve(c->interface,x,y,xtol,ytol);
	    if (is_nr == NO)
	    	return NO;
	}
	else
	{
	    if (nodes_of_c_on_same_bdry_like_path(c) != YES)
		return NO;
	    for (b = c->first;  b != c->last;  b = b->next)
	    {
		pe = b->end;
		x = Coords(pe)[0];
		y = Coords(pe)[1];
		is_nr = is_pt_near_bdry_like_curve(c->interface,x,y,xtol,ytol);
		if (is_nr == NO)
		    return NO;
	    }
	}
	return YES;
}		/*end c_parallel_to_bdry_like_curves*/



LOCAL	boolean is_pt_near_bdry_like_curve(
	INTERFACE	*intfc,
	double		x,
	double		y,
	double		tolx,
	double		toly)
{
	CURVE		*bc;
	BOND		*bb;
	FT_POINT		*ps, *pe;
	double		tol, dist;
	double		bx, by, bt;
	double		tx, ty, nx, ny, blen;

	nearest_bdry_like_point(x,y,intfc,&bx,&by,&bt,&bb,&bc);
	if (bb == NULL || bc == NULL)
	    return NO;

	dist = hypot(x-bx,y-by);
	ps = bb->start;
	pe = bb->end;
	blen = bond_length(bb);
	tx = (Coords(pe)[0] - Coords(ps)[0])/blen;
	ty = (Coords(pe)[1] - Coords(ps)[1])/blen;
	nx = fabs(ty);			ny = fabs(tx);
	tol = fabs(nx*tolx + ny*toly);
	return (dist > tol) ? NO : YES;
}		/*end is_pt_near_bdry_like_curve*/


LOCAL	boolean nodes_of_c_on_same_bdry_like_path(
	CURVE		*c)
{
	INTERFACE	*intfc = c->interface;
	NODE		*ns, *ne, *bcn;
	CURVE		*bc;
	BOND		*bb;
	double		x, y, bx, by, bt;
	ORIENTATION	bc_orient, opp_bc_orient;
	int    		i;
	size_t	        max_c;

	ns = c->start;			ne = c->end;

	if (ns == ne)
	    return YES;

	x = Coords(ns->posn)[0];		y = Coords(ns->posn)[1];
	nearest_bdry_like_point(x,y,c->interface,&bx,&by,&bt,&bb,&bc);
	bc_orient = (bc->start == ns) ? POSITIVE_ORIENTATION
				      : NEGATIVE_ORIENTATION;
	opp_bc_orient = Opposite_orient(bc_orient);
	bcn = Node_of(bc,opp_bc_orient);
	i = 0;
	max_c = Num_curves(intfc);
	while(i++ < max_c)	/* Prevent infinite loops */
	{
    	    /* ns test must precede ne to include closed curves c */

    	    if (bcn == ns)
	        return NO;
	    if (bcn == ne)
		return YES;
	    if (!next_boundary(bc,opp_bc_orient,&bc,&bc_orient))
	    {
		screen("ERROR in nodes_of_c_on_same_bdry_like_path(), "
		       "next_boundary() failed\n");
		(void) printf("curve c\n");
		print_curve(c);
		if (bc != NULL)
		{
		    print_orientation("bc, bc_orient = ",opp_bc_orient,"\n");
		    print_curve(bc);
		}
		print_interface(intfc);
		clean_up(ERROR);
	    }
	    opp_bc_orient = Opposite_orient(bc_orient);
	    bcn = Node_of(bc,opp_bc_orient);
	}
	screen("ERROR in nodes_of_c_on_same_bdry_like_path(), "
	       "Possible infinite loop found\n"
	       "\t\tTerminating run\n");
	(void) printf("curve c\n");
	print_curve(c);
	print_interface(intfc);
	clean_up(ERROR);
	return NO;				/* Fools lint */
}		/*end nodes_of_c_on_same_bdry_like_path*/


/*
*		shortest_connecting_bdry_like_path():
*
*	Tries to find a path of boundary like curves connecting the end
*	points of pc.  Returns:
*		bc	  pointer to first curve in shorter loop
*		bc_or	  ang dir from pc to bc
*		is_cw	  is there a clockwise loop?
*		is_cw_sub the clockwise loop is a subdomain boundary
*		is_cc	  is there a counter clockwise loop?
*		is_cw_sub the counter clockwise loop is a subdomain boundary
*		cw_len	  len of cw loop
*		cc_len	  len of cc loop
*/

EXPORT	boolean shortest_connecting_bdry_like_path(
	CURVE		*pc,
	ORIENTATION	pc_or,
	CURVE		**bc,
	ORIENTATION	*bc_or,
	boolean		*is_cw,
	boolean		*is_cw_sub,
	double		*cw_len,
	boolean		*is_cc,
	boolean		*is_cc_sub,
	double		*cc_len)
{
	if (!is_loop_of_bdry_like_curves(pc,pc_or,CLOCKWISE,cw_len,
					 is_cw_sub,is_cw)
	    ||
	    !is_loop_of_bdry_like_curves(pc,pc_or,COUNTER_CLOCK,cc_len,
					 is_cc_sub,is_cc))
	{
	    (void) printf("WARNING shortest_connecting_bdry_like_path() "
			  "failed\n");
	    *bc = NULL;
	    return NO;
	}
	else if ((*cw_len <= *cc_len) && (*is_cw != NO))
	    *bc = adjacent_curve(pc,pc_or,CLOCKWISE,bc_or);
	else if ((*cc_len <= *cw_len) && (*is_cc != NO))
	    *bc = adjacent_curve(pc,pc_or,COUNTER_CLOCK,bc_or);
	else
	    *bc = NULL;
	return YES;
}		/*end shortest_connecting_bdry_like_path*/


/*
*			shift_c_states_to_bdry_curves():
*
*	The states on the appropriate side of curve c are ft_assigned to the
*	boundary curve chain starting with curve cb_in.
*/

EXPORT	void shift_c_states_to_bdry_curves(
	CURVE		*cp,
	CURVE		*cb_in,
	Front		*fr)
{
	INTERFACE	*intfc = cp->interface;
	NODE		*ns, *ne, *nbe;
	CURVE		*cb_last;
	CURVE		*cb = cb_in;
	int		i;
	ORIENTATION	cp_or, cb_or, cb_opp_or;
	size_t	        max_c;

	max_c = Num_curves(intfc);
	ns = cp->start;			ne = cp->end;

	cp_or = POSITIVE_ORIENTATION;
	cb_or = (ns == cb->start) ? POSITIVE_ORIENTATION
				    : NEGATIVE_ORIENTATION;

	i = 0;    			/* i count prevents infinite loops */
	while(cb != NULL)
	{
	    if (i++ >= max_c)
	    {
	        screen("ERROR in shift_c_states_to_bdry_curves(), "
	               "Possible infinite loop flagged\n"
	               "\t\tTerminating run\n");
	        clean_up(ERROR);
	    }
	    if (is_excluded_comp(negative_component(cb),intfc))
	    {
	        if (cp_or == cb_or)
	    	    transfer_states_cp_to_cb(cp,POSITIVE_SIDE,cb,cb_or,
					     POSITIVE_SIDE,fr);
		else
		    transfer_states_cp_to_cb(cp,NEGATIVE_SIDE,cb,cb_or,
					     POSITIVE_SIDE,fr);
	    }
	    else
	    {
	        if (cp_or == cb_or)
	    	    transfer_states_cp_to_cb(cp,NEGATIVE_SIDE,cb,cb_or,
					     NEGATIVE_SIDE,fr);
	        else
	    	    transfer_states_cp_to_cb(cp,POSITIVE_SIDE,cb,cb_or,
					     NEGATIVE_SIDE,fr);
	    }

	    cb_opp_or = Opposite_orient(cb_or);
	    nbe = Node_of(cb,cb_opp_or);
	    if (nbe == ne)
	        cb = NULL;
	    else
	    {
	        cb_last = cb;
	        if (!next_boundary(cb_last,cb_opp_or,&cb,&cb_or))
	        {
	            (void) printf("WARNING in shift_c_states_to_bdry_curves(), "
	    		      "next_boundary() failed  cp %llu cb_last %llu\n",
	    		      curve_number(cp),curve_number(cb_last));
	    	    print_interface(intfc);
	    	    return;
	        }
	    }
	}
}		/*end shift_c_states_to_bdry_curves*/



/*
*			transfer_states_cp_to_cb():
*
*	Transfers states from cp_side of curve cp to cb_side of curve cb.
*	The transfer is done onto the points of cb from locations of cp
*	obtained by long_nearest_interface_point().
*	In addition to states, the curve component values are also transferred.
*
*	NOTE: there is a similar routine in fbdry1.c.
*/

LOCAL	void transfer_states_cp_to_cb(
	CURVE		*cp,
	SIDE		cp_side,
	CURVE		*cb,
	ORIENTATION	cb_orient,
	SIDE		cb_side,
	Front		*fr)
{
	HYPER_SURF	*hs;
	HYPER_SURF_ELEMENT *hse;
	CURVE		*ppc;
	BOND		*bs, *be, *b, *bp;
	FT_POINT		*p;
	double		coords[MAXD], t;
	ORIENTATION	cb_opp_orient;

	cb_opp_orient = Opposite_orient(cb_orient);
	
	p = Node_of(cb,cb_orient)->posn;
	if (long_nearest_interface_point(Coords(p),negative_component(cp),
					 cp->interface,NO_BOUNDARIES,
					 Hyper_surf(cp),coords,&t,
					 &hse,&hs) != YES)
	{
	    screen("ERROR in transfer_states_cp_to_cb(), "
		   "long_nearest_interface_point failed\n");
	    clean_up(ERROR);
	}
	bp = Bond_of_hse(hse);
	ppc = Curve_of_hs(hs);
	if (cp_side == NEGATIVE_SIDE)
	{
	    if (cb_side == NEGATIVE_SIDE)
	    {
		left_state_along_bond(t,bp,ppc,
				      Left_state_at_node(cb,cb_orient));
		negative_component(cb) = negative_component(cp);
	    }
	    else
	    {
		left_state_along_bond(t,bp,ppc,
				      Right_state_at_node(cb,cb_orient));
		positive_component(cb) = negative_component(cp);
	    }
	}
	else
	{
	    if (cb_side == NEGATIVE_SIDE)
	    {
		right_state_along_bond(t,bp,ppc,
				       Left_state_at_node(cb,cb_orient));
		negative_component(cb) = positive_component(cp);
	    }
	    else
	    {
		right_state_along_bond(t,bp,ppc,
				       Right_state_at_node(cb,cb_orient));
		positive_component(cb) = positive_component(cp);
	    }
	}


	bs = Bond_at_node(cb,cb_orient);
	be =  Bond_at_node(cb,cb_opp_orient);
	for (b = bs;  b != be;  b = Following_bond(b,cb_orient))
	{
	    p = Point_of_bond(b,cb_opp_orient);
	    if (long_nearest_interface_point(Coords(p),negative_component(cp),
				             cp->interface,NO_BOUNDARIES,
					     Hyper_surf(cp),coords,
					     &t,&hse,&hs) != YES)
	    {
		screen("ERROR in transfer_states_cp_to_cb(), "
		       "long_nearest_interface_point() failed\n");
		clean_up(ERROR);
	    }
	    bp = Bond_of_hse(hse);
	    ppc = Curve_of_hs(hs);
		
	    if (cp_side == NEGATIVE_SIDE)
	    {
	        if (cb_side == NEGATIVE_SIDE)
	    	    left_state_along_bond(t,bp,ppc,left_state(p));
	        else
	    	    left_state_along_bond(t,bp,ppc,right_state(p));
	    }
	    else
	    {
	        if (cb_side == NEGATIVE_SIDE)
	    	    right_state_along_bond(t,bp,ppc,left_state(p));
	        else
	    	    right_state_along_bond(t,bp,ppc,right_state(p));
	    }
	}

	p = Node_of(cb,cb_opp_orient)->posn;
	if (long_nearest_interface_point(Coords(p),negative_component(cp),
					 cp->interface,NO_BOUNDARIES,
					 Hyper_surf(cp),coords,
					 &t,&hse,&hs) != YES)
	{
	    screen("ERROR in transfer_states_cp_to_cb(), "
		   "long_nearest_interface_point() failed\n");
	    clean_up(ERROR);
	}
	bp = Bond_of_hse(hse);
	ppc = Curve_of_hs(hs);
	if (cp_side == NEGATIVE_SIDE)
	{
	    if (cb_side == NEGATIVE_SIDE)
		left_state_along_bond(t,bp,ppc,
				      Left_state_at_node(cb,cb_opp_orient));
	    else
		left_state_along_bond(t,bp,ppc,
				      Right_state_at_node(cb,cb_opp_orient));
	}
	else
	{
	    if (cb_side == NEGATIVE_SIDE)
		right_state_along_bond(t,bp,ppc,
				       Left_state_at_node(cb,cb_opp_orient));
	    else
		right_state_along_bond(t,bp,ppc,
				       Right_state_at_node(cb,cb_opp_orient));
	}

	/*
	*  If cb is a DIRICHLET BOUNDARY,  assign the states on
	*  on the interior side of cb to the exterior side as well
	*/

	if (wave_type(cb) == DIRICHLET_BOUNDARY)
	{
	    Locstate st_l, st_r;

	    st_l = left_start_state(cb);
	    st_r = right_start_state(cb);
	    if (cb_side == NEGATIVE_SIDE)
	    	obstacle_state(fr->interf,st_r,fr->sizest);
	    else
	    	obstacle_state(fr->interf,st_l,fr->sizest);

	    for (b = cb->first; b != cb->last; b = b->next)
	    {
	    	st_l = left_state(b->end);
	    	st_r = right_state(b->end);
	    	if (cb_side == NEGATIVE_SIDE)
	    	    obstacle_state(fr->interf,st_r,fr->sizest);
	    	else
	    	    obstacle_state(fr->interf,st_l,fr->sizest);
	    }

	    st_l = left_end_state(cb);
	    st_r = right_end_state(cb);
	    if (cb_side == NEGATIVE_SIDE)
	    	obstacle_state(fr->interf,st_r,fr->sizest);
	    else
	        obstacle_state(fr->interf,st_l,fr->sizest);
	}
}		/*end transfer_states_cp_to_cb*/


/*
*		is_loop_of_boundary_like_curves():
*
*	Other than the input curve pc, are all other curves in the
*	loop defined by "dir" at Node_of(pc,orient) boundary-like curves?
*/

LOCAL	boolean is_loop_of_bdry_like_curves(
	CURVE		*pc,
	ORIENTATION	orient,
	ANGLE_DIRECTION	dir,
	double		*len,
	boolean         *is_subd,
	boolean         *pans)
{
	CURVE		*nc;
	ORIENTATION	nc_or;
	boolean		ans;

	*is_subd = NO;
	ans = YES;		*len = 0.0;
	nc = pc;		nc_or = orient;
	while((nc = adjacent_curve(nc,nc_or,dir,&nc_or)) != pc)
	{
	    if (nc == NULL)
	    {
	       (void) printf("WARNING in is_loop_of_bdry_like_curves(), "
	                     "adjacent_curve() returns NULL\n"
	                     "Posssible inconsistent components\n");
	       *pans = NO;
	       return NO;
	    }
	    *len = *len + curve_length(nc);
	    if (! is_bdry_like_curve(nc))
		ans =  NO;
	    if (is_subdomain_boundary(Hyper_surf(nc)))
		*is_subd = YES;
	    nc_or = Opposite_orient(nc_or);
	}
	*pans = ans;
	return YES;
}		/*end is_loop_of_bdry_like_curves*/





/*
*			nearest_bdry_like_point():
*
*	Given a point x,y, locates the closest bdry-like point px,py of
*	the INTERFACE intfc.
*	Also returns the bond B and curve C containing p, and the parametric
*	location of the point on this bond.
*	Returns value 1 or 0 if succesful or not in finding a closest point.
*/

LOCAL	double	t_last;

LOCAL	void nearest_bdry_like_point(
	double		x,
	double		y,
	INTERFACE	*intfc,
	double		*px,
	double		*py,
	double		*t,
	BOND		**B,
	CURVE		**C)
{
	CURVE		*c_closest = NULL;
	BOND		*b_closest = NULL;
	FT_POINT		*p_closest = NULL;
	FT_POINT		*p;
	BOND		*b;
	CURVE		**c;
	SIDE		side;	/* Side of interface bond - 
				   either POSITIVE_SIDE or NEGATIVE_SIDE */
	double		distance;	/* Distance from (x,y) to a Bond */
	double		min_distance;	/* Distance to Nearest Bond */
	double		norm_dist;	/* Normal Distance to a Bond */
	double		min_norm_dist;	/* Normal Distance to Nearest Bond */
	double		t_closest;

			/* Find Closest Point on Front */

	min_distance = HUGE_VAL;
	min_norm_dist = HUGE_VAL;
	for (c = intfc->curves;  *c;  c++)
	{
	    if (! is_bdry_like_curve(*c))
		    continue;

	    for (b = (*c)->first;  b  ; b = b->next)
	    {
	    	short_dist(x,y,b,&p,&distance,&norm_dist,&side);

	    	if (p != NULL && p == p_closest)
	    	{
	
		/*							*
		*							*
		*   Whenever p!=NULL the point x,y is beyond the end of *
		*   the bond.   Thus this is the case where x,y is      *
		*   beyond the end of two adjacent bonds, with their    *
		*   intersection being the closest point to x,y.        *
		*							*
		*   The side is in general ambiguous.			*
		*							*
		*   The correct bond wrt which the side should be com-  *
		*   puted is that which has the larger normal distance  *
		*   from x,y.						*
		*							*
		*   The following code assumes that both bonds belong	*
		*   to the same curve.					*
		*							*/



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

	*B = b_closest;
	*C = c_closest;
	if (b_closest==NULL)
	    return;

	*px = (1.0 - t_closest)*Coords(b_closest->start)[0] + 
		     t_closest *Coords(b_closest->end)[0];
	*py = (1.0 - t_closest)*Coords(b_closest->start)[1] + 
		     t_closest *Coords(b_closest->end)[1];
	*t  = t_closest;
	return;
}		/*end nearest_bdry_like_point*/



/*
*				short_dist():
*
*	This is a copy of shortest_distance() in intfc/comp.c
*
*	Computes the Shortest Distance Squared From point (x,y) to the inter-
*	face BOND b.    
*
*	Determines on which side of BOND b the point (x,y) lies.  
*
*	Also computes the normal distance squared, the distance to the 
*	infinite line through the bond.
*
*	In case the BOND has zero length, distance is set equal to 1000000.
*
*	The paramater  t  below is used to find the parametric 
*	equation of the interface BOND starting at POINT b->start. 
*/

#define  dist(t)   sqr(t*x21 - x_1) + sqr(t*y21 - y_1)
#define  EPS3 	   (EPSILON*EPSILON*EPSILON)


LOCAL	void short_dist(
	double		x,
	double		y,
	BOND		*b,
	FT_POINT		**p,
	double		*distance,
	double		*norm_dist,
	SIDE		*side)
{
	double		x1 = Coords(b->start)[0], y1 = Coords(b->start)[1];
	double		x2 = Coords(b->end)[0],   y2 = Coords(b->end)[1];
	double		y21 = y2-y1,		  x21 = x2-x1;
	double		x_1 = x - x1,		  y_1 = y - y1;
	double		scalar_prod;
	double		t;	/* Paramater for Equation of Line  1 -> 2 */
	double		l;	/* squared length of bond */

		/* Compute Vector Product to get Side: */
	
	*side = (y_1*x21 - x_1*y21 >= 0.0) ? NEGATIVE_SIDE : POSITIVE_SIDE;

	*p = NULL;
	scalar_prod = x21*x_1 + y21*y_1;

	if ((l = x21*x21 + y21*y21) <= EPS3)
	{
	    if (l == 0.0)
	    {
	        *distance = HUGE_VAL;
	        return;
	    }
	    else if (l <= EPS3*scalar_prod)
	    {
	        *distance = HUGE_VAL;
	        return;
	    }
	}

	t =  scalar_prod/l;

	*distance = *norm_dist = (double)dist(t);
	if (t >= 1.0)
	{
	    *distance = (sqr(x - x2) + sqr(y - y2));
	    *p = b->end;
	    t_last = 1.0;
	}
	else if (t <= 0.0)
	{
	    *distance = (double)(sqr(x_1) + sqr(y_1));
	    *p = b->start;
	    t_last = 0.0;
	}
	else
		t_last = t;
}		/*end short_dist*/

#endif /* defined(TWOD) */
