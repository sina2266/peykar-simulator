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
*				ftop.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	The routines in this file perform various manipulations on an
*	interface not covered in the intfc library routines.
*/

#include <front/fdecs.h>

	/* LOCAL Function Declarations */
LOCAL	int	merge_cross_with_node(CURVE*,ORIENTATION,CROSS*,COMPONENT*,
				      COMPONENT*,CURVE**);
LOCAL	int	split_bond_at_cross(CROSS*,int,CURVE**,COMPONENT*,COMPONENT*,
				    INTERFACE*,double,POINTER,NODE**);
LOCAL	void	resolve_sink_node_fold_backs(NODE*,int*,Front*);

EXPORT	void	f_reflect_point(
	FT_POINT		*point,/* point being reflected */
	double		*p,	/* point on reflection plane */
	double		*n,	/* normal to plane */
	INTERFACE	*intfc)	/* interface being reflected */
{
	reflect_state( left_state(point),intfc,Coords(point),p,n);
	reflect_state(right_state(point),intfc,Coords(point),p,n);
	if ((intfc->dim == 1) && (size_of_state(intfc) > 0))
	{
	    static Locstate tmpst = NULL;
	    if (tmpst == NULL)
		alloc_state(intfc,&tmpst,size_of_state(intfc));
	    ft_assign(tmpst,left_state(point),size_of_state(intfc));
	    ft_assign(left_state(point),right_state(point),size_of_state(intfc));
	    ft_assign(right_state(point),tmpst,size_of_state(intfc));

	}
	i_reflect_point(point,p,n,intfc);
}		/*end f_reflect_point*/

EXPORT  boolean f_is_subdomain_boundary(
	HYPER_SURF	*hs)
{
	return (wave_type(hs) == SUBDOMAIN_BOUNDARY) ? YES : NO;
}		/*end f_is_subdomain_boundary*/

EXPORT	int	synchronize_untangle_status(
	int status)
{
	int i;
	long gs;
	static struct {int status; long on, off;} action[4] = {
			{ERROR_IN_UNTANGLE,1L,0L},
			{MODIFY_TIME_STEP_TO_UNTANGLE,1L,0L},
		    	{MODIFY_TIME_STEP_TO_UNTANGLE_BUT_FORCE_TANGLE,1L,0L},
		    	{CURVES_UNTANGLED,0L,1L}};

	for (i = 0; i < 4; ++i)
	{
	    gs = (status == action[i].status) ? action[i].on : action[i].off;
	    pp_global_lmax(&gs,1L);
	    if (gs == action[i].on)
	    	return action[i].status;
	}
	return ERROR_IN_UNTANGLE;
}		/*end synchronize_untangle_status*/


EXPORT  void f_invert_curve(
	CURVE		*c)
{
	i_invert_curve(c);
	if (c->interface->dim == 2)
	{
	    BOND	*b;
	    INTERFACE	*intfc = c->interface;
	    Locstate	ls;

	    if (interpolate_intfc_states(intfc) &&
		(size_of_state(intfc) != 0))
	    {
	        ls = left_start_state(c);
	        left_start_state(c) = right_start_state(c);
		right_start_state(c) = ls;
	        for (b = c->first; b != c->last; b = b->next)
	        {
	            ls = left_state(b->end);
	            left_state(b->end) = right_state(b->end);
	            right_state(b->end) = ls;
	        }
	        ls = left_end_state(c);
	        left_end_state(c) = right_end_state(c);
		right_end_state(c) = ls;
	    }
	    if (!rst_cor_after_invert_curve(c))
	    {
	        screen("Failure in invert_curve()\n");
	        screen("Unable to reset curve correspondence list\n");
	        clean_up(ERROR);
	    }
	}
}		/*end f_invert_curve*/

EXPORT  void f_reverse_curve(
	CURVE		*c)
{
	i_reverse_curve(c);
	if (c->interface->dim == 2)
	{
	    Locstate	ls, rs;
	    ls = left_start_state(c);
	    rs = right_start_state(c);
	    left_start_state(c) = left_end_state(c);
	    right_start_state(c) = right_end_state(c);
	    left_end_state(c) = ls;
	    right_end_state(c) = rs;
	}
}		/*end f_reverse_curve*/

EXPORT	boolean	f_untrack_point(
	FT_POINT		*p,
	COMPONENT	newcomp,
	Front		*fr)
{
	INTERFACE	*intfc = fr->interf;
	FT_POINT		**pp;
	int		i;

	debug_print("untrack","Entered f_untrack_point()\n");

	pp = intfc->points;
	if (pp == NULL)
	{
	    (void) printf("WARNING in f_untrack_point(), "
			  "intfc->points is NULL\n");
	    debug_print("untrack","Left f_untrack_point()\n");
	    return NO;
	}

	if (debugging("untrack"))
	{
	   (void) printf("point %llu selected for untracking\n",
			 point_number(p));
	   print_point(p);
	}

	/* Locate tracked fronts to the left and right of p */
	for (i = 0; pp[i] != NULL; ++i)
	    if (p == pp[i])
		break;

	if (i > 0)
	    positive_component(pp[i-1]) = newcomp;
	if ((i+1) < intfc->num_points)
	    negative_component(pp[i+1]) = newcomp;

	positive_component(p) = negative_component(p) = newcomp;
	if (!delete_point(p))
	{
	    screen("ERROR in f_untrack_point(), can't delete point\n");
	    clean_up(ERROR);
	}
	reset_intfc_components(fr->interf);
	debug_print("untrack","Left f_untrack_point()\n");
	return YES;
}		/*end f_untrack_point*/

EXPORT	void	f_reflect_curve2d(
	CURVE		*curve,/* curve being reflected */
	double		*p,	/* point on reflection plane */
	double		*n)	/* normal to plane */
{
	if (curve == NULL || curve->interface == NULL)
		return;
	if ((hs_bstate(curve) != NULL) && (boundary_state(curve) != NULL))
	{
	    double pt[MAXD];

	    pt[0] = 0.5*(Coords(curve->start->posn)[0] +
			 Coords(curve->end->posn)[0]);
	    pt[1] = 0.5*(Coords(curve->start->posn)[1] +
	    	         Coords(curve->end->posn)[1]);
	    reflect_state(boundary_state(curve),curve->interface,pt,p,n);
	}

	i_reflect_curve(curve,p,n);
}		/*end f_reflect_curve2d*/

EXPORT	void	f_reflect_node2d(
	NODE		*node,/* node being reflected */
	double		*p,	/* point on reflection plane */
	double		*n)	/* normal to plane */
{
	INTERFACE	*intfc = node->interface;
	CURVE		**c;
	double		*pt = Coords(node->posn);

	for (c = node->in_curves; c && *c; ++c)
	{
	    reflect_state(left_end_state(*c),intfc,pt,p,n);
	    reflect_state(right_end_state(*c),intfc,pt,p,n);
	}
	for (c = node->out_curves; c && *c; ++c)
	{
	    reflect_state(left_start_state(*c),intfc,pt,p,n);
	    reflect_state(right_start_state(*c),intfc,pt,p,n);
	}
	i_reflect_node(node,p,n);
}		/*end f_reflect_node2d*/

