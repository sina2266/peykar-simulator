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
*				frp2.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	Contains supplementary routines for the resolution of two
*	dimensional Riemann problems.
*/

#if defined(TWOD)

#define DEBUG_STRING "2drp"
#include <front/fdecs.h>		/* includes int.h, table.h */

	/* LOCAL Function Declarations */
LOCAL	CURVE	*merge_propagated_curves(O_CURVE*,O_CURVE*,O_CURVE*,O_CURVE*,
					 NODE**,Front*,POINTER,RPROBLEM*,double);
LOCAL	boolean	test_ocurves_for_cross(O_CURVE*,O_CURVE*,CROSS*,CROSS**,
				       Front*,POINTER,double);
LOCAL	void	connect_curves_at_new_node(NODE**,O_CURVE**,BOND**,double*,
					   FT_POINT*,Front*);
LOCAL	void	ensure_node_crosses_bdry(Front*,RP_NODE*,double);
LOCAL 	void 	f_replace_null_curves_in_family(O_CURVE_FAMILY**,RPROBLEM*);

/*
*		incident_curve_crosses_fixed_node():
*
*	This routine performs the riemann problem for a physical node
*	crossing a fixed_node on a boundary.  It is assumed that the
*	rproblem structure "rp" is non empty as it uses the structure to
*	find the nodes and curves involved.
*
*	The main call in this routine is to (*front->node_propagate)().
*
*	If node_propagate() fails with the return of a non-null newrp,
*	the status is checked to see if one of the the two curves
*	has propagated completely past the other.  The flag
*	CROSS_PAST_CURVE_NODE is reserved for this case and it
*	is an error in the node status functions of fcrstatus.c
*	if CROSS_PAST_CURVE_NODE is returned for any other case.
*	In this case the function curve_exits_parallel_to_bdry()
*	is called to resolve the interaction,  otherwise control
*	is returned to the calling routine with a return status of
*	MODIFY_TIME_STEP.
*	
*	Successful return of node_propagate() returns control to the calling
*	routine with a return status of GOOD_STEP.
*/

