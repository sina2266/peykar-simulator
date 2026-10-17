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
*				fbdry3.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	This file contains routines to determine, impose or untangle
*	front boundaries and boundary conditions. Specifically:
*			curve_exits_parallel_to_bdry()
*/


#include <front/fdecs.h>

	/* LOCAL function prototypes */
LOCAL	boolean	correct_exterior_curve(CURVE*,Front*);
LOCAL	void	find_bdry_curves_at_node(NODE*,CURVE**,ORIENTATION*,
					CURVE**,ORIENTATION*);
LOCAL	boolean 	remove_ext_curve_at_single_bdry(Front*,CURVE*,CURVE*,
					NODE*,NODE*);
LOCAL	boolean 	remove_ext_curve_at_corner(Front*,CURVE*,CURVE*,ORIENTATION,
					CURVE*,ORIENTATION,NODE*,NODE*);

/*
*		find_bdry_curves_at_node():
*/

LOCAL	void find_bdry_curves_at_node(
	NODE		*node,
	CURVE		**bc1,
	ORIENTATION	*bc1_orient,
	CURVE		**bc2,
	ORIENTATION	*bc2_orient)
{
	CURVE		**c;

	debug_print("2drp","Entered find_bdry_curves_at_node()\n");
	*bc1 = *bc2 = NULL;
	for (c = node->in_curves; c && *c; c++) 
	{
		if (wave_type(*c) >= FIRST_PHYSICS_WAVE_TYPE
		 || wave_type(*c) == PASSIVE_BOUNDARY)
			continue;
		else if (!*bc1) 
		{
			*bc1 = *c;
			*bc1_orient = NEGATIVE_ORIENTATION;
		}
		else if (!*bc2) 
		{
			*bc2 = *c;
			*bc2_orient = NEGATIVE_ORIENTATION;
		}
	}
	for (c = node->out_curves; c && *c; c++) 
	{
		if (wave_type(*c) >= FIRST_PHYSICS_WAVE_TYPE
		 || wave_type(*c) == PASSIVE_BOUNDARY)
			continue;
		else if (!*bc1) 
		{
			*bc1 = *c;
			*bc1_orient = POSITIVE_ORIENTATION;
		}
		else if (!*bc2) 
		{
			*bc2 = *c;
			*bc2_orient = POSITIVE_ORIENTATION;
		}
	}
	debug_print("2drp","Left find_bdry_curves_at_node()\n");
}		/*end find_bdry_curves_at_node*/

/*
*			correct_for_exterior_curves():
*
*	This routine searches for curves of the following form which
*	may become "inverted through the boundary".
*
*	     -------->---------
*	    /		       \
*	---*--------------------*---   ->  ---*----------------------*---  
*					       \                    /
*					        ---------->---------
*
*	This function assumes that fr->interf is untangled.
*/