EXPORT  boolean f_is_subdomain_node(
	NODE		*node)
{
	return (node_type(node) == SUBDOMAIN_NODE) ? YES : NO;
}		/*end f_is_subdomain_node*/


EXPORT  boolean f_is_virtual_fixed_node(
	NODE		*node)
{
	CURVE		**c;
	boolean		subdomain = NO;

	if (!is_fixed_node(node))
	    return NO;
	if (!is_bdry(node))
	    return NO;
	for (c = node->in_curves; c && *c; ++c)
	{
	    if (!is_bdry(*c))
		return NO;
	    if (is_subdomain_boundary(Hyper_surf(*c)))
		subdomain = YES;
	}
	for (c = node->out_curves; c && *c; ++c)
	{
	    if (!is_bdry(*c))
		return NO;
	    if (is_subdomain_boundary(Hyper_surf(*c)))
		subdomain = YES;
	}
	return subdomain;
}		/*end f_is_virtual_fixed_node*/

/*
*			split_curves_at_cross():
*
*	Splits the two curves at a cross.
*	If the cross is on the first (last) bond of a curve
*	and has scaled distance less than min_sc_sep from the
*	node, the curve is not split but the node (if not a source
*	or sink) is moved to the cross point.
*	If one of the arrays ncomp1, pcomp1, ncomp2, pcomp2 are
*	non NULL the curves1[i] or curves2[i] is ft_assigned left
*	and right components ncomp1[i], pcomp1[i] or ncomp2[i], pcomp2[i]
*	respectively.
*/

EXPORT	void split_curves_at_cross(
	CROSS		*cr,
	Front		*fr,
	NODE		**newn,
	CURVE		**curves1,
	COMPONENT	*ncomp1,
	COMPONENT	*pcomp1,
	CURVE		**curves2,
	COMPONENT	*ncomp2,
	COMPONENT	*pcomp2,
	double		min_sc_sep,
	POINTER		rp)
{
	INTERFACE	*intfc = fr->interf;
	NODE		*newn1, *newn2;
	int		is_bdry;

	if (cr == NULL) return;
	debug_print("split_curves","Entered split_curves_at_cross\n");

	is_bdry = (is_bdry(cr->c1) || is_bdry(cr->c2)) ? YES : NO;

	if (split_bond_at_cross(cr,1,curves1,ncomp1,pcomp1,
				intfc,min_sc_sep,rp,&newn1))
	{
	    rcl_after_split(cr,cr->p,cr->b1,cr->c1,curves1);
	}

	cr->p = Point(Coords(cr->p));

	if (split_bond_at_cross(cr,2,curves2,ncomp2,pcomp2,intfc,
		min_sc_sep,rp,&newn2))
	{
	    if (curves1[0] == cr->c2)
		curves1[0] = curves2[1];
	    if (curves1[1] == cr->c2)
		curves1[1] = curves2[0];
	    rcl_after_split(cr,cr->p,cr->b2,cr->c2,curves2);
	}

	merge_and_delete_nodes(newn1,newn2);
	*newn = newn1;

	if (fr->identify_physical_node)
	    (*fr->identify_physical_node)(*newn);
	if (is_bdry)
	    set_is_bdry(*newn);
	else
	    set_not_bdry(*newn);
	debug_print("split_curves","Left split_curves_at_cross\n");
}		/*end split_curves_at_cross*/


EXPORT  boolean f_move_closed_loop_node(
	CURVE		*c,
	BOND		*b)
{
	NODE		*c_node;
	FT_POINT		*p, *oldn_p;
	size_t		sizest;
	boolean		status;

	if (c->start != c->end)
	    return NO;
	if ((b == c->first) || (b == NULL) || is_bdry(c->start))
	    return YES;               /* Why call this routine for nothing */
					/* Don't move boundary node */

	c_node = c->start;
	oldn_p = c_node->posn;
	status = i_move_closed_loop_node(c,b);
	if (!status)
	    return NO;

		/* Set new node states */

	p = c_node->posn;
	sizest = size_of_state(c->interface);
	if (sizest != 0)
	{
	    ft_assign(right_state(oldn_p),right_start_state(c),sizest);
	    ft_assign( left_state(oldn_p), left_start_state(c),sizest);

	    ft_assign(right_start_state(c),right_state(p),sizest);
	    ft_assign(  right_end_state(c),right_state(p),sizest);
	    ft_assign( left_start_state(c), left_state(p),sizest);
	    ft_assign(   left_end_state(c), left_state(p),sizest);
	}

	return status;
}		/*end f_move_closed_loop_node*/

EXPORT	CURVE *f_attach_curve_to_node(
	CURVE		*c1,
	FT_POINT		*p,
	BOND		*b,
	NODE		*n)
{
	INTERFACE	*intfc = c1->interface;
	size_t		sizest;
	CURVE		*c2;

	c2 = i_attach_curve_to_node(c1,p,b,n);

	if (c2 != NULL && (sizest = size_of_state(intfc)) != 0)
	{
	    ft_assign(  left_end_state(c2), left_end_state(c1),sizest);
	    ft_assign( right_end_state(c2),right_end_state(c1),sizest);
	    ft_assign( left_start_state(c2), left_state(p),sizest);
	    ft_assign(right_start_state(c2),right_state(p),sizest);
	    ft_assign(   left_end_state(c1), left_state(p),sizest);
	    ft_assign(  right_end_state(c1),right_state(p),sizest);
	    bstate_index(c2) = bstate_index(c1);
	}

	/* set wave_types, correspond_curve */

	if (!rst_cor_after_attach_curve_to_node(c1,c2))
	{
	    screen("ERROR in attach_curve_to_node(), "
	           "Unable to reset curve correspondence list\n");
	    clean_up(ERROR);
	}
	if (c2 != NULL)
	{
	    wave_type(c2) = wave_type(c1);
	}

	return c2;
}		/*end f_attach_curve_to_node*/

/*
*		split_bond_at_cross():
*
*	Attempts to replace a single curve at a cross point by
*	a pair of curves with a common node at the cross.
*	If the cross is very close to an existing node,  or
*	the curve is a closed curve with a closed node,
*	no split is performed,  instead the node is moved to
*	the cross.
*
*	Returns YES is split_curve() is called, NO otherwise.
*/