/*ARGSUSED*/
EXPORT	int incident_curve_crosses_fixed_node(
	Front		*front,
	Front		*newfront,
	POINTER		wave,
	RPROBLEM	*rp)
{
	RPROBLEM	*newrp = NULL;
	RP_NODE		*rp_fixedn, *rpn, **rpn_list;
	O_CURVE		*ahead;
	NODE		*nf, *nl;
	CURVE		*ctmp;
	double		dt_frac;
	ORIENTATION	ctmp_orient;
	int		i, num_cross_node;
	NODE_FLAG	flag;
	int		status;
	boolean		debug_added = NO;
	boolean		apply_cfl;

		/* find the fixed node in the rproblem */

	debug_print("iccfn","Entered incident_curve_crosses_fixed_node()\n");

	clear_node_flag(flag);
	continue_past_fixed_node(flag) = YES;
	continue_past_bifurcation(flag) = YES;
	nf = Node_of_o_curve(rp->bdry_curves->first);
	nl = Node_of_o_curve(rp->bdry_curves->last);
	if (is_fixed_node(nf) || fixed_type_node(nf))
	{
	    if (!rp_node_with_node(&rp_fixedn,rp,nf))
	    {
	        screen("ERROR in incident_curve_crosses_fixed_node() "
		       "rp_node_with_node() failed\n");
		clean_up(ERROR);
	    }
	    ahead = rp->bdry_curves->first;
	}
	else if (is_fixed_node(nl) || fixed_type_node(nl))
	{
	    if (!rp_node_with_node(&rp_fixedn,rp,nl))
	    {
	        screen("ERROR in incident_curve_crosses_fixed_node() "
		       "rp_node_with_node() failed\n");
		clean_up(ERROR);
	    }
	    ahead = rp->bdry_curves->last;
	}
	else
	{
	    (void) printf("WARNING in incident_curve_crosses_fixed_node(), "
	                  "No fixed node in "
	                  "incident_curve_crosses_fixed_node()\n");
	    (void) printf("Probable error in identifying rproblem\n");
	    (void) printf("Attempting repropagation at larger dt\n");
	    rp->dt_frac = Max_time_step_modification_factor(front);
	    return MODIFY_TIME_STEP;
	}
	if (!next_boundary(ahead->curve,ahead->orient,&ctmp,&ctmp_orient))
	{
	    screen("ERROR in incident_curve_crosses_fixed_node() "
	           "next_boundary() failed\n");
	    clean_up(ERROR);
	}
	if (debugging("iccfn"))
	{
	    (void) printf("Input rp\n");
	    print_rproblem(rp);
	    (void) printf("rp_fixedn\n");
	    print_rp_node(rp_fixedn,rp);
	    (void) printf("ahead OCURVE\n");
	    print_o_curve(ahead);
	    (void) printf("next boundary curve, orient = %s\n",
	                  orientation_name(ctmp_orient));
	    print_curve(ctmp);
	}


	rpn_list = NULL;
	num_cross_node = 0;
	while (is_null_curve(ctmp,rp))
	{
	    ++num_cross_node;
	    ctmp_orient = Opposite_orient(ctmp_orient);
	    if(!rp_node_with_node(&rpn,rp,Node_of(ctmp,ctmp_orient)))
	    {
	        screen("ERROR in incident_curve_crosses_fixed_node() "
		       "rp_node_with_node() failed\n");
		clean_up(ERROR);
	    }
	    if (!add_to_pointers(rpn,&rpn_list))
	    {
	        screen("ERROR in incident_curve_crosses_fixed_node() "
		       "add_to_pointers() failed\n");
		clean_up(ERROR);
	    }
	    if (!next_boundary(ctmp,ctmp_orient,&ctmp,&ctmp_orient))
	    {
	        screen("ERROR in incident_curve_crosses_fixed_node() "
		       "next_boundary() failed\n");
		clean_up(ERROR);
	    }
	}
	if (debugging("iccfn"))
	    (void) printf("num_cross_node = %d\n",num_cross_node);


	apply_cfl = Apply_CFL_at_nodes(front);
	Apply_CFL_at_nodes(front) = NO;
	for (i = 0; i < num_cross_node; ++i)
	{
	    if (debugging("iccfn"))
	    {
	    	(void) printf("Calling node_propagate\n");
	    	if (!debugging("B_node"))
	    	{
	    	    add_to_debug("B_node");
	    	    debug_added = YES;
	    	}
	    }
	    dt_frac = 1.0;
	    status = (*front->node_propagate)(front,wave,rpn_list[i]->old_node,
					      rpn_list[i]->node,&newrp,rp->dt,
					      &dt_frac,flag,NULL);
	    if (debugging("iccfn"))
	    {
	    	if (debug_added)
	    	    remove_from_debug("B_node");
	    }

	    if (status == MODIFY_TIME_STEP_NODE)
	    {
	        rp->dt_frac = Min_time_step_modification_factor(front);
	        free_rp(newrp);
	        status = MODIFY_TIME_STEP;
	        break;
	    }

	    if (status == CROSS_PAST_CURVE_NODE)
	    {
	        RPROBLEM *rp1, *rp2;
	        /* TO DO: Correct for multiple curve exits */
	        merge_rproblems(rp,newrp);	newrp = NULL;
	        rp2 = rp->prev;
	        while(rp2) 
	        {
	            rp1 = rp2->prev;
	            merge_and_delete_overlaps(rp,rp2);
		    rp2 = rp1;
		}

		(*front->init_2drproblem)(rp,front);


		status = curve_exits_parallel_to_bdry(front,wave,rp);
		if (status == GOOD_STEP)
		    status = GOOD_NODE;
		else
		{
		    status = ERROR_NODE;
		    (void) printf("WARNING in "
				  "incident_curve_crosses_fixed_node(), "
				  "curve_exits_parallel_to_bdry failed\n");
		    (void) printf("status set to ERROR_NODE\n");
		}
	    }

	    if (status != GOOD_NODE) 
	    {
	        (void) printf("WARNING in incident_curve_crosses_fixed_node(),"
		              " node propagation returns %s\n",
			      node_status_as_string(status));
		(void) printf("Attempting repropagation at larger dt\n");
	    	rp->dt_frac = Max_time_step_modification_factor(front);
		free_rp(newrp);
		status = MODIFY_TIME_STEP;
		break;
	    }
	    status = GOOD_STEP;
	}
	Apply_CFL_at_nodes(front) = apply_cfl;
	if (debugging("iccfn"))
	{
	    (void) printf("Interface after "
	                  "incident_curve_crosses_fixed_node()\n");
	    print_interface(rp->new_intfc);
	}
	debug_print("iccfn","Left incident_curve_crosses_fixed_node()\n");
	return status;
}		/*end incident_curve_crosses_fixed_node*/

/*
*		phys_node_crosses_bdry():
*
*	A physical node exits through a boundary.
*/

