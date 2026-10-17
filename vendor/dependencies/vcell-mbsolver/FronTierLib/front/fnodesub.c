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
*				fnodesub.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	Contains the routines
*
*			find_propagation_orientation()
*			find_physical_curve_at_node()
*			cut_curve()
*			shift_node()
*			shift_node_past()
*			assign_interacting_states()
*			insert_point_adjacent_to_node()
*			init_redundant_node_for_deletion()
*			unpropagate_node()
*			find_tangent_to_curve()
*			find_tangent_to_propagated_curve()
*			bond_secant_to_curve()
*			propagated_tangent_bond_at_node()
*			modify_B_node()
*
*	to be used by the node propagation routines.
*/


#include <front/fdecs.h>

	/* LOCAL Function Declarations */
#if defined(TWOD)
LOCAL	void	move_node(NODE*,double*);
#endif /* defined(TWOD) */

#if defined(TWOD) || defined(THREED)
/*
*			find_tangent_to_curve():
*
*	This routine returns a unit tangent vector at the point p on the bond b
*	on curve c.
*/

EXPORT	void find_tangent_to_curve(
	FT_POINT		*p,
	BOND		*b,
	CURVE		*c,
	ORIENTATION	orient,
	double		*t,
	Front		*fr)
{
	static	BOND	*bdir = NULL;
	int		i, dim = fr->rect_grid->dim;

	if (bdir == NULL)
	{
	    scalar(&bdir,sizeof(BOND));
	    bdir->start = Static_point(fr->interf);
	    bdir->end = Static_point(fr->interf);
	}

	bond_tangent_to_curve(p,b,c,orient,bdir,fr);
	if (orient == POSITIVE_ORIENTATION)
	{
	    for (i = 0; i < dim; ++i)
	    {
	        t[i] = (Coords(bdir->end)[i] - Coords(bdir->start)[i]) /
				bond_length(bdir);
	    }
	}
	else
	{
	    for (i = 0; i < dim; ++i)
	    {
	        t[i] = (Coords(bdir->start)[i] - Coords(bdir->end)[i]) /
				bond_length(bdir);
	    }
	}
}		/*end find_tangent_to_curve*/

/*
*			bond_secant_to_curve():
*
*	Given a point p on the bond b of a curve c, this routine returns a 
*	bond with nontrivial length which agrees with the given bond at the 
*	point p.
*
*	INPUT:
*	p, b, and c	- the input point, bond and curve.  b may be NULL,
*			  indicating end of curve.  p and c may not be NULL.
*	orient		- indicates the direction to step along the curve.
*	ds		- specifies the length of the result (bond).  If
*			  ds is not specified (ds == 0.0 initially), ds
*			  is given an arbitrary value.  In this case, the
*			  result must be at least as long as the first bond
*			  in the direction orient.  See notes below.
*			  A negative value of ds indicates that the result
*			  will point in the opposite direction of the curve.
*	OUTPUT:
*	bdir		- the answer.  It is assumed this is a valid bond,
*			  i.e. that that the start and end points have been
*			  allocated.  The "start" point (wrt orient) will
*			  agree with the input point p.
*
*	Note: The Res code, having internal bdries of rather arbitrary shape
*	has shown that the branch of this routine reached by the test  
*	`while(scaled_length < ds)' is fail safe ONLY IF the computational
*	domain is simply connected with concave boundaries.  Preferably,
*	this routine should ONLY return the tangent to the given bond b
*	regardless of its (nonzero) length.  As a temporary comprimise, we
*	allow ds to have a small, non_zero value.
*       Too large:  ds = 0.5*Front_spacing(fr,GENERAL_WAVE);
*	This is only an issue when ds is not given (ds == 0.0 initially).
*
*	Note to Note:  This question is more complex than the above comment
*	suggests.  The need to be able to ignore bonds of length less than
*	some tolerance is needed to avoid stability problems due to kinks
*	on physical caused by short bonds.  A more reasonable approach is to
*	use different tolerances for physical curves than for boundary curves.
*	Another approach would be have a field in the bond structure that
*	stores the "fixed" tangent of a bond on a fixed boundary.  A third
*	approach is to allow this calculation to proceed onto the next
*	boundary curve when the bond is too short.
*
*	Note to Note to Note:  There are instances in the gas code where we
*	wish to set the tangent to a curve at a node based on some
*	computation (see notes before adjust_angle_at_node()).  Thus in every
*	such instance, we need to know the default value of ds below.  Unless
*	this is standardized in some way, expect problems if and when the
*	default is changed.
*/

#define at_beginning(p,b,orient)					\
	(  (orient == POSITIVE_ORIENTATION && p == b->start)		\
 	|| (orient == NEGATIVE_ORIENTATION && p == b->end))

EXPORT void bond_secant_to_curve(
	FT_POINT		*p,
	BOND		*b,
	CURVE		*c,
	ORIENTATION	orient,
	BOND		*bdir,
	Front		*fr,
	double		ds)
{
	BOND		*curr_b;
	BOND		*follower;	/* follower of curr_b wrt curr_or */
	INTERFACE	*intfc = c->interface;
	double		*p1, *p2;
	double		*h = computational_grid(intfc)->h;
	double		scaled_length;
	double		alpha;
	double		*pt;
	double		*ps = Coords(bdir->start), *pe = Coords(bdir->end);
	ORIENTATION	opp_or;
	int		on_first_bond = YES;
	int		neg_ds = NO;
	int		ds_given = YES;
	int		i, dim = intfc->dim;

	if (ds < 0.0)
	{
	    neg_ds = YES;
	    ds = -ds;
	    orient = Opposite_orient(orient);
	}
	else if (ds == 0.0)	/* see notes at top of function */
	{
	    ds_given = NO;
	    ds = 0.1*Front_spacing(fr,GENERAL_WAVE);/*TOLERANCE*/
	}

	opp_or = Opposite_orient(orient);
	curr_b = (b == NULL || at_beginning(p,b,orient)) ?
					b : Following_bond(b,orient);

	if (orient == POSITIVE_ORIENTATION)
	{
	    bdir->prev = (curr_b != NULL) ? curr_b->prev : b;
	    for (i = 0; i < dim; ++i)
		ps[i] = Coords(p)[i];
	}
	else
	{
	    bdir->next = (curr_b != NULL) ? curr_b->next : b;
	    for (i = 0; i < dim; ++i)
		pe[i] = Coords(p)[i];
	}

	if (curr_b == NULL) 	/* at termination of curve */
	{
	    if (!is_closed_curve(c))
	    {
	        if (b != NULL)
	        {
	            if (!ds_given)
			ds = 0.0;
	            bond_secant_to_curve(p,b,c,opp_or,bdir,fr,ds);
		    if (!neg_ds)
			invert_bond(bdir);

		    if (orient == POSITIVE_ORIENTATION) 
			bdir->next = NULL;
		    else 
		       	bdir->prev = NULL;
		    return;
		}
		goto no_continuation;
	    }

	    curr_b = Bond_at_node(c,orient);
	}

	while ((scaled_length = scaled_bond_length(curr_b,h,dim)) < ds) 
	{
	    ds -= scaled_length;
	    follower = Following_bond(curr_b,orient);
	    if (follower == NULL) 	/* at termination of curve */
	    {
	    	if (!is_closed_curve(c))
	    	    goto no_continuation;

		curr_b = Bond_at_node(c,orient);
	    }
	    else
		curr_b = follower;
	    on_first_bond = NO;
	}

	if (!ds_given && on_first_bond)
	{
	    /* go at least one bond length */

	    p1 = Coords(Point_of_bond(curr_b,opp_or));
	    alpha = Front_spacing(fr,GENERAL_WAVE)/scaled_length;
	    if (alpha > 1.0)
		alpha = 1.0;
	    if (orient == POSITIVE_ORIENTATION) 
	    {
	    	bdir->next = curr_b->next;
	    	for (i = 0; i < dim; ++i)
	    	    pe[i] = (1.0 - alpha)*ps[i] + alpha*p1[i];
	    }
	    else 
	    {
	    	bdir->prev = curr_b->prev;
	    	for (i = 0; i < dim; ++i)
	    	    ps[i] = (1.0 - alpha)*pe[i] + alpha*p1[i];
	    }
	}
	else
	{
	    /* Interpolate point along curr_b */

	    alpha = ds/scaled_length;
	    if (orient == POSITIVE_ORIENTATION) 
	    {
	    	bdir->next = Following_bond(curr_b,orient);
	    	p1 = Coords(Point_of_bond(curr_b,orient));
	    	p2 = Coords(Point_of_bond(curr_b,opp_or));
	    	for (i = 0; i < dim; ++i)
	    	{
	    	    pe[i] = (1.0 - alpha)*p1[i] + alpha*p2[i];
		}
	    }
	    else 
	    {
	    	bdir->prev = Following_bond(curr_b,orient);
	    	p2 = Coords(Point_of_bond(curr_b,orient));
	    	p1 = Coords(Point_of_bond(curr_b,opp_or));
	    	for (i = 0; i < dim; ++i)
	    	{
	    	    ps[i] = alpha*p1[i] + (1.0 - alpha)*p2[i];
	        }
	    }
	}
	set_bond_length(bdir,dim);
	if (neg_ds)
	    invert_bond(bdir);
	return;

no_continuation:
	pt = Coords(Node_of(c,opp_or)->posn);
	if ((!ds_given) && on_first_bond)
	{
	    if (orient == POSITIVE_ORIENTATION) 
	    	scaled_length = _scaled_separation(ps,pt,h,dim);
	    else
	    	scaled_length = _scaled_separation(pe,pt,h,dim);

	    alpha = Front_spacing(fr,GENERAL_WAVE)/scaled_length;
	    if (alpha > 1.0)
		alpha = 1.0;

	    if (orient == POSITIVE_ORIENTATION) 
	    {
	    	bdir->next = NULL;
	    	for (i = 0; i < dim; ++i)
	    	    pe[i] = (1.0 - alpha)*ps[i] + alpha*pt[i];
	    }
	    else 
	    {
	    	bdir->prev = NULL;
	    	for (i = 0; i < dim; ++i)
	    	    ps[i] = (1.0 - alpha)*pe[i] + alpha*pt[i];
	    }
	}
	else
	{
	    if (orient == POSITIVE_ORIENTATION) 
	    {
	    	for (i = 0; i < dim; ++i)
		    pe[i] = pt[i];
	    	bdir->next = NULL;
	    }
	    else 
	    {
	    	for (i = 0; i < dim; ++i)
		    ps[i] = pt[i];
	    	bdir->prev = NULL;
	    }
	}
	set_bond_length(bdir,dim);
	if (neg_ds)
	    invert_bond(bdir);
	return;
}		/*end bond_secant_to_curve*/