LOCAL	int split_bond_at_cross(
	CROSS		*cr,
	int		curve_to_split,
	CURVE		**curves,
	COMPONENT	*l_comp,
	COMPONENT	*r_comp,
	INTERFACE	*intfc,
	double		min_sc_sep,
	POINTER		rp,
	NODE		**newn)
{
	RECT_GRID	*gr = computational_grid(intfc);
	CURVE		*c;
	CURVE		**curs;
	BOND		*b;
	double		*h = gr->h;
	double		d_s, d_e;
	int		dim = intfc->dim;
	boolean		sav_interp;
	int		curves_split = NO;
	int		force_split = NO;

	sav_interp = interpolate_intfc_states(intfc);

	*newn = NULL;
	c = (curve_to_split == 1) ? cr->c1 : cr->c2;
	b = (curve_to_split == 1) ? cr->b1 : cr->b2;
	d_s = scaled_separation(cr->p,b->start,h,dim);
	d_e = scaled_separation(cr->p,b->end,h,dim);
	/* No need for this and cause problem for MOVABLE_BODY_BOUNDARY
	   XL LI, 5/7/2009
	if (wave_type(c) < FIRST_PHYSICS_WAVE_TYPE)
	    interpolate_intfc_states(intfc) = NO;
	else
	    interpolate_intfc_states(intfc) = YES;
	*/
	interpolate_intfc_states(intfc) = YES;

	if (b == c->first && d_s < min_sc_sep)
	{
	    if (merge_cross_with_node(c,POSITIVE_ORIENTATION,cr,
	    			      l_comp,r_comp,curves))
	    {
		*newn = c->start;
		return NO;
	    }
	    else
		force_split = YES;
	}
	if (b == c->last && d_e < min_sc_sep)
	{
	    if (merge_cross_with_node(c,NEGATIVE_ORIENTATION,cr,
				      l_comp,r_comp,curves))
	    {
		*newn = c->end;
		return NO;
	    }
	    else
		force_split = YES;
	}

	if (d_s < min_sc_sep && !force_split)
	{
	    Coords(b->start)[0] = Coords(cr->p)[0];
	    Coords(b->start)[1] = Coords(cr->p)[1];
	    set_bond_length(b,dim);
	    set_bond_length(b->prev,dim);
	    cr->p = b->start;
	    b = b->prev;
	    if (curve_to_split == 1)
		cr->b1 = b;
	    else
		cr->b2 = b;
	}
	else if (d_e < min_sc_sep && !force_split)
	{
	     Coords(b->end)[0] = Coords(cr->p)[0];
	     Coords(b->end)[1] = Coords(cr->p)[1];
	     set_bond_length(b,dim);
	     set_bond_length(b->next,dim);
	     cr->p = b->end;
	}
	else
	{
	    if (insert_point_in_bond(cr->p,b,c) != FUNCTION_SUCCEEDED)
	    {
	        screen("ERROR in split_bond_at_cross(), "
		       "insert_point_in_bond() failed\n");
	        clean_up(ERROR);
	    }
	    rcl_after_insert_point(cr,cr->p,b);
	}

	if (is_closed_node(c->start) &&
	    is_node_of_closed_curve_only(c->start))
	{
	    if (cr->p == b->start)
	    	(void) move_closed_loop_node(c,b);
	    else
	    	(void) move_closed_loop_node(c,b->next);
	    curves[0] = c;
	    curves[1] = c;
	    *newn = c->start;
	    node_type(c->start) = ERROR;
	}
	else
	{
	    boolean sav_scs;

	    set_copy_intfc_states(YES);
	    interpolate_intfc_states(intfc) = NO;
	    sav_scs = interpolate_states_at_split_curve_node();
	    set_interpolate_states_at_split_curve_node(NO);
	    curs = split_curve(cr->p,b,c,
			       l_comp?l_comp[0]:negative_component(c),
			       r_comp?r_comp[0]:positive_component(c),
			       l_comp?l_comp[1]:negative_component(c),
			       r_comp?r_comp[1]:positive_component(c));
	    set_interpolate_states_at_split_curve_node(sav_scs);
	    curves[0] = curs[0];
	    curves[1] = curs[1];
	    roclists_after_split((RPROBLEM *)rp,c,curs,YES);
	    curves_split = YES;
	    *newn = curs[0]->end;
	}
	interpolate_intfc_states(intfc) = sav_interp;
	return curves_split;
}		/*end split_bond_at_cross*/

LOCAL	int merge_cross_with_node(
	CURVE		*c,
	ORIENTATION	c_orient,
	CROSS		*cr,
	COMPONENT	*l_comp,
	COMPONENT	*r_comp,
	CURVE		**curves)
{
	NODE		*oldn, *n;
	INTERFACE	*intfc = c->interface;

	oldn = Node_of(c,c_orient);
	if (     is_bdry_like_node(oldn) 
	     && (wave_type(cr->c1) > FIRST_PHYSICS_WAVE_TYPE)
	     && (wave_type(cr->c2) > FIRST_PHYSICS_WAVE_TYPE)  )
	{
	    /* Don't move boundary nodes to the interior */

	    return NO;
	}
	n = node_of_point(cr->p,intfc);
	if (c_orient == POSITIVE_ORIENTATION)
	{
	    curves[0] = (is_closed_curve(c)) ? c : NULL;
	    curves[1] = c;
	    if (l_comp != NULL && r_comp != NULL)
	    {
	    	negative_component(curves[1]) = l_comp[1];
	    	positive_component(curves[1]) = r_comp[1];
	    }
	}
	else
	{
	    curves[0] = c;
	    curves[1] = (is_closed_curve(c)) ? c : NULL;
	    if (l_comp != NULL && r_comp != NULL)
	    {
	    	negative_component(curves[0]) = l_comp[0];
	    	positive_component(curves[0]) = r_comp[0];
	    }
	}
	if (is_stationary_node(oldn))
	{
	    /* Don't move stationary nodes */

	    Coords(cr->p)[0] = Coords(oldn->posn)[0];
	    Coords(cr->p)[1] = Coords(oldn->posn)[1];
	}
	else
	{
	    Coords(oldn->posn)[0] = Coords(cr->p)[0];
	    Coords(oldn->posn)[1] = Coords(cr->p)[1];
	    rbl_after_move_node(oldn);
	}
	if (n != NULL)
	    merge_and_delete_nodes(n,oldn);
	else 
	{
	    n = oldn;
	    cr->p = n->posn;
	}
	if (is_closed_node(n))
	    node_type(n) = ERROR;
	return YES;
}		/*end merge_cross_with_node*/



/*
*		intfc_delete_fold_back_bonds():
*
*	This routine deletes all bonds in an interface that 'fold back'
*	upon the previous bond causing a 'needle point' configuration in
*	a curve. This condition will currently prevail around sink nodes
*	until some form of local mesh refinement prevents the situation.
*	A tolerance of tol < .01 * min(hx,hy)) is employed.
*	NOTE: Care is taken to reset first_ and last_ n_ and t_
*		_propagate pointers.
*
*	Upgraded to handle fold backs at sinks in which two curves are
*	involved.
*
*	Returns YES sucessful NO otherwise.
*/