/*ARGSUSED*/
EXPORT	int phys_node_crosses_bdry(
	Front		*front,
	Front		*newfront,
	POINTER		wave,
	RPROBLEM	*rp,
	NODE_FLAG	flag)
{
	RP_NODE		*rpn_phys;
	RPROBLEM	*rp_new = NULL, *loc_rp = NULL;
	CROSS		*cross;
	int		status;
	int		utflg = NORMAL_ATTEMPT_TO_UNTANGLE;
	double		dt_frac = 1.0;
	NODE		*new_node = NULL;

	debug_print("pncb","Entered phys_node_crosses_bdry()\n");

	if (debugging("pncb"))
	{
	    (void) printf("Old interface\n");
	    print_interface(front->interf);
	    (void) printf("New interface\n");
	    print_interface(newfront->interf);
	}

		/* Identify exiting physical node */

	find_rpn_with_physical_node(&rpn_phys,rp,YES);


	to_next_node_only(flag) = YES;
	node_velocity_preset(flag) = YES;
	continue_past_bifurcation(flag) = YES;
	ensure_node_crosses_bdry(front,rpn_phys,rp->dt);

		/* Propagate node through boundary */

	status = (*front->node_propagate)(front,wave,rpn_phys->old_node,
				          rpn_phys->node,&rp_new,rp->dt,
				          &dt_frac,flag,NULL);
	
	if (status != GOOD_NODE)
	{
	    free_rp(rp_new);
	    free_rp(loc_rp);
	    if (status == MODIFY_TIME_STEP_NODE)
		status = MODIFY_TIME_STEP;
	    else
	    {
		status = ERROR_IN_STEP;
	        (void) printf("WARNING in phys_node_crosses_bdry(), "
	                      "node propagate failed for rp, status = %s\n",
			      node_status_as_string(status));
	    }
	    return status;
	}

	if (!generate_boundary_cross_list(&cross,rp,front,wave))
	{
	    (void) printf("WARNING in phys_node_crosses_bdry(), "
	                  "generate_boundary_cross_list() failed\n");
	    return MODIFY_TIME_STEP;
	}

	status = (front->fr_bdry_untangle != NULL) ?
		(*front->fr_bdry_untangle)(newfront,&cross,rp,new_node,utflg) :
	    	f_boundary_untangle(newfront,&cross,rp,new_node,utflg);
	switch (status)
	{
	case CURVES_UNTANGLED:
	    status = GOOD_STEP;
	    break;
	case MODIFY_TIME_STEP_TO_UNTANGLE:
	case MODIFY_TIME_STEP_TO_UNTANGLE_BUT_FORCE_TANGLE:
	    if (debugging("pncb"))
	    {
	    	(void) printf("boundary untangle returns "
	    	              "MODIFY_TIME_STEP_TO_UNTANGLE\n");
	    }
	    free_rp(rp_new);
	    free_rp(loc_rp);
	    debug_print("pncb","Left phys_node_crosses_bdry()\n");
	    return MODIFY_TIME_STEP;
	case ERROR_IN_UNTANGLE:
	default:
	    free_rp(rp_new);
	    free_rp(loc_rp);
	    (void) printf("WARNING in phys_node_crosses_bdry(), "
	                  "boundary untangle() failed\n");
	    debug_print("pncb","Left phys_node_crosses_bdry()\n");
	    return ERROR_IN_STEP;
	}

	delete_null_physical_curves(rp);
	(void) delete_node(rpn_phys->node);
	rpn_phys->node = NULL;
	delete_null_boundary_curves(rp,newfront,wave);

	free_rp(rp_new);
	free_rp(loc_rp);
	if (debugging("pncb"))
	{
	    (void) printf("Untangled interface\n");
	    print_interface(newfront->interf);
	}
	debug_print("pncb","Left phys_node_crosses_bdry()\n");

	return GOOD_STEP;
}		/*end phys_node_crosses_bdry*/


LOCAL	void ensure_node_crosses_bdry(
	Front		*front,
	RP_NODE		*rpn,
	double		dt)
{
	NODE		*oldn = rpn->old_node, *newn = rpn->node;
	double		pb[MAXD];
	double		*L = front->rect_grid->L, *U = front->rect_grid->U;
	double		*h = front->rect_grid->h;
	double		*nv = Node_vel(newn), ns;
	double		ds;
	int		i;
	int		dim = front->rect_grid->dim;

	Check_return(
	    intersect_ray_with_boundary(Coords(oldn->posn),nv,L,U,pb,dim),
	    ensure_node_crosses_bdry)

	for (i = 0; i < dim; ++i)
	    pb[i] -= Coords(oldn->posn)[i];
	ds = scaled_hypot(pb,h,dim);
	for (i = 0; i < dim; ++i)
	    pb[i] *= (1.0 + 0.5/ds);/*TOLERANCE*/
	ds = mag_vector(pb,dim);
	ns = mag_vector(nv,dim);
	if (ds > ns*dt)
	{
	    /* Node velocity is too slow to reach the boundary */

	    ns = ds/(dt*ns);
	    for (i = 0; i < dim; ++i)
		nv[i] *= ns;
	}
}		/*end ensure_node_crosses_bdry*/

EXPORT	boolean generate_boundary_cross_list(
	CROSS		**cross,
	RPROBLEM	*rp,
	Front		*front,
	POINTER		wave)
{
	CROSS		Cr, *cr;
	O_CURVE		*physoc;
	O_CURVE		BOC;
	int		cr_found;

	*cross = NULL;
	Cr.next = Cr.prev = NULL;
	cr = &Cr;
	for (physoc = rp->ang_ordered_curves->first; physoc != NULL;
				physoc = physoc->next)
	{
	    if (wave_type(physoc->curve) >= FIRST_PHYSICS_WAVE_TYPE)
	    {
	    	cr_found = NO;
	    	BOC.curve = rp->bdry_curves->first->curve;
	    	BOC.orient = rp->bdry_curves->first->orient;
	    	while (BOC.curve != NULL)
	    	{
	    	    if (test_ocurves_for_cross(physoc,&BOC,&Cr,&cr,
	    				front,wave,rp->dt))
	    	    	cr_found = YES;
	    	    if (BOC.curve == rp->bdry_curves->last->curve)
	    	    	break;
	    	    Check_return(
			next_boundary(BOC.curve,BOC.orient,&BOC.curve,
				      &BOC.orient),
			generate_boundary_cross_list)
	    	    if (BOC.curve != rp->bdry_curves->last->curve)
	    	    	BOC.orient =Opposite_orient(BOC.orient);
	    	}
	    	if (!cr_found) return NO;
	    }
	    if (physoc == rp->ang_ordered_curves->last) break;
	}
	*cross = Cr.next;
	if (*cross)
	    (*cross)->prev = NULL;
	return YES;
}		/*end generate_boundary_cross_list*/