/*
*			find_secant_to_curve():
*
*	This routine returns a unit vector which has the same direction 
*	as the secant from the point p to the point on c a distance ds 
*	(in the scaled metric) along c
*	in the given direction.
*
*	The function may eventually be merged with find_tangent_to_curve.
*/

EXPORT	void find_secant_to_curve(
	FT_POINT		*p,
	BOND		*b,
	CURVE		*c,
	ORIENTATION	orient,
	double		*t,
	Front		*fr,
	double		ds)
{
	static	BOND	*bdir = NULL;
	int		i, dim = c->interface->dim;

	if (bdir == NULL)
	{
	    scalar(&bdir,sizeof(BOND));
	    bdir->start = Static_point(fr->interf);
	    bdir->end = Static_point(fr->interf);
	}

	bond_secant_to_curve(p,b,c,orient,bdir,fr,ds);
	if (orient == POSITIVE_ORIENTATION)
	{
	    for (i = 0; i < dim; ++i)
	    {
	    	t[i] = (Coords(bdir->end)[i] - Coords(bdir->start)[i])/
					bond_length(bdir);
	    }
	}
	else
	{
	    for (i = 0; i < dim; ++i)
	    {
	    	t[i] = (Coords(bdir->start)[i] - Coords(bdir->end)[i])/
	    			bond_length(bdir);
	    }
	}
}		/*end find_secant_to_curve*/


#endif /* defined(TWOD) || defined(THREED) */

#if defined(TWOD)
EXPORT	int	velocity_satisfies_CFL(
	NODE		*newn,
	double		dt,
	double		*dt_frac,
	Front		*front)
{
	double		frac;
	double		*h = front->rect_grid->h;
	double		*v = Node_vel(newn);
	int		dim = front->rect_grid->dim;

	/* Check for possible violation of CFL */

	if (Apply_CFL_at_nodes(front) == NO)
	    return GOOD_NODE;
	if (is_closed_node(newn))
	    return GOOD_NODE;
	if (node_type(newn) < FIRST_PHYSICS_NODE_TYPE)
	    return GOOD_NODE;
	frac = scaled_hypot(v,h,dim)*dt;
	if (debugging("CFL"))
	{
	    print_general_vector("In velocity_satisfies_CFL(), "
				 "Node_vel(newn) = ",Node_vel(newn),dim,"\n");
	    (void) printf("In velocity_satisfies_CFL(), frac = %g, ",frac);
	    (void) printf("Time_step_factor(front) = %g\n",
			  Time_step_factor(front));
	}
	if (frac >  Time_step_factor(front))
	{
	    if (last_time_step_modification(front) == YES)
	    	return GOOD_NODE;
	    (void) printf("WARNING in velocity_satisfies_CFL(), "
	                  "possible CFL violation,  reducing time step\n");
	    (void) printf("frac = %g, Time_step_factor(front) = %g\n",
			  frac,Time_step_factor(front));
	    frac = Time_step_factor(front)/frac;
	    frac = min(Max_time_step_modification_factor(front),frac);
	    frac = max(Min_time_step_modification_factor(front),frac);
	    (void) printf("modified frac = %g, dt_frac = %g\n",frac,*dt_frac);
	    print_node(newn);
	    *dt_frac = min(*dt_frac,frac);
	    (void) printf("modified dt_frac = %g\n",*dt_frac);
	    return MODIFY_TIME_STEP_NODE;
	}
	return GOOD_NODE;
}		/*end velocity_satisfies_CFL*/



/*
*			find_propagation_orientation():
*
*	By propagating the node oldn on curve oldcinc through time interval
*	dt using point_propagate(), this routine determines the
*	the boundary curve cahead toward which the node moves and the sides
*	of cahead adjacent to cinc and adjacent to the new node position.
*
*	Note:  the curves newcahead, oldcahead, newcbehind and oldcbehind
*	are not necessarily boundary curves.  They are simply the adjacent
*	curves to the incident.  For the ahead curves, these must always be
*	tracked as they are on the incoming side of the node.  For the behind
*	curves, these will often be reflected waves, and we must allow for
*	the case where they are not tracked.
*/

EXPORT void find_propagation_orientation(
	Front		*fr,
	POINTER		wave,
	NODE		*oldn,
	NODE		*newn,
	FT_POINT           *newp,
	O_CURVE		*oldcinc,
	double		dt,
	ANGLE_DIRECTION	*i_to_prop_dir,
	O_CURVE		*oldcahead,
	O_CURVE		*newcahead,
	O_CURVE		*oldcbehind,
	O_CURVE		*newcbehind,
	SIDE		*inc_side,
	SIDE		*propagation_side,
	COMPONENT	*ahead_comp,
	COMPONENT	*inc_prop_comp)
{
	double V[MAXD];

	debug_print("find_propagation_orientation",
	      "Entering find_propagation_orientation()\n");

	if (newp == NULL)
	{
	    static FT_POINT *newp_store = NULL;
	    if (newp_store == NULL)
	        newp_store = Static_point(fr->interf);
	    newp = newp_store;
	}

		/* Propagate oldn */

	if (*i_to_prop_dir == ANGLE_DIRECTION_NOT_SET)
	    *i_to_prop_dir = find_i_to_prop_dir(fr,wave,oldn,oldcinc->curve,
					    oldcinc->orient,dt,ahead_comp,
					    newp,V);
	else
	    *i_to_prop_dir = Opposite_ang_dir(*i_to_prop_dir);
	oldcahead->curve = adjacent_curve(oldcinc->curve,oldcinc->orient,
					  *i_to_prop_dir,&oldcahead->orient);
	if (!find_correspond_of_oriented_curve(oldcahead,newcahead,newn,fr,
					       newn->interface))
	{
	    screen("ERROR in find_propagation_orientation(), "
	           "find_correspond_of_oriented_curve() failed\n");
	    clean_up(ERROR);
	}
	oldcbehind->curve = adjacent_curve(oldcinc->curve,oldcinc->orient,
					   Opposite_ang_dir(*i_to_prop_dir),
					   &oldcbehind->orient);
	(void) find_correspond_of_oriented_curve(oldcbehind,newcbehind,
						 newn,fr,newn->interface);

	*inc_side = 
		(curve_ang_oriented_l_to_r(*i_to_prop_dir,oldcahead->orient)) ?
			NEGATIVE_SIDE : POSITIVE_SIDE;

		/* Find inc_prop_comp into which the node has moved */

	*inc_prop_comp = component(Coords(newp),fr->interf);

	*propagation_side = find_propagation_side(oldcahead,newp,*inc_side,fr);

	if (debugging("find_propagation_orientation"))
	{
	    int   dim = fr->rect_grid->dim;
	    (void) printf("find_propagation_orientation:\n");
	    print_general_vector("oldn at ",Coords(oldn->posn),dim," ");
	    print_general_vector("is propagated to ",Coords(newp),dim,"\n");
	    print_side("inc_side = ",*inc_side,"\n");
	    print_side("propagation_side =",*propagation_side,"\n");
	    (void) printf("ahead_comp = %d left,right comp = %d %d\n",
			  *ahead_comp,negative_component(oldcinc->curve),
			  positive_component(oldcinc->curve));
	    (void) printf("inc_prop_comp = %d\n",*inc_prop_comp);
	    print_angle_direction("i_to_prop_dir =",*i_to_prop_dir,"\n");

	    (void) printf("Curves inside find_prop_orientation:\n");
	    (void) printf("OLD AHEAD_CURVE, ");
	    print_o_curve(oldcahead);
	    (void) printf("OLD BEHIND_CURVE, ");
	    print_o_curve(oldcbehind);
	    (void) printf("NEW AHEAD_CURVE, ");
	    print_o_curve(newcahead);
	    (void) printf("NEW BEHIND_CURVE, ");
	    print_o_curve(newcbehind);
	}
	debug_print("find_propagation_orientation",
	      "Left find_propagation_orientation()\n");
}		/*end find_propagation_orientation*/