EXPORT	boolean correct_for_exterior_curves(
	Front		*fr)
{
	INTERFACE	*intfc = fr->interf;
	RECT_GRID	*rgr   = fr->rect_grid;

	COMPONENT   extcomp;
	CURVE	    **cc, *c;
	NODE	    *ns, *ne;
	BOND	    *b;
	FT_POINT	    *p1;
	boolean	    exterior_points, interior_points;
	int	    i, dim = rgr->dim;
	double	    lo[MAXD], hi[MAXD], tol;
	static const double EXT_TOLFAC = 0.0001;	/* TOLERANCE */

	debug_print("c_ext","Entered correct_for_exterior_curves()\n");

	for (i = 0; i < dim; i++)
	{
	    tol = EXT_TOLFAC * rgr->h[i];
	    lo[i]  = (rgr->lbuf[i] == 0) ? rgr->L[i] - tol : -HUGE_VAL;
	    hi[i]  = (rgr->ubuf[i] == 0) ? rgr->U[i] + tol : HUGE_VAL;
	}


	extcomp = exterior_component(intfc);
	if( debugging("c_ext") )
	{
	    (void) printf("correct_for_exterior_curves(): extcomp %d\n",
	    	      extcomp);
	    print_general_vector("lo = ",lo,dim,"\n");
	    print_general_vector("hi = ",hi,dim,"\n");
	    (void) printf("input interface -\n");
	    print_interface(intfc);
	}

	for( cc = intfc->curves;  cc && *cc;  cc++ )
	{
	    c = *cc;

	    if ((is_bdry_like_curve(c)) ||
		(wave_type(c)<FIRST_PHYSICS_WAVE_TYPE))
		continue;
	    ns = c->start;		ne = c->end;

	    if ((!is_bdry_like_node(ns) ) || (!is_bdry_like_node(ne)))
	    	continue;

	    exterior_points = NO;
	    interior_points = NO;
	    if( c->first == c->last )
	    {
		double coords_mid[MAXD];
		for (i = 0; i < dim; ++i)
		{
		    if (Coords(c->start->posn)[i] == Coords(c->end->posn)[i])
		    {
		    	exterior_points = YES;
			break;
		    }
		    coords_mid[i] = 0.5*(Coords(c->start->posn)[i] +
		    		Coords(c->end->posn)[i]);
		}
		if (outside_point(coords_mid,lo,hi,dim))
	    	    exterior_points = YES;
	    }
	    else
	    {
	        for( b = c->first;  b != c->last;  b = b->next )
	        {
	            p1 = b->end;
	            if (outside_point(Coords(p1),lo,hi,dim))
	                exterior_points = YES;
		    else
			interior_points = YES;
	        }
	    }

	    if ((exterior_points == NO) || (interior_points == YES))
	        continue;

	    /* Curve is exterior */

	    if( debugging("c_ext") )
	    {
	    	(void) printf("Nodes of exterior curve\n");

	    	(void) printf("ns -\n");	print_node(ns);
	    	(void) printf("ne -\n");	print_node(ne);
		print_curve(c);
	    }

	    if (!correct_exterior_curve(c,fr))
		return NO;

	}
	if( debugging("c_ext") )
	{
	    (void) printf("after correct_for_exterior_curves(), "
	                  "input interface -\n");
	    print_interface(intfc);
	}
	debug_print("c_ext","Left correct_for_exterior_curves()\n");
	return YES;
}		/*end correct_for_exterior_curves*/