EXPORT	boolean intfc_delete_fold_back_bonds(
	Front		*fr)
{
	INTERFACE	*intfc = fr->interf;
	NODE		*ns, *ne;
	CURVE		**c;
	NODE		**n;
	int		n_type;
	int		c_fnd = NO, zl_c = NO, n_fnd = NO;
	double		min_sc_sep = MIN_SC_SEP(fr->interf);

	debug_print("fold_back","Entered intfc_delete_fold_back_bonds()\n");

redo_loop:
	for (c = intfc->curves;  *c;  ++c)
	{
		if (is_bdry_like_curve(*c) || omit_redistribution(*c) ||
		    ((*c)->num_points < 4) ||
		    ((node_type((*c)->start) == CLOSED_NODE) &&
		    	 ((*c)->num_points < 5)))
			continue;

		delete_fold_back_bonds(fr,*c,min_sc_sep,&c_fnd,&zl_c);

		if (zl_c == YES)
		{
		    if ((node_type((*c)->start) != SUBDOMAIN_NODE) &&
		        (node_type((*c)->end)   != SUBDOMAIN_NODE))
		    {
		    	if (debugging("fold_back"))
		    		(void) printf("deleting curve %llu\n",
		    			      curve_number(*c));

			ns = (*c)->start;	ne = (*c)->end;
			(void) delete_curve(*c);
			(void) delete_redundant_node(ns,NULL,NULL,fr);
			if (ne != ns)
				(void) delete_redundant_node(ne,NULL,NULL,fr);
			goto redo_loop;
		    }
		}
	}

	for (n = intfc->nodes;  n && *n;  ++n)
	{
		n_type = node_type(*n);
		if (n_type == SINK_NODE)
			resolve_sink_node_fold_backs(*n,&n_fnd,fr);
	}

	if (debugging("fold_back"))
	{
		(void) printf("\tc_fnd = %s n_fnd = %s\n",
			      (c_fnd == YES) ? "YES" : "NO",
			      (n_fnd == YES) ? "YES" : "NO");
	}
	debug_print("fold_back","Leaving intfc_delete_fold_back_bonds()\n");
	return YES;
}		/*end intfc_delete_fold_back_bonds*/

/*
*		f_delete_fold_back_bonds():
*
*	This routine deletes all bonds in a curve that 'fold back' upon
*	the previous bond causing a 'needle point' configuration in the
*	curve. This condition will currently prevail around sink nodes
*	until some form of local mesh refinement prevents the situation.
*	A tolerance of tol < .01 * min(hx,hy)) is employed.
*	NOTE: Care is taken to reset first_ and last_ n_ and t_
*		_propagate pointers.
*
*	Returns found == YES if any fold back is detected. Also flags
*	for the presence of resulting zero_length curves by zl_c == YES.
*/

EXPORT	void f_delete_fold_back_bonds(
	Front		*fr,
	CURVE		*c,
	double		min_sc_sep,
	int		*found,
	int		*zl_c)
{
	BOND		*b1, *b2;
	NODE		*ns;
	RECT_GRID	*gr = fr->rect_grid;
	double		minus_cos_min_ang, min_ang, sp, tol;
	double		*h = gr->h;
	int		dim = gr->dim;
	int		deb_fb = NO;

	if (debugging("fold_back"))
	{
	    deb_fb = YES;
	    (void) printf("Testing curve %llu for fold_backs\n",curve_number(c));
	    print_curve(c);
	}

	min_ang = 0.1 * min(fabs(angle(h[0],h[1])),fabs(angle(h[1],h[0])));
	minus_cos_min_ang = -cos(min_ang);

	*zl_c = NO;
redo_curve:
	for (b1 = c->first, b2 = b1->next;  b2;  b1 = b2, b2 = b2->next)
	{
	    sp = scalar_product_on_bonds(b1,b2,c->interface->dim);
	    tol = bond_length(b1) * bond_length(b2) * minus_cos_min_ang;
	    if (sp <= tol)
	    {
	    	*found = YES;
	    	if (deb_fb)
	    	{
	    	    (void) printf("found needle on curve %llu\n",
	    		          curve_number(c));
	    	    (void) printf("b1->prev - ");
	    	    print_bond(b1->prev);
	    	    (void) printf("b1 - ");
	    	    print_bond(b1);
	    	    (void) printf("b2 - ");
	    	    print_bond(b2);
	    	}

		/* check if result yields zero length curve */

		/*******
		if ((b1->prev == NULL) && (b2->next == NULL))
		{
		    *zl_c = YES;
		    return;
		}
		*******/

		if (scaled_separation(b1->start,b2->end,h,dim) > min_sc_sep)
		{
		    (void) delete_start_of_bond(b2,c);
		    goto redo_curve;
		}
		else if (b1->prev != NULL || node_type(c->start) == CLOSED_NODE)
		{
		    if (b1->prev == NULL)
			(void) move_closed_loop_node(c,c->last);

		    (void) delete_start_of_bond(b2,c);
		    (void) delete_start_of_bond(b1,c);
			goto redo_curve;
		}
		else if (b1->prev == NULL)
		{
		    (void) delete_start_of_bond(b2->next,c);
		    (void) delete_start_of_bond(b2,c);
		    goto redo_curve;
		}
	    }
	}

			/* Check fold back around closed_curve node */

    redo_ccur:
	ns = c->start;
	if (is_node_of_closed_curve_only(ns))
	{
	    if (deb_fb)
	    	(void) printf("checking at node of closed curve\n");
	    b1 = c->last;		b2 = c->first;

	    sp = scalar_product_on_bonds(b1,b2,c->interface->dim);
	    tol = bond_length(b1) * bond_length(b2) * minus_cos_min_ang;
	    if (deb_fb) (void) printf("\tsp %g tol %g\n",sp,tol);
	    if (sp <= tol)
	    {
	    	*found = YES;
	    	if (deb_fb)
	    	{
	    	    (void) printf("found needle ");
	    	    (void) printf("at node %llu of curve %llu\n",
	    	    	      node_number(c->start),curve_number(c));
	    	    print_bond(b1->prev);
	    	    print_bond(b1);
	    	    print_bond(b2);
	    	    print_bond(b2->next);
	    	}
		if ((b2->next == b1) || (b2->next == b1->prev))
		{
		    *zl_c = YES;
		    return;
		}
		if (is_source_sink_node((c)->start))
		{
		    (void) delete_start_of_bond(b2->next,c);
		    (void) delete_start_of_bond(b1,c);
		}
		else
		{
		    (void) move_closed_loop_node(c,b1->prev);
		    if (scaled_separation(b1->start,b2->end,h,dim) > min_sc_sep)
		    {
			if (b2 == c->first)
			    (void) move_closed_loop_node(c,c->last);
			(void) delete_start_of_bond(b2,c);
		    }
		    else
		    {
			if (b2 == c->first)
			    (void) move_closed_loop_node(c,c->last);
		    	(void) delete_start_of_bond(b2,c);
			if (b1 == c->first)
			    (void) move_closed_loop_node(c,c->last);
		    	(void) delete_start_of_bond(b1,c);
		    }
		}
		goto redo_ccur;
	    }
	}
}		/*end f_delete_fold_back_bonds*/


/*
*			resolve_sink_node_fold_backs():
*
*	Resolve sharp fold_backs at sinks due to ends of two curves
*	meeting at a sink node.
*/