EXPORT	SIDE	find_propagation_side(
	O_CURVE *cahead,
	FT_POINT   *newp,
	SIDE    inc_side,
	Front   *fr)
{
	BOND  *oldb;
	double nor[3], dot_prod;
	int   i, dim = fr->rect_grid->dim;

	oldb = Bond_at_node_of_o_curve(cahead);
	normal(Node_of_o_curve(cahead)->posn,
	       Hyper_surf_element(oldb),Hyper_surf(cahead->curve),nor,fr);
	for (dot_prod = 0.0, i = 0; i < dim; ++i)
	    dot_prod += nor[i]*(Coords(newp)[i] -
			        Coords(Node_of_o_curve(cahead)->posn)[i]); 
	if (dot_prod > 0.)
	    return POSITIVE_SIDE;
	else if (dot_prod < 0.)
	    return NEGATIVE_SIDE;
	else
	    return Opposite_side(inc_side);
}		/*end find_propagation_side*/


EXPORT ANGLE_DIRECTION f_find_i_to_prop_dir(
	Front		*fr,
	POINTER		wave,
	NODE		*oldn,
	CURVE		*oldc,
	ORIENTATION	c_orient,
	double		dt,
	COMPONENT	*ahead_comp,
	FT_POINT		*newp,
	double		*V)
{
	double		nor[MAXD], dp[MAXD];
	BOND		*oldb = Bond_at_node(oldc,c_orient);
	int		i, dim = fr->rect_grid->dim;
	ANGLE_DIRECTION	i_to_prop_dir;

	point_propagate(fr,wave,oldn->posn,newp,oldb,oldc,dt,V);

		/* Find ahead curve and side defined by node propagation */

	normal(oldn->posn,Hyper_surf_element(oldb),Hyper_surf(oldc),nor,fr);
	for (i = 0; i < dim; ++i)
	    dp[i] = Coords(newp)[i] - Coords(oldn->posn)[i];
	if (scalar_product(nor,dp,dim) < 0.0)
	{
	    *ahead_comp = negative_component(oldc);
	    i_to_prop_dir = (c_orient == POSITIVE_ORIENTATION) ?
			COUNTER_CLOCK : CLOCKWISE;
	}
	else
	{
	    *ahead_comp = positive_component(oldc);
	    i_to_prop_dir = (c_orient == NEGATIVE_ORIENTATION) ?
			COUNTER_CLOCK : CLOCKWISE;
	}
	return i_to_prop_dir;
}		/*end find_i_to_prop_dir*/


/*
*			find_physical_curve_at_node():
*
*	Finds a curve at the node n that has a physical wave_type.
*	If the search fails *c is set to NULL.
*/

EXPORT	CURVE *find_physical_curve_at_node(
	NODE		*n,
	ORIENTATION	*orient)
{
	CURVE		**pc, **c_beg;
	int		i;
	ORIENTATION	c_or;

	for (i = 0, c_beg = n->in_curves,  c_or = NEGATIVE_ORIENTATION;
	     i < 2;
	     ++i,   c_beg = n->out_curves, c_or = POSITIVE_ORIENTATION)
	{
		for (pc = c_beg; pc && *pc; ++pc)
		{
			if (wave_type(*pc) >= FIRST_PHYSICS_WAVE_TYPE) 
			{
				*orient = c_or;
				return *pc;
			}
		}
	}
	return NULL;
}		/*end find_physical_curve_at_node*/



/*
*                   		cut_curve():
*
*	This routine modifies a curve in the vicinity of a node by
*	shifting the node position and deleting certain nearby points.
*
*       It first removes all points of the curve that lie (strictly) between
*	the node Node_of(c,orient) and the point at the far end
*	(with respect to the orientation) of the bond bcut (which is
*	assumed to be non-NULL).  Care is taken to maintain the propagate
*	flags properly.  It then moves the node	position to the position of
*	the point newp.  Finally, it ft_assigns the given left and right states
*	left_st and right_st to the corresponding curve end.
*/


EXPORT void cut_curve(
	FT_POINT		*newp,
	BOND		*bcut,
	CURVE		*c,
	ORIENTATION	orient,
	Front		*fr,
	Locstate	left_st,
	Locstate	right_st)
{
	BOND		*b,*bcut_follower;
	NODE		*n;
	double		sc_b_len;
	double		min_sc_sep;

	if (bcut == NULL) return;

	debug_print("cut_curve","Entering cut_curve()\n");
	min_sc_sep = MIN_SC_SEP(fr->interf);
	if (debugging("cut_curve"))
	{
	    (void) printf("newp = %llu %g %g\n",point_number(newp),
	    	          Coords(newp)[0],Coords(newp)[1]);
	    (void) printf("bcut = %llu %g %g -> %g %g\n",
	    	          bond_number(bcut,fr->interf),
	    	          Coords(bcut->start)[0],Coords(bcut->start)[1],
	    	          Coords(bcut->end)[0],Coords(bcut->end)[1]);
	    (void) printf("left_st: ");	(*fr->print_state)(left_st);
	    (void) printf("right_st: ");	(*fr->print_state)(right_st);

	    (void) printf("\n\t\tCURVE before being cut\n");
	    if (debugging("states")) show_curve_states(c);
	    else			 print_curve(c);
	}

		/* Delete all bonds between node and bcut */

	b = Bond_at_node(c,orient);
	bcut_follower = Following_bond(bcut,orient);
	while (Following_bond(b,orient) != bcut_follower)
	{
	    if (delete_point_adjacent_to_node(fr,c,orient) == FUNCTION_FAILED)
	    {
	    	(void) printf("WARNING in cut_curve(), ");
	    	(void) printf("delete_point_adjacent_to_node failed\n");
	    	break;
	    }
	    b = Bond_at_node(c,orient);
	}

		/* Shift the node position */

	n = Node_of(c,orient);
	Coords(n->posn)[0] = Coords(newp)[0];
	Coords(n->posn)[1] = Coords(newp)[1];
	b = Bond_at_node(c,orient);
	bond_length(b) = separation(b->start,b->end,fr->interf->dim);

	/* TODO: IF AT ALL POSSIBLE THIS DELETE_ SHOULD NOT BE PERFORMED
		If is is found necessary to retain it, the cut-off
		MUST be made compatible with the cut_off in
		robust_cross_bonds. It CANNOT be larger

		** Delete the bond at the node if too small **

	*/
	sc_b_len = scaled_bond_length(b,fr->rect_grid->h,fr->rect_grid->dim);
	if ((sc_b_len < min_sc_sep) && (c->num_points >= 4))
	{
	    (void) delete_point_adjacent_to_node(fr,c,orient);
	}

		/* Assign the states */

	ft_assign(Left_state_at_node(c,orient),left_st,fr->sizest);
	ft_assign(Right_state_at_node(c,orient),right_st,fr->sizest);

	if (debugging("cut_curve"))
	{
	    (void) printf("\n\t\tCURVE after being cut:\n");
	    if (debugging("states"))
		show_curve_states(c);
	    else
		print_curve(c);
	}
	debug_print("cut_curve","Left cut_curve()\n");
}		/*end cut_curve*/



/*
*              		 shift_node():
*
*	This routine modifies two curves in the vicinity of their common
*	node by shifting the node position and transferring points that lie
*	between the old and new positions from one curve to the other.
*
*	It first inserts in curve c2 a copy of the old node position.
*	Then for every point that lies (strictly) between the node and the
*	far end (with respect to the node) of the bond bshift, which is
*	assumed to be on curve c1 and non-NULL, it inserts this point in
*	curve c2 and deletes it from curve c1. 	It then shifts the node
*	position to the position of the point newp.  Care is taken to
*	maintain the propagate flags properly.  It ft_assigns the given
*	left and right states left_st1/right_st1 to the end of curve c1
*	and left_st2/right_st2 to the end of curve c2.  All of the newly
*	inserted ("interacting") points on curve c2 are given linearly 
*	interpolated state values.
*/