LOCAL	boolean correct_exterior_curve(
	CURVE		*c,
	Front		*fr)
{
	INTERFACE	*intfc = fr->interf;
	NODE		*ns = c->start, *ne = c->end, *nc;
	CURVE		*nsc1, *nsc2, *nec1, *nec2, *bc, *bc1, *bc2;
	ORIENTATION	nsc1_or, nsc2_or, nec1_or, nec2_or, bc1_or, bc2_or;
	boolean		save_swtch,status;

	debug_print("c_ext_cur","Entered correct_exterior_curve()\n");

	if (debugging("c_ext_cur"))
	{
	    (void) printf("Correcting for exterior curve %llu\n",
	    	          curve_number(c));
	    print_curve(c);
	}
	find_bdry_curves_at_node(ns,&nsc1,&nsc1_or,&nsc2,&nsc2_or);
	find_bdry_curves_at_node(ne,&nec1,&nec1_or,&nec2,&nec2_or);
	if (debugging("c_ext_cur"))
	{
	    (void) printf("nsc1 = %llu, ",curve_number(nsc1));
	    print_orientation("nsc1_or = ",nsc1_or,"\n");
	    (void) printf("nsc2 = %llu, ",curve_number(nsc2));
	    print_orientation("nsc2_or = ",nsc2_or,"\n");
	    (void) printf("nec1 = %llu, ",curve_number(nec1));
	    print_orientation("nec1_or = ",nec1_or,"\n");
	    (void) printf("nec2 = %llu, ",curve_number(nec2));
	    print_orientation("nec2_or = ",nec2_or,"\n");
	}
	bc = bc1 = bc2 = NULL;
	nc = NULL;
	if ((nsc1 == nec1) || (nsc1 == nec2))
	    bc = nsc1; 
	else if ((nsc2 == nec1) || (nsc2 == nec2))
	    bc = nsc2;
	else if (Opp_node_of(nsc1,nsc1_or) == Opp_node_of(nec1,nec1_or))
	{
	    bc1 = nsc1;		bc2 = nec1;
	    bc1_or = nsc1_or;	bc2_or = nec1_or;
	    nc = Opp_node_of(nsc1,nsc1_or);
	}
	else if (Opp_node_of(nsc2,nsc2_or) == Opp_node_of(nec1,nec1_or))
	{
	    bc1 = nsc2;		bc2 = nec1;
	    bc1_or = nsc2_or;	bc2_or = nec1_or;
	    nc = Opp_node_of(nsc2,nsc2_or);
	}
	else if (Opp_node_of(nsc1,nsc1_or) == Opp_node_of(nec2,nec2_or))
	{
	    bc1 = nsc1;		bc2 = nec2;
	    bc1_or = nsc1_or;	bc2_or = nec2_or;
	    nc = Opp_node_of(nsc1,nsc1_or);
	}
	else if (Opp_node_of(nsc2,nsc2_or) == Opp_node_of(nec2,nec2_or))
	{
	    bc1 = nsc2;		bc2 = nec2;
	    bc1_or = nsc2_or;	bc2_or = nec2_or;
	    nc = Opp_node_of(nsc2,nsc2_or);
	}
	else
	{
	    (void) printf("do not share a common boundary curve or"
			      " a corner node\n");
	    debug_print("c_ext_cur","Left correct_exterior_curve()\n");
	    return NO;
	}

	if( bc == NULL && nc == NULL)
	{
	    (void) printf("ERROR in correct_for_exterior_curves(), "
	           "Unable to identify boundary curve bc or corner"
	           "node nc at curve %llu\n",curve_number(c));
	    print_interface(intfc);
	    debug_print("c_ext_cur","Left correct_exterior_curve()\n");
	    return NO;
	}

	if (debugging("c_ext_cur"))
	{
	    if (bc)
	    {
	    	(void) printf("Boundary curve bc %llu at exterior curve\n",
	    	          		curve_number(bc));
	    	print_curve(bc);
	    }
	    else if (nc)
	    {
		(void) printf("Corner node between start and end node:\n");
		print_node(nc);
	    	(void) printf("Boundary curve bc1 %llu at exterior curve\n",
	    	          		curve_number(bc1));
	    	print_curve(bc1);
	    	(void) printf("Boundary curve bc2 %llu at exterior curve\n",
	    	          		curve_number(bc2));
	    	print_curve(bc2);
	    }
	}

	save_swtch = interpolate_intfc_states(intfc);
	interpolate_intfc_states(intfc) = YES;
	if (bc)
	    status = remove_ext_curve_at_single_bdry(fr,c,bc,ns,ne);
	else if (nc)
	    status = remove_ext_curve_at_corner(fr,c,bc1,bc1_or,bc2,
	    			bc2_or,ns,ne);
	interpolate_intfc_states(intfc) = save_swtch;
	debug_print("c_ext_cur","Left correct_exterior_curve()\n");
	return status;
}		/*end correct_exterior_curve*/