LOCAL	boolean test_ocurves_for_cross(
	O_CURVE		*oc1,
	O_CURVE		*oc2,
	CROSS		*crhead,
	CROSS		**cross,
	Front		*front,
	POINTER		wave,
	double		dt)
{
	CROSS	  *cr;
	BOND	  *crb1, *crb2;
	FT_POINT	  *pcr;
	NODE_FLAG flag;
	double	  s1, s2;
	int	  dim = front->interf->dim;

	set_ordinary_cross_only(flag);
	if (intersection_of_two_o_curves(oc1,NULL,oc2,NULL,
				      &crb1,&crb2,&pcr,&s1,&s2,
				      front,wave,dt,flag) == NO)
		return NO;
	
	/* Allocate new cross structure */

	for (cr = crhead->next; cr != NULL; cr = cr->next)
	{
	    if (cr->c1 != oc1->curve)
		continue;
	    if (oc1->orient == POSITIVE_ORIENTATION)
	    {
	    	if (points_in_strict_order(cr->p,cr->b1,pcr,crb1,dim) == YES)
	    	    return YES;
	    }
	    else
	    {
	    	if (points_in_strict_order(pcr,crb1,cr->p,cr->b1,dim) == YES)
	    	    return YES;
	    }
	    cr->p = pcr;
	    cr->c1 = oc1->curve;
	    cr->b1 = crb1;
	    cr->c2 = oc2->curve;
	    cr->b2 = crb2;
	    return YES;
	}

	add_to_cross_list(cross,oc1->curve,crb1,oc2->curve,crb2,pcr);
	return YES;
}		/*end test_ocurves_for_cross*/

EXPORT	void find_rpn_with_physical_node(
	RP_NODE		**rpn_phys,
	RPROBLEM	*rp,
	int		interior_node_only)
{
	RP_NODE		*rpn;

	for (rpn = rp->first_rp_node; rpn != NULL; rpn = rpn->next)
	{
		if (node_type(rpn->node) >= FIRST_PHYSICS_NODE_TYPE)
		{
			if (!interior_node_only)      break;
			else if (!is_bdry(rpn->node)) break;
		}
	}
	
	*rpn_phys = rpn;
	if (*rpn_phys == NULL)
	{
		screen("ERROR in find_rpn_with_physical_node(), ");
		screen("No physical node in rproblem\n");
		print_rproblem(rp);
		clean_up(ERROR);
	}
}		/*end find_rpn_with_physical_node*/


/*
*		merge_propagated_curves():
*
*	Given two curves that have been propagated except at their nodes,
*	this functions merges the two curves into a common curve.
*/

LOCAL	CURVE *merge_propagated_curves(
	O_CURVE		*oldc1,
	O_CURVE		*newc1,
	O_CURVE		*oldc2,
	O_CURVE		*newc2,
	NODE		**newn,
	Front		*front,
	POINTER		wave,
	RPROBLEM	*rp,
	double		dt)
{
	FT_POINT		*p1, *p2;
	INTERFACE	*intfc = newc1->curve->interface;
	CURVE		*cur;
	boolean		sav_interp = interpolate_intfc_states(intfc);
	size_t		sizest = front->sizest;
	double		V[MAXD];
	Locstate	st_l, st_r;

	p1 = Point(NULL);	p2 = Point(NULL);
	point_propagate(front,wave,Node_of_o_curve(oldc1)->posn,
			p1,Bond_at_node_of_o_curve(oldc1),oldc1->curve,dt,V);
	if (oldc1->orient != newc1->orient)
		reverse_states_at_point(p1,front);
	point_propagate(front,wave,Node_of_o_curve(oldc2)->posn,
			p2,Bond_at_node_of_o_curve(oldc2),oldc2->curve,dt,V);
	if (oldc2->orient != newc2->orient)
		reverse_states_at_point(p2,front);

	if ((*newn = make_node(p2)) == NULL) return NULL;
	change_node_of_curve(newc2->curve,newc2->orient,*newn);
	interpolate_intfc_states(intfc) = NO;
	insert_point_adjacent_to_node(p1,newc1->curve,newc1->orient);
	interpolate_intfc_states(intfc) = YES;
	change_node_of_curve(newc1->curve,newc1->orient,*newn);
	st_l = left_state(p2);
	st_r = right_state(p2);
	if (newc1->orient == newc2->orient)
	{
		invert_curve(newc1->curve);
		newc1->orient = Opposite_orient(newc1->orient);
	}
	if (newc2->orient == POSITIVE_ORIENTATION)
	{
		ft_assign(left_end_state(newc1->curve),st_l,sizest);
		ft_assign(right_end_state(newc1->curve),st_r,sizest);
		ft_assign(left_start_state(newc2->curve),st_l,sizest);
		ft_assign(right_start_state(newc2->curve),st_r,sizest);
	}
	else
	{
		ft_assign(left_start_state(newc1->curve),st_l,sizest);
		ft_assign(right_start_state(newc1->curve),st_r,sizest);
		ft_assign(left_end_state(newc2->curve),st_l,sizest);
		ft_assign(right_end_state(newc2->curve),st_r,sizest);
	}
	if (newc1->curve == newc2->curve)
	{
		node_type(*newn) = CLOSED_NODE;
		cur = newc1->curve;
	}
	else if (newc2->orient == POSITIVE_ORIENTATION)
	{
		cur = join_curves(newc1->curve,newc2->curve,
			          negative_component(newc2->curve),
			          positive_component(newc2->curve),NULL);
		roclists_after_join(rp,newc1->curve,newc1,
					newc2->curve,newc2,cur);
		(void) delete_node(*newn);
		*newn = NULL;
		newc2->curve = newc1->curve = NULL;
	}
	else
	{
		cur = join_curves(newc2->curve,newc1->curve,
			          negative_component(newc2->curve),
			          positive_component(newc2->curve),NULL);
		roclists_after_join(rp,newc2->curve,newc2,
					newc1->curve,newc1,cur);
		(void) delete_node(*newn);
		*newn = NULL;
		newc2->curve = newc1->curve = NULL;
	}
	interpolate_intfc_states(intfc) = sav_interp;
	return cur;
}		/*end merge_propagated_curves*/