EXPORT void shift_node(
	FT_POINT		*newp,
	BOND		*bshift,
	CURVE		*c1,
	ORIENTATION	c1_orient,
	CURVE		*c2,
	ORIENTATION	c2_orient,
	NODE		*n,
	Front		*fr,
	Locstate	left_st1,
	Locstate	 right_st1,
	Locstate	 left_st2,
	Locstate	 right_st2)
{
	double		*h = fr->rect_grid->h;
	double		min_sc_sep = MIN_SC_SEP(fr->interf);
	FT_POINT		*oldp,*transfer_point;
	FT_POINT		*p1, *p2;
	BOND		*b1,*b2,*bshift_follower;
	INTERFACE	*save_intfc;
	INTERFACE	*intfc = c2->interface;
	int		dim = fr->rect_grid->dim;
	boolean		sav_interp;

	debug_print("shift_node","Entered shift_node()\n");
	save_intfc = current_interface();
	set_current_interface(intfc);
	sav_interp = interpolate_intfc_states(intfc);
	interpolate_intfc_states(intfc) = NO;
	if (debugging("shift_node"))
	{
	    (void) printf("old_node %llu",node_number(n));
	    print_node(n);

	    (void) printf("\n\t\tTHIS IS CURVE1 BEFORE SHIFT_NODE\n");
	    (void) printf("start node %llu posn %llu\n",
	    	          node_number(c1->start),
	    	          point_number(c1->start->posn));
	    (void) printf("end node %llu posn %llu\n",
	    	          node_number(c1->end),point_number(c1->end->posn));
	    print_node(c1->start);
	    print_node(c1->end);
	    if (debugging("states"))
		show_curve_states(c1);
	    else
		print_curve(c1);

	    (void) printf("\n\t\tTHIS IS CURVE2 BEFORE SHIFT_NODE\n");
	    (void) printf("start node %llu posn %llu\n",
	    	          node_number(c2->start),
	    	          point_number(c2->start->posn));
	    (void) printf("end node %llu posn %llu\n",
	    	          node_number(c2->end),point_number(c2->end->posn));
	    print_node(c2->start);
	    print_node(c2->end);
	    if (debugging("states"))
		show_curve_states(c2);
	    else
		print_curve(c2);
	    if (fr->sizest)
	    {
	        (void) printf("left_st1\n");	(*fr->print_state)(left_st1);
	        (void) printf("right_st1\n");	(*fr->print_state)(right_st1);
	        (void) printf("left_st2\n");	(*fr->print_state)(left_st2);
	        (void) printf("right_st2\n");	(*fr->print_state)(right_st2);
	    }
	}

		/* Save the old node position */

	oldp = Point(Coords(n->posn));

	ft_assign( Left_state_at_node(c2,c2_orient),left_st2,fr->sizest);
	ft_assign(Right_state_at_node(c2,c2_orient),right_st2,fr->sizest);

	if (debugging("assign_interacting_states"))
	{
	    (void) printf("ft_assigning states for c2 %llu\n",curve_number(c2));
	    (void) printf("left_state_at_node %g %g:\n",Coords(oldp)[0],
							Coords(oldp)[1]);
	    (*fr->print_state)(Left_state_at_node(c2,c2_orient));
	    (void) printf("right_state_at_node %g %g:\n",
	    	          Coords(oldp)[0],Coords(oldp)[1]);
	    (*fr->print_state)(Right_state_at_node(c2,c2_orient));
	}

		/* Shift the node position */

	move_node(n,Coords(newp));

	if (debugging("shift_node"))
	{
	    (void) printf("After shift node posn\n");
	    if (debugging("states"))
	    {
	    	show_curve_states(c1);
	    	show_curve_states(c2);
	    }
	    else
	    {
	    	print_curve(c1);
		print_curve(c2);
	    }
	}

	p2 = Point_adjacent_to_node(c2,c2_orient);
	p1 = (bshift == Bond_at_node(c1,c1_orient)) ?
	     newp : Point_adjacent_to_node(c1,c1_orient);
	if ((scaled_separation(p2,oldp,h,dim) > min_sc_sep) &&
	    (scaled_separation(p1,oldp,h,dim) > min_sc_sep))
	{
	    insert_point_adjacent_to_node(oldp,c2,c2_orient);
	    ft_assign( left_state(oldp),left_st2 ,fr->sizest);
	    ft_assign(right_state(oldp),right_st2,fr->sizest);
	}
	if (debugging("shift_node"))
	{
	    (void) printf("After insert_point_adjacent_to_node()\n");
	    (void) printf("C1 -\n");	
	    if (debugging("states"))
		show_curve_states(c1);
	    else
		print_curve(c1);
	    (void) printf("C2 -\n");	
	    if (debugging("states"))
		show_curve_states(c2);
	    else
		print_curve(c2);
	}


		/* Transfer all bonds between node and bshift */

	b1 = Bond_at_node(c1,c1_orient);
	bshift_follower = Following_bond(bshift,c1_orient);
	if (debugging("shift_node"))
	{
	    (void) printf("node bond of c1\n");
	    print_bond(b1);
	    (void) printf("bshift\n");
	    print_bond(bshift);
	    (void) printf("following bond\n");
	    print_bond(bshift_follower);
	}
	while (Following_bond(b1,c1_orient) != bshift_follower)
	{
	    transfer_point = Point_adjacent_to_node(c1,c1_orient);
	    if (debugging("shift_node"))
	    {
	    	(void) printf("transfer point %g %g\n",
	    		      Coords(transfer_point)[0],
	    		      Coords(transfer_point)[1]);
	    }
	    insert_point_adjacent_to_node(transfer_point,c2,c2_orient);
	    ft_assign( left_state(transfer_point),left_st2 ,fr->sizest);
	    ft_assign(right_state(transfer_point),right_st2,fr->sizest);
	    (void) delete_point_adjacent_to_node(fr,c1,c1_orient);
	    b1 = Bond_at_node(c1,c1_orient);
	}

	    /* Delete small bonds */

	b1 = Bond_at_node(c1,c1_orient);
	if (scaled_bond_length(b1,h,dim) < min_sc_sep)
	    (void) delete_point_adjacent_to_node(fr,c1,c1_orient);
	b2 = Bond_at_node(c2,c2_orient);
	if (scaled_bond_length(b2,h,dim) < min_sc_sep)
	{
	    if (Point_adjacent_to_node(c2,c2_orient) == oldp)
	    	oldp = n->posn;		/* oldp is needed below */
	    (void) delete_point_adjacent_to_node(fr,c2,c2_orient);
	}

	if (debugging("shift_node"))
	{
	    (void) printf("\n\t\tTHIS IS CURVE1 BEFORE ASSIGN_STATES\n");
	    if (debugging("states"))
		show_curve_states(c1);
	    else
		print_curve(c1);
	    (void) printf("\n\t\tTHIS IS CURVE2 BEFORE ASSIGN_STATES\n");
	    if (debugging("states"))
		show_curve_states(c2);
	    else
		print_curve(c2);
	}

		/* Assign states at node and at newly inserted points */

	assign_interacting_states(n->posn,c1,c1_orient,fr,left_st1,right_st1);

	if (debugging("shift_node"))
	{
	    (void) printf("\n\t\tTHIS IS CURVE1 AFTER SHIFT_NODE\n");
	    if (debugging("states"))
		show_curve_states(c1);
	    else
		print_curve(c1);
	    (void) printf("\n\t\tTHIS IS CURVE2 AFTER SHIFT_NODE\n");
	    if (debugging("states"))
		show_curve_states(c2);
	    else
		print_curve(c2);
	}
	interpolate_intfc_states(intfc) = sav_interp;
	set_current_interface(save_intfc);
	debug_print("shift_node","Left shift_node()\n");
}		/*end shift_node*/


/*
*			shift_node_past():
*
*	Shifts a node past boundary obstacles depending on the choice of flag,
*	by calls to shift_node.
*
*	Note:  There is a small problem with the ft_assignment of the boundary
*	state data for the new curve created by shifting one node past
*	another on the boundary.  There are two possibilities, either one
*	of which can be correct in a given instance.
*
*	In one case we are shifting past a boundary node which exists because
*	of the computation only.  An example is a corner of the domain
*	separating Dirichlet boundaries.  In this case we want the boundary
*	state info for the new curve to come from the behind curve, as if
*	we were stretching the behind curve.
*
*	Sometimes the node serves a more fundamental physical purpose, and
*       separates boundaries of different characteristics.  Here we want the
*	boundary state info for the new curve to come from the ahead curve, so
*	that the physical curve passes from one type of boundary to another.
*
*	As of right now, it is impossible to specify which case is desired
*	for each instance in a problem.  There are two sets of code below, both
*	labelled.  As of right now, the only time the first possibility is
*	used is, as mentioned above, when we have two Dirichlet boundaries.
*
*	TODO: upgrade to allow more flexibility here.  This will probably
*	require a new boundary curve type.
*/