EXPORT	void replace_cphys_by_cbdry(
	CURVE		*cp,
	CURVE		*cb,
	Front		*fr)
{
	BOND	  	*b;
	FT_POINT		*pnew;
	RECT_GRID 	*rgr = fr->rect_grid;
	double		*cbs, t[MAXD], v[MAXD], magt;
	double		l, llast;
	int		flag;
	size_t		sizest = fr->sizest;
	int		i, dim = rgr->dim;
	SIDE		phys_side_bc;
	boolean		sav_interp = interpolate_intfc_states(cb->interface);

	debug_print("rcbc","Entered replace_cphys_by_cbdry()\n");
	if (is_subdomain_boundary(Hyper_surf(cb)))
	{
	    debug_print("rcbc","Left replace_cphys_by_cbdry()\n");
	    return;
	}

	cbs = Coords(cb->start->posn);
	for (i = 0; i < dim; i++)
	    t[i] = Coords(cb->end->posn)[i] - cbs[i];
	magt = mag_vector(t,dim);
	interpolate_intfc_states(cb->interface) = NO;

	phys_side_bc = is_excluded_comp(negative_component(cb),cb->interface) ?
				POSITIVE_SIDE : NEGATIVE_SIDE;

	if( debugging("rcbc") )
	{
	    (void) printf("Replacing curve cp %llu by curve cb %llu\n",
	    	          curve_number(cp),curve_number(cb));
	    (void) printf("Physical curve:\n");
	    print_curve(cp);
	    (void) printf("Boundary curve:\n");
	    print_curve(cb);
	}
	if( cp->start == cb->start )
	    flag = (phys_side_bc == POSITIVE_SIDE) ? 0 : 1;
	else
	    flag = (phys_side_bc == POSITIVE_SIDE) ? 2 : 3;

	switch (flag)
	{
	case 0:
	    positive_component(cb) = positive_component(cp);
	    ft_assign(right_start_state(cb),right_start_state(cp),sizest);
	    ft_assign(right_end_state(cb),right_end_state(cp),sizest);
	    break;
	case 1:
	    negative_component(cb) = negative_component(cp);
	    ft_assign(left_start_state(cb),left_start_state(cp),sizest);
	    ft_assign(left_end_state(cb),left_end_state(cp),sizest);
	    break;
	case 2:
	    positive_component(cb) = negative_component(cp);
	    ft_assign(right_start_state(cb),left_end_state(cp),sizest);
	    ft_assign(right_end_state(cb),left_start_state(cp),sizest);
	    break;
	case 3:
	    negative_component(cb) = positive_component(cp);
	    ft_assign(left_start_state(cb),right_end_state(cp),sizest);
	    ft_assign(left_end_state(cb),right_start_state(cp),sizest);
	    break;
	}

	delete_interior_points_of_curve(fr,cb);

	if (magt > MIN_SCALED_LENGTH(fr->interf))
	{
	    for (i = 0; i < dim; i++)
		t[i] /= magt;
	    llast = 0.0;
	    for( b = cp->first;  b != cp->last;  b = b->next )
	    {
	    	for (i = 0; i < dim; i++)
	    	    v[i] = Coords(b->end)[i] - cbs[i];
	    	l = scalar_product(v,t,dim);
	    	if (l < llast)
	    	{
	    	    delete_interior_points_of_curve(fr,cb);
	    	    break;
	    	}
		if (l >= magt)
		    break;
	    	pnew = Point(NULL);
	    	for (i = 0; i < dim; i++)
	    	    Coords(pnew)[i] = cbs[i] + l*t[i];
	        if( debugging("rcbc") )
		{
		    print_general_vector("v = ",v,dim,"\n");
		    (void) printf("mag(v) = %g\n",mag_vector(v,dim));
		    (void) printf("magt = %g, l = %g\n",magt,l);
		    print_general_vector("inserting pnew at ",
					 Coords(pnew),dim,"\n");
		}
	    	if (insert_point_in_bond(pnew,cb->last,cb)!=FUNCTION_SUCCEEDED)
		{
		    screen("ERROR in replace_cphys_by_cbdry(), "
		           "insert_point_in_bond() failed\n");
		    clean_up(ERROR);
		}
	    	switch (flag%4)
	    	{
	    	case 0:
	    	    ft_assign(right_state(pnew),right_state(b->end),sizest);
		    if (flag/4)
	    	        ft_assign(left_state(pnew),right_state(b->end),sizest);
	    	    break;
	    	case 1:
	    	    ft_assign(left_state(pnew),left_state(b->end),sizest);
		    if (flag/4)
	    	        ft_assign(right_state(pnew),left_state(b->end),sizest);
	    	    break;
	    	case 2:
	    	    ft_assign(right_state(pnew),left_state(b->end),sizest);
		    if (flag/4)
	    	        ft_assign(left_state(pnew),left_state(b->end),sizest);
	    	    break;
	    	case 3:
	    	    ft_assign(left_state(pnew),right_state(b->end),sizest);
		    if (flag/4)
	    	        ft_assign(right_state(pnew),right_state(b->end),sizest);
	    	    break;
	    	}
	    	llast = l;
	    }
	}

	(void) delete_curve(cp);

	if( debugging("rcbc") )
	{
	    (void) printf("Final curve\n");
	    (void) printf("cb -\n"); print_curve(cb);
	    show_curve_states(cb);
	}
	interpolate_intfc_states(cb->interface) = sav_interp;
	debug_print("rcbc","Left replace_cphys_by_cbdry()\n");
}		/*end replace_cphys_by_cbdry*/