EXPORT	int join_propagated_curves(
	NODE		**newn,
	CURVE		**newc,
	O_CURVE		**oc,
	O_CURVE		**oldoc,
	int		add_new_node,
	Front		*front,
	Front		*newfront,
	POINTER		wave,
	RPROBLEM	*rp)
{
	BOND	  *crb[2];
	NODE_FLAG flag;
	FT_POINT	  *pcr;
	double	  t_cr[2];
	double	  dt = rp->dt;

	debug_print("join_prop","Entered join_propagated_curves()\n");

	set_ordinary_cross_only(flag);
	*newn = NULL;
	*newc = NULL;
	if (!oc)
	{
	    (void) printf("WARNING in join_propagated_curves(), oc == NULL\n");
	    debug_print("join_prop","Left join_propagated_curves()\n");
	    return ERROR_IN_STEP;
	}

	if (!oc[0] || !oc[1])
	{
	    if (debugging("join_prop"))
	    {
	    	if (oc[0] == NULL)
	    	    (void) printf("oc[0] == NULL, ");
	    	else
	    	    (void) printf("oc[0] != NULL, ");
	    	if (oc[1] == NULL)
	    	    (void) printf("oc[1] == NULL\n");
	    	else
	    	    (void) printf("oc[1] != NULL\n");
	    }
	    debug_print("join_prop","Left join_propagated_curves()\n");
	    return GOOD_STEP;
	}

	if (debugging("join_prop"))
	{
	    (void) printf("Joining propagated curve pair\n");
	    (void) printf("add_new_node = %s\n",(add_new_node) ? "YES" : "NO");
	    (void) printf("oc[0] = %p, oc[0]->curve = %llu, ",
	    	          (POINTER)oc[0],curve_number(oc[0]->curve));
	    print_orientation("oc[0]->orient = ",oc[0]->orient,"\n");
	    (void) printf("oc[1] = %p, oc[1]->curve = %llu, ",
	    	          (POINTER)oc[1],curve_number(oc[1]->curve));
	    print_orientation("oc[1]->orient = ",oc[1]->orient,"\n");
	    if (oc[0]->curve == oc[1]->curve)
	    	(void) printf("New closed curve will be formed\n");
	}

	/* Check for intersection of propagated curves */

	if (intersection_of_two_o_curves(oc[0],oldoc[0],oc[1],oldoc[1],crb,
					 crb+1,&pcr,t_cr,t_cr+1,front,wave,dt,
					 flag) == YES)
	{
	    if (debugging("join_prop"))
	    	(void) printf("Propagated curves intersect\n");

	    connect_curves_at_new_node(newn,oc,crb,t_cr,pcr,newfront);

	    if (oc[0]->curve == oc[1]->curve)
	    {
	    	node_type(*newn) = CLOSED_NODE;
	    	if (!add_new_node)
	    	{
	    	    CURVE *c = oc[0]->curve;
	    	    double *crds = Coords((*newn)->posn);
	    	    size_t sizest = front->sizest;
	    	    static Locstate st = NULL;

	    	    if (st == NULL)
	    	    	alloc_state(front->interf,&st,sizest);
	    	    interpolate_states(front,0.5,0.5,
	    			       crds,left_start_state(c),
	    			       crds,left_end_state(c),st);
	    	    ft_assign(left_start_state(c),st,sizest);
	    	    ft_assign(left_end_state(c),st,sizest);
	    	    interpolate_states(front,0.5,0.5,
	    			       crds,right_start_state(c),
	    			       crds,right_end_state(c),st);
	    	    ft_assign(right_start_state(c),st,sizest);
	    	    ft_assign(right_end_state(c),st,sizest);
	    	}
	    }
	    else if (!add_new_node)
	    {
	    	interpolate_intfc_states(oc[0]->curve->interface) = YES;
		if (oc[0]->orient == oc[1]->orient)
		{
		    invert_curve(oc[0]->curve);
		    roclists_after_invert(rp,oc[0]->curve,oc[0]);
		}
		if (oc[1]->orient == POSITIVE_ORIENTATION)
		{
		    *newc = join_curves(oc[0]->curve,oc[1]->curve,
				        negative_component(oc[1]->curve),
				        positive_component(oc[1]->curve),NULL);
		    roclists_after_join(rp,oc[0]->curve,oc[0],
				        oc[1]->curve,oc[1],*newc);
		}
		else
		{
		    *newc = join_curves(oc[1]->curve,oc[0]->curve,
				        negative_component(oc[1]->curve),
				        positive_component(oc[1]->curve),NULL);
		    roclists_after_join(rp,oc[1]->curve,oc[1],
				        oc[0]->curve,oc[0],*newc);
		}
		if (*newc == NULL)
		{
		    (void) printf("WARNING in join_propagated_curves(), "
		           "join_curves() returns NULL\n");
		    return ERROR_IN_STEP;
		}
		(void) delete_node(*newn);
		rrpnlist_after_delete_node(rp,*newn);
		newn = NULL;
	    }
	    debug_print("join_prop","Left join_propagated_curves()\n");
	    return GOOD_STEP;
	}

	if (debugging("join_prop"))
	    (void) printf("Propagated curves don't intersect\n");

	/* Join non-intersecting curves */

	*newc = merge_propagated_curves(oldoc[0],oc[0],oldoc[1],oc[1],newn,
					front,wave,rp,dt);

	if (debugging("join_prop"))
	{
	    (void) printf("merged curve\n");
	    print_curve(*newc);
	}

	if (*newc == NULL)
	{
	    (void) printf("WARNING in join_propagated_curves(), "
	                  "merge_propagated_curves() returns NULL\n");
	    return ERROR_IN_STEP;
	}
	debug_print("join_prop","Left join_propagated_curves()\n");
	return GOOD_STEP;
}		/*end join_propagated_curves*/