EXPORT void shift_node_past(
	FT_POINT		*newp,
	BOND		*bshift,
	CURVE		*c1,
	ORIENTATION	c1_orient,
	CURVE		*c2,
	ORIENTATION	c2_orient,
	ANGLE_DIRECTION	i_to_a_orient,
	NODE		*n,
	Front		*fr,
	NODE_FLAG	flag,
	Locstate	left_st1,
	Locstate	right_st1,
	Locstate	left_st2,
	Locstate	right_st2)
{
	CURVE		*nextbc;
	ORIENTATION	nextbc_orient;
	NODE		*oppn;
	CURVE		**c,**ctmp;
	CURVE		**in_curves,**out_curves;
	CURVE		*adjc;
	ORIENTATION	adjc_orient;
	BOND		*b;
	FT_POINT		*endpt_nextbc;
	COMPONENT	exterior_c1_comp;
	double		*h = fr->rect_grid->h;
	double		min_sc_sep = MIN_SC_SEP(fr->interf);
	int		num_in_c,num_out_c;
	int		dim = fr->rect_grid->dim;
	Locstate	left_stnbc,right_stnbc;		/* left right states
							   on nextbc */
	Locstate	ext_st1,int_st2;

	debug_print("shift_node","Entering shift_node_past()\n");
	if (debugging("shift_node"))
		(void) printf("newp = %g %g\n",Coords(newp)[0],Coords(newp)[1]);
	if (to_next_node_only(flag) == YES)
	{
		shift_node(newp,bshift,c1,c1_orient,c2,c2_orient,
			n,fr,left_st1,right_st1,left_st2,right_st2);
		return;
	}
	Check_return(next_boundary(c2,c2_orient,&nextbc,&nextbc_orient),
		     shift_node_past)
	if (nextbc == c1)
	{
		shift_node(newp,bshift,c1,c1_orient,c2,c2_orient,n,
			fr,left_st1,right_st1,left_st2,right_st2);
		return;
	}
	if (continue_past_fixed_node(flag) != YES)
		return;
	oppn = Node_of(nextbc,Opposite_orient(nextbc_orient));
	if (debugging("shift_node"))
	{
		(void) printf("C1 = AHEAD CURVE\n");
		 print_curve(c1);
		(void) printf("C2 = BEHIND CURVE\n");
		 print_curve(c2);
		(void) printf("nextbc = BETWEEN BDRY CURVE\n");
		print_curve(nextbc);
		(void) printf("NODE n\n");	print_node(n);
		(void) printf("NODE oppn\n");	print_node(oppn);
	}

		/* Find left/right states for nextbc */

	if (c2_orient != nextbc_orient)
	{
		left_stnbc = left_st2;
		right_stnbc = right_st2;
	}
	else
	{
		left_stnbc = right_st2;
		right_stnbc = left_st2;
	}
	bstate_index(nextbc) = bstate_index(c2);

		/* nextbc state settings for Neumann curves */

	if (wave_type(c1) == NEUMANN_BOUNDARY)
	{
		bstate_index(nextbc) = bstate_index(c1);
	}
	else if (wave_type(c2) == NEUMANN_BOUNDARY)
	{
		bstate_index(nextbc) = bstate_index(c1);
	}

		/* Exterior has only passive curves */

	adjc = c2;
	adjc_orient = c2_orient;
	while (adjc == c2 || wave_type(adjc) == PASSIVE_BOUNDARY)
		adjc = adjacent_curve(adjc,adjc_orient,
			CLOCKWISE,&adjc_orient);
	if (adjc == nextbc)
	{
		ext_st1   = (c1_orient     == POSITIVE_ORIENTATION) ?
			left_st1 : right_st1;
		int_st2   = (c2_orient     == POSITIVE_ORIENTATION) ?
			left_st2 : right_st2;

			/* set exterior state on nextbc */
		if (wave_type(c1) == NEUMANN_BOUNDARY)
		{
			if (nextbc_orient == POSITIVE_ORIENTATION)
				left_stnbc = ext_st1;
			else right_stnbc = ext_st1;
		}
		else if (wave_type(c2) == NEUMANN_BOUNDARY)
		{
			if (nextbc_orient == POSITIVE_ORIENTATION)
				left_stnbc = int_st2;
			else right_stnbc = int_st2;
		}
	}
	else
	{
		ext_st1   = (c1_orient     == NEGATIVE_ORIENTATION) ?
			left_st1 : right_st1;
		int_st2   = (c2_orient     == NEGATIVE_ORIENTATION) ?
			left_st2 : right_st2;

			/* set exterior state on nextbc */
		if (wave_type(c1) == NEUMANN_BOUNDARY)
		{
			if (nextbc_orient == NEGATIVE_ORIENTATION)
				left_stnbc = ext_st1;
			else right_stnbc = ext_st1;
		}
		else if (wave_type(c2) == NEUMANN_BOUNDARY)
		{
			if (nextbc_orient == NEGATIVE_ORIENTATION)
				left_stnbc = int_st2;
			else right_stnbc = int_st2;
		}
	}
	if (debugging("shift_node"))
	{
		(void) printf("left right states for nextbc\n");
		(*fr->print_state)(left_stnbc);
		(*fr->print_state)(right_stnbc);
	}

		/* Assign states on nextbc */

	assign_interacting_states(oppn->posn,nextbc,nextbc_orient,
		fr,left_stnbc,right_stnbc);

		/* Insert zero length bond at end of nextbc */

	endpt_nextbc = Point(Coords(oppn->posn));
	insert_point_adjacent_to_node(endpt_nextbc,nextbc,
		Opposite_orient(nextbc_orient));

		/* Shift node to oppn position */

	shift_node(oppn->posn,
		Bond_at_node(nextbc,Opposite_orient(nextbc_orient)),
		nextbc,nextbc_orient,c2,c2_orient,n,
		fr,left_stnbc,right_stnbc,left_st2,right_st2);

		/* Transfer data on nextbc */
	/* See note at top of function.  This is the second possibility */

	wave_type(nextbc) = wave_type(c1);
	if (is_bdry(c1))
		set_is_bdry(nextbc);
	else
		set_not_bdry(nextbc);
	bstate_index(nextbc) = bstate_index(c1);
	if (c2_orient != nextbc_orient)
	{
		negative_component(nextbc) = negative_component(c2);
		positive_component(nextbc) = positive_component(c2);
	}
	else
	{
		negative_component(nextbc) = positive_component(c2);
		positive_component(nextbc) = negative_component(c2);
	}
	if (curve_ang_oriented_l_to_r(i_to_a_orient,c1_orient))
		exterior_c1_comp = positive_component(c1);
	else
		exterior_c1_comp = negative_component(c1);

	if (curve_ang_oriented_l_to_r(i_to_a_orient,nextbc_orient))
		positive_component(nextbc) = exterior_c1_comp;
	else
		negative_component(nextbc) = exterior_c1_comp;
		
	if (correspond_curve(nextbc) != NULL)
		correspond_hyper_surf(correspond_hyper_surf(nextbc)) = NULL;
	correspond_hyper_surf(nextbc) = NULL;

	node_type(n) = node_type(oppn);
	if (is_bdry(oppn))
		set_is_bdry(n);
	else
		set_not_bdry(n);

	node_type(oppn) = bdry_node_type(wave_type(c1));
	if (node_type(oppn) == UNKNOWN_NODE_TYPE)
	{
		screen("ERROR in shift_node_past(), ");
		screen("can't set boundary node type\n");
		clean_up(ERROR);
	}


	if (is_bdry(c1))
		set_is_bdry(oppn);
	else
		set_not_bdry(oppn);

		/* Transfer non boundary curves */

	if (debugging("shift_node"))
		(void) printf("Begin transfer of non bdry curves\n");
	num_in_c = num_out_c = 0;
	for (c = n->in_curves; c && *c; ++c)
		++num_in_c;
	for (c = n->out_curves; c && *c; ++c)
		++num_out_c;
	uni_array(&in_curves,num_in_c + 1,sizeof(CURVE *));
	uni_array(&out_curves,num_out_c + 1,sizeof(CURVE *));
	ctmp = in_curves;
delete_in_n:
	for (c = n->in_curves; c && *c; ++c)
	{
		if (*c == nextbc || *c == c2) continue;
		*ctmp = *c;
		if (!delete_from_pointers(*c,&n->in_curves))
		{
			screen("ERROR in shift_node_past(), ");
			screen("delete_from_pointers() failed\n");
			clean_up(ERROR);
		}
		++ctmp;
		goto delete_in_n;
	}
	*ctmp = NULL;
delete_in_oppn:
	for (c = oppn->in_curves; c && *c; ++c)
	{
		if (*c == nextbc || *c == c1) continue;
		(*c)->end = n;
		(*c)->last->end = n->posn;
		if (!add_to_pointers(*c,&n->in_curves))
		{
			screen("ERROR in shift_node_past(), ");
			screen("add_to_pointers() failed\n");
			clean_up(ERROR);
		}
		if (!delete_from_pointers(*c,&oppn->in_curves))
		{
			screen("ERROR in shift_node_past(), ");
			screen("delete_from_pointers() failed\n");
			clean_up(ERROR);
		}
		goto delete_in_oppn;
	}
	for (c = in_curves; c && *c; ++c)
	{
		if (!add_to_pointers(*c,&oppn->in_curves))
		{
			screen("ERROR in shift_node_past(), ");
			screen("add_to_pointers() failed\n");
			clean_up(ERROR);
		}
		(*c)->end = oppn;
		(*c)->last->end = oppn->posn;
	}
	ctmp = out_curves;
delete_out_n:
	for (c = n->out_curves;c && *c; ++c)
	{
		if (*c == nextbc || *c == c2) continue;
		*ctmp = *c;
		if (!delete_from_pointers(*c,&n->out_curves))
		{
			screen("ERROR in shift_node_past(), ");
			screen("delete_from_pointers() failed\n");
			clean_up(ERROR);
		}
		++ctmp;
		goto delete_out_n;
	}
	*ctmp = NULL;
delete_out_oppn:
	for (c = oppn->out_curves; c && *c; ++c)
	{
		if (*c == nextbc || *c == c1) continue;
		(*c)->start = n;
		(*c)->first->start = n->posn;
		if (!add_to_pointers(*c,&n->out_curves))
		{
			screen("ERROR in shift_node_past(), ");
			screen("add_to_pointers() failed\n");
			clean_up(ERROR);
		}
		if (!delete_from_pointers(*c,&oppn->out_curves))
		{
			screen("ERROR in shift_node_past(), ");
			screen("delete_from_pointers() failed\n");
			clean_up(ERROR);
		}
		goto delete_out_oppn;
	}
	for (c = out_curves; c && *c; ++c)
	{
		if (!add_to_pointers(*c,&oppn->out_curves))
		{
			screen("ERROR in shift_node_past(), ");
			screen("add_to_pointers() failed\n");
			clean_up(ERROR);
		}
		(*c)->start = oppn;
		(*c)->first->start = oppn->posn;
	}
	free(in_curves);
	free(out_curves);
	if (debugging("shift_node"))
		(void) printf("Transfer completed\n");

	shift_node(newp,bshift,c1,c1_orient,nextbc,
		   Opposite_orient(nextbc_orient),oppn,
		   fr,left_st1,right_st1,left_stnbc,right_stnbc);

	/* See note at top of function.  This is the first possibility. */
	if ((wave_type(c1) == DIRICHLET_BOUNDARY) &&
	    (wave_type(c2) == DIRICHLET_BOUNDARY))
	{
	    wave_type(nextbc) = wave_type(c2);
	    if (is_bdry(c1))
	    	set_is_bdry(nextbc);
	    else
	    	set_not_bdry(nextbc);
	    bstate_index(nextbc) = bstate_index(c2);
	}

	b = Bond_at_node(nextbc,nextbc_orient);
	bond_length(b) = separation(b->start,b->end,fr->interf->dim);
	if (scaled_bond_length(b,h,dim) < min_sc_sep)
	{
		if (is_short_curve(nextbc,nextbc_orient,fr->rect_grid,1.0))
		{
			b = Bond_at_node(c1,c1_orient);
			if (c1_orient == POSITIVE_ORIENTATION)
			{
				Coords(newp)[0] = .99*Coords(b->start)[0] +
					          .01*Coords(b->end)[0];
				Coords(newp)[1] = .99*Coords(b->start)[1] +
					          .01*Coords(b->end)[1];
				Coords(oppn->posn)[0] = Coords(newp)[0];
				Coords(oppn->posn)[1] = Coords(newp)[1];
			}
			else
			{
				Coords(newp)[0] = .01*Coords(b->start)[0] +
					          .99*Coords(b->end)[0];
				Coords(newp)[1] = .01*Coords(b->start)[1] +
					          .99*Coords(b->end)[1];
				Coords(oppn->posn)[0] = Coords(newp)[0];
				Coords(oppn->posn)[1] = Coords(newp)[1];
			}
			bond_length(b) =
				separation(b->start,b->end,fr->interf->dim);
			b = Bond_at_node(nextbc,Opposite_orient(nextbc_orient));
			bond_length(b) =
				separation(b->start,b->end,fr->interf->dim);
		}
		else
			(void) delete_point_adjacent_to_node(fr,nextbc,
							     nextbc_orient);
	}
	if (debugging("shift_node"))
	{
		(void) printf("nextbc after shift_node_past()\n");
		print_curve(nextbc);
		(void) printf("c1 after shift_node_past()\n");
		print_curve(c1);
	}

	debug_print("shift_node","Left shift_node_past\n");
}		/*end shift_node_past*/