/*
*			curve_exits_parallel_to_bdry():
*
*	Performs the interaction which occurs when a curve exits parallel
*	to a boundary curve.  The function currently only supports
*	Dirichlet boundaries.
*/


EXPORT int curve_exits_parallel_to_bdry(
	Front		*front,
	POINTER		wave,
	RPROBLEM	*rp)
{
	RP_NODE		*rpn[2], *rp_node;
	NODE		*ns, *ne;
	O_CURVE		*ocbehind[2];
	CURVE		*null_bc[2];
	CURVE		*cphys, *cbdry, *cbehind[2];
	CURVE		*oldcphys, *oldcbdry, *oldcbehind[2];
	double		t[MAXD], tb[MAXD], cp, V[MAXD];
	double		dt = rp->dt;
	ORIENTATION	cphys_orient, cbdry_orient, cbehind_orient[2];
	SIDE		int_side, bdry_int_side;
	ORIENTATION	null_bc_or[2];
	int		dim = front->rect_grid->dim;
	int		i;
	static	FT_POINT	*pnew = NULL;

	if (pnew == NULL)
	{
	    pnew = Static_point(rp->new_intfc);
	}

	debug_print("parallel","Entered curve_exits_parallel_to_bdry()\n");
	if (debugging("parallel"))
	{
	    (void) printf("Rproblems\n");
	    (void) printf("rp\n");
	    print_rproblem(rp);
	}
	ocbehind[0] = rp->bdry_curves->first;
	if (!rp_node_with_node(&rpn[0],rp,Node_of_o_curve(ocbehind[0])))
	{
	    screen("ERROR in curve_exits_parallel_to_bdry() "
	           "rp_node_with_node() failed\n");
	    clean_up(ERROR);
	}
	if (rpn[0] == NULL)
	{
	    (void) printf("WARNING in curve_exits_parallel_to_bdry(), "
	                  "Unable to find rpn[0]\n");
	    debug_print("parallel","Left curve_exits_parallel_to_bdry()\n");
	    return ERROR_IN_STEP;
	}
	cphys = NULL;
	if ((cphys = find_physical_curve_at_node(rpn[0]->node,&cphys_orient))
								== NULL)
	{
	    (void) printf("WARNING in curve_exits_parallel_to_bdry(), "
	                  "Unable to find physical curve\n");
	    debug_print("parallel","Left curve_exits_parallel_to_bdry()\n");
	    return ERROR_IN_STEP;
	}
	if (!next_boundary(ocbehind[0]->curve,ocbehind[0]->orient,
		              null_bc,null_bc_or)) 
	{
	    (void) printf("WARNING in curve_exits_parallel_to_bdry(), "
	                  "next_boundary() failed\n");
	    debug_print("parallel","Left curve_exits_parallel_to_bdry()\n");
	    return ERROR_IN_STEP;
	}
	if (!next_boundary(null_bc[0],Opposite_orient(null_bc_or[0]),
		&cbdry,&cbdry_orient)) 
	{
	    (void) printf("ERROR in curve_exits_parallel_to_bdry(), "
	                  "next_boundary() failed\n");
	    debug_print("parallel","Left curve_exits_parallel_to_bdry()\n");
	    return ERROR_IN_STEP;
	}
	ocbehind[1] = rp->bdry_curves->last;
	if (!rp_node_with_node(&rpn[1],rp,Node_of_o_curve(ocbehind[1])))
	{
	    (void) printf("WARNING in curve_exits_parallel_to_bdry(), "
	                  "Unable to find rpn[1]\n");
	    debug_print("parallel","Left curve_exits_parallel_to_bdry()\n");
	    return ERROR_IN_STEP;
	}
	if (!next_boundary(ocbehind[1]->curve,ocbehind[1]->orient,
		null_bc+1,null_bc_or+1)) 
	{
	    (void) printf("WARNING in curve_exits_parallel_to_bdry(), "
	                  "next_boundary() failed\n");
	    debug_print("parallel","Left curve_exits_parallel_to_bdry()\n");
	    return ERROR_IN_STEP;
	}


	if (debugging("parallel"))
	{
	    (void) printf("Exiting physical curve, orient = %s\n",
	                  orientation_name(cphys_orient));
	    print_curve(cphys);
	    (void) printf("Boundary curve, orient = %s\n",
	                  orientation_name(cbdry_orient));
	    print_curve(cbdry);
	}
	
	/* Find corresponding curves on old interface */

	if (cphys_orient == POSITIVE_ORIENTATION)
	{
	    ns = rpn[0]->old_node;	ne = rpn[1]->old_node;
	}
	else
	{
	    ns = rpn[1]->old_node;	ne = rpn[0]->old_node;
	}
	oldcphys = find_correspond_curve(cphys,ns,ne,front,rp->old_intfc);

	ns = ne = NULL;
	if (rp_node_with_node(&rp_node,rp,cbdry->start))
	    ns = rp_node->old_node;
	if (rp_node_with_node(&rp_node,rp,cbdry->end))
	    ne = rp_node->old_node;
	oldcbdry = find_correspond_curve(cbdry,ns,ne,front,rp->old_intfc);

	for (i = 0; i < 2; i++)
	{
	    ns = ne = NULL;
	    cbehind[i] = ocbehind[i]->curve;
	    cbehind_orient[i] = ocbehind[i]->orient;
	    if (cbehind_orient[i] == POSITIVE_ORIENTATION)
	    	ns = rpn[i]->old_node;
	    else
	    	ne = rpn[i]->old_node;
	    oldcbehind[i] = find_correspond_curve(cbehind[i],ns,ne,front,
					rp->old_intfc);
	}
	if (oldcphys == NULL || oldcbdry == NULL || oldcbehind[0] == NULL
	 || oldcbehind[1] == NULL)
	{
	    (void) printf("WARNING in curve_exits_parallel_to_bdry(), "
	                  "Unable to find old curves\n");
	    debug_print("parallel","Left curve_exits_parallel_to_bdry()\n");
	    return ERROR_IN_STEP;
	}

	/* Propagate old curves */

	point_propagate(front,wave,oldcphys->start->posn,pnew, 
		oldcphys->first,oldcphys,dt,V);
	ft_assign(left_start_state(cphys),left_state(pnew),front->sizest);
	ft_assign(right_start_state(cphys),right_state(pnew),front->sizest);
	point_propagate(front,wave,oldcphys->end->posn,pnew, 
		oldcphys->last,oldcphys,dt,V);
	ft_assign(left_end_state(cphys),left_state(pnew),front->sizest);
	ft_assign(right_end_state(cphys),right_state(pnew),front->sizest);
	
	
	/* Find direction in which curve exits */

	find_tangent_to_curve(Node_of(oldcphys,cphys_orient)->posn,
		Bond_at_node(oldcphys,cphys_orient),oldcphys,
		cphys_orient,t,front);
	find_tangent_to_curve(Node_of(oldcbehind[0],cbehind_orient[0])->posn,
		Bond_at_node(oldcbehind[0],cbehind_orient[0]),oldcbehind[0],
		cbehind_orient[0],tb,front);
	(void) vector_product(t,tb,&cp,dim);
	if ((cp > 0.0 && cphys_orient == POSITIVE_ORIENTATION) ||
	    (cp < 0.0 && cphys_orient == NEGATIVE_ORIENTATION))
	{
	    int_side = NEGATIVE_SIDE;
	}
	else
	{
	    int_side = POSITIVE_SIDE;
	}
	find_tangent_to_curve(Node_of(oldcbdry,cbdry_orient)->posn,
		Bond_at_node(oldcbdry,cbdry_orient),oldcbdry,
		cbdry_orient,t,front);
	(void) vector_product(t,tb,&cp,dim);
	if ((cp > 0.0 && cbdry_orient == POSITIVE_ORIENTATION) ||
	    (cp < 0.0 && cbdry_orient == NEGATIVE_ORIENTATION))
	{
	    bdry_int_side = NEGATIVE_SIDE;
	}
	else
	{
	    bdry_int_side = POSITIVE_SIDE;
	}

	if (wave_type(cbdry) == NEUMANN_BOUNDARY &&
		wave_type(cphys) >= FIRST_VECTOR_PHYSICS_WAVE_TYPE &&
		front->parallel_refl_vec_wave != NULL)
	{
	    (*front->parallel_refl_vec_wave)(cphys,cphys_orient,
			int_side,cbdry,cbdry_orient,bdry_int_side,
			rp,front,wave);
	}
	else
	{
	    NODE *ntmp;
	    map_phys_cur_states_to_bdry_cur(cphys,cphys_orient,
			int_side,cbdry,cbdry_orient,bdry_int_side,
			YES,YES,rp->new_intfc,front);

	    (void) delete_curve(cphys);
	    (void) delete_curve(null_bc[0]);
	    ntmp = Node_of_o_curve(ocbehind[0]);
	    change_node_of_curve(ocbehind[0]->curve,ocbehind[0]->orient,
			Node_of(cbdry,cbdry_orient));
	    (void) delete_node(ntmp);
	    (void) delete_curve(null_bc[1]);
	    ntmp = Node_of_o_curve(ocbehind[1]);
	    if (cbdry_orient == POSITIVE_ORIENTATION)
	    {
		ft_assign(left_start_state(ocbehind[1]->curve),
			left_start_state(cbdry),front->sizest);
		ft_assign(right_start_state(ocbehind[1]->curve),
			right_start_state(cbdry),front->sizest);
	    }
	    else
	    {
		ft_assign(left_end_state(ocbehind[1]->curve),
			left_end_state(cbdry),front->sizest);
		ft_assign(right_end_state(ocbehind[1]->curve),
			right_end_state(cbdry),front->sizest);
	    }
	    change_node_of_curve(ocbehind[1]->curve,ocbehind[1]->orient,
			Node_of(cbdry,cbdry_orient));
	    (void) delete_node(ntmp);
	}
	debug_print("parallel","Left curve_exits_parallel_to_bdry()\n");
	return GOOD_STEP;
}		/*end curve_exits_parallel_to_bdry*/