LOCAL void connect_curves_at_new_node(
	NODE		**newn,
	O_CURVE		**sh,
	BOND		**crb,
	double		*s,
	FT_POINT		*pcr,
	Front		*front)
{
	int		i;
	static Locstate st_l = NULL, st_r = NULL;

	if (st_r == NULL)
	{
	    alloc_state(front->interf,&st_l,front->sizest);
	    alloc_state(front->interf,&st_r,front->sizest);
	}

	*newn = make_node(pcr);
	for (i = 0; i < 2; ++i)
	{
	    left_state_along_bond(s[i],crb[i],sh[i]->curve,st_l);
	    right_state_along_bond(s[i],crb[i],sh[i]->curve,st_r);
	    change_node_of_curve(sh[i]->curve,sh[i]->orient,*newn);
	    cut_curve(pcr,crb[i],sh[i]->curve,sh[i]->orient,front,st_l,st_r);
	}
}		/*end connect_curves_at_new_node*/


EXPORT boolean rp_node_with_node(
	RP_NODE		**rpn,
	RPROBLEM	*rp,
	NODE		*node)
{
	RP_NODE		*rp_node;

	*rpn = NULL;
	if (node == NULL) return NO;
	for (rp_node = rp->first_rp_node; rp_node; rp_node = rp_node->next)
	{
	    if (rp_node->node == node)
	    {
	    	*rpn = rp_node;
	    	return YES;
	    }
	    else if (rp_node->old_node == node)
	    {
	    	*rpn = rp_node;
	    	return YES;
	    }
	}
	return NO;
}		/*end rp_node_with_node*/

EXPORT	boolean	fixed_type_node(
	NODE *n)
{
	CURVE **c;

	for (c = n->in_curves; c && *c; ++c)
	{
	    if (wave_type(*c) >= FIRST_PHYSICS_WAVE_TYPE)
	        return NO;
	}
	for (c = n->out_curves; c && *c; ++c)
	{
	    if (wave_type(*c) >= FIRST_PHYSICS_WAVE_TYPE)
	        return NO;
	}
	return YES;
}		/*end boundary_curve_at_node*/

/*
*			bdry_rp_1i_0f():
*
*	Handles the case of one incident curve && no fixed nodes. This
*	involves a bifurcation of one of the following types: B_node ->
*	regular reflection, mach stem or attached_B_node, regular reflection 
*	-> mach stem or attached_B_node -> B_node + fixed_node.
*/

EXPORT int bdry_rp_1i_0f(
	Front		*front,
	POINTER		wave,
	RPROBLEM	*rp)
{
	NODE_FLAG	flag;
	int		status;
	NODE		*oldn,*newn;
	RP_NODE		*rpn;
	RPROBLEM	*newrp = NULL;
	double		dt_frac;

	DEBUG_ENTER(bdry_rp_1i_0f)
	clear_node_flag(flag);
		/* TODO is this value for flag correct? */
	to_next_node_only(flag) = YES;
	continue_past_bifurcation(flag) = YES;

	for (rpn = rp->first_rp_node; rpn; rpn = rpn->next) 
	{
	    if (is_fixed_node(rpn->node))
		continue;
	    oldn = rpn->old_node;
	    newn = rpn->node;
	    break;
	}
	if (DEBUG)
	    (void) printf("Calling node_propagate\n");
	dt_frac = 1.0;
	status = (*front->node_propagate)(front,wave,oldn,newn,&newrp,
		                          rp->dt,&dt_frac,flag,NULL);
	if (status != GOOD_NODE) 
	{
	    free_rp(newrp);
	    (void) printf("WARNING: in bdry_rp_1i_0f()\n"
	                  "node propagation failed\n");
	    DEBUG_LEAVE(bdry_rp_1i_0f)
	    return ERROR_IN_STEP;
	}
	DEBUG_LEAVE(bdry_rp_1i_0f)
	return GOOD_STEP;
}		/*end bdry_rp_1i_0f*/