EXPORT	int	bdry_node_type(
	int	bdry_type)
{
	switch(bdry_type)
	{
	case NEUMANN_BOUNDARY:
	    return NEUMANN_NODE;
	case DIRICHLET_BOUNDARY:
	    return DIRICHLET_NODE;
	case SUBDOMAIN_BOUNDARY:
#if defined(USE_OVERTURE)
        case AMR_SUBDOMAIN_BOUNDARY:
#endif /* if defined(USE_OVERTURE) */

	    return SUBDOMAIN_NODE;
	default:
	    return UNKNOWN_NODE_TYPE;
	}
}		/*end bdry_node_type*/


/*
*			assign_interacting_states():
*
*	Assigns the given left and right states to the left/right states of
*	certain points near a node.  Typically these new points have been
*	inserted in a curve because of the interaction around the node, hence
*	are "interacting" points.
*
*	The states at the end of the curve c at the node Node_of(c,orient)
*	together with the states of every point between this node and
*	the point p, inclusively are ft_assigned the given left and right
*	states left_st and right_st.  It is assumed that p is actually a
*	point on the curve c.
*/

EXPORT void assign_interacting_states(
	FT_POINT		*p,
	CURVE		*c,
	ORIENTATION	orient,
	Front		*fr,
	Locstate	left_st,
	Locstate	right_st)
{
	BOND		*b;
	FT_POINT		*bpoint;
	ORIENTATION	opp_orient = Opposite_orient(orient);
	
	debug_print("assign_interacting_states",
		"Entering assign_interacting_states()\n");

	ft_assign(Left_state_at_node(c,orient),left_st,fr->sizest);
	ft_assign(Right_state_at_node(c,orient),right_st,fr->sizest);
	if (debugging("assign_interacting_states"))
	{
		(void) printf("ft_assigning states for curve %llu\n",
			      curve_number(c));
		(void) printf("left_state_at_node:\n");
		(*fr->print_state)(Left_state_at_node(c,orient));
		(void) printf("right_state_at_node:\n");
		(*fr->print_state)(Right_state_at_node(c,orient));
	}
	if (p == Node_of(c,orient)->posn)
		return;

	for (b = Bond_at_node(c,orient); ; b = Following_bond(b,orient))
	{
		bpoint = Point_of_bond(b,opp_orient);
		ft_assign(left_state(bpoint),left_st,fr->sizest);
		ft_assign(right_state(bpoint),right_st,fr->sizest);
		if (debugging("assign_interacting_states"))
		{
		       (void) printf("ft_assigning state for point %llu at %g %g\n",
				     point_number(bpoint),
				     Coords(bpoint)[0],Coords(bpoint)[1]);
			(void) printf("left state:\n");
			(*fr->print_state)(left_state(bpoint));
			(void) printf("right state:\n");
			(*fr->print_state)(right_state(bpoint));
		}
		if (bpoint == p) break;
	}
	if (Node_of(c,Opposite_orient(orient))->posn == p)
	{
		ft_assign(Left_state_at_node(c,opp_orient),left_st,fr->sizest);
		ft_assign(Right_state_at_node(c,opp_orient),right_st,fr->sizest);
		if (debugging("assign_interacting_states"))
		{
			(void) printf("ft_assigning states for curve %llu\n",
				      curve_number(c));
			(void) printf("left_state_at_node:\n");
			(*fr->print_state)(Left_state_at_node(
				c,Opposite_orient(orient)));
			(void) printf("right_state_at_node:\n");
			(*fr->print_state)(Right_state_at_node(
				c,Opposite_orient(orient)));
		}
	}

	debug_print("assign_interacting_states",
	      "Left assign_interacting_states()\n");
}		/*end assign_interacting_states*/





/*
*			insert_point_adjacent_to_node():
*
*	Inserts the point p in the bond on curve c at the end of c
*	determined by its orientation: start for POSITIVE_ORIENTATION,
*	end for NEGATIVE_ORIENTATION.
*	Care is taken to maintain the propagate flags properly.  For this
*	reason it is preferable to use this routine when inserting points
*	during the node propagation.
*/

EXPORT void insert_point_adjacent_to_node(
	FT_POINT		*p,
	CURVE		*c,
	ORIENTATION	c_orient)
{
	BOND		*b = Bond_at_node(c,c_orient);

	if (debugging("insert_point_adj"))
	{
	    (void) printf("inserting point %llu, %g %g in bond %llu, ",
			  point_number(p),Coords(p)[0],Coords(p)[1],
			  bond_number(b,c->interface));
	    (void) printf("%g %g -> %g %g\n",
			  Coords(b->start)[0],Coords(b->start)[1],
			  Coords(b->end)[0],Coords(b->end)[1]);
	}

	if (insert_point_in_bond(p,b,c) != FUNCTION_SUCCEEDED)
	{
	    screen("ERROR in insert_point_adjacent_to_node(), "
		   "insert_point_in_bond() failed\n");
	    clean_up(ERROR);
	}
}		/*insert_point_adjacent_to_node*/

/*
*			init_redundant_node_for_deletion():
*
*	This function prepares an assumedly redundant node (ie exactly
*	two curves meet at rnode) and prepares it for deletion by setting
*	the states at the node.  The node gets propagated as a regular
*	point.
*/

EXPORT void init_redundant_node_for_deletion(
	NODE		*rnode,
	NODE		*old_rnode,
	Front		*fr,
	POINTER		wave,
	double		dt)
{
	O_CURVE		*oc1, *oldoc1, *oc2, *oldoc2;
	CURVE		**c;
	INTERFACE	*old_intfc = old_rnode->interface;;

	oc1 = oldoc1 = oc2 = oldoc2 = NULL;
	for (c = rnode->in_curves; c && *c; ++c)
	{
		if (oc1 == NULL)
			init_o_curve(&oc1,*c,NEGATIVE_ORIENTATION);
		else
			init_o_curve(&oc2,*c,NEGATIVE_ORIENTATION);
	}
	for (c = rnode->out_curves; c && *c; ++c)
	{
		if (oc1 == NULL)
			init_o_curve(&oc1,*c,POSITIVE_ORIENTATION);
		else
			init_o_curve(&oc2,*c,POSITIVE_ORIENTATION);
	}
	init_o_curve(&oldoc1,NULL,ORIENTATION_NOT_SET);
	Check_return(
	    find_correspond_of_oriented_curve(oc1,oldoc1,old_rnode,fr,
					      old_intfc),
	    init_redundant_node_for_deletion)
	set_states_at_node_by_propagate(fr,wave,oldoc1,oc1,dt);
	init_o_curve(&oldoc2,NULL,ORIENTATION_NOT_SET);
	Check_return(
	    find_correspond_of_oriented_curve(oc2,oldoc2,old_rnode,fr,
					      old_intfc),
	    init_redundant_node_for_deletion)
	set_states_at_node_by_propagate(fr,wave,oldoc2,oc2,dt);
	free_these(4,oc1,oc2,oldoc1,oldoc2);
}		/*end init_redundant_node_for_deletion*/


/*
*			delete_redundant_node():
*
*	Deletes any non source or sink nodes with no in or out curves.
*	Also if just two different curves of the same wave type meet
*	determine whether a "redundant" node can really be deleted.
*	Returns YES if the node is deleted, NO otherwise.
*/