LOCAL	boolean remove_ext_curve_at_single_bdry(
	Front *fr,
	CURVE *c,
	CURVE *bc,
	NODE *ns,
	NODE *ne)
{
	switch ( wave_type(bc)) 
	{
	case NEUMANN_BOUNDARY:
	    if (debugging("c_ext_cur"))
	    	(void) printf("NEUMANN_BOUNDARY\n");
	    if( wave_type(c) >= FIRST_VECTOR_PHYSICS_WAVE_TYPE )
	    {
	    	(void) printf("WARNING in correct_exterior_curve(), ");
	    	(void) printf("Boundary reflection needed ");
	    	if (debugging("c_ext_cur"))
	    	{
	    	    print_curve(c);
	    	    print_interface(c->interface);
	    	}
	    	return NO;
	    }
	    else
	    {
	    	replace_cphys_by_cbdry(c,bc,fr);
	    	ns = bc->start;		ne = bc->end;
	    	(void) delete_redundant_node(ns,NULL,NULL,fr);
	    	(void) delete_redundant_node(ne,NULL,NULL,fr);
	    }
	    break;
	case DIRICHLET_BOUNDARY:
	    if (debugging("c_ext_cur"))
	    	(void) printf("%s\n","DIRICHLET_BOUNDARY");
	    replace_cphys_by_cbdry(c,bc,fr);
	    (void) delete_redundant_node(ns,NULL,NULL,fr);
	    (void) delete_redundant_node(ne,NULL,NULL,fr);
	    break;
	case SUBDOMAIN_BOUNDARY:
	    if (debugging("c_ext_cur"))
	    	(void) printf("%s\n","SUBDOMAIN_BOUNDARY");
	    break;
	}
	return YES;
}	/* end remove_ext_curve_at_single_bdry */