LOCAL	void resolve_sink_node_fold_backs(
	NODE		*n,
	int		*found,
	Front		*fr)
{
	O_NODE		*on;
	CURVE		*cin, *cout, *newc;
	NODE		*tmpn;
	FT_POINT		*p, *p1, *p2;
	double		hx = fr->rect_grid->h[0], hy = fr->rect_grid->h[1];
	double		min_ang;
	double		mid_coords[MAXD];
	int		i, j;
	boolean		sav_interp;
	int		deb_rsnfb = NO;

	if (debugging("fold_back")) deb_rsnfb = YES;

	min_ang = 0.2 * min(fabs(angle(hx,hy)),fabs(angle(hy,hx)));

	on = make_onode(n);
	if (on->num_c < 1) return;

	if (deb_rsnfb)
	{
		(void) printf("resolving fold_backs at sink node %llu\n",
		       node_number(n));
		print_onode(on);
		(void) printf("angle(hx,hy) %g angle(hy,hx) %g min_ang %g\n",
					angle(hx,hy),angle(hy,hx),min_ang);
	}

	for (i = 0;  i < on->num_c;  ++i)
	{
	    j = (i+1) % on->num_c;
	    if (fabs(on->ang[j] - on->ang[i]) < min_ang)
	    {
		*found = YES;

			/* May be closed curve !! */
			/* 	   detach	  */
		if (on->nc[i] == on->nc[j])
		{
			cin = cout = on->nc[i];
			if (deb_rsnfb)
				(void) printf("fold back curve %llu is closed\n",
					      curve_number(cin));
		}
		else if (on->orient[i] == NEGATIVE_ORIENTATION)
		{
			cin  = on->nc[i];
			cout = on->nc[j];
			if (on->orient[j] != POSITIVE_ORIENTATION)
				invert_curve(cout);
		}
		else if (on->orient[j] == NEGATIVE_ORIENTATION)
		{
			cin  = on->nc[j];
			cout = on->nc[i];
			if (on->orient[i] != POSITIVE_ORIENTATION)
				invert_curve(cout);
		}
		else
		{
			cin = on->nc[i];
			invert_curve(cin);
			cout = on->nc[j];
		}

		if (deb_rsnfb)
		{
		    (void) printf("fold back for curves -\n");
		    if (cin == cout)
		    {
			(void) printf("cin = cout -\n");
			print_curve(cin);
		    }
		    else
		    {
			(void) printf("cin  -\n");	print_curve(cin);
			(void) printf("cout -\n");	print_curve(cout);
		    }
		}

		p1 = cin->last->start;
		p2 = cout->first->end;
		mid_coords[0] = .5 * (Coords(p1)[0] + Coords(p2)[0]);
		mid_coords[1] = .5 * (Coords(p1)[1] + Coords(p2)[1]);
		p = Point(mid_coords);
		tmpn = make_node(p);

			/* For the case cin == cout, resulting in a closed */
			/* curve which will be pulled away from the SINK   */
			/* The following operations delete two points from */
			/* the curve. Therefore if the curve originally    */
			/* has 4 points (the node point is double counted  */
			/* the following operation results in a curve of   */
			/* zero length and must be deleted. See below.     */

		if (cin->last->prev)
			(void)delete_point_adjacent_to_node(fr,cin,
						      NEGATIVE_ORIENTATION);
		if (cout->first->next) 
			(void)delete_point_adjacent_to_node(fr,cout,
						      POSITIVE_ORIENTATION);
		change_node_of_curve(cin,NEGATIVE_ORIENTATION,tmpn);
		change_node_of_curve(cout,POSITIVE_ORIENTATION,tmpn);

		if (deb_rsnfb)
		{
		    (void) printf("after load tmpn and delete_pt_adj\n");
		    (void) printf("n    - ");		print_node(n);
		    (void) printf("tmpn - ");		print_node(tmpn);
		    (void) printf("cin = cout -\n");	print_curve(cin);
		}

		if (cin == cout)
		{
			if (deb_rsnfb)
			{
			    (void) printf("Node before identification\n");
			    print_node(tmpn);
			}
			node_type(tmpn) = CLOSED_NODE;
			set_not_bdry(tmpn);
			if (deb_rsnfb)
			{
			    (void) printf("Node after identification\n");
			    print_node(tmpn);
			}
			if (cin->num_points == 2)
			{
			    (void) delete_curve(cin);
			    (void) delete_node(tmpn);
			    if (deb_rsnfb)
			    {
				(void) printf("2 points left on resultant ");
				(void) printf("closed curve. Deleting\n");
			    }
			}
		}
		else if ((negative_component(cin)  == negative_component(cout))
						&&
			 (positive_component(cin) == positive_component(cout)))
		{
			if (deb_rsnfb) (void) printf("Joining curves\n");

			sav_interp = interpolate_intfc_states(cin->interface);
			interpolate_intfc_states(cin->interface) = YES;
			newc = join_curves(cin,cout,negative_component(cin),
					   positive_component(cin),NULL);
			interpolate_intfc_states(newc->interface) = sav_interp;
			if (deb_rsnfb)
			{
				(void) printf("After join_curves, newc -\n");
				print_curve(newc);
			}
			(void) delete_redundant_node(tmpn,NULL,NULL,fr);

			if (deb_rsnfb)
				(void) printf("After delete_redundant_node\n");
		}
		else
		{
			CURVE	*c3;
			O_NODE	*otmpn;
			int	k, l;

			if (deb_rsnfb)
				(void) printf("making triple node of tmpn\n");

			c3 = make_curve(positive_component(cin),
					positive_component(cout),n,tmpn);
			otmpn = make_onode(tmpn);
			for (k = 0;  k < otmpn->num_c;  ++k)
			{
				if (otmpn->nc[k] == c3) break;
			}
			l = (k+1) % otmpn->num_c;
			if (otmpn->nc[l] == cin)
			{
			    positive_component(c3) = negative_component(cin);
			    negative_component(c3)  = negative_component(cout);
			    ft_assign(right_end_state(c3),left_end_state(cin),
						size_of_state(c3->interface));
			    ft_assign(right_start_state(c3),left_end_state(cin),
						size_of_state(c3->interface));
			    ft_assign(left_end_state(c3),left_start_state(cin),
						size_of_state(c3->interface));
			    ft_assign(left_start_state(c3),left_start_state(cin),
						size_of_state(c3->interface));
			}
			else
			{
			    ft_assign(left_end_state(c3),right_end_state(cin),
						size_of_state(c3->interface));
			    ft_assign(left_start_state(c3),right_end_state(cin),
						size_of_state(c3->interface));
			    ft_assign(right_end_state(c3),right_start_state(cin),
						size_of_state(c3->interface));
			    ft_assign(right_start_state(c3),right_start_state(cin),
						size_of_state(c3->interface));
			}

			if (deb_rsnfb)
			{
			    (void) printf("After make triple node of tmpn\n");
			    (void) printf("cin  -\n");	print_curve(cin);
			    (void) printf("cout -\n");	print_curve(cout);
			    (void) printf("c3 -\n");	print_curve(c3);
			}
		}
		break;
	    }
	}
}		/*end resolve_sink_fold_backs*/