EXPORT	boolean delete_redundant_node(
	NODE		*n,
	CROSS		*cross,
	RPROBLEM	*rp,
	Front		*fr)
{
	CURVE		*c1, *c2;
	CURVE		*cur;
	CURVE		**c;
	ORIENTATION	c1_orient, c2_orient;
	INTERFACE	*intfc, *sav_intfc;
	boolean		sav_interp;
	boolean		status;

	if (n == NULL || n->interface == NULL)
	    return NO;

	intfc = n->interface;
	if (debugging("drnode"))
	    print_interface(intfc);
	if (is_stationary_node(n))
	    return NO;
	c1 = c2 = NULL;
	for (c = n->in_curves; c && *c; ++c)
	{
	    if (!c1)
	    {
	    	c1 = *c;
	    	c1_orient = NEGATIVE_ORIENTATION;
	    }
	    else if (!c2)
	    {
	    	c2 = *c;
	    	c2_orient = NEGATIVE_ORIENTATION;
	    }
	    else /* More than two curves meet at n */
		return NO;
	}
	for (c = n->out_curves; c && *c; ++c)
	{
	    if (!c1)
	    {
	    	c1 = *c;
	    	c1_orient = POSITIVE_ORIENTATION;
	    }
	    else if (!c2)
	    {
	    	c2 = *c;
	    	c2_orient = POSITIVE_ORIENTATION;
	    }
	    else /* More than two curves meet at n */
		return NO;
	}
	if (!c1 && !c2)		/* No curves at non source-sink node */
	{
	    (void) delete_node(n);
	    return YES;
	}
		 /* Exactly one curve at node (May be CLOSED node) */
	if ((c1 == c2) || (!c2))
	{
	    if (node_type(n) == ERROR)
	    {
	        (void) printf("WARNING in delete_redundant_node(), "
	                      "Node of closed curve only with type ERROR\n");
	       node_type(n) = CLOSED_NODE;
	    }
	    return NO;
	}
	if (debugging("drnode")) 
	{
	    (void) printf("In delete_redundant_node()\n");
	    print_curve(c1);
	    print_curve(c2);
	    (void) printf("before check_delete_redundant_node\n");
	    (void) printf("c1->hs %llu ",hypersurface_number(Hyper_surf(c1)));
	    (void) printf("c2->hs %llu ",hypersurface_number(Hyper_surf(c2)));
	    print_wave_type("wave type c1 = ",wave_type(c1)," ",intfc);
	    print_wave_type("wave type c2 = ",wave_type(c2)," ",intfc);
	    (void) printf("front = %p",(POINTER)fr);
	    (void) printf("check_delete_redundant_node %p\n",
	    	          fr->_check_delete_redundant_node);
	}

	if (!(check_delete_redundant_node(n,c1,c2,fr)))
	{
	    if (debugging("drnode"))  
	    	(void) printf("after check_delete_redundant_node\n");
	    return NO;
	}
	if (debugging("drnode"))  
	    (void) printf("after check_delete_redundant_node\n");
	if (c1_orient == c2_orient)
	    invert_curve(c2);
	if (debugging("drnode"))
	    (void) printf("after invert_curve\n");

	if ((negative_component(c1) != negative_component(c2)) ||
	    (positive_component(c1) != positive_component(c2)))
	{
	    screen("WARNING: curve components do not match in "
	           "delete_redundant_node()\n");
	    print_curve(c1);
	    print_curve(c2);
	}
	sav_intfc = current_interface();
	set_current_interface(intfc);
	sav_interp = interpolate_intfc_states(intfc);
	interpolate_intfc_states(intfc) = YES;
	if (debugging("drnode"))  (void) printf("before join_curves()\n");
	if (c1_orient == NEGATIVE_ORIENTATION)
	{
	    cur = join_curves(c1,c2,negative_component(c1),
	    		      positive_component(c1),NULL);
	    if (cross)
		rcl_after_join(cross,cur,c1,c2);
	    if (rp)
		roclists_after_join(rp,c1,NULL,c2,NULL,cur);
	}
	else
	{
	    cur = join_curves(c2,c1,negative_component(c1),
	    		  positive_component(c1),NULL);
	    if (cross) rcl_after_join(cross,cur,c2,c1);
	    if (rp) roclists_after_join(rp,c2,NULL,c1,NULL,cur);
	}
	if (debugging("drnode"))
	    (void) printf("after join_curves()\n");
	if (cur == NULL)
	    status = NO;
	else
	{
	    (void) delete_node(n);
	    status = YES;
	}
	interpolate_intfc_states(intfc) = sav_interp;
	set_current_interface(sav_intfc);
	return status;
}		/*end delete_redundant_node*/


/*
*		f_check_delete_redundant_node():
*
*	Physics independent checks to make sure we actually want 
*	to delete a node at which only two curves meet.
*/

/*ARGSUSED*/
EXPORT boolean f_check_delete_redundant_node(
	NODE		*n,
	CURVE		*c1,
	CURVE		*c2)
{
	return (wave_type(c1) != wave_type(c2)) ? NO : YES;
}		/*end f_check_delete_redundant_node*/


/*
*			find_tangent_to_propagated_curve():
*
*	This routine returns a unit tangent vector at the point newp of
*	the curve newc obtained by propagating the curve oldc one time step.
*	It is assumed the newp lies along the bond newb and that newb is
*	connected to newc.  Furthermore it is assumed the the new curve has
*	already been propagated at least up to the point where the new bond
*	joins the curve.
*/

EXPORT	void find_tangent_to_propagated_curve(
	FT_POINT		*newp,
	BOND		*newb,
	O_CURVE		*oldc,
	O_CURVE		*newc,
	double		*t,
	Front		*fr,
	POINTER		wave,
	double		dt)
{
	BOND		*bindex, *follower;
	FT_POINT		*end_pt;
	double		scaled_length;
	double		*h = fr->rect_grid->h;
	double		V[MAXD];
	double		len;
	int		dim = fr->rect_grid->dim;
	int		i;

	bindex = newb;
	end_pt = Point_of_bond(bindex,Opposite_orient(newc->orient));
	scaled_length = scaled_separation(end_pt,newp,h,dim);
	follower = Following_bond(bindex,newc->orient);
	while ((scaled_length < 0.1) && (follower != NULL))
	{
	    bindex = follower;
	    end_pt = Point_of_bond(bindex,Opposite_orient(newc->orient));
	    scaled_length += scaled_bond_length(bindex,h,dim);
	    follower = Following_bond(bindex,newc->orient);
	}
	if ((follower == NULL) && 
	    (propagation_status(Opp_node_of_o_curve(newc)) != PROPAGATED_NODE))
	{
	    static	FT_POINT	*ptmp = NULL;
		
	    if (ptmp == NULL)
	        ptmp = Static_point(fr->interf);

	    point_propagate(fr,wave,Opp_node_of_o_curve(oldc)->posn,
			    ptmp,Bond_at_opp_node_of_o_curve(oldc),
			    oldc->curve,dt,V);
	    end_pt = ptmp;
	}
	len = 0.0;
	for (i = 0; i < dim; ++i)
	{
	    t[i] = Coords(end_pt)[i] - Coords(newp)[i];
	    len += sqr(t[i]);
	}
	len = sqrt(len);
	for (i = 0; i < dim; ++i)
	    t[i] /= len;
}		/*end find_tangent_to_propagated_curve*/


/*
*			propagated_tangent_bond_at_node():
*
*	Given an old curve and a time step dt, this function finds a bond
*	which is tangent to the propagated curve at its node.  This function
*	differs from other tangent functions in that the curve
*	itself is not propagated.  That is the propagated positions of points
*	on the old curve are found and used to find a tangent bond, but these
*	points are not recorded in the new curve structure.  This allows 
*	flexibility in using partial or substitute time steps.  This facility 
*	is needed in some cases such as Mach reflection when the actual time 
*	step is too small and an artifical time step is used in order to find 
*	the intersection of a curve and a circle for a velocity computation.  
*	This function is also limited to finding tangent bonds at the node.
*	It is assumed that storage for the start and end points of
*	the bond newb has been allocated and that for such storage rewrite
*	is "safe".
*/

EXPORT	void propagated_tangent_bond_at_node(
	BOND		*tan_bond,
	CURVE		*curve,
	ORIENTATION	curve_orient,
	Front		*fr,
	POINTER		wave,
	double		dt)
{
	RECT_GRID	*gr = fr->rect_grid;
	double		*h = gr->h;
	double		V[MAXD],psave[MAXD],scaled_length;
	FT_POINT		*oldp = Node_of(curve,curve_orient)->posn;
	BOND		*oldb = Bond_at_node(curve,curve_orient);
	int		i, dim = gr->dim;

	if (tan_bond->start == NULL || tan_bond->end == NULL)
	{
		Error(0,"ERROR: in propagated_tangent_bond_at_node()\n");
		screen("NULL endpoint on bond\n");
		clean_up(ERROR);
	}
	if (curve_orient == POSITIVE_ORIENTATION)
	{
		point_propagate(fr,wave,oldp,tan_bond->start,oldb,
			curve,dt,V);
		oldb = Following_bond(oldb,curve_orient);
		oldp = Point_adjacent_to_node(curve,curve_orient);
		point_propagate(fr,wave,oldp,tan_bond->end,
			oldb,curve,dt,V);
	}
	else
	{
		point_propagate(fr,wave,oldp,tan_bond->end,oldb,
			curve,dt,V);
		oldb = Following_bond(oldb,curve_orient);
		oldp = Point_adjacent_to_node(curve,curve_orient);
		point_propagate(fr,wave,oldp,tan_bond->start,
			oldb,curve,dt,V);
	}
	scaled_length = scaled_bond_length(tan_bond,h,dim);
	while (scaled_length < 0.1 &&
		(oldb = Following_bond(oldb,curve_orient)) != NULL && 
		Following_bond(oldb,curve_orient) != NULL)
	{ 
		if (curve_orient == POSITIVE_ORIENTATION)
		{
			oldp = oldb->end;
			for (i = 0; i < dim; ++i)
				psave[i] = Coords(tan_bond->end)[i];
			point_propagate(fr,wave,oldp,tan_bond->end,
				oldb,curve,dt,V);
			scaled_length +=
				_scaled_separation(Coords(tan_bond->end),psave,
						h,dim);
		}
		else
		{
			oldp = oldb->start;
			for (i = 0; i < dim; ++i)
				psave[i] = Coords(tan_bond->start)[i];
			point_propagate(fr,wave,oldp,tan_bond->start,
				oldb,curve,dt,V);
			scaled_length += 
				_scaled_separation(Coords(tan_bond->start),
					psave,h,dim);
		}
	}
	bond_length(tan_bond) =
		separation(tan_bond->start,tan_bond->end,gr->dim);
}		/*end propagated_tangent_bond_at_node*/