EXPORT int bdry_rp_2i(
	Front		*front,
	Front		*newfront,
	POINTER		wave,
	RPROBLEM	*rp)
{
	NODE		*newn;
	CURVE		*newc;
	O_CURVE		*newci[2], *oldci[2];
	int		status;

	DEBUG_ENTER(bdry_rp_2i)
	if (find_curves_at_rp_with_status(oldci,newci,2,rp,INCIDENT) ==
							FUNCTION_FAILED)
	{
	    (void) printf("WARNING in bdry_rp_2i(), "
			  "can't find incident curves\n");
	    DEBUG_LEAVE(bdry_rp_2i)
	    return ERROR_IN_STEP;
	}
	if (newci[0]->orient != newci[1]->orient)
	{
	    if (negative_component(newci[0]->curve) != 
	    			negative_component(newci[1]->curve))
	    {
	    	reset_component_of_loop(newci[0]->curve,newci[0]->orient,
				(newci[0]->orient == NEGATIVE_ORIENTATION) ?
				COUNTER_CLOCK : CLOCKWISE,
				negative_component(newci[1]->curve),front);
	    }
	    if (positive_component(newci[0]->curve) != 
					positive_component(newci[1]->curve))
	    {
	    	reset_component_of_loop(newci[0]->curve,newci[0]->orient,
				(newci[0]->orient == POSITIVE_ORIENTATION) ?
				COUNTER_CLOCK : CLOCKWISE,
				positive_component(newci[1]->curve),front);
	    }
	}
	else
	{
	    if (negative_component(newci[0]->curve) != 
					negative_component(newci[1]->curve))
	    {
	    	reset_component_of_loop(newci[0]->curve,newci[0]->orient,
				(newci[0]->orient == NEGATIVE_ORIENTATION) ?
				COUNTER_CLOCK : CLOCKWISE,
				positive_component(newci[1]->curve),front);
	    }
	    if (positive_component(newci[0]->curve) != 
				negative_component(newci[1]->curve))
	    {
	    	reset_component_of_loop(newci[0]->curve,newci[0]->orient,
				(newci[0]->orient == POSITIVE_ORIENTATION) ?
				COUNTER_CLOCK : CLOCKWISE,
				negative_component(newci[1]->curve),front);
	    }
	}

	status = join_propagated_curves(&newn,&newc,newci,oldci,NO,front,
			                newfront,wave,rp);

	if (status != GOOD_STEP)
	{
	    DEBUG_LEAVE(bdry_rp_2i)
	    return status;
	}

	delete_null_boundary_curves(rp,front,wave);

	DEBUG_LEAVE(bdry_rp_2i)
	return status;
}		/*end bdry_rp_2i*/


EXPORT	boolean find_curves_at_rp_with_status(
	O_CURVE		**oldc,
	O_CURVE		**newc,
	int		numc,
	RPROBLEM	*rp,
	int		status)
{
	O_CURVE		*oc;
	int		i;

	DEBUG_ENTER(find_curves_at_rp_with_status)
	for (i = 0; i < numc; i++)
	{
	    oldc[i] = newc[i] = NULL;
	}
	i = 0;
	for (oc = rp->ang_ordered_curves->first; oc != NULL; oc = oc->next)
	{
	    if (status_at_node(oc->curve,oc->orient) == status)
	    {
	    	if (i >= numc)
	    	{
	    	    DEBUG_LEAVE(find_curves_at_rp_with_status)
	    	    return FUNCTION_FAILED;
	    	}
	    	newc[i++] = oc;
	    }
	    if (i > numc || oc == rp->ang_ordered_curves->last) break;
	}
	if (i != numc)
	{
	    DEBUG_LEAVE(find_curves_at_rp_with_status)
	    return FUNCTION_FAILED;
	}
	i = 0;
	for (oc = rp->old_ang_ordered_curves->first; oc != NULL; oc = oc->next)
	{
	    if (status_at_node(oc->curve,oc->orient) == status)
	    {
	    	if (i >= numc)
	    	{
	    	    DEBUG_LEAVE(find_curves_at_rp_with_status)
	    	    return FUNCTION_FAILED;
	    	}
	    	oldc[i++] = oc;
	    }
	    if (i > numc || oc == rp->old_ang_ordered_curves->last) break;
	}
	DEBUG_LEAVE(find_curves_at_rp_with_status)
	return (i != numc) ? FUNCTION_FAILED : FUNCTION_SUCCEEDED;
}		/*end find_curves_at_rp_with_status*/