/* 
*			f_delete_small_loops():
*
*	Deletes closed curves that consist of two and three bond loops, or
*	have smaller perimeter than two mesh blocks.
*	Deletes closed curves terminating on SINK_NODES that are smaller
*	in perimeter than 0.5 * the perimeter of a mesh square.
*/

EXPORT	void f_delete_small_loops(
	Front		*fr)
{
	CURVE		**c;
	INTERFACE	*intfc = fr->interf;
	NODE		*m;
	double		perim_tol;
	double		*h = computational_grid(intfc)->h;

	perim_tol = h[0] + h[1];

label_2:
	for (c = intfc->curves;  c && *c;  ++c)
	{
	    if (!is_closed_curve(*c)) continue;
	    /*if (wave_type(*c) < FIRST_PHYSICS_WAVE_TYPE) continue; */

	    if ((node_type((*c)->start) == SINK_NODE) &&
	        ((*c)->num_points < 10) &&	/*TOLERANCE*/
	        (curve_length(*c) < perim_tol))
	    {
	    	if (debugging("small_loops"))
	    	    (void) printf("Deleting small loop %llu\n",curve_number(*c));
		(void) delete_curve(*c);
		goto label_2;
	    }
	    if (((*c)->num_points < 5) ||
		    (curve_length(*c) < 4.0*perim_tol)) /*TOLERANCE*/
	    {
	    	if (debugging("small_loops"))
	    	    (void) printf("Deleting small loop %llu\n",curve_number(*c));
	    	m = (*c)->start;
	    	(void) delete_curve(*c);
	    	if (node_type(m) == CLOSED_NODE) (void) delete_node(m);
	    	goto label_2;
	    }
	}
}		/*end f_delete_small_loops*/


/*
*		intfc_delete_very_short_bonds():
*
*	This routine deletes all bonds of very short length 
*	(scaled_bond_length(b,hx,hy) < min_sc_sep)
*	which may have been formed by the unravel routine.
*	NOTE: Care is taken to reset first_ and last_ n_ and t_
*		_propagate pointers.
*/

struct _CLIST {
	CURVE *curve;
	struct _CLIST *next;
};

typedef struct _CLIST CLIST;

EXPORT	void intfc_delete_very_short_bonds(
	Front		*front)
{
	NODE		*ns, *ne;
	BOND		*b;
	CURVE		**c;
	CLIST		delete_curve_list;
	CLIST		*cl;
	INTERFACE	*intfc = front->interf;
	double		space = Front_spacing(front,GENERAL_WAVE);
	double		*h = computational_grid(intfc)->h;
	double		sl;
	double		min_sc_sep = MIN_SC_SEP(intfc);
	int		dim = intfc->dim;

	debug_print("delete_short_bonds",
	      "Entered intfc_delete_very_short_bonds()\n");
	if (debugging("delete_short_bonds"))
	{
	    (void) printf("hx %g hy %g min_sc_sep %g\n",h[0],h[1],min_sc_sep);
	    (void) printf("Interface into intfc_delete_very_short_bonds()\n");
	    print_interface(intfc);
	}
	cl = &delete_curve_list;
	cl->curve = NULL;
	cl->next = NULL;
	for (c = intfc->curves; c && *c; ++c)
	{
	    if (omit_redistribution(*c))
		continue;

	    /*
	     * For short, non-boundary curves having few points, delete
	     * only bonds which actually have zero length.  This operation
	     * should not change the interface in any significant way, and
	     * provides the benefit of a trivial untangle.
	     */

	    if ((!is_bdry_like_curve(*c)) && ((*c)->num_points <= 5))
	    {
	    	if (!is_closed_curve(*c))
	    	    MIN_SC_SEP(intfc) = 0.0;
	    	sl = 0.0;
	    	for (b = (*c)->first; b != NULL;  b = b->next)
	    	    sl += scaled_bond_length(b,h,dim);
	    	if (sl < 1.5 * space * ((*c)->num_points - 1))
	    	    MIN_SC_SEP(intfc) = 0.0;
	    }

	    /* if zero length curve, store in cl for deletion */

	    if ((*c)->num_points == 2)
	    {
	    	if (scaled_bond_length((*c)->first,h,dim) < min_sc_sep)
	    	{
	    	    cl->next = (CLIST *) Store(sizeof(CLIST));
	    	    cl = cl->next;
	    	    cl->curve = *c;
	    	    cl->next = NULL;
	    	    MIN_SC_SEP(intfc) = min_sc_sep;
	    	    continue;
	    	}
	    }

			/* delete short bonds on curve */
	    curve_delete_very_short_bonds(*c);
	    MIN_SC_SEP(intfc) = min_sc_sep;
	}

	/* Delete the isolated zero length curves flagged in cl */

	for (cl = delete_curve_list.next; cl != NULL; cl = cl->next)
	{
	    ns = cl->curve->start;
	    ne = cl->curve->end;

	    if (ns == ne)
	    {
	    	if (!is_node_of_closed_curve_only(ns))
	    	    continue;
	    }
	    else if ((ns->in_curves != NULL)  ||  (ns->out_curves[1] != NULL) ||
		     (ne->out_curves != NULL) || (ne->in_curves[1] != NULL))
		    continue;

	    if (debugging("delete_short_bonds"))
	    {
	    	(void) printf("deleting curve:\n");
	    	print_curve(cl->curve);
	    }
	    (void) delete_curve(cl->curve);
	    if (!is_source_sink_node(ns))
		(void) delete_node(ns);
	    if (!is_source_sink_node(ne))
		(void) delete_node(ne);

	    /* Don't free cl storage as it is allocated by store */
	}
	if (debugging("delete_short_bonds"))
	{
	    (void) printf("Interface after intfc_delete_very_short_bonds()\n");
	    print_interface(intfc);
	}
	debug_print("delete_short_bonds",
	      "Leaving intfc_delete_very_short_bonds()\n");
}		/*end intfc_delete_very_short_bonds*/