/*
*			modify_B_node():
*
*	Uses shift_node() and cut_curve() to modify the interface in the
*	neighborhood of a B_node().
*
*	Note: in the special case of crossing a fixed node
*	newcahead and newcbehind are not the correct curves.
*	(oldcahead is also wrong - simply crspd of newcahead).  Technically
*	speaking, the correct curves cannot be computed until the node has
*	been shifted.  In the normal case, all these curves are correct.
*/

EXPORT	int modify_B_node(
	NODE		*oldn,
	NODE		*newn,
	O_CURVE		*oldcphys,
	O_CURVE		*newcphys,
	O_CURVE		*oldcahead,
	O_CURVE		*newcahead,
	O_CURVE		*oldcaprop,
	O_CURVE		*newcaprop,
	O_CURVE		*oldcbehind,
	O_CURVE		*newcbehind,
	O_CURVE		*oldcbprop,
	O_CURVE		*newcbprop,
	FT_POINT		*pc,
	BOND		*crossbphys,
	BOND		*crossbahead,
	ANGLE_DIRECTION	i_to_prop_dir,
	double		tcr_phys,
	double		tcr_ahead,
	RPROBLEM	**rp,
	Front		*fr,
	POINTER		wave,
	double		dt,
	double		*dt_frac,
	NODE_FLAG	flag)
{
	int		status;
	SIDE		p_side_of_b, b_side_of_p;
	double		V[MAXD];
	static Locstate	left_st_a = NULL, right_st_a = NULL;
	static Locstate	left_st_p = NULL, right_st_p = NULL;
	static FT_POINT	*p2 = NULL;

	debug_print("B_node","Entered modify_B_node()\n");
	if (p2 == NULL) 
	{
	    p2 = Static_point(fr->interf);
	    if (fr->sizest)
	    {
	    	alloc_state(fr->interf, &left_st_a,fr->sizest);
	    	alloc_state(fr->interf,&right_st_a,fr->sizest);
	    	alloc_state(fr->interf, &left_st_p,fr->sizest);
	    	alloc_state(fr->interf,&right_st_p,fr->sizest);
	    }
	}

		/* Keep cross point on outer boundary  */
		/* IF outer boundary is appropriate.   */
		/* Modified to handle internal Neumann */
		/* boundaries. A more general approach */
		/* to deal with internal boundaries is */
		/* required.			       */

	if ((is_bdry(oldn) && is_bdry(oldcaprop->curve) &&
	     (to_next_node_only(flag) == YES))
				||
	    ((continue_past_fixed_node(flag) == YES) &&
	     is_bdry(oldcaprop->curve)))
	{
	    nearest_boundary_point(Coords(pc),Coords(pc),fr->rect_grid);
	}

			/* Interpolate physical states */

	left_state_along_bond(tcr_phys,crossbphys,newcphys->curve,left_st_p);
	right_state_along_bond(tcr_phys,crossbphys,newcphys->curve,right_st_p);
	if (debugging("B_node") && fr->sizest) 
	{
	    (void) printf("Phys curve states\n");
	    (void) printf("left state:  ");
	    (*fr->print_state)( left_st_p);
	    (void) printf("right state: ");
	    (*fr->print_state)(right_st_p);
	    (void) printf("\n");
	}

			/* Interpolate ahead states */

	left_state_along_bond(tcr_ahead,crossbahead,newcaprop->curve,left_st_a);
	right_state_along_bond(tcr_ahead,crossbahead,newcaprop->curve,
								right_st_a);
	if (debugging("B_node") && fr->sizest) 
	{
	    (void) printf("Ahead states\n");
	    (void) printf("left state:  ");
	    (*fr->print_state)(left_st_a);
	    (void) printf("right state: ");
	    (*fr->print_state)(right_st_a);
	    (void) printf("\n");
	}


	/*  POINT_PROPAGATE CALL ALONE IS INAPPROPRIATE FOR SETTING   */
	/*    STATES FOR SOME FLOWS, CAUSING BOUNDARY PROPAGATION     */
	/* PROBLEMS HENCE, CODE FOLLOWING PT_PROP CALL HAS BEEN ADDED */

		/* Obtain behind states by propagating behind curve */

	point_propagate(fr,wave,oldn->posn,p2,
			Bond_at_node_of_o_curve(oldcbehind),
			oldcbehind->curve,dt,V);
	if (oldcbehind->orient != newcbehind->orient)
	    reverse_states_at_point(p2,fr);

		/* Obtain behind states on physical side */
		/*    from incident (physical) curve     */

	if (curve_ang_oriented_l_to_r(i_to_prop_dir,oldcbehind->orient))
	    p_side_of_b = POSITIVE_SIDE;
	else
	    p_side_of_b = NEGATIVE_SIDE;

	if (curve_ang_oriented_l_to_r(i_to_prop_dir,oldcphys->orient))
	    b_side_of_p = NEGATIVE_SIDE;
	else
	    b_side_of_p = POSITIVE_SIDE;

	if (fr->sizest)
	{
	    if ((wave_type(oldcbehind->curve) == SUBDOMAIN_BOUNDARY) &&
	        (wave_type(newcaprop->curve) == DIRICHLET_BOUNDARY))
	    {
	    /* The node is crossing the corner from a subdomain boundary
	     * to a Dirichlet boundary, so states on both sides of the
	     * new behind boundary need to be copied from behind the
	     * physical curve.
	     */

	        if (p_side_of_b == NEGATIVE_SIDE)
	        {
	    	    ft_assign(left_state(p2),left_st_p,fr->sizest);
	    	    obstacle_state(fr->interf,right_state(p2),fr->sizest);
	        }
	        else
	        {
	    	    obstacle_state(fr->interf,left_state(p2),fr->sizest);
	    	    ft_assign(right_state(p2),right_st_p,fr->sizest);
	        }
	    }
	    else if (p_side_of_b == b_side_of_p)
	    {
	        if (p_side_of_b == NEGATIVE_SIDE)
	    	    ft_assign(left_state(p2),left_st_p,fr->sizest);
	        else
	    	    ft_assign(right_state(p2),right_st_p,fr->sizest);
	    }
	    else
	    {
	        if (p_side_of_b == NEGATIVE_SIDE)
	    	    ft_assign(left_state(p2),right_st_p,fr->sizest);
	        else
	    	    ft_assign(right_state(p2),left_st_p,fr->sizest);
	    }
	}
	if (debugging("B_node"))
	{
	    print_side("p_side_of_b ",p_side_of_b," ");
	    print_side("b_side_of_p ",b_side_of_p,"\n");
	}

	if (debugging("B_node") && fr->sizest)
	{
	    (void) printf("Behind states\n");
	    (void) printf("left state: ");
	    (*fr->print_state)(left_state(p2));
	    (void) printf("right state: ");
	    (*fr->print_state)(right_state(p2));
	    (void) printf("\n");
	}
		/* Modify boundary curves in vicinity of node */

	shift_node_past(pc,crossbahead,newcaprop->curve,newcaprop->orient,
			newcbehind->curve,newcbehind->orient,i_to_prop_dir,
			newn,fr,flag,left_st_a,right_st_a,left_state(p2),
			right_state(p2));

	cut_curve(pc,crossbphys,newcphys->curve,newcphys->orient,fr,
				left_st_p,right_st_p);

	if (continue_past_fixed_node(flag) == YES)
	{
	    newcbprop->curve = adjacent_curve(newcphys->curve,newcphys->orient,
					      Opposite_ang_dir(i_to_prop_dir),
					      &newcbprop->orient);
	    copy_o_curve(oldcbprop,oldcaprop);
	}

	if (fr->B_node_bifurcation)
	    status = (*fr->B_node_bifurcation)(fr,wave,oldcphys,newcphys,
			                       oldcahead,newcahead,oldcaprop,
					       newcaprop,oldcbehind,newcbehind,
					       oldcbprop,newcbprop,oldn->posn,
			                       left_st_p,right_st_p,
					       i_to_prop_dir,rp,dt,
					       dt_frac,flag);
	else
	    status = GOOD_NODE;

	debug_print("B_node","Left modify_B_node()\n");
	return	status;
}		/*end modify_B_node*/

LOCAL	void	move_node(
	NODE  *n,
	double *coords)
{
	CURVE **c;
	int i, dim = n->interface->dim;
	for (i = 0; i < dim; ++i)
	    Coords(n->posn)[i] = coords[i];
	for (c = n->in_curves; c && *c; ++c)
	    set_bond_length((*c)->last,dim);
	for (c = n->out_curves; c && *c; ++c)
	    set_bond_length((*c)->first,dim);
}		/*end move_node */
#endif /* defined(TWOD) */