LOCAL	boolean remove_ext_curve_at_corner(
	Front *fr,
	CURVE *c,
	CURVE *bc1,
	ORIENTATION bc1_or, 	/* In reference to ns */
	CURVE *bc2,
	ORIENTATION bc2_or, 	/* In reference to ne */
	NODE *ns,
	NODE *ne)
{
	Locstate st_s,st_e;
	SIDE phys_side_bc1,phys_side_bc2,phys_side_,phys_side_c;
	INTERFACE *intfc = c->interface;
	COMPONENT phys_comp,disappearing_comp;

	phys_side_bc1 = is_excluded_comp(negative_component(bc1),intfc) ?
	                POSITIVE_SIDE : NEGATIVE_SIDE;
	disappearing_comp = (phys_side_bc1 == POSITIVE_SIDE) ? 
			positive_component(bc1) : negative_component(bc1);

	phys_side_bc2 = is_excluded_comp(negative_component(bc2),intfc) ?
	                POSITIVE_SIDE : NEGATIVE_SIDE;
	phys_side_c = (negative_component(c) == disappearing_comp) ?
	                POSITIVE_SIDE : NEGATIVE_SIDE;

	if (phys_side_c == NEGATIVE_SIDE)
	{
	    st_s = left_start_state(c);
	    st_e = left_end_state(c);
	    phys_comp = negative_component(c);
	}
	else
	{
	    st_s = right_start_state(c);
	    st_e = right_end_state(c);
	    phys_comp = positive_component(c);
	}
	if (bc1_or == POSITIVE_ORIENTATION)
	{
	    if (phys_side_bc1 == NEGATIVE_SIDE)
	    {
		negative_component(bc1) = phys_comp;
	    	ft_assign(left_start_state(bc1),st_s,fr->sizest);
		bi_interpolate_intfc_states(intfc,0.5,0.5,NULL,st_s,NULL,st_e,
			left_end_state(bc1));
	    }
	    else
	    {
		positive_component(bc1) = phys_comp;
	    	ft_assign(right_start_state(bc1),st_s,fr->sizest);
		bi_interpolate_intfc_states(intfc,0.5,0.5,NULL,st_s,NULL,st_e,
			right_end_state(bc1));
	    }
	}
	else
	{
	    if (phys_side_bc1 == NEGATIVE_SIDE)
	    {
		negative_component(bc1) = phys_comp;
	    	ft_assign(left_end_state(bc1),st_s,fr->sizest);
		bi_interpolate_intfc_states(intfc,0.5,0.5,NULL,st_s,NULL,st_e,
			left_start_state(bc1));
	    }
	    else
	    {
		positive_component(bc1) = phys_comp;
	    	ft_assign(right_end_state(bc1),st_s,fr->sizest);
		bi_interpolate_intfc_states(intfc,0.5,0.5,NULL,st_s,NULL,st_e,
			right_start_state(bc1));
	    }
	}
	if (bc2_or == POSITIVE_ORIENTATION)
	{
	    if (phys_side_bc2 == NEGATIVE_SIDE)
	    {
		negative_component(bc2) = phys_comp;
	    	ft_assign(left_start_state(bc2),st_e,fr->sizest);
		bi_interpolate_intfc_states(intfc,0.5,0.5,NULL,st_s,NULL,st_e,
			left_end_state(bc2));
	    }
	    else
	    {
		positive_component(bc2) = phys_comp;
	    	ft_assign(right_start_state(bc2),st_e,fr->sizest);
		bi_interpolate_intfc_states(intfc,0.5,0.5,NULL,st_s,NULL,st_e,
			right_end_state(bc2));
	    }
	}
	else
	{
	    if (phys_side_bc2 == NEGATIVE_SIDE)
	    {
		negative_component(bc2) = phys_comp;
	    	ft_assign(left_end_state(bc2),st_e,fr->sizest);
		bi_interpolate_intfc_states(intfc,0.5,0.5,NULL,st_s,NULL,st_e,
			left_start_state(bc2));
	    }
	    else
	    {
		positive_component(bc2) = phys_comp;
	    	ft_assign(right_end_state(bc2),st_e,fr->sizest);
		bi_interpolate_intfc_states(intfc,0.5,0.5,NULL,st_s,NULL,st_e,
			right_start_state(bc2));
	    }
	}
	(void) delete_interior_points_of_curve(fr,bc1);
	(void) delete_interior_points_of_curve(fr,bc2);
	(void) delete_curve(c);
	(void) delete_redundant_node(ns,NULL,NULL,fr);
	(void) delete_redundant_node(ne,NULL,NULL,fr);
}	/* end remove_ext_curve_at_corner */