EXPORT	void curve_delete_very_short_bonds(
	CURVE		*c)
{
	BOND		*b;
	RECT_GRID	*gr = computational_grid(c->interface);
	double		*h = gr->h;
	double		min_sc_sep = MIN_SC_SEP(c->interface);
	int		dim = gr->dim;

	/* Note: it may be necessary to allow this function to return a
	 * status to indicate failure. */

redo:
	for (b = c->first; b != NULL; b = b->next)
	{
	    if (scaled_bond_length(b,h,dim) < min_sc_sep)
	    {
		if (b->prev == NULL)
		{
		    if (b->next == NULL)
			return;
		    if (cross_rect_grid_bdry(b,gr) &&
		    	cross_rect_grid_bdry(b->next,gr)) continue;
		    if (delete_start_of_bond(b->next,c)==FUNCTION_FAILED)
		    {
			(void) printf("WARNING in "
				      "curve_delete_very_short_bonds(), "
				      "delete_start_of_bond() failed "
				      "at point adjacent to node\n");
			continue;
		    }

		    goto redo;	/* may be multiple short bonds in a row */
		}
		else
		{
		    if (cross_rect_grid_bdry(b->prev,gr) &&
		    	cross_rect_grid_bdry(b,gr)) 
			continue;
		    if (delete_start_of_bond(b,c) == FUNCTION_FAILED)
		    {
			(void) printf("WARNING in "
				      "curve_delete_very_short_bonds(), "
				      "delete_start_of_bond() failed\n");
			continue;
		    }
		}
	    }
	}
}		/*end curve_delete_very_short_bonds*/

/*
*			is_stationary_node():
*
*	Returns YES if the position of the node is fixed.
*/

EXPORT	boolean is_stationary_node(
	NODE	*node)
{
	if (is_source_sink_node(node) ||
	    is_fixed_node(node) ||
	    is_passive_node(node))
	    return YES;

	return NO;
}		/*end is_stationary_node*/


/*
*			f_area_of_loop():
*
*	Calculates the area of the polygon formed by (the generalized extensions
*	of) curves c1 and c2 (from a common node).  If c1 is closed (as a 
*	generalized  curve), c2 = NULL, or c2 = c1, only the area of
*	the polygon formed by (the extension of) c1 is calculated.
*
*	Returns -1.0 if c1 = NULL, c1 and c2 (the actual curves) do not have a 
*	common node, or if the curves do not form a closed polygon.
*
*	NOTE: The area returned will be incorrect (too small) if the 
*	curves intersect.  For example, fabs(A1-A2) will be returned
*	in the case below:
*		         |-->--\ /--<--|
*                        ^  A1  x  A2  ^
*	                 |--<--/ \-->--|
*/

EXPORT	double f_area_of_loop(
	CURVE		*c1,
	ORIENTATION	orient1, /* orient of c1 relative to a
				    common node of c1,c2*/
	CURVE		*c2)

{
	double		area,area1,area2;
	CURVE		*nextc,*c;
	NODE		*endn1,*endn2,*common_n;
	int		n_type;
	ORIENTATION	orient2;
	BOND		*b;
	int		dbg = debugging("f_area_of_loop");

	if (dbg)
	   (void) printf("Entered f_area_of_loop - curves %llu %llu orient1 %d\n",
			 curve_number(c1),curve_number(c2),orient1);
	area = -1.0;
	if (! c1)
	    goto Exit;
	common_n = Node_of(c1,orient1);

		/* area for part of loop subtended by first curve */

	c = c1;
	nextc = NULL;
	n_type = UNKNOWN_NODE_TYPE;
	area1 = 0.0;
	while (nextc != c1)  	/* until gen curve c1 closes */
	{
	    for (b = c->first; b; b = b->next)
	    {
	    	double cp;

	    	(void) vector_product_on_points(Coords(common_n->posn),
	    		                        Coords(b->end),Coords(b->start),
						2,&cp);
	    	area1 += cp;
	    }

	    nextc = next_curve_of_gen_curve(c,orient1,&n_type,&endn1);
	    if (! nextc) /* end of general curve */
		break;
	    c = nextc;
	}
	area1 = 0.5*area1;
	if (dbg)
	   (void) printf("Area of loop subtended by curves %llu thru %llu = %g\n",
			 curve_number(c1),curve_number(c),area1);

		/* should c2 be ignored? */

	if (common_n == endn1) /* c1 is closed */
	{
	    area = fabs(area1);
	    goto Exit;
	}
	if (c2 == NULL || c2 == c1)
	    goto Exit;

		/* determine orientation of c2 relative to common_n */
	
	if (c2->start == common_n) 
	    orient2 = POSITIVE_ORIENTATION;
	else if (c2->end == common_n)
	    orient2 = NEGATIVE_ORIENTATION;
	else
	{
	    (void) printf("WARNING in f_area_of_loop(), "
			  "curves %llu %llu do not have common node ",
	    	          curve_number(c1),curve_number(c2));
	    (void) printf("in f_area_of_loop()\n");
	    goto Exit;
	}
		
		/* area for part of loop subtended by second curve */

	c = c2;
	nextc = NULL;
	n_type = UNKNOWN_NODE_TYPE;
	area2 = 0.0;
	while (nextc != c2)  	/* until gen curve c1 closes */
	{
	    for (b = c->first; b; b = b->next)
	    {
	    	double cp;
			 
	    	(void) vector_product_on_points(Coords(common_n->posn),
				                Coords(b->end),Coords(b->start),
						2,&cp);
		area2 += cp;
	    }

	    nextc = next_curve_of_gen_curve(c,orient2,&n_type,&endn2);
	    if (! nextc) /* end of general curve */
		break;
	    c = nextc;
	}
	area2 = 0.5*area2;
	if (dbg)
	   (void) printf("Area of loop subtended by curves %llu thru %llu = %g\n",
			 curve_number(c2),curve_number(c),area2);

		/* check for closed polygon */
	
	if (endn1 != endn2)
	    goto Exit;

		/* make signed areas consistent */

	if (orient1 != orient2)
	    area = fabs(area1 + area2);
	else 
	    area = fabs(area1 - area2);

Exit:
	if (area < 0.0)
	    (void) printf("ERROR: polygon not closed in f_area_of_loop\n");
	if (dbg)
	    (void) printf("Leaving f_area_of_loop - area %g\n",area);
	return area;
}		/*end f_area_of_loop*/

/*
*        	       find_loop():
*
*	Given a curve and orientation and an angle direction,  this function
*	identifies the generalized closed loop containing that curve.
*	Briefly,  starting at Node_of(c,c_orient) this function shifts
*	to the opposite node of c and finds the adjacent curve to c
*	in the given angle direction.  This process is continued until
*	the starting node is reached again.
*
*	NOTE:  This function allocates an O_CURVE_FAMILY structure
*	and this storage is to be freed by the user when the data
*	is no longer needed.
*/