EXPORT	int pp_curve_exits_at_bdry(
	Front		*front,
	Front		*newfront,
	POINTER		wave,
	RPROBLEM	**rp)
{
	BOND		*b;
	CURVE		*pc;
	NODE		*interact_nodes[5];
	O_CURVE		*oc, *old_oc;
	RECT_GRID	*gr = (*rp)->fr->rect_grid;
	RP_NODE		*rp_n;
	int		status, dim = gr->dim;

	DEBUG_ENTER(pp_curve_exits_at_bdry)

	/* WARNING: list is circular */
	for (oc = (*rp)->ang_ordered_curves->first,
			old_oc = (*rp)->old_ang_ordered_curves->first;
	    oc != NULL; oc = oc->next, old_oc = old_oc->next)
	{
	    if (wave_type(oc->curve) >= FIRST_PHYSICS_WAVE_TYPE)
	    	break;
	    if (oc == (*rp)->ang_ordered_curves->last)
	    {
	    	DEBUG_LEAVE(pp_curve_exits_at_bdry)
	    	return GOOD_STEP;
	    }
	}

	pc = oc->curve;
	for (b = pc->first; b != pc->last; b = b->next)
	{
	    if (!outside_point(Coords(b->end),gr->VL,gr->VU,dim))
	    {
		/* The physical curve crosses back into the physical domain
		 * at some point. */

		for (rp_n=(*rp)->first_rp_node; rp_n!=NULL; rp_n=rp_n->next)
		    if (node_type(rp_n->node) == SUBDOMAIN_NODE) break;

		if (rp_n == NULL)
		{
		    (void) printf("WARNING in pp_curve_exits_at_boundary(), "
		                  "unable to locate SUBDOMAIN_NODE.\n");
	    	    return ERROR_IN_STEP;
	        }

	        if (((*rp)->bdry_type1 == DIRICHLET_BOUNDARY) ||
		    ((*rp)->bdry_type2 == DIRICHLET_BOUNDARY))
		{
		    node_type(rp_n->node) = DIRICHLET_NODE;
		}
		else if (((*rp)->bdry_type1 == NEUMANN_BOUNDARY) ||
			 ((*rp)->bdry_type2 == NEUMANN_BOUNDARY))
		{
		    node_type(rp_n->node) = NEUMANN_NODE;
		}

		propagation_status(rp_n->node) = UNPROPAGATED_NODE;
		status = incident_curve_crosses_fixed_node(front,newfront,
							   (POINTER)wave,*rp);
		if (status != GOOD_STEP)
		{
		    (void) printf("WARNING in pp_curve_exits_at_bdry(), "
				  "incident_curve_crosses_fixed_node() "
				  "returns %s\n",
				  time_step_status_as_string(status));
		}

		DEBUG_LEAVE(pp_curve_exits_at_bdry)
		return status;
	    }
	}
	interact_nodes[0] = pc->start;
	interact_nodes[1] = old_oc->curve->start;
	interact_nodes[2] = pc->end;
	interact_nodes[3] = old_oc->curve->end;
	interact_nodes[4] = NULL;
	augment_rproblem_list(rp,interact_nodes,(*rp)->dt,(*rp)->dt_frac,
		              (*rp)->old_intfc,(*rp)->new_intfc,(*rp)->fr,
			      (POINTER)wave);
	(*(*rp)->fr->init_2drproblem)(*rp,(*rp)->fr);
	status = curve_exits_parallel_to_bdry((*rp)->fr,(POINTER)wave,*rp);

	DEBUG_LEAVE(pp_curve_exits_at_bdry)
	return status;
}		/*end pp_curve_exits_at_bdry*/


EXPORT  void find_curves_with_status(
	NODE		*n,
	CURVE		**c1,
	ORIENTATION	*orient1,
	CURVE		**c2,
	ORIENTATION	*orient2,
	int		status)
{
	CURVE		**c;

	DEBUG_ENTER(find_curves_with_status)
	*c1 = *c2 = NULL;

	for (c = n->in_curves; c && *c; c++)
	{
	    if (end_status(*c) == status) 
	    {
	    	if (*c1 == NULL) 
	    	{
	    	    *c1 = *c;
	    	    *orient1 = NEGATIVE_ORIENTATION;
	    	}
	    	else 
	    	{
	    	    *c2 = *c;
	    	    *orient2 = NEGATIVE_ORIENTATION;
	    	    DEBUG_LEAVE(find_curves_with_status)
	    	    return;
	    	}
	    }
	}
	for (c = n->out_curves; c && *c; c++)
	{
	    if (start_status(*c) == status) 
	    {
	    	if (*c1 == NULL) 
	    	{
	    	    *c1 = *c;
	    	    *orient1 = POSITIVE_ORIENTATION;
	    	}
	    	else 
	    	{
	    	    *c2 = *c;
	    	    *orient2 = POSITIVE_ORIENTATION;
	    	    DEBUG_LEAVE(find_curves_with_status)
	    	    return;
	    	}
	    }
	}
	DEBUG_LEAVE(find_curves_with_status)
}		/*end find_curves_with_status*/


LOCAL void f_replace_null_curves_in_family(
	O_CURVE_FAMILY	**cfamily,
	RPROBLEM	*rp)
{
	O_CURVE		*oc;
	O_CURVE		Baseoc;

	DEBUG_ENTER(f_replace_null_curves_in_family)
		/* Test for null curves */
	
	if (!*cfamily)
	{
	    DEBUG_LEAVE(f_replace_null_curves_in_family)
	    return;
	}

	Baseoc.curve = NULL;
testnull:
	for (oc = (*cfamily)->first; oc; oc = oc->next)
	    if (is_null_curve(oc->curve,rp)) break;

	if (!oc)
	{
	    DEBUG_LEAVE(f_replace_null_curves_in_family)
	    return;
	}

	if (DEBUG)
	{
	    (void) printf("Null curve found\n");
	    print_curve(oc->curve);
	}

	if (Baseoc.curve == NULL) Baseoc = *oc;

	    /* Replace null curves */

	oc->orient = Opposite_orient(oc->orient);
	switch(node_type(Node_of(oc->curve,oc->orient))) 
	{
	case PASSIVE_NODE:
	case DIRICHLET_NODE:
	case NEUMANN_NODE:
	case FIXED_NODE:
	case SUBDOMAIN_NODE:
	    delete_oc_curve_from_family(&oc,cfamily);
	    break;
	}
	if (*cfamily) goto testnull;
	DEBUG_LEAVE(f_replace_null_curves_in_family)
}		/*end f_replace_null_curves_in_family*/
#endif /* defined(TWOD) */