EXPORT O_CURVE_FAMILY *find_loop(
	CURVE		*c,
	ORIENTATION	c_orient,
	ANGLE_DIRECTION	ang_dir)
{
	NODE		*ns = Node_of(c,c_orient);
	O_CURVE_FAMILY	*loop;
	O_CURVE		*oc;

	debug_print("find_loop","Entered find_loop()\n");
	init_cfamily(&loop,c,c_orient);
	while (Node_of(c,Opposite_orient(c_orient)) != ns)
	{
		c = adjacent_curve(c,Opposite_orient(c_orient),
				   ang_dir,&c_orient);
		if (c != NULL)
		{
			init_o_curve(&oc,c,c_orient);
			add_oc_curve_to_family(oc,&loop);
		}
		else
		{
			/* We have reached a node with only one curve, 
			 * assumedly a temporary condition due to dynamic
			 * untracking.  This curve should be included
			 * twice, once with each orientation, as if it
			 * were traversed once on each side in the loop.
			 *
			 *            you are here
			 *                 x 
			 *              ^  |  |
			 *              |  |  |
			 *              |  |  |
			 *       --->   |  |  v   --->
			 *      ------------------------
			 */

			c = loop->last->curve;
			c_orient = Opposite_orient(loop->last->orient);
			init_o_curve(&oc,c,c_orient);
			add_oc_curve_to_family(oc,&loop);

			/* If we started on the problem curve (ns is at the
			 * base of the vertical curve in the picture), then
			 * we need to increment c to be the next curve
			 * (on the right in the picture), or else the
			 * while loop will halt prematurely.
			 */

			if ((Node_of(c,Opposite_orient(c_orient)) == ns) &&
			    ((c = adjacent_curve(c,Opposite_orient(c_orient),
						 ang_dir,&c_orient)) == NULL))
			{
				/* We started on the problem curve, and there
				 * is no adjacent curve at either end, which
				 * is an impossible geometry. This function
				 * should not have been called in this case.
				 */

				screen("ERROR in find_loop(), ");
				screen("inconsistent configuration found\n");
				clean_up(ERROR);
			}
		}
	}                                         

	debug_print("find_loop","Leaving find_loop()\n");
	return loop;
}		/*end find_loop*/

/*
*			f_delete_point_adjacent_to_node():
*
*	Deletes the first non-node point on curve c away from the node n.
*
*	Returns NO on error, YES on success.
*/

/*ARGSUSED*/
EXPORT	boolean f_delete_point_adjacent_to_node(
	Front		*fr,
	CURVE		*c,
	ORIENTATION	orient)
{
	i_delete_point_adjacent_to_node(c,orient);
}		/*end f_delete_point_adjacent_to_node*/


EXPORT void delete_interior_points_of_curve(
	Front		*fr,
	CURVE		*curve)
{
	if (curve == NULL) return;
	while (curve->first != curve->last)
		(void) delete_point_adjacent_to_node(fr,curve,
						     POSITIVE_ORIENTATION);
}		/*end delete_interior_points_of_curve*/


EXPORT	void	f_reflect_surface(
	SURFACE		*surface,/* surface being reflected */
	double		*p,	/* point on reflection plane */
	double		*n)	/* normal to plane */
{
	i_reflect_surface(surface,p,n);
}		/*end f_reflect_surface*/

/*
*			f_untrack_surface():
*
*	Only a partial implementation,  assumes no non-trivial curves on the
*	surface	to be untracked.  Also deletes all nodes and curves that were
*	only connected to the delete surface.
*/

EXPORT	boolean f_untrack_surface(
	SURFACE   *s,
	COMPONENT newcomp,
	Front     *fr)
{
	SURFACE   **ss;
	CURVE     **c;
	CURVE     **curves;
	NODE      *ns, *ne;
	COMPONENT pcomp, ncomp;
	INTERFACE *intfc = s->interface;
	boolean   PisFSR, NisFSR;

	pcomp = positive_component(s);
	ncomp = negative_component(s);
	PisFSR = ComponentIsFlowSpecified(pcomp,fr);
	NisFSR = ComponentIsFlowSpecified(ncomp,fr);
	if ((NisFSR == YES) && (PisFSR == NO))
	    SetActiveFlowComponent(ncomp,fr);
	if ((NisFSR == NO) && (PisFSR == YES))
	    SetActiveFlowComponent(pcomp,fr);
	if (equivalent_comps(pcomp,ncomp,intfc) == NO)
	    set_equivalent_comps(pcomp,ncomp,intfc);
	if (equivalent_comps(pcomp,newcomp,intfc) == NO)
	    set_equivalent_comps(pcomp,newcomp,intfc);
	if (equivalent_comps(ncomp,newcomp,intfc) == NO)
	    set_equivalent_comps(ncomp,newcomp,intfc);

	curves = NULL;
	for (c = s->pos_curves; c && *c; ++c)
	{
	    if (unique_add_to_pointers(*c,&curves) != FUNCTION_SUCCEEDED)
	    {
		screen("ERROR in f_untrack_surface(), unique_add_to_pointers "
		       "failed\n");
		clean_up(ERROR);
	    }
	}
	for (c = s->neg_curves; c && *c; ++c)
	{
	    if (unique_add_to_pointers(*c,&curves) != FUNCTION_SUCCEEDED)
	    {
	        screen("ERROR in f_untrack_surface(), unique_add_to_pointers "
	    	       "failed\n");
	        clean_up(ERROR);
	    }
	}
	
	(void) delete_surface(s);
	for (ss = intfc->surfaces; ss && *ss; ++ss)
	{
	    if (positive_component(*ss) == pcomp)
		positive_component(*ss) = newcomp;
	    if (negative_component(*ss) == pcomp)
		negative_component(*ss) = newcomp;
	    if (positive_component(*ss) == ncomp)
		positive_component(*ss) = newcomp;
	    if (negative_component(*ss) == ncomp)
		negative_component(*ss) = newcomp;
	}

	for (c = curves; c && *c; ++c)
	{
	    if (((*c)->pos_surfaces == NULL) && ((*c)->neg_surfaces == NULL))
	    {
		ns = (*c)->start;
		ne = (*c)->end;
		(void) delete_curve(*c);
		if ((ns->in_curves == NULL) && (ns->out_curves == NULL))
		    (void) delete_node(ns);
		if ((ne != ns) &&
		    (ne->in_curves == NULL) && (ne->out_curves == NULL))
		    (void) delete_node(ne);
		    
	    }
	}
	return YES;
}		/*end f_untrack_surface*/

EXPORT   boolean  cross_rect_grid_bdry(
        BOND *b,
        RECT_GRID *rgr)
{
        double *L = rgr->L;         
	double *U = rgr->U;
        int i,dim = rgr->dim;
        FT_POINT *ps = b->start;
        FT_POINT *pe = b->end;

        for (i = 0; i < dim; ++i)
        {
            if (Coords(ps)[i] < L[i] && Coords(pe)[i] > L[i])
                return YES;
            if (Coords(ps)[i] > L[i] && Coords(pe)[i] < L[i])
                return YES;
            if (Coords(ps)[i] < U[i] && Coords(pe)[i] > U[i])
                return YES;
            if (Coords(ps)[i] > U[i] && Coords(pe)[i] < U[i])
                return YES;
        }
        return NO;
}       /* end cross_rect_grid_bdry */

#include <plotdecs.h>

EXPORT	void f_fset_hyper_surf_color(
	FILE       *file,
	HYPER_SURF *hs)
{
	if (wave_type(hs) < FIRST_PHYSICS_WAVE_TYPE)
		fset_color_from_table(file,1);
	else if (wave_type(hs) < FIRST_VECTOR_PHYSICS_WAVE_TYPE)
		fset_color_from_table(file,7);	
}		/*end f_fset_hyper_surf_color*/
